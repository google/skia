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

// PathtagReduce
VelloPathtagReduceStep::VelloPathtagReduceStep() : VelloStep(
        /*resources=*/{
            {
                /*type=*/ResourceType::kUniformBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kMapped,
                /*slot=*/kVelloSlot_ConfigUniform,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kMapped,
                /*slot=*/kVelloSlot_Scene,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_PathtagReduceOutput,
            },
        }) {}

// PathtagScanSmall
VelloPathtagScanSmallStep::VelloPathtagScanSmallStep() : VelloStep(
        /*resources=*/{
            {
                /*type=*/ResourceType::kUniformBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_ConfigUniform,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_Scene,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_PathtagReduceOutput,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_TagMonoid,
            },
        }) {}


// PathtagReduce2
VelloPathtagReduce2Step::VelloPathtagReduce2Step() : VelloStep(
        /*resources=*/{
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_LargePathtagReduceFirstPassOutput,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_LargePathtagReduceSecondPassOutput,
            },
        }) {}

// PathtagScan1
VelloPathtagScan1Step::VelloPathtagScan1Step() : VelloStep(
        /*resources=*/{
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_LargePathtagReduceFirstPassOutput,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_LargePathtagReduceSecondPassOutput,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_LargePathtagScanFirstPassOutput,
            },
        }) {}

// PathtagScanLarge
VelloPathtagScanLargeStep::VelloPathtagScanLargeStep() : VelloStep(
        /*resources=*/{
            {
                /*type=*/ResourceType::kUniformBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_ConfigUniform,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_Scene,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_LargePathtagScanFirstPassOutput,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_TagMonoid,
            },
        }) {}

// BboxClear
VelloBboxClearStep::VelloBboxClearStep() : VelloStep(
        /*resources=*/{
            {
                /*type=*/ResourceType::kUniformBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_ConfigUniform,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_PathBBoxes,
            },
        }) {}

// Pathseg
VelloPathsegStep::VelloPathsegStep() : VelloStep(
        /*resources=*/{
            {
                /*type=*/ResourceType::kUniformBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_ConfigUniform,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_Scene,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_TagMonoid,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_PathBBoxes,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_Cubics,
            },
        }) {}

// DrawReduce
VelloDrawReduceStep::VelloDrawReduceStep() : VelloStep(
        /*resources=*/{
            {
                /*type=*/ResourceType::kUniformBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_ConfigUniform,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_Scene,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_DrawReduceOutput,
            },
        }) {}

// DrawLeaf
VelloDrawLeafStep::VelloDrawLeafStep() : VelloStep(
        /*resources=*/{
            {
                /*type=*/ResourceType::kUniformBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_ConfigUniform,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_Scene,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_DrawReduceOutput,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_PathBBoxes,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_DrawMonoid,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_InfoBinData,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_ClipInput,
            },
        }) {}

// ClipReduce
VelloClipReduceStep::VelloClipReduceStep() : VelloStep(
        /*resources=*/{
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_ClipInput,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_PathBBoxes,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_ClipBicyclic,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_ClipElement,
            },
        }) {}

// ClipLeaf
VelloClipLeafStep::VelloClipLeafStep() : VelloStep(
        /*resources=*/{
            {
                /*type=*/ResourceType::kUniformBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_ConfigUniform,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_ClipInput,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_PathBBoxes,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_ClipBicyclic,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_ClipElement,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_DrawMonoid,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_ClipBBoxes,
            },
        }) {}

// Binning
VelloBinningStep::VelloBinningStep() : VelloStep(
        /*resources=*/{
            {
                /*type=*/ResourceType::kUniformBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_ConfigUniform,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_DrawMonoid,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_PathBBoxes,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_ClipBBoxes,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_DrawBBoxes,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kClear,
                /*slot=*/kVelloSlot_BumpAlloc,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_InfoBinData,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_BinHeader,
            },
        }) {}

