/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPictureShader.h"

#include "SkBitmap.h"
#include "SkBitmapProcShader.h"
#include "SkCanvas.h"
#include "SkImageGenerator.h"
#include "SkMatrixUtils.h"
#include "SkPicture.h"
#include "SkReadBuffer.h"
#include "SkResourceCache.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif

namespace {
static unsigned gBitmapSkaderKeyNamespaceLabel;

class PictureImageGenerator : public SkImageGenerator {
public:
    PictureImageGenerator(const SkPicture* picture,
                          const SkRect& pictureTile,
                          const SkISize& tileSize)
        : fPicture(SkRef(picture))
        , fPictureTile(pictureTile)
        , fRasterTileInfo(SkImageInfo::MakeN32Premul(tileSize)) {}

protected:
    virtual bool onGetInfo(SkImageInfo *info) SK_OVERRIDE {
        *info = fRasterTileInfo;
        return true;
    }

    virtual Result onGetPixels(const SkImageInfo& info, void *pixels, size_t rowBytes,
                             SkPMColor ctable[], int *ctableCount) SK_OVERRIDE {
        if (info != fRasterTileInfo || SkToBool(ctable) || SkToBool(ctableCount)) {
            return kInvalidConversion;
        }

        SkSize tileScale = SkSize::Make(SkIntToScalar(info.width()) / fPictureTile.width(),
                                        SkIntToScalar(info.height()) / fPictureTile.height());
        SkBitmap tileBitmap;
        if (!tileBitmap.installPixels(info, pixels, rowBytes)) {
            return kInvalidParameters;
        }
        tileBitmap.eraseColor(SK_ColorTRANSPARENT);

        // Always disable LCD text, since we can't assume our image will be opaque.
        SkCanvas tileCanvas(tileBitmap, SkSurfaceProps(0, kUnknown_SkPixelGeometry));
        tileCanvas.scale(tileScale.width(), tileScale.height());
        tileCanvas.translate(-fPictureTile.x(), -fPictureTile.y());
        tileCanvas.drawPicture(fPicture);

        return kSuccess;
    }

private:
    SkAutoTUnref<const SkPicture> fPicture;
    const SkRect                  fPictureTile;
    const SkImageInfo             fRasterTileInfo;
};

struct BitmapShaderKey : public SkResourceCache::Key {
public:
    BitmapShaderKey(uint32_t pictureID,
                    const SkRect& tile,
                    SkShader::TileMode tmx,
                    SkShader::TileMode tmy,
                    const SkSize& scale,
                    const SkMatrix& localMatrix)
        : fPictureID(pictureID)
        , fTile(tile)
        , fTmx(tmx)
        , fTmy(tmy)
        , fScale(scale) {

        for (int i = 0; i < 9; ++i) {
            fLocalMatrixStorage[i] = localMatrix[i];
        }

        static const size_t keySize = sizeof(fPictureID) +
                                      sizeof(fTile) +
                                      sizeof(fTmx) + sizeof(fTmy) +
                                      sizeof(fScale) +
                                      sizeof(fLocalMatrixStorage);
        // This better be packed.
        SkASSERT(sizeof(uint32_t) * (&fEndOfStruct - &fPictureID) == keySize);
        this->init(&gBitmapSkaderKeyNamespaceLabel, keySize);
    }

private:
    uint32_t           fPictureID;
    SkRect             fTile;
    SkShader::TileMode fTmx, fTmy;
    SkSize             fScale;
    SkScalar           fLocalMatrixStorage[9];

    SkDEBUGCODE(uint32_t fEndOfStruct;)
};

struct BitmapShaderRec : public SkResourceCache::Rec {
    BitmapShaderRec(const BitmapShaderKey& key, SkShader* tileShader, size_t bitmapBytes)
        : fKey(key)
        , fShader(SkRef(tileShader))
        , fBitmapBytes(bitmapBytes) {}

    BitmapShaderKey        fKey;
    SkAutoTUnref<SkShader> fShader;
    size_t                 fBitmapBytes;

    const Key& getKey() const SK_OVERRIDE { return fKey; }
    size_t bytesUsed() const SK_OVERRIDE {
        return sizeof(fKey) + sizeof(SkShader) + fBitmapBytes;
    }

