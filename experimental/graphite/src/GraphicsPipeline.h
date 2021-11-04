/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GraphicsPipeline_DEFINED
#define skgpu_GraphicsPipeline_DEFINED

#include "include/core/SkRefCnt.h"

namespace skgpu {

/**
 * GraphicsPipeline corresponds to a backend specific pipeline used for rendering (vs. compute),
 * e.g. MTLRenderPipelineState (Metal),
 *      CreateRenderPipeline (Dawn),
 *      CreateGraphicsPipelineState (D3D12),
 *   or VkGraphicsPipelineCreateInfo (Vulkan).
 *
 * A GraphicsPipeline is created from the combination of a GraphicsPipelineDesc (representing draw
 * specific configuration) and a RenderPassDesc (representing the target of the draw).
 */
class GraphicsPipeline : public SkRefCnt {
public:
    ~GraphicsPipeline() override;

protected:
    GraphicsPipeline();

private:
};

} // namespace skgpu

#endif // skgpu_GraphicsPipeline_DEFINED
