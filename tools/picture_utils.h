/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef picture_utils_DEFINED
#define picture_utils_DEFINED
#include "SkTypes.h"

class SkBitmap;
class SkFILEStream;
class SkString;
class SkPicture;

namespace sk_tools {
    // Creates a posix style filepath by concatenating name onto dir with a
    // forward slash into path.
    void make_filepath(SkString* path, const char* dir, const SkString& name);

    // Prepares the bitmap so that it can be written.
    //
    // Specifically, it configures the bitmap, allocates pixels and then
    // erases the pixels to transparent black.
    void setup_bitmap(SkBitmap* bitmap, int width, int height);
}

#endif  // picture_utils_DEFINED
