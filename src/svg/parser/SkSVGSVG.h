/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSVGSVG_DEFINED
#define SkSVGSVG_DEFINED

#include "SkSVGElements.h"

class SkSVGSVG : public SkSVGElement {
    DECLARE_SVG_INFO(SVG);
    virtual bool isFlushable();
private:
    SkString f_enable_background;
    SkString f_height;
    SkString f_overflow;
    SkString f_width;
    SkString f_version;
    SkString f_viewBox;
    SkString f_x;
    SkString f_xml_space;
    SkString f_xmlns;
    SkString f_xml_xlink;
    SkString f_y;

    typedef SkSVGElement INHERITED;
};

#endif // SkSVGSVG_DEFINED
