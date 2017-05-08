/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcShader.h"

#include "SkArenaAlloc.h"
#include "SkBitmapProcState.h"
#include "SkBitmapProvider.h"
#include "SkXfermodePriv.h"

static bool only_scale_and_translate(const SkMatrix& matrix) {
    unsigned mask = SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask;
    return (matrix.getType() & ~mask) == 0;
}

class BitmapProcInfoContext : public SkShader::Context {
public:
    // The info has been allocated elsewhere, but we are responsible for calling its destructor.
    BitmapProcInfoContext(const SkShader& shader, const SkShader::ContextRec& rec,
                            SkBitmapProcInfo* info)
        : INHERITED(shader, rec)
        , fInfo(info)
    {
        fFlags = 0;
        if (fInfo->fPixmap.isOpaque() && (255 == this->getPaintAlpha())) {
            fFlags |= SkShader::kOpaqueAlpha_Flag;
        }

        if (1 == fInfo->fPixmap.height() && only_scale_and_translate(this->getTotalInverse())) {
            fFlags |= SkShader::kConstInY32_Flag;
        }
    }

    uint32_t getFlags() const override { return fFlags; }

private:
    SkBitmapProcInfo*   fInfo;
    uint32_t            fFlags;

    typedef SkShader::Context INHERITED;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class BitmapProcShaderContext : public BitmapProcInfoContext {
public:
    BitmapProcShaderContext(const SkShader& shader, const SkShader::ContextRec& rec,
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

    ShadeProc asAShadeProc(void** ctx) override {
        if (fState->getShaderProc32()) {
            *ctx = fState;
            return (ShadeProc)fState->getShaderProc32();
        }
        return nullptr;
    }

private:
    SkBitmapProcState*  fState;

    typedef BitmapProcInfoContext INHERITED;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "SkLinearBitmapPipeline.h"
#include "SkPM4f.h"

class LinearPipelineContext : public BitmapProcInfoContext {
public:
    LinearPipelineContext(const SkShader& shader, const SkShader::ContextRec& rec,
                          SkBitmapProcInfo* info, SkArenaAlloc* alloc)
        : INHERITED(shader, rec, info), fAllocator{alloc}
    {
        // Save things off in case we need to build a blitter pipeline.
        fSrcPixmap = info->fPixmap;
        fAlpha = SkColorGetA(info->fPaintColor) / 255.0f;
        fFilterQuality = info->fFilterQuality;
        fMatrixTypeMask = info->fRealInvMatrix.getType();

        fShaderPipeline = alloc->make<SkLinearBitmapPipeline>(
            info->fRealInvMatrix, info->fFilterQuality,
            info->fTileModeX, info->fTileModeY,
            info->fPaintColor,
            info->fPixmap,
            fAllocator);

        // To implement the old shadeSpan entry-point, we need to efficiently convert our native
        // floats into SkPMColor. The SkXfermode::D32Procs do exactly that.
        //
        fSrcModeProc = SkXfermode::GetD32Proc(SkBlendMode::kSrc, 0);
    }

    void shadeSpan4f(int x, int y, SkPM4f dstC[], int count) override {
        fShaderPipeline->shadeSpan4f(x, y, dstC, count);
    }

    void shadeSpan(int x, int y, SkPMColor dstC[], int count) override {
        const int N = 128;
        SkPM4f  tmp[N];

        while (count > 0) {
            const int n = SkTMin(count, N);
            fShaderPipeline->shadeSpan4f(x, y, tmp, n);
            fSrcModeProc(SkBlendMode::kSrc, dstC, tmp, n, nullptr);
            dstC += n;
            x += n;
            count -= n;
        }
    }

    bool onChooseBlitProcs(const SkImageInfo& dstInfo, BlitState* state) override {
        if ((fBlitterPipeline = SkLinearBitmapPipeline::ClonePipelineForBlitting(
            *fShaderPipeline,
            fMatrixTypeMask,
            fFilterQuality, fSrcPixmap,
            fAlpha, state->fMode, dstInfo, fAllocator)))
        {
            state->fStorage[0] = fBlitterPipeline;
            state->fBlitBW = &LinearPipelineContext::ForwardToPipeline;

            return true;
        }

        return false;
    }

    static void ForwardToPipeline(BlitState* state, int x, int y, const SkPixmap& dst, int count) {
        SkLinearBitmapPipeline* pipeline = static_cast<SkLinearBitmapPipeline*>(state->fStorage[0]);
        void* addr = dst.writable_addr32(x, y);
        pipeline->blitSpan(x, y, addr, count);
    }

private:
    // Store the allocator from the context creation incase we are asked to build a blitter.
    SkArenaAlloc*           fAllocator;
    SkLinearBitmapPipeline* fShaderPipeline;
    SkLinearBitmapPipeline* fBlitterPipeline;
    SkXfermode::D32Proc     fSrcModeProc;
    SkPixmap                fSrcPixmap;
    float                   fAlpha;
    SkMatrix::TypeMask      fMatrixTypeMask;
    SkFilterQuality         fFilterQuality;

    typedef BitmapProcInfoContext INHERITED;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

static bool choose_linear_pipeline(const SkShader::ContextRec& rec, const SkImageInfo& srcInfo) {
    // If we get here, we can reasonably use either context, respect the caller's preference
    //
    bool needsPremul = srcInfo.alphaType() == kUnpremul_SkAlphaType;
    bool needsSwizzle = srcInfo.bytesPerPixel() == 4 && srcInfo.colorType() != kN32_SkColorType;
    return SkShader::ContextRec::kPM4f_DstType == rec.fPreferredDstType
           || needsPremul || needsSwizzle;
}

size_t SkBitmapProcLegacyShader::ContextSize(const ContextRec& rec, const SkImageInfo& srcInfo) {
    size_t size0 = sizeof(BitmapProcShaderContext) + sizeof(SkBitmapProcState);
    size_t size1 = sizeof(LinearPipelineContext) + sizeof(SkBitmapProcInfo);
    size_t s = SkTMax(size0, size1);
    return s;
}

SkShader::Context* SkBitmapProcLegacyShader::MakeContext(
    const SkShader& shader, TileMode tmx, TileMode tmy,
    const SkBitmapProvider& provider, const ContextRec& rec, SkArenaAlloc* alloc)
{
    SkMatrix totalInverse;
    // Do this first, so we know the matrix can be inverted.
    if (!shader.computeTotalInverse(rec, &totalInverse)) {
        return nullptr;
    }

    // Decide if we can/want to use the new linear pipeline
    bool useLinearPipeline = choose_linear_pipeline(rec, provider.info());

    if (useLinearPipeline) {
        SkBitmapProcInfo* info = alloc->make<SkBitmapProcInfo>(provider, tmx, tmy);
        if (!info->init(totalInverse, *rec.fPaint)) {
            return nullptr;
        }

        return alloc->make<LinearPipelineContext>(shader, rec, info, alloc);
    } else {
        SkBitmapProcState* state = alloc->make<SkBitmapProcState>(provider, tmx, tmy);
        if (!state->setup(totalInverse, *rec.fPaint)) {
            return nullptr;
        }
        return alloc->make<BitmapProcShaderContext>(shader, rec, state);
    }
}
