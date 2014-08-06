/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkShader.h"
#include "Test.h"

// Test that attempting to create a picture shader with a NULL picture or
// empty picture returns NULL.
DEF_TEST(PictureShader_empty, reporter) {
    SkShader* shader = SkShader::CreatePictureShader(NULL,
            SkShader::kClamp_TileMode, SkShader::kClamp_TileMode, NULL, NULL);
    REPORTER_ASSERT(reporter, NULL == shader);

    SkPictureRecorder factory;
    factory.beginRecording(0, 0, NULL, 0);
    SkAutoTUnref<SkPicture> picture(factory.endRecording());
    shader = SkShader::CreatePictureShader(picture.get(),
            SkShader::kClamp_TileMode, SkShader::kClamp_TileMode, NULL, NULL);
    REPORTER_ASSERT(reporter, NULL == shader);
}
