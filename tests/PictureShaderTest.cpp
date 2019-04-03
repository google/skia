/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkPictureShader.h"
#include "SkShader.h"
#include "SkSurface.h"
#include "Test.h"

#ifdef SK_SUPPORT_LEGACY_TILEMODE_ENUM
// Test that attempting to create a picture shader with a nullptr picture or
// empty picture returns a shader that draws nothing.
DEF_TEST(PictureShader_empty, reporter) {
    SkPaint paint;

    SkBitmap bitmap;
    bitmap.allocN32Pixels(1,1);

    SkCanvas canvas(bitmap);
    canvas.clear(SK_ColorGREEN);

    paint.setShader(SkShader::MakePictureShader(
            nullptr, SkShader::kClamp_TileMode, SkShader::kClamp_TileMode, nullptr, nullptr));

    canvas.drawRect(SkRect::MakeWH(1,1), paint);
    REPORTER_ASSERT(reporter, *bitmap.getAddr32(0,0) == SK_ColorGREEN);


    SkPictureRecorder factory;
    factory.beginRecording(0, 0, nullptr, 0);
    paint.setShader(SkShader::MakePictureShader(factory.finishRecordingAsPicture(),
                                                SkShader::kClamp_TileMode,
                                                SkShader::kClamp_TileMode, nullptr, nullptr));

    canvas.drawRect(SkRect::MakeWH(1,1), paint);
    REPORTER_ASSERT(reporter, *bitmap.getAddr32(0,0) == SK_ColorGREEN);
}
#endif

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
