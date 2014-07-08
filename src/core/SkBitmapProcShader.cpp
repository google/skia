
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkColorPriv.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkPixelRef.h"
#include "SkErrorInternals.h"
#include "SkBitmapProcShader.h"

#if SK_SUPPORT_GPU
#include "effects/GrSimpleTextureEffect.h"
#include "effects/GrBicubicEffect.h"
#endif

bool SkBitmapProcShader::CanDo(const SkBitmap& bm, TileMode tx, TileMode ty) {
    switch (bm.colorType()) {
        case kAlpha_8_SkColorType:
        case kRGB_565_SkColorType:
        case kIndex_8_SkColorType:
        case kN32_SkColorType:
    //        if (tx == ty && (kClamp_TileMode == tx || kRepeat_TileMode == tx))
                return true;
        default:
            break;
    }
    return false;
}

SkBitmapProcShader::SkBitmapProcShader(const SkBitmap& src, TileMode tmx, TileMode tmy,
                                       const SkMatrix* localMatrix)
        : INHERITED(localMatrix) {
    fRawBitmap = src;
    fTileModeX = (uint8_t)tmx;
    fTileModeY = (uint8_t)tmy;
}

SkBitmapProcShader::SkBitmapProcShader(SkReadBuffer& buffer)
        : INHERITED(buffer) {
    buffer.readBitmap(&fRawBitmap);
    fRawBitmap.setImmutable();
    fTileModeX = buffer.readUInt();
    fTileModeY = buffer.readUInt();
}

SkShader::BitmapType SkBitmapProcShader::asABitmap(SkBitmap* texture,
                                                   SkMatrix* texM,
                                                   TileMode xy[]) const {
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
    return kDefault_BitmapType;
}

void SkBitmapProcShader::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);

    buffer.writeBitmap(fRawBitmap);
    buffer.writeUInt(fTileModeX);
    buffer.writeUInt(fTileModeY);
}

static bool only_scale_and_translate(const SkMatrix& matrix) {
    unsigned mask = SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask;
    return (matrix.getType() & ~mask) == 0;
}

bool SkBitmapProcShader::isOpaque() const {
    return fRawBitmap.isOpaque();
}

static bool valid_for_drawing(const SkBitmap& bm) {
    if (0 == bm.width() || 0 == bm.height()) {
        return false;   // nothing to draw
    }
    if (NULL == bm.pixelRef()) {
        return false;   // no pixels to read
    }
    if (kIndex_8_SkColorType == bm.colorType()) {
        // ugh, I have to lock-pixels to inspect the colortable
        SkAutoLockPixels alp(bm);
        if (!bm.getColorTable()) {
            return false;
        }
    }
    return true;
}

SkShader::Context* SkBitmapProcShader::onCreateContext(const ContextRec& rec, void* storage) const {
    if (!fRawBitmap.getTexture() && !valid_for_drawing(fRawBitmap)) {
        return NULL;
    }

    SkMatrix totalInverse;
    // Do this first, so we know the matrix can be inverted.
    if (!this->computeTotalInverse(rec, &totalInverse)) {
        return NULL;
    }

    void* stateStorage = (char*)storage + sizeof(BitmapProcShaderContext);
    SkBitmapProcState* state = SkNEW_PLACEMENT(stateStorage, SkBitmapProcState);

    SkASSERT(state);
    state->fTileModeX = fTileModeX;
    state->fTileModeY = fTileModeY;
    state->fOrigBitmap = fRawBitmap;
    if (!state->chooseProcs(totalInverse, *rec.fPaint)) {
        state->~SkBitmapProcState();
        return NULL;
    }

    return SkNEW_PLACEMENT_ARGS(storage, BitmapProcShaderContext, (*this, rec, state));
}

size_t SkBitmapProcShader::contextSize() const {
    // The SkBitmapProcState is stored outside of the context object, with the context holding
    // a pointer to it.
    return sizeof(BitmapProcShaderContext) + sizeof(SkBitmapProcState);
}

