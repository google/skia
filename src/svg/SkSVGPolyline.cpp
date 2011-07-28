
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkSVGPolyline.h"
#include "SkSVGParser.h"

enum {
    kCliipRule,
    kFillRule,
    kPoints
};

const SkSVGAttribute SkSVGPolyline::gAttributes[] = {
    SVG_LITERAL_ATTRIBUTE(clip-rule, f_clipRule),
    SVG_LITERAL_ATTRIBUTE(fill-rule, f_fillRule),
    SVG_ATTRIBUTE(points)
};

DEFINE_SVG_INFO(Polyline)

void SkSVGPolyline::addAttribute(SkSVGParser& , int attrIndex, 
        const char* attrValue, size_t attrLength) {
    if (attrIndex != kPoints)
        return;
    f_points.set("[");
    f_points.append(attrValue, attrLength);
    SkSVGParser::ConvertToArray(f_points);
}

void SkSVGPolyline::translate(SkSVGParser& parser, bool defState) {
    parser._startElement("polyline");
    INHERITED::translate(parser, defState);
    SVG_ADD_ATTRIBUTE(points);
    if (f_fillRule.size() > 0) 
        parser._addAttribute("fillType", f_fillRule.equals("evenodd") ? "evenOdd" : "winding");
    parser._endElement();
}
