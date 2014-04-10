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
class SkPicture;
class SkString;

namespace sk_tools {
    // since PNG insists on unpremultiplying our alpha, we take no precision
    // chances and force all pixels to be 100% opaque, otherwise on compare we
    // may not get a perfect match.
    //
    // This expects a bitmap with a config type of 8888 and for the pixels to
    // not be on the GPU.
    void force_all_opaque(const SkBitmap& bitmap);

    /**
     * Replaces all instances of oldChar with newChar in str.
     *
     * TODO: This function appears here and in skimage_main.cpp ;
     * we should add the implementation to src/core/SkString.cpp, write tests for it,
     * and remove it from elsewhere.
     */
    void replace_char(SkString* str, const char oldChar, const char newChar);

    // Creates a posix style filepath by concatenating name onto dir with a
    // forward slash into path.
    // TODO(epoger): delete in favor of SkOSPath::SkPathJoin()?
    void make_filepath(SkString* path, const SkString&, const SkString& name);

    // Returns the last part of the path (file name or leaf directory name)
    //
    // This basically just looks for a foward slash or backslash (windows
    // only).
    // TODO(epoger): delete in favor of SkOSPath::SkBasename()?
    void get_basename(SkString* basename, const SkString& path);

    // Returns true if the string ends with %
    bool is_percentage(const char* const string);

    // Prepares the bitmap so that it can be written.
    //
    // Specifically, it configures the bitmap, allocates pixels and then
    // erases the pixels to transparent black.
    void setup_bitmap(SkBitmap* bitmap, int width, int height);
}

#endif  // picture_utils_DEFINED
