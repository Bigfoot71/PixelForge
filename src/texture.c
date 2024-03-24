#include "pixelforge.h"
#include <stdlib.h>
#include <stddef.h>
#include <math.h>

/* Internal convert functions */

static PFushort FloatToHalf(PFfloat x)
{
    const PFuint b = (*(PFuint*)&x)+0x00001000; // round-to-nearest-even: add last bit after truncated mantissa
    const PFuint e = (b&0x7F800000)>>23; // exponent
    const PFuint m = b&0x007FFFFF; // mantissa; in line below: 0x007FF000 = 0x00800000-0x00001000 = decimal indicator flag - initial rounding
    return (b&0x80000000)>>16 | (e>112)*((((e-112)<<10)&0x7C00)|m>>13) | ((e<113)&(e>101))*((((0x007FF000+m)>>(125-e))+1)>>1) | (e>143)*0x7FFF; // sign : normalized : denormalized : saturate
}

static PFfloat HalfToFloat(PFushort x)
{
    const PFuint e = (x&0x7C00)>>10; // exponent
    const PFuint m = (x&0x03FF)<<13; // mantissa
    const PFfloat fm = (PFfloat)m;
    const PFuint v = (*(PFuint*)&fm)>>23; // evil log2 bit hack to count leading zeros in denormalized format
    const PFuint r = (x&0x8000)<<16 | (e!=0)*((e+112)<<23|m) | ((e==0)&(m!=0))*((v-37)<<23|((m<<(150-v))&0x007FE000)); // sign : normalized : denormalized
    return *(PFfloat*)&r;
}


/* Internal pixel setter functions */

static void SetGrayscale(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate grayscale equivalent color
    PFvec3f nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255 };
    PFubyte gray = (PFubyte)((nCol.x*0.299f + nCol.y*0.587f + nCol.z*0.114f)*255.0f);

    ((PFubyte*)pixels)[offset] = gray;
}

static void SetGrayAlpha(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate grayscale equivalent color
    PFvec3f nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255 };
    PFubyte gray = (PFubyte)((nCol.x*0.299f + nCol.y*0.587f + nCol.z*0.114f)*255.0f);

    ((PFubyte*)pixels)[offset*2] = gray;
    ((PFubyte*)pixels)[offset*2 + 1] = color.a;
}

static void SetR5G6B5(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate R5G6B5 equivalent color
    PFvec3f nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255 };

    PFubyte r = (PFubyte)(round(nCol.x*31.0f));
    PFubyte g = (PFubyte)(round(nCol.y*63.0f));
    PFubyte b = (PFubyte)(round(nCol.z*31.0f));

    ((PFushort*)pixels)[offset] = (PFushort)r << 11 | (PFushort)g << 5 | (PFushort)b;
}

static void SetR8G8B8(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFubyte*)pixels)[offset*3] = color.r;
    ((PFubyte*)pixels)[offset*3 + 1] = color.g;
    ((PFubyte*)pixels)[offset*3 + 2] = color.b;
}

static void SetR5G5B5A1(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate R5G5B5A1 equivalent color
    PFvec4f nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255, (PFfloat)color.a*INV_255 };

    PFubyte r = (PFubyte)(round(nCol.x*31.0f));
    PFubyte g = (PFubyte)(round(nCol.y*31.0f));
    PFubyte b = (PFubyte)(round(nCol.z*31.0f));
    PFubyte a = (nCol.w > ((PFfloat)PF_PIXELFORMAT_R5G5B5A1_ALPHA_THRESHOLD*INV_255))? 1 : 0;

    ((PFushort*)pixels)[offset] = (PFushort)r << 11 | (PFushort)g << 6 | (PFushort)b << 1 | (PFushort)a;
}

static void SetR4G4B4A4(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate R5G5B5A1 equivalent color
    PFvec4f nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255, (PFfloat)color.a*INV_255 };

    PFubyte r = (PFubyte)(round(nCol.x*15.0f));
    PFubyte g = (PFubyte)(round(nCol.y*15.0f));
    PFubyte b = (PFubyte)(round(nCol.z*15.0f));
    PFubyte a = (PFubyte)(round(nCol.w*15.0f));

    ((PFushort*)pixels)[offset] = (PFushort)r << 12 | (PFushort)g << 8 | (PFushort)b << 4 | (PFushort)a;
}

static void SetR8G8B8A8(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFuint*)pixels)[offset] = *(PFuint*)(&color);
}

