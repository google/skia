/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef PngInterface_DEFINED
#define PngInterface_DEFINED

#include <vector>

/* These functions abstract out reading and writing PNG files.  */

bool WritePngRgba8888ToFile(int w, int h, const void* src, const char* path);

bool ReadPngRgba8888FromFile(const char* path, int* w, int* h, std::vector<unsigned char>* dst);

#endif  // PngInterface_DEFINED
