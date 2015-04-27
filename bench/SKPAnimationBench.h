/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKPAnimationBench_DEFINED
#define SKPAnimationBench_DEFINED

#include "SKPBench.h"

/**
 * Runs an SkPicture as a benchmark by repeatedly drawing it, first centering the picture and
 * for each step it concats the passed in matrix
 */
class SKPAnimationBench : public SKPBench {
public:
    SKPAnimationBench(const char* name, const SkPicture*, const SkIRect& devClip,
                      SkMatrix viewMatrix, int steps);

protected:
    const char* onGetName() override;
    const char* onGetUniqueName() override;
    void onPerCanvasPreDraw(SkCanvas* canvas) override;

    void drawMPDPicture() override {
        SkFAIL("MPD not supported\n");
    }
    void drawPicture() override;

private:
    int fSteps;
    SkMatrix fAnimationMatrix;
    SkString fName;
    SkString fUniqueName;
    SkPoint fCenter;

    typedef SKPBench INHERITED;
};

#endif
