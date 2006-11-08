/* libs/graphics/svg/SkSVGUse.cpp
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

#include "SkSVGUse.h"
#include "SkSVGParser.h"

const SkSVGAttribute SkSVGUse::gAttributes[] = {
    SVG_ATTRIBUTE(height),
    SVG_ATTRIBUTE(width),
    SVG_ATTRIBUTE(x),
    SVG_LITERAL_ATTRIBUTE(xlink:href, f_xlink_href),
    SVG_ATTRIBUTE(y)
};

DEFINE_SVG_INFO(Use)

void SkSVGUse::translate(SkSVGParser& parser, bool defState) {
    INHERITED::translate(parser, defState);
    parser._startElement("add");
    const char* start = strchr(f_xlink_href.c_str(), '#') + 1;
    SkASSERT(start);
    parser._addAttributeLen("use", start, strlen(start) - 1);
    parser._endElement();   // clip
}