static void SetR32(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate grayscale equivalent color (normalized to 32bit)
    PFvec3f nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255 };

    ((PFfloat*)pixels)[offset] = nCol.x*0.299f + nCol.y*0.587f + nCol.z*0.114f;
}

static void SetR32G32B32(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate R32G32B32 equivalent color (normalized to 32bit)
    PFvec3f nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255 };

    ((PFfloat*)pixels)[offset*3] = nCol.x;
    ((PFfloat*)pixels)[offset*3 + 1] = nCol.y;
    ((PFfloat*)pixels)[offset*3 + 2] = nCol.z;
}

static void SetR32G32B32A32(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate R32G32B32A32 equivalent color (normalized to 32bit)
    PFvec4f nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255, (PFfloat)color.a*INV_255 };

    ((PFfloat*)pixels)[offset*4] = nCol.x;
    ((PFfloat*)pixels)[offset*4 + 1] = nCol.y;
    ((PFfloat*)pixels)[offset*4 + 2] = nCol.z;
    ((PFfloat*)pixels)[offset*4 + 3] = nCol.w;
}

static void SetR16(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate grayscale equivalent color (normalized to 32bit)
    PFvec3f nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255 };

    ((PFushort*)pixels)[offset] = FloatToHalf(nCol.x*0.299f + nCol.y*0.587f + nCol.z*0.114f);
}

static void SetR16G16B16(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate R32G32B32 equivalent color (normalized to 32bit)
    PFvec3f nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255 };

    ((PFushort*)pixels)[offset*3] = FloatToHalf(nCol.x);
    ((PFushort*)pixels)[offset*3 + 1] = FloatToHalf(nCol.y);
    ((PFushort*)pixels)[offset*3 + 2] = FloatToHalf(nCol.z);
}

static void SetR16G16B16A16(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate R32G32B32A32 equivalent color (normalized to 32bit)
    PFvec4f nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255, (PFfloat)color.a*INV_255 };

    ((PFushort*)pixels)[offset*4] = FloatToHalf(nCol.x);
    ((PFushort*)pixels)[offset*4 + 1] = FloatToHalf(nCol.y);
    ((PFushort*)pixels)[offset*4 + 2] = FloatToHalf(nCol.z);
    ((PFushort*)pixels)[offset*4 + 3] = FloatToHalf(nCol.w);
}


/* Internal pixel getter functions */

static PFcolor GetGrayscale(const void* pixels, PFsizei offset)
{
    PFubyte gray = ((PFubyte*)pixels)[offset];
    return (PFcolor) { gray, gray, gray, 255 };
}

static PFcolor GetGrayAlpha(const void* pixels, PFsizei offset)
{
    PFubyte gray = ((PFubyte*)pixels)[offset*2];
    PFubyte alpha = ((PFubyte*)pixels)[offset*2 + 1];
    return (PFcolor) { gray, gray, gray, alpha };
}

static PFcolor GetR5G6B5(const void* pixels, PFsizei offset)
{
    PFushort pixel = ((PFushort*)pixels)[offset];

    return (PFcolor) {
        (PFubyte)((PFfloat)((pixel & 0b1111100000000000) >> 11)*(255/31)),
        (PFubyte)((PFfloat)((pixel & 0b0000011111100000) >> 5)*(255/63)),
        (PFubyte)((PFfloat)(pixel & 0b0000000000011111)*(255/31)),
        255
    };
}

static PFcolor GetR8G8B8(const void* pixels, PFsizei offset)
{
    return (PFcolor) {
        (PFubyte)((PFubyte*)pixels)[offset*3],
        (PFubyte)((PFubyte*)pixels)[offset*3 + 1],
        (PFubyte)((PFubyte*)pixels)[offset*3 + 2],
        255
    };
}

static PFcolor GetR5G5B5A1(const void* pixels, PFsizei offset)
{
    PFushort pixel = ((PFushort*)pixels)[offset];

    return (PFcolor) {
        (PFubyte)((PFfloat)((pixel & 0b1111100000000000) >> 11)*(255/31)),
        (PFubyte)((PFfloat)((pixel & 0b0000011111000000) >> 6)*(255/31)),
        (PFubyte)((PFfloat)((pixel & 0b0000000000111110) >> 1)*(255/31)),
        (PFubyte)((pixel & 0b0000000000000001)*255)
    };
}

