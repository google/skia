/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkShader.h"
#include "Test.h"

// Test that attempting to create a picture shader with a NULL picture or
// empty picture returns a shader that draws nothing.
DEF_TEST(PictureShader_empty, reporter) {
    SkPaint paint;

    SkBitmap bitmap;
    bitmap.allocN32Pixels(1,1);

    SkCanvas canvas(bitmap);
    canvas.clear(SK_ColorGREEN);

    SkShader* shader = SkShader::CreatePictureShader(
            NULL, SkShader::kClamp_TileMode, SkShader::kClamp_TileMode, NULL, NULL);
    paint.setShader(shader)->unref();

    canvas.drawRect(SkRect::MakeWH(1,1), paint);
    REPORTER_ASSERT(reporter, *bitmap.getAddr32(0,0) == SK_ColorGREEN);


    SkPictureRecorder factory;
    factory.beginRecording(0, 0, NULL, 0);
    SkAutoTUnref<SkPicture> picture(factory.endRecording());
    shader = SkShader::CreatePictureShader(
            picture.get(), SkShader::kClamp_TileMode, SkShader::kClamp_TileMode, NULL, NULL);
    paint.setShader(shader)->unref();

    canvas.drawRect(SkRect::MakeWH(1,1), paint);
    REPORTER_ASSERT(reporter, *bitmap.getAddr32(0,0) == SK_ColorGREEN);
}
