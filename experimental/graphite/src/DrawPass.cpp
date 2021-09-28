/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/DrawPass.h"

#include "experimental/graphite/src/DrawList.h"
#include "experimental/graphite/src/SurfaceDrawContext.h"

namespace skgpu {

std::unique_ptr<DrawPass> DrawPass::Make(std::unique_ptr<DrawList> cmds, SurfaceDrawContext* sdc) {
    // TODO: DrawList processing will likely go here and then move the results into the DrawPass
    return std::unique_ptr<DrawPass>(new DrawPass());
}

} // namespace skgpu
