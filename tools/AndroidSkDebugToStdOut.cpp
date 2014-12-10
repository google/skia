/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Need to include SkTypes before checking SK_BUILD_FOR_ANDROID, so it will be
// set in the Android framework build.
#include "SkTypes.h"
#ifdef SK_BUILD_FOR_ANDROID
extern bool gSkDebugToStdOut;

// Use a static initializer to set gSkDebugToStdOut to true, sending SkDebugf
// to stdout.
class SendToStdOut {
public:
    SendToStdOut() {
        gSkDebugToStdOut = true;
    }
};

static SendToStdOut gSendToStdOut;
#endif // SK_BUILD_FOR_ANDROID
