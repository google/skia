/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef EncodeUtils_DEFINED
#define EncodeUtils_DEFINED

class SkBitmap;
class SkPixmap;
class SkString;

namespace ToolUtils {

// Encodes the bitmap into a data:/image/png;base64,... url suitable to view in a browser after
// printing to a log. If false is returned, dst holds an error message instead of a URI.
bool BitmapToBase64DataURI(const SkBitmap& bitmap, SkString* dst);

bool EncodeImageToPngFile(const char* path, const SkBitmap& src);
bool EncodeImageToPngFile(const char* path, const SkPixmap& src);

}  // namespace ToolUtils

#endif
