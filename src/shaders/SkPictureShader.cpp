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
#include "SkCachedData.h"
#include "SkCanvas.h"
#include "SkColorSpaceXformCanvas.h"
#include "SkImage.h"
#include "SkImage_Gpu.h"
#include "SkImageShader.h"
#include "SkMatrixUtils.h"
#include "SkPicture.h"
#include "SkPictureImageGenerator.h"
#include "SkReadBuffer.h"
#include "SkResourceCache.h"

#if SK_SUPPORT_GPU
#include "GrCaps.h"
#include "GrColorSpaceInfo.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrFragmentProcessor.h"
#include "GrProxyProvider.h"
#endif

namespace {

static int32_t gNextID = 1;
uint32_t next_id() {
    int32_t id;
    do {
        id = sk_atomic_inc(&gNextID);
    } while (id == SK_InvalidGenID);
    return static_cast<uint32_t>(id);
}

static unsigned gBitmapTileKeyNamespaceLabel;

struct BitmapTileKey : public SkResourceCache::Key {
public:
    BitmapTileKey(sk_sp<SkColorSpace> colorSpace,
                  uint32_t pictureID,
                  const SkRect& tile,
                  const SkISize& tileSize,
                  SkTransferFunctionBehavior blendBehavior)
        : fColorSpace(std::move(colorSpace))
        , fTile(tile)
        , fTileSize(tileSize)
        , fBlendBehavior(blendBehavior) {

        static const size_t keySize = sizeof(fColorSpace) +
                                      sizeof(fTile) +
                                      sizeof(fTileSize) +
                                      sizeof(fBlendBehavior);
        // This better be packed.
        SkASSERT(sizeof(uint32_t) * (&fEndOfStruct - (uint32_t*)&fColorSpace) == keySize);
        this->init(&gBitmapTileKeyNamespaceLabel, MakeSharedID(pictureID), keySize);
    }

    static uint64_t MakeSharedID(uint32_t shaderID) {
        uint64_t sharedID = SkSetFourByteTag('p', 's', 'd', 'r');
        return (sharedID << 32) | shaderID;
    }

private:
    // TODO: there are some fishy things about using CS sk_sps in the key:
    //   - false negatives: keys are memcmp'ed, so we don't detect equivalent CSs
    //     (SkColorspace::Equals)
    //   - we're keeping the CS alive, even when the client releases it
    //
    // Ideally we'd be using unique IDs or some other weak ref + purge mechanism
    // when the CS is deleted.
    sk_sp<SkColorSpace>        fColorSpace;
    SkRect                     fTile;
    SkISize                    fTileSize;
    SkTransferFunctionBehavior fBlendBehavior;

    SkDEBUGCODE(uint32_t fEndOfStruct;)
};

struct BitmapTileRec : public SkResourceCache::Rec {
    BitmapTileRec(const BitmapTileKey& key, SkCachedData* cachedData)
        : fKey(key)
        , fCachedData(cachedData) {
        fCachedData->attachToCacheAndRef();
    }

    ~BitmapTileRec() override {
        fCachedData->detachFromCacheAndUnref();
    }

    BitmapTileKey fKey;
    SkCachedData* fCachedData;

    const Key& getKey() const override { return fKey; }

    size_t bytesUsed() const override {
        return sizeof(*this) + fCachedData->size();
    }

    const char* getCategory() const override { return "picture-shader"; }

    SkDiscardableMemory* diagnostic_only_getDiscardable() const override {
        return fCachedData->diagnostic_only_getDiscardable();
    }

    static bool Visitor(const SkResourceCache::Rec& baseRec, void* ctx) {
        const BitmapTileRec& rec = static_cast<const BitmapTileRec&>(baseRec);

        auto dataRef = sk_ref_sp(rec.fCachedData);
        if (!dataRef->data()) {
            return false;
        }

        *static_cast<sk_sp<SkCachedData>*>(ctx) = std::move(dataRef);

        return true;
    }
};

} // namespace

