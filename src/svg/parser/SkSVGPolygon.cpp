/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkSVGPolygon.h"
#include "SkSVGParser.h"

const SkSVGAttribute SkSVGPolygon::gAttributes[] = {
    SVG_LITERAL_ATTRIBUTE(clip-rule, f_clipRule),
    SVG_LITERAL_ATTRIBUTE(fill-rule, f_fillRule),
    SVG_ATTRIBUTE(points)
};

DEFINE_SVG_INFO(Polygon)

void SkSVGPolygon::addAttribute(SkSVGParser& parser, int attrIndex,
        const char* attrValue, size_t attrLength) {
    INHERITED::addAttribute(parser, attrIndex, attrValue, attrLength);
}

void SkSVGPolygon::translate(SkSVGParser& parser, bool defState) {
    parser._startElement("polygon");
    SkSVGElement::translate(parser, defState);
    SVG_ADD_ATTRIBUTE(points);
    if (f_fillRule.size() > 0)
        parser._addAttribute("fillType", f_fillRule.equals("evenodd") ? "evenOdd" : "winding");
    parser._endElement();
}