static PFcolor GetR4G4B4A4(const void* pixels, PFsizei offset)
{
    PFushort pixel = ((PFushort*)pixels)[offset];

    return (PFcolor) {
        (PFubyte)((PFfloat)((pixel & 0b1111000000000000) >> 12)*(255/15)),
        (PFubyte)((PFfloat)((pixel & 0b0000111100000000) >> 8)*(255/15)),
        (PFubyte)((PFfloat)((pixel & 0b0000000011110000) >> 4)*(255/15)),
        (PFubyte)((PFfloat)(pixel & 0b0000000000001111)*(255/15))
    };
}

static PFcolor GetR8G8B8A8(const void* pixels, PFsizei offset)
{
    return (PFcolor) {
        ((PFubyte*)pixels)[offset*4],
        ((PFubyte*)pixels)[offset*4 + 1],
        ((PFubyte*)pixels)[offset*4 + 2],
        ((PFubyte*)pixels)[offset*4 + 3]
    };
}

static PFcolor GetR32(const void* pixels, PFsizei offset)
{
    return (PFcolor) {
        (PFubyte)(((PFfloat*)pixels)[offset]*255.0f),
        0, 0, 255
    };
}

static PFcolor GetR32G32B32(const void* pixels, PFsizei offset)
{
    return (PFcolor) {
        (PFubyte)(((PFfloat*)pixels)[offset*3]*255.0f),
        (PFubyte)(((PFfloat*)pixels)[offset*3 + 1]*255.0f),
        (PFubyte)(((PFfloat*)pixels)[offset*3 + 2]*255.0f),
        255
    };
}

static PFcolor GetR32G32B32A32(const void* pixels, PFsizei offset)
{
    return (PFcolor) {
        (PFubyte)(((PFfloat*)pixels)[offset*4]*255.0f),
        (PFubyte)(((PFfloat*)pixels)[offset*4]*255.0f),
        (PFubyte)(((PFfloat*)pixels)[offset*4]*255.0f),
        (PFubyte)(((PFfloat*)pixels)[offset*4]*255.0f)
    };
}

static PFcolor GetR16(const void* pixels, PFsizei offset)
{
    return (PFcolor) {
        (PFubyte)(HalfToFloat(((PFushort*)pixels)[offset])*255.0f),
        0, 0, 255
    };
}

static PFcolor GetR16G16B16(const void* pixels, PFsizei offset)
{
    return (PFcolor) {
        (PFubyte)(HalfToFloat(((PFushort*)pixels)[offset*3])*255.0f),
        (PFubyte)(HalfToFloat(((PFushort*)pixels)[offset*3 + 1])*255.0f),
        (PFubyte)(HalfToFloat(((PFushort*)pixels)[offset*3 + 2])*255.0f),
        255
    };
}

static PFcolor GetR16G16B16A16(const void* pixels, PFsizei offset)
{
    return (PFcolor) {
        (PFubyte)(HalfToFloat(((PFushort*)pixels)[offset*4])*255.0f),
        (PFubyte)(HalfToFloat(((PFushort*)pixels)[offset*4])*255.0f),
        (PFubyte)(HalfToFloat(((PFushort*)pixels)[offset*4])*255.0f),
        (PFubyte)(HalfToFloat(((PFushort*)pixels)[offset*4])*255.0f)
    };
}


/* Internal helper functions */

static void DefineGetterSetter(PFpixelgetter* getter, PFpixelsetter* setter, PFpixelformat format)
{
    switch (format)
    {
        case PF_PIXELFORMAT_GRAYSCALE:
            *getter = GetGrayscale;
            *setter = SetGrayscale;
            break;

        case PF_PIXELFORMAT_GRAY_ALPHA:
            *getter = GetGrayAlpha;
            *setter = SetGrayAlpha;
            break;

        case PF_PIXELFORMAT_R5G6B5:
            *getter = GetR5G6B5;
            *setter = SetR5G6B5;
            break;

        case PF_PIXELFORMAT_R5G5B5A1:
            *getter = GetR5G5B5A1;
            *setter = SetR5G5B5A1;
            break;

        case PF_PIXELFORMAT_R4G4B4A4:
            *getter = GetR4G4B4A4;
            *setter = SetR4G4B4A4;
            break;

        case PF_PIXELFORMAT_R8G8B8:
            *getter = GetR8G8B8;
            *setter = SetR8G8B8;
            break;

        case PF_PIXELFORMAT_R8G8B8A8:
            *getter = GetR8G8B8A8;
            *setter = SetR8G8B8A8;
            break;

        case PF_PIXELFORMAT_R32:
            *getter = GetR32;
            *setter = SetR32;
            break;

        case PF_PIXELFORMAT_R32G32B32:
            *getter = GetR32G32B32;
            *setter = SetR32G32B32;
            break;

        case PF_PIXELFORMAT_R32G32B32A32:
            *getter = GetR32G32B32A32;
            *setter = SetR32G32B32A32;
            break;

        case PF_PIXELFORMAT_R16:
            *getter = GetR16;
            *setter = SetR16;
            break;

        case PF_PIXELFORMAT_R16G16B16:
            *getter = GetR16G16B16;
            *setter = SetR16G16B16;
            break;

        case PF_PIXELFORMAT_R16G16B16A16:
            *getter = GetR16G16B16A16;
            *setter = SetR16G16B16A16;
            break;

        default: break;
    }
}

