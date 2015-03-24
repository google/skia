/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkDQuadImplicit_DEFINED
#define SkDQuadImplicit_DEFINED

#include "SkPathOpsQuad.h"

class SkDQuadImplicit {
public:
    explicit SkDQuadImplicit(const SkDQuad& q);

    bool match(const SkDQuadImplicit& two) const;
    static bool Match(const SkDQuad& quad1, const SkDQuad& quad2);

    double x2() const { return fP[kXx_Coeff]; }
    double xy() const { return fP[kXy_Coeff]; }
    double y2() const { return fP[kYy_Coeff]; }
    double x() const { return fP[kX_Coeff]; }
    double y() const { return fP[kY_Coeff]; }
    double c() const { return fP[kC_Coeff]; }

private:
    enum Coeffs {
        kXx_Coeff,
        kXy_Coeff,
        kYy_Coeff,
        kX_Coeff,
        kY_Coeff,
        kC_Coeff,
    };

    double fP[kC_Coeff + 1];
};

#endif
