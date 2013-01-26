
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSVGFilter_DEFINED
#define SkSVGFilter_DEFINED

#include "SkSVGElements.h"

class SkSVGFilter : public SkSVGElement {
    DECLARE_SVG_INFO(Filter);
protected:
    SkString f_filterUnits;
    SkString f_height;
    SkString f_width;
    SkString f_x;
    SkString f_y;
private:
    typedef SkSVGElement INHERITED;
};

#endif // SkSVGFilter_DEFINEDRITED;
