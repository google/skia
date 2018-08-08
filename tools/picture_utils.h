/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef picture_utils_DEFINED
#define picture_utils_DEFINED

class SkBitmap;
class SkString;

namespace sk_tools {
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

} // namespace sk_tools

#endif  // picture_utils_DEFINED
