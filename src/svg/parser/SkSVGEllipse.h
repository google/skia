
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSVGEllipse_DEFINED
#define SkSVGEllipse_DEFINED

#include "SkSVGElements.h"

class SkSVGEllipse : public SkSVGElement {
    DECLARE_SVG_INFO(Ellipse);
private:
    SkString f_cx;
    SkString f_cy;
    SkString f_rx;
    SkString f_ry;
    typedef SkSVGElement INHERITED;
};

#endif // SkSVGEllipse_DEFINED
