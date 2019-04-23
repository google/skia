/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/SkBitmapProcShader.h"

#include "include/private/SkArenaAlloc.h"
#include "src/core/SkBitmapProcState.h"
#include "src/core/SkBitmapProvider.h"
#include "src/core/SkXfermodePriv.h"

static bool only_scale_and_translate(const SkMatrix& matrix) {
    unsigned mask = SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask;
    return (matrix.getType() & ~mask) == 0;
}

class BitmapProcInfoContext : public SkShaderBase::Context {
public:
    // The info has been allocated elsewhere, but we are responsible for calling its destructor.
    BitmapProcInfoContext(const SkShaderBase& shader, const SkShaderBase::ContextRec& rec,
                            SkBitmapProcInfo* info)
        : INHERITED(shader, rec)
        , fInfo(info)
    {
        fFlags = 0;
        if (fInfo->fPixmap.isOpaque() && (255 == this->getPaintAlpha())) {
            fFlags |= SkShaderBase::kOpaqueAlpha_Flag;
        }

        if (1 == fInfo->fPixmap.height() && only_scale_and_translate(this->getTotalInverse())) {
            fFlags |= SkShaderBase::kConstInY32_Flag;
        }
    }

    uint32_t getFlags() const override { return fFlags; }

private:
    SkBitmapProcInfo*   fInfo;
    uint32_t            fFlags;

    typedef SkShaderBase::Context INHERITED;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class BitmapProcShaderContext : public BitmapProcInfoContext {
public:
    BitmapProcShaderContext(const SkShaderBase& shader, const SkShaderBase::ContextRec& rec,
                            SkBitmapProcState* state)
        : INHERITED(shader, rec, state)
        , fState(state)
    {}

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
            int n = SkTMin(count, max);
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

    typedef BitmapProcInfoContext INHERITED;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

SkShaderBase::Context* SkBitmapProcLegacyShader::MakeContext(
    const SkShaderBase& shader, SkTileMode tmx, SkTileMode tmy,
    const SkBitmapProvider& provider, const ContextRec& rec, SkArenaAlloc* alloc)
{
    SkMatrix totalInverse;
    // Do this first, so we know the matrix can be inverted.
    if (!shader.computeTotalInverse(*rec.fMatrix, rec.fLocalMatrix, &totalInverse)) {
        return nullptr;
    }

    SkBitmapProcState* state = alloc->make<SkBitmapProcState>(provider, tmx, tmy);
    if (!state->setup(totalInverse, *rec.fPaint)) {
        return nullptr;
    }
    return alloc->make<BitmapProcShaderContext>(shader, rec, state);
}
