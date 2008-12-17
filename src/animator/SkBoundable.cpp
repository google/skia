/* libs/graphics/animator/SkBoundable.cpp
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

#include "SkBoundable.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"

SkBoundable::SkBoundable() {
    clearBounds();
    fBounds.fTop = 0;
    fBounds.fRight = 0;
    fBounds.fBottom = 0;
}

void SkBoundable::clearBounder() {
    fBounds.fLeft = 0x7fff;
}

void SkBoundable::getBounds(SkRect* rect) {
    SkASSERT(rect);
    if (fBounds.fLeft == (int16_t)0x8000U) {
        INHERITED::getBounds(rect);
        return;
    }
    rect->fLeft = SkIntToScalar(fBounds.fLeft);
    rect->fTop = SkIntToScalar(fBounds.fTop);
    rect->fRight = SkIntToScalar(fBounds.fRight);
    rect->fBottom = SkIntToScalar(fBounds.fBottom);
}

void SkBoundable::enableBounder() {
    fBounds.fLeft = 0;
}


SkBoundableAuto::SkBoundableAuto(SkBoundable* boundable, 
        SkAnimateMaker& maker) : fBoundable(boundable), fMaker(maker) {
    if (fBoundable->hasBounds()) {
        fMaker.fCanvas->setBounder(&maker.fDisplayList);
        fMaker.fDisplayList.fBounds.setEmpty();
    }
}

SkBoundableAuto::~SkBoundableAuto() {
    if (fBoundable->hasBounds() == false)
        return;
    fMaker.fCanvas->setBounder(NULL);
    fBoundable->setBounds(fMaker.fDisplayList.fBounds);
}

