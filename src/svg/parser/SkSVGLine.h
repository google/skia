/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSVGLine_DEFINED
#define SkSVGLine_DEFINED

#include "SkSVGElements.h"

class SkSVGLine : public SkSVGElement {
    DECLARE_SVG_INFO(Line);
private:
    SkString f_x1;
    SkString f_x2;
    SkString f_y1;
    SkString f_y2;
    typedef SkSVGElement INHERITED;
};

#endif // SkSVGLine_DEFINED