SkBitmapProcShader::BitmapProcShaderContext::BitmapProcShaderContext(
        const SkBitmapProcShader& shader, const ContextRec& rec, SkBitmapProcState* state)
    : INHERITED(shader, rec)
    , fState(state)
{
    const SkBitmap& bitmap = *fState->fBitmap;
    bool bitmapIsOpaque = bitmap.isOpaque();

    // update fFlags
    uint32_t flags = 0;
    if (bitmapIsOpaque && (255 == this->getPaintAlpha())) {
        flags |= kOpaqueAlpha_Flag;
    }

    switch (bitmap.colorType()) {
        case kRGB_565_SkColorType:
            flags |= (kHasSpan16_Flag | kIntrinsicly16_Flag);
            break;
        case kIndex_8_SkColorType:
        case kN32_SkColorType:
            if (bitmapIsOpaque) {
                flags |= kHasSpan16_Flag;
            }
            break;
        case kAlpha_8_SkColorType:
            break;  // never set kHasSpan16_Flag
        default:
            break;
    }

    if (rec.fPaint->isDither() && bitmap.colorType() != kRGB_565_SkColorType) {
        // gradients can auto-dither in their 16bit sampler, but we don't so
        // we clear the flag here.
        flags &= ~kHasSpan16_Flag;
    }

    // if we're only 1-pixel high, and we don't rotate, then we can claim this
    if (1 == bitmap.height() &&
            only_scale_and_translate(this->getTotalInverse())) {
        flags |= kConstInY32_Flag;
        if (flags & kHasSpan16_Flag) {
            flags |= kConstInY16_Flag;
        }
    }

    fFlags = flags;
}

SkBitmapProcShader::BitmapProcShaderContext::~BitmapProcShaderContext() {
    // The bitmap proc state has been created outside of the context on memory that will be freed
    // elsewhere. Only call the destructor but leave the freeing of the memory to the caller.
    fState->~SkBitmapProcState();
}

#define BUF_MAX     128

#define TEST_BUFFER_OVERRITEx

#ifdef TEST_BUFFER_OVERRITE
    #define TEST_BUFFER_EXTRA   32
    #define TEST_PATTERN    0x88888888
#else
    #define TEST_BUFFER_EXTRA   0
#endif

void SkBitmapProcShader::BitmapProcShaderContext::shadeSpan(int x, int y, SkPMColor dstC[],
                                                            int count) {
    const SkBitmapProcState& state = *fState;
    if (state.getShaderProc32()) {
        state.getShaderProc32()(state, x, y, dstC, count);
        return;
    }

    uint32_t buffer[BUF_MAX + TEST_BUFFER_EXTRA];
    SkBitmapProcState::MatrixProc   mproc = state.getMatrixProc();
    SkBitmapProcState::SampleProc32 sproc = state.getSampleProc32();
    int max = state.maxCountForBufferSize(sizeof(buffer[0]) * BUF_MAX);

    SkASSERT(state.fBitmap->getPixels());
    SkASSERT(state.fBitmap->pixelRef() == NULL ||
             state.fBitmap->pixelRef()->isLocked());

    for (;;) {
        int n = count;
        if (n > max) {
            n = max;
        }
        SkASSERT(n > 0 && n < BUF_MAX*2);
#ifdef TEST_BUFFER_OVERRITE
        for (int i = 0; i < TEST_BUFFER_EXTRA; i++) {
            buffer[BUF_MAX + i] = TEST_PATTERN;
        }
#endif
        mproc(state, buffer, n, x, y);
#ifdef TEST_BUFFER_OVERRITE
        for (int j = 0; j < TEST_BUFFER_EXTRA; j++) {
            SkASSERT(buffer[BUF_MAX + j] == TEST_PATTERN);
        }
#endif
        sproc(state, buffer, n, dstC);

        if ((count -= n) == 0) {
            break;
        }
        SkASSERT(count > 0);
        x += n;
        dstC += n;
    }
}

SkShader::Context::ShadeProc SkBitmapProcShader::BitmapProcShaderContext::asAShadeProc(void** ctx) {
    if (fState->getShaderProc32()) {
        *ctx = fState;
        return (ShadeProc)fState->getShaderProc32();
    }
    return NULL;
}

