/* libs/graphics/svg/SkSVGLine.cpp
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

#include "SkSVGLine.h"
#include "SkSVGParser.h"

const SkSVGAttribute SkSVGLine::gAttributes[] = {
    SVG_ATTRIBUTE(x1),
    SVG_ATTRIBUTE(x2),
    SVG_ATTRIBUTE(y1),
    SVG_ATTRIBUTE(y2)
};

DEFINE_SVG_INFO(Line)

void SkSVGLine::translate(SkSVGParser& parser, bool defState) {
    parser._startElement("line");
    INHERITED::translate(parser, defState);
    SVG_ADD_ATTRIBUTE(x1);
    SVG_ADD_ATTRIBUTE(y1);
    SVG_ADD_ATTRIBUTE(x2);
    SVG_ADD_ATTRIBUTE(y2);
    parser._endElement();
}
