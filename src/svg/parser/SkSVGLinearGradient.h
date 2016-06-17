/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSVGLinearGradient_DEFINED
#define SkSVGLinearGradient_DEFINED

#include "SkSVGGradient.h"

class SkSVGLinearGradient : public SkSVGGradient {
    DECLARE_SVG_INFO(LinearGradient);
private:
    SkString f_gradientTransform;
    SkString f_gradientUnits;
    SkString f_x1;
    SkString f_x2;
    SkString f_y1;
    SkString f_y2;
    SkString fMatrixID;
    typedef SkSVGGradient INHERITED;
};

#endif // SkSVGLinearGradient_DEFINED
