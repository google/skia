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
 * "stencil" pass DepthStencilSettings reusable for RenderSteps following some form of
 * stencil-then-cover multi-pass algorithm.
 */

// Increments stencil value on clockwise triangles. Used for "winding" fill.
constexpr StencilSettings::Face kIncrementCW = {
        /*stencilFail=*/   StencilOp::kKeep,
        /*depthFail=*/     StencilOp::kKeep,
        /*dsPass=*/        StencilOp::kIncWrap,
        /*compare=*/       CompareOp::kAlways
};

// Decrements stencil value on counterclockwise triangles. Used for "winding" fill.
constexpr StencilSettings::Face kDecrementCCW = {
        /*stencilFail=*/   StencilOp::kKeep,
        /*depthFail=*/     StencilOp::kKeep,
        /*dsPass=*/        StencilOp::kDecWrap,
        /*compare=*/       CompareOp::kAlways
};

// Increments the stencil value whenever a triangle is encountered. Used for storing a stroked
// path in the stencil buffer.
constexpr StencilSettings::Face kIncrementAlways = {
        /*stencilFail=*/   StencilOp::kKeep,
        /*depthFail=*/     StencilOp::kKeep,
        /*dsPass=*/        StencilOp::kIncClamp,
        /*compare=*/       CompareOp::kAlways
};

// Toggles the bottom stencil bit. Used for "even-odd" fill.
constexpr StencilSettings::Face kToggle = {
        /*stencilFail=*/   StencilOp::kKeep,
        /*depthFail=*/     StencilOp::kKeep,
        /*dsPass=*/        StencilOp::kInvert,
        /*compare=*/       CompareOp::kAlways
};

// Stencil settings to use for a standard Redbook "stencil" pass corresponding to a "winding"
// fill rule (regular or inverse is selected by a follow-up pass).
constexpr StencilSettings kWindingStencilPass = {
        /*front=*/           kIncrementCW,
        /*back=*/            kDecrementCCW,
        /*stencilRef=*/      0,
        /*stencilReadMask=*/ 0xffffffff,
        /*stencilWriteMask=*/0xffffffff
};

// Stencil settings to use for a standard Redbook "stencil" pass corresponding to an "even-odd"
// fill rule (regular or inverse is selected by a follow-up pass).
constexpr StencilSettings kEvenOddStencilPass = {
        /*front=*/           kToggle,
        /*back=*/            kToggle,
        /*stencilRef=*/      0,
        /*stencilReadMask=*/ 0xffffffff,
        /*stencilWriteMask=*/0x00000001
};

// Stencil settings to use for always stenciling out any triangle encountered, useful for stroked
// paths.
constexpr StencilSettings kIncrementStencilPass = {
        /*front=*/           kIncrementAlways,
        /*back=*/            kIncrementAlways,
        /*stencilRef=*/      0,
        /*stencilReadMask=*/ 0xffffffff,
        /*stencilWriteMask=*/0xffffffff
};

/**
 * "cover" pass DepthStencilSettings reusable for RenderSteps following some form of
 * stencil-then-cover multi-pass algorithm.
 */

// Resets non-zero bits to 0, passes when not zero. We set depthFail to kZero because if we
// encounter that case, the kNotEqual=0 stencil test passed, so it does need to be set back to 0
// and the dsPass op won't be run. In practice, since the stencil steps will fail the same depth
// test, the stencil value will likely not be non-zero, but best to be explicit.
constexpr StencilSettings::Face kPassNonZero = {
        /*stencilFail=*/   StencilOp::kKeep,
        /*depthFail=*/     StencilOp::kZero,
        /*dsPass=*/        StencilOp::kZero,
        /*compare=*/       CompareOp::kNotEqual
};

 // Resets non-zero bits to 0, passes when zero.
constexpr StencilSettings::Face kPassZero = {
        /*stencilFail=*/   StencilOp::kZero,
        /*depthFail=*/     StencilOp::kKeep,
        /*dsPass=*/        StencilOp::kKeep,
        /*compare=*/       CompareOp::kEqual
};

// Stencil settings to use for a standard Redbook "cover" pass for a regular fill, assuming that the
// stencil buffer has been modified by either kWindingStencilPass or kEvenOddStencilPass.
constexpr StencilSettings kRegularCoverPass = {
        /*front=*/           kPassNonZero,
        /*back=*/            kPassNonZero,
        /*stencilRef=*/      0,
        /*stencilReadMask=*/ 0xffffffff,
        /*stencilWriteMask=*/0xffffffff
};

// Stencil settings to use for a standard Redbook "cover" pass for inverse fills, assuming that the
// stencil buffer has been modified by either kWindingStencilPass or kEvenOddStencilPass.
constexpr StencilSettings kInverseCoverPass = {
        /*front=*/           kPassZero,
        /*back=*/            kPassZero,
        /*stencilRef=*/      0,
        /*stencilReadMask=*/ 0xffffffff,
        /*stencilWriteMask=*/0xffffffff
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_render_CommonDepthStencilSettings_DEFINED
