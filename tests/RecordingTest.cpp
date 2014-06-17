/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "../include/record/SkRecording.h"

// Minimally exercise the public SkRecording API.

DEF_TEST(RecordingTest, r) {
    EXPERIMENTAL::SkRecording recording(1920, 1080);

    // Some very exciting commands here.
    recording.canvas()->clipRect(SkRect::MakeWH(320, 240));

    SkAutoTDelete<const EXPERIMENTAL::SkPlayback> playback(recording.releasePlayback());

    SkCanvas target;
    playback->draw(&target);

    // Here's another recording we never call releasePlayback().
    // However pointless, this should be safe.
    EXPERIMENTAL::SkRecording pointless(1920, 1080);
    pointless.canvas()->clipRect(SkRect::MakeWH(320, 240));
}
