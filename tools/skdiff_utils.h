/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skdiff_utils_DEFINED
#define skdiff_utils_DEFINED

#include "skdiff.h"

class SkBitmap;
class SkData;
class SkString;

/** Returns true if the two buffers passed in are both non-nullptr,
 *  have the same length, and contain exactly the same byte values.
 */
bool are_buffers_equal(SkData* skdata1, SkData* skdata2);

/** Reads the file at the given path and returns its complete contents as an
 *  SkData object (or returns nullptr on error).
 */
SkData* read_file(const char* file_path);

/** Decodes the fileBits into the resource.fBitmap. Returns false on failure. */
bool get_bitmap(SkData* fileBits, DiffResource& resource, bool sizeOnly);

/** Writes the bitmap as a PNG to the path specified. */
bool write_bitmap(const SkString& path, const SkBitmap& bitmap);

/** Given an image filename, returns the name of the file containing
 *  the associated difference image.
 */
SkString filename_to_diff_filename(const SkString& filename);

/** Given an image filename, returns the name of the file containing
 *  the "white" difference image.
 */
SkString filename_to_white_filename(const SkString& filename);

/** Calls compute_diff and handles the difference and white diff resources.
 *  If !outputDir.isEmpty(), writes out difference and white images.
 */
void create_and_write_diff_image(DiffRecord* drp,
                                 DiffMetricProc dmp,
                                 const int colorThreshold,
                                 const SkString& outputDir,
                                 const SkString& filename);

#endif
