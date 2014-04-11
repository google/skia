/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkRecording.h"

// Minimally exercise the public SkRecording API.

DEF_TEST(RecordingTest, r) {
    EXPERIMENTAL::SkRecording* recording = EXPERIMENTAL::SkRecording::Create(1920, 1080);

    // Some very exciting commands here.
    recording->canvas()->clipRect(SkRect::MakeWH(320, 240));

    SkAutoTDelete<const EXPERIMENTAL::SkPlayback> playback(
        EXPERIMENTAL::SkRecording::Delete(recording));

    SkCanvas target;
    playback->draw(&target);
}
