/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTTypes_DEFINED
#define GrNXTTypes_DEFINED

#include "GrTypes.h"
#include "nxt/nxt.h"
#include "nxt/nxtcpp.h"

struct GrNXTImageInfo {
    nxtTexture          fTexture;
    nxt::TextureFormat  fFormat;
    uint32_t            fLevelCount;
};

#endif
