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
#include "SkImage.h"
#include "SkMatrixUtils.h"
#include "SkPicture.h"
#include "SkReadBuffer.h"
#include "SkResourceCache.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrCaps.h"
#endif

namespace {
static unsigned gBitmapSkaderKeyNamespaceLabel;

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
        this->init(&gBitmapSkaderKeyNamespaceLabel, 0, keySize);
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

    const Key& getKey() const override { return fKey; }
    size_t bytesUsed() const override {
        return sizeof(fKey) + sizeof(SkShader) + fBitmapBytes;
    }
    const char* getCategory() const override { return "bitmap-shader"; }
    SkDiscardableMemory* diagnostic_only_getDiscardable() const override { return nullptr; }

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

SkPictureShader::SkPictureShader(sk_sp<SkPicture> picture, TileMode tmx, TileMode tmy,
                                 const SkMatrix* localMatrix, const SkRect* tile)
    : INHERITED(localMatrix)
    , fPicture(std::move(picture))
    , fTile(tile ? *tile : fPicture->cullRect())
    , fTmx(tmx)
    , fTmy(tmy) {
}

sk_sp<SkShader> SkPictureShader::Make(sk_sp<SkPicture> picture, TileMode tmx, TileMode tmy,
                                      const SkMatrix* localMatrix, const SkRect* tile) {
    if (!picture || picture->cullRect().isEmpty() || (tile && tile->isEmpty())) {
        return SkShader::MakeEmptyShader();
    }
    return sk_sp<SkShader>(new SkPictureShader(std::move(picture), tmx, tmy, localMatrix, tile));
}

sk_sp<SkFlattenable> SkPictureShader::CreateProc(SkReadBuffer& buffer) {
    SkMatrix lm;
    buffer.readMatrix(&lm);
    TileMode mx = (TileMode)buffer.read32();
    TileMode my = (TileMode)buffer.read32();
    SkRect tile;
    buffer.readRect(&tile);

    sk_sp<SkPicture> picture;

    if (buffer.isCrossProcess() && SkPicture::PictureIOSecurityPrecautionsEnabled()) {
        if (buffer.isVersionLT(SkReadBuffer::kPictureShaderHasPictureBool_Version)) {
            // Older code blindly serialized pictures.  We don't trust them.
            buffer.validate(false);
            return nullptr;
        }
        // Newer code won't serialize pictures in disallow-cross-process-picture mode.
        // Assert that they didn't serialize anything except a false here.
        buffer.validate(!buffer.readBool());
    } else {
        // Old code always serialized the picture.  New code writes a 'true' first if it did.
        if (buffer.isVersionLT(SkReadBuffer::kPictureShaderHasPictureBool_Version) ||
            buffer.readBool()) {
            picture = SkPicture::MakeFromBuffer(buffer);
        }
    }
    return SkPictureShader::Make(picture, mx, my, &lm, &tile);
}

void SkPictureShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeMatrix(this->getLocalMatrix());
    buffer.write32(fTmx);
    buffer.write32(fTmy);
    buffer.writeRect(fTile);

    // The deserialization code won't trust that our serialized picture is safe to deserialize.
    // So write a 'false' telling it that we're not serializing a picture.
    if (buffer.isCrossProcess() && SkPicture::PictureIOSecurityPrecautionsEnabled()) {
        buffer.writeBool(false);
    } else {
        buffer.writeBool(true);
        fPicture->flatten(buffer);
    }
}

