/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTTypes_DEFINED
#define GrNXTTypes_DEFINED

#include "GrTypes.h"
#include "dawn/dawn.h"
#include "dawn/dawncpp.h"

struct GrNXTImageInfo {
    nxtTexture          fTexture;
    dawn::TextureFormat fFormat;
    uint32_t            fLevelCount;
};

#endif
