/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTTypes_DEFINED
#define GrNXTTypes_DEFINED

#include "dawn/dawncpp.h"

struct GrNXTImageInfo {
    dawn::Texture       fTexture;
    dawn::TextureFormat fFormat;
    uint32_t            fLevelCount;
    GrNXTImageInfo() : fTexture(nullptr), fFormat(), fLevelCount(0) {
    }
    GrNXTImageInfo(const GrNXTImageInfo& other)
        : fTexture(other.fTexture.Clone())
        , fFormat(other.fFormat)
        , fLevelCount(other.fLevelCount) {
    }
    GrNXTImageInfo& operator=(const GrNXTImageInfo& other) {
        fTexture = other.fTexture.Clone();
        fFormat = other.fFormat;
        fLevelCount = other.fLevelCount;
        return *this;
    }
};

#endif
