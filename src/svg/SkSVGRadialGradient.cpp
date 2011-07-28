
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkSVGRadialGradient.h"
#include "SkSVGParser.h"

const SkSVGAttribute SkSVGRadialGradient::gAttributes[] = {
    SVG_ATTRIBUTE(cx),
    SVG_ATTRIBUTE(cy),
    SVG_ATTRIBUTE(fx),
    SVG_ATTRIBUTE(fy),
    SVG_ATTRIBUTE(gradientTransform),
    SVG_ATTRIBUTE(gradientUnits),
    SVG_ATTRIBUTE(r)
};

DEFINE_SVG_INFO(RadialGradient)

void SkSVGRadialGradient::translate(SkSVGParser& parser, bool defState) {
    if (fMatrixID.size() == 0)
        parser.translateMatrix(f_gradientTransform, &fMatrixID);
    parser._startElement("radialGradient");
    if (fMatrixID.size() > 0)
        parser._addAttribute("matrix", fMatrixID);
    INHERITED::translateGradientUnits(f_gradientUnits);
    SkString center;
    center.appendUnichar('[');
    center.append(f_cx);
    center.appendUnichar(',');
    center.append(f_cy);
    center.appendUnichar(']');
    parser._addAttribute("center", center);
    parser._addAttribute("radius", f_r);
    INHERITED::translate(parser, defState);
    parser._endElement();
}
