/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "experimental/graphite/include/BackendTexture.h"
#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/include/mtl/MtlTypes.h"

#import <Metal/Metal.h>

using namespace skgpu;

namespace {
    const SkISize kSize = {16, 16};
}

DEF_GRAPHITE_TEST_FOR_CONTEXTS(MtlBackendTextureTest, reporter, context) {
    mtl::TextureInfo textureInfo;
    textureInfo.fSampleCount = 1;
    textureInfo.fLevelCount = 1;
    textureInfo.fFormat = MTLPixelFormatRGBA8Unorm;
    textureInfo.fStorageMode = MTLStorageModePrivate;
    textureInfo.fUsage = MTLTextureUsageShaderRead;

    // TODO: For now we are just testing the basic case of RGBA single sample because that is what
    // we've added to the backend. However, once we expand the backend support to handle all the
    // formats this test should iterate over a large set of combinations. See the Ganesh
    // MtlBackendAllocationTest for example of doing this.

    auto beTexture = context->createBackendTexture(kSize, textureInfo);
    REPORTER_ASSERT(reporter, beTexture.isValid());
    context->deleteBackendTexture(beTexture);

    // It should also pass if we set the usage to be a render target
    textureInfo.fUsage |= MTLTextureUsageRenderTarget;
    beTexture = context->createBackendTexture(kSize, textureInfo);
    REPORTER_ASSERT(reporter, beTexture.isValid());
    context->deleteBackendTexture(beTexture);

    // It should fail with a format that isn't rgba8
    textureInfo.fFormat = MTLPixelFormatR8Unorm;
    beTexture = context->createBackendTexture(kSize, textureInfo);
    REPORTER_ASSERT(reporter, !beTexture.isValid());
    context->deleteBackendTexture(beTexture);

    // It should fail with a sample count greater than 1
    textureInfo.fFormat = MTLPixelFormatRGBA8Unorm;
    textureInfo.fSampleCount = 4;
    beTexture = context->createBackendTexture(kSize, textureInfo);
    REPORTER_ASSERT(reporter, !beTexture.isValid());
    context->deleteBackendTexture(beTexture);
}
