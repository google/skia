/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "VisualDebugModule.h"

#include "SkCanvas.h"

VisualDebugModule::VisualDebugModule(VisualBench* owner)
    : fState(kInit_State)
    , fIndex(0)
    , fOwner(owner) {
    // VisualDebugModule only really makes sense for SKPs
    fBenchmarkStream.reset(new VisualBenchmarkStream(owner->getSurfaceProps(), true));
}

bool VisualDebugModule::advanceIfNecessary(SkCanvas* canvas) {
    Benchmark* benchmark = fBenchmarkStream->current();
    switch (fState) {
        case kInit_State: {
            // setup new benchmark
            benchmark->delayedSetup();
            fOwner->clear(canvas, SK_ColorWHITE, 3);
            benchmark->preTimingHooks(canvas);

            // reset debug canvas
            SkIPoint size = benchmark->getSize();
            fDebugCanvas.reset(new SkDebugCanvas(size.fX, size.fY));

            // pour benchmark into canvas
            benchmark->draw(1, fDebugCanvas);
            fIndex = fDebugCanvas->getSize() - 1;
            fState = kPlay_State;
            break;
        }
        case kPlay_State: break;
        case kNext_State:
            // cleanup after the last SKP
            benchmark->postTimingHooks(canvas);
            fOwner->reset();
            if (!fBenchmarkStream->next()) {
                SkDebugf("Exiting VisualBench successfully\n");
                fOwner->closeWindow();
                return false;
            }
            fState = kInit_State;
            break;
    }
    return true;
}

void VisualDebugModule::draw(SkCanvas* canvas) {
    if (!fBenchmarkStream->current() || !this->advanceIfNecessary(canvas)) {
        return;
    }

    fDebugCanvas->drawTo(canvas, fIndex);
    canvas->flush();
    fOwner->present();
}

bool VisualDebugModule::onHandleChar(SkUnichar c) {
    switch (c) {
        case ' ': fState = kNext_State; break;
        case 'a': fIndex = (fIndex + 1) % (fDebugCanvas->getSize() - 1); break;
        case 's': fIndex = fIndex <= 0 ? fDebugCanvas->getSize() - 1 : fIndex - 1; break;
        default: break;
    }

    return true;
}
