/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/SkPictureShader.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/private/SkImageInfoPriv.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkMatrixUtils.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkResourceCache.h"
#include "src/core/SkVM.h"
#include "src/shaders/SkBitmapProcShader.h"
#include "src/shaders/SkImageShader.h"
#include <atomic>

#if SK_SUPPORT_GPU
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/image/SkImage_Base.h"
#endif

sk_sp<SkShader> SkPicture::makeShader(SkTileMode tmx, SkTileMode tmy, SkFilterMode filter,
                                      const SkMatrix* localMatrix, const SkRect* tile) const {
    if (localMatrix && !localMatrix->invert(nullptr)) {
        return nullptr;
    }
    return SkPictureShader::Make(sk_ref_sp(this), tmx, tmy, filter, localMatrix, tile);
}

namespace {
static unsigned gImageFromPictureKeyNamespaceLabel;

struct ImageFromPictureKey : public SkResourceCache::Key {
public:
    ImageFromPictureKey(SkColorSpace* colorSpace, SkColorType colorType,
                        uint32_t pictureID, const SkRect& subset,
                        SkSize scale)
        : fColorSpaceXYZHash(colorSpace->toXYZD50Hash())
        , fColorSpaceTransferFnHash(colorSpace->transferFnHash())
        , fColorType(static_cast<uint32_t>(colorType))
        , fSubset(subset)
        , fScale(scale)
    {
        static const size_t keySize = sizeof(fColorSpaceXYZHash) +
                                      sizeof(fColorSpaceTransferFnHash) +
                                      sizeof(fColorType) +
                                      sizeof(fSubset) +
                                      sizeof(fScale);
        // This better be packed.
        SkASSERT(sizeof(uint32_t) * (&fEndOfStruct - &fColorSpaceXYZHash) == keySize);
        this->init(&gImageFromPictureKeyNamespaceLabel,
                   SkPicturePriv::MakeSharedID(pictureID),
                   keySize);
    }

private:
    uint32_t    fColorSpaceXYZHash;
    uint32_t    fColorSpaceTransferFnHash;
    uint32_t    fColorType;
    SkRect      fSubset;
    SkSize      fScale;

    SkDEBUGCODE(uint32_t fEndOfStruct;)
};

struct ImageFromPictureRec : public SkResourceCache::Rec {
    ImageFromPictureRec(const ImageFromPictureKey& key, sk_sp<SkImage> image)
        : fKey(key)
        , fImage(std::move(image)) {}

    ImageFromPictureKey fKey;
    sk_sp<SkImage>  fImage;

    const Key& getKey() const override { return fKey; }
    size_t bytesUsed() const override {
        // Just the record overhead -- the actual pixels are accounted by SkImage_Lazy.
        return sizeof(fKey) + (size_t)fImage->width() * fImage->height() * 4;
    }
    const char* getCategory() const override { return "bitmap-shader"; }
    SkDiscardableMemory* diagnostic_only_getDiscardable() const override { return nullptr; }

    static bool Visitor(const SkResourceCache::Rec& baseRec, void* contextShader) {
        const ImageFromPictureRec& rec = static_cast<const ImageFromPictureRec&>(baseRec);
        sk_sp<SkImage>* result = reinterpret_cast<sk_sp<SkImage>*>(contextShader);

        *result = rec.fImage;
        return true;
    }
};

} // namespace

SkPictureShader::SkPictureShader(sk_sp<SkPicture> picture, SkTileMode tmx, SkTileMode tmy,
                                 SkFilterMode filter, const SkMatrix* localMatrix, const SkRect* tile)
    : INHERITED(localMatrix)
    , fPicture(std::move(picture))
    , fTile(tile ? *tile : fPicture->cullRect())
    , fTmx(tmx)
    , fTmy(tmy)
    , fFilter(filter) {}

sk_sp<SkShader> SkPictureShader::Make(sk_sp<SkPicture> picture, SkTileMode tmx, SkTileMode tmy,
                                      SkFilterMode filter, const SkMatrix* lm, const SkRect* tile) {
    if (!picture || picture->cullRect().isEmpty() || (tile && tile->isEmpty())) {
        return SkShaders::Empty();
    }
    return sk_sp<SkShader>(new SkPictureShader(std::move(picture), tmx, tmy, filter, lm, tile));
}

