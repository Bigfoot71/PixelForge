/**
 *  Copyright (c) 2024 Le Juez Victor
 *
 *  This software is provided "as-is", without any express or implied warranty. In no event 
 *  will the authors be held liable for any damages arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose, including commercial 
 *  applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not claim that you 
 *  wrote the original software. If you use this software in a product, an acknowledgment 
 *  in the product documentation would be appreciated but is not required.
 *
 *  2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *  as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#include "pixelforge.h"
#include <limits.h>

/* Blending API functions */

PFcolor pfBlend(PFcolor source, PFcolor destination)
{
    PFcolor result;
    result.r = (PFubyte)((source.r + destination.r) >> 1);
    result.g = (PFubyte)((source.g + destination.g) >> 1);
    result.b = (PFubyte)((source.b + destination.b) >> 1);
    result.a = (PFubyte)((source.a + destination.a) >> 1);
    return result;
}

PFcolor pfBlendAlpha(PFcolor source, PFcolor destination)
{
    PFuint alpha = source.a + 1;
    PFuint invAlpha = 256 - alpha;

    PFcolor result;
    result.r = (PFubyte)((alpha*source.r + invAlpha*destination.r) >> 8);
    result.g = (PFubyte)((alpha*source.g + invAlpha*destination.g) >> 8);
    result.b = (PFubyte)((alpha*source.b + invAlpha*destination.b) >> 8);
    result.a = (PFubyte)((alpha*255 + invAlpha*destination.a) >> 8);
    return result;
}

PFcolor pfBlendAdditive(PFcolor source, PFcolor destination)
{
    PFcolor result;
    result.r = (PFubyte)MIN_255((PFint)(destination.r + source.r));
    result.g = (PFubyte)MIN_255((PFint)(destination.g + source.g));
    result.b = (PFubyte)MIN_255((PFint)(destination.b + source.b));
    result.a = (PFubyte)MIN_255((PFint)(destination.a + source.a));
    return result;
}

PFcolor pfBlendSubtractive(PFcolor source, PFcolor destination)
{
    PFcolor result;
    result.r = (PFubyte)MAX_0((PFint)(destination.r - source.r));
    result.g = (PFubyte)MAX_0((PFint)(destination.g - source.g));
    result.b = (PFubyte)MAX_0((PFint)(destination.b - source.b));
    result.a = (PFubyte)MAX_0((PFint)(destination.a - source.a));
    return result;
}

PFcolor pfBlendMultiplicative(PFcolor source, PFcolor destination)
{
    PFcolor result;
    result.r = (PFubyte)((source.r*destination.r)/255);
    result.g = (PFubyte)((source.g*destination.g)/255);
    result.b = (PFubyte)((source.b*destination.b)/255);
    result.a = (PFubyte)((source.a*destination.a)/255);
    return result;
}

PFcolor pfBlendScreen(PFcolor source, PFcolor destination)
{
    PFcolor result;
    result.r = (PFubyte)MIN_255((PFint)((destination.r*(255 - source.r) >> 8) + source.r));
    result.g = (PFubyte)MIN_255((PFint)((destination.g*(255 - source.g) >> 8) + source.g));
    result.b = (PFubyte)MIN_255((PFint)((destination.b*(255 - source.b) >> 8) + source.b));
    result.a = (PFubyte)MIN_255((PFint)((destination.a*(255 - source.a) >> 8) + source.a));
    return result;
}

PFcolor pfBlendLighten(PFcolor source, PFcolor destination)
{
    PFcolor result;
    result.r = (PFubyte)(MAX(source.r, destination.r));
    result.g = (PFubyte)(MAX(source.g, destination.g));
    result.b = (PFubyte)(MAX(source.b, destination.b));
    result.a = (PFubyte)(MAX(source.a, destination.a));
    return result;
}

PFcolor pfBlendDarken(PFcolor source, PFcolor destination)
{
    PFcolor result;
    result.r = (PFubyte)(MIN(source.r, destination.r));
    result.g = (PFubyte)(MIN(source.g, destination.g));
    result.b = (PFubyte)(MIN(source.b, destination.b));
    result.a = (PFubyte)(MIN(source.a, destination.a));
    return result;
}
