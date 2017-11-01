/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef png_interface_DEFINED
#define png_interface_DEFINED

#include "gm_knowledge.h"

#include <cstdint>
#include <vector>

/* These functions abstract out reading and writing PNG files.  */

bool WritePngRgba8888ToFile(GMK_ImageData data, const char* path);

// Returns nullptr on error.
GMK_ImageData ReadPngRgba8888FromFile(const char* path, std::vector<uint32_t>* pixels);

#endif  // png_interface_DEFINED
