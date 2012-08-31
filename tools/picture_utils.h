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
    // since PNG insists on unpremultiplying our alpha, we take no precision
    // chances and force all pixels to be 100% opaque, otherwise on compare we
    // may not get a perfect match.
    //
    // This expects a bitmap with a config type of 8888 and for the pixels to
    // not be on the GPU.
    void force_all_opaque(const SkBitmap& bitmap);

    // Creates a posix style filepath by concatenating name onto dir with a
    // forward slash into path.
    void make_filepath(SkString* path, const SkString&, const SkString& name);

    // Returns the last part of the path (file name or leaf directory name)
    //
    // This basically just looks for a foward slash or backslash (windows
    // only).
    void get_basename(SkString* basename, const SkString& path);

    // Returns true if the string ends with %
    bool is_percentage(const char* const string);

    // Prints to STDOUT so that test results can be easily seperated from the
    // error stream. Note, that this still prints to the same stream as SkDebugf
    // on Andoid.
    void print_msg(const char msg[]);

    // Prepares the bitmap so that it can be written.
    //
    // Specifically, it configures the bitmap, allocates pixels and then
    // erases the pixels to transparent black.
    void setup_bitmap(SkBitmap* bitmap, int width, int height);
}

#endif  // picture_utils_DEFINED
