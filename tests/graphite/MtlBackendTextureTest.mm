/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypes.h"

#import <Metal/Metal.h>

using namespace skgpu::graphite;

namespace {
    const SkISize kSize = {16, 16};
}

DEF_GRAPHITE_TEST_FOR_METAL_CONTEXT(MtlBackendTextureTest, reporter, context, testContext) {
    auto recorder = context->makeRecorder();

    MtlTextureInfo textureInfo;
    textureInfo.fSampleCount = 1;
    textureInfo.fMipmapped = skgpu::Mipmapped::kNo;
    textureInfo.fFormat = MTLPixelFormatRGBA8Unorm;
    textureInfo.fStorageMode = MTLStorageModePrivate;
    textureInfo.fUsage = MTLTextureUsageShaderRead;

    // TODO: For now we are just testing the basic case of RGBA single sample because that is what
    // we've added to the backend. However, once we expand the backend support to handle all the
    // formats this test should iterate over a large set of combinations. See the Ganesh
    // MtlBackendAllocationTest for example of doing this.

    auto beTexture = recorder->createBackendTexture(kSize, textureInfo);
    REPORTER_ASSERT(reporter, beTexture.isValid());
    recorder->deleteBackendTexture(beTexture);

    // It should also pass if we set the usage to be a render target
    textureInfo.fUsage |= MTLTextureUsageRenderTarget;
    beTexture = recorder->createBackendTexture(kSize, textureInfo);
    REPORTER_ASSERT(reporter, beTexture.isValid());
    recorder->deleteBackendTexture(beTexture);

    // It should fail with a format that isn't one of our supported formats
    textureInfo.fFormat = MTLPixelFormatRGB9E5Float;
    beTexture = recorder->createBackendTexture(kSize, textureInfo);
    REPORTER_ASSERT(reporter, !beTexture.isValid());
    recorder->deleteBackendTexture(beTexture);

    // It should fail with a sample count greater than 1
    textureInfo.fFormat = MTLPixelFormatRGBA8Unorm;
    textureInfo.fSampleCount = 4;
    beTexture = recorder->createBackendTexture(kSize, textureInfo);
    REPORTER_ASSERT(reporter, !beTexture.isValid());
    recorder->deleteBackendTexture(beTexture);
}
