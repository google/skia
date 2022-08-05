/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/piet/Render.h"

#include "include/core/SkTypes.h"
#include "src/gpu/piet/Scene.h"

namespace skgpu::piet {

RendererBase::RendererBase(void* device, void* queue) : Object(pgpu_renderer_new(device, queue)) {
    SkASSERT(this->get() != nullptr);
}

void RendererBase::render(const Scene& scene, void* target, void* cmdBuffer) const {
    // TODO: track ID and release resources upon completion
    pgpu_renderer_render(this->get(), scene.get(), target, cmdBuffer);
}

}  // namespace skgpu::piet
