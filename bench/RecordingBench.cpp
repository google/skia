/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "RecordingBench.h"

#include "SkBBHFactory.h"
#include "SkPictureRecorder.h"

RecordingBench::RecordingBench(const char* name, const SkPicture* pic, bool useBBH)
    : fSrc(SkRef(pic))
    , fName(name)
    , fUseBBH(useBBH) {}

const char* RecordingBench::onGetName() {
    return fName.c_str();
}

bool RecordingBench::isSuitableFor(Backend backend) {
    return backend == kNonRendering_Backend;
}

SkIPoint RecordingBench::onGetSize() {
    return SkIPoint::Make(SkScalarCeilToInt(fSrc->cullRect().width()),
                          SkScalarCeilToInt(fSrc->cullRect().height()));
}

void RecordingBench::onDraw(const int loops, SkCanvas*) {
    SkRTreeFactory factory;
    const SkScalar w = fSrc->cullRect().width(),
                   h = fSrc->cullRect().height();

    uint32_t flags = SkPictureRecorder::kComputeSaveLayerInfo_RecordFlag
                   | SkPictureRecorder::kPlaybackDrawPicture_RecordFlag;
    for (int i = 0; i < loops; i++) {
        SkPictureRecorder recorder;
        fSrc->playback(recorder.beginRecording(w, h, fUseBBH ? &factory : NULL, flags));
        SkSafeUnref(recorder.endRecording());
    }
}
