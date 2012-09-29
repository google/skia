
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrContext.h"

#include "effects/GrConvolutionEffect.h"
#include "effects/GrSingleTextureEffect.h"
#include "effects/GrConfigConversionEffect.h"

#include "GrBufferAllocPool.h"
#include "GrGpu.h"
#include "GrIndexBuffer.h"
#include "GrInOrderDrawBuffer.h"
#include "GrPathRenderer.h"
#include "GrPathUtils.h"
#include "GrResourceCache.h"
#include "GrSoftwarePathRenderer.h"
#include "GrStencilBuffer.h"
#include "GrTextStrike.h"
#include "SkTLazy.h"
#include "SkTLS.h"
#include "SkTrace.h"

SK_DEFINE_INST_COUNT(GrContext)
SK_DEFINE_INST_COUNT(GrDrawState)

// It can be useful to set this to kNo_BufferedDraw to test whether a bug is caused by using the
// InOrderDrawBuffer, to compare performance of using/not using InOrderDrawBuffer, or to make
// debugging easier.
#define DEFAULT_BUFFERING (GR_DISABLE_DRAW_BUFFERING ? kNo_BufferedDraw : kYes_BufferedDraw)

#define MAX_BLUR_SIGMA 4.0f

// When we're using coverage AA but the blend is incompatible (given gpu
// limitations) should we disable AA or draw wrong?
#define DISABLE_COVERAGE_AA_FOR_BLEND 1

#if GR_DEBUG
    // change this to a 1 to see notifications when partial coverage fails
    #define GR_DEBUG_PARTIAL_COVERAGE_CHECK 0
#else
    #define GR_DEBUG_PARTIAL_COVERAGE_CHECK 0
#endif

static const size_t MAX_TEXTURE_CACHE_COUNT = 256;
static const size_t MAX_TEXTURE_CACHE_BYTES = 16 * 1024 * 1024;

static const size_t DRAW_BUFFER_VBPOOL_BUFFER_SIZE = 1 << 15;
static const int DRAW_BUFFER_VBPOOL_PREALLOC_BUFFERS = 4;

static const size_t DRAW_BUFFER_IBPOOL_BUFFER_SIZE = 1 << 11;
static const int DRAW_BUFFER_IBPOOL_PREALLOC_BUFFERS = 4;

#define ASSERT_OWNED_RESOURCE(R) GrAssert(!(R) || (R)->getContext() == this)

GrContext* GrContext::Create(GrEngine engine,
                             GrPlatform3DContext context3D) {
    GrContext* ctx = NULL;
    GrGpu* fGpu = GrGpu::Create(engine, context3D);
    if (NULL != fGpu) {
        ctx = SkNEW_ARGS(GrContext, (fGpu));
        fGpu->unref();
    }
    return ctx;
}

namespace {
void* CreateThreadInstanceCount() {
    return SkNEW_ARGS(int, (0));
}
void DeleteThreadInstanceCount(void* v) {
    delete reinterpret_cast<int*>(v);
}
#define THREAD_INSTANCE_COUNT                                               \
    (*reinterpret_cast<int*>(SkTLS::Get(CreateThreadInstanceCount,          \
                                        DeleteThreadInstanceCount)))

}

int GrContext::GetThreadInstanceCount() {
    return THREAD_INSTANCE_COUNT;
}

GrContext::~GrContext() {
    this->flush();

    // Since the gpu can hold scratch textures, give it a chance to let go
    // of them before freeing the texture cache
    fGpu->purgeResources();

    delete fTextureCache;
    fTextureCache = NULL;
    delete fFontCache;
    delete fDrawBuffer;
    delete fDrawBufferVBAllocPool;
    delete fDrawBufferIBAllocPool;

    fAARectRenderer->unref();

    fGpu->unref();
    GrSafeUnref(fPathRendererChain);
    GrSafeUnref(fSoftwarePathRenderer);
    fDrawState->unref();

    --THREAD_INSTANCE_COUNT;
}

void GrContext::contextLost() {
    contextDestroyed();
    this->setupDrawBuffer();
}

void GrContext::contextDestroyed() {
    // abandon first to so destructors
    // don't try to free the resources in the API.
    fGpu->abandonResources();

    // a path renderer may be holding onto resources that
    // are now unusable
    GrSafeSetNull(fPathRendererChain);
    GrSafeSetNull(fSoftwarePathRenderer);

    delete fDrawBuffer;
    fDrawBuffer = NULL;

    delete fDrawBufferVBAllocPool;
    fDrawBufferVBAllocPool = NULL;

    delete fDrawBufferIBAllocPool;
    fDrawBufferIBAllocPool = NULL;

    fAARectRenderer->reset();

    fTextureCache->purgeAllUnlocked();
    fFontCache->freeAll();
    fGpu->markContextDirty();
}

void GrContext::resetContext() {
    fGpu->markContextDirty();
}

void GrContext::freeGpuResources() {
    this->flush();

    fGpu->purgeResources();

    fAARectRenderer->reset();

    fTextureCache->purgeAllUnlocked();
    fFontCache->freeAll();
    // a path renderer may be holding onto resources
    GrSafeSetNull(fPathRendererChain);
    GrSafeSetNull(fSoftwarePathRenderer);
}

size_t GrContext::getGpuTextureCacheBytes() const {
  return fTextureCache->getCachedResourceBytes();
}

////////////////////////////////////////////////////////////////////////////////

namespace {

void scale_rect(SkRect* rect, float xScale, float yScale) {
    rect->fLeft = SkScalarMul(rect->fLeft, SkFloatToScalar(xScale));
    rect->fTop = SkScalarMul(rect->fTop, SkFloatToScalar(yScale));
    rect->fRight = SkScalarMul(rect->fRight, SkFloatToScalar(xScale));
    rect->fBottom = SkScalarMul(rect->fBottom, SkFloatToScalar(yScale));
}

float adjust_sigma(float sigma, int *scaleFactor, int *radius) {
    *scaleFactor = 1;
    while (sigma > MAX_BLUR_SIGMA) {
        *scaleFactor *= 2;
        sigma *= 0.5f;
    }
    *radius = static_cast<int>(ceilf(sigma * 3.0f));
    GrAssert(*radius <= GrConvolutionEffect::kMaxKernelRadius);
    return sigma;
}

void convolve_gaussian(GrDrawTarget* target,
                       GrTexture* texture,
                       const SkRect& rect,
                       float sigma,
                       int radius,
                       Gr1DKernelEffect::Direction direction) {
    GrRenderTarget* rt = target->drawState()->getRenderTarget();
    GrDrawTarget::AutoStateRestore asr(target, GrDrawTarget::kReset_ASRInit);
    GrDrawState* drawState = target->drawState();
    drawState->setRenderTarget(rt);
    GrMatrix sampleM;
    sampleM.setIDiv(texture->width(), texture->height());
    drawState->sampler(0)->reset(sampleM);
    SkAutoTUnref<GrConvolutionEffect> conv(SkNEW_ARGS(GrConvolutionEffect,
                                                      (texture, direction, radius,
                                                       sigma)));
    drawState->sampler(0)->setCustomStage(conv);
    target->drawSimpleRect(rect, NULL);
}

}


GrTexture* GrContext::findTexture(const GrCacheKey& key) {
    return static_cast<GrTexture*>(fTextureCache->find(key.key()));
}

GrTexture* GrContext::findTexture(const GrTextureDesc& desc,
                                  const GrCacheData& cacheData,
                                  const GrTextureParams* params) {
    GrResourceKey resourceKey = GrTexture::ComputeKey(fGpu, params, desc, cacheData, false);
    GrResource* resource = fTextureCache->find(resourceKey);
    return static_cast<GrTexture*>(resource);
}

bool GrContext::isTextureInCache(const GrTextureDesc& desc,
                                 const GrCacheData& cacheData,
                                 const GrTextureParams* params) const {
    GrResourceKey resourceKey = GrTexture::ComputeKey(fGpu, params, desc, cacheData, false);
    return fTextureCache->hasKey(resourceKey);
}

void GrContext::addStencilBuffer(GrStencilBuffer* sb) {
    ASSERT_OWNED_RESOURCE(sb);

    GrResourceKey resourceKey = GrStencilBuffer::ComputeKey(sb->width(),
                                                            sb->height(),
                                                            sb->numSamples());
    fTextureCache->create(resourceKey, sb);
}

GrStencilBuffer* GrContext::findStencilBuffer(int width, int height,
                                              int sampleCnt) {
    GrResourceKey resourceKey = GrStencilBuffer::ComputeKey(width,
                                                            height,
                                                            sampleCnt);
    GrResource* resource = fTextureCache->find(resourceKey);
    return static_cast<GrStencilBuffer*>(resource);
}

static void stretchImage(void* dst,
                         int dstW,
                         int dstH,
                         void* src,
                         int srcW,
                         int srcH,
                         int bpp) {
    GrFixed dx = (srcW << 16) / dstW;
    GrFixed dy = (srcH << 16) / dstH;

    GrFixed y = dy >> 1;

    int dstXLimit = dstW*bpp;
    for (int j = 0; j < dstH; ++j) {
        GrFixed x = dx >> 1;
        void* srcRow = (uint8_t*)src + (y>>16)*srcW*bpp;
        void* dstRow = (uint8_t*)dst + j*dstW*bpp;
        for (int i = 0; i < dstXLimit; i += bpp) {
            memcpy((uint8_t*) dstRow + i,
                   (uint8_t*) srcRow + (x>>16)*bpp,
                   bpp);
            x += dx;
        }
        y += dy;
    }
}