sk_sp<SkFlattenable> SkPictureShader::CreateProc(SkReadBuffer& buffer) {
    SkMatrix lm;
    buffer.readMatrix(&lm);
    auto tmx = buffer.read32LE(SkTileMode::kLastTileMode);
    auto tmy = buffer.read32LE(SkTileMode::kLastTileMode);
    SkRect tile = buffer.readRect();

    sk_sp<SkPicture> picture;

    SkFilterMode filter = SkFilterMode::kNearest;
    if (buffer.isVersionLT(SkPicturePriv::kNoFilterQualityShaders_Version)) {
        if (buffer.isVersionLT(SkPicturePriv::kPictureShaderFilterParam_Version)) {
            bool didSerialize = buffer.readBool();
            if (didSerialize) {
                picture = SkPicturePriv::MakeFromBuffer(buffer);
            }
        } else {
            unsigned legacyFilter = buffer.read32();
            if (legacyFilter <= (unsigned)SkFilterMode::kLast) {
                filter = (SkFilterMode)legacyFilter;
            }
            picture = SkPicturePriv::MakeFromBuffer(buffer);
        }
    } else {
        filter = buffer.read32LE(SkFilterMode::kLast);
        picture = SkPicturePriv::MakeFromBuffer(buffer);
    }
    return SkPictureShader::Make(picture, tmx, tmy, filter, &lm, &tile);
}

void SkPictureShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeMatrix(this->getLocalMatrix());
    buffer.write32((unsigned)fTmx);
    buffer.write32((unsigned)fTmy);
    buffer.writeRect(fTile);
    buffer.write32((unsigned)fFilter);
    SkPicturePriv::Flatten(fPicture, buffer);
}

static sk_sp<SkColorSpace> ref_or_srgb(SkColorSpace* cs) {
    return cs ? sk_ref_sp(cs) : SkColorSpace::MakeSRGB();
}

struct CachedImageInfo {
    bool        success;
    SkSize      tileScale;
    SkMatrix    matrixForDraw;
    SkImageInfo imageInfo;

    static CachedImageInfo Make(const SkRect& bounds,
                                const SkMatrix& viewMatrix,
                                SkTCopyOnFirstWrite<SkMatrix>* localMatrix,     // in/out
                                SkColorType dstColorType,
                                SkColorSpace* dstColorSpace,
                                const int maxTextureSize) {
        const SkMatrix m = SkMatrix::Concat(viewMatrix, **localMatrix);

        const SkSize scaledSize = [&]() {
            SkSize size;
            // Use a rotation-invariant scale
            if (!m.decomposeScale(&size, nullptr)) {
                size = {1, 1};
            }
            size.fWidth  *= bounds.width();
            size.fHeight *= bounds.height();

            // Clamp the tile size to about 4M pixels
            static const SkScalar kMaxTileArea = 2048 * 2048;
            SkScalar tileArea = size.width() * size.height();
            if (tileArea > kMaxTileArea) {
                SkScalar clampScale = SkScalarSqrt(kMaxTileArea / tileArea);
                size.set(size.width() * clampScale, size.height() * clampScale);
            }

            // Scale down the tile size if larger than maxTextureSize for GPU Path
            // or it should fail on create texture
            if (maxTextureSize) {
                if (size.width() > maxTextureSize || size.height() > maxTextureSize) {
                    SkScalar downScale = maxTextureSize / std::max(size.width(),
                                                                   size.height());
                    size.set(SkScalarFloorToScalar(size.width() * downScale),
                             SkScalarFloorToScalar(size.height() * downScale));
                }
            }
            return size;
        }();

        const SkISize tileSize = scaledSize.toCeil();
        if (tileSize.isEmpty()) {
            return {false, {}, {}, {}};
        }

        const SkSize tileScale = {
            tileSize.width() / bounds.width(), tileSize.height() / bounds.height()
        };
        auto imgCS = ref_or_srgb(dstColorSpace);
        const SkColorType imgCT = SkColorTypeMaxBitsPerChannel(dstColorType) <= 8
                                ? kRGBA_8888_SkColorType
                                : kRGBA_F16Norm_SkColorType;

        if (tileScale.width() != 1 || tileScale.height() != 1) {
            localMatrix->writable()->preScale(1 / tileScale.width(), 1 / tileScale.height());
        }

        return {
            true,
            tileScale,
            SkMatrix::RectToRect(bounds, SkRect::MakeIWH(tileSize.width(), tileSize.height())),
            SkImageInfo::Make(tileSize.width(), tileSize.height(),
                              imgCT, kPremul_SkAlphaType, imgCS),
        };
    }

