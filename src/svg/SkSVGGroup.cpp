/* libs/graphics/svg/SkSVGGroup.cpp
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

#include "SkSVGGroup.h"
#include "SkSVGParser.h"

SkSVGGroup::SkSVGGroup() {
    fIsNotDef = false;
}

SkSVGElement* SkSVGGroup::getGradient() {
    for (SkSVGElement** ptr = fChildren.begin(); ptr < fChildren.end(); ptr++) {
        SkSVGElement* result = (*ptr)->getGradient();
        if (result != NULL)
            return result;
    }
    return NULL;
}

bool SkSVGGroup::isDef() {
    return fParent ? fParent->isDef() : false;
}

bool SkSVGGroup::isFlushable() {
    return false;
}

bool SkSVGGroup::isGroup() {
    return true;
}

bool SkSVGGroup::isNotDef() {
    return fParent ? fParent->isNotDef() : false;
}

void SkSVGGroup::translate(SkSVGParser& parser, bool defState) {
    for (SkSVGElement** ptr = fChildren.begin(); ptr < fChildren.end(); ptr++)
        parser.translate(*ptr, defState);
}
