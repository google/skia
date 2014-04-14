/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRecording.h"

#include "SkRecord.h"
#include "SkRecordCulling.h"
#include "SkRecordDraw.h"
#include "SkRecorder.h"

namespace EXPERIMENTAL {

SkPlayback::SkPlayback(const SkRecord* record) : fRecord(record) {}

SkPlayback::~SkPlayback() {
    SkDELETE(fRecord);
}

void SkPlayback::draw(SkCanvas* canvas) const {
    SkASSERT(fRecord != NULL);
    SkRecordDraw(*fRecord, canvas);
}

/*static*/ SkRecording* SkRecording::Create(int width, int height) {
    return SkNEW_ARGS(SkRecording, (width, height));
}

SkRecording::SkRecording(int width, int height) {
    SkRecord* record = SkNEW(SkRecord);
    fRecorder = SkNEW_ARGS(SkRecorder, (SkRecorder::kReadWrite_Mode, record, width, height));
    fRecord = record;
}

/*static*/ const SkPlayback* SkRecording::Delete(SkRecording* recording) {
    SkRecord* record = recording->fRecord;
    SkRecordAnnotateCullingPairs(record);
    SkDELETE(recording);
    return SkNEW_ARGS(SkPlayback, (record));
}

SkRecording::~SkRecording() {
    SkDELETE(fRecorder);
}

SkCanvas* SkRecording::canvas() {
    return fRecorder;
}

}  // namespace EXPERIMENTAL
