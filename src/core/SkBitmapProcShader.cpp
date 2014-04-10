
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
        case kPMColor_SkColorType:
    //        if (tx == ty && (kClamp_TileMode == tx || kRepeat_TileMode == tx))
                return true;
        default:
            break;
    }
    return false;
}

SkBitmapProcShader::SkBitmapProcShader(const SkBitmap& src,
                                       TileMode tmx, TileMode tmy) {
    fRawBitmap = src;
    fState.fTileModeX = (uint8_t)tmx;
    fState.fTileModeY = (uint8_t)tmy;
    fFlags = 0; // computed in setContext
}

SkBitmapProcShader::SkBitmapProcShader(SkReadBuffer& buffer)
        : INHERITED(buffer) {
    buffer.readBitmap(&fRawBitmap);
    fRawBitmap.setImmutable();
    fState.fTileModeX = buffer.readUInt();
    fState.fTileModeY = buffer.readUInt();
    fFlags = 0; // computed in setContext
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
        xy[0] = (TileMode)fState.fTileModeX;
        xy[1] = (TileMode)fState.fTileModeY;
    }
    return kDefault_BitmapType;
}

void SkBitmapProcShader::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);

    buffer.writeBitmap(fRawBitmap);
    buffer.writeUInt(fState.fTileModeX);
    buffer.writeUInt(fState.fTileModeY);
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

bool SkBitmapProcShader::setContext(const SkBitmap& device,
                                    const SkPaint& paint,
                                    const SkMatrix& matrix) {
    if (!fRawBitmap.getTexture() && !valid_for_drawing(fRawBitmap)) {
        return false;
    }

    // do this first, so we have a correct inverse matrix
    if (!this->INHERITED::setContext(device, paint, matrix)) {
        return false;
    }

    fState.fOrigBitmap = fRawBitmap;
    if (!fState.chooseProcs(this->getTotalInverse(), paint)) {
        this->INHERITED::endContext();
        return false;
    }

    const SkBitmap& bitmap = *fState.fBitmap;
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
        case kPMColor_SkColorType:
            if (bitmapIsOpaque) {
                flags |= kHasSpan16_Flag;
            }
            break;
        case kAlpha_8_SkColorType:
            break;  // never set kHasSpan16_Flag
        default:
            break;
    }

    if (paint.isDither() && bitmap.colorType() != kRGB_565_SkColorType) {
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
    return true;
}

void SkBitmapProcShader::endContext() {
    fState.endContext();
    this->INHERITED::endContext();
}

#define BUF_MAX     128

#define TEST_BUFFER_OVERRITEx

#ifdef TEST_BUFFER_OVERRITE
    #define TEST_BUFFER_EXTRA   32
    #define TEST_PATTERN    0x88888888
#else
    #define TEST_BUFFER_EXTRA   0
#endif

