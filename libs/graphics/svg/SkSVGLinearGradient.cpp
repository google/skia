/* libs/graphics/svg/SkSVGLinearGradient.cpp
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
