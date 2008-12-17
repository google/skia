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
}

SkBitmapProcShader::SkBitmapProcShader(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {
    fRawBitmap.unflatten(buffer);
    fState.fTileModeX = buffer.readU8();
    fState.fTileModeY = buffer.readU8();
}

void SkBitmapProcShader::beginSession() {
    this->INHERITED::beginSession();

    fRawBitmap.lockPixels();
}

void SkBitmapProcShader::endSession() {
    fRawBitmap.unlockPixels();

    this->INHERITED::endSession();
}

bool SkBitmapProcShader::asABitmap(SkBitmap* texture, SkMatrix* texM,
                                   TileMode xy[]) {
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
    return true;
}

void SkBitmapProcShader::flatten(SkFlattenableWriteBuffer& buffer) {
    this->INHERITED::flatten(buffer);

    fRawBitmap.flatten(buffer);
    buffer.write8(fState.fTileModeX);
    buffer.write8(fState.fTileModeY);
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
    if (fState.fOrigBitmap.getPixels() == NULL) {
        fState.fOrigBitmap.unlockPixels();
        return false;
    }

    if (!fState.chooseProcs(this->getTotalInverse(), paint)) {
        return false;
    }

    bool bitmapIsOpaque = fState.fBitmap->isOpaque();
    
    // filtering doesn't guarantee that opaque stays opaque (finite precision)
    // so pretend we're not opaque if we're being asked to filter. If we had
    // more blit-procs, we could specialize on opaque src, and just OR in 0xFF
    // after the filter to be sure...
    if (paint.isFilterBitmap()) {
        bitmapIsOpaque = false;
    }

    // update fFlags
    fFlags = 0; // this should happen in SkShader.cpp

    if (bitmapIsOpaque && (255 == this->getPaintAlpha())) {
        fFlags |= kOpaqueAlpha_Flag;
    }

    switch (fState.fBitmap->config()) {
        case SkBitmap::kRGB_565_Config:
            fFlags |= (kHasSpan16_Flag | kIntrinsicly16_Flag);
            break;
        case SkBitmap::kIndex8_Config:
        case SkBitmap::kARGB_8888_Config:
            if (bitmapIsOpaque) {
                fFlags |= kHasSpan16_Flag;
            }
            break;
        case SkBitmap::kA8_Config:
            break;  // never set kHasSpan16_Flag
        default:
            break;
    }
    return true;
}

#define BUF_MAX     128

void SkBitmapProcShader::shadeSpan(int x, int y, SkPMColor dstC[], int count) {
    uint32_t buffer[BUF_MAX];

    const SkBitmapProcState&        state = fState;
    SkBitmapProcState::MatrixProc   mproc = state.fMatrixProc;
    SkBitmapProcState::SampleProc32 sproc = state.fSampleProc32;
    int max = fState.fDoFilter ? (BUF_MAX >> 1) : BUF_MAX;

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

void SkBitmapProcShader::shadeSpan16(int x, int y, uint16_t dstC[], int count) {
    uint32_t buffer[BUF_MAX];
    
    const SkBitmapProcState&        state = fState;
    SkBitmapProcState::MatrixProc   mproc = state.fMatrixProc;
    SkBitmapProcState::SampleProc16 sproc = state.fSampleProc16;
    int max = fState.fDoFilter ? (BUF_MAX >> 1) : BUF_MAX;

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

#include "SkTemplatesPriv.h"

SkShader* SkShader::CreateBitmapShader(const SkBitmap& src,
                                       TileMode tmx, TileMode tmy,
                                       void* storage, size_t storageSize) {
    SkShader* shader;
    SK_PLACEMENT_NEW_ARGS(shader, SkBitmapProcShader, storage,
                          storageSize, (src, tmx, tmy));
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