// The desired texture is NPOT and tiled but that isn't supported by
// the current hardware. Resize the texture to be a POT
GrTexture* GrContext::createResizedTexture(const GrTextureDesc& desc,
                                           const GrCacheData& cacheData,
                                           void* srcData,
                                           size_t rowBytes,
                                           bool needsFiltering) {
    GrTexture* clampedTexture = this->findTexture(desc, cacheData, NULL);
    if (NULL == clampedTexture) {
        clampedTexture = this->createTexture(NULL, desc, cacheData, srcData, rowBytes);

        GrAssert(NULL != clampedTexture);
        if (NULL == clampedTexture) {
            return NULL;
        }
    }

    clampedTexture->ref();

    GrTextureDesc rtDesc = desc;
    rtDesc.fFlags =  rtDesc.fFlags |
                     kRenderTarget_GrTextureFlagBit |
                     kNoStencil_GrTextureFlagBit;
    rtDesc.fWidth  = GrNextPow2(GrMax(desc.fWidth, 64));
    rtDesc.fHeight = GrNextPow2(GrMax(desc.fHeight, 64));

    GrTexture* texture = fGpu->createTexture(rtDesc, NULL, 0);

    if (NULL != texture) {
        GrDrawTarget::AutoStateRestore asr(fGpu, GrDrawTarget::kReset_ASRInit);
        GrDrawState* drawState = fGpu->drawState();
        drawState->setRenderTarget(texture->asRenderTarget());

        // if filtering is not desired then we want to ensure all
        // texels in the resampled image are copies of texels from
        // the original.
        drawState->sampler(0)->reset(SkShader::kClamp_TileMode,
                                     needsFiltering);
        drawState->createTextureEffect(0, clampedTexture);

        static const GrVertexLayout layout =
                            GrDrawTarget::StageTexCoordVertexLayoutBit(0,0);
        GrDrawTarget::AutoReleaseGeometry arg(fGpu, layout, 4, 0);

        if (arg.succeeded()) {
            GrPoint* verts = (GrPoint*) arg.vertices();
            verts[0].setIRectFan(0, 0,
                                    texture->width(),
                                    texture->height(),
                                    2*sizeof(GrPoint));
            verts[1].setIRectFan(0, 0, 1, 1, 2*sizeof(GrPoint));
            fGpu->drawNonIndexed(kTriangleFan_GrPrimitiveType,
                                    0, 4);
        }
        texture->releaseRenderTarget();
    } else {
        // TODO: Our CPU stretch doesn't filter. But we create separate
        // stretched textures when the sampler state is either filtered or
        // not. Either implement filtered stretch blit on CPU or just create
        // one when FBO case fails.

        rtDesc.fFlags = kNone_GrTextureFlags;
        // no longer need to clamp at min RT size.
        rtDesc.fWidth  = GrNextPow2(desc.fWidth);
        rtDesc.fHeight = GrNextPow2(desc.fHeight);
        int bpp = GrBytesPerPixel(desc.fConfig);
        SkAutoSMalloc<128*128*4> stretchedPixels(bpp *
                                                    rtDesc.fWidth *
                                                    rtDesc.fHeight);
        stretchImage(stretchedPixels.get(), rtDesc.fWidth, rtDesc.fHeight,
                        srcData, desc.fWidth, desc.fHeight, bpp);

        size_t stretchedRowBytes = rtDesc.fWidth * bpp;

        GrTexture* texture = fGpu->createTexture(rtDesc,
                                                    stretchedPixels.get(),
                                                    stretchedRowBytes);
        GrAssert(NULL != texture);
    }

    clampedTexture->unref();
    return texture;
}

GrTexture* GrContext::createTexture(
        const GrTextureParams* params,
        const GrTextureDesc& desc,
        const GrCacheData& cacheData,
        void* srcData,
        size_t rowBytes) {
    SK_TRACE_EVENT0("GrContext::createAndLockTexture");

#if GR_DUMP_TEXTURE_UPLOAD
    GrPrintf("GrContext::createAndLockTexture [%d %d]\n", desc.fWidth, desc.fHeight);
#endif

    GrResourceKey resourceKey = GrTexture::ComputeKey(fGpu, params, desc, cacheData, false);

    SkAutoTUnref<GrTexture> texture;
    if (GrTexture::NeedsResizing(resourceKey)) {
        texture.reset(this->createResizedTexture(desc, cacheData,
                                             srcData, rowBytes,
                                             GrTexture::NeedsFiltering(resourceKey)));
    } else {
        texture.reset(fGpu->createTexture(desc, srcData, rowBytes));
    }

    if (NULL != texture) {
        fTextureCache->create(resourceKey, texture);
    }

    return texture;
}

GrTexture* GrContext::lockScratchTexture(const GrTextureDesc& inDesc,
                                         ScratchTexMatch match) {
    GrTextureDesc desc = inDesc;
    GrCacheData cacheData(GrCacheData::kScratch_CacheID);

    GrAssert((desc.fFlags & kRenderTarget_GrTextureFlagBit) ||
             !(desc.fFlags & kNoStencil_GrTextureFlagBit));

    if (kExact_ScratchTexMatch != match) {
        // bin by pow2 with a reasonable min
        static const int MIN_SIZE = 256;
        desc.fWidth  = GrMax(MIN_SIZE, GrNextPow2(desc.fWidth));
        desc.fHeight = GrMax(MIN_SIZE, GrNextPow2(desc.fHeight));
    }

    GrResource* resource = NULL;
    int origWidth = desc.fWidth;
    int origHeight = desc.fHeight;
    bool doubledW = false;
    bool doubledH = false;

    do {
        GrResourceKey key = GrTexture::ComputeKey(fGpu, NULL, desc, cacheData, true);
        resource = fTextureCache->find(key);
        // if we miss, relax the fit of the flags...
        // then try doubling width... then height.
        if (NULL != resource || kExact_ScratchTexMatch == match) {
            break;
        }
        // We no longer try to reuse textures that were previously used as render targets in
        // situations where no RT is needed; doing otherwise can confuse the video driver and
        // cause significant performance problems in some cases.
        if (desc.fFlags & kNoStencil_GrTextureFlagBit) {
            desc.fFlags = desc.fFlags & ~kNoStencil_GrTextureFlagBit;
        } else if (!doubledW) {
            desc.fFlags = inDesc.fFlags;
            desc.fWidth *= 2;
            doubledW = true;
        } else if (!doubledH) {
            desc.fFlags = inDesc.fFlags;
            desc.fWidth = origWidth;
            desc.fHeight *= 2;
            doubledH = true;
        } else {
            break;
        }

    } while (true);

    if (NULL == resource) {
        desc.fFlags = inDesc.fFlags;
        desc.fWidth = origWidth;
        desc.fHeight = origHeight;
        SkAutoTUnref<GrTexture> texture(fGpu->createTexture(desc, NULL, 0));
        if (NULL != texture) {
            GrResourceKey key = GrTexture::ComputeKey(fGpu, NULL,
                                                      texture->desc(),
                                                      cacheData,
                                                      true);
            fTextureCache->create(key, texture);
            resource = texture;
        }
    }

    // If the caller gives us the same desc/sampler twice we don't want
    // to return the same texture the second time (unless it was previously
    // released). So make it exclusive to hide it from future searches.
    if (NULL != resource) {
        fTextureCache->makeExclusive(resource->getCacheEntry());
    }

    return static_cast<GrTexture*>(resource);
}

void GrContext::addExistingTextureToCache(GrTexture* texture) {

    if (NULL == texture) {
        return;
    }

    // This texture should already have a cache entry since it was once
    // attached
    GrAssert(NULL != texture->getCacheEntry());

    // Conceptually, the cache entry is going to assume responsibility
    // for the creation ref.
    GrAssert(1 == texture->getRefCnt());

    // Since this texture came from an AutoScratchTexture it should
    // still be in the exclusive pile
    fTextureCache->makeNonExclusive(texture->getCacheEntry());

    this->purgeCache();
}


void GrContext::unlockScratchTexture(GrTexture* texture) {
    ASSERT_OWNED_RESOURCE(texture);
    GrAssert(NULL != texture->getCacheEntry());

    // If this is a scratch texture we detached it from the cache
    // while it was locked (to avoid two callers simultaneously getting
    // the same texture).
    if (GrTexture::IsScratchTexture(texture->getCacheEntry()->key())) {
        fTextureCache->makeNonExclusive(texture->getCacheEntry());
    }

    this->purgeCache();
}

void GrContext::purgeCache() {
    if (NULL != fTextureCache) {
        fTextureCache->purgeAsNeeded();
    }
}

GrTexture* GrContext::createUncachedTexture(const GrTextureDesc& descIn,
                                            void* srcData,
                                            size_t rowBytes) {
    GrTextureDesc descCopy = descIn;
    return fGpu->createTexture(descCopy, srcData, rowBytes);
}