void SkBitmapProcShader::shadeSpan(int x, int y, SkPMColor dstC[], int count) {
    const SkBitmapProcState& state = fState;
    if (state.getShaderProc32()) {
        state.getShaderProc32()(state, x, y, dstC, count);
        return;
    }

    uint32_t buffer[BUF_MAX + TEST_BUFFER_EXTRA];
    SkBitmapProcState::MatrixProc   mproc = state.getMatrixProc();
    SkBitmapProcState::SampleProc32 sproc = state.getSampleProc32();
    int max = fState.maxCountForBufferSize(sizeof(buffer[0]) * BUF_MAX);

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

SkShader::ShadeProc SkBitmapProcShader::asAShadeProc(void** ctx) {
    if (fState.getShaderProc32()) {
        *ctx = &fState;
        return (ShadeProc)fState.getShaderProc32();
    }
    return NULL;
}

void SkBitmapProcShader::shadeSpan16(int x, int y, uint16_t dstC[], int count) {
    const SkBitmapProcState& state = fState;
    if (state.getShaderProc16()) {
        state.getShaderProc16()(state, x, y, dstC, count);
        return;
    }

    uint32_t buffer[BUF_MAX];
    SkBitmapProcState::MatrixProc   mproc = state.getMatrixProc();
    SkBitmapProcState::SampleProc16 sproc = state.getSampleProc16();
    int max = fState.maxCountForBufferSize(sizeof(buffer));

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
        case kPMColor_SkColorType:
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
        SkShader::TileMode tmy, SkTBlitterAllocator* allocator) {
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
            shader = SkNEW_ARGS(SkBitmapProcShader, (src, tmx, tmy));
        } else {
            shader = allocator->createT<SkBitmapProcShader>(src, tmx, tmy);
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
                 gTileModeName[fState.fTileModeX],
                 gTileModeName[fState.fTileModeY]);

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

// Note that this will return -1 if either matrix is perspective.
static SkScalar get_combined_min_stretch(const SkMatrix& viewMatrix, const SkMatrix& localMatrix) {
    if (localMatrix.isIdentity()) {
        return viewMatrix.getMinStretch();
    } else {
        SkMatrix combined;
        combined.setConcat(viewMatrix, localMatrix);
        return combined.getMinStretch();
    }
}

GrEffectRef* SkBitmapProcShader::asNewEffect(GrContext* context, const SkPaint& paint) const {
    SkMatrix matrix;
    matrix.setIDiv(fRawBitmap.width(), fRawBitmap.height());

    SkMatrix lmInverse;
    if (!this->getLocalMatrix().invert(&lmInverse)) {
        return NULL;
    }
    matrix.preConcat(lmInverse);

    SkShader::TileMode tm[] = {
        (TileMode)fState.fTileModeX,
        (TileMode)fState.fTileModeY,
    };

    // Must set wrap and filter on the sampler before requesting a texture. In two places below
    // we check the matrix scale factors to determine how to interpret the filter quality setting.
    // This completely ignores the complexity of the drawVertices case where explicit local coords
    // are provided by the caller.
    SkPaint::FilterLevel paintFilterLevel = paint.getFilterLevel();
    GrTextureParams::FilterMode textureFilterMode;
    switch(paintFilterLevel) {
        case SkPaint::kNone_FilterLevel:
            textureFilterMode = GrTextureParams::kNone_FilterMode;
            break;
        case SkPaint::kLow_FilterLevel:
            textureFilterMode = GrTextureParams::kBilerp_FilterMode;
            break;
        case SkPaint::kMedium_FilterLevel:
            if (get_combined_min_stretch(context->getMatrix(), this->getLocalMatrix()) <
                SK_Scalar1) {
                textureFilterMode = GrTextureParams::kMipMap_FilterMode;
            } else {
                // Don't trigger MIP level generation unnecessarily.
                textureFilterMode = GrTextureParams::kBilerp_FilterMode;
            }
            break;
        case SkPaint::kHigh_FilterLevel:
            // Minification can look bad with bicubic filtering.
            if (get_combined_min_stretch(context->getMatrix(), this->getLocalMatrix()) >=
                SK_Scalar1) {
                // fall back to no filtering here; we will install another shader that will do the
                // HQ filtering.
                textureFilterMode = GrTextureParams::kNone_FilterMode;
            } else {
                // Fall back to MIP-mapping.
                paintFilterLevel = SkPaint::kMedium_FilterLevel;
                textureFilterMode = GrTextureParams::kMipMap_FilterMode;
            }
            break;
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
        return NULL;
    }

    GrEffectRef* effect = NULL;
    if (paintFilterLevel == SkPaint::kHigh_FilterLevel) {
        effect = GrBicubicEffect::Create(texture, matrix, tm);
    } else {
        effect = GrSimpleTextureEffect::Create(texture, matrix, params);
    }
    GrUnlockAndUnrefCachedBitmapTexture(texture);
    return effect;
}
#endif
