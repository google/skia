/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPictureShader.h"

#include "SkArenaAlloc.h"
#include "SkBitmap.h"
#include "SkBitmapProcShader.h"
#include "SkCanvas.h"
#include "SkImage.h"
#include "SkImageShader.h"
#include "SkMatrixUtils.h"
#include "SkPicture.h"
#include "SkPictureImageGenerator.h"
#include "SkReadBuffer.h"
#include "SkResourceCache.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrCaps.h"
#include "GrFragmentProcessor.h"
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
    BitmapShaderRec(const BitmapShaderKey& key, SkShader* tileShader)
        : fKey(key)
        , fShader(SkRef(tileShader)) {}

    BitmapShaderKey fKey;
    sk_sp<SkShader> fShader;
    size_t          fBitmapBytes;

    const Key& getKey() const override { return fKey; }
    size_t bytesUsed() const override {
        // Just the record overhead -- the actual pixels are accounted by SkImageCacherator.
        return sizeof(fKey) + sizeof(SkImageShader);
    }
    const char* getCategory() const override { return "bitmap-shader"; }
    SkDiscardableMemory* diagnostic_only_getDiscardable() const override { return nullptr; }

    static bool Visitor(const SkResourceCache::Rec& baseRec, void* contextShader) {
        const BitmapShaderRec& rec = static_cast<const BitmapShaderRec&>(baseRec);
        sk_sp<SkShader>* result = reinterpret_cast<sk_sp<SkShader>*>(contextShader);

        *result = rec.fShader;

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
                                                 SkColorSpace* dstColorSpace,
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
    SkScalar tileArea = scaledSize.width() * scaledSize.height();
    if (tileArea > kMaxTileArea) {
        SkScalar clampScale = SkScalarSqrt(kMaxTileArea / tileArea);
        scaledSize.set(scaledSize.width() * clampScale,
                       scaledSize.height() * clampScale);
    }
#if SK_SUPPORT_GPU
    // Scale down the tile size if larger than maxTextureSize for GPU Path or it should fail on create texture
    if (maxTextureSize) {
        if (scaledSize.width() > maxTextureSize || scaledSize.height() > maxTextureSize) {
            SkScalar downScale = maxTextureSize / SkMaxScalar(scaledSize.width(), scaledSize.height());
            scaledSize.set(SkScalarFloorToScalar(scaledSize.width() * downScale),
                           SkScalarFloorToScalar(scaledSize.height() * downScale));
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

        sk_sp<SkImage> tileImage = SkImage::MakeFromGenerator(
                SkPictureImageGenerator::Make(tileSize, fPicture, &tileMatrix, nullptr,
                                              SkImage::BitDepth::kU8, sk_ref_sp(dstColorSpace)));
        if (!tileImage) {
            return nullptr;
        }

        SkMatrix shaderMatrix = this->getLocalMatrix();
        shaderMatrix.preScale(1 / tileScale.width(), 1 / tileScale.height());
        tileShader = tileImage->makeShader(fTmx, fTmy, &shaderMatrix);

        SkResourceCache::Add(new BitmapShaderRec(key, tileShader.get()));
    }

    return tileShader;
}

bool SkPictureShader::onAppendStages(SkRasterPipeline* p, SkColorSpace* cs, SkArenaAlloc* alloc,
                                     const SkMatrix& ctm, const SkPaint& paint,
                                     const SkMatrix* localMatrix) const {
    // Keep bitmapShader alive by using alloc instead of stack memory
    auto& bitmapShader = *alloc->make<sk_sp<SkShader>>();
    bitmapShader = this->refBitmapShader(ctm, localMatrix, cs);
    return bitmapShader && bitmapShader->appendStages(p, cs, alloc, ctm, paint);
}

/////////////////////////////////////////////////////////////////////////////////////////
SkShader::Context* SkPictureShader::onMakeContext(const ContextRec& rec, SkArenaAlloc* alloc)
const {
    sk_sp<SkShader> bitmapShader(this->refBitmapShader(*rec.fMatrix, rec.fLocalMatrix,
                                                       rec.fDstColorSpace));
    if (!bitmapShader) {
        return nullptr;
    }

    PictureShaderContext* ctx =
        alloc->make<PictureShaderContext>(*this, rec, std::move(bitmapShader), alloc);
    if (nullptr == ctx->fBitmapShaderContext) {
        ctx = nullptr;
    }
    return ctx;
}

/////////////////////////////////////////////////////////////////////////////////////////

SkPictureShader::PictureShaderContext::PictureShaderContext(
        const SkPictureShader& shader, const ContextRec& rec, sk_sp<SkShader> bitmapShader,
        SkArenaAlloc* alloc)
    : INHERITED(shader, rec)
    , fBitmapShader(std::move(bitmapShader))
{
    fBitmapShaderContext = fBitmapShader->makeContext(rec, alloc);
    //if fBitmapShaderContext is null, we are invalid
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
sk_sp<GrFragmentProcessor> SkPictureShader::asFragmentProcessor(const AsFPArgs& args) const {
    int maxTextureSize = 0;
    if (args.fContext) {
        maxTextureSize = args.fContext->caps()->maxTextureSize();
    }
    sk_sp<SkShader> bitmapShader(this->refBitmapShader(*args.fViewMatrix, args.fLocalMatrix,
                                                       args.fDstColorSpace, maxTextureSize));
    if (!bitmapShader) {
        return nullptr;
    }
    return bitmapShader->asFragmentProcessor(SkShader::AsFPArgs(
        args.fContext, args.fViewMatrix, nullptr, args.fFilterQuality, args.fDstColorSpace));
}
#endif
