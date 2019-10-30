/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "src/shaders/SkPictureShader.h"
#include "tests/Test.h"

// Test that the SkPictureShader cache is purged on shader deletion.
DEF_TEST(PictureShader_caching, reporter) {
    auto makePicture = [] () {
        SkPictureRecorder recorder;
        recorder.beginRecording(100, 100)->drawColor(SK_ColorGREEN);
        return recorder.finishRecordingAsPicture();
    };

    sk_sp<SkPicture> picture = makePicture();
    REPORTER_ASSERT(reporter, picture->unique());

    sk_sp<SkSurface> surface = SkSurface::MakeRasterN32Premul(100, 100);

    {
        SkPaint paint;
        paint.setShader(picture->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat));
        surface->getCanvas()->drawPaint(paint);

        // We should have about 3 refs by now: local + shader + shader cache.
        REPORTER_ASSERT(reporter, !picture->unique());
    }

    // Draw another picture shader to have a chance to purge.
    {
        SkPaint paint;
        paint.setShader(makePicture()->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat));
        surface->getCanvas()->drawPaint(paint);

    }

    // All but the local ref should be gone now.
    REPORTER_ASSERT(reporter, picture->unique());
}
