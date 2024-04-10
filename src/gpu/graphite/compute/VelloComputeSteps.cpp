/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/compute/VelloComputeSteps.h"

namespace skgpu::graphite {

std::string_view VelloStageName(vello_cpp::ShaderStage stage) {
    auto name = vello_cpp::shader(stage).name();
    return {name.data(), name.length()};
}

WorkgroupSize VelloStageLocalSize(vello_cpp::ShaderStage stage) {
    auto wgSize = vello_cpp::shader(stage).workgroup_size();
    return WorkgroupSize(wgSize.x, wgSize.y, wgSize.z);
}

skia_private::TArray<ComputeStep::WorkgroupBufferDesc> VelloWorkgroupBuffers(
        vello_cpp::ShaderStage stage) {
    auto wgBuffers = vello_cpp::shader(stage).workgroup_buffers();
    skia_private::TArray<ComputeStep::WorkgroupBufferDesc> result;
    if (!wgBuffers.empty()) {
        result.reserve(wgBuffers.size());
        for (const auto& desc : wgBuffers) {
            result.push_back({desc.size_in_bytes, desc.index});
        }
    }
    return result;
}

ComputeStep::NativeShaderSource VelloNativeShaderSource(vello_cpp::ShaderStage stage,
                                                        ComputeStep::NativeShaderFormat format) {
    using NativeFormat = ComputeStep::NativeShaderFormat;

    const auto& shader = vello_cpp::shader(stage);
    ::rust::Str source;
    std::string entryPoint;
    switch (format) {
#ifdef SK_DAWN
        case NativeFormat::kWGSL:
            source = shader.wgsl();
            entryPoint = "main";
            break;
#endif
#ifdef SK_METAL
        case NativeFormat::kMSL:
            source = shader.msl();
            entryPoint = "main_";
            break;
#endif
        default:
            return {std::string_view(), ""};
    }

    return {{source.data(), source.length()}, std::move(entryPoint)};
}

#define BUFFER_BINDING(slot, type, policy)                       \
        {                                                        \
            /*type=*/ComputeStep::ResourceType::k##type##Buffer, \
            /*flow=*/ComputeStep::DataFlow::kShared,             \
            /*policy=*/ComputeStep::ResourcePolicy::k##policy,   \
            /*slot=*/kVelloSlot_##slot,                          \
        }

#define TEXTURE_BINDING(slot, type, policy)                       \
        {                                                         \
            /*type=*/ComputeStep::ResourceType::k##type##Texture, \
            /*flow=*/ComputeStep::DataFlow::kShared,              \
            /*policy=*/ComputeStep::ResourcePolicy::k##policy,    \
            /*slot=*/kVelloSlot_##slot,                           \
        }

// PathtagReduce
VelloPathtagReduceStep::VelloPathtagReduceStep()
        : VelloStep({
                  BUFFER_BINDING(ConfigUniform,       Uniform, Mapped),
                  BUFFER_BINDING(Scene,               ReadOnlyStorage, Mapped),
                  BUFFER_BINDING(PathtagReduceOutput, Storage, None),
          }) {}

// PathtagScanSmall
VelloPathtagScanSmallStep::VelloPathtagScanSmallStep()
        : VelloStep({
                  BUFFER_BINDING(ConfigUniform,       Uniform, None),
                  BUFFER_BINDING(Scene,               ReadOnlyStorage, None),
                  BUFFER_BINDING(PathtagReduceOutput, ReadOnlyStorage, None),
                  BUFFER_BINDING(TagMonoid,           Storage, None),
          }) {}

// PathtagReduce2
VelloPathtagReduce2Step::VelloPathtagReduce2Step()
        : VelloStep({
                  BUFFER_BINDING(LargePathtagReduceFirstPassOutput,  ReadOnlyStorage, None),
                  BUFFER_BINDING(LargePathtagReduceSecondPassOutput, Storage, None),
          }) {}

// PathtagScan1
VelloPathtagScan1Step::VelloPathtagScan1Step()
        : VelloStep({
                  BUFFER_BINDING(LargePathtagReduceFirstPassOutput,  ReadOnlyStorage, None),
                  BUFFER_BINDING(LargePathtagReduceSecondPassOutput, ReadOnlyStorage, None),
                  BUFFER_BINDING(LargePathtagScanFirstPassOutput,    Storage, None),
          }) {}

// PathtagScanLarge
VelloPathtagScanLargeStep::VelloPathtagScanLargeStep()
        : VelloStep({
                  BUFFER_BINDING(ConfigUniform,                   Uniform, None),
                  BUFFER_BINDING(Scene,                           ReadOnlyStorage, None),
                  BUFFER_BINDING(LargePathtagScanFirstPassOutput, ReadOnlyStorage, None),
                  BUFFER_BINDING(TagMonoid,                       Storage, None),
          }) {}

// BboxClear
VelloBboxClearStep::VelloBboxClearStep()
        : VelloStep({
                  BUFFER_BINDING(ConfigUniform, Uniform, None),
                  BUFFER_BINDING(PathBBoxes,    Storage, None),
          }) {}

// Flatten
VelloFlattenStep::VelloFlattenStep()
        : VelloStep({
                  BUFFER_BINDING(ConfigUniform, Uniform, None),
                  BUFFER_BINDING(Scene,         ReadOnlyStorage, None),
                  BUFFER_BINDING(TagMonoid,     ReadOnlyStorage, None),
                  BUFFER_BINDING(PathBBoxes,    Storage, None),
                  BUFFER_BINDING(BumpAlloc,     Storage, Clear),
                  BUFFER_BINDING(Lines,         Storage, None),
          }) {}

// DrawReduce
VelloDrawReduceStep::VelloDrawReduceStep()
        : VelloStep({
                  BUFFER_BINDING(ConfigUniform,    Uniform, None),
                  BUFFER_BINDING(Scene,            ReadOnlyStorage, None),
                  BUFFER_BINDING(DrawReduceOutput, Storage, None),
          }) {}

// DrawLeaf
VelloDrawLeafStep::VelloDrawLeafStep()
        : VelloStep({
                  BUFFER_BINDING(ConfigUniform,    Uniform, None),
                  BUFFER_BINDING(Scene,            ReadOnlyStorage, None),
                  BUFFER_BINDING(DrawReduceOutput, ReadOnlyStorage, None),
                  BUFFER_BINDING(PathBBoxes,       ReadOnlyStorage, None),
                  BUFFER_BINDING(DrawMonoid,       Storage, None),
                  BUFFER_BINDING(InfoBinData,      Storage, None),
                  BUFFER_BINDING(ClipInput,        Storage, None),
          }) {}

// ClipReduce
VelloClipReduceStep::VelloClipReduceStep()
        : VelloStep({
                  BUFFER_BINDING(ClipInput,    ReadOnlyStorage, None),
                  BUFFER_BINDING(PathBBoxes,   ReadOnlyStorage, None),
                  BUFFER_BINDING(ClipBicyclic, Storage, None),
                  BUFFER_BINDING(ClipElement,  Storage, None),
          }) {}

// ClipLeaf
VelloClipLeafStep::VelloClipLeafStep()
        : VelloStep({
                  BUFFER_BINDING(ConfigUniform, Uniform, None),
                  BUFFER_BINDING(ClipInput,     ReadOnlyStorage, None),
                  BUFFER_BINDING(PathBBoxes,    ReadOnlyStorage, None),
                  BUFFER_BINDING(ClipBicyclic,  ReadOnlyStorage, None),
                  BUFFER_BINDING(ClipElement,   ReadOnlyStorage, None),
                  BUFFER_BINDING(DrawMonoid,    Storage, None),
                  BUFFER_BINDING(ClipBBoxes,    Storage, None),
          }) {}

// Binning
VelloBinningStep::VelloBinningStep()
        : VelloStep({
                  BUFFER_BINDING(ConfigUniform, Uniform, None),
                  BUFFER_BINDING(DrawMonoid,    ReadOnlyStorage, None),
                  BUFFER_BINDING(PathBBoxes,    ReadOnlyStorage, None),
                  BUFFER_BINDING(ClipBBoxes,    ReadOnlyStorage, None),
                  BUFFER_BINDING(DrawBBoxes,    Storage, None),
                  BUFFER_BINDING(BumpAlloc,     Storage, None),
                  BUFFER_BINDING(InfoBinData,   Storage, None),
                  BUFFER_BINDING(BinHeader,     Storage, None),
          }) {}

// TileAlloc
VelloTileAllocStep::VelloTileAllocStep()
        : VelloStep({
                  BUFFER_BINDING(ConfigUniform, Uniform, None),
                  BUFFER_BINDING(Scene,         ReadOnlyStorage, None),
                  BUFFER_BINDING(DrawBBoxes,    ReadOnlyStorage, None),
                  BUFFER_BINDING(BumpAlloc,     Storage, None),
                  BUFFER_BINDING(Path,          Storage, None),
                  BUFFER_BINDING(Tile,          Storage, None),
          }) {}

// PathCountSetup
VelloPathCountSetupStep::VelloPathCountSetupStep()
        : VelloStep({
                  BUFFER_BINDING(BumpAlloc,     Storage, None),
                  BUFFER_BINDING(IndirectCount, Storage, None),
          }) {}

// PathCount
VelloPathCountStep::VelloPathCountStep()
        : VelloStep({
                  BUFFER_BINDING(ConfigUniform, Uniform, None),
                  BUFFER_BINDING(BumpAlloc,     Storage, None),
                  BUFFER_BINDING(Lines,         ReadOnlyStorage, None),
                  BUFFER_BINDING(Path,          ReadOnlyStorage, None),
                  BUFFER_BINDING(Tile,          Storage, None),
                  BUFFER_BINDING(SegmentCounts, Storage, None),
          }) {}

// BackdropDyn
VelloBackdropDynStep::VelloBackdropDynStep()
        : VelloStep({
                  BUFFER_BINDING(ConfigUniform, Uniform, None),
                  BUFFER_BINDING(Path, ReadOnlyStorage, None),
                  BUFFER_BINDING(Tile, Storage, None),
          }) {}

// Coarse
VelloCoarseStep::VelloCoarseStep()
        : VelloStep({
                  BUFFER_BINDING(ConfigUniform, Uniform, None),
                  BUFFER_BINDING(Scene,         ReadOnlyStorage, None),
                  BUFFER_BINDING(DrawMonoid,    ReadOnlyStorage, None),
                  BUFFER_BINDING(BinHeader,     ReadOnlyStorage, None),
                  BUFFER_BINDING(InfoBinData,   ReadOnlyStorage, None),
                  BUFFER_BINDING(Path,          ReadOnlyStorage, None),
                  BUFFER_BINDING(Tile,          Storage, None),
                  BUFFER_BINDING(BumpAlloc,     Storage, None),
                  BUFFER_BINDING(PTCL,          Storage, None),
          }) {}

// PathTilingSetup
VelloPathTilingSetupStep::VelloPathTilingSetupStep()
        : VelloStep({
                  BUFFER_BINDING(BumpAlloc,     Storage, None),
                  BUFFER_BINDING(IndirectCount, Storage, None),
                  BUFFER_BINDING(PTCL,          Storage, None),
          }) {}

// PathTiling
VelloPathTilingStep::VelloPathTilingStep()
        : VelloStep({
                  BUFFER_BINDING(BumpAlloc,     Storage, None),
                  BUFFER_BINDING(SegmentCounts, ReadOnlyStorage, None),
                  BUFFER_BINDING(Lines,         ReadOnlyStorage, None),
                  BUFFER_BINDING(Path,          ReadOnlyStorage, None),
                  BUFFER_BINDING(Tile,          ReadOnlyStorage, None),
                  BUFFER_BINDING(Segments,      Storage, None),
          }) {}

// Fine
static constexpr ComputeStep::ResourceDesc kFineAreaResources[] = {
        BUFFER_BINDING(ConfigUniform, Uniform,          None),
        BUFFER_BINDING(Segments,      ReadOnlyStorage,  None),
        BUFFER_BINDING(PTCL,          ReadOnlyStorage,  None),
        BUFFER_BINDING(InfoBinData,   ReadOnlyStorage,  None),
        TEXTURE_BINDING(OutputImage,  WriteOnlyStorage, None),
};

static constexpr ComputeStep::ResourceDesc kFineMsaaResources[] = {
        BUFFER_BINDING(ConfigUniform, Uniform,          None),
        BUFFER_BINDING(Segments,      ReadOnlyStorage,  None),
        BUFFER_BINDING(PTCL,          ReadOnlyStorage,  None),
        BUFFER_BINDING(InfoBinData,   ReadOnlyStorage,  None),
        TEXTURE_BINDING(OutputImage,  WriteOnlyStorage, None),
        BUFFER_BINDING(MaskLUT, ReadOnlyStorage, Mapped),
};

VelloFineAreaStep::VelloFineAreaStep() : VelloFineStepBase(kFineAreaResources) {}

VelloFineMsaa16Step::VelloFineMsaa16Step() : VelloFineMsaaStepBase(kFineMsaaResources) {}

VelloFineMsaa8Step::VelloFineMsaa8Step() : VelloFineMsaaStepBase(kFineMsaaResources) {}

VelloFineAreaAlpha8Step::VelloFineAreaAlpha8Step() : VelloFineStepBase(kFineAreaResources) {}

VelloFineMsaa16Alpha8Step::VelloFineMsaa16Alpha8Step()
        : VelloFineMsaaStepBase(kFineMsaaResources) {}

VelloFineMsaa8Alpha8Step::VelloFineMsaa8Alpha8Step() : VelloFineMsaaStepBase(kFineMsaaResources) {}

}  // namespace skgpu::graphite
