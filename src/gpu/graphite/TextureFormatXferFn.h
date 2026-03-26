/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_TextureFormatXferFn_DEFINED
#define skgpu_graphite_TextureFormatXferFn_DEFINED

#include "src/base/SkArenaAlloc.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/gpu/graphite/TextureFormat.h"

#include <cstdint>
#include <functional>
#include <optional>

struct SkColorSpaceXformSteps;
enum SkColorType : int;

namespace skgpu::graphite {

class TextureFormatXferFn {
public:
    // Builds a transfer function that will convert from CPU data described by `srcCT` to the
    // bit layout required for `dstFormat`, such that converted data can be copied into a texture of
    // the same format. Colorspace and alpha type conversions are applied between loading src data
    // and writing out the dst format data.
    static std::optional<TextureFormatXferFn> MakeCpuToGpu(SkColorType srcCT,
                                                           const SkColorSpaceXformSteps& csSteps,
                                                           TextureFormat dstFormat,
                                                           Swizzle dstReadSwizzle);

    // Builds a transfer function that will convert from GPU data described by `srcFormat` to the
    // CPU-oriented `dstCT`. This assumes that the input data is the result of a texture-to-buffer
    // copy from a texture of the same format. Colorspace and alpha type conversions are applied
    // between loading the swizzled src format data and writing out the dst color data.
    static std::optional<TextureFormatXferFn> MakeGpuToCpu(TextureFormat srcFormat,
                                                           Swizzle srcReadSwizzle,
                                                           const SkColorSpaceXformSteps& csSteps,
                                                           SkColorType dstCT);

    // Apply the transfer function to a width x height block of pixels in `src` and write the
    // results to `dst`. Each `width` row of pixels in the src and dst images advances their
    // respective buffers by `srcRowBytes` and `dstRowBytes` respectively.
    //
    // It is assumed that the row bytes and data contents match the formats and color types provided
    // when the TextureFormatXferFn was created.
    void run(int width, int height,
             const void* src, size_t srcRowBytes,
             void* dst, size_t dstRowBytes) const;

private:
    struct RPOps {
        SkRasterPipelineContexts::MemoryCtx fSrcCtx{nullptr, 0};
        SkRasterPipelineContexts::MemoryCtx fDstCtx{nullptr, 0};

        SkSTArenaAlloc<256> fArena; // holds raster pipeline and other op contexts
        SkRasterPipeline fRP; // backed by fArena

        const int fSrcBpp;
        const int fDstBpp;

        template<typename... RPModifiers>
        static std::unique_ptr<RPOps> Make(SkColorType srcCT, SkColorType dstCT, RPModifiers...);

        // Returns true if RasterPipeline can process the whole 2D block via its strides
        bool setStrides(size_t srcRowBytes, size_t dstRowBytes, uint8_t otherOps);

    private:
        RPOps(int srcBpp, int dstBpp) : fRP(&fArena), fSrcBpp(srcBpp), fDstBpp(dstBpp) {}
    };

    TextureFormatXferFn(TextureFormat format,
                        uint8_t preOps,
                        std::unique_ptr<RPOps> rp,
                        uint8_t postOps)
            : fFormat(format), fPreOps(preOps), fRP(std::move(rp)), fPostOps(postOps) {
        // At least one direction should not add extra conversion operations
        SkASSERT(preOps == 0 || postOps == 0);
    }

    // At most one of fPreOps or fPostOps will be non-identity; whichever that is determines the
    // side of the conversion for which this TextureFormat defines the raw data format. When both
    // pre and post ops are identity, the TextureFormat is equivalent to the src or dst color type
    // and that ambiguity ends up irrelevant in the conversion logic.
    TextureFormat fFormat;

    // Logical flow of conversion (preOps and postOps are bit masks holding an extended set of
    // conversion ops built from FormatXferOp):
    uint8_t fPreOps;            // 1. Bit manipulations to convert raw data to src ct
    std::unique_ptr<RPOps> fRP; // 2. Raster pipeline to convert to dst ct
    uint8_t fPostOps;           // 3. Bit manipulations to convert dst ct to raw data
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_TextureFormatXferFn_DEFINED
