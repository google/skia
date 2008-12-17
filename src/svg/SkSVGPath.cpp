/* libs/graphics/svg/SkSVGPath.cpp
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

#include "SkSVGPath.h"
#include "SkSVGParser.h"

const SkSVGAttribute SkSVGPath::gAttributes[] = {
    SVG_ATTRIBUTE(d)
};

DEFINE_SVG_INFO(Path)

void SkSVGPath::translate(SkSVGParser& parser, bool defState) {
    parser._startElement("path");
    INHERITED::translate(parser, defState);
    bool hasMultiplePaths = false;
    const char* firstZ = strchr(f_d.c_str(), 'z');
    if (firstZ != NULL) {
        firstZ++; // skip over 'z'
        while (*firstZ == ' ')
            firstZ++;
        hasMultiplePaths = *firstZ != '\0';
    }
    if (hasMultiplePaths) {
        SkString& fillRule = parser.getPaintLast(SkSVGPaint::kFillRule);
        if (fillRule.size() > 0) 
            parser._addAttribute("fillType", fillRule.equals("evenodd") ? "evenOdd" : "winding");
    }
    SVG_ADD_ATTRIBUTE(d);
    parser._endElement();
}
