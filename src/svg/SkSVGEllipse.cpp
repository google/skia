/* libs/graphics/svg/SkSVGEllipse.cpp
**
** Copyright 2006, The Android Open Source Project
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
    sprintf(scratch, "%g", left);
    parser._addAttribute("left", scratch);
    sprintf(scratch, "%g", top);
    parser._addAttribute("top", scratch);
    sprintf(scratch, "%g", right);
    parser._addAttribute("right", scratch);
    sprintf(scratch, "%g", bottom);
    parser._addAttribute("bottom", scratch);
    parser._endElement();
}
