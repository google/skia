/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSurface.h"

#include "SkBitmap.h"
#include "SkImageEncoder.h"
#include <stdio.h>

SK_DEFINE_INST_COUNT(GrSurface)

bool GrSurface::savePixels(const char* filename) {
    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, this->width(), this->height());
    bm.allocPixels();

    bool result = readPixels(0, 0, this->width(), this->height(), kSkia8888_GrPixelConfig,
                             bm.getPixels());
    if (!result) {
        SkDebugf("------ failed to read pixels for %s\n", filename);
        return false;
    }

    // remove any previous version of this file
    remove(filename);

    if (!SkImageEncoder::EncodeFile(filename, bm, SkImageEncoder::kPNG_Type, 100)) {
        SkDebugf("------ failed to encode %s\n", filename);
        remove(filename);   // remove any partial file
        return false;
    }

    return true;
}
