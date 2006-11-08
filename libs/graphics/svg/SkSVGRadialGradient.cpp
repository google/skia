/* libs/graphics/svg/SkSVGRadialGradient.cpp
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
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
