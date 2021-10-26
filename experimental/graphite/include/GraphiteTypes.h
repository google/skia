/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GraphiteTypes_DEFINED
#define skgpu_GraphiteTypes_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkVx.h"

namespace skgpu {

/**
 * Actually submit work to the GPU and track its completion
 */
enum class SyncToCpu : bool {
    kYes = true,
    kNo = false
};

/**
 * Possible 3D APIs that may be used by Graphite.
 */
enum class BackendApi : unsigned {
    kMetal,
    kMock,
};

/**
 * Is the texture mipmapped or not
 */
enum class Mipmapped: bool {
    kNo = false,
    kYes = true,
};

/**
 * Is the data protected on the GPU or not.
 */
enum class Protected : bool {
    kNo = false,
    kYes = true,
};

} // namespace skgpu

#endif // skgpu_GraphiteTypes_DEFINED
