/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/TextureFormatXferFn.h"

#include "include/core/SkColorType.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"

namespace skgpu::graphite {

std::optional<TextureFormatXferFn> TextureFormatXferFn::MakeCpuToGpu(SkColorType srcCT,
                                                                     TextureFormat dstFormat,
                                                                     Swizzle dstReadSwizzle) {
    auto [baseCT, xferOps] = TextureFormatColorTypeInfo(dstFormat);
    if (xferOps & FormatXferOp::kDisabled) {
        return std::nullopt;
    }
    SkASSERT(!(xferOps & FormatXferOp::kDropAlpha)); // Not implemented yet
    Swizzle srcToDst = dstReadSwizzle.invert();
    if (xferOps & FormatXferOp::kSwapRB) {
        srcToDst = Swizzle::Concat(srcToDst, Swizzle::BGRA());
    }
    return TextureFormatXferFn(srcCT, srcToDst, baseCT);
}

std::optional<TextureFormatXferFn> TextureFormatXferFn::MakeGpuToCpu(TextureFormat srcFormat,
                                                                     Swizzle srcReadSwizzle,
                                                                     SkColorType dstCT) {
    auto [baseCT, xferOps] = TextureFormatColorTypeInfo(srcFormat);
    if (xferOps & FormatXferOp::kDisabled) {
        return std::nullopt;
    }
    SkASSERT(!(xferOps & FormatXferOp::kDropAlpha)); // Not implemented yet
    Swizzle srcToDst = srcReadSwizzle;
    if (xferOps & FormatXferOp::kSwapRB) {
        srcToDst = Swizzle::Concat(srcToDst, Swizzle::BGRA());
    }
    return TextureFormatXferFn(baseCT, srcToDst, dstCT);
}

// TODO(michaelludwig): This is a simple stub implementation (we aren't planning to use the
// SkConvertPixels code currently used by UploadTask and asyncReads). It's purpose is to validate
// that the generated test cases make sense before we introduce a new TextureFormatXferFn utility.
// As such, 3-channel formats are not supported.
void TextureFormatXferFn::run(int width, int height,
                              const void* src, size_t srcRowBytes,
                              void* dst, size_t dstRowBytes) const {
    SkASSERT(width >= 1 && height >= 1);
    // The "strides" specified in the SkRP contexts are in pixel units so we have to convert.
    const int srcBpp = SkColorTypeBytesPerPixel(fSrcColorType);
    const int dstBpp = SkColorTypeBytesPerPixel(fDstColorType);
    SkASSERT(srcRowBytes % srcBpp == 0 && srcRowBytes / srcBpp >= (size_t) width);
    SkASSERT(dstRowBytes % dstBpp == 0 && dstRowBytes / dstBpp >= (size_t) width);

    SkRasterPipelineContexts::MemoryCtx srcCtx{const_cast<void*>(src), SkTo<int>(srcRowBytes / srcBpp)},
                                        dstCtx{dst, SkTo<int>(dstRowBytes / dstBpp)};
    SkRasterPipeline_<256> rp;
    rp.appendLoad(fSrcColorType, &srcCtx);
    fSrcToDstSwizzle.apply(&rp);
    rp.appendStore(fDstColorType, &dstCtx);

    rp.run(0, 0, width, height);
}

} // namespace skgpu::graphite
