/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrDrawOpTest.h"

#include "include/core/SkTypes.h"
#include "include/private/GrContext_Base.h"
#include "include/utils/SkRandom.h"
#include "src/gpu/GrBaseContextPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrUserStencilSettings.h"

#if GR_TEST_UTILS

const GrUserStencilSettings* GrGetRandomStencil(SkRandom* random, GrContext_Base* context) {
    if (context->priv().caps()->avoidStencilBuffers()) {
        return &GrUserStencilSettings::kUnused;
    }
    static constexpr GrUserStencilSettings kReads(
        GrUserStencilSettings::StaticInit<
            0x8080,
            GrUserStencilTest::kLess,
            0xffff,
            GrUserStencilOp::kKeep,
            GrUserStencilOp::kKeep,
            0xffff>()
    );
    static constexpr GrUserStencilSettings kWrites(
        GrUserStencilSettings::StaticInit<
            0xffff,
            GrUserStencilTest::kAlways,
            0xffff,
            GrUserStencilOp::kReplace,
            GrUserStencilOp::kReplace,
            0xffff>()
    );
    static constexpr GrUserStencilSettings kReadsAndWrites(
        GrUserStencilSettings::StaticInit<
            0x8000,
            GrUserStencilTest::kEqual,
            0x6000,
            GrUserStencilOp::kIncWrap,
            GrUserStencilOp::kInvert,
            0x77ff>()
    );

    static const GrUserStencilSettings* kStencilSettings[] = {
            &GrUserStencilSettings::kUnused,
            &kReads,
            &kWrites,
            &kReadsAndWrites,
    };
    return kStencilSettings[random->nextULessThan(SK_ARRAY_COUNT(kStencilSettings))];
}

#endif
