#include "pixelforge.h"
#include <limits.h>

/* Blending API functions */

PFcolor pfBlend(PFcolor source, PFcolor destination)
{
    return (PFcolor) {
        (PFubyte)((source.r + destination.r)/2),
        (PFubyte)((source.g + destination.g)/2),
        (PFubyte)((source.b + destination.b)/2),
        (PFubyte)((source.a + destination.a)/2)
    };
}

PFcolor pfBlendAlpha(PFcolor source, PFcolor destination)
{
    PFuint alpha = source.a + 1;
    PFuint invAlpha = 256 - alpha;

    return (PFcolor) {
        (PFubyte)((alpha*source.r + invAlpha*destination.r) >> 8),
        (PFubyte)((alpha*source.g + invAlpha*destination.g) >> 8),
        (PFubyte)((alpha*source.b + invAlpha*destination.b) >> 8),
        (PFubyte)((alpha*255 + invAlpha*destination.a) >> 8)
    };
}

PFcolor pfBlendAdditive(PFcolor source, PFcolor destination)
{
    return (PFcolor) {
        (PFubyte)MIN_255((PFint)(destination.r + source.r)),
        (PFubyte)MIN_255((PFint)(destination.g + source.g)),
        (PFubyte)MIN_255((PFint)(destination.b + source.b)),
        (PFubyte)MIN_255((PFint)(destination.a + source.a))
    };
}

PFcolor pfBlendSubtractive(PFcolor source, PFcolor destination)
{
    return (PFcolor) {
        (PFubyte)MAX_0((PFint)(destination.r - source.r)),
        (PFubyte)MAX_0((PFint)(destination.g - source.g)),
        (PFubyte)MAX_0((PFint)(destination.b - source.b)),
        (PFubyte)MAX_0((PFint)(destination.a - source.a))
    };
}

PFcolor pfBlendMultiplicative(PFcolor source, PFcolor destination)
{
    return (PFcolor) {
        (PFubyte)((source.r*destination.r)/255),
        (PFubyte)((source.g*destination.g)/255),
        (PFubyte)((source.b*destination.b)/255),
        (PFubyte)((source.a*destination.a)/255)
    };
}

PFcolor pfBlendScreen(PFcolor source, PFcolor destination)
{
    return (PFcolor) {
        (PFubyte)MIN_255((PFint)(destination.r * (255 - source.r) / 255 + source.r)),
        (PFubyte)MIN_255((PFint)(destination.g * (255 - source.g) / 255 + source.g)),
        (PFubyte)MIN_255((PFint)(destination.b * (255 - source.b) / 255 + source.b)),
        (PFubyte)MIN_255((PFint)(destination.a * (255 - source.a) / 255 + source.a))
    };
}

PFcolor pfBlendLighten(PFcolor source, PFcolor destination)
{
    return (PFcolor) {
        (PFubyte)(MAX(source.r, destination.r)),
        (PFubyte)(MAX(source.g, destination.g)),
        (PFubyte)(MAX(source.b, destination.b)),
        (PFubyte)(MAX(source.a, destination.a))
    };
}

PFcolor pfBlendDarken(PFcolor source, PFcolor destination)
{
    return (PFcolor) {
        (PFubyte)(MIN(source.r, destination.r)),
        (PFubyte)(MIN(source.g, destination.g)),
        (PFubyte)(MIN(source.b, destination.b)),
        (PFubyte)(MIN(source.a, destination.a))
    };
}
