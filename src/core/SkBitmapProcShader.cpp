#include "SkBitmapProcShader.h"
#include "SkColorPriv.h"
#include "SkPixelRef.h"

bool SkBitmapProcShader::CanDo(const SkBitmap& bm, TileMode tx, TileMode ty) {
    switch (bm.config()) {
        case SkBitmap::kA8_Config:
        case SkBitmap::kRGB_565_Config:
        case SkBitmap::kIndex8_Config:
        case SkBitmap::kARGB_8888_Config:
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

SkBitmapProcShader::SkBitmapProcShader(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {
    fRawBitmap.unflatten(buffer);
    fState.fTileModeX = buffer.readU8();
    fState.fTileModeY = buffer.readU8();
    fFlags = 0; // computed in setContext
}

void SkBitmapProcShader::beginSession() {
    this->INHERITED::beginSession();

    fRawBitmap.lockPixels();
}

void SkBitmapProcShader::endSession() {
    fRawBitmap.unlockPixels();

    this->INHERITED::endSession();
}

SkShader::BitmapType SkBitmapProcShader::asABitmap(SkBitmap* texture,
                                                   SkMatrix* texM,
                                                   TileMode xy[],
                                       SkScalar* twoPointRadialParams) const {
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

void SkBitmapProcShader::flatten(SkFlattenableWriteBuffer& buffer) {
    this->INHERITED::flatten(buffer);

    fRawBitmap.flatten(buffer);
    buffer.write8(fState.fTileModeX);
    buffer.write8(fState.fTileModeY);
}

static bool only_scale_and_translate(const SkMatrix& matrix) {
    unsigned mask = SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask;
    return (matrix.getType() & ~mask) == 0;
}

bool SkBitmapProcShader::setContext(const SkBitmap& device,
                                    const SkPaint& paint,
                                    const SkMatrix& matrix) {
    // do this first, so we have a correct inverse matrix
    if (!this->INHERITED::setContext(device, paint, matrix)) {
        return false;
    }

    fState.fOrigBitmap = fRawBitmap;
    fState.fOrigBitmap.lockPixels();
    if (!fState.fOrigBitmap.getTexture() && !fState.fOrigBitmap.readyToDraw()) {
        fState.fOrigBitmap.unlockPixels();
        return false;
    }

    if (!fState.chooseProcs(this->getTotalInverse(), paint)) {
        return false;
    }

    const SkBitmap& bitmap = *fState.fBitmap;
    bool bitmapIsOpaque = bitmap.isOpaque();

    // update fFlags
    uint32_t flags = 0;
    if (bitmapIsOpaque && (255 == this->getPaintAlpha())) {
        flags |= kOpaqueAlpha_Flag;
    }

    switch (bitmap.config()) {
        case SkBitmap::kRGB_565_Config:
            flags |= (kHasSpan16_Flag | kIntrinsicly16_Flag);
            break;
        case SkBitmap::kIndex8_Config:
        case SkBitmap::kARGB_8888_Config:
            if (bitmapIsOpaque) {
                flags |= kHasSpan16_Flag;
            }
            break;
        case SkBitmap::kA8_Config:
            break;  // never set kHasSpan16_Flag
        default:
            break;
    }

    if (paint.isDither() && bitmap.config() != SkBitmap::kRGB_565_Config) {
        // gradients can auto-dither in their 16bit sampler, but we don't so
        // we clear the flag here.
        flags &= ~kHasSpan16_Flag;
    }

    // if we're only 1-pixel heigh, and we don't rotate, then we can claim this
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
    if (state.fShaderProc32) {
        state.fShaderProc32(state, x, y, dstC, count);
        return;
    }

    uint32_t buffer[BUF_MAX + TEST_BUFFER_EXTRA];
    SkBitmapProcState::MatrixProc   mproc = state.fMatrixProc;
    SkBitmapProcState::SampleProc32 sproc = state.fSampleProc32;
    int max = fState.maxCountForBufferSize(sizeof(buffer[0]) * BUF_MAX);

    SkASSERT(state.fBitmap->getPixels());
    SkASSERT(state.fBitmap->pixelRef() == NULL ||
             state.fBitmap->pixelRef()->getLockCount());

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

void SkBitmapProcShader::shadeSpan16(int x, int y, uint16_t dstC[], int count) {
    const SkBitmapProcState& state = fState;
    if (state.fShaderProc16) {
        state.fShaderProc16(state, x, y, dstC, count);
        return;
    }

    uint32_t buffer[BUF_MAX];
    SkBitmapProcState::MatrixProc   mproc = state.fMatrixProc;
    SkBitmapProcState::SampleProc16 sproc = state.fSampleProc16;
    int max = fState.maxCountForBufferSize(sizeof(buffer));

    SkASSERT(state.fBitmap->getPixels());
    SkASSERT(state.fBitmap->pixelRef() == NULL ||
             state.fBitmap->pixelRef()->getLockCount());

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

    switch (bm.config()) {
        case SkBitmap::kARGB_8888_Config:
            *color = SkUnPreMultiply::PMColorToColor(*bm.getAddr32(0, 0));
            return true;
        case SkBitmap::kRGB_565_Config:
            *color = SkPixel16ToColor(*bm.getAddr16(0, 0));
            return true;
        case SkBitmap::kIndex8_Config:
            *color = SkUnPreMultiply::PMColorToColor(bm.getIndex8Color(0, 0));
            return true;
        default: // just skip the other configs for now
            break;
    }
    return false;
}

#include "SkTemplatesPriv.h"

SkShader* SkShader::CreateBitmapShader(const SkBitmap& src,
                                       TileMode tmx, TileMode tmy,
                                       void* storage, size_t storageSize) {
    SkShader* shader;
    SkColor color;
    if (canUseColorShader(src, &color)) {
        SK_PLACEMENT_NEW_ARGS(shader, SkColorShader, storage, storageSize,
                              (color));
    } else {
        SK_PLACEMENT_NEW_ARGS(shader, SkBitmapProcShader, storage,
                              storageSize, (src, tmx, tmy));
    }
    return shader;
}

static SkFlattenable::Registrar gBitmapProcShaderReg("SkBitmapProcShader",
                                               SkBitmapProcShader::CreateProc);

///////////////////////////////////////////////////////////////////////////////

static const char* gTileModeName[] = {
    "clamp", "repeat", "mirror"
};

bool SkBitmapProcShader::toDumpString(SkString* str) const {
    str->printf("BitmapShader: [%d %d %d",
                fRawBitmap.width(), fRawBitmap.height(),
                fRawBitmap.bytesPerPixel());

    // add the pixelref
    SkPixelRef* pr = fRawBitmap.pixelRef();
    if (pr) {
        const char* uri = pr->getURI();
        if (uri) {
            str->appendf(" \"%s\"", uri);
        }
    }

    // add the (optional) matrix
    {
        SkMatrix m;
        if (this->getLocalMatrix(&m)) {
            SkString info;
            m.toDumpString(&info);
            str->appendf(" %s", info.c_str());
        }
    }

    str->appendf(" [%s %s]]",
                 gTileModeName[fState.fTileModeX],
                 gTileModeName[fState.fTileModeY]);
    return true;
}

