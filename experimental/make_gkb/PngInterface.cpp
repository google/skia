/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "PngInterface.h"

#include <cassert>
#include <cstring>

#include <png.h>

/* Call libpng directly rather than include Skia: this program might need to be
   run on the cloud: simpler dependencies might be advantageous.  */

bool WritePngRgba8888ToFile(int w, int h, const void* src, const char* path) {
    png_image image;
    memset(&image, 0, (sizeof(image)));
    image.version = PNG_IMAGE_VERSION;
    image.width = w;
    image.height = h;
    image.format = PNG_FORMAT_RGBA;
    return 0 != png_image_write_to_file(&image, path, 0, src, 4 * w, nullptr);
}

bool ReadPngRgba8888FromFile(const char* path, int* w, int* h, std::vector<unsigned char>* img) {
    assert(w);
    assert(h);
    assert(img);
    png_image image;
    memset(&image, 0, (sizeof(image)));
    image.version = PNG_IMAGE_VERSION;
    if (png_image_begin_read_from_file(&image, path) == 0) {
        return false;
    }
    image.format = PNG_FORMAT_RGBA;
    image.flags |= PNG_TRANSFORM_STRIP_16;
    *w = image.width;
    *h = image.height;
    size_t s = PNG_IMAGE_SIZE(image);
    assert(s == image.width * image.height * 4);
    if (img->size() != s) {
        img->resize(s);  // otherwise, overwrite.
    }
    return png_image_finish_read(&image, NULL, img->data(), 0, NULL) != 0;
}