    sk_sp<SkImage> makeImage(sk_sp<SkSurface> surf, const SkPicture* pict) const {
        if (!surf) {
            return nullptr;
        }
        auto canvas = surf->getCanvas();
        canvas->concat(matrixForDraw);
        canvas->drawPicture(pict);
        return surf->makeImageSnapshot();
    }
};

// Returns a cached image shader, which wraps a single picture tile at the given
// CTM/local matrix.  Also adjusts the local matrix for tile scaling.
sk_sp<SkShader> SkPictureShader::rasterShader(const SkMatrix& viewMatrix,
                                              SkTCopyOnFirstWrite<SkMatrix>* localMatrix,
                                              SkColorType dstColorType,
                                              SkColorSpace* dstColorSpace) const {
    const int maxTextureSize_NotUsedForCPU = 0;
    CachedImageInfo info = CachedImageInfo::Make(fTile, viewMatrix, localMatrix,
                                                 dstColorType, dstColorSpace,
                                                 maxTextureSize_NotUsedForCPU);
    if (!info.success) {
        return nullptr;
    }

    ImageFromPictureKey key(info.imageInfo.colorSpace(), info.imageInfo.colorType(),
                        fPicture->uniqueID(), fTile, info.tileScale);

    sk_sp<SkImage> image;
    if (!SkResourceCache::Find(key, ImageFromPictureRec::Visitor, &image)) {
        image = info.makeImage(SkSurface::MakeRaster(info.imageInfo), fPicture.get());
        if (!image) {
            return nullptr;
        }

        SkResourceCache::Add(new ImageFromPictureRec(key, image));
        SkPicturePriv::AddedToCache(fPicture.get());
    }
    return image->makeShader(fTmx, fTmy, SkSamplingOptions(fFilter), nullptr);
}

bool SkPictureShader::onAppendStages(const SkStageRec& rec) const {
    auto lm = this->totalLocalMatrix(rec.fLocalM);
    // Keep bitmapShader alive by using alloc instead of stack memory
    auto& bitmapShader = *rec.fAlloc->make<sk_sp<SkShader>>();
    bitmapShader = this->rasterShader(rec.fMatrixProvider.localToDevice(), &lm,
                                      rec.fDstColorType, rec.fDstCS);
    if (!bitmapShader) {
        return false;
    }

    SkStageRec localRec = rec;
    localRec.fLocalM = lm->isIdentity() ? nullptr : lm.get();

    return as_SB(bitmapShader)->appendStages(localRec);
}

