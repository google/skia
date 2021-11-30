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
#include "experimental/graphite/src/ContextUtils.h"
#include "experimental/graphite/src/DrawTypes.h"
#include "include/core/SkSpan.h"
#include "include/private/SkOpts_spi.h"
#include "include/private/SkTArray.h"

#include <array>
namespace skgpu {

class RenderStep;

/**
 * GraphicsPipelineDesc represents the state needed to create a backend specific GraphicsPipeline,
 * minus the target-specific properties that can be inferred from the DrawPass and RenderPassTask.
 */
class GraphicsPipelineDesc {
public:
    GraphicsPipelineDesc();

    SkSpan<const uint32_t> asKey() const { return SkMakeSpan(fKey.data(), fKey.size()); }

    bool operator==(const GraphicsPipelineDesc& that) const {
        return this->fKey == that.fKey;
    }

    bool operator!=(const GraphicsPipelineDesc& other) const {
        return !(*this == other);
    }

    // Describes the geometric portion of the pipeline's program and the pipeline's fixed state
    // (except for renderpass-level state that will never change between draws).
    const RenderStep* renderStep() const { return fRenderStep; }
    // Key describing the color shading tree of the pipeline's program
    Combination shaderCombo() const { return fCombination; }

    void setProgram(const RenderStep* step, const Combination& shaderCombo) {
        SkASSERT(step);
        fRenderStep = step;
        fCombination = shaderCombo;

        uintptr_t addr = reinterpret_cast<uintptr_t>(fRenderStep);
        memcpy(fKey.data(), &addr, sizeof(uintptr_t));
        fKey[kWords - 1] = shaderCombo.key();
    }

    struct Hash {
        uint32_t operator()(const GraphicsPipelineDesc& desc) const {
            return SkOpts::hash_fn(desc.fKey.data(), desc.fKey.size() * sizeof(uint32_t), 0);
        }
    };

private:
    // The key is the RenderStep address and the uint32_t key from Combination
    static constexpr int kWords = sizeof(uintptr_t) / sizeof(uint32_t) + 1;
    static_assert(sizeof(uintptr_t) % sizeof(uint32_t) == 0);

    // TODO: I wonder if we could expose the "key" as just a char[] union over the renderstep and
    // paint combination? That would avoid extra size, but definitely locks GraphicsPipelineDesc
    // keys to the current process, which is probably okay since we can have something a with a more
    // stable hash used for the pre-compilation combos.
    std::array<uint32_t, kWords> fKey;

    // Each RenderStep defines a fixed set of attributes and rasterization state, as well as the
    // shader fragments that control the geometry and coverage calculations. The RenderStep's shader
    // is combined with the rest of the shader generated from the PaintParams. Because each
    // RenderStep is fixed, its pointer can be used as a proxy for everything that it specifies in
    // the GraphicsPipeline.
    const RenderStep* fRenderStep = nullptr;

    // TODO: Right now the Combination is roughly the equivalent of the PaintBlob description, so
    // eventually it won't be a fixed size, as it can eventually represent arbitrary shader trees.
    // However, in that world, each PaintBlob structure will have a unique ID and a map from ID to
    // blob, so the GraphicsPipelineDesc can be reduced to just storing RenderStep + unique ID int.
    Combination fCombination;
};

} // namespace skgpu

#endif // skgpu_GraphicsPipelineDesc_DEFINED
