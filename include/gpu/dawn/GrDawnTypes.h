/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnTypes_DEFINED
#define GrDawnTypes_DEFINED

#include "dawn/dawncpp.h"

struct GrDawnImageInfo {
    dawn::Texture       fTexture;
    dawn::TextureFormat fFormat;
    uint32_t            fLevelCount;
    GrDawnImageInfo() : fTexture(nullptr), fFormat(), fLevelCount(0) {
    }
    GrDawnImageInfo(const GrDawnImageInfo& other)
        : fTexture(other.fTexture.Clone())
        , fFormat(other.fFormat)
        , fLevelCount(other.fLevelCount) {
    }
    GrDawnImageInfo& operator=(const GrDawnImageInfo& other) {
        fTexture = other.fTexture.Clone();
        fFormat = other.fFormat;
        fLevelCount = other.fLevelCount;
        return *this;
    }
};

#endif
