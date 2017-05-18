/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKPAnimationBench_DEFINED
#define SKPAnimationBench_DEFINED

#include "SKPBench.h"
#include "Timer.h"

/**
 * Runs an SkPicture as a benchmark by repeatedly drawing it, first centering the picture and
 * for each step it concats the passed in matrix
 */
class SKPAnimationBench : public SKPBench {
public:
    class Animation : public SkRefCnt {
    public:
        virtual const char* getTag() = 0;
        virtual void preConcatFrameMatrix(double animationTimeMs, const SkIRect& devBounds,
                                          SkMatrix* drawMatrix) = 0;
        virtual ~Animation() {}
    };

    SKPAnimationBench(const char* name, const SkPicture*, const SkIRect& devClip, Animation*,
                      bool doLooping);

    static Animation* CreateZoomAnimation(SkScalar zoomMax, double zoomPeriodMs);

protected:
    const char* onGetUniqueName() override;
    void onPerCanvasPreDraw(SkCanvas* canvas) override;

    void drawMPDPicture() override {
        SkFAIL("MPD not supported\n");
    }
    void drawPicture() override;

private:
    sk_sp<Animation> fAnimation;
    WallTimer        fAnimationTimer;
    SkString         fUniqueName;
    SkIRect          fDevBounds;

    typedef SKPBench INHERITED;
};

#endif
