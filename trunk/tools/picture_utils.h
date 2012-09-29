/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef picture_utils_DEFINED
#define picture_utils_DEFINED

#include "SkTypes.h"
#include "SkSize.h"

template <typename T> class SkAutoTUnref;
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

    // Prepares the bitmap so that it can be written.
    //
    // Specifically, it configures the bitmap, allocates pixels and then
    // erases the pixels to transparent black.
    void setup_bitmap(SkBitmap* bitmap, int width, int height);

    // Determines whether the given dimensions are too large and suggests a new
    // size.
    bool area_too_big(int w, int h, SkISize* newSize);

    // Determines whether the given SkPicture is too large and, if so, replaces
    // it with a new, scaled-down SkPicture.
    void resize_if_needed(SkAutoTUnref<SkPicture>* aur);
}

#endif  // picture_utils_DEFINED
