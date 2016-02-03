/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Gr1DKernelEffect_DEFINED
#define Gr1DKernelEffect_DEFINED

#include "GrSingleTextureEffect.h"
#include "SkMatrix.h"

/**
 * Base class for 1D kernel effects. The kernel operates either in X or Y and
 * has a pixel radius. The kernel is specified in the src texture's space
 * and the kernel center is pinned to a texel's center. The radius specifies
 * the number of texels on either side of the center texel in X or Y that are
 * read. Since the center pixel is also read, the total width is one larger than
 * two times the radius.
 */

class Gr1DKernelEffect : public GrSingleTextureEffect {

public:
    enum Direction {
        kX_Direction,
        kY_Direction,
    };

    Gr1DKernelEffect(GrTexture* texture,
                     Direction direction,
                     int radius)
        : INHERITED(texture, GrCoordTransform::MakeDivByTextureWHMatrix(texture))
        , fDirection(direction)
        , fRadius(radius) {}

    virtual ~Gr1DKernelEffect() {};

    static int WidthFromRadius(int radius) { return 2 * radius + 1; }

    int radius() const { return fRadius; }
    int width() const { return WidthFromRadius(fRadius); }
    Direction direction() const { return fDirection; }

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("Direction: %s, Radius: %d ", kX_Direction == fDirection ? "X" : "Y", fRadius);
        str.append(INHERITED::dumpInfo());
        return str;
    }

private:

    Direction       fDirection;
    int             fRadius;

    typedef GrSingleTextureEffect INHERITED;
};

#endif
