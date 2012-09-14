/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkPicture.h"
#include "SkStream.h"

#ifdef SK_DEBUG
// Ensure that deleting SkPicturePlayback does not assert. Asserts only fire in debug mode, so only
// run in debug mode.
static void test_deleting_empty_playback() {
    SkPicture picture;
    // Creates an SkPictureRecord
    picture.beginRecording(0, 0);
    // Turns that into an SkPicturePlayback
    picture.endRecording();
    // Deletes the old SkPicturePlayback, and creates a new SkPictureRecord
    picture.beginRecording(0, 0);
}

// Ensure that serializing an empty picture does not assert. Likewise only runs in debug mode.
static void test_serializing_empty_picture() {
    SkPicture picture;
    picture.beginRecording(0, 0);
    picture.endRecording();
    SkDynamicMemoryWStream stream;
    picture.serialize(&stream);
}
#endif

static void TestPicture(skiatest::Reporter* reporter) {
#ifdef SK_DEBUG
    test_deleting_empty_playback();
    test_serializing_empty_picture();
#endif
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Picture", PictureTestClass, TestPicture)
