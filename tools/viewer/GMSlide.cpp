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
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "tools/viewer/GMSlide.h"

GMSlide::GMSlide(std::unique_ptr<skiagm::GM> gm) : fGM(std::move(gm)) {
    fGM->setMode(skiagm::GM::kSample_Mode);

    fName.printf("GM_%s", fGM->getName());
}

GMSlide::~GMSlide() = default;

void GMSlide::gpuTeardown() {
    fGM->gpuTeardown();
}

void GMSlide::draw(SkCanvas* canvas) {
    SkString msg;

    auto direct = GrAsDirectContext(canvas->recordingContext());
    auto result = fGM->gpuSetup(direct, canvas, &msg);
    if (result != skiagm::GM::DrawResult::kOk) {
        return;
    }

    fGM->draw(canvas, &msg);
}

bool GMSlide::animate(double nanos) { return fGM->animate(nanos); }

bool GMSlide::onChar(SkUnichar c) { return fGM->onChar(c); }

bool GMSlide::onGetControls(SkMetaData* controls) {
    return fGM->getControls(controls);
}

void GMSlide::onSetControls(const SkMetaData& controls) {
    fGM->setControls(controls);
}

