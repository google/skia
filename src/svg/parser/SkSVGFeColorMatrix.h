/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSVGFeColorMatrix_DEFINED
#define SkSVGFeColorMatrix_DEFINED

#include "SkSVGElements.h"

class SkSVGFeColorMatrix : public SkSVGElement {
    DECLARE_SVG_INFO(FeColorMatrix);
protected:
    SkString f_color_interpolation_filters;
    SkString f_result;
    SkString f_type;
    SkString f_values;
private:
    typedef SkSVGElement INHERITED;
};

#endif // SkSVGFeColorMatrix_DEFINED
