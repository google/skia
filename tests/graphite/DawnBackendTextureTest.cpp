/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/dawn/DawnTypes.h"
#include "src/gpu/graphite/dawn/DawnGraphiteTypesPriv.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

using namespace skgpu::graphite;

namespace {
const SkISize kSize = {16, 16};
}

DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(DawnBackendTextureSimpleCreationTest,
                                   reporter,
                                   context,
                                   testContext) {
    auto recorder = context->makeRecorder();

    DawnTextureInfo textureInfo;
    textureInfo.fSampleCount = 1;
    textureInfo.fMipmapped = skgpu::Mipmapped::kNo;
    textureInfo.fFormat = wgpu::TextureFormat::RGBA8Unorm;
    textureInfo.fUsage = wgpu::TextureUsage::TextureBinding;

    auto beTexture = recorder->createBackendTexture(kSize, TextureInfos::MakeDawn(textureInfo));
    REPORTER_ASSERT(reporter, beTexture.isValid());
    recorder->deleteBackendTexture(beTexture);

    // It should also pass if we set the usage to be a render target
    textureInfo.fUsage |= wgpu::TextureUsage::RenderAttachment;
    beTexture = recorder->createBackendTexture(kSize, TextureInfos::MakeDawn(textureInfo));
    REPORTER_ASSERT(reporter, beTexture.isValid());
    recorder->deleteBackendTexture(beTexture);
}

// Test that copying BackendTexture variables works.
DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(DawnBackendTextureCopyVariableTest,
                                   reporter,
                                   context,
                                   testContext) {
    auto recorder = context->makeRecorder();

    DawnTextureInfo textureInfo;
    textureInfo.fSampleCount = 1;
    textureInfo.fMipmapped = skgpu::Mipmapped::kNo;
    textureInfo.fFormat = wgpu::TextureFormat::RGBA8Unorm;
    textureInfo.fUsage = wgpu::TextureUsage::TextureBinding;

    BackendTexture beTexture =
            recorder->createBackendTexture(kSize, TextureInfos::MakeDawn(textureInfo));
    REPORTER_ASSERT(reporter, beTexture.isValid());

    BackendTexture beTexture2;
    REPORTER_ASSERT(reporter, beTexture2 != beTexture);
    REPORTER_ASSERT(reporter, BackendTextures::GetDawnTexturePtr(beTexture2) == nullptr);
    REPORTER_ASSERT(reporter, BackendTextures::GetDawnTextureViewPtr(beTexture2) == nullptr);

    beTexture2 = beTexture;
    REPORTER_ASSERT(reporter, beTexture2 == beTexture);
    REPORTER_ASSERT(reporter, BackendTextures::GetDawnTexturePtr(beTexture2) != nullptr);
    REPORTER_ASSERT(reporter,
                    BackendTextures::GetDawnTexturePtr(beTexture2) ==
                            BackendTextures::GetDawnTexturePtr(beTexture));

    recorder->deleteBackendTexture(beTexture);
}
