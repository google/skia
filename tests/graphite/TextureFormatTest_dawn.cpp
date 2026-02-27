/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/graphite/TextureFormatTest.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/dawn/DawnGraphiteTypes.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtils.h"

#include "webgpu/webgpu_cpp.h"

namespace skgpu::graphite {

WGPU_IMPORT_BITMASK_OPERATORS

static TextureInfo create_dawn_texture_info(TextureFormat format) {
    DawnTextureInfo info;
    info.fFormat = TextureFormatToDawnFormat(format);
    info.fUsage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment;

    return TextureInfos::MakeDawn(info);
}

DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(TextureFormatTest_Dawn, r, ctx, testCtx) {
    for (int i = 0; i < kTextureFormatCount; ++i) {
        RunTextureFormatTest(r, ctx->priv().caps(),
                             static_cast<TextureFormat>(i),
                             create_dawn_texture_info);
    }
}

} // namespace skgpu::graphite
