/* libs/graphics/svg/SkSVGSVG.cpp
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

#include "SkSVGSVG.h"
#include "SkParse.h"
#include "SkRect.h"
#include "SkSVGParser.h"

const SkSVGAttribute SkSVGSVG::gAttributes[] = {
    SVG_LITERAL_ATTRIBUTE(enable-background, f_enable_background),
    SVG_ATTRIBUTE(height),
    SVG_ATTRIBUTE(overflow),
    SVG_ATTRIBUTE(width),
    SVG_ATTRIBUTE(version),
    SVG_ATTRIBUTE(viewBox),
    SVG_LITERAL_ATTRIBUTE(xml:space, f_xml_space),
    SVG_ATTRIBUTE(xmlns),
    SVG_LITERAL_ATTRIBUTE(xmlns:xlink, f_xml_xlink)
};

DEFINE_SVG_INFO(SVG)


bool SkSVGSVG::isFlushable() {
    return false;
}

void SkSVGSVG::translate(SkSVGParser& parser, bool defState) {
    SkScalar height, width;
    SkScalar viewBox[4];
    const char* hSuffix = SkParse::FindScalar(f_height.c_str(), &height);
    if (strcmp(hSuffix, "pt") == 0)
        height = SkScalarMulDiv(height, SK_Scalar1 * 72, SK_Scalar1 * 96);
    const char* wSuffix = SkParse::FindScalar(f_width.c_str(), &width);
    if (strcmp(wSuffix, "pt") == 0)
        width = SkScalarMulDiv(width, SK_Scalar1 * 72, SK_Scalar1 * 96);
    SkParse::FindScalars(f_viewBox.c_str(), viewBox, 4);
    SkRect box;
    box.fLeft = SkScalarDiv(viewBox[0], width);
    box.fTop = SkScalarDiv(viewBox[1], height);
    box.fRight = SkScalarDiv(viewBox[2], width);
    box.fBottom = SkScalarDiv(viewBox[3], height);
    if (box.fLeft == 0 && box.fTop == 0 && 
        box.fRight == SK_Scalar1 && box.fBottom == SK_Scalar1) 
            return;
    parser._startElement("matrix");
    if (box.fLeft != 0) {
        SkString x;
        x.appendScalar(box.fLeft);
        parser._addAttributeLen("translateX", x.c_str(), x.size());
    }
    if (box.fTop != 0) {
        SkString y;
        y.appendScalar(box.fTop);
        parser._addAttributeLen("translateY", y.c_str(), y.size());
    }
    if (box.fRight != SK_Scalar1) {
        SkString x;
        x.appendScalar(box.fRight);
        parser._addAttributeLen("scaleX", x.c_str(), x.size());
    }
    if (box.fBottom != SK_Scalar1) {
        SkString y;
        y.appendScalar(box.fBottom);
        parser._addAttributeLen("scaleY", y.c_str(), y.size());
    }
    parser._endElement();   
}