SkPictureShader::SkPictureShader(sk_sp<SkPicture> picture, TileMode tmx, TileMode tmy,
                                 const SkMatrix* localMatrix, const SkRect* tile,
                                 sk_sp<SkColorSpace> colorSpace)
    : INHERITED(localMatrix)
    , fPicture(std::move(picture))
    , fTile(tile ? *tile : fPicture->cullRect())
    , fTmx(tmx)
    , fTmy(tmy)
    , fColorSpace(std::move(colorSpace))
    , fUniqueID(next_id())
    , fAddedToCache(false) {}

SkPictureShader::~SkPictureShader() {
    if (fAddedToCache.load()) {
        SkResourceCache::PostPurgeSharedID(BitmapTileKey::MakeSharedID(fUniqueID));
    }
}

sk_sp<SkShader> SkPictureShader::Make(sk_sp<SkPicture> picture, TileMode tmx, TileMode tmy,
                                      const SkMatrix* localMatrix, const SkRect* tile) {
    if (!picture || picture->cullRect().isEmpty() || (tile && tile->isEmpty())) {
        return SkShader::MakeEmptyShader();
    }
    return sk_sp<SkShader>(new SkPictureShader(std::move(picture), tmx, tmy, localMatrix, tile,
                                               nullptr));
}

sk_sp<SkFlattenable> SkPictureShader::CreateProc(SkReadBuffer& buffer) {
    SkMatrix lm;
    buffer.readMatrix(&lm);
    TileMode mx = (TileMode)buffer.read32();
    TileMode my = (TileMode)buffer.read32();
    SkRect tile;
    buffer.readRect(&tile);

    sk_sp<SkPicture> picture;

    bool didSerialize = buffer.readBool();
    if (didSerialize) {
        picture = SkPicture::MakeFromBuffer(buffer);
    }
    return SkPictureShader::Make(picture, mx, my, &lm, &tile);
}

void SkPictureShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeMatrix(this->getLocalMatrix());
    buffer.write32(fTmx);
    buffer.write32(fTmy);
    buffer.writeRect(fTile);

    buffer.writeBool(true);
    fPicture->flatten(buffer);
}

struct SkPictureShader::TileInfo {
    SkImageInfo                fInfo;
    SkMatrix                   fTileMatrix;
    SkTransferFunctionBehavior fBlendBehavior;
};

SkPictureShader::TileInfo SkPictureShader::computeTileInfo(const SkMatrix& ctm,
                                                           const SkMatrix* localMatrix,
                                                           SkColorSpace* dstCS,
                                                           int maxTileSize) const {
    SkMatrix totalLocalMatrix = this->getLocalMatrix();
    if (localMatrix) {
        totalLocalMatrix.preConcat(*localMatrix);
    }
    const auto m = SkMatrix::Concat(ctm, totalLocalMatrix);

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

    if (scaledSize.width() > maxTileSize || scaledSize.height() > maxTileSize) {
        SkScalar downScale = maxTileSize / SkMaxScalar(scaledSize.width(), scaledSize.height());
        scaledSize.set(SkScalarFloorToScalar(scaledSize.width() * downScale),
                       SkScalarFloorToScalar(scaledSize.height() * downScale));
    }

    const auto tileSize = scaledSize.toCeil();

    // |fColorSpace| will only be set when using an SkColorSpaceXformCanvas to do pre-draw xforms.
    // This canvas is strictly for legacy mode.  A non-null |dstColorSpace| indicates that we
    // should perform color correct rendering and xform at draw time.
    SkASSERT(!fColorSpace || !dstCS);
    sk_sp<SkColorSpace>   cs = dstCS ? sk_ref_sp(dstCS) : fColorSpace;
    const auto blendBehavior = dstCS ? SkTransferFunctionBehavior::kRespect
                                     : SkTransferFunctionBehavior::kIgnore;
    const auto info = SkImageInfo::MakeN32Premul(tileSize.width(),
                                                 tileSize.height(),
                                                 std::move(cs));
    const auto tileMatrix =
        SkMatrix::Concat(totalLocalMatrix, SkMatrix::MakeScale(fTile.width() / tileSize.width(),
                                                               fTile.height() / tileSize.height()));
    return { info, tileMatrix, blendBehavior };
}

