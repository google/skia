/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBuiltInCodeSnippetID_DEFINED
#define SkBuiltInCodeSnippetID_DEFINED

#include "include/core/SkTypes.h"

// TODO: this needs to be expanded into a more flexible dictionary (esp. for user-supplied SkSL)
enum class SkBuiltInCodeSnippetID : uint8_t {
    // TODO: It seems like this requires some refinement. Fundamentally this doesn't seem like a
    // draw that originated from a PaintParams.
    kDepthStencilOnlyDraw,

    // SkShader code snippets
    kSolidColorShader,
    kLinearGradientShader,
    kRadialGradientShader,
    kSweepGradientShader,
    kConicalGradientShader,

    kImageShader,
    kBlendShader,     // aka ComposeShader

    // BlendMode code snippets
    kSimpleBlendMode,

    kLast = kSimpleBlendMode
};
static constexpr int kBuiltInCodeSnippetIDCount = static_cast<int>(SkBuiltInCodeSnippetID::kLast)+1;

#endif // SkBuiltInCodeSnippetID_DEFINED
