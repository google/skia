/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "RecordingBench.h"
#include "SkBBHFactory.h"
#include "SkLiteDL.h"
#include "SkLiteRecorder.h"
#include "SkPictureRecorder.h"

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

#include "SkPipe.h"
#include "SkStream.h"

PipingBench::PipingBench(const char* name, const SkPicture* pic) : INHERITED(name, pic) {
    fName.prepend("pipe_");
}

void PipingBench::onDraw(int loops, SkCanvas*) {
    SkDynamicMemoryWStream stream;
    SkPipeSerializer serializer;

    while (loops --> 0) {
        fSrc->playback(serializer.beginWrite(fSrc->cullRect(), &stream));
        serializer.endWrite();
        stream.reset();
    }
}
