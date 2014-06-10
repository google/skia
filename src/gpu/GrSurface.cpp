/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSurface.h"

#include "SkBitmap.h"
#include "SkGr.h"
#include "SkImageEncoder.h"
#include <stdio.h>

SkImageInfo GrSurface::info() const {
    SkImageInfo info;
    if (!GrPixelConfig2ColorType(this->config(), &info.fColorType)) {
        sk_throw();
    }
    info.fWidth = this->width();
    info.fHeight = this->height();
    info.fAlphaType = kPremul_SkAlphaType;
    return info;
}

bool GrSurface::savePixels(const char* filename) {
    SkBitmap bm;
    if (!bm.allocPixels(SkImageInfo::MakeN32Premul(this->width(),
                                                   this->height()))) {
        return false;
    }

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
