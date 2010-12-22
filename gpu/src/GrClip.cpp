/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "GrClip.h"

GrClip::GrClip() {
    fBounds.setEmpty();
    this->validate();
}

GrClip::GrClip(const GrClip& src) {
    *this = src;
}

GrClip::GrClip(GrClipIterator* iter) {
    fBounds.setEmpty();
    this->setFromIterator(iter);
}

GrClip::~GrClip() {}

GrClip& GrClip::operator=(const GrClip& src) {
    fList = src.fList;
    fBounds = src.fBounds;
    this->validate();
    return *this;
}

void GrClip::setEmpty() {
    fList.reset();
    fBounds.setEmpty();
    this->validate();
}

void GrClip::setRect(const GrIRect& r) {
    fList.reset();

    // we need a canonical "empty" rect, so that our operator== will behave
    // correctly with two empty clips.
    if (r.isEmpty()) {
        fBounds.setEmpty();
    } else {
        fBounds = r;
    }
    this->validate();
}

void GrClip::addRect(const GrIRect& r) {
    if (!r.isEmpty()) {
        this->validate();
        if (this->isEmpty()) {
            GrAssert(fList.count() == 0);
            fBounds = r;
        } else {
            if (this->isRect()) {
                *fList.append() = fBounds;
            }
            *fList.append() = r;
            fBounds.unionWith(r);
        }
        this->validate();
    }
}

void GrClip::setFromIterator(GrClipIterator* iter) {
    this->setEmpty();
    if (iter) {
        for (iter->rewind(); !iter->isDone(); iter->next()) {
            GrIRect r;
            iter->getRect(&r);
            this->addRect(r);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void GrClipIter::reset(const GrClip& clip) { fClip = &clip; fIndex = 0; }

bool GrClipIter::isDone() { 
    return (NULL == fClip) || (fIndex >= fClip->countRects()); 
}

void GrClipIter::rewind() { fIndex = 0; }
void GrClipIter::getRect(GrIRect* r) { *r = fClip->getRects()[fIndex]; }
void GrClipIter::next() { fIndex += 1; }
void GrClipIter::computeBounds(GrIRect* r) { 
    if (NULL == fClip) {
        r->setEmpty();
    } else {
        *r = fClip->getBounds();
    }
}

///////////////////////////////////////////////////////////////////////////////

#if GR_DEBUG

void GrClip::validate() const {
    if (fBounds.isEmpty()) {
        GrAssert(0 == fBounds.fLeft);
        GrAssert(0 == fBounds.fTop);
        GrAssert(0 == fBounds.fRight);
        GrAssert(0 == fBounds.fBottom);
        GrAssert(0 == fList.count());
    } else {
        int count = fList.count();
        if (count > 0) {
            GrAssert(count > 1);
            GrAssert(!fList[0].isEmpty());
            GrIRect bounds = fList[0];
            for (int i = 1; i < count; i++) {
                GrAssert(!fList[i].isEmpty());
                bounds.unionWith(fList[i]);
            }
            GrAssert(fBounds == bounds);
        }
    }
}

#endif