PFsizei GetPixelByte(PFpixelformat format)
{
    switch (format)
    {
        case PF_PIXELFORMAT_GRAYSCALE:      return 1;
        case PF_PIXELFORMAT_GRAY_ALPHA:
        case PF_PIXELFORMAT_R5G6B5:
        case PF_PIXELFORMAT_R5G5B5A1:
        case PF_PIXELFORMAT_R4G4B4A4:       return 2;
        case PF_PIXELFORMAT_R8G8B8A8:       return 4;
        case PF_PIXELFORMAT_R8G8B8:         return 3;
        case PF_PIXELFORMAT_R32:            return 4;
        case PF_PIXELFORMAT_R32G32B32:      return 4*3;
        case PF_PIXELFORMAT_R32G32B32A32:   return 4*4;
        case PF_PIXELFORMAT_R16:            return 2;
        case PF_PIXELFORMAT_R16G16B16:      return 2*3;
        case PF_PIXELFORMAT_R16G16B16A16:   return 2*4;
        default: break;
    }

    return 0;
}

/* Texture functions */

PFtexture pfTextureGenFromBuffer(void* pixels, PFuint width, PFuint height, PFpixelformat format)
{
    PFtexture texture = { NULL, NULL, pixels, width, height, format };
    if (format == PF_PIXELFORMAT_UNKNOWN) return texture;

    DefineGetterSetter(&texture.pixelGetter, &texture.pixelSetter, format);

    return texture;
}

PFtexture pfTextureGenBuffer(PFuint width, PFuint height, PFpixelformat format)
{
    PFtexture texture = { NULL, NULL, NULL, width, height, format };
    if (format == PF_PIXELFORMAT_UNKNOWN) return texture;

    DefineGetterSetter(&texture.pixelGetter, &texture.pixelSetter, format);

    PFsizei size = width*height;
    texture.pixels = PF_MALLOC(size*GetPixelByte(format));

    for (PFsizei i = 0; i < size; i++)
    {
        texture.pixelSetter(texture.pixels, i, (PFcolor) { 0 });
    }

    return texture;
}

PFtexture pfTextureGenBufferColor(PFuint width, PFuint height, PFcolor color, PFpixelformat format)
{
    PFtexture texture = { NULL, NULL, NULL, width, height, format };
    if (format == PF_PIXELFORMAT_UNKNOWN) return texture;

    DefineGetterSetter(&texture.pixelGetter, &texture.pixelSetter, format);

    PFsizei size = width*height;
    texture.pixels = PF_MALLOC(size*GetPixelByte(format));

    for (PFsizei i = 0; i < size; i++)
    {
        texture.pixelSetter(texture.pixels, i, color);
    }

    return texture;
}

void pfTextureDestroy(PFtexture* texture)
{
    if (texture)
    {
        if (texture->pixels)
        {
            PF_FREE(texture->pixels);
        }

        texture->pixels = NULL;
        texture->width = texture->height = 0;
        texture->format = PF_PIXELFORMAT_UNKNOWN;
    }
}

void pfTextureSetPixel(PFtexture* texture, PFuint x, PFuint y, PFcolor color)
{
    texture->pixelSetter(texture->pixels, y*texture->width + x, color);
}

PFcolor pfTextureGetPixel(const PFtexture* texture, PFuint x, PFuint y)
{
    return texture->pixelGetter(texture->pixels, y*texture->width + x);
}

void pfTextureSetFragment(PFtexture* texture, PFfloat u, PFfloat v, PFcolor color)
{
    pfTextureSetPixel(texture, (u - (PFint)u)*(texture->width-1), (v - (PFint)v)*(texture->height-1), color);
}

PFcolor pfTextureGetFragment(const PFtexture* texture, PFfloat u, PFfloat v)
{
    return pfTextureGetPixel(texture, (u - (PFint)u)*(texture->width-1), (v - (PFint)v)*(texture->height-1));
}
