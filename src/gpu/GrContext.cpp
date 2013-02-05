
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
#include "SkStrokeRec.h"
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

static const size_t MAX_TEXTURE_CACHE_COUNT = 2048;
static const size_t MAX_TEXTURE_CACHE_BYTES = GR_DEFAULT_TEXTURE_CACHE_MB_LIMIT * 1024 * 1024;

static const size_t DRAW_BUFFER_VBPOOL_BUFFER_SIZE = 1 << 15;
static const int DRAW_BUFFER_VBPOOL_PREALLOC_BUFFERS = 4;

static const size_t DRAW_BUFFER_IBPOOL_BUFFER_SIZE = 1 << 11;
static const int DRAW_BUFFER_IBPOOL_PREALLOC_BUFFERS = 4;

#define ASSERT_OWNED_RESOURCE(R) GrAssert(!(R) || (R)->getContext() == this)

GrContext* GrContext::Create(GrBackend backend, GrBackendContext context) {
    GrContext* ctx = NULL;
    GrGpu* fGpu = GrGpu::Create(backend, context);
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
    for (int i = 0; i < fCleanUpData.count(); ++i) {
        (*fCleanUpData[i].fFunc)(this, fCleanUpData[i].fInfo);
    }

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
    SkAutoTUnref<GrEffectRef> conv(GrConvolutionEffect::CreateGaussian(texture,
                                                                       direction,
                                                                       radius,
                                                                       sigma));
    drawState->setEffect(0, conv);
    target->drawSimpleRect(rect, NULL);
}

}

////////////////////////////////////////////////////////////////////////////////

GrTexture* GrContext::findAndRefTexture(const GrTextureDesc& desc,
                                        const GrCacheID& cacheID,
                                        const GrTextureParams* params) {
    GrResourceKey resourceKey = GrTexture::ComputeKey(fGpu, params, desc, cacheID);
    GrResource* resource = fTextureCache->find(resourceKey);
    SkSafeRef(resource);
    return static_cast<GrTexture*>(resource);
}

bool GrContext::isTextureInCache(const GrTextureDesc& desc,
                                 const GrCacheID& cacheID,
                                 const GrTextureParams* params) const {
    GrResourceKey resourceKey = GrTexture::ComputeKey(fGpu, params, desc, cacheID);
    return fTextureCache->hasKey(resourceKey);
}

void GrContext::addStencilBuffer(GrStencilBuffer* sb) {
    ASSERT_OWNED_RESOURCE(sb);

    GrResourceKey resourceKey = GrStencilBuffer::ComputeKey(sb->width(),
                                                            sb->height(),
                                                            sb->numSamples());
    fTextureCache->addResource(resourceKey, sb);
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
                                           const GrCacheID& cacheID,
                                           void* srcData,
                                           size_t rowBytes,
                                           bool needsFiltering) {
    SkAutoTUnref<GrTexture> clampedTexture(this->findAndRefTexture(desc, cacheID, NULL));
    if (NULL == clampedTexture) {
        clampedTexture.reset(this->createTexture(NULL, desc, cacheID, srcData, rowBytes));

        if (NULL == clampedTexture) {
            return NULL;
        }
    }

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
        GrTextureParams params(SkShader::kClamp_TileMode, needsFiltering);
        drawState->createTextureEffect(0, clampedTexture, SkMatrix::I(), params);

        static const GrVertexLayout layout = GrDrawState::StageTexCoordVertexLayoutBit(0,0);
        GrDrawTarget::AutoReleaseGeometry arg(fGpu, layout, 4, 0);

        if (arg.succeeded()) {
            GrPoint* verts = (GrPoint*) arg.vertices();
            verts[0].setIRectFan(0, 0, texture->width(), texture->height(), 2 * sizeof(GrPoint));
            verts[1].setIRectFan(0, 0, 1, 1, 2 * sizeof(GrPoint));
            fGpu->drawNonIndexed(kTriangleFan_GrPrimitiveType, 0, 4);
        }
        texture->releaseRenderTarget();
    } else {
        // TODO: Our CPU stretch doesn't filter. But we create separate
        // stretched textures when the texture params is either filtered or
        // not. Either implement filtered stretch blit on CPU or just create
        // one when FBO case fails.

        rtDesc.fFlags = kNone_GrTextureFlags;
        // no longer need to clamp at min RT size.
        rtDesc.fWidth  = GrNextPow2(desc.fWidth);
        rtDesc.fHeight = GrNextPow2(desc.fHeight);
        int bpp = GrBytesPerPixel(desc.fConfig);
        SkAutoSMalloc<128*128*4> stretchedPixels(bpp * rtDesc.fWidth * rtDesc.fHeight);
        stretchImage(stretchedPixels.get(), rtDesc.fWidth, rtDesc.fHeight,
                     srcData, desc.fWidth, desc.fHeight, bpp);

        size_t stretchedRowBytes = rtDesc.fWidth * bpp;

        SkDEBUGCODE(GrTexture* texture = )fGpu->createTexture(rtDesc, stretchedPixels.get(), stretchedRowBytes);
        GrAssert(NULL != texture);
    }

    return texture;
}

