/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkSVGEllipse.h"
#include "SkSVGParser.h"
#include "SkParse.h"
#include <stdio.h>

const SkSVGAttribute SkSVGEllipse::gAttributes[] = {
    SVG_ATTRIBUTE(cx),
    SVG_ATTRIBUTE(cy),
    SVG_ATTRIBUTE(rx),
    SVG_ATTRIBUTE(ry)
};

DEFINE_SVG_INFO(Ellipse)

void SkSVGEllipse::translate(SkSVGParser& parser, bool defState) {
    parser._startElement("oval");
    INHERITED::translate(parser, defState);
    SkScalar cx, cy, rx, ry;
    SkParse::FindScalar(f_cx.c_str(), &cx);
    SkParse::FindScalar(f_cy.c_str(), &cy);
    SkParse::FindScalar(f_rx.c_str(), &rx);
    SkParse::FindScalar(f_ry.c_str(), &ry);
    SkScalar left, top, right, bottom;
    left = cx - rx;
    top = cy - ry;
    right = cx + rx;
    bottom = cy + ry;
    char scratch[16];
    sprintf(scratch, "%g", SkScalarToDouble(left));
    parser._addAttribute("left", scratch);
    sprintf(scratch, "%g", SkScalarToDouble(top));
    parser._addAttribute("top", scratch);
    sprintf(scratch, "%g", SkScalarToDouble(right));
    parser._addAttribute("right", scratch);
    sprintf(scratch, "%g", SkScalarToDouble(bottom));
    parser._addAttribute("bottom", scratch);
    parser._endElement();
}
