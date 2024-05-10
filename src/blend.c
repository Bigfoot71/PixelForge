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
    return (PFcolor) {
        (PFubyte)((source.r + destination.r) >> 1),
        (PFubyte)((source.g + destination.g) >> 1),
        (PFubyte)((source.b + destination.b) >> 1),
        (PFubyte)((source.a + destination.a) >> 1)
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
        (PFubyte)MIN_255((PFint)((destination.r*(255 - source.r) >> 8) + source.r)),
        (PFubyte)MIN_255((PFint)((destination.g*(255 - source.g) >> 8) + source.g)),
        (PFubyte)MIN_255((PFint)((destination.b*(255 - source.b) >> 8) + source.b)),
        (PFubyte)MIN_255((PFint)((destination.a*(255 - source.a) >> 8) + source.a))
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
