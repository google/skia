/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathStencilSettings_DEFINED
#define GrPathStencilSettings_DEFINED

#include "GrUserStencilSettings.h"

////////////////////////////////////////////////////////////////////////////////
// Stencil rules for paths

////// Even/Odd

static constexpr GrUserStencilSettings gEOStencilPass(
    GrUserStencilSettings::StaticInit<
        0xffff,
        GrUserStencilTest::kAlwaysIfInClip,
        0xffff,
        GrUserStencilOp::kInvert,
        GrUserStencilOp::kKeep,
        0xffff>()
);

// ok not to check clip b/c stencil pass only wrote inside clip
static constexpr GrUserStencilSettings gEOColorPass(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kNotEqual,
        0xffff,
        GrUserStencilOp::kZero,
        GrUserStencilOp::kZero,
        0xffff>()
);

// have to check clip b/c outside clip will always be zero.
static constexpr GrUserStencilSettings gInvEOColorPass(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kEqualIfInClip,
        0xffff,
        GrUserStencilOp::kZero,
        GrUserStencilOp::kZero,
        0xffff>()
);

////// Winding

// when we have separate stencil we increment front faces / decrement back faces
// when we don't have wrap incr and decr we use the stencil test to simulate
// them.

static constexpr GrUserStencilSettings gWindStencilSeparateWithWrap(
    GrUserStencilSettings::StaticInitSeparate<
        0xffff,                                0xffff,
        GrUserStencilTest::kAlwaysIfInClip,    GrUserStencilTest::kAlwaysIfInClip,
        0xffff,                                0xffff,
        GrUserStencilOp::kIncWrap,             GrUserStencilOp::kDecWrap,
        GrUserStencilOp::kKeep,                GrUserStencilOp::kKeep,
        0xffff,                                0xffff>()
);

// if inc'ing the max value, invert to make 0
// if dec'ing zero invert to make all ones.
// we can't avoid touching the stencil on both passing and
// failing, so we can't resctrict ourselves to the clip.
static constexpr GrUserStencilSettings gWindStencilSeparateNoWrap(
    GrUserStencilSettings::StaticInitSeparate<
        0xffff,                                0x0000,
        GrUserStencilTest::kEqual,             GrUserStencilTest::kEqual,
        0xffff,                                0xffff,
        GrUserStencilOp::kInvert,              GrUserStencilOp::kInvert,
        GrUserStencilOp::kIncMaybeClamp,       GrUserStencilOp::kDecMaybeClamp,
        0xffff,                                0xffff>()
);

// When there are no separate faces we do two passes to setup the winding rule
// stencil. First we draw the front faces and inc, then we draw the back faces
// and dec. These are same as the above two split into the incrementing and
// decrementing passes.
static constexpr GrUserStencilSettings gWindSingleStencilWithWrapInc(
    GrUserStencilSettings::StaticInit<
        0xffff,
        GrUserStencilTest::kAlwaysIfInClip,
        0xffff,
        GrUserStencilOp::kIncWrap,
        GrUserStencilOp::kKeep,
        0xffff>()
);

static constexpr GrUserStencilSettings gWindSingleStencilWithWrapDec(
    GrUserStencilSettings::StaticInit<
        0xffff,
        GrUserStencilTest::kAlwaysIfInClip,
        0xffff,
        GrUserStencilOp::kDecWrap,
        GrUserStencilOp::kKeep,
        0xffff>()
);

static constexpr GrUserStencilSettings gWindSingleStencilNoWrapInc(
    GrUserStencilSettings::StaticInit<
        0xffff,
        GrUserStencilTest::kEqual,
        0xffff,
        GrUserStencilOp::kInvert,
        GrUserStencilOp::kIncMaybeClamp,
        0xffff>()
);

static constexpr GrUserStencilSettings gWindSingleStencilNoWrapDec(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kEqual,
        0xffff,
        GrUserStencilOp::kInvert,
        GrUserStencilOp::kDecMaybeClamp,
        0xffff>()
);

// Color passes are the same whether we use the two-sided stencil or two passes

static constexpr GrUserStencilSettings gWindColorPass(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kLessIfInClip, // "0 < stencil" is equivalent to "0 != stencil".
        0xffff,
        GrUserStencilOp::kZero,
        GrUserStencilOp::kZero,
        0xffff>()
);

static constexpr GrUserStencilSettings gInvWindColorPass(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kEqualIfInClip,
        0xffff,
        GrUserStencilOp::kZero,
        GrUserStencilOp::kZero,
        0xffff>()
);

////// Normal render to stencil

// Sometimes the default path renderer can draw a path directly to the stencil
// buffer without having to first resolve the interior / exterior.
static constexpr GrUserStencilSettings gDirectToStencil(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kAlwaysIfInClip,
        0xffff,
        GrUserStencilOp::kZero,
        GrUserStencilOp::kIncMaybeClamp,
        0xffff>()
);

#endif
