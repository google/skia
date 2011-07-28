
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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