std::unique_ptr<SkImageGenerator> SkPictureShader::makeTileGenerator(const TileInfo& ti) const {
    const auto matrix = SkMatrix::MakeRectToRect(fTile,
                                                 SkRect::MakeIWH(ti.fInfo.width(),
                                                                 ti.fInfo.height()),
                                                 SkMatrix::kFill_ScaleToFit);
    return SkPictureImageGenerator::Make(ti.fInfo.dimensions(),
                                         fPicture,
                                         &matrix,
                                         nullptr,
                                         SkImage::BitDepth::kU8,
                                         ti.fInfo.refColorSpace());
}

sk_sp<SkShader> SkPictureShader::lockTile(const TileInfo& tileInfo) const {
    const BitmapTileKey key(tileInfo.fInfo.refColorSpace(),
                            fUniqueID, // TODO: use the picture unique ID for caching purposes
                            fTile,
                            tileInfo.fInfo.dimensions(),
                            tileInfo.fBlendBehavior);
    sk_sp<SkCachedData> data;

    if (!SkResourceCache::Find(key, BitmapTileRec::Visitor, &data)) {
        auto generator = this->makeTileGenerator(tileInfo);
        data.reset(SkResourceCache::NewCachedData(tileInfo.fInfo.computeMinByteSize()));

        SkImageGenerator::Options options;
        options.fBehavior = tileInfo.fBlendBehavior;

        if (!data || !generator || !generator->getPixels(tileInfo.fInfo,
                                                         data->writable_data(),
                                                         tileInfo.fInfo.minRowBytes(),
                                                         &options)) {
            return nullptr;
        }

        SkResourceCache::Add(new BitmapTileRec(key, data.get()));
        fAddedToCache.store(true);
    }

    SkASSERT(data && data->data());
    auto tileImage = SkImage::MakeFromRaster(SkPixmap(tileInfo.fInfo,
                                                      data->data(),
                                                      tileInfo.fInfo.minRowBytes()),
                                             [](const void*, void* ctx) {
                                                 static_cast<SkCachedData*>(ctx)->unref();
                                             }, data.release()); // Pass ref to the image.

    return tileImage ? tileImage->makeShader(fTmx, fTmy) : nullptr;
}

bool SkPictureShader::onIsRasterPipelineOnly(const SkMatrix& ctm) const {
    return SkImageShader::IsRasterPipelineOnly(ctm, kN32_SkColorType, kPremul_SkAlphaType,
                                               fTmx, fTmy, this->getLocalMatrix());
}

bool SkPictureShader::onAppendStages(const StageRec& rec) const {
    const auto tileInfo = this->computeTileInfo(rec.fCTM, rec.fLocalM, rec.fDstCS);

    // Keep tileShader alive by using alloc instead of stack memory
    auto& tileShader = *rec.fAlloc->make<sk_sp<SkShader>>(this->lockTile(tileInfo));
    if (!tileShader) {
        return false;
    }

    StageRec localRec = rec;
    localRec.fLocalM = &tileInfo.fTileMatrix;

    return as_SB(tileShader)->appendStages(localRec);
}

/////////////////////////////////////////////////////////////////////////////////////////
SkShaderBase::Context* SkPictureShader::onMakeContext(const ContextRec& rec,
                                                      SkArenaAlloc* alloc) const {
    const auto tileInfo = this->computeTileInfo(*rec.fMatrix, rec.fLocalMatrix, rec.fDstColorSpace);
    auto tileShader = this->lockTile(tileInfo);
    if (!tileShader) {
        return nullptr;
    }

    ContextRec localRec = rec;
    localRec.fLocalMatrix = &tileInfo.fTileMatrix;

    PictureShaderContext* ctx =
        alloc->make<PictureShaderContext>(*this, localRec, std::move(tileShader), alloc);
    if (nullptr == ctx->fBitmapShaderContext) {
        ctx = nullptr;
    }

    return ctx;
}

