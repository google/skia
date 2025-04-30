/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ContextUtils_DEFINED
#define skgpu_graphite_ContextUtils_DEFINED

#include "src/core/SkSLTypeShared.h"
#include "src/gpu/graphite/Uniform.h"

#include <cstdint>
#include <optional>
#include <string>

class SkColorInfo;
class SkM44;
enum class SkBlendMode;
struct SkIRect;

namespace skgpu {
enum class BackendApi : unsigned int;

namespace graphite {
class Caps;
class ComputeStep;
enum class Coverage;
class Geometry;
class PaintParams;
class PaintParamsKeyBuilder;
class PipelineDataGatherer;
class Recorder;
struct RenderPassDesc;
class RenderStep;
struct ResourceBindingRequirements;
class ShaderCodeDictionary;
class UniformManager;
class UniquePaintParamsID;


enum class Layout : uint8_t;

UniquePaintParamsID ExtractPaintData(Recorder*,
                                     PipelineDataGatherer* gatherer,
                                     PaintParamsKeyBuilder* builder,
                                     const Layout layout,
                                     const SkM44& local2Dev,
                                     const PaintParams&,
                                     const Geometry& geometry,
                                     const SkColorInfo& targetColorInfo);

// Intrinsic uniforms used by every program created in Graphite.
//
// `viewport` should hold the actual viewport set as backend state (defining the NDC -> pixel
// transform). The viewport's dimensions are used to define the SkDevice->NDC transform applied in
// the vertex shader, but this assumes that the (0,0) device coordinate maps to the corner of the
// top-left of the NDC cube. The viewport's origin is used in the fragment shader to reconstruct
// the logical fragment coordinate from the target's current frag coord (which are not relative to
// active viewport).
//
// It is assumed that `dstReadBounds` is in the same coordinate space as the `viewport` (e.g.
// final backing target's pixel coords) and that its width and height match the dimensions of the
// texture to be sampled for dst reads.
static constexpr Uniform kIntrinsicUniforms[] = { {"viewport",      SkSLType::kFloat4},
                                                  {"dstReadBounds", SkSLType::kFloat4} };

void CollectIntrinsicUniforms(const Caps* caps,
                              SkIRect viewport,
                              SkIRect dstReadBounds,
                              UniformManager*);

// Returns whether or not hardware blending can be used. If not, we must perform a dst read within
// the shader.
bool CanUseHardwareBlending(const Caps*, std::optional<SkBlendMode>, Coverage);

std::string GetPipelineLabel(const ShaderCodeDictionary*,
                             const RenderPassDesc& renderPassDesc,
                             const RenderStep* renderStep,
                             UniquePaintParamsID paintID);

// TODO(b/396420770): Right now, BuildComputeSkSL must consider the backend in order to make certain
// decisions. It would be ideal if we could make this more backend-agnostic, perhaps by having a
// compute-specific equivalent to ResourceBindingRequirements.
std::string BuildComputeSkSL(const Caps*, const ComputeStep*, BackendApi);

std::string EmitSamplerLayout(const ResourceBindingRequirements&, int* binding);

} // namespace graphite
} // namespace skgpu

#endif // skgpu_graphite_ContextUtils_DEFINED