GrTexture* GrContext::createTexture(const GrTextureParams* params,
                                    const GrTextureDesc& desc,
                                    const GrCacheID& cacheID,
                                    void* srcData,
                                    size_t rowBytes) {
    SK_TRACE_EVENT0("GrContext::createTexture");

#if GR_DUMP_TEXTURE_UPLOAD
    GrPrintf("GrContext::createTexture[%d %d]\n", desc.fWidth, desc.fHeight);
#endif

    GrResourceKey resourceKey = GrTexture::ComputeKey(fGpu, params, desc, cacheID);

    GrTexture* texture;
    if (GrTexture::NeedsResizing(resourceKey)) {
        texture = this->createResizedTexture(desc, cacheID,
                                             srcData, rowBytes,
                                             GrTexture::NeedsFiltering(resourceKey));
    } else {
        texture= fGpu->createTexture(desc, srcData, rowBytes);
    }

    if (NULL != texture) {
        fTextureCache->addResource(resourceKey, texture);
    }

    return texture;
}

GrTexture* GrContext::lockAndRefScratchTexture(const GrTextureDesc& inDesc, ScratchTexMatch match) {
    GrTextureDesc desc = inDesc;

    GrAssert((desc.fFlags & kRenderTarget_GrTextureFlagBit) ||
             !(desc.fFlags & kNoStencil_GrTextureFlagBit));

    if (kApprox_ScratchTexMatch == match) {
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
        GrResourceKey key = GrTexture::ComputeScratchKey(desc);
        // Ensure we have exclusive access to the texture so future 'find' calls don't return it
        resource = fTextureCache->find(key, GrResourceCache::kHide_OwnershipFlag);
        if (NULL != resource) {
            resource->ref();
            break;
        }
        if (kExact_ScratchTexMatch == match) {
            break;
        }
        // We had a cache miss and we are in approx mode, relax the fit of the flags... then try
        // doubling width... then the height.

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
        GrTexture* texture = fGpu->createTexture(desc, NULL, 0);
        if (NULL != texture) {
            GrResourceKey key = GrTexture::ComputeScratchKey(texture->desc());
            // Make the resource exclusive so future 'find' calls don't return it
            fTextureCache->addResource(key, texture, GrResourceCache::kHide_OwnershipFlag);
            resource = texture;
        }
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
    if (texture->getCacheEntry()->key().isScratch()) {
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

GrTexture* GrContext::wrapBackendTexture(const GrBackendTextureDesc& desc) {
    return fGpu->wrapBackendTexture(desc);
}

GrRenderTarget* GrContext::wrapBackendRenderTarget(const GrBackendRenderTargetDesc& desc) {
    return fGpu->wrapBackendRenderTarget(desc);
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

    fDrawState->setState(GrDrawState::kClip_StateBit,
                         clipData && clipData->fClipStack && !clipData->fClipStack->isWideOpen());
}

////////////////////////////////////////////////////////////////////////////////

void GrContext::clear(const GrIRect* rect,
                      const GrColor color,
                      GrRenderTarget* target) {
    this->prepareToDraw(NULL, DEFAULT_BUFFERING)->clear(rect, color, target);
}

void GrContext::drawPaint(const GrPaint& origPaint) {
    // set rect to be big enough to fill the space, but not super-huge, so we
    // don't overflow fixed-point implementations
    GrRect r;
    r.setLTRB(0, 0,
              SkIntToScalar(getRenderTarget()->width()),
              SkIntToScalar(getRenderTarget()->height()));
    SkMatrix inverse;
    SkTCopyOnFirstWrite<GrPaint> paint(origPaint);
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
        if (!am.setIdentity(this, paint.writable())) {
            GrPrintf("Could not invert matrix\n");
            return;
        }
    }
    // by definition this fills the entire clip, no need for AA
    if (paint->isAntiAlias()) {
        paint.writable()->setAntiAlias(false);
    }
    this->drawRect(*paint, r);
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
                               SkScalar width) {
    const SkScalar rad = SkScalarHalf(width);
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
    return SkScalarIsInt(r.fLeft) && SkScalarIsInt(r.fTop) &&
           SkScalarIsInt(r.fRight) && SkScalarIsInt(r.fBottom);
}

static bool apply_aa_to_rect(GrDrawTarget* target,
                             const GrRect& rect,
                             SkScalar width,
                             const SkMatrix* matrix,
                             SkMatrix* combinedMatrix,
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
                         SkScalar width,
                         const SkMatrix* matrix) {
    SK_TRACE_EVENT0("GrContext::drawRect");

    GrDrawTarget* target = this->prepareToDraw(&paint, DEFAULT_BUFFERING);
    GrDrawState::AutoStageDisable atr(fDrawState);

    GrRect devRect = rect;
    SkMatrix combinedMatrix;
    bool useVertexCoverage;
    bool needAA = paint.isAntiAlias() &&
                  !this->getRenderTarget()->isMultisampled();
    bool doAA = needAA && apply_aa_to_rect(target, rect, width, matrix,
                                           &combinedMatrix, &devRect,
                                           &useVertexCoverage);

    if (doAA) {
        GrDrawState::AutoDeviceCoordDraw adcd(target->drawState());
        if (!adcd.succeeded()) {
            return;
        }
        if (width >= 0) {
            GrVec strokeSize;
            if (width > 0) {
                strokeSize.set(width, width);
                combinedMatrix.mapVectors(&strokeSize, 1);
                strokeSize.setAbs(strokeSize);
            } else {
                strokeSize.set(SK_Scalar1, SK_Scalar1);
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
            avmr.set(drawState, *matrix);
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
            SkMatrix m;
            m.setAll(rect.width(),    0,             rect.fLeft,
                        0,            rect.height(), rect.fTop,
                        0,            0,             SkMatrix::I()[8]);

            if (NULL != matrix) {
                m.postConcat(*matrix);
            }
            GrDrawState::AutoViewMatrixRestore avmr(drawState, m);

            target->drawNonIndexed(kTriangleFan_GrPrimitiveType, 0, 4);
#else
            target->drawSimpleRect(rect, matrix);
#endif
    }
}

void GrContext::drawRectToRect(const GrPaint& paint,
                               const GrRect& dstRect,
                               const GrRect& srcRect,
                               const SkMatrix* dstMatrix,
                               const SkMatrix* srcMatrix) {
    SK_TRACE_EVENT0("GrContext::drawRectToRect");

    // srcRect refers to paint's first color stage
    if (!paint.isColorStageEnabled(0)) {
        drawRect(paint, dstRect, -1, dstMatrix);
        return;
    }

    GrDrawTarget* target = this->prepareToDraw(&paint, DEFAULT_BUFFERING);

#if GR_STATIC_RECT_VB
    GrDrawState::AutoStageDisable atr(fDrawState);
    GrDrawState* drawState = target->drawState();

    SkMatrix m;

    m.setAll(dstRect.width(), 0,                dstRect.fLeft,
             0,               dstRect.height(), dstRect.fTop,
             0,               0,                SkMatrix::I()[8]);
    if (NULL != dstMatrix) {
        m.postConcat(*dstMatrix);
    }

    // The first color stage's coords come from srcRect rather than applying a matrix to dstRect.
    // We explicitly compute a matrix for that stage below, no need to adjust here.
    static const uint32_t kExplicitCoordMask = 1 << GrPaint::kFirstColorStage;
    GrDrawState::AutoViewMatrixRestore avmr(drawState, m, kExplicitCoordMask);

    m.setAll(srcRect.width(), 0,                srcRect.fLeft,
             0,               srcRect.height(), srcRect.fTop,
             0,               0,                SkMatrix::I()[8]);
    if (NULL != srcMatrix) {
        m.postConcat(*srcMatrix);
    }

    drawState->preConcatStageMatrices(kExplicitCoordMask, m);

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
    const SkMatrix* srcMatrices[GrDrawState::kNumStages] = {NULL};
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
        layout |= GrDrawState::StageTexCoordVertexLayoutBit(0, 0);
    }
    if (NULL != colors) {
        layout |= GrDrawState::kColor_VertexLayoutBit;
    }
    int vertexSize = GrDrawState::VertexSize(layout);

    if (sizeof(GrPoint) != vertexSize) {
        if (!geo.set(target, layout, vertexCount, 0)) {
            GrPrintf("Failed to get space for vertices!\n");
            return;
        }
        int texOffsets[GrDrawState::kMaxTexCoords];
        int colorOffset;
        GrDrawState::VertexSizeAndOffsetsByIdx(layout,
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
    SkScalar fOuterRadius;
    SkScalar fInnerRadius;
};

inline bool circleStaysCircle(const SkMatrix& m) {
    return m.isSimilarity();
}

}

void GrContext::drawOval(const GrPaint& paint,
                         const GrRect& oval,
                         const SkStrokeRec& stroke) {

    if (!canDrawOval(paint, oval, stroke)) {
        SkPath path;
        path.addOval(oval);
        this->drawPath(paint, path, stroke);
        return;
    }

    internalDrawOval(paint, oval, stroke);
}

bool GrContext::canDrawOval(const GrPaint& paint, const GrRect& oval, const SkStrokeRec& stroke) const {

    if (!paint.isAntiAlias()) {
        return false;
    }

    // we can draw circles in any style
    bool isCircle = SkScalarNearlyEqual(oval.width(), oval.height())
                    && circleStaysCircle(this->getMatrix());
    // and for now, axis-aligned ellipses only with fill or stroke-and-fill
    SkStrokeRec::Style style = stroke.getStyle();
    bool isStroke = (style == SkStrokeRec::kStroke_Style || style == SkStrokeRec::kHairline_Style);
    bool isFilledAxisAlignedEllipse = this->getMatrix().rectStaysRect() && !isStroke;

    return isCircle || isFilledAxisAlignedEllipse;
}

void GrContext::internalDrawOval(const GrPaint& paint,
                                 const GrRect& oval,
                                 const SkStrokeRec& stroke) {

    SkScalar xRadius = SkScalarHalf(oval.width());
    SkScalar yRadius = SkScalarHalf(oval.height());

    SkScalar strokeWidth = stroke.getWidth();
    SkStrokeRec::Style style = stroke.getStyle();

    bool isCircle = SkScalarNearlyEqual(xRadius, yRadius) && circleStaysCircle(this->getMatrix());
#ifdef SK_DEBUG
    {
        // we should have checked for this previously
        bool isStroke = (style == SkStrokeRec::kStroke_Style || style == SkStrokeRec::kHairline_Style);
        bool isFilledAxisAlignedEllipse = this->getMatrix().rectStaysRect() && !isStroke;
        SkASSERT(paint.isAntiAlias() && (isCircle || isFilledAxisAlignedEllipse));
    }
#endif

    GrDrawTarget* target = this->prepareToDraw(&paint, DEFAULT_BUFFERING);

    GrDrawState* drawState = target->drawState();
    GrDrawState::AutoStageDisable atr(fDrawState);
    const SkMatrix vm = drawState->getViewMatrix();

    const GrRenderTarget* rt = drawState->getRenderTarget();
    if (NULL == rt) {
        return;
    }

    GrDrawState::AutoDeviceCoordDraw adcd(drawState);
    if (!adcd.succeeded()) {
        return;
    }

    GrVertexLayout layout = GrDrawState::kEdge_VertexLayoutBit;
    GrAssert(sizeof(CircleVertex) == GrDrawState::VertexSize(layout));

    GrDrawTarget::AutoReleaseGeometry geo(target, layout, 4, 0);
    if (!geo.succeeded()) {
        GrPrintf("Failed to get space for vertices!\n");
        return;
    }

    CircleVertex* verts = reinterpret_cast<CircleVertex*>(geo.vertices());

    GrPoint center = GrPoint::Make(oval.centerX(), oval.centerY());
    vm.mapPoints(&center, 1);

    SkScalar L;
    SkScalar R;
    SkScalar T;
    SkScalar B;

    if (isCircle) {
        drawState->setVertexEdgeType(GrDrawState::kCircle_EdgeType);

        xRadius = vm.mapRadius(xRadius);

        SkScalar outerRadius = xRadius;
        SkScalar innerRadius = 0;
        SkScalar halfWidth = 0;
        if (style != SkStrokeRec::kFill_Style) {
            strokeWidth = vm.mapRadius(strokeWidth);
            if (SkScalarNearlyZero(strokeWidth)) {
                halfWidth = SK_ScalarHalf;
            } else {
                halfWidth = SkScalarHalf(strokeWidth);
            }

            outerRadius += halfWidth;
            if (style == SkStrokeRec::kStroke_Style || style == SkStrokeRec::kHairline_Style) {
                innerRadius = SkMaxScalar(0, xRadius - halfWidth);
            }
        }

        for (int i = 0; i < 4; ++i) {
            verts[i].fCenter = center;
            verts[i].fOuterRadius = outerRadius;
            verts[i].fInnerRadius = innerRadius;
        }

        L = -outerRadius;
        R = +outerRadius;
        T = -outerRadius;
        B = +outerRadius;
    } else {  // is axis-aligned ellipse
        drawState->setVertexEdgeType(GrDrawState::kEllipse_EdgeType);

        SkRect xformedRect;
        vm.mapRect(&xformedRect, oval);

        xRadius = SkScalarHalf(xformedRect.width());
        yRadius = SkScalarHalf(xformedRect.height());

        if (style == SkStrokeRec::kStrokeAndFill_Style && strokeWidth > 0.0f) {
            SkScalar halfWidth = SkScalarHalf(strokeWidth);
            // do (potentially) anisotropic mapping
            SkVector scaledStroke;
            scaledStroke.set(halfWidth, halfWidth);
            vm.mapVectors(&scaledStroke, 1);
            // this is legit only if scale & translation (which should be the case at the moment)
            xRadius += scaledStroke.fX;
            yRadius += scaledStroke.fY;
        }

        SkScalar ratio = SkScalarDiv(xRadius, yRadius);

        for (int i = 0; i < 4; ++i) {
            verts[i].fCenter = center;
            verts[i].fOuterRadius = xRadius;
            verts[i].fInnerRadius = ratio;
        }

        L = -xRadius;
        R = +xRadius;
        T = -yRadius;
        B = +yRadius;
    }

    // The fragment shader will extend the radius out half a pixel
    // to antialias. Expand the drawn rect here so all the pixels
    // will be captured.
    L += center.fX - SK_ScalarHalf;
    R += center.fX + SK_ScalarHalf;
    T += center.fY - SK_ScalarHalf;
    B += center.fY + SK_ScalarHalf;

    verts[0].fPos = SkPoint::Make(L, T);
    verts[1].fPos = SkPoint::Make(R, T);
    verts[2].fPos = SkPoint::Make(L, B);
    verts[3].fPos = SkPoint::Make(R, B);

    target->drawNonIndexed(kTriangleStrip_GrPrimitiveType, 0, 4);
}

void GrContext::drawPath(const GrPaint& paint, const SkPath& path, const SkStrokeRec& stroke) {

    if (path.isEmpty()) {
       if (path.isInverseFillType()) {
           this->drawPaint(paint);
       }
       return;
    }

    SkRect ovalRect;
    bool isOval = path.isOval(&ovalRect);

    if (isOval && !path.isInverseFillType() && this->canDrawOval(paint, ovalRect, stroke)) {
        this->drawOval(paint, ovalRect, stroke);
        return;
    }

    this->internalDrawPath(paint, path, stroke);
}

void GrContext::internalDrawPath(const GrPaint& paint, const SkPath& path, const SkStrokeRec& stroke) {

    // Note that below we may sw-rasterize the path into a scratch texture.
    // Scratch textures can be recycled after they are returned to the texture
    // cache. This presents a potential hazard for buffered drawing. However,
    // the writePixels that uploads to the scratch will perform a flush so we're
    // OK.
    GrDrawTarget* target = this->prepareToDraw(&paint, DEFAULT_BUFFERING);
    GrDrawState::AutoStageDisable atr(fDrawState);

    bool prAA = paint.isAntiAlias() && !this->getRenderTarget()->isMultisampled();

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

    GrPathRendererChain::DrawType type = prAA ? GrPathRendererChain::kColorAntiAlias_DrawType :
                                                GrPathRendererChain::kColor_DrawType;

    const SkPath* pathPtr = &path;
    SkPath tmpPath;
    SkStrokeRec strokeRec(stroke);

    // Try a 1st time without stroking the path and without allowing the SW renderer
    GrPathRenderer* pr = this->getPathRenderer(*pathPtr, strokeRec, target, false, type);

    if (NULL == pr) {
        if (!strokeRec.isHairlineStyle()) {
            // It didn't work the 1st time, so try again with the stroked path
            if (strokeRec.applyToPath(&tmpPath, *pathPtr)) {
                pathPtr = &tmpPath;
                strokeRec.setFillStyle();
            }
        }
        // This time, allow SW renderer
        pr = this->getPathRenderer(*pathPtr, strokeRec, target, true, type);
    }

    if (NULL == pr) {
#if GR_DEBUG
        GrPrintf("Unable to find path renderer compatible with path.\n");
#endif
        return;
    }

    pr->drawPath(*pathPtr, strokeRec, target, prAA);
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
            // compute a matrix to perform the draw
            SkMatrix textureMatrix;
            if (flipY) {
                textureMatrix.setTranslate(SK_Scalar1 * left,
                                    SK_Scalar1 * (top + height));
                textureMatrix.set(SkMatrix::kMScaleY, -SK_Scalar1);
            } else {
                textureMatrix.setTranslate(SK_Scalar1 *left, SK_Scalar1 *top);
            }
            textureMatrix.postIDiv(src->width(), src->height());

            SkAutoTUnref<const GrEffectRef> effect;
            if (unpremul) {
                effect.reset(this->createPMToUPMEffect(src, swapRAndB, textureMatrix));
                if (NULL != effect) {
                    unpremul = false; // we no longer need to do this on CPU after the readback.
                }
            }
            // If we failed to create a PM->UPM effect and have no other conversions to perform then
            // there is no longer any point to using the scratch.
            if (NULL != effect || flipY || swapRAndB) {
                if (!effect) {
                    effect.reset(GrConfigConversionEffect::Create(
                                                    src,
                                                    swapRAndB,
                                                    GrConfigConversionEffect::kNone_PMConversion,
                                                    textureMatrix));
                }
                swapRAndB = false; // we will handle the swap in the draw.
                flipY = false; // we already incorporated the y flip in the matrix

                GrDrawTarget::AutoStateRestore asr(fGpu, GrDrawTarget::kReset_ASRInit);
                GrDrawState* drawState = fGpu->drawState();
                GrAssert(effect);
                drawState->setEffect(0, effect);

                drawState->setRenderTarget(texture->asRenderTarget());
                GrRect rect = GrRect::MakeWH(SkIntToScalar(width), SkIntToScalar(height));
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
    // Perform any conversions we weren't able to perform using a scratch texture.
    if (unpremul || swapRAndB || flipY) {
        // These are initialized to suppress a warning
        SkCanvas::Config8888 srcC8888 = SkCanvas::kNative_Premul_Config8888;
        SkCanvas::Config8888 dstC8888 = SkCanvas::kNative_Premul_Config8888;

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

void GrContext::copyTexture(GrTexture* src, GrRenderTarget* dst, const SkIPoint* topLeft) {
    if (NULL == src || NULL == dst) {
        return;
    }
    ASSERT_OWNED_RESOURCE(src);

    // Writes pending to the source texture are not tracked, so a flush
    // is required to ensure that the copy captures the most recent contents
    // of the source texture. See similar behavior in
    // GrContext::resolveRenderTarget.
    this->flush();

    GrDrawTarget::AutoStateRestore asr(fGpu, GrDrawTarget::kReset_ASRInit);
    GrDrawState* drawState = fGpu->drawState();
    drawState->setRenderTarget(dst);
    SkMatrix sampleM;
    sampleM.setIDiv(src->width(), src->height());
    SkIRect srcRect = SkIRect::MakeWH(dst->width(), dst->height());
    if (NULL != topLeft) {
        srcRect.offset(*topLeft);
    }
    SkIRect srcBounds = SkIRect::MakeWH(src->width(), src->height());
    if (!srcRect.intersect(srcBounds)) {
        return;
    }
    sampleM.preTranslate(SkIntToScalar(srcRect.fLeft), SkIntToScalar(srcRect.fTop));
    drawState->createTextureEffect(0, src, sampleM);
    SkRect dstR = SkRect::MakeWH(SkIntToScalar(srcRect.width()), SkIntToScalar(srcRect.height()));
    fGpu->drawSimpleRect(dstR, NULL);
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

    SkAutoTUnref<const GrEffectRef> effect;
    SkMatrix textureMatrix;
    textureMatrix.setIDiv(texture->width(), texture->height());

    // allocate a tmp buffer and sw convert the pixels to premul
    SkAutoSTMalloc<128 * 128, uint32_t> tmpPixels(0);

    if (kUnpremul_PixelOpsFlag & flags) {
        if (kRGBA_8888_GrPixelConfig != config && kBGRA_8888_GrPixelConfig != config) {
            return;
        }
        effect.reset(this->createUPMToPMEffect(texture, swapRAndB, textureMatrix));
        if (NULL == effect) {
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
    if (NULL == effect) {
        effect.reset(GrConfigConversionEffect::Create(texture,
                                                      swapRAndB,
                                                      GrConfigConversionEffect::kNone_PMConversion,
                                                      textureMatrix));
    }

    this->writeTexturePixels(texture,
                             0, 0, width, height,
                             textureConfig, buffer, rowBytes,
                             flags & ~kUnpremul_PixelOpsFlag);

    GrDrawTarget::AutoStateRestore  asr(fGpu, GrDrawTarget::kReset_ASRInit);
    GrDrawState* drawState = fGpu->drawState();
    GrAssert(effect);
    drawState->setEffect(0, effect);

    SkMatrix matrix;
    matrix.setTranslate(SkIntToScalar(left), SkIntToScalar(top));
    drawState->setViewMatrix(matrix);
    drawState->setRenderTarget(target);

    fGpu->drawSimpleRect(GrRect::MakeWH(SkIntToScalar(width), SkIntToScalar(height)), NULL);
}
////////////////////////////////////////////////////////////////////////////////

GrDrawTarget* GrContext::prepareToDraw(const GrPaint* paint, BufferedDraw buffered) {
    if (kNo_BufferedDraw == buffered && kYes_BufferedDraw == fLastDrawWasBuffered) {
        this->flushDrawBuffer();
        fLastDrawWasBuffered = kNo_BufferedDraw;
    }
    if (NULL != paint) {
        GrAssert(fDrawState->stagesDisabled());
        fDrawState->setFromPaint(*paint);
#if GR_DEBUG_PARTIAL_COVERAGE_CHECK
        if ((paint->hasMask() || 0xff != paint->fCoverage) &&
            !fGpu->canApplyCoverage()) {
            GrPrintf("Partial pixel coverage will be incorrectly blended.\n");
        }
#endif
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
                                           const SkStrokeRec& stroke,
                                           const GrDrawTarget* target,
                                           bool allowSW,
                                           GrPathRendererChain::DrawType drawType,
                                           GrPathRendererChain::StencilSupport* stencilSupport) {

    if (NULL == fPathRendererChain) {
        fPathRendererChain = SkNEW_ARGS(GrPathRendererChain, (this));
    }

    GrPathRenderer* pr = fPathRendererChain->getPathRenderer(path,
                                                             stroke,
                                                             target,
                                                             drawType,
                                                             stencilSupport);

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

const SkMatrix& GrContext::getMatrix() const {
    return fDrawState->getViewMatrix();
}

void GrContext::setMatrix(const SkMatrix& m) {
    fDrawState->setViewMatrix(m);
}

void GrContext::setIdentityMatrix() {
    fDrawState->viewMatrix()->reset();
}

void GrContext::concatMatrix(const SkMatrix& m) const {
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

const GrEffectRef* GrContext::createPMToUPMEffect(GrTexture* texture,
                                                  bool swapRAndB,
                                                  const SkMatrix& matrix) {
    if (!fDidTestPMConversions) {
        test_pm_conversions(this, &fPMToUPMConversion, &fUPMToPMConversion);
        fDidTestPMConversions = true;
    }
    GrConfigConversionEffect::PMConversion pmToUPM =
        static_cast<GrConfigConversionEffect::PMConversion>(fPMToUPMConversion);
    if (GrConfigConversionEffect::kNone_PMConversion != pmToUPM) {
        return GrConfigConversionEffect::Create(texture, swapRAndB, pmToUPM, matrix);
    } else {
        return NULL;
    }
}

const GrEffectRef* GrContext::createUPMToPMEffect(GrTexture* texture,
                                                  bool swapRAndB,
                                                  const SkMatrix& matrix) {
    if (!fDidTestPMConversions) {
        test_pm_conversions(this, &fPMToUPMConversion, &fUPMToPMConversion);
        fDidTestPMConversions = true;
    }
    GrConfigConversionEffect::PMConversion upmToPM =
        static_cast<GrConfigConversionEffect::PMConversion>(fUPMToPMConversion);
    if (GrConfigConversionEffect::kNone_PMConversion != upmToPM) {
        return GrConfigConversionEffect::Create(texture, swapRAndB, upmToPM, matrix);
    } else {
        return NULL;
    }
}

GrTexture* GrContext::gaussianBlur(GrTexture* srcTexture,
                                   bool canClobberSrc,
                                   const SkRect& rect,
                                   float sigmaX, float sigmaY) {
    ASSERT_OWNED_RESOURCE(srcTexture);

    AutoRenderTarget art(this);

    AutoMatrix am;
    am.setIdentity(this);

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
    if (NULL == dstTexture || NULL == tempTexture) {
        return NULL;
    }

    GrPaint paint;
    paint.reset();

    for (int i = 1; i < scaleFactorX || i < scaleFactorY; i *= 2) {
        SkMatrix matrix;
        matrix.setIDiv(srcTexture->width(), srcTexture->height());
        this->setRenderTarget(dstTexture->asRenderTarget());
        SkRect dstRect(srcRect);
        scale_rect(&dstRect, i < scaleFactorX ? 0.5f : 1.0f,
                             i < scaleFactorY ? 0.5f : 1.0f);

        paint.colorStage(0)->setEffect(GrSimpleTextureEffect::Create(srcTexture,
                                                                     matrix,
                                                                     true))->unref();
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
        SkMatrix matrix;
        // FIXME:  This should be mitchell, not bilinear.
        matrix.setIDiv(srcTexture->width(), srcTexture->height());
        this->setRenderTarget(dstTexture->asRenderTarget());
        paint.colorStage(0)->setEffect(GrSimpleTextureEffect::Create(srcTexture,
                                                                     matrix,
                                                                     true))->unref();
        SkRect dstRect(srcRect);
        scale_rect(&dstRect, (float) scaleFactorX, (float) scaleFactorY);
        this->drawRectToRect(paint, dstRect, srcRect);
        srcRect = dstRect;
        srcTexture = dstTexture;
        SkTSwap(dstTexture, tempTexture);
    }
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