    static bool Visitor(const SkResourceCache::Rec& baseRec, void* contextShader) {
        const BitmapShaderRec& rec = static_cast<const BitmapShaderRec&>(baseRec);
        SkAutoTUnref<SkShader>* result = reinterpret_cast<SkAutoTUnref<SkShader>*>(contextShader);

        result->reset(SkRef(rec.fShader.get()));

        // The bitmap shader is backed by an image generator, thus it can always re-generate its
        // pixels if discarded.
        return true;
    }
};

} // namespace

SkPictureShader::SkPictureShader(const SkPicture* picture, TileMode tmx, TileMode tmy,
                                 const SkMatrix* localMatrix, const SkRect* tile)
    : INHERITED(localMatrix)
    , fPicture(SkRef(picture))
    , fTile(tile ? *tile : picture->cullRect())
    , fTmx(tmx)
    , fTmy(tmy) {
}

SkPictureShader* SkPictureShader::Create(const SkPicture* picture, TileMode tmx, TileMode tmy,
                                         const SkMatrix* localMatrix, const SkRect* tile) {
    if (!picture || picture->cullRect().isEmpty() || (tile && tile->isEmpty())) {
        return NULL;
    }
    return SkNEW_ARGS(SkPictureShader, (picture, tmx, tmy, localMatrix, tile));
}

SkFlattenable* SkPictureShader::CreateProc(SkReadBuffer& buffer) {
    SkMatrix lm;
    buffer.readMatrix(&lm);
    TileMode mx = (TileMode)buffer.read32();
    TileMode my = (TileMode)buffer.read32();
    SkRect tile;
    buffer.readRect(&tile);
    SkAutoTUnref<SkPicture> picture(SkPicture::CreateFromBuffer(buffer));
    return SkPictureShader::Create(picture, mx, my, &lm, &tile);
}

void SkPictureShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeMatrix(this->getLocalMatrix());
    buffer.write32(fTmx);
    buffer.write32(fTmy);
    buffer.writeRect(fTile);
    fPicture->flatten(buffer);
}

SkShader* SkPictureShader::refBitmapShader(const SkMatrix& matrix, const SkMatrix* localM) const {
    SkASSERT(fPicture && !fPicture->cullRect().isEmpty());

    SkMatrix m;
    m.setConcat(matrix, this->getLocalMatrix());
    if (localM) {
        m.preConcat(*localM);
    }

    // Use a rotation-invariant scale
    SkPoint scale;
    if (!SkDecomposeUpper2x2(m, NULL, &scale, NULL)) {
        // Decomposition failed, use an approximation.
        scale.set(SkScalarSqrt(m.getScaleX() * m.getScaleX() + m.getSkewX() * m.getSkewX()),
                  SkScalarSqrt(m.getScaleY() * m.getScaleY() + m.getSkewY() * m.getSkewY()));
    }
    SkSize scaledSize = SkSize::Make(SkScalarAbs(scale.x() * fTile.width()),
                                     SkScalarAbs(scale.y() * fTile.height()));

    // Clamp the tile size to about 16M pixels
    static const SkScalar kMaxTileArea = 4096 * 4096;
    SkScalar tileArea = SkScalarMul(scaledSize.width(), scaledSize.height());
    if (tileArea > kMaxTileArea) {
        SkScalar clampScale = SkScalarSqrt(SkScalarDiv(kMaxTileArea, tileArea));
        scaledSize.set(SkScalarMul(scaledSize.width(), clampScale),
                       SkScalarMul(scaledSize.height(), clampScale));
    }

    SkISize tileSize = scaledSize.toRound();
    if (tileSize.isEmpty()) {
        return NULL;
    }

    // The actual scale, compensating for rounding & clamping.
    SkSize tileScale = SkSize::Make(SkIntToScalar(tileSize.width()) / fTile.width(),
                                    SkIntToScalar(tileSize.height()) / fTile.height());

    SkAutoTUnref<SkShader> tileShader;
    BitmapShaderKey key(fPicture->uniqueID(),
                        fTile,
                        fTmx,
                        fTmy,
                        tileScale,
                        this->getLocalMatrix());

    if (!SkResourceCache::Find(key, BitmapShaderRec::Visitor, &tileShader)) {
        SkBitmap bm;
        if (!SkInstallDiscardablePixelRef(SkNEW_ARGS(PictureImageGenerator,
                                                     (fPicture, fTile, tileSize)), &bm)) {
            return NULL;
        }

        SkMatrix shaderMatrix = this->getLocalMatrix();
        shaderMatrix.preScale(1 / tileScale.width(), 1 / tileScale.height());
        tileShader.reset(CreateBitmapShader(bm, fTmx, fTmy, &shaderMatrix));

        SkResourceCache::Add(SkNEW_ARGS(BitmapShaderRec, (key, tileShader.get(), bm.getSize())));
    }

    return tileShader.detach();
}

