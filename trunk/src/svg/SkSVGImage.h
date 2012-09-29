
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSVGImage_DEFINED
#define SkSVGImage_DEFINED

#include "SkSVGElements.h"

class SkSVGImage : public SkSVGElement {
public:
    DECLARE_SVG_INFO(Image);
private:
    void translateImage(SkSVGParser& parser);
    SkString f_height;
    SkString f_width;
    SkString f_x;
    SkString f_xlink_href;
    SkString f_y;
    typedef SkSVGElement INHERITED;
};

#endif // SkSVGImage_DEFINED
