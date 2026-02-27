/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/graphite/TextureFormatTest.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypes.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/mtl/MtlGraphiteUtils.h"

namespace skgpu::graphite {

static TextureInfo create_mtl_texture_info(TextureFormat format) {
        MtlTextureInfo info;
        info.fFormat = TextureFormatToMTLPixelFormat(format);
        info.fUsage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;

        return TextureInfos::MakeMetal(info);
}

DEF_GRAPHITE_TEST_FOR_METAL_CONTEXT(TextureFormatTest_Metal, r, ctx, testCtx) {
    for (int i = 0; i < kTextureFormatCount; ++i) {
        RunTextureFormatTest(r, ctx->priv().caps(),
                             static_cast<TextureFormat>(i),
                             create_mtl_texture_info);
    }
}

} // namespace skgpu::graphite
