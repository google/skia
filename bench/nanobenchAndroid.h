/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef nanobenchAndroid_DEFINED
#define nanobenchAndroid_DEFINED

#include "SkAndroidSDKCanvas.h"
#include "SkHwuiRenderer.h"

#include "nanobench.h"

struct HWUITarget : public Target {
    explicit HWUITarget(const Config& c, Benchmark* bench);

    SkHwuiRenderer renderer;
    SkAndroidSDKCanvas fc;

    void setup() override;
    SkCanvas* beginTiming(SkCanvas* canvas) override;
    void endTiming() override;
    void fence() override;
    bool needsFrameTiming(int* frameLag) const override;

    bool init(SkImageInfo info, Benchmark* bench) override;
    bool capturePixels(SkBitmap* bmp) override;
};

#endif  // nanobenchAndroid_DEFINED
