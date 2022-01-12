/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkPaintParamsKey.h"

#include <cstring>
#include "src/core/SkKeyHelpers.h"

bool SkPaintParamsKey::operator==(const SkPaintParamsKey& that) const {
    return fNumBytes == that.fNumBytes &&
           !memcmp(fData.data(), that.fData.data(), fNumBytes);
}

#ifdef SK_DEBUG
typedef void (*Dumper)(const SkPaintParamsKey&, uint32_t headerOffset);

static Dumper gDumpers[kCodeSnippetIDCount] = {
    DepthStencilOnlyBlock::Dump,  // kDepthStencilOnlyDraw

    SolidColorShaderBlock::Dump,  // kSolidColorShader
    GradientShaderBlocks::Dump,   // kLinearGradientShader
    GradientShaderBlocks::Dump,   // kRadialGradientShader
    GradientShaderBlocks::Dump,   // kSweepGradientShader
    GradientShaderBlocks::Dump,   // kConicalGradientShader

    BlendModeBlock::Dump,         // kSimpleBlendMode
};

// This just iterates over the top-level blocks calling block-specific dump methods.
void SkPaintParamsKey::dump() const {
    SkDebugf("SkPaintParamsKey %dB:\n", this->sizeInBytes());

    int curHeaderOffset = 0;
    while (curHeaderOffset < this->sizeInBytes()) {
        auto [codeSnippetID, blockSize] = this->readCodeSnippetID(curHeaderOffset);

        (gDumpers[(int) codeSnippetID])(*this, curHeaderOffset);

        curHeaderOffset += blockSize;
    }
}
#endif
