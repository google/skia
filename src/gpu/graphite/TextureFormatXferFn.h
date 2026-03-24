/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_TextureFormatXferFn_DEFINED
#define skgpu_graphite_TextureFormatXferFn_DEFINED

#include "src/gpu/graphite/TextureFormat.h"

#include <cstdint>
#include <optional>

enum SkColorType : int;

namespace skgpu::graphite {

class TextureFormatXferFn {
public:
    // Builds a transfer function that will convert from CPU data described by `srcCT` to the
    // bit layout required for `dstFormat`, such that converted data can be copied into a texture of
    // the same format.
    static std::optional<TextureFormatXferFn> MakeCpuToGpu(SkColorType srcCT,
                                                           TextureFormat dstFormat,
                                                           Swizzle dstReadSwizzle);

    // Builds a transfer function that will convert from GPU data described by `srcFormat` to the
    // CPU-oriented `dstCT`. This assumes that the input data is the result of a texture-to-buffer
    // copy from a texture of the same format.
    static std::optional<TextureFormatXferFn> MakeGpuToCpu(TextureFormat srcFormat,
                                                           Swizzle srcReadSwizzle,
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
    TextureFormatXferFn(TextureFormat format,
                        SkEnumBitMask<FormatXferOp> preOps,
                        SkColorType srcCT,
                        Swizzle srcToDstSwizzle,
                        SkColorType dstCT,
                        SkEnumBitMask<FormatXferOp> postOps)
            : fFormat(format)
            , fPreOps(preOps)
            , fSrcColorType(srcCT)
            , fSrcToDstSwizzle(srcToDstSwizzle)
            , fDstColorType(dstCT)
            , fPostOps(postOps) {}

    // At most one of fPreOps or fPostOps will be non-identity; whichever that is determines the
    // side of the conversion for which this TextureFormat defines the raw data format. When both
    // pre and post ops are identity, the TextureFormat is equivalent to the src or dst color type
    // and that ambiguity ends up irrelevant in the conversion logic.
    TextureFormat fFormat;

    // Logical flow of conversion:
    SkEnumBitMask<FormatXferOp> fPreOps;  // 1. Bit manipulations to convert raw data to src ct
    SkColorType fSrcColorType;            // 2. The src colortype loaded via SkRasterPipeline
    Swizzle fSrcToDstSwizzle;             // 3. Swizzle applied via SkRasterPipeline to produce dst
    SkColorType fDstColorType;            // 4. The dst colortype stored via SkRasterPipeline
    SkEnumBitMask<FormatXferOp> fPostOps; // 5. Bit manipulations to convert dst to raw data
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_TextureFormatXferFn_DEFINED
