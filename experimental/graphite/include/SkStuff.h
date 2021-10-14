/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStuff_DEFINED
#define SkStuff_DEFINED

#include "include/core/SkRefCnt.h"

struct SkImageInfo;
class SkSurface;

namespace skgpu {
    class Recorder;
}

// TODO: Should be in SkSurface.h
sk_sp<SkSurface> MakeGraphite(sk_sp<skgpu::Recorder>, const SkImageInfo&);

#endif // SkStuff_DEFINED
