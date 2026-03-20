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
    TextureFormatXferFn(SkColorType srcCT,
                        Swizzle srcToDstSwizzle,
                        SkColorType dstCT)
            : fSrcColorType(srcCT)
            , fSrcToDstSwizzle(srcToDstSwizzle)
            , fDstColorType(dstCT) {}

    // Logical flow of conversion:
    SkColorType fSrcColorType;            // 1. The src colortype loaded via SkRasterPipeline
    Swizzle fSrcToDstSwizzle;             // 2. Swizzle applied via SkRasterPipeline to produce dst
    SkColorType fDstColorType;            // 3. The dst colortype stored via SkRasterPipeline
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_TextureFormatXferFn_DEFINED
