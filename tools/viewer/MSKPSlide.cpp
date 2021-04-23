/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/MSKPSlide.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"
#include "src/core/SkOSFile.h"

MSKPSlide::MSKPSlide(const SkString& name, const SkString& path)
        : MSKPSlide(name, SkStream::MakeFromFile(path.c_str())) {}

MSKPSlide::MSKPSlide(const SkString& name, std::unique_ptr<SkStreamSeekable> stream)
        : fStream(std::move(stream)) {
    fName = name;
}

SkISize MSKPSlide::getDimensions() const {
    return fPlayer ? fPlayer->maxDimensions() : SkISize{0, 0};
}

void MSKPSlide::draw(SkCanvas* canvas) {
    if (fPlayer) {
        fPlayer->playFrame(canvas, fFrame);
    }
}

bool MSKPSlide::animate(double nanos) {
    if (!fPlayer) {
        return false;
    }
    double elapsed = nanos - fLastFrameTime;
    double frameTime = 1E9/fFPS;
    int framesToAdvance = elapsed/frameTime;
    fFrame = (fFrame + framesToAdvance)%fPlayer->numFrames();
    // Instead of just adding elapsed, note the time when this frame should have begun.
    fLastFrameTime += framesToAdvance*frameTime;
    return framesToAdvance%fPlayer->numFrames() != 0;
}

void MSKPSlide::load(SkScalar, SkScalar) {
    if (!fStream) {
        SkDebugf("No skp stream for slide %s.\n", fName.c_str());
        return;
    }
    fStream->rewind();
    fPlayer = MSKPPlayer::Make(fStream.get());
    if (!fPlayer) {
        SkDebugf("Could parse MSKP from stream for slide %s.\n", fName.c_str());
        return;
    }
}

void MSKPSlide::unload() { fPlayer.reset(); }

void MSKPSlide::gpuTeardown() { fPlayer->resetLayers(); }

