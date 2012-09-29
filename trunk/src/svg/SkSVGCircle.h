
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSVGCircle_DEFINED
#define SkSVGCircle_DEFINED

#include "SkSVGElements.h"

class SkSVGCircle : public SkSVGElement {
    DECLARE_SVG_INFO(Circle);
private:
    SkString f_cx;
    SkString f_cy;
    SkString f_r;
    typedef SkSVGElement INHERITED;
};

#endif // SkSVGCircle_DEFINED
