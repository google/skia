/* libs/graphics/svg/SkSVGStop.cpp
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

#include "SkSVGStop.h"
#include "SkSVGParser.h"

const SkSVGAttribute SkSVGStop::gAttributes[] = {
    SVG_ATTRIBUTE(offset)
};

DEFINE_SVG_INFO(Stop)

void SkSVGStop::translate(SkSVGParser& parser, bool defState) {
    parser._startElement("color");
    INHERITED::translate(parser, defState);
    parser._addAttribute("color", parser.getPaintLast(SkSVGPaint::kStopColor));
    parser._endElement();
}
