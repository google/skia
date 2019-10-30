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
#include "include/private/SkTDArray.h"

class SkSurface;

/**
 * Runs an SkPicture as a benchmark by repeatedly drawing it scaled inside a device clip.
 */
class SKPBench : public Benchmark {
public:
    SKPBench(const char* name, const SkPicture*, const SkIRect& devClip, SkScalar scale,
             bool useMultiPictureDraw, bool doLooping);
    ~SKPBench() override;

    int calculateLoops(int defaultLoops) const override {
        return fDoLooping ? defaultLoops : 1;
    }

    void getGpuStats(SkCanvas*, SkTArray<SkString>* keys, SkTArray<double>* values) override;

protected:
    const char* onGetName() override;
    const char* onGetUniqueName() override;
    void onPerCanvasPreDraw(SkCanvas*) override;
    void onPerCanvasPostDraw(SkCanvas*) override;
    bool isSuitableFor(Backend backend) override;
    void onDraw(int loops, SkCanvas* canvas) override;
    SkIPoint onGetSize() override;

    virtual void drawMPDPicture();
    virtual void drawPicture();

    const SkPicture* picture() const { return fPic.get(); }
    const SkTArray<sk_sp<SkSurface>>& surfaces() const { return fSurfaces; }
    const SkTDArray<SkIRect>& tileRects() const { return fTileRects; }

private:
    sk_sp<const SkPicture> fPic;
    const SkIRect fClip;
    const SkScalar fScale;
    SkString fName;
    SkString fUniqueName;

    const bool fUseMultiPictureDraw;
    SkTArray<sk_sp<SkSurface>> fSurfaces;   // for MultiPictureDraw
    SkTDArray<SkIRect> fTileRects;     // for MultiPictureDraw

    const bool fDoLooping;

    typedef Benchmark INHERITED;
};

#endif
