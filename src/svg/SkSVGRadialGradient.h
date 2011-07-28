
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSVGRadialGradient_DEFINED
#define SkSVGRadialGradient_DEFINED

#include "SkSVGGradient.h"

class SkSVGRadialGradient : public SkSVGGradient {
    DECLARE_SVG_INFO(RadialGradient);
protected:
    SkString f_cx;
    SkString f_cy;
    SkString f_fx;
    SkString f_fy;
    SkString f_gradientTransform;
    SkString f_gradientUnits;
    SkString f_r;
    SkString fMatrixID;
private:
    typedef SkSVGGradient INHERITED;
};

#endif // SkSVGRadialGradient_DEFINED