size_t SkPictureShader::contextSize() const {
    return sizeof(PictureShaderContext);
}

SkShader::Context* SkPictureShader::onCreateContext(const ContextRec& rec, void* storage) const {
    SkAutoTUnref<SkShader> bitmapShader(this->refBitmapShader(*rec.fMatrix, rec.fLocalMatrix));
    if (NULL == bitmapShader.get()) {
        return NULL;
    }
    return PictureShaderContext::Create(storage, *this, rec, bitmapShader);
}

/////////////////////////////////////////////////////////////////////////////////////////

SkShader::Context* SkPictureShader::PictureShaderContext::Create(void* storage,
                   const SkPictureShader& shader, const ContextRec& rec, SkShader* bitmapShader) {
    PictureShaderContext* ctx = SkNEW_PLACEMENT_ARGS(storage, PictureShaderContext,
                                                     (shader, rec, bitmapShader));
    if (NULL == ctx->fBitmapShaderContext) {
        ctx->~PictureShaderContext();
        ctx = NULL;
    }
    return ctx;
}

SkPictureShader::PictureShaderContext::PictureShaderContext(
        const SkPictureShader& shader, const ContextRec& rec, SkShader* bitmapShader)
    : INHERITED(shader, rec)
    , fBitmapShader(SkRef(bitmapShader))
{
    fBitmapShaderContextStorage = sk_malloc_throw(bitmapShader->contextSize());
    fBitmapShaderContext = bitmapShader->createContext(rec, fBitmapShaderContextStorage);
    //if fBitmapShaderContext is null, we are invalid
}

SkPictureShader::PictureShaderContext::~PictureShaderContext() {
    if (fBitmapShaderContext) {
        fBitmapShaderContext->~Context();
    }
    sk_free(fBitmapShaderContextStorage);
}

uint32_t SkPictureShader::PictureShaderContext::getFlags() const {
    SkASSERT(fBitmapShaderContext);
    return fBitmapShaderContext->getFlags();
}

SkShader::Context::ShadeProc SkPictureShader::PictureShaderContext::asAShadeProc(void** ctx) {
    SkASSERT(fBitmapShaderContext);
    return fBitmapShaderContext->asAShadeProc(ctx);
}

void SkPictureShader::PictureShaderContext::shadeSpan(int x, int y, SkPMColor dstC[], int count) {
    SkASSERT(fBitmapShaderContext);
    fBitmapShaderContext->shadeSpan(x, y, dstC, count);
}

void SkPictureShader::PictureShaderContext::shadeSpan16(int x, int y, uint16_t dstC[], int count) {
    SkASSERT(fBitmapShaderContext);
    fBitmapShaderContext->shadeSpan16(x, y, dstC, count);
}

#ifndef SK_IGNORE_TO_STRING
void SkPictureShader::toString(SkString* str) const {
    static const char* gTileModeName[SkShader::kTileModeCount] = {
        "clamp", "repeat", "mirror"
    };

    str->appendf("PictureShader: [%f:%f:%f:%f] ",
                 fPicture->cullRect().fLeft,
                 fPicture->cullRect().fTop,
                 fPicture->cullRect().fRight,
                 fPicture->cullRect().fBottom);

    str->appendf("(%s, %s)", gTileModeName[fTmx], gTileModeName[fTmy]);

    this->INHERITED::toString(str);
}
#endif

#if SK_SUPPORT_GPU
bool SkPictureShader::asFragmentProcessor(GrContext* context, const SkPaint& paint,
                                          const SkMatrix& viewM, const SkMatrix* localMatrix,
                                          GrColor* paintColor,
                                          GrFragmentProcessor** fp) const {
    SkAutoTUnref<SkShader> bitmapShader(this->refBitmapShader(viewM, localMatrix));
    if (!bitmapShader) {
        return false;
    }
    return bitmapShader->asFragmentProcessor(context, paint, viewM, NULL, paintColor, fp);
}
#else
bool SkPictureShader::asFragmentProcessor(GrContext*, const SkPaint&, const SkMatrix&,
                                          const SkMatrix*, GrColor*,
                                          GrFragmentProcessor**) const {
    SkDEBUGFAIL("Should not call in GPU-less build");
    return false;
}
#endif