void GrContext::getTextureCacheLimits(int* maxTextures,
                                      size_t* maxTextureBytes) const {
    fTextureCache->getLimits(maxTextures, maxTextureBytes);
}

void GrContext::setTextureCacheLimits(int maxTextures, size_t maxTextureBytes) {
    fTextureCache->setLimits(maxTextures, maxTextureBytes);
}

int GrContext::getMaxTextureSize() const {
    return fGpu->getCaps().maxTextureSize();
}

int GrContext::getMaxRenderTargetSize() const {
    return fGpu->getCaps().maxRenderTargetSize();
}

///////////////////////////////////////////////////////////////////////////////

GrTexture* GrContext::createPlatformTexture(const GrPlatformTextureDesc& desc) {
    return fGpu->createPlatformTexture(desc);
}

GrRenderTarget* GrContext::createPlatformRenderTarget(const GrPlatformRenderTargetDesc& desc) {
    return fGpu->createPlatformRenderTarget(desc);
}

///////////////////////////////////////////////////////////////////////////////

bool GrContext::supportsIndex8PixelConfig(const GrTextureParams* params,
                                          int width, int height) const {
    const GrDrawTarget::Caps& caps = fGpu->getCaps();
    if (!caps.eightBitPaletteSupport()) {
        return false;
    }

    bool isPow2 = GrIsPow2(width) && GrIsPow2(height);

    if (!isPow2) {
        bool tiled = NULL != params && params->isTiled();
        if (tiled && !caps.npotTextureTileSupport()) {
            return false;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

const GrClipData* GrContext::getClip() const {
    return fGpu->getClip();
}

void GrContext::setClip(const GrClipData* clipData) {
    fGpu->setClip(clipData);
    fDrawState->enableState(GrDrawState::kClip_StateBit);
}

////////////////////////////////////////////////////////////////////////////////

void GrContext::clear(const GrIRect* rect,
                      const GrColor color,
                      GrRenderTarget* target) {
    this->prepareToDraw(NULL, DEFAULT_BUFFERING)->clear(rect, color, target);
}

void GrContext::drawPaint(const GrPaint& paint) {
    // set rect to be big enough to fill the space, but not super-huge, so we
    // don't overflow fixed-point implementations
    GrRect r;
    r.setLTRB(0, 0,
              GrIntToScalar(getRenderTarget()->width()),
              GrIntToScalar(getRenderTarget()->height()));
    GrMatrix inverse;
    SkTLazy<GrPaint> tmpPaint;
    const GrPaint* p = &paint;
    AutoMatrix am;

    // We attempt to map r by the inverse matrix and draw that. mapRect will
    // map the four corners and bound them with a new rect. This will not
    // produce a correct result for some perspective matrices.
    if (!this->getMatrix().hasPerspective()) {
        if (!fDrawState->getViewInverse(&inverse)) {
            GrPrintf("Could not invert matrix\n");
            return;
        }
        inverse.mapRect(&r);
    } else {
        if (paint.hasTextureOrMask()) {
            tmpPaint.set(paint);
            p = tmpPaint.get();
            if (!tmpPaint.get()->preConcatSamplerMatricesWithInverse(fDrawState->getViewMatrix())) {
                GrPrintf("Could not invert matrix\n");
            }
        }
        am.set(this, GrMatrix::I());
    }
    // by definition this fills the entire clip, no need for AA
    if (paint.fAntiAlias) {
        if (!tmpPaint.isValid()) {
            tmpPaint.set(paint);
            p = tmpPaint.get();
        }
        GrAssert(p == tmpPaint.get());
        tmpPaint.get()->fAntiAlias = false;
    }
    this->drawRect(*p, r);
}

////////////////////////////////////////////////////////////////////////////////

namespace {
inline bool disable_coverage_aa_for_blend(GrDrawTarget* target) {
    return DISABLE_COVERAGE_AA_FOR_BLEND && !target->canApplyCoverage();
}
}

////////////////////////////////////////////////////////////////////////////////

/*  create a triangle strip that strokes the specified triangle. There are 8
 unique vertices, but we repreat the last 2 to close up. Alternatively we
 could use an indices array, and then only send 8 verts, but not sure that
 would be faster.
 */
static void setStrokeRectStrip(GrPoint verts[10], GrRect rect,
                               GrScalar width) {
    const GrScalar rad = GrScalarHalf(width);
    rect.sort();

    verts[0].set(rect.fLeft + rad, rect.fTop + rad);
    verts[1].set(rect.fLeft - rad, rect.fTop - rad);
    verts[2].set(rect.fRight - rad, rect.fTop + rad);
    verts[3].set(rect.fRight + rad, rect.fTop - rad);
    verts[4].set(rect.fRight - rad, rect.fBottom - rad);
    verts[5].set(rect.fRight + rad, rect.fBottom + rad);
    verts[6].set(rect.fLeft + rad, rect.fBottom - rad);
    verts[7].set(rect.fLeft - rad, rect.fBottom + rad);
    verts[8] = verts[0];
    verts[9] = verts[1];
}

/**
 * Returns true if the rects edges are integer-aligned.
 */
static bool isIRect(const GrRect& r) {
    return GrScalarIsInt(r.fLeft) && GrScalarIsInt(r.fTop) &&
           GrScalarIsInt(r.fRight) && GrScalarIsInt(r.fBottom);
}

static bool apply_aa_to_rect(GrDrawTarget* target,
                             const GrRect& rect,
                             GrScalar width,
                             const GrMatrix* matrix,
                             GrMatrix* combinedMatrix,
                             GrRect* devRect,
                             bool* useVertexCoverage) {
    // we use a simple coverage ramp to do aa on axis-aligned rects
    // we check if the rect will be axis-aligned, and the rect won't land on
    // integer coords.

    // we are keeping around the "tweak the alpha" trick because
    // it is our only hope for the fixed-pipe implementation.
    // In a shader implementation we can give a separate coverage input
    // TODO: remove this ugliness when we drop the fixed-pipe impl
    *useVertexCoverage = false;
    if (!target->canTweakAlphaForCoverage()) {
        if (disable_coverage_aa_for_blend(target)) {
#if GR_DEBUG
            //GrPrintf("Turning off AA to correctly apply blend.\n");
#endif
            return false;
        } else {
            *useVertexCoverage = true;
        }
    }
    const GrDrawState& drawState = target->getDrawState();
    if (drawState.getRenderTarget()->isMultisampled()) {
        return false;
    }

    if (0 == width && target->willUseHWAALines()) {
        return false;
    }

    if (!drawState.getViewMatrix().preservesAxisAlignment()) {
        return false;
    }

    if (NULL != matrix &&
        !matrix->preservesAxisAlignment()) {
        return false;
    }

    *combinedMatrix = drawState.getViewMatrix();
    if (NULL != matrix) {
        combinedMatrix->preConcat(*matrix);
        GrAssert(combinedMatrix->preservesAxisAlignment());
    }

    combinedMatrix->mapRect(devRect, rect);
    devRect->sort();

    if (width < 0) {
        return !isIRect(*devRect);
    } else {
        return true;
    }
}

void GrContext::drawRect(const GrPaint& paint,
                         const GrRect& rect,
                         GrScalar width,
                         const GrMatrix* matrix) {
    SK_TRACE_EVENT0("GrContext::drawRect");

    GrDrawTarget* target = this->prepareToDraw(&paint, DEFAULT_BUFFERING);
    GrDrawState::AutoStageDisable atr(fDrawState);

    GrRect devRect = rect;
    GrMatrix combinedMatrix;
    bool useVertexCoverage;
    bool needAA = paint.fAntiAlias &&
                  !this->getRenderTarget()->isMultisampled();
    bool doAA = needAA && apply_aa_to_rect(target, rect, width, matrix,
                                           &combinedMatrix, &devRect,
                                           &useVertexCoverage);

    if (doAA) {
        GrDrawTarget::AutoDeviceCoordDraw adcd(target);
        if (!adcd.succeeded()) {
            return;
        }
        if (width >= 0) {
            GrVec strokeSize;;
            if (width > 0) {
                strokeSize.set(width, width);
                combinedMatrix.mapVectors(&strokeSize, 1);
                strokeSize.setAbs(strokeSize);
            } else {
                strokeSize.set(GR_Scalar1, GR_Scalar1);
            }
            fAARectRenderer->strokeAARect(this->getGpu(), target, devRect,
                                         strokeSize, useVertexCoverage);
        } else {
            fAARectRenderer->fillAARect(this->getGpu(), target,
                                       devRect, useVertexCoverage);
        }
        return;
    }

    if (width >= 0) {
        // TODO: consider making static vertex buffers for these cases.
        // Hairline could be done by just adding closing vertex to
        // unitSquareVertexBuffer()

        static const int worstCaseVertCount = 10;
        GrDrawTarget::AutoReleaseGeometry geo(target, 0, worstCaseVertCount, 0);

        if (!geo.succeeded()) {
            GrPrintf("Failed to get space for vertices!\n");
            return;
        }

        GrPrimitiveType primType;
        int vertCount;
        GrPoint* vertex = geo.positions();

        if (width > 0) {
            vertCount = 10;
            primType = kTriangleStrip_GrPrimitiveType;
            setStrokeRectStrip(vertex, rect, width);
        } else {
            // hairline
            vertCount = 5;
            primType = kLineStrip_GrPrimitiveType;
            vertex[0].set(rect.fLeft, rect.fTop);
            vertex[1].set(rect.fRight, rect.fTop);
            vertex[2].set(rect.fRight, rect.fBottom);
            vertex[3].set(rect.fLeft, rect.fBottom);
            vertex[4].set(rect.fLeft, rect.fTop);
        }

        GrDrawState::AutoViewMatrixRestore avmr;
        if (NULL != matrix) {
            GrDrawState* drawState = target->drawState();
            avmr.set(drawState);
            drawState->preConcatViewMatrix(*matrix);
            drawState->preConcatSamplerMatrices(*matrix);
        }

        target->drawNonIndexed(primType, 0, vertCount);
    } else {
#if GR_STATIC_RECT_VB
            const GrVertexBuffer* sqVB = fGpu->getUnitSquareVertexBuffer();
            if (NULL == sqVB) {
                GrPrintf("Failed to create static rect vb.\n");
                return;
            }
            target->setVertexSourceToBuffer(0, sqVB);
            GrDrawState* drawState = target->drawState();
            GrDrawState::AutoViewMatrixRestore avmr(drawState);
            GrMatrix m;
            m.setAll(rect.width(),    0,             rect.fLeft,
                        0,            rect.height(), rect.fTop,
                        0,            0,             GrMatrix::I()[8]);

            if (NULL != matrix) {
                m.postConcat(*matrix);
            }
            drawState->preConcatViewMatrix(m);
            drawState->preConcatSamplerMatrices(m);

            target->drawNonIndexed(kTriangleFan_GrPrimitiveType, 0, 4);
#else
            target->drawSimpleRect(rect, matrix);
#endif
    }
}

void GrContext::drawRectToRect(const GrPaint& paint,
                               const GrRect& dstRect,
                               const GrRect& srcRect,
                               const GrMatrix* dstMatrix,
                               const GrMatrix* srcMatrix) {
    SK_TRACE_EVENT0("GrContext::drawRectToRect");

    // srcRect refers to paint's first texture
    if (!paint.isTextureStageEnabled(0)) {
        drawRect(paint, dstRect, -1, dstMatrix);
        return;
    }

    GrDrawTarget* target = this->prepareToDraw(&paint, DEFAULT_BUFFERING);

#if GR_STATIC_RECT_VB
    GrDrawState::AutoStageDisable atr(fDrawState);
    GrDrawState* drawState = target->drawState();
    GrDrawState::AutoViewMatrixRestore avmr(drawState);

    GrMatrix m;

    m.setAll(dstRect.width(), 0,                dstRect.fLeft,
             0,               dstRect.height(), dstRect.fTop,
             0,               0,                GrMatrix::I()[8]);
    if (NULL != dstMatrix) {
        m.postConcat(*dstMatrix);
    }
    drawState->preConcatViewMatrix(m);

    // we explicitly setup the correct coords for the first stage. The others
    // must know about the view matrix change.
    for (int s = 1; s < GrPaint::kTotalStages; ++s) {
        if (drawState->isStageEnabled(s)) {
            drawState->sampler(s)->preConcatMatrix(m);
        }
    }

    m.setAll(srcRect.width(), 0,                srcRect.fLeft,
             0,               srcRect.height(), srcRect.fTop,
             0,               0,                GrMatrix::I()[8]);
    if (NULL != srcMatrix) {
        m.postConcat(*srcMatrix);
    }
    drawState->sampler(GrPaint::kFirstTextureStage)->preConcatMatrix(m);

    const GrVertexBuffer* sqVB = fGpu->getUnitSquareVertexBuffer();
    if (NULL == sqVB) {
        GrPrintf("Failed to create static rect vb.\n");
        return;
    }
    target->setVertexSourceToBuffer(0, sqVB);
    target->drawNonIndexed(kTriangleFan_GrPrimitiveType, 0, 4);
#else
    GrDrawState::AutoStageDisable atr(fDrawState);

    const GrRect* srcRects[GrDrawState::kNumStages] = {NULL};
    const GrMatrix* srcMatrices[GrDrawState::kNumStages] = {NULL};
    srcRects[0] = &srcRect;
    srcMatrices[0] = srcMatrix;

    target->drawRect(dstRect, dstMatrix, srcRects, srcMatrices);
#endif
}

void GrContext::drawVertices(const GrPaint& paint,
                             GrPrimitiveType primitiveType,
                             int vertexCount,
                             const GrPoint positions[],
                             const GrPoint texCoords[],
                             const GrColor colors[],
                             const uint16_t indices[],
                             int indexCount) {
    SK_TRACE_EVENT0("GrContext::drawVertices");

    GrDrawTarget::AutoReleaseGeometry geo;

    GrDrawTarget* target = this->prepareToDraw(&paint, DEFAULT_BUFFERING);
    GrDrawState::AutoStageDisable atr(fDrawState);

    GrVertexLayout layout = 0;
    if (NULL != texCoords) {
        layout |= GrDrawTarget::StageTexCoordVertexLayoutBit(0, 0);
    }
    if (NULL != colors) {
        layout |= GrDrawTarget::kColor_VertexLayoutBit;
    }
    int vertexSize = GrDrawTarget::VertexSize(layout);

    if (sizeof(GrPoint) != vertexSize) {
        if (!geo.set(target, layout, vertexCount, 0)) {
            GrPrintf("Failed to get space for vertices!\n");
            return;
        }
        int texOffsets[GrDrawState::kMaxTexCoords];
        int colorOffset;
        GrDrawTarget::VertexSizeAndOffsetsByIdx(layout,
                                                texOffsets,
                                                &colorOffset,
                                                NULL,
                                                NULL);
        void* curVertex = geo.vertices();

        for (int i = 0; i < vertexCount; ++i) {
            *((GrPoint*)curVertex) = positions[i];

            if (texOffsets[0] > 0) {
                *(GrPoint*)((intptr_t)curVertex + texOffsets[0]) = texCoords[i];
            }
            if (colorOffset > 0) {
                *(GrColor*)((intptr_t)curVertex + colorOffset) = colors[i];
            }
            curVertex = (void*)((intptr_t)curVertex + vertexSize);
        }
    } else {
        target->setVertexSourceToArray(layout, positions, vertexCount);
    }

    // we don't currently apply offscreen AA to this path. Need improved
    // management of GrDrawTarget's geometry to avoid copying points per-tile.

    if (NULL != indices) {
        target->setIndexSourceToArray(indices, indexCount);
        target->drawIndexed(primitiveType, 0, 0, vertexCount, indexCount);
    } else {
        target->drawNonIndexed(primitiveType, 0, vertexCount);
    }
}

///////////////////////////////////////////////////////////////////////////////
namespace {

struct CircleVertex {
    GrPoint fPos;
    GrPoint fCenter;
    GrScalar fOuterRadius;
    GrScalar fInnerRadius;
};

/* Returns true if will map a circle to another circle. This can be true
 * if the matrix only includes square-scale, rotation, translation.
 */
inline bool isSimilarityTransformation(const SkMatrix& matrix,
                                       SkScalar tol = SK_ScalarNearlyZero) {
    if (matrix.isIdentity() || matrix.getType() == SkMatrix::kTranslate_Mask) {
        return true;
    }
    if (matrix.hasPerspective()) {
        return false;
    }

    SkScalar mx = matrix.get(SkMatrix::kMScaleX);
    SkScalar sx = matrix.get(SkMatrix::kMSkewX);
    SkScalar my = matrix.get(SkMatrix::kMScaleY);
    SkScalar sy = matrix.get(SkMatrix::kMSkewY);

    if (mx == 0 && sx == 0 && my == 0 && sy == 0) {
        return false;
    }

    // it has scales or skews, but it could also be rotation, check it out.
    SkVector vec[2];
    vec[0].set(mx, sx);
    vec[1].set(sy, my);

    return SkScalarNearlyZero(vec[0].dot(vec[1]), SkScalarSquare(tol)) &&
           SkScalarNearlyEqual(vec[0].lengthSqd(), vec[1].lengthSqd(),
                SkScalarSquare(tol));
}

}

// TODO: strokeWidth can't be larger than zero right now.
// It will be fixed when drawPath() can handle strokes.
void GrContext::drawOval(const GrPaint& paint,
                         const GrRect& rect,
                         SkScalar strokeWidth) {
    GrAssert(strokeWidth <= 0);
    if (!isSimilarityTransformation(this->getMatrix()) ||
        !paint.fAntiAlias ||
        rect.height() != rect.width()) {
        SkPath path;
        path.addOval(rect);
        GrPathFill fill = (strokeWidth == 0) ?
                           kHairLine_GrPathFill : kWinding_GrPathFill;
        this->internalDrawPath(paint, path, fill, NULL);
        return;
    }

    GrDrawTarget* target = this->prepareToDraw(&paint, DEFAULT_BUFFERING);

    GrDrawState* drawState = target->drawState();
    GrDrawState::AutoStageDisable atr(fDrawState);
    const GrMatrix vm = drawState->getViewMatrix();

    const GrRenderTarget* rt = drawState->getRenderTarget();
    if (NULL == rt) {
        return;
    }

    GrDrawTarget::AutoDeviceCoordDraw adcd(target);
    if (!adcd.succeeded()) {
        return;
    }

    GrVertexLayout layout = GrDrawTarget::kEdge_VertexLayoutBit;
    GrAssert(sizeof(CircleVertex) == GrDrawTarget::VertexSize(layout));

    GrPoint center = GrPoint::Make(rect.centerX(), rect.centerY());
    GrScalar radius = SkScalarHalf(rect.width());

    vm.mapPoints(&center, 1);
    radius = vm.mapRadius(radius);

    GrScalar outerRadius = radius;
    GrScalar innerRadius = 0;
    SkScalar halfWidth = 0;
    if (strokeWidth == 0) {
        halfWidth = SkScalarHalf(SK_Scalar1);

        outerRadius += halfWidth;
        innerRadius = SkMaxScalar(0, radius - halfWidth);
    }

    GrDrawTarget::AutoReleaseGeometry geo(target, layout, 4, 0);
    if (!geo.succeeded()) {
        GrPrintf("Failed to get space for vertices!\n");
        return;
    }

    CircleVertex* verts = reinterpret_cast<CircleVertex*>(geo.vertices());

    // The fragment shader will extend the radius out half a pixel
    // to antialias. Expand the drawn rect here so all the pixels
    // will be captured.
    SkScalar L = center.fX - outerRadius - SkFloatToScalar(0.5f);
    SkScalar R = center.fX + outerRadius + SkFloatToScalar(0.5f);
    SkScalar T = center.fY - outerRadius - SkFloatToScalar(0.5f);
    SkScalar B = center.fY + outerRadius + SkFloatToScalar(0.5f);

    verts[0].fPos = SkPoint::Make(L, T);
    verts[1].fPos = SkPoint::Make(R, T);
    verts[2].fPos = SkPoint::Make(L, B);
    verts[3].fPos = SkPoint::Make(R, B);

    for (int i = 0; i < 4; ++i) {
        // this goes to fragment shader, it should be in y-points-up space.
        verts[i].fCenter = SkPoint::Make(center.fX, rt->height() - center.fY);

        verts[i].fOuterRadius = outerRadius;
        verts[i].fInnerRadius = innerRadius;
    }

    drawState->setVertexEdgeType(GrDrawState::kCircle_EdgeType);
    target->drawNonIndexed(kTriangleStrip_GrPrimitiveType, 0, 4);
}

void GrContext::drawPath(const GrPaint& paint, const SkPath& path,
                         GrPathFill fill, const GrPoint* translate) {

    if (path.isEmpty()) {
       if (GrIsFillInverted(fill)) {
           this->drawPaint(paint);
       }
       return;
    }

    SkRect ovalRect;
    if (!GrIsFillInverted(fill) && path.isOval(&ovalRect)) {
        if (translate) {
            ovalRect.offset(*translate);
        }
        SkScalar width = (fill == kHairLine_GrPathFill) ? 0 : -SK_Scalar1;
        this->drawOval(paint, ovalRect, width);
        return;
    }

    internalDrawPath(paint, path, fill, translate);
}

void GrContext::internalDrawPath(const GrPaint& paint, const SkPath& path,
                                 GrPathFill fill, const GrPoint* translate) {

    // Note that below we may sw-rasterize the path into a scratch texture.
    // Scratch textures can be recycled after they are returned to the texture
    // cache. This presents a potential hazard for buffered drawing. However,
    // the writePixels that uploads to the scratch will perform a flush so we're
    // OK.
    GrDrawTarget* target = this->prepareToDraw(&paint, DEFAULT_BUFFERING);
    GrDrawState::AutoStageDisable atr(fDrawState);

    bool prAA = paint.fAntiAlias && !this->getRenderTarget()->isMultisampled();

    // An Assumption here is that path renderer would use some form of tweaking
    // the src color (either the input alpha or in the frag shader) to implement
    // aa. If we have some future driver-mojo path AA that can do the right
    // thing WRT to the blend then we'll need some query on the PR.
    if (disable_coverage_aa_for_blend(target)) {
#if GR_DEBUG
        //GrPrintf("Turning off AA to correctly apply blend.\n");
#endif
        prAA = false;
    }

    GrPathRenderer* pr = this->getPathRenderer(path, fill, target, prAA, true);
    if (NULL == pr) {
#if GR_DEBUG
        GrPrintf("Unable to find path renderer compatible with path.\n");
#endif
        return;
    }

    pr->drawPath(path, fill, translate, target, prAA);
}

////////////////////////////////////////////////////////////////////////////////

void GrContext::flush(int flagsBitfield) {
    if (kDiscard_FlushBit & flagsBitfield) {
        fDrawBuffer->reset();
    } else {
        this->flushDrawBuffer();
    }
    if (kForceCurrentRenderTarget_FlushBit & flagsBitfield) {
        fGpu->forceRenderTargetFlush();
    }
}

void GrContext::flushDrawBuffer() {
    if (fDrawBuffer) {
        // With addition of the AA clip path, flushing the draw buffer can
        // result in the generation of an AA clip mask. During this
        // process the SW path renderer may be invoked which recusively
        // calls this method (via internalWriteTexturePixels) creating
        // infinite recursion
        GrInOrderDrawBuffer* temp = fDrawBuffer;
        fDrawBuffer = NULL;

        temp->flushTo(fGpu);

        fDrawBuffer = temp;
    }
}

void GrContext::writeTexturePixels(GrTexture* texture,
                                   int left, int top, int width, int height,
                                   GrPixelConfig config, const void* buffer, size_t rowBytes,
                                   uint32_t flags) {
    SK_TRACE_EVENT0("GrContext::writeTexturePixels");
    ASSERT_OWNED_RESOURCE(texture);

    // TODO: use scratch texture to perform conversion
    if (kUnpremul_PixelOpsFlag & flags) {
        return;
    }
    if (!(kDontFlush_PixelOpsFlag & flags)) {
        this->flush();
    }

    fGpu->writeTexturePixels(texture, left, top, width, height,
                             config, buffer, rowBytes);
}

bool GrContext::readTexturePixels(GrTexture* texture,
                                  int left, int top, int width, int height,
                                  GrPixelConfig config, void* buffer, size_t rowBytes,
                                  uint32_t flags) {
    SK_TRACE_EVENT0("GrContext::readTexturePixels");
    ASSERT_OWNED_RESOURCE(texture);

    // TODO: code read pixels for textures that aren't also rendertargets
    GrRenderTarget* target = texture->asRenderTarget();
    if (NULL != target) {
        return this->readRenderTargetPixels(target,
                                            left, top, width, height,
                                            config, buffer, rowBytes,
                                            flags);
    } else {
        return false;
    }
}

#include "SkConfig8888.h"

namespace {
/**
 * Converts a GrPixelConfig to a SkCanvas::Config8888. Only byte-per-channel
 * formats are representable as Config8888 and so the function returns false
 * if the GrPixelConfig has no equivalent Config8888.
 */
bool grconfig_to_config8888(GrPixelConfig config,
                            bool unpremul,
                            SkCanvas::Config8888* config8888) {
    switch (config) {
        case kRGBA_8888_GrPixelConfig:
            if (unpremul) {
                *config8888 = SkCanvas::kRGBA_Unpremul_Config8888;
            } else {
                *config8888 = SkCanvas::kRGBA_Premul_Config8888;
            }
            return true;
        case kBGRA_8888_GrPixelConfig:
            if (unpremul) {
                *config8888 = SkCanvas::kBGRA_Unpremul_Config8888;
            } else {
                *config8888 = SkCanvas::kBGRA_Premul_Config8888;
            }
            return true;
        default:
            return false;
    }
}

// It returns a configuration with where the byte position of the R & B components are swapped in
// relation to the input config. This should only be called with the result of
// grconfig_to_config8888 as it will fail for other configs.
SkCanvas::Config8888 swap_config8888_red_and_blue(SkCanvas::Config8888 config8888) {
    switch (config8888) {
        case SkCanvas::kBGRA_Premul_Config8888:
            return SkCanvas::kRGBA_Premul_Config8888;
        case SkCanvas::kBGRA_Unpremul_Config8888:
            return SkCanvas::kRGBA_Unpremul_Config8888;
        case SkCanvas::kRGBA_Premul_Config8888:
            return SkCanvas::kBGRA_Premul_Config8888;
        case SkCanvas::kRGBA_Unpremul_Config8888:
            return SkCanvas::kBGRA_Unpremul_Config8888;
        default:
            GrCrash("Unexpected input");
            return SkCanvas::kBGRA_Unpremul_Config8888;;
    }
}
}

bool GrContext::readRenderTargetPixels(GrRenderTarget* target,
                                       int left, int top, int width, int height,
                                       GrPixelConfig config, void* buffer, size_t rowBytes,
                                       uint32_t flags) {
    SK_TRACE_EVENT0("GrContext::readRenderTargetPixels");
    ASSERT_OWNED_RESOURCE(target);

    if (NULL == target) {
        target = fDrawState->getRenderTarget();
        if (NULL == target) {
            return false;
        }
    }

    if (!(kDontFlush_PixelOpsFlag & flags)) {
        this->flush();
    }

    // Determine which conversions have to be applied: flipY, swapRAnd, and/or unpremul.

    // If fGpu->readPixels would incur a y-flip cost then we will read the pixels upside down. We'll
    // either do the flipY by drawing into a scratch with a matrix or on the cpu after the read.
    bool flipY = fGpu->readPixelsWillPayForYFlip(target, left, top,
                                                 width, height, config,
                                                 rowBytes);
    bool swapRAndB = fGpu->preferredReadPixelsConfig(config) == GrPixelConfigSwapRAndB(config);

    bool unpremul = SkToBool(kUnpremul_PixelOpsFlag & flags);

    // flipY will get set to false when it is handled below using a scratch. However, in that case
    // we still want to do the read upside down.
    bool readUpsideDown = flipY;

    if (unpremul && kRGBA_8888_GrPixelConfig != config && kBGRA_8888_GrPixelConfig != config) {
        // The unpremul flag is only allowed for these two configs.
        return false;
    }

    GrPixelConfig readConfig;
    if (swapRAndB) {
        readConfig = GrPixelConfigSwapRAndB(config);
        GrAssert(kUnknown_GrPixelConfig != config);
    } else {
        readConfig = config;
    }

    // If the src is a texture and we would have to do conversions after read pixels, we instead
    // do the conversions by drawing the src to a scratch texture. If we handle any of the
    // conversions in the draw we set the corresponding bool to false so that we don't reapply it
    // on the read back pixels.
    GrTexture* src = target->asTexture();
    GrAutoScratchTexture ast;
    if (NULL != src && (swapRAndB || unpremul || flipY)) {
        // Make the scratch a render target because we don't have a robust readTexturePixels as of
        // yet. It calls this function.
        GrTextureDesc desc;
        desc.fFlags = kRenderTarget_GrTextureFlagBit;
        desc.fWidth = width;
        desc.fHeight = height;
        desc.fConfig = readConfig;

        // When a full readback is faster than a partial we could always make the scratch exactly
        // match the passed rect. However, if we see many different size rectangles we will trash
        // our texture cache and pay the cost of creating and destroying many textures. So, we only
        // request an exact match when the caller is reading an entire RT.
        ScratchTexMatch match = kApprox_ScratchTexMatch;
        if (0 == left &&
            0 == top &&
            target->width() == width &&
            target->height() == height &&
            fGpu->fullReadPixelsIsFasterThanPartial()) {
            match = kExact_ScratchTexMatch;
        }
        ast.set(this, desc, match);
        GrTexture* texture = ast.texture();
        if (texture) {
            SkAutoTUnref<GrCustomStage> stage;
            if (unpremul) {
                stage.reset(this->createPMToUPMEffect(src, swapRAndB));
            }
            // If we failed to create a PM->UPM effect and have no other conversions to perform then
            // there is no longer any point to using the scratch.
            if (NULL != stage || flipY || swapRAndB) {
                if (NULL == stage) {
                    stage.reset(GrConfigConversionEffect::Create(src, swapRAndB));
                    GrAssert(NULL != stage);
                } else {
                    unpremul = false; // we will handle the UPM conversion in the draw
                }
                swapRAndB = false; // we will handle the swap in the draw.

                GrDrawTarget::AutoStateRestore asr(fGpu, GrDrawTarget::kReset_ASRInit);
                GrDrawState* drawState = fGpu->drawState();
                drawState->setRenderTarget(texture->asRenderTarget());
                GrMatrix matrix;
                if (flipY) {
                    matrix.setTranslate(SK_Scalar1 * left,
                                        SK_Scalar1 * (top + height));
                    matrix.set(GrMatrix::kMScaleY, -GR_Scalar1);
                    flipY = false; // the y flip will be handled in the draw
                } else {
                    matrix.setTranslate(SK_Scalar1 *left, SK_Scalar1 *top);
                }
                matrix.postIDiv(src->width(), src->height());
                drawState->sampler(0)->reset(matrix);
                drawState->sampler(0)->setCustomStage(stage);
                GrRect rect = GrRect::MakeWH(GrIntToScalar(width), GrIntToScalar(height));
                fGpu->drawSimpleRect(rect, NULL);
                // we want to read back from the scratch's origin
                left = 0;
                top = 0;
                target = texture->asRenderTarget();
            }
        }
    }
    if (!fGpu->readPixels(target,
                          left, top, width, height,
                          readConfig, buffer, rowBytes, readUpsideDown)) {
        return false;
    }
    // Perform any conversions we weren't able to perfom using a scratch texture.
    if (unpremul || swapRAndB || flipY) {
        SkCanvas::Config8888 srcC8888;
        SkCanvas::Config8888 dstC8888;
        bool c8888IsValid = grconfig_to_config8888(config, false, &srcC8888);
        grconfig_to_config8888(config, unpremul, &dstC8888);
        if (swapRAndB) {
            GrAssert(c8888IsValid); // we should only do r/b swap on 8888 configs
            srcC8888 = swap_config8888_red_and_blue(srcC8888);
        }
        if (flipY) {
            size_t tightRB = width * GrBytesPerPixel(config);
            if (0 == rowBytes) {
                rowBytes = tightRB;
            }
            SkAutoSTMalloc<256, uint8_t> tempRow(tightRB);
            intptr_t top = reinterpret_cast<intptr_t>(buffer);
            intptr_t bot = top + (height - 1) * rowBytes;
            while (top < bot) {
                uint32_t* t = reinterpret_cast<uint32_t*>(top);
                uint32_t* b = reinterpret_cast<uint32_t*>(bot);
                uint32_t* temp = reinterpret_cast<uint32_t*>(tempRow.get());
                memcpy(temp, t, tightRB);
                if (c8888IsValid) {
                    SkConvertConfig8888Pixels(t, tightRB, dstC8888,
                                              b, tightRB, srcC8888,
                                              width, 1);
                    SkConvertConfig8888Pixels(b, tightRB, dstC8888,
                                              temp, tightRB, srcC8888,
                                              width, 1);
                } else {
                    memcpy(t, b, tightRB);
                    memcpy(b, temp, tightRB);
                }
                top += rowBytes;
                bot -= rowBytes;
            }
            // The above loop does nothing on the middle row when height is odd.
            if (top == bot && c8888IsValid && dstC8888 != srcC8888) {
                uint32_t* mid = reinterpret_cast<uint32_t*>(top);
                SkConvertConfig8888Pixels(mid, tightRB, dstC8888, mid, tightRB, srcC8888, width, 1);
            }
        } else {
            // if we aren't flipping Y then we have no reason to be here other than doing
            // conversions for 8888 (r/b swap or upm).
            GrAssert(c8888IsValid);
            uint32_t* b32 = reinterpret_cast<uint32_t*>(buffer);
            SkConvertConfig8888Pixels(b32, rowBytes, dstC8888,
                                      b32, rowBytes, srcC8888,
                                      width, height);
        }
    }
    return true;
}

void GrContext::resolveRenderTarget(GrRenderTarget* target) {
    GrAssert(target);
    ASSERT_OWNED_RESOURCE(target);
    // In the future we may track whether there are any pending draws to this
    // target. We don't today so we always perform a flush. We don't promise
    // this to our clients, though.
    this->flush();
    fGpu->resolveRenderTarget(target);
}

void GrContext::copyTexture(GrTexture* src, GrRenderTarget* dst) {
    if (NULL == src || NULL == dst) {
        return;
    }
    ASSERT_OWNED_RESOURCE(src);

    // Writes pending to the source texture are not tracked, so a flush
    // is required to ensure that the copy captures the most recent contents
    // of the source texture. See similar behaviour in
    // GrContext::resolveRenderTarget.
    this->flush();

    GrDrawTarget::AutoStateRestore asr(fGpu, GrDrawTarget::kReset_ASRInit);
    GrDrawState* drawState = fGpu->drawState();
    drawState->setRenderTarget(dst);
    GrMatrix sampleM;
    sampleM.setIDiv(src->width(), src->height());
    drawState->sampler(0)->reset(sampleM);
    drawState->createTextureEffect(0, src);
    SkRect rect = SkRect::MakeXYWH(0, 0,
                                   SK_Scalar1 * src->width(),
                                   SK_Scalar1 * src->height());
    fGpu->drawSimpleRect(rect, NULL);
}

void GrContext::writeRenderTargetPixels(GrRenderTarget* target,
                                        int left, int top, int width, int height,
                                        GrPixelConfig config,
                                        const void* buffer,
                                        size_t rowBytes,
                                        uint32_t flags) {
    SK_TRACE_EVENT0("GrContext::writeRenderTargetPixels");
    ASSERT_OWNED_RESOURCE(target);

    if (NULL == target) {
        target = fDrawState->getRenderTarget();
        if (NULL == target) {
            return;
        }
    }

    // TODO: when underlying api has a direct way to do this we should use it (e.g. glDrawPixels on
    // desktop GL).

    // We will always call some form of writeTexturePixels and we will pass our flags on to it.
    // Thus, we don't perform a flush here since that call will do it (if the kNoFlush flag isn't
    // set.)

    // If the RT is also a texture and we don't have to premultiply then take the texture path.
    // We expect to be at least as fast or faster since it doesn't use an intermediate texture as
    // we do below.

#if !GR_MAC_BUILD
    // At least some drivers on the Mac get confused when glTexImage2D is called on a texture
    // attached to an FBO. The FBO still sees the old image. TODO: determine what OS versions and/or
    // HW is affected.
    if (NULL != target->asTexture() && !(kUnpremul_PixelOpsFlag & flags)) {
        this->writeTexturePixels(target->asTexture(),
                                 left, top, width, height,
                                 config, buffer, rowBytes, flags);
        return;
    }
#endif
    SkAutoTUnref<GrCustomStage> stage;
    bool swapRAndB = (fGpu->preferredReadPixelsConfig(config) == GrPixelConfigSwapRAndB(config));

    GrPixelConfig textureConfig;
    if (swapRAndB) {
        textureConfig = GrPixelConfigSwapRAndB(config);
    } else {
        textureConfig = config;
    }

    GrTextureDesc desc;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = textureConfig;
    GrAutoScratchTexture ast(this, desc);
    GrTexture* texture = ast.texture();
    if (NULL == texture) {
        return;
    }
    // allocate a tmp buffer and sw convert the pixels to premul
    SkAutoSTMalloc<128 * 128, uint32_t> tmpPixels(0);

    if (kUnpremul_PixelOpsFlag & flags) {
        if (kRGBA_8888_GrPixelConfig != config && kBGRA_8888_GrPixelConfig != config) {
            return;
        }
        stage.reset(this->createUPMToPMEffect(texture, swapRAndB));
        if (NULL == stage) {
            SkCanvas::Config8888 srcConfig8888, dstConfig8888;
            GR_DEBUGCODE(bool success = )
            grconfig_to_config8888(config, true, &srcConfig8888);
            GrAssert(success);
            GR_DEBUGCODE(success = )
            grconfig_to_config8888(config, false, &dstConfig8888);
            GrAssert(success);
            const uint32_t* src = reinterpret_cast<const uint32_t*>(buffer);
            tmpPixels.reset(width * height);
            SkConvertConfig8888Pixels(tmpPixels.get(), 4 * width, dstConfig8888,
                                      src, rowBytes, srcConfig8888,
                                      width, height);
            buffer = tmpPixels.get();
            rowBytes = 4 * width;
        }
    }
    if (NULL == stage) {
        stage.reset(GrConfigConversionEffect::Create(texture, swapRAndB));
        GrAssert(NULL != stage);
    }

    this->writeTexturePixels(texture,
                             0, 0, width, height,
                             textureConfig, buffer, rowBytes,
                             flags & ~kUnpremul_PixelOpsFlag);

    GrDrawTarget::AutoStateRestore  asr(fGpu, GrDrawTarget::kReset_ASRInit);
    GrDrawState* drawState = fGpu->drawState();

    GrMatrix matrix;
    matrix.setTranslate(GrIntToScalar(left), GrIntToScalar(top));
    drawState->setViewMatrix(matrix);
    drawState->setRenderTarget(target);

    matrix.setIDiv(texture->width(), texture->height());
    drawState->sampler(0)->reset(matrix);
    drawState->sampler(0)->setCustomStage(stage);

    fGpu->drawSimpleRect(GrRect::MakeWH(SkIntToScalar(width), SkIntToScalar(height)), NULL);
}
////////////////////////////////////////////////////////////////////////////////

void GrContext::setPaint(const GrPaint& paint) {
    GrAssert(fDrawState->stagesDisabled());

    for (int i = 0; i < GrPaint::kMaxTextures; ++i) {
        int s = i + GrPaint::kFirstTextureStage;
        if (paint.isTextureStageEnabled(i)) {
            *fDrawState->sampler(s) = paint.getTextureSampler(i);
        }
    }

    fDrawState->setFirstCoverageStage(GrPaint::kFirstMaskStage);

    for (int i = 0; i < GrPaint::kMaxMasks; ++i) {
        int s = i + GrPaint::kFirstMaskStage;
        if (paint.isMaskStageEnabled(i)) {
            *fDrawState->sampler(s) = paint.getMaskSampler(i);
        }
    }

    // disable all stages not accessible via the paint
    for (int s = GrPaint::kTotalStages; s < GrDrawState::kNumStages; ++s) {
        fDrawState->disableStage(s);
    }

    fDrawState->setColor(paint.fColor);

    if (paint.fDither) {
        fDrawState->enableState(GrDrawState::kDither_StateBit);
    } else {
        fDrawState->disableState(GrDrawState::kDither_StateBit);
    }
    if (paint.fAntiAlias) {
        fDrawState->enableState(GrDrawState::kHWAntialias_StateBit);
    } else {
        fDrawState->disableState(GrDrawState::kHWAntialias_StateBit);
    }
    if (paint.fColorMatrixEnabled) {
        fDrawState->enableState(GrDrawState::kColorMatrix_StateBit);
        fDrawState->setColorMatrix(paint.fColorMatrix);
    } else {
        fDrawState->disableState(GrDrawState::kColorMatrix_StateBit);
    }
    fDrawState->setBlendFunc(paint.fSrcBlendCoeff, paint.fDstBlendCoeff);
    fDrawState->setColorFilter(paint.fColorFilterColor, paint.fColorFilterXfermode);
    fDrawState->setCoverage(paint.fCoverage);
#if GR_DEBUG_PARTIAL_COVERAGE_CHECK
    if ((paint.hasMask() || 0xff != paint.fCoverage) &&
        !fGpu->canApplyCoverage()) {
        GrPrintf("Partial pixel coverage will be incorrectly blended.\n");
    }
#endif
}

GrDrawTarget* GrContext::prepareToDraw(const GrPaint* paint, BufferedDraw buffered) {
    if (kNo_BufferedDraw == buffered && kYes_BufferedDraw == fLastDrawWasBuffered) {
        this->flushDrawBuffer();
        fLastDrawWasBuffered = kNo_BufferedDraw;
    }
    if (NULL != paint) {
        this->setPaint(*paint);
    }
    if (kYes_BufferedDraw == buffered) {
        fDrawBuffer->setClip(fGpu->getClip());
        fLastDrawWasBuffered = kYes_BufferedDraw;
        return fDrawBuffer;
    } else {
        GrAssert(kNo_BufferedDraw == buffered);
        return fGpu;
    }
}

/*
 * This method finds a path renderer that can draw the specified path on
 * the provided target.
 * Due to its expense, the software path renderer has split out so it can
 * can be individually allowed/disallowed via the "allowSW" boolean.
 */
GrPathRenderer* GrContext::getPathRenderer(const SkPath& path,
                                           GrPathFill fill,
                                           const GrDrawTarget* target,
                                           bool antiAlias,
                                           bool allowSW) {
    if (NULL == fPathRendererChain) {
        fPathRendererChain =
            SkNEW_ARGS(GrPathRendererChain,
                       (this, GrPathRendererChain::kNone_UsageFlag));
    }

    GrPathRenderer* pr = fPathRendererChain->getPathRenderer(path, fill,
                                                             target,
                                                             antiAlias);

    if (NULL == pr && allowSW) {
        if (NULL == fSoftwarePathRenderer) {
            fSoftwarePathRenderer = SkNEW_ARGS(GrSoftwarePathRenderer, (this));
        }

        pr = fSoftwarePathRenderer;
    }

    return pr;
}

////////////////////////////////////////////////////////////////////////////////

void GrContext::setRenderTarget(GrRenderTarget* target) {
    ASSERT_OWNED_RESOURCE(target);
    fDrawState->setRenderTarget(target);
}

GrRenderTarget* GrContext::getRenderTarget() {
    return fDrawState->getRenderTarget();
}

const GrRenderTarget* GrContext::getRenderTarget() const {
    return fDrawState->getRenderTarget();
}

bool GrContext::isConfigRenderable(GrPixelConfig config) const {
    return fGpu->isConfigRenderable(config);
}

const GrMatrix& GrContext::getMatrix() const {
    return fDrawState->getViewMatrix();
}

void GrContext::setMatrix(const GrMatrix& m) {
    fDrawState->setViewMatrix(m);
}

void GrContext::concatMatrix(const GrMatrix& m) const {
    fDrawState->preConcatViewMatrix(m);
}

static inline intptr_t setOrClear(intptr_t bits, int shift, intptr_t pred) {
    intptr_t mask = 1 << shift;
    if (pred) {
        bits |= mask;
    } else {
        bits &= ~mask;
    }
    return bits;
}

GrContext::GrContext(GrGpu* gpu) {
    ++THREAD_INSTANCE_COUNT;

    fGpu = gpu;
    fGpu->ref();
    fGpu->setContext(this);

    fDrawState = SkNEW(GrDrawState);
    fGpu->setDrawState(fDrawState);

    fPathRendererChain = NULL;
    fSoftwarePathRenderer = NULL;

    fTextureCache = SkNEW_ARGS(GrResourceCache,
                               (MAX_TEXTURE_CACHE_COUNT,
                                MAX_TEXTURE_CACHE_BYTES));
    fFontCache = SkNEW_ARGS(GrFontCache, (fGpu));

    fLastDrawWasBuffered = kNo_BufferedDraw;

    fDrawBuffer = NULL;
    fDrawBufferVBAllocPool = NULL;
    fDrawBufferIBAllocPool = NULL;

    fAARectRenderer = SkNEW(GrAARectRenderer);

    fDidTestPMConversions = false;

    this->setupDrawBuffer();
}

void GrContext::setupDrawBuffer() {

    GrAssert(NULL == fDrawBuffer);
    GrAssert(NULL == fDrawBufferVBAllocPool);
    GrAssert(NULL == fDrawBufferIBAllocPool);

    fDrawBufferVBAllocPool =
        SkNEW_ARGS(GrVertexBufferAllocPool, (fGpu, false,
                                    DRAW_BUFFER_VBPOOL_BUFFER_SIZE,
                                    DRAW_BUFFER_VBPOOL_PREALLOC_BUFFERS));
    fDrawBufferIBAllocPool =
        SkNEW_ARGS(GrIndexBufferAllocPool, (fGpu, false,
                                   DRAW_BUFFER_IBPOOL_BUFFER_SIZE,
                                   DRAW_BUFFER_IBPOOL_PREALLOC_BUFFERS));

    fDrawBuffer = SkNEW_ARGS(GrInOrderDrawBuffer, (fGpu,
                                          fDrawBufferVBAllocPool,
                                          fDrawBufferIBAllocPool));

    fDrawBuffer->setQuadIndexBuffer(this->getQuadIndexBuffer());
    if (fDrawBuffer) {
        fDrawBuffer->setAutoFlushTarget(fGpu);
        fDrawBuffer->setDrawState(fDrawState);
    }
}

GrDrawTarget* GrContext::getTextTarget(const GrPaint& paint) {
    return prepareToDraw(&paint, DEFAULT_BUFFERING);
}

const GrIndexBuffer* GrContext::getQuadIndexBuffer() const {
    return fGpu->getQuadIndexBuffer();
}

namespace {
void test_pm_conversions(GrContext* ctx, int* pmToUPMValue, int* upmToPMValue) {
    GrConfigConversionEffect::PMConversion pmToUPM;
    GrConfigConversionEffect::PMConversion upmToPM;
    GrConfigConversionEffect::TestForPreservingPMConversions(ctx, &pmToUPM, &upmToPM);
    *pmToUPMValue = pmToUPM;
    *upmToPMValue = upmToPM;
}
}

GrCustomStage* GrContext::createPMToUPMEffect(GrTexture* texture, bool swapRAndB) {
    if (!fDidTestPMConversions) {
        test_pm_conversions(this, &fPMToUPMConversion, &fUPMToPMConversion);
        fDidTestPMConversions = true;
    }
    GrConfigConversionEffect::PMConversion pmToUPM =
        static_cast<GrConfigConversionEffect::PMConversion>(fPMToUPMConversion);
    if (GrConfigConversionEffect::kNone_PMConversion != pmToUPM) {
        return GrConfigConversionEffect::Create(texture, swapRAndB, pmToUPM);
    } else {
        return NULL;
    }
}

GrCustomStage* GrContext::createUPMToPMEffect(GrTexture* texture, bool swapRAndB) {
    if (!fDidTestPMConversions) {
        test_pm_conversions(this, &fPMToUPMConversion, &fUPMToPMConversion);
        fDidTestPMConversions = true;
    }
    GrConfigConversionEffect::PMConversion upmToPM =
        static_cast<GrConfigConversionEffect::PMConversion>(fUPMToPMConversion);
    if (GrConfigConversionEffect::kNone_PMConversion != upmToPM) {
        return GrConfigConversionEffect::Create(texture, swapRAndB, upmToPM);
    } else {
        return NULL;
    }
}

GrTexture* GrContext::gaussianBlur(GrTexture* srcTexture,
                                   bool canClobberSrc,
                                   const SkRect& rect,
                                   float sigmaX, float sigmaY) {
    ASSERT_OWNED_RESOURCE(srcTexture);
    GrRenderTarget* oldRenderTarget = this->getRenderTarget();
    AutoMatrix avm(this, GrMatrix::I());
    SkIRect clearRect;
    int scaleFactorX, radiusX;
    int scaleFactorY, radiusY;
    sigmaX = adjust_sigma(sigmaX, &scaleFactorX, &radiusX);
    sigmaY = adjust_sigma(sigmaY, &scaleFactorY, &radiusY);

    SkRect srcRect(rect);
    scale_rect(&srcRect, 1.0f / scaleFactorX, 1.0f / scaleFactorY);
    srcRect.roundOut();
    scale_rect(&srcRect, static_cast<float>(scaleFactorX),
                         static_cast<float>(scaleFactorY));

    AutoClip acs(this, srcRect);

    GrAssert(kBGRA_8888_GrPixelConfig == srcTexture->config() ||
             kRGBA_8888_GrPixelConfig == srcTexture->config() ||
             kAlpha_8_GrPixelConfig == srcTexture->config());

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit | kNoStencil_GrTextureFlagBit;
    desc.fWidth = SkScalarFloorToInt(srcRect.width());
    desc.fHeight = SkScalarFloorToInt(srcRect.height());
    desc.fConfig = srcTexture->config();

    GrAutoScratchTexture temp1, temp2;
    GrTexture* dstTexture = temp1.set(this, desc);
    GrTexture* tempTexture = canClobberSrc ? srcTexture : temp2.set(this, desc);

    GrPaint paint;
    paint.reset();
    paint.textureSampler(0)->textureParams()->setBilerp(true);

    for (int i = 1; i < scaleFactorX || i < scaleFactorY; i *= 2) {
        paint.textureSampler(0)->matrix()->setIDiv(srcTexture->width(),
                                                   srcTexture->height());
        this->setRenderTarget(dstTexture->asRenderTarget());
        SkRect dstRect(srcRect);
        scale_rect(&dstRect, i < scaleFactorX ? 0.5f : 1.0f,
                            i < scaleFactorY ? 0.5f : 1.0f);
        paint.textureSampler(0)->setCustomStage(SkNEW_ARGS(GrSingleTextureEffect,
                                                           (srcTexture)))->unref();
        this->drawRectToRect(paint, dstRect, srcRect);
        srcRect = dstRect;
        srcTexture = dstTexture;
        SkTSwap(dstTexture, tempTexture);
    }

    SkIRect srcIRect;
    srcRect.roundOut(&srcIRect);

    if (sigmaX > 0.0f) {
        if (scaleFactorX > 1) {
            // Clear out a radius to the right of the srcRect to prevent the
            // X convolution from reading garbage.
            clearRect = SkIRect::MakeXYWH(srcIRect.fRight, srcIRect.fTop,
                                          radiusX, srcIRect.height());
            this->clear(&clearRect, 0x0);
        }

        this->setRenderTarget(dstTexture->asRenderTarget());
        GrDrawTarget* target = this->prepareToDraw(NULL, DEFAULT_BUFFERING);
        convolve_gaussian(target, srcTexture, srcRect, sigmaX, radiusX,
                          Gr1DKernelEffect::kX_Direction);
        srcTexture = dstTexture;
        SkTSwap(dstTexture, tempTexture);
    }

    if (sigmaY > 0.0f) {
        if (scaleFactorY > 1 || sigmaX > 0.0f) {
            // Clear out a radius below the srcRect to prevent the Y
            // convolution from reading garbage.
            clearRect = SkIRect::MakeXYWH(srcIRect.fLeft, srcIRect.fBottom,
                                          srcIRect.width(), radiusY);
            this->clear(&clearRect, 0x0);
        }

        this->setRenderTarget(dstTexture->asRenderTarget());
        GrDrawTarget* target = this->prepareToDraw(NULL, DEFAULT_BUFFERING);
        convolve_gaussian(target, srcTexture, srcRect, sigmaY, radiusY,
                          Gr1DKernelEffect::kY_Direction);
        srcTexture = dstTexture;
        SkTSwap(dstTexture, tempTexture);
    }

    if (scaleFactorX > 1 || scaleFactorY > 1) {
        // Clear one pixel to the right and below, to accommodate bilinear
        // upsampling.
        clearRect = SkIRect::MakeXYWH(srcIRect.fLeft, srcIRect.fBottom,
                                      srcIRect.width() + 1, 1);
        this->clear(&clearRect, 0x0);
        clearRect = SkIRect::MakeXYWH(srcIRect.fRight, srcIRect.fTop,
                                      1, srcIRect.height());
        this->clear(&clearRect, 0x0);
        // FIXME:  This should be mitchell, not bilinear.
        paint.textureSampler(0)->textureParams()->setBilerp(true);
        paint.textureSampler(0)->matrix()->setIDiv(srcTexture->width(),
                                                   srcTexture->height());
        this->setRenderTarget(dstTexture->asRenderTarget());
        paint.textureSampler(0)->setCustomStage(SkNEW_ARGS(GrSingleTextureEffect,
                                                           (srcTexture)))->unref();
        SkRect dstRect(srcRect);
        scale_rect(&dstRect, (float) scaleFactorX, (float) scaleFactorY);
        this->drawRectToRect(paint, dstRect, srcRect);
        srcRect = dstRect;
        srcTexture = dstTexture;
        SkTSwap(dstTexture, tempTexture);
    }
    this->setRenderTarget(oldRenderTarget);
    if (srcTexture == temp1.texture()) {
        return temp1.detach();
    } else if (srcTexture == temp2.texture()) {
        return temp2.detach();
    } else {
        srcTexture->ref();
        return srcTexture;
    }
}

///////////////////////////////////////////////////////////////////////////////
#if GR_CACHE_STATS
void GrContext::printCacheStats() const {
    fTextureCache->printStats();
}
#endif
