/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "experimental/graphite/include/BackendTexture.h"
#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/include/Recorder.h"
#include "experimental/graphite/include/SkStuff.h"
#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/ContextPriv.h"
#include "experimental/graphite/src/Gpu.h"
#include "experimental/graphite/src/ResourceTypes.h"

#include "include/core/SkSurface.h"

using namespace skgpu;

namespace {
    const SkISize kSize = {16, 16};
}

DEF_GRAPHITE_TEST_FOR_CONTEXTS(BackendTextureTest, reporter, context) {
    auto caps = context->priv().gpu()->caps();

    TextureInfo info = caps->getDefaultSampledTextureInfo(kRGBA_8888_SkColorType,
                                                          /*levelCount=*/1,
                                                          Protected::kNo,
                                                          Renderable::kNo);
    REPORTER_ASSERT(reporter, info.isValid());

    auto texture1 = context->createBackendTexture(kSize, info);
    REPORTER_ASSERT(reporter, texture1.isValid());

    // We make a copy to do the remaining tests so we still have texture1 to safely delete the
    // backend object.
    auto texture1Copy = texture1;
    REPORTER_ASSERT(reporter, texture1Copy.isValid());
    REPORTER_ASSERT(reporter, texture1 == texture1Copy);

    auto texture2 = context->createBackendTexture(kSize, info);
    REPORTER_ASSERT(reporter, texture2.isValid());

    REPORTER_ASSERT(reporter, texture1Copy != texture2);

    // Test state after assignment
    texture1Copy = texture2;
    REPORTER_ASSERT(reporter, texture1Copy.isValid());
    REPORTER_ASSERT(reporter, texture1Copy == texture2);

    BackendTexture invalidTexture;
    REPORTER_ASSERT(reporter, !invalidTexture.isValid());

    texture1Copy = invalidTexture;
    REPORTER_ASSERT(reporter, !texture1Copy.isValid());

    texture1Copy = texture1;
    REPORTER_ASSERT(reporter, texture1Copy.isValid());
    REPORTER_ASSERT(reporter, texture1 == texture1Copy);

    context->deleteBackendTexture(texture1);
    context->deleteBackendTexture(texture2);
}

// Tests the wrapping of a BackendTexture in an SkSurface
DEF_GRAPHITE_TEST_FOR_CONTEXTS(SurfaceBackendTextureTest, reporter, context) {
    // TODO: Right now this just tests very basic combinations of surfaces. This should be expanded
    // to conver a much broader set of things once we add more support in Graphite for different
    // formats, color types, etc.

    auto caps = context->priv().gpu()->caps();
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    TextureInfo info = caps->getDefaultSampledTextureInfo(kRGBA_8888_SkColorType,
                                                          /*levelCount=*/1,
                                                          Protected::kNo,
                                                          Renderable::kYes);

    auto texture = context->createBackendTexture(kSize, info);
    REPORTER_ASSERT(reporter, texture.isValid());

    sk_sp<SkSurface> surface = MakeGraphiteFromBackendTexture(recorder.get(),
                                                              texture,
                                                              kRGBA_8888_SkColorType,
                                                              /*colorSpace=*/nullptr,
                                                              /*props=*/nullptr);
    REPORTER_ASSERT(reporter, surface);

    surface.reset();

    // We should fail when trying to wrap the same texture in a surface with a non compatible
    // color type.
    surface = MakeGraphiteFromBackendTexture(recorder.get(),
                                             texture,
                                             kAlpha_8_SkColorType,
                                             /*colorSpace=*/nullptr,
                                             /*props=*/nullptr);
    REPORTER_ASSERT(reporter, !surface);

    context->deleteBackendTexture(texture);

    // We should fail to make a wrap non renderable texture in a surface.
    info = caps->getDefaultSampledTextureInfo(kRGBA_8888_SkColorType,
                                              /*levelCount=*/1,
                                              Protected::kNo,
                                              Renderable::kNo);
    texture = context->createBackendTexture(kSize, info);
    REPORTER_ASSERT(reporter, texture.isValid());

    surface = MakeGraphiteFromBackendTexture(recorder.get(),
                                             texture,
                                             kRGBA_8888_SkColorType,
                                             /*colorSpace=*/nullptr,
                                             /*props=*/nullptr);

    REPORTER_ASSERT(reporter, !surface);
    context->deleteBackendTexture(texture);
}

