/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGradientShader.h"
#include "SkPictureRecorder.h"
#include "gm.h"

DEF_SIMPLE_GM(bug6643, canvas, 200, 200) {
    SkColor colors[] = { SK_ColorTRANSPARENT, SK_ColorGREEN, SK_ColorTRANSPARENT };

    SkPaint p;
    p.setAntiAlias(true);
    p.setShader(SkGradientShader::MakeSweep(100, 100, colors, nullptr, SK_ARRAY_COUNT(colors),
                                            SkGradientShader::kInterpolateColorsInPremul_Flag,
                                            nullptr));

    SkPictureRecorder recorder;
    recorder.beginRecording(200, 200)->drawPaint(p);

    p.setShader(recorder.finishRecordingAsPicture()->makeShader(
                                            SkTileMode::kRepeat, SkTileMode::kRepeat));
    canvas->drawColor(SK_ColorWHITE);
    canvas->drawPaint(p);
}
