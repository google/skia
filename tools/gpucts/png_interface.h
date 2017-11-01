/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef png_interface_DEFINED
#define png_interface_DEFINED

#include <cstdint>
#include <vector>

namespace png_interface {

struct Image {
    int width;
    int height;
    std::vector<uint32_t> pixels;
};

/* These functions abstract out reading and writing PNG files.  */

bool WritePngRgba8888ToFile(int width, int height, const uint32_t* pixels, const char* path);

// Returns nullptr on error.
Image ReadPngRgba8888FromFile(const char* path);

}
#endif  // png_interface_DEFINED