void SkBitmapProcShader::BitmapProcShaderContext::shadeSpan16(int x, int y, uint16_t dstC[],
                                                              int count) {
    const SkBitmapProcState& state = *fState;
    if (state.getShaderProc16()) {
        state.getShaderProc16()(state, x, y, dstC, count);
        return;
    }

    uint32_t buffer[BUF_MAX];
    SkBitmapProcState::MatrixProc   mproc = state.getMatrixProc();
    SkBitmapProcState::SampleProc16 sproc = state.getSampleProc16();
    int max = state.maxCountForBufferSize(sizeof(buffer));

    SkASSERT(state.fBitmap->getPixels());
    SkASSERT(state.fBitmap->pixelRef() == NULL ||
             state.fBitmap->pixelRef()->isLocked());

    for (;;) {
        int n = count;
        if (n > max) {
            n = max;
        }
        mproc(state, buffer, n, x, y);
        sproc(state, buffer, n, dstC);

        if ((count -= n) == 0) {
            break;
        }
        x += n;
        dstC += n;
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "SkUnPreMultiply.h"
#include "SkColorShader.h"
#include "SkEmptyShader.h"

// returns true and set color if the bitmap can be drawn as a single color
// (for efficiency)
static bool canUseColorShader(const SkBitmap& bm, SkColor* color) {
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

static bool bitmapIsTooBig(const SkBitmap& bm) {
    // SkBitmapProcShader stores bitmap coordinates in a 16bit buffer, as it
    // communicates between its matrix-proc and its sampler-proc. Until we can
    // widen that, we have to reject bitmaps that are larger.
    //
    const int maxSize = 65535;

    return bm.width() > maxSize || bm.height() > maxSize;
}

SkShader* CreateBitmapShader(const SkBitmap& src, SkShader::TileMode tmx,
        SkShader::TileMode tmy, const SkMatrix* localMatrix, SkTBlitterAllocator* allocator) {
    SkShader* shader;
    SkColor color;
    if (src.isNull() || bitmapIsTooBig(src)) {
        if (NULL == allocator) {
            shader = SkNEW(SkEmptyShader);
        } else {
            shader = allocator->createT<SkEmptyShader>();
        }
    }
    else if (canUseColorShader(src, &color)) {
        if (NULL == allocator) {
            shader = SkNEW_ARGS(SkColorShader, (color));
        } else {
            shader = allocator->createT<SkColorShader>(color);
        }
    } else {
        if (NULL == allocator) {
            shader = SkNEW_ARGS(SkBitmapProcShader, (src, tmx, tmy, localMatrix));
        } else {
            shader = allocator->createT<SkBitmapProcShader>(src, tmx, tmy, localMatrix);
        }
    }
    return shader;
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
#include "effects/GrSimpleTextureEffect.h"
#include "SkGr.h"

bool SkBitmapProcShader::asNewEffect(GrContext* context, const SkPaint& paint,
                                     const SkMatrix* localMatrix, GrColor* paintColor,
                                     GrEffect** effect) const {
    SkMatrix matrix;
    matrix.setIDiv(fRawBitmap.width(), fRawBitmap.height());

    SkMatrix lmInverse;
    if (!this->getLocalMatrix().invert(&lmInverse)) {
        return false;
    }
    if (localMatrix) {
        SkMatrix inv;
        if (!localMatrix->invert(&inv)) {
            return false;
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
    bool useBicubic = false;
    GrTextureParams::FilterMode textureFilterMode;
    switch(paint.getFilterLevel()) {
        case SkPaint::kNone_FilterLevel:
            textureFilterMode = GrTextureParams::kNone_FilterMode;
            break;
        case SkPaint::kLow_FilterLevel:
            textureFilterMode = GrTextureParams::kBilerp_FilterMode;
            break;
        case SkPaint::kMedium_FilterLevel: {
            SkMatrix matrix;
            matrix.setConcat(context->getMatrix(), this->getLocalMatrix());
            if (matrix.getMinScale() < SK_Scalar1) {
                textureFilterMode = GrTextureParams::kMipMap_FilterMode;
            } else {
                // Don't trigger MIP level generation unnecessarily.
                textureFilterMode = GrTextureParams::kBilerp_FilterMode;
            }
            break;
        }
        case SkPaint::kHigh_FilterLevel: {
            SkMatrix matrix;
            matrix.setConcat(context->getMatrix(), this->getLocalMatrix());
            useBicubic = GrBicubicEffect::ShouldUseBicubic(matrix, &textureFilterMode);
            break;
        }
        default:
            SkErrorInternals::SetError( kInvalidPaint_SkError,
                                        "Sorry, I don't understand the filtering "
                                        "mode you asked for.  Falling back to "
                                        "MIPMaps.");
            textureFilterMode = GrTextureParams::kMipMap_FilterMode;
            break;

    }
    GrTextureParams params(tm, textureFilterMode);
    GrTexture* texture = GrLockAndRefCachedBitmapTexture(context, fRawBitmap, &params);

    if (NULL == texture) {
        SkErrorInternals::SetError( kInternalError_SkError,
                                    "Couldn't convert bitmap to texture.");
        return false;
    }
    
    *paintColor = (kAlpha_8_SkColorType == fRawBitmap.colorType()) ?
                                                SkColor2GrColor(paint.getColor()) :
                                                SkColor2GrColorJustAlpha(paint.getColor());

    if (useBicubic) {
        *effect = GrBicubicEffect::Create(texture, matrix, tm);
    } else {
        *effect = GrSimpleTextureEffect::Create(texture, matrix, params);
    }
    GrUnlockAndUnrefCachedBitmapTexture(texture);

    return true;
}

#else 

bool SkBitmapProcShader::asNewEffect(GrContext* context, const SkPaint& paint,
                                     const SkMatrix* localMatrix, GrColor* paintColor,
                                     GrEffect** effect) const {
    SkDEBUGFAIL("Should not call in GPU-less build");
    return false;
}

#endif
