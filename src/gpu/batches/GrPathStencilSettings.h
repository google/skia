/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathStencilSettings_DEFINED
#define GrPathStencilSettings_DEFINED

////////////////////////////////////////////////////////////////////////////////
// Stencil rules for paths

////// Even/Odd

static constexpr GrStencilSettings gEOStencilPass(
    kInvert_StencilOp,
    kKeep_StencilOp,
    kAlwaysIfInClip_StencilFunc,
    0xffff,
    0xffff,
    0xffff);

// ok not to check clip b/c stencil pass only wrote inside clip
static constexpr GrStencilSettings gEOColorPass(
    kZero_StencilOp,
    kZero_StencilOp,
    kNotEqual_StencilFunc,
    0xffff,
    0x0000,
    0xffff);

// have to check clip b/c outside clip will always be zero.
static constexpr GrStencilSettings gInvEOColorPass(
    kZero_StencilOp,
    kZero_StencilOp,
    kEqualIfInClip_StencilFunc,
    0xffff,
    0x0000,
    0xffff);

////// Winding

// when we have separate stencil we increment front faces / decrement back faces
// when we don't have wrap incr and decr we use the stencil test to simulate
// them.

static constexpr GrStencilSettings gWindStencilSeparateWithWrap(
    kIncWrap_StencilOp,             kDecWrap_StencilOp,
    kKeep_StencilOp,                kKeep_StencilOp,
    kAlwaysIfInClip_StencilFunc,    kAlwaysIfInClip_StencilFunc,
    0xffff,                         0xffff,
    0xffff,                         0xffff,
    0xffff,                         0xffff);

// if inc'ing the max value, invert to make 0
// if dec'ing zero invert to make all ones.
// we can't avoid touching the stencil on both passing and
// failing, so we can't resctrict ourselves to the clip.
static constexpr GrStencilSettings gWindStencilSeparateNoWrap(
    kInvert_StencilOp,              kInvert_StencilOp,
    kIncClamp_StencilOp,            kDecClamp_StencilOp,
    kEqual_StencilFunc,             kEqual_StencilFunc,
    0xffff,                         0xffff,
    0xffff,                         0x0000,
    0xffff,                         0xffff);

// When there are no separate faces we do two passes to setup the winding rule
// stencil. First we draw the front faces and inc, then we draw the back faces
// and dec. These are same as the above two split into the incrementing and
// decrementing passes.
static constexpr GrStencilSettings gWindSingleStencilWithWrapInc(
    kIncWrap_StencilOp,
    kKeep_StencilOp,
    kAlwaysIfInClip_StencilFunc,
    0xffff,
    0xffff,
    0xffff);

static constexpr GrStencilSettings gWindSingleStencilWithWrapDec(
    kDecWrap_StencilOp,
    kKeep_StencilOp,
    kAlwaysIfInClip_StencilFunc,
    0xffff,
    0xffff,
    0xffff);

static constexpr GrStencilSettings gWindSingleStencilNoWrapInc(
    kInvert_StencilOp,
    kIncClamp_StencilOp,
    kEqual_StencilFunc,
    0xffff,
    0xffff,
    0xffff);

static constexpr GrStencilSettings gWindSingleStencilNoWrapDec(
    kInvert_StencilOp,
    kDecClamp_StencilOp,
    kEqual_StencilFunc,
    0xffff,
    0x0000,
    0xffff);

// Color passes are the same whether we use the two-sided stencil or two passes

static constexpr GrStencilSettings gWindColorPass(
    kZero_StencilOp,
    kZero_StencilOp,
    kNonZeroIfInClip_StencilFunc,
    0xffff,
    0x0000,
    0xffff);

static constexpr GrStencilSettings gInvWindColorPass(
    kZero_StencilOp,
    kZero_StencilOp,
    kEqualIfInClip_StencilFunc,
    0xffff,
    0x0000,
    0xffff);

////// Normal render to stencil

// Sometimes the default path renderer can draw a path directly to the stencil
// buffer without having to first resolve the interior / exterior.
static constexpr GrStencilSettings gDirectToStencil(
    kZero_StencilOp,
    kIncClamp_StencilOp,
    kAlwaysIfInClip_StencilFunc,
    0xffff,
    0x0000,
    0xffff);

#endif
