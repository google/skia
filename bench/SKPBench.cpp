/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SKPBench.h"
#include "SkCommandLineFlags.h"

DEFINE_int32(benchTile, 256, "Tile dimension used for SKP playback.");

SKPBench::SKPBench(const char* name, const SkPicture* pic, const SkIRect& clip, SkScalar scale)
    : fPic(SkRef(pic))
    , fClip(clip)
    , fScale(scale)
    , fName(name) {
    fUniqueName.printf("%s_%.2g", name, scale);  // Scale makes this unqiue for skiaperf.com traces.
}

const char* SKPBench::onGetName() {
    return fName.c_str();
}

const char* SKPBench::onGetUniqueName() {
    return fUniqueName.c_str();
}

bool SKPBench::isSuitableFor(Backend backend) {
    return backend != kNonRendering_Backend;
}

SkIPoint SKPBench::onGetSize() {
    return SkIPoint::Make(fClip.width(), fClip.height());
}

void SKPBench::onDraw(const int loops, SkCanvas* canvas) {
    SkIRect bounds;
    SkAssertResult(canvas->getClipDeviceBounds(&bounds));

    SkAutoCanvasRestore overall(canvas, true/*save now*/);
    canvas->scale(fScale, fScale);

    for (int i = 0; i < loops; i++) {
        for (int y = bounds.fTop; y < bounds.fBottom; y += FLAGS_benchTile) {
            for (int x = bounds.fLeft; x < bounds.fRight; x += FLAGS_benchTile) {
                SkAutoCanvasRestore perTile(canvas, true/*save now*/);
                canvas->clipRect(SkRect::Make(
                            SkIRect::MakeXYWH(x, y, FLAGS_benchTile, FLAGS_benchTile)));
                fPic->playback(canvas);
            }
        }
        canvas->flush();
    }
}
