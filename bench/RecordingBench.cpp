/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/RecordingBench.h"
#include "include/core/SkBBHFactory.h"
#include "include/core/SkPictureRecorder.h"
#include "src/core/SkLiteDL.h"
#include "src/core/SkLiteRecorder.h"

PictureCentricBench::PictureCentricBench(const char* name, const SkPicture* pic) : fName(name) {
    // Flatten the source picture in case it's trivially nested (useless for timing).
    SkPictureRecorder rec;
    pic->playback(rec.beginRecording(pic->cullRect(), nullptr,
                                     SkPictureRecorder::kPlaybackDrawPicture_RecordFlag));
    fSrc = rec.finishRecordingAsPicture();
}

const char* PictureCentricBench::onGetName() {
    return fName.c_str();
}

bool PictureCentricBench::isSuitableFor(Backend backend) {
    return backend == kNonRendering_Backend;
}

SkIPoint PictureCentricBench::onGetSize() {
    return SkIPoint::Make(SkScalarCeilToInt(fSrc->cullRect().width()),
                          SkScalarCeilToInt(fSrc->cullRect().height()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

RecordingBench::RecordingBench(const char* name, const SkPicture* pic, bool useBBH, bool lite)
    : INHERITED(name, pic)
    , fUseBBH(useBBH)
{
    // If we're recording into an SkLiteDL, also record _from_ one.
    if (lite) {
        fDL.reset(new SkLiteDL());
        SkLiteRecorder r;
        r.reset(fDL.get(), fSrc->cullRect().roundOut());
        fSrc->playback(&r);
    }
}

void RecordingBench::onDraw(int loops, SkCanvas*) {
    if (fDL) {
        SkLiteRecorder rec;
        while (loops --> 0) {
            SkLiteDL dl;
            rec.reset(&dl, fSrc->cullRect().roundOut());
            fDL->draw(&rec);
        }

    } else {
        SkRTreeFactory factory;
        SkPictureRecorder recorder;
        while (loops --> 0) {
            fSrc->playback(recorder.beginRecording(fSrc->cullRect(), fUseBBH ? &factory : nullptr));
            (void)recorder.finishRecordingAsPicture();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "include/core/SkSerialProcs.h"

DeserializePictureBench::DeserializePictureBench(const char* name, sk_sp<SkData> data)
    : fName(name)
    , fEncodedPicture(std::move(data))
{}

const char* DeserializePictureBench::onGetName() {
    return fName.c_str();
}

bool DeserializePictureBench::isSuitableFor(Backend backend) {
    return backend == kNonRendering_Backend;
}

SkIPoint DeserializePictureBench::onGetSize() {
    return SkIPoint::Make(128, 128);
}

void DeserializePictureBench::onDraw(int loops, SkCanvas*) {
    for (int i = 0; i < loops; ++i) {
        SkPicture::MakeFromData(fEncodedPicture.get());
    }
}
