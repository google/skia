/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawOpTest.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "GrUserStencilSettings.h"
#include "SkRandom.h"
#include "SkTypes.h"

#if GR_TEST_UTILS

const GrUserStencilSettings* GrGetRandomStencil(SkRandom* random, GrContext* context) {
    if (context->caps()->avoidStencilBuffers()) {
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
