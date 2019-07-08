/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKPMultiFrameBench_DEFINED
#define SKPMultiFrameBench_DEFINED

#include <vector>

#include "bench/Benchmark.h"
#include "tools/timer/Timer.h"
#include "src/utils/SkMultiPictureDocument.h"

/**
 * Plays a Multi Frame SkPicture as a benchmark by advancing the frame index each draw.
 * Created by deserializing the multi frame skp format.
 */
class SKPMultiFrameBench : public Benchmark {
public:
    SKPMultiFrameBench(const SkString& name, const SkString& path, const SkIRect& clip);

protected:
    const char* onGetUniqueName() override;
    const char* onGetName() override { return "Multi Frame SKP"; }
    void onPerCanvasPreDraw(SkCanvas* canvas) override;
    void onDraw(int loops, SkCanvas*) override;

private:
    int fFrameIndex = 0;
    std::vector<SkDocumentPage> fFrames;
    WallTimer        fAnimationTimer;
    SkString         fUniqueName;
    SkIRect          fDevBounds;
};

#endif
