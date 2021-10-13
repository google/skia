/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef FillPathFlags_DEFINED
#define FillPathFlags_DEFINED

#include "include/gpu/GrTypes.h"

namespace skgpu::v1 {

// We send these flags to the internal path filling Ops to control how a path gets rendered.
enum class FillPathFlags {
    kNone        = 0,
    kStencilOnly = (1 << 0),
    kWireframe   = (1 << 1)
};

GR_MAKE_BITFIELD_CLASS_OPS(FillPathFlags)

} // namespace skgpu::v1

#endif // FillPathFlags_DEFINED