sk_sp<SkShader> SkPictureShader::refBitmapShader(const SkMatrix& viewMatrix, const SkMatrix* localM,
                                                 const int maxTextureSize) const {
    SkASSERT(fPicture && !fPicture->cullRect().isEmpty());

    SkMatrix m;
    m.setConcat(viewMatrix, this->getLocalMatrix());
    if (localM) {
        m.preConcat(*localM);
    }

    // Use a rotation-invariant scale
    SkPoint scale;
    //
    // TODO: replace this with decomposeScale() -- but beware LayoutTest rebaselines!
    //
    if (!SkDecomposeUpper2x2(m, nullptr, &scale, nullptr)) {
        // Decomposition failed, use an approximation.
        scale.set(SkScalarSqrt(m.getScaleX() * m.getScaleX() + m.getSkewX() * m.getSkewX()),
                  SkScalarSqrt(m.getScaleY() * m.getScaleY() + m.getSkewY() * m.getSkewY()));
    }
    SkSize scaledSize = SkSize::Make(SkScalarAbs(scale.x() * fTile.width()),
                                     SkScalarAbs(scale.y() * fTile.height()));

    // Clamp the tile size to about 4M pixels
    static const SkScalar kMaxTileArea = 2048 * 2048;
    SkScalar tileArea = SkScalarMul(scaledSize.width(), scaledSize.height());
    if (tileArea > kMaxTileArea) {
        SkScalar clampScale = SkScalarSqrt(kMaxTileArea / tileArea);
        scaledSize.set(SkScalarMul(scaledSize.width(), clampScale),
                       SkScalarMul(scaledSize.height(), clampScale));
    }
#if SK_SUPPORT_GPU
    // Scale down the tile size if larger than maxTextureSize for GPU Path or it should fail on create texture
    if (maxTextureSize) {
        if (scaledSize.width() > maxTextureSize || scaledSize.height() > maxTextureSize) {
            SkScalar downScale = maxTextureSize / SkMaxScalar(scaledSize.width(), scaledSize.height());
            scaledSize.set(SkScalarFloorToScalar(SkScalarMul(scaledSize.width(), downScale)),
                           SkScalarFloorToScalar(SkScalarMul(scaledSize.height(), downScale)));
        }
    }
#endif

#ifdef SK_SUPPORT_LEGACY_PICTURESHADER_ROUNDING
    const SkISize tileSize = scaledSize.toRound();
#else
    const SkISize tileSize = scaledSize.toCeil();
#endif
    if (tileSize.isEmpty()) {
        return SkShader::MakeEmptyShader();
    }

    // The actual scale, compensating for rounding & clamping.
    const SkSize tileScale = SkSize::Make(SkIntToScalar(tileSize.width()) / fTile.width(),
                                          SkIntToScalar(tileSize.height()) / fTile.height());

    sk_sp<SkShader> tileShader;
    BitmapShaderKey key(fPicture->uniqueID(),
                        fTile,
                        fTmx,
                        fTmy,
                        tileScale,
                        this->getLocalMatrix());

    if (!SkResourceCache::Find(key, BitmapShaderRec::Visitor, &tileShader)) {
        SkMatrix tileMatrix;
        tileMatrix.setRectToRect(fTile, SkRect::MakeIWH(tileSize.width(), tileSize.height()),
                                 SkMatrix::kFill_ScaleToFit);

        sk_sp<SkImage> tileImage(
            SkImage::MakeFromPicture(fPicture, tileSize, &tileMatrix, nullptr));
        if (!tileImage) {
            return nullptr;
        }

        SkMatrix shaderMatrix = this->getLocalMatrix();
        shaderMatrix.preScale(1 / tileScale.width(), 1 / tileScale.height());
        tileShader = tileImage->makeShader(fTmx, fTmy, &shaderMatrix);

        const SkImageInfo tileInfo = SkImageInfo::MakeN32Premul(tileSize);
        SkResourceCache::Add(new BitmapShaderRec(key, tileShader.get(),
                                                 tileInfo.getSafeSize(tileInfo.minRowBytes())));
    }

    return tileShader;
}

size_t SkPictureShader::onContextSize(const ContextRec&) const {
    return sizeof(PictureShaderContext);
}

SkShader::Context* SkPictureShader::onCreateContext(const ContextRec& rec, void* storage) const {
    sk_sp<SkShader> bitmapShader(this->refBitmapShader(*rec.fMatrix, rec.fLocalMatrix));
    if (!bitmapShader) {
        return nullptr;
    }
    return PictureShaderContext::Create(storage, *this, rec, bitmapShader);
}

/////////////////////////////////////////////////////////////////////////////////////////

SkShader::Context* SkPictureShader::PictureShaderContext::Create(void* storage,
                   const SkPictureShader& shader, const ContextRec& rec,
                                                                 sk_sp<SkShader> bitmapShader) {
    PictureShaderContext* ctx = new (storage) PictureShaderContext(shader, rec,
                                                                   std::move(bitmapShader));
    if (nullptr == ctx->fBitmapShaderContext) {
        ctx->~PictureShaderContext();
        ctx = nullptr;
    }
    return ctx;
}

SkPictureShader::PictureShaderContext::PictureShaderContext(
        const SkPictureShader& shader, const ContextRec& rec, sk_sp<SkShader> bitmapShader)
    : INHERITED(shader, rec)
    , fBitmapShader(std::move(bitmapShader))
{
    fBitmapShaderContextStorage = sk_malloc_throw(fBitmapShader->contextSize(rec));
    fBitmapShaderContext = fBitmapShader->createContext(rec, fBitmapShaderContextStorage);
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
sk_sp<GrFragmentProcessor> SkPictureShader::asFragmentProcessor(
                                                     GrContext* context, const SkMatrix& viewM,
                                                     const SkMatrix* localMatrix,
                                                     SkFilterQuality fq,
                                                     SkSourceGammaTreatment gammaTreatment) const {
    int maxTextureSize = 0;
    if (context) {
        maxTextureSize = context->caps()->maxTextureSize();
    }
    sk_sp<SkShader> bitmapShader(this->refBitmapShader(viewM, localMatrix, maxTextureSize));
    if (!bitmapShader) {
        return nullptr;
    }
    return bitmapShader->asFragmentProcessor(context, viewM, nullptr, fq, gammaTreatment);
}
#endif
