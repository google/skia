/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
/*
* Copyright 2014 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkCanvas.h"
#include "tools/viewer/GMSlide.h"

GMSlide::GMSlide(std::unique_ptr<skiagm::GM> gm) : fGM(std::move(gm)) {
    fName.printf("GM_%s", fGM->getName());
}

GMSlide::~GMSlide() = default;

void GMSlide::draw(SkCanvas* canvas) {
    // Do we care about timing the draw of the background (once)?
    // Does the GM ever rely on drawBackground to lazily compute something?
    fGM->drawBackground(canvas);
    fGM->drawContent(canvas);
}

bool GMSlide::animate(double nanos) { return fGM->animate(nanos); }

bool GMSlide::onChar(SkUnichar c) { return fGM->onChar(c); }

bool GMSlide::onGetControls(SkMetaData* controls) {
    return fGM->getControls(controls);
}

void GMSlide::onSetControls(const SkMetaData& controls) {
    fGM->setControls(controls);
}

