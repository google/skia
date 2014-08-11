/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../../include/record/SkRecording.h"

#include "SkRecord.h"
#include "SkRecordOpts.h"
#include "SkRecordDraw.h"
#include "SkRecorder.h"

namespace EXPERIMENTAL {

SkPlayback::SkPlayback(const SkRecord* record) : fRecord(record) {}

SkPlayback::~SkPlayback() {}

void SkPlayback::draw(SkCanvas* canvas) const {
    SkASSERT(fRecord.get() != NULL);
    SkRecordDraw(*fRecord, canvas, NULL/*bbh*/, NULL/*callback*/);
}

SkRecording::SkRecording(int width, int height)
    : fRecord(SkNEW(SkRecord))
    , fRecorder(SkNEW_ARGS(SkRecorder, (fRecord.get(), width, height)))
    {}

SkPlayback* SkRecording::releasePlayback() {
    SkASSERT(fRecorder->unique());
    fRecorder->forgetRecord();
    SkRecordOptimize(fRecord.get());
    return SkNEW_ARGS(SkPlayback, (fRecord.detach()));
}

SkRecording::~SkRecording() {}

SkCanvas* SkRecording::canvas() {
    return fRecord.get() ? fRecorder.get() : NULL;
}

}  // namespace EXPERIMENTAL
