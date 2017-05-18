/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef picture_utils_DEFINED
#define picture_utils_DEFINED

#include "SkBitmap.h"

class SkData;
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
     */
    void replace_char(SkString* str, const char oldChar, const char newChar);

    // Returns true if the string ends with %
    bool is_percentage(const char* const string);

    // Prepares the bitmap so that it can be written.
    //
    // Specifically, it configures the bitmap, allocates pixels and then
    // erases the pixels to transparent black.
    void setup_bitmap(SkBitmap* bitmap, int width, int height);

    /**
     * Write a bitmap file to disk.
     *
     * @param bm the bitmap to record
     * @param dirPath directory within which to write the image file
     * @param subdirOrNull subdirectory within dirPath, or nullptr to just write into dirPath
     * @param baseName last part of the filename
     *
     * @return true if written out successfully
     */
    bool write_bitmap_to_disk(const SkBitmap& bm, const SkString& dirPath,
                              const char *subdirOrNull, const SkString& baseName);

    // Return raw unpremultiplied RGBA bytes, suitable for storing in a PNG. The output
    // colors are assumed to be sRGB values. This is only guaranteed to work for the
    // cases that are currently emitted by tools:
    // Linear premul 8888, sRGB premul 8888, Linear premul F16
    sk_sp<SkData> encode_bitmap_for_png(SkBitmap bitmap);

} // namespace sk_tools

#endif  // picture_utils_DEFINED
