/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GraphicsPipelineDesc_DEFINED
#define skgpu_GraphicsPipelineDesc_DEFINED

#include "include/core/SkTypes.h"

#include "experimental/graphite/src/Attribute.h"
#include "experimental/graphite/src/DrawTypes.h"
#include "include/private/SkTArray.h"

namespace skgpu {

class RenderStep;

/**
 * GraphicsPipelineDesc represents the state needed to create a backend specific GraphicsPipeline,
 * minus the target-specific properties that can be inferred from the DrawPass and RenderPassTask.
 */
class GraphicsPipelineDesc {
public:
    GraphicsPipelineDesc();

    // Returns this as a uint32_t array to be used as a key in the pipeline cache.
    // TODO: Do we want to do anything here with a tuple or an SkSpan?
    const uint32_t* asKey() const {
        return fKey.data();
    }

    // Gets the number of bytes in asKey(). It will be a 4-byte aligned value.
    uint32_t keyLength() const {
        return fKey.size() * sizeof(uint32_t);
    }

    bool operator==(const GraphicsPipelineDesc& that) const {
        return this->fKey == that.fKey;
    }

    bool operator!=(const GraphicsPipelineDesc& other) const {
        return !(*this == other);
    }

    const RenderStep* renderStep() const { return fRenderStep; }
    void setRenderStep(const RenderStep* step) {
        SkASSERT(step);
        fRenderStep = step;

        static constexpr int kWords = sizeof(uintptr_t) / sizeof(uint32_t);
        static_assert(sizeof(uintptr_t) % sizeof(uint32_t) == 0);

        if (fKey.count() < kWords) {
            fKey.push_back_n(kWords - fKey.count());
        }

        uintptr_t addr = reinterpret_cast<uintptr_t>(fRenderStep);
        memcpy(fKey.data(), &addr, sizeof(uintptr_t));
    }

private:
    // Estimate of max expected key size
    // TODO: flesh this out
    inline static constexpr int kPreAllocSize = 1;

    // TODO: I wonder if we could expose the "key" as just a char[] union over the renderstep and
    // paint combination? That would avoid extra size, but definitely locks GraphicsPipelineDesc
    // keys to the current process, which is probably okay since we can have something a with a more
    // stable hash used for the pre-compilation combos.
    SkSTArray<kPreAllocSize, uint32_t, true> fKey;

    // Each RenderStep defines a fixed set of attributes and rasterization state, as well as the
    // shader fragments that control the geometry and coverage calculations. The RenderStep's shader
    // is combined with the rest of the shader generated from the PaintParams. Because each
    // RenderStep is fixed, its pointer can be used as a proxy for everything that it specifies in
    // the GraphicsPipeline.
    const RenderStep* fRenderStep = nullptr;
};

} // namespace skgpu

#endif // skgpu_GraphicsPipelineDesc_DEFINED