// TileAlloc
VelloTileAllocStep::VelloTileAllocStep() : VelloStep(
        /*resources=*/{
            {
                /*type=*/ResourceType::kUniformBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_ConfigUniform,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_Scene,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_DrawBBoxes,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_BumpAlloc,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_Path,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_Tile,
            },
        }) {}

// PathCoarseFull
VelloPathCoarseFullStep::VelloPathCoarseFullStep() : VelloStep(
        /*resources=*/{
            {
                /*type=*/ResourceType::kUniformBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_ConfigUniform,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_Scene,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_Cubics,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_Path,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_BumpAlloc,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_Tile,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_Segments,
            },
        }) {}

// Backdrop
VelloBackdropStep::VelloBackdropStep() : VelloStep(
        /*resources=*/{
            {
                /*type=*/ResourceType::kUniformBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_ConfigUniform,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_Tile,
            },
        }) {}

// BackdropDyn
VelloBackdropDynStep::VelloBackdropDynStep() : VelloStep(
        /*resources=*/{
            {
                /*type=*/ResourceType::kUniformBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_ConfigUniform,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_Path,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_Tile,
            },
        }) {}

// Coarse
VelloCoarseStep::VelloCoarseStep() : VelloStep(
        /*resources=*/{
            {
                /*type=*/ResourceType::kUniformBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_ConfigUniform,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_Scene,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_DrawMonoid,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_BinHeader,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_InfoBinData,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_Path,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_Tile,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_BumpAlloc,
            },
            {
                /*type=*/ResourceType::kStorageBuffer,
                /*flow=*/DataFlow::kShared,
                /*policy=*/ResourcePolicy::kNone,
                /*slot=*/kVelloSlot_PTCL,
            },
        }) {}

// Fine
VelloFineStep::VelloFineStep(SkColorType targetFormat)
        : VelloStep(
            /*resources=*/{
                {
                    /*type=*/ResourceType::kUniformBuffer,
                    /*flow=*/DataFlow::kShared,
                    /*policy=*/ResourcePolicy::kNone,
                    /*slot=*/kVelloSlot_ConfigUniform,
                },
                {
                    /*type=*/ResourceType::kStorageBuffer,
                    /*flow=*/DataFlow::kShared,
                    /*policy=*/ResourcePolicy::kNone,
                    /*slot=*/kVelloSlot_Segments,
                },
                {
                    /*type=*/ResourceType::kStorageBuffer,
                    /*flow=*/DataFlow::kShared,
                    /*policy=*/ResourcePolicy::kNone,
                    /*slot=*/kVelloSlot_PTCL,
                },
                {
                    /*type=*/ResourceType::kStorageBuffer,
                    /*flow=*/DataFlow::kShared,
                    /*policy=*/ResourcePolicy::kNone,
                    /*slot=*/kVelloSlot_InfoBinData,
                },
                {
                    /*type=*/ResourceType::kWriteOnlyStorageTexture,
                    /*flow=*/DataFlow::kShared,
                    /*policy=*/ResourcePolicy::kNone,
                    /*slot=*/kVelloSlot_OutputImage,
                },
                {
                    /*type=*/ResourceType::kReadOnlyTexture,
                    /*flow=*/DataFlow::kShared,
                    /*policy=*/ResourcePolicy::kNone,
                    /*slot=*/kVelloSlot_GradientImage,
                },
                {
                    /*type=*/ResourceType::kReadOnlyTexture,
                    /*flow=*/DataFlow::kShared,
                    /*policy=*/ResourcePolicy::kNone,
                    /*slot=*/kVelloSlot_ImageAtlas,
                },
            })
        , fTargetFormat(targetFormat) {}

std::tuple<SkISize, SkColorType> VelloFineStep::calculateTextureParameters(int index, const ResourceDesc&) const {
    return {{}, index == 4 ? fTargetFormat : kRGBA_8888_SkColorType};
}

}  // namespace skgpu::graphite
