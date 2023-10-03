/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKPBench_DEFINED
#define SKPBench_DEFINED

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPicture.h"
#include "include/private/base/SkTDArray.h"

class SkSurface;

/**
 * Runs an SkPicture as a benchmark by repeatedly drawing it scaled inside a device clip.
 */
class SKPBench : public Benchmark {
public:
    SKPBench(const char* name, const SkPicture*, const SkIRect& devClip, SkScalar scale,
             bool doLooping);
    ~SKPBench() override;

    bool shouldLoop() const override {
        return fDoLooping;
    }

    void getGpuStats(SkCanvas*,
                     skia_private::TArray<SkString>* keys,
                     skia_private::TArray<double>* values) override;
    bool getDMSAAStats(GrRecordingContext*) override;

protected:
    const char* onGetName() override;
    const char* onGetUniqueName() override;
    void onPerCanvasPreDraw(SkCanvas*) override;
    void onPerCanvasPostDraw(SkCanvas*) override;
    bool isSuitableFor(Backend backend) override;
    void onDraw(int loops, SkCanvas* canvas) override;
    SkISize onGetSize() override;

    virtual void drawMPDPicture();
    virtual void drawPicture();

    const SkPicture* picture() const { return fPic.get(); }
    const skia_private::TArray<sk_sp<SkSurface>>& surfaces() const { return fSurfaces; }
    const SkTDArray<SkIRect>& tileRects() const { return fTileRects; }

private:
    sk_sp<const SkPicture> fPic;
    const SkIRect fClip;
    const SkScalar fScale;
    SkString fName;
    SkString fUniqueName;

    skia_private::TArray<sk_sp<SkSurface>> fSurfaces;   // for MultiPictureDraw
    SkTDArray<SkIRect> fTileRects;     // for MultiPictureDraw

    const bool fDoLooping;

    using INHERITED = Benchmark;
};

#endif
