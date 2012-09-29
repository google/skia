
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkSVGLinearGradient.h"
#include "SkSVGParser.h"

const SkSVGAttribute SkSVGLinearGradient::gAttributes[] = {
    SVG_ATTRIBUTE(gradientTransform),
    SVG_ATTRIBUTE(gradientUnits),
    SVG_ATTRIBUTE(x1),
    SVG_ATTRIBUTE(x2),
    SVG_ATTRIBUTE(y1),
    SVG_ATTRIBUTE(y2)
};

DEFINE_SVG_INFO(LinearGradient)

void SkSVGLinearGradient::translate(SkSVGParser& parser, bool defState) {
    if (fMatrixID.size() == 0)
        parser.translateMatrix(f_gradientTransform, &fMatrixID);
    parser._startElement("linearGradient");
    if (fMatrixID.size() > 0)
        parser._addAttribute("matrix", fMatrixID);
    INHERITED::translateGradientUnits(f_gradientUnits);
    SkString points;
    points.appendUnichar('[');
    points.append(f_x1);
    points.appendUnichar(',');
    points.append(f_y1);
    points.appendUnichar(',');
    points.append(f_x2);
    points.appendUnichar(',');
    points.append(f_y2);
    points.appendUnichar(']');
    parser._addAttribute("points", points.c_str());
    INHERITED::translate(parser, defState);
    parser._endElement();
}
