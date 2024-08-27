/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef FillPathFlags_DEFINED
#define FillPathFlags_DEFINED

#include "include/private/base/SkMacros.h"

namespace skgpu::ganesh {

// We send these flags to the internal path filling Ops to control how a path gets rendered.
enum class FillPathFlags {
    kNone        = 0,
    kStencilOnly = (1 << 0),
    kWireframe   = (1 << 1)
};

SK_MAKE_BITFIELD_CLASS_OPS(FillPathFlags)

}  // namespace skgpu::ganesh

#endif // FillPathFlags_DEFINED
