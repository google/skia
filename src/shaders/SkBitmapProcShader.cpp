/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/SkBitmapProcShader.h"

#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPixmap.h"
#include "include/private/base/SkAssert.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkBitmapProcState.h"

#include <algorithm>
#include <cstdint>

enum class SkTileMode;

class BitmapProcShaderContext : public SkShaderBase::Context {
public:
    BitmapProcShaderContext(const SkShaderBase& shader, const SkShaderBase::ContextRec& rec,
                            SkBitmapProcState* state)
        : INHERITED(shader, rec)
        , fState(state)
        , fFlags(0)
    {
        if (fState->fPixmap.isOpaque() && (255 == this->getPaintAlpha())) {
            fFlags |= SkShaderBase::kOpaqueAlpha_Flag;
        }
    }

    uint32_t getFlags() const override { return fFlags; }

    void shadeSpan(int x, int y, SkPMColor dstC[], int count) override {
        const SkBitmapProcState& state = *fState;
        if (state.getShaderProc32()) {
            state.getShaderProc32()(&state, x, y, dstC, count);
            return;
        }

        const int BUF_MAX = 128;
        uint32_t buffer[BUF_MAX];
        SkBitmapProcState::MatrixProc   mproc = state.getMatrixProc();
        SkBitmapProcState::SampleProc32 sproc = state.getSampleProc32();
        const int max = state.maxCountForBufferSize(sizeof(buffer[0]) * BUF_MAX);

        SkASSERT(state.fPixmap.addr());

        for (;;) {
            int n = std::min(count, max);
            SkASSERT(n > 0 && n < BUF_MAX*2);
            mproc(state, buffer, n, x, y);
            sproc(state, buffer, n, dstC);

            if ((count -= n) == 0) {
                break;
            }
            SkASSERT(count > 0);
            x += n;
            dstC += n;
        }
    }

private:
    SkBitmapProcState*  fState;
    uint32_t            fFlags;

    using INHERITED = SkShaderBase::Context;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

SkShaderBase::Context* SkBitmapProcLegacyShader::MakeContext(
    const SkShaderBase& shader, SkTileMode tmx, SkTileMode tmy, const SkSamplingOptions& sampling,
    const SkImage_Base* image, const ContextRec& rec, SkArenaAlloc* alloc)
{
    SkMatrix totalInverse;
    // Do this first, so we know the matrix can be inverted.
    if (!rec.fMatrixRec.totalInverse(&totalInverse)) {
        return nullptr;
    }

    SkBitmapProcState* state = alloc->make<SkBitmapProcState>(image, tmx, tmy);
    if (!state->setup(totalInverse, rec.fPaintAlpha, sampling)) {
        return nullptr;
    }
    return alloc->make<BitmapProcShaderContext>(shader, rec, state);
}
