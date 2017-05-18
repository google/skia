/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageEncoder_DEFINED
#define SkImageEncoder_DEFINED

#include "SkBitmap.h"
#include "SkEncodedImageFormat.h"
#include "SkStream.h"

/**
 * Encode SkPixmap in the given binary image format.
 *
 * @param  dst     results are written to this stream.
 * @param  src     source pixels.
 * @param  format  image format, not all formats are supported.
 * @param  quality range from 0-100, this is supported by jpeg and webp.
 *                 higher values correspond to improved visual quality, but less compression.
 *
 * @return false iff input is bad or format is unsupported.
 *
 * Will always return false if Skia is compiled without image
 * encoders.
 *
 * Note that webp encodes will use webp lossy compression.
 *
 * For examples of encoding an image to a file or to a block of memory,
 * see tools/sk_tool_utils.h.
 */
SK_API bool SkEncodeImage(SkWStream* dst, const SkPixmap& src,
                          SkEncodedImageFormat format, int quality);
/**
 * The following helper function wraps SkEncodeImage().
 */
inline bool SkEncodeImage(SkWStream* dst, const SkBitmap& src, SkEncodedImageFormat f, int q) {
    SkPixmap pixmap;
    return src.peekPixels(&pixmap) && SkEncodeImage(dst, pixmap, f, q);
}

#endif  // SkImageEncoder_DEFINED
