/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_piet_Renderer_DEFINED
#define skgpu_piet_Renderer_DEFINED

#include "src/gpu/piet/PietTypes.h"

#include <memory>

#if defined(SK_METAL) && defined(__OBJC__)
#import <Metal/Metal.h>
#endif

/**
 * Renderer provides the piet library interface for encoding compute dispatches to render a
 * skgpu::piet::Scene to a target texture. The Renderer template is meant to be specialized to
 * directly depend on graphics backend types and acts as the glue between a GPU backend and the
 * pgpu-render library.
 *
 * For instance, the `MtlRenderer` specialization operates directly on Metal framework types and
 * this header file can be included directly in an Obj-C++ program.
 */

namespace skgpu::piet {

class Scene;

class RendererBase : public Object<PgpuRenderer, pgpu_renderer_destroy> {
protected:
    RendererBase(void* device, void* queue);
    ~RendererBase() override = default;

    void render(const Scene& scene, void* target, void* cmdBuffer) const;

private:
    RendererBase(const RendererBase&) = delete;
    RendererBase(RendererBase&&) = delete;
};

template <typename BackendTraits> class Renderer final : public RendererBase {
    using Device = typename BackendTraits::Device;
    using CommandQueue = typename BackendTraits::CommandQueue;
    using CommandBuffer = typename BackendTraits::CommandBuffer;
    using Texture = typename BackendTraits::Texture;

public:
    Renderer(Device device, CommandQueue queue)
            : RendererBase(static_cast<void*>(device), static_cast<void*>(queue)) {}

    ~Renderer() override = default;

    void render(const Scene& scene, Texture target, CommandBuffer cmdBuffer) const {
        RendererBase::render(scene, static_cast<void*>(target), static_cast<void*>(cmdBuffer));
    }
};

#if defined(SK_METAL) && defined(__OBJC__)

// The MtlRenderer is inteded to be accessed by Obj-C code as it has a direct dependency on Metal
// framework objects.
struct MtlBackendTraits {
    using Device = id<MTLDevice>;
    using CommandQueue = id<MTLCommandQueue>;
    using CommandBuffer = id<MTLCommandBuffer>;
    using Texture = id<MTLTexture>;
};
using MtlRenderer = Renderer<MtlBackendTraits>;

#endif  // defined(SK_METAL) && defined(__OBJC__)

}  // namespace skgpu::piet

#endif  // skgpu_piet_Renderer_DEFINED
