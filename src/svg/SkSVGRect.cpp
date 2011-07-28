
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkSVGRect.h"
#include "SkSVGParser.h"

const SkSVGAttribute SkSVGRect::gAttributes[] = {
    SVG_ATTRIBUTE(height),
    SVG_ATTRIBUTE(width),
    SVG_ATTRIBUTE(x),
    SVG_ATTRIBUTE(y)
};

DEFINE_SVG_INFO(Rect)

SkSVGRect::SkSVGRect() {
    f_x.set("0");
    f_y.set("0");
}

void SkSVGRect::translate(SkSVGParser& parser, bool defState) {
    parser._startElement("rect");
    INHERITED::translate(parser, defState);
    SVG_ADD_ATTRIBUTE_ALIAS(left, x);
    SVG_ADD_ATTRIBUTE_ALIAS(top, y);
    SVG_ADD_ATTRIBUTE(width);
    SVG_ADD_ATTRIBUTE(height);
    parser._endElement();
}