skvm::Color SkPictureShader::onProgram(skvm::Builder* p,
                                       skvm::Coord device, skvm::Coord local, skvm::Color paint,
                                       const SkMatrixProvider& matrices, const SkMatrix* localM,
                                       const SkColorInfo& dst,
                                       skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const {
    auto lm = this->totalLocalMatrix(localM);

    // Keep bitmapShader alive by using alloc instead of stack memory
    auto& bitmapShader = *alloc->make<sk_sp<SkShader>>();
    bitmapShader = this->rasterShader(matrices.localToDevice(), &lm,
                                      dst.colorType(), dst.colorSpace());
    if (!bitmapShader) {
        return {};
    }

    return as_SB(bitmapShader)->program(p, device,local, paint,
                                        matrices,lm, dst,
                                        uniforms,alloc);
}

/////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
SkShaderBase::Context* SkPictureShader::onMakeContext(const ContextRec& rec, SkArenaAlloc* alloc)
const {
    auto lm = this->totalLocalMatrix(rec.fLocalMatrix);
    sk_sp<SkShader> bitmapShader = this->rasterShader(*rec.fMatrix, &lm, rec.fDstColorType,
                                                      rec.fDstColorSpace);
    if (!bitmapShader) {
        return nullptr;
    }

    ContextRec localRec = rec;
    localRec.fLocalMatrix = lm->isIdentity() ? nullptr : lm.get();

    PictureShaderContext* ctx =
        alloc->make<PictureShaderContext>(*this, localRec, std::move(bitmapShader), alloc);
    if (nullptr == ctx->fBitmapShaderContext) {
        ctx = nullptr;
    }
    return ctx;
}
#endif

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

#if SK_SUPPORT_GPU

#include "src/gpu/GrProxyProvider.h"

std::unique_ptr<GrFragmentProcessor> SkPictureShader::asFragmentProcessor(
        const GrFPArgs& args) const {

    auto ctx = args.fContext;
    auto lm = this->totalLocalMatrix(args.fPreLocalMatrix);
    SkColorType dstColorType = GrColorTypeToSkColorType(args.fDstColorInfo->colorType());
    if (dstColorType == kUnknown_SkColorType) {
        dstColorType = kRGBA_8888_SkColorType;
    }

    auto dstCS = ref_or_srgb(args.fDstColorInfo->colorSpace());
    auto info = CachedImageInfo::Make(fTile, args.fMatrixProvider.localToDevice(), &lm,
                                      dstColorType, dstCS.get(),
                                      ctx->priv().caps()->maxTextureSize());
    SkMatrix inv;
    if (!info.success || !(*lm).invert(&inv)) {
        return nullptr;
    }

    // Gotta be sure the GPU can support our requested colortype (might be FP16)
    if (!ctx->colorTypeSupportedAsSurface(info.imageInfo.colorType())) {
        info.imageInfo = info.imageInfo.makeColorType(kRGBA_8888_SkColorType);
    }

    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey key;
    GrUniqueKey::Builder builder(&key, kDomain, 10, "Picture Shader Image");
    builder[0] = dstCS->toXYZD50Hash();
    builder[1] = dstCS->transferFnHash();
    builder[2] = static_cast<uint32_t>(dstColorType);
    builder[3] = fPicture->uniqueID();
    memcpy(&builder[4], &fTile, sizeof(fTile));                     // 4,5,6,7
    memcpy(&builder[8], &info.tileScale, sizeof(info.tileScale));   // 8,9
    builder.finish();

    GrProxyProvider* provider = ctx->priv().proxyProvider();
    GrSurfaceProxyView view;
    if (auto proxy = provider->findOrCreateProxyByUniqueKey(key)) {
        view = GrSurfaceProxyView(proxy, kTopLeft_GrSurfaceOrigin, GrSwizzle());
    } else {
        const int msaaSampleCount = 0;
        const SkSurfaceProps* props = nullptr;
        const bool createWithMips = false;
        auto image = info.makeImage(SkSurface::MakeRenderTarget(ctx,
                                                                SkBudgeted::kYes,
                                                                info.imageInfo,
                                                                msaaSampleCount,
                                                                kTopLeft_GrSurfaceOrigin,
                                                                props,
                                                                createWithMips),
                                    fPicture.get());
        if (!image) {
            return nullptr;
        }
        auto [v, ct] = as_IB(image)->asView(ctx, GrMipmapped::kNo);
        view = std::move(v);
        provider->assignUniqueKeyToProxy(key, view.asTextureProxy());
    }

    const GrSamplerState sampler(static_cast<GrSamplerState::WrapMode>(fTmx),
                                 static_cast<GrSamplerState::WrapMode>(fTmy),
                                 fFilter);

    return GrFragmentProcessor::MulChildByInputAlpha(
        GrTextureEffect::Make(std::move(view), kPremul_SkAlphaType, inv, sampler,
                              *ctx->priv().caps()));
}
#endif
