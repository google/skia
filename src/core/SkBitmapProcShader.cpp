/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcShader.h"
#include "SkBitmapProcState.h"
#include "SkBitmapProvider.h"
#include "SkColorPriv.h"
#include "SkErrorInternals.h"
#include "SkPixelRef.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "SkGrPriv.h"
#include "effects/GrBicubicEffect.h"
#include "effects/GrSimpleTextureEffect.h"
#endif

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

size_t SkBitmapProcShader::ContextSize(const ContextRec& rec, const SkImageInfo& srcInfo) {
    size_t size0 = sizeof(BitmapProcShaderContext) + sizeof(SkBitmapProcState);
    size_t size1 = sizeof(LinearPipelineContext) + sizeof(SkBitmapProcInfo);
    return SkTMax(size0, size1);
}

SkShader::Context* SkBitmapProcShader::MakeContext(const SkShader& shader,
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

SkShader::Context* SkBitmapProcShader::onCreateContext(const ContextRec& rec, void* storage) const {
    return MakeContext(*this, (TileMode)fTileModeX, (TileMode)fTileModeY,
                       SkBitmapProvider(fRawBitmap), rec, storage);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkBitmapProcShader::SkBitmapProcShader(const SkBitmap& src, TileMode tmx, TileMode tmy,
                                       const SkMatrix* localMatrix)
        : INHERITED(localMatrix) {
    fRawBitmap = src;
    fTileModeX = (uint8_t)tmx;
    fTileModeY = (uint8_t)tmy;
}

bool SkBitmapProcShader::onIsABitmap(SkBitmap* texture, SkMatrix* texM, TileMode xy[]) const {
    if (texture) {
        *texture = fRawBitmap;
    }
    if (texM) {
        texM->reset();
    }
    if (xy) {
        xy[0] = (TileMode)fTileModeX;
        xy[1] = (TileMode)fTileModeY;
    }
    return true;
}

sk_sp<SkFlattenable> SkBitmapProcShader::CreateProc(SkReadBuffer& buffer) {
    SkMatrix lm;
    buffer.readMatrix(&lm);
    SkBitmap bm;
    if (!buffer.readBitmap(&bm)) {
        return nullptr;
    }
    bm.setImmutable();
    TileMode mx = (TileMode)buffer.readUInt();
    TileMode my = (TileMode)buffer.readUInt();
    return SkShader::MakeBitmapShader(bm, mx, my, &lm);
}

void SkBitmapProcShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeMatrix(this->getLocalMatrix());
    buffer.writeBitmap(fRawBitmap);
    buffer.writeUInt(fTileModeX);
    buffer.writeUInt(fTileModeY);
}

bool SkBitmapProcShader::isOpaque() const {
    return fRawBitmap.isOpaque();
}

bool SkBitmapProcShader::BitmapIsTooBig(const SkBitmap& bm) {
    static const int kMaxSize = 65535;

    return bm.width() > kMaxSize || bm.height() > kMaxSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkUnPreMultiply.h"
#include "SkColorShader.h"
#include "SkEmptyShader.h"

// returns true and set color if the bitmap can be drawn as a single color
// (for efficiency)
static bool can_use_color_shader(const SkBitmap& bm, SkColor* color) {
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    // HWUI does not support color shaders (see b/22390304)
    return false;
#endif

    if (1 != bm.width() || 1 != bm.height()) {
        return false;
    }

    SkAutoLockPixels alp(bm);
    if (!bm.readyToDraw()) {
        return false;
    }

    switch (bm.colorType()) {
        case kN32_SkColorType:
            *color = SkUnPreMultiply::PMColorToColor(*bm.getAddr32(0, 0));
            return true;
        case kRGB_565_SkColorType:
            *color = SkPixel16ToColor(*bm.getAddr16(0, 0));
            return true;
        case kIndex_8_SkColorType:
            *color = SkUnPreMultiply::PMColorToColor(bm.getIndex8Color(0, 0));
            return true;
        default: // just skip the other configs for now
            break;
    }
    return false;
}

static bool bitmap_is_too_big(const SkBitmap& bm) {
    // SkBitmapProcShader stores bitmap coordinates in a 16bit buffer, as it
    // communicates between its matrix-proc and its sampler-proc. Until we can
    // widen that, we have to reject bitmaps that are larger.
    //
    static const int kMaxSize = 65535;

    return bm.width() > kMaxSize || bm.height() > kMaxSize;
}

sk_sp<SkShader> SkMakeBitmapShader(const SkBitmap& src, SkShader::TileMode tmx,
                                   SkShader::TileMode tmy, const SkMatrix* localMatrix,
                                   SkTBlitterAllocator* allocator) {
    SkShader* shader;
    SkColor color;
    if (src.isNull() || bitmap_is_too_big(src)) {
        if (nullptr == allocator) {
            shader = new SkEmptyShader;
        } else {
            shader = allocator->createT<SkEmptyShader>();
        }
    } else if (can_use_color_shader(src, &color)) {
        if (nullptr == allocator) {
            shader = new SkColorShader(color);
        } else {
            shader = allocator->createT<SkColorShader>(color);
        }
    } else {
        if (nullptr == allocator) {
            shader = new SkBitmapProcShader(src, tmx, tmy, localMatrix);
        } else {
            shader = allocator->createT<SkBitmapProcShader>(src, tmx, tmy, localMatrix);
        }
    }
    return sk_sp<SkShader>(shader);
}

///////////////////////////////////////////////////////////////////////////////

#ifndef SK_IGNORE_TO_STRING
void SkBitmapProcShader::toString(SkString* str) const {
    static const char* gTileModeName[SkShader::kTileModeCount] = {
        "clamp", "repeat", "mirror"
    };

    str->append("BitmapShader: (");

    str->appendf("(%s, %s)",
                 gTileModeName[fTileModeX],
                 gTileModeName[fTileModeY]);

    str->append(" ");
    fRawBitmap.toString(str);

    this->INHERITED::toString(str);

    str->append(")");
}
#endif

///////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrTextureAccess.h"
#include "SkGr.h"
#include "effects/GrSimpleTextureEffect.h"

sk_sp<GrFragmentProcessor> SkBitmapProcShader::asFragmentProcessor(GrContext* context,
                                             const SkMatrix& viewM, const SkMatrix* localMatrix,
                                             SkFilterQuality filterQuality,
                                             SkSourceGammaTreatment gammaTreatment) const {
    SkMatrix matrix;
    matrix.setIDiv(fRawBitmap.width(), fRawBitmap.height());

    SkMatrix lmInverse;
    if (!this->getLocalMatrix().invert(&lmInverse)) {
        return nullptr;
    }
    if (localMatrix) {
        SkMatrix inv;
        if (!localMatrix->invert(&inv)) {
            return nullptr;
        }
        lmInverse.postConcat(inv);
    }
    matrix.preConcat(lmInverse);

    SkShader::TileMode tm[] = {
        (TileMode)fTileModeX,
        (TileMode)fTileModeY,
    };

    // Must set wrap and filter on the sampler before requesting a texture. In two places below
    // we check the matrix scale factors to determine how to interpret the filter quality setting.
    // This completely ignores the complexity of the drawVertices case where explicit local coords
    // are provided by the caller.
    bool doBicubic;
    GrTextureParams::FilterMode textureFilterMode =
            GrSkFilterQualityToGrFilterMode(filterQuality, viewM, this->getLocalMatrix(),
                                            &doBicubic);
    GrTextureParams params(tm, textureFilterMode);
    SkAutoTUnref<GrTexture> texture(GrRefCachedBitmapTexture(context, fRawBitmap, params,
                                                             gammaTreatment));

    if (!texture) {
        SkErrorInternals::SetError( kInternalError_SkError,
                                    "Couldn't convert bitmap to texture.");
        return nullptr;
    }

    sk_sp<GrFragmentProcessor> inner;
    if (doBicubic) {
        inner = GrBicubicEffect::Make(texture, matrix, tm);
    } else {
        inner = GrSimpleTextureEffect::Make(texture, matrix, params);
    }

    if (kAlpha_8_SkColorType == fRawBitmap.colorType()) {
        return GrFragmentProcessor::MulOutputByInputUnpremulColor(std::move(inner));
    }
    return GrFragmentProcessor::MulOutputByInputAlpha(std::move(inner));
}

#endif
