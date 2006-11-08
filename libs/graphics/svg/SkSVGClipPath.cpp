/* libs/graphics/svg/SkSVGClipPath.cpp
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

#include "SkSVGClipPath.h"
#include "SkSVGParser.h"
#include "SkSVGUse.h"

DEFINE_SVG_NO_INFO(ClipPath)

bool SkSVGClipPath::isDef() {
    return true;
}

bool SkSVGClipPath::isNotDef() {
    return false;
}

void SkSVGClipPath::translate(SkSVGParser& parser, bool defState) {
    parser._startElement("clip");
    INHERITED::translate(parser, defState);
    SkASSERT(fChildren.count() == 1);
    SkSVGElement* child = *fChildren.begin();
    SkASSERT(child->getType() == SkSVGType_Use);
    SkSVGUse* use = (SkSVGUse*) child;
    SkSVGElement* ref;
    const char* refStr = &use->f_xlink_href.c_str()[1];
    SkASSERT(parser.getIDs().find(refStr, &ref));
    SkASSERT(ref);
    if (ref->getType() == SkSVGType_Rect) 
        parser._addAttribute("rectangle", refStr);
    else
        parser._addAttribute("path", refStr);
    parser._endElement();
}