sk_sp<SkShader> SkPictureShader::onMakeColorSpace(SkColorSpaceXformer* xformer) const {
    sk_sp<SkColorSpace> dstCS = xformer->dst();
    if (SkColorSpace::Equals(dstCS.get(), fColorSpace.get())) {
        return sk_ref_sp(const_cast<SkPictureShader*>(this));
    }

    return sk_sp<SkPictureShader>(new SkPictureShader(fPicture, fTmx, fTmy, &this->getLocalMatrix(),
                                                      &fTile, std::move(dstCS)));
}

/////////////////////////////////////////////////////////////////////////////////////////

SkPictureShader::PictureShaderContext::PictureShaderContext(
        const SkPictureShader& shader, const ContextRec& rec, sk_sp<SkShader> bitmapShader,
        SkArenaAlloc* alloc)
    : INHERITED(shader, rec)
    , fBitmapShader(std::move(bitmapShader))
{
    fBitmapShaderContext = as_SB(fBitmapShader)->makeContext(rec, alloc);
    //if fBitmapShaderContext is null, we are invalid
}

uint32_t SkPictureShader::PictureShaderContext::getFlags() const {
    SkASSERT(fBitmapShaderContext);
    return fBitmapShaderContext->getFlags();
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
sk_sp<SkShader> SkPictureShader::lockTile(const TileInfo& tileInfo, const GrFPArgs& args) const {
    const BitmapTileKey key(tileInfo.fInfo.refColorSpace(),
                            fUniqueID, // TODO: use the picture unique ID for caching purposes
                            fTile,
                            tileInfo.fInfo.dimensions(),
                            tileInfo.fBlendBehavior);
    static const GrUniqueKey::Domain kPictureShaderTileDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey grKey;
    key.toGrUniqueKey(kPictureShaderTileDomain, &grKey);

    SkASSERT(grKey.isValid());
    GrProxyProvider* proxyProvider = args.fContext->contextPriv().proxyProvider();
    auto proxy = proxyProvider->findOrCreateProxyByUniqueKey(grKey, kTopLeft_GrSurfaceOrigin);

    if (!proxy) {
        auto generator = this->makeTileGenerator(tileInfo);
        if (!generator)
            return nullptr;
        proxy = generator->generateTexture(args.fContext,
                                           tileInfo.fInfo,
                                           SkIPoint::Make(0, 0),
                                           tileInfo.fBlendBehavior,
                                           false);
        if (!proxyProvider->assignUniqueKeyToProxy(grKey, proxy.get())) {
            return nullptr;
        }

        fAddedToCache.store(true);
    }

    SkASSERT(proxy);
    auto tileImage = sk_make_sp<SkImage_Gpu>(args.fContext,
                                             kNeedNewImageUniqueID,
                                             kPremul_SkAlphaType,
                                             std::move(proxy),
                                             tileInfo.fInfo.refColorSpace(),
                                             SkBudgeted::kNo);
    return tileImage ? tileImage->makeShader(fTmx, fTmy) : nullptr;
}

std::unique_ptr<GrFragmentProcessor> SkPictureShader::asFragmentProcessor(
        const GrFPArgs& args) const {
    const auto tileInfo = this->computeTileInfo(*args.fViewMatrix,
                                                args.fLocalMatrix,
                                                args.fDstColorSpaceInfo->colorSpace());
    auto tileShader = this->lockTile(tileInfo, args);
    if (!tileShader) {
        return nullptr;
    }

    return as_SB(tileShader)->asFragmentProcessor(GrFPArgs(args.fContext,
                                                           args.fViewMatrix,
                                                           &tileInfo.fTileMatrix,
                                                           args.fFilterQuality,
                                                           args.fDstColorSpaceInfo));
}
#endif
