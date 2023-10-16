/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_render_CommonDepthStencilSettings_DEFINED
#define skgpu_graphite_render_CommonDepthStencilSettings_DEFINED

#include "src/gpu/graphite/DrawTypes.h"

namespace skgpu::graphite {

/**
 * DepthStencilSettings reusable by RenderSteps that can shade directly in a single pass, using
 * GREATER or GEQUAL depth tests depending on if they allow self-intersections.
 */

static constexpr DepthStencilSettings kDirectDepthGreaterPass = {
        /*front=*/       {},
        /*back=*/        {},
        /*stencilRef=*/  0,
        /*stencilTest=*/ false,
        /*depthCompare=*/CompareOp::kGreater,
        /*depthTest=*/   true,
        /*depthWrite=*/  true
};

static constexpr DepthStencilSettings kDirectDepthGEqualPass = {
        /*front=*/       {},
        /*back=*/        {},
        /*stencilRef=*/  0,
        /*stencilTest=*/ false,
        /*depthCompare=*/CompareOp::kGEqual,
        /*depthTest=*/   true,
        /*depthWrite=*/  true
};

/**
 * "stencil" pass DepthStencilSettings reusable for RenderSteps following some form of
 * stencil-then-cover multi-pass algorithm.
 */

// Increments stencil value on clockwise triangles. Used for "winding" fill.
constexpr DepthStencilSettings::Face kIncrementCW = {
        /*stencilFail=*/   StencilOp::kKeep,
        /*depthFail=*/     StencilOp::kKeep,
        /*dsPass=*/        StencilOp::kIncWrap,
        /*compare=*/       CompareOp::kAlways,
        /*readMask=*/      0xffffffff,
        /*writeMask=*/     0xffffffff
};

// Decrements stencil value on counterclockwise triangles. Used for "winding" fill.
constexpr DepthStencilSettings::Face kDecrementCCW = {
        /*stencilFail=*/   StencilOp::kKeep,
        /*depthFail=*/     StencilOp::kKeep,
        /*dsPass=*/        StencilOp::kDecWrap,
        /*compare=*/       CompareOp::kAlways,
        /*readMask=*/      0xffffffff,
        /*writeMask=*/     0xffffffff
};

// Toggles the bottom stencil bit. Used for "even-odd" fill.
constexpr DepthStencilSettings::Face kToggle = {
        /*stencilFail=*/   StencilOp::kKeep,
        /*depthFail=*/     StencilOp::kKeep,
        /*dsPass=*/        StencilOp::kInvert,
        /*compare=*/       CompareOp::kAlways,
        /*readMask=*/      0xffffffff,
        /*writeMask=*/     0x00000001
};

// Stencil settings to use for a standard Redbook "stencil" pass corresponding to a "winding"
// fill rule (regular or inverse is selected by a follow-up pass).
constexpr DepthStencilSettings kWindingStencilPass = {
        /*front=*/       kIncrementCW,
        /*back=*/        kDecrementCCW,
        /*stencilRef=*/  0,
        /*stencilTest=*/ true,
        /*depthCompare=*/CompareOp::kGreater,
        /*depthTest=*/   true,
        /*depthWrite=*/  false // The depth write will be handled by the covering pass
};

// Stencil settings to use for a standard Redbook "stencil" pass corresponding to an "even-odd"
// fill rule (regular or inverse is selected by a follow-up pass).
constexpr DepthStencilSettings kEvenOddStencilPass = {
        /*front=*/       kToggle,
        /*back=*/        kToggle,
        /*stencilRef=*/  0,
        /*stencilTest=*/ true,
        /*depthCompare=*/CompareOp::kGreater,
        /*depthTest=*/   true,
        /*depthWrite=*/  false // The depth write will be handled by the covering pass
};

/**
 * "cover" pass DepthStencilSettings reusable for RenderSteps following some form of
 * stencil-then-cover multi-pass algorithm.
 */

// Resets non-zero bits to 0, passes when not zero. We set depthFail to kZero because if we
// encounter that case, the kNotEqual=0 stencil test passed, so it does need to be set back to 0
// and the dsPass op won't be run. In practice, since the stencil steps will fail the same depth
// test, the stencil value will likely not be non-zero, but best to be explicit.
constexpr DepthStencilSettings::Face kPassNonZero = {
        /*stencilFail=*/   StencilOp::kKeep,
        /*depthFail=*/     StencilOp::kZero,
        /*dsPass=*/        StencilOp::kZero,
        /*compare=*/       CompareOp::kNotEqual,
        /*readMask=*/      0xffffffff,
        /*writeMask=*/     0xffffffff
};

 // Resets non-zero bits to 0, passes when zero.
constexpr DepthStencilSettings::Face kPassZero = {
        /*stencilFail=*/   StencilOp::kZero,
        /*depthFail=*/     StencilOp::kKeep,
        /*dsPass=*/        StencilOp::kKeep,
        /*compare=*/       CompareOp::kEqual,
        /*readMask=*/      0xffffffff,
        /*writeMask=*/     0xffffffff
};

// Stencil settings to use for a standard Redbook "cover" pass for a regular fill, assuming that the
// stencil buffer has been modified by either kWindingStencilPass or kEvenOddStencilPass.
constexpr DepthStencilSettings kRegularCoverPass = {
        /*front=*/       kPassNonZero,
        /*back=*/        kPassNonZero,
        /*stencilRef=*/  0,
        /*stencilTest=*/ true,
        /*depthCompare=*/CompareOp::kGreater,
        /*depthTest=*/   true,
        /*depthWrite=*/  true
};

// Stencil settings to use for a standard Redbook "cover" pass for inverse fills, assuming that the
// stencil buffer has been modified by either kWindingStencilPass or kEvenOddStencilPass.
constexpr DepthStencilSettings kInverseCoverPass = {
        /*front=*/       kPassZero,
        /*back=*/        kPassZero,
        /*stencilRef=*/  0,
        /*stencilTest=*/ true,
        /*depthCompare=*/CompareOp::kGreater,
        /*depthTest=*/   true,
        /*depthWrite=*/  true
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_render_CommonDepthStencilSettings_DEFINED
