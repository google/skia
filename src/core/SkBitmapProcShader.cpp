/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcShader.h"
#include "SkBitmapProcState.h"
#include "SkBitmapProvider.h"

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

    ~BitmapProcInfoContext() override {
        fInfo->~SkBitmapProcInfo();
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
#include "SkXfermode.h"

class LinearPipelineContext : public BitmapProcInfoContext {
public:
    LinearPipelineContext(const SkShader& shader, const SkShader::ContextRec& rec,
                          SkBitmapProcInfo* info)
        : INHERITED(shader, rec, info)
    {
        // Save things off in case we need to build a blitter pipeline.
        fSrcPixmap = info->fPixmap;
        fAlpha = SkColorGetA(info->fPaintColor) / 255.0f;
        fXMode = info->fTileModeX;
        fYMode = info->fTileModeY;
        fFilterQuality = info->fFilterQuality;
        fMatrixTypeMask = info->fRealInvMatrix.getType();

        fShaderPipeline.init(
            info->fRealInvMatrix, info->fFilterQuality,
            info->fTileModeX, info->fTileModeY,
            info->fPaintColor,
            info->fPixmap);

        // To implement the old shadeSpan entry-point, we need to efficiently convert our native
        // floats into SkPMColor. The SkXfermode::D32Procs do exactly that.
        //
        sk_sp<SkXfermode> xfer(SkXfermode::Make(SkXfermode::kSrc_Mode));
        fXferProc = SkXfermode::GetD32Proc(xfer.get(), 0);
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
            fXferProc(nullptr, dstC, tmp, n, nullptr);
            dstC += n;
            x += n;
            count -= n;
        }
    }

    bool onChooseBlitProcs(const SkImageInfo& dstInfo, BlitState* state) override {
        SkXfermode::Mode mode;
        if (!SkXfermode::AsMode(state->fXfer, &mode)) { return false; }

        if (SkLinearBitmapPipeline::ClonePipelineForBlitting(
            &fBlitterPipeline, *fShaderPipeline,
            fMatrixTypeMask,
            fXMode, fYMode,
            fFilterQuality, fSrcPixmap,
            fAlpha, mode, dstInfo))
        {
            state->fStorage[0] = fBlitterPipeline.get();
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
    SkEmbeddableLinearPipeline fShaderPipeline;
    SkEmbeddableLinearPipeline fBlitterPipeline;
    SkXfermode::D32Proc        fXferProc;
    SkPixmap                   fSrcPixmap;
    float                      fAlpha;
    SkShader::TileMode         fXMode;
    SkShader::TileMode         fYMode;
    SkMatrix::TypeMask         fMatrixTypeMask;
    SkFilterQuality            fFilterQuality;

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
    return SkTMax(size0, size1);
}

SkShader::Context* SkBitmapProcLegacyShader::MakeContext(const SkShader& shader,
                                                   TileMode tmx, TileMode tmy,
                                                   const SkBitmapProvider& provider,
                                                   const ContextRec& rec, void* storage) {
    SkMatrix totalInverse;
    // Do this first, so we know the matrix can be inverted.
    if (!shader.computeTotalInverse(rec, &totalInverse)) {
        return nullptr;
    }

    // Decide if we can/want to use the new linear pipeline
    bool useLinearPipeline = choose_linear_pipeline(rec, provider.info());
    SkSourceGammaTreatment treatment = SkMipMap::DeduceTreatment(rec);

    if (useLinearPipeline) {
        void* infoStorage = (char*)storage + sizeof(LinearPipelineContext);
        SkBitmapProcInfo* info = new (infoStorage) SkBitmapProcInfo(provider, tmx, tmy, treatment);
        if (!info->init(totalInverse, *rec.fPaint)) {
            info->~SkBitmapProcInfo();
            return nullptr;
        }

        return new (storage) LinearPipelineContext(shader, rec, info);
    } else {
        void* stateStorage = (char*)storage + sizeof(BitmapProcShaderContext);
        SkBitmapProcState* state = new (stateStorage) SkBitmapProcState(provider, tmx, tmy,
                                                                        treatment);
        if (!state->setup(totalInverse, *rec.fPaint)) {
            state->~SkBitmapProcState();
            return nullptr;
        }
        return new (storage) BitmapProcShaderContext(shader, rec, state);
    }
}
