/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKPBench_DEFINED
#define SKPBench_DEFINED

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkPicture.h"

/**
 * Runs an SkPicture as a benchmark by repeatedly drawing it scaled inside a device clip.
 */
class SKPBench : public Benchmark {
public:
    SKPBench(const char* name, const SkPicture*, const SkIRect& devClip, SkScalar scale,
             bool useMultiPictureDraw);
    ~SKPBench() SK_OVERRIDE;

protected:
    const char* onGetName() SK_OVERRIDE;
    const char* onGetUniqueName() SK_OVERRIDE;
    void onPerCanvasPreDraw(SkCanvas*) SK_OVERRIDE;
    void onPerCanvasPostDraw(SkCanvas*) SK_OVERRIDE;
    bool isSuitableFor(Backend backend) SK_OVERRIDE;
    void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE;
    SkIPoint onGetSize() SK_OVERRIDE;

private:
    SkAutoTUnref<const SkPicture> fPic;
    const SkIRect fClip;
    const SkScalar fScale;
    SkString fName;
    SkString fUniqueName;

    const bool fUseMultiPictureDraw;
    SkTDArray<SkSurface*> fSurfaces;   // for MultiPictureDraw
    SkTDArray<SkIRect> fTileRects;     // for MultiPictureDraw

    typedef Benchmark INHERITED;
};

#endif
