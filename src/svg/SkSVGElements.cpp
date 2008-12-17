/* libs/graphics/svg/SkSVGElements.cpp
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

#include "SkSVGElements.h"
#include "SkSVGParser.h"

SkSVGBase::~SkSVGBase() {
}

void SkSVGBase::addAttribute(SkSVGParser& parser, int attrIndex, 
        const char* attrValue, size_t attrLength) {
    SkString* first = (SkString*) ((char*) this + sizeof(SkSVGElement));
    first += attrIndex;
    first->set(attrValue, attrLength);
}


SkSVGElement::SkSVGElement() : fParent(NULL), fIsDef(false), fIsNotDef(true) {
}

SkSVGElement::~SkSVGElement() {
}

SkSVGElement* SkSVGElement::getGradient() {
    return NULL;
}

bool SkSVGElement::isGroupParent() {
    SkSVGElement* parent = fParent;
    while (parent) {
        if (parent->getType() != SkSVGType_G)
            return false;
        parent = parent->fParent;
    }
    return true;
}

bool SkSVGElement::isDef() {
    return isGroupParent() == false ? fParent->isDef() : fIsDef;
}

bool SkSVGElement::isFlushable() {
    return true;
}

bool SkSVGElement::isGroup() {
    return false;
}

bool SkSVGElement::isNotDef() {
    return isGroupParent() == false ? fParent->isNotDef() : fIsNotDef;
}

bool SkSVGElement::onEndElement(SkSVGParser& parser) {
    if (f_id.size() > 0)
        parser.getIDs().set(f_id.c_str(), f_id.size(), this);
    return false;
}

bool SkSVGElement::onStartElement(SkSVGElement* child) {
    *fChildren.append() = child;
    return false;
}

void SkSVGElement::translate(SkSVGParser& parser, bool) {
    if (f_id.size() > 0)
        SVG_ADD_ATTRIBUTE(id);
}

void SkSVGElement::setIsDef() {
    fIsDef = isDef();
}

//void SkSVGElement::setIsNotDef() {
//  fIsNotDef = isNotDef();
//}

void SkSVGElement::write(SkSVGParser& , SkString& ) {
    SkASSERT(0); 
}


