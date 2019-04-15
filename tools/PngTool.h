// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef PngTool_DEFINED
#define PngTool_DEFINED

#include <memory>
#include <cstddef>

namespace PngTool {

enum class ColorType {
    kGray      = 0,
    kRGB       = 2,
    kGrayAlpha = 4,
    kRGBA      = 6,
};

struct Pixmap {
    // rowbytes determined by width, bit depth, and color_type (png_get_rowbytes)
    // pixels are contiguous: their size is (height * rowbytes)
    unsigned char* pixels = nullptr;
    unsigned width = 0;
    unsigned height = 0;
    int bitDepth = 0;    // per channel
    ColorType colorType = (ColorType)(-1);
};

struct WriteOptions {
    // author, description, iccNama, iccData are optional
    const char* author = nullptr;
    const char* description = nullptr;
    const char* iccName = nullptr;
    const void* iccData = nullptr;
    size_t iccDataLength = 0;
    bool fast = false;  // compression tradeoff
};

bool Write(const char* path,
           const Pixmap& pixmap,
           const WriteOptions& opts = WriteOptions());

// caller must free Pixmap::pixels.
Pixmap Read(const char* path, void* (*mallocFn)(size_t));

}  // namespace PngTool
#endif  // PngTool_DEFINED
