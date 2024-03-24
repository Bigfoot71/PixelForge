#include "pixelforge.h"

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
        (PFubyte)(MIN(source.r + destination.r, 255)),
        (PFubyte)(MIN(source.g + destination.g, 255)),
        (PFubyte)(MIN(source.b + destination.b, 255)),
        (PFubyte)(MIN(source.a + destination.a, 255))
    };
}

PFcolor pfBlendSubtractive(PFcolor source, PFcolor destination)
{
    return (PFcolor) {
        (PFubyte)(MAX(destination.r - source.r, 0)),
        (PFubyte)(MAX(destination.g - source.g, 0)),
        (PFubyte)(MAX(destination.b - source.b, 0)),
        (PFubyte)(MAX(destination.a - source.a, 0))
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
