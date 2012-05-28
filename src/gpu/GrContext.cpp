
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrContext.h"

#include "GrBufferAllocPool.h"
#include "GrClipIterator.h"
#include "effects/GrConvolutionEffect.h"
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
#include "SkTrace.h"

#define DEFER_TEXT_RENDERING 1

#define DEFER_PATHS 1

#define BATCH_RECT_TO_RECT (1 && !GR_STATIC_RECT_VB)

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

// path rendering is the only thing we defer today that uses non-static indices
static const size_t DRAW_BUFFER_IBPOOL_BUFFER_SIZE = DEFER_PATHS ? 1 << 11 : 0;
static const int DRAW_BUFFER_IBPOOL_PREALLOC_BUFFERS = DEFER_PATHS ? 4 : 0;

#define ASSERT_OWNED_RESOURCE(R) GrAssert(!(R) || (R)->getContext() == this)

GrContext* GrContext::Create(GrEngine engine,
                             GrPlatform3DContext context3D) {
    GrContext* ctx = NULL;
    GrGpu* fGpu = GrGpu::Create(engine, context3D);
    if (NULL != fGpu) {
        ctx = new GrContext(fGpu);
        fGpu->unref();
    }
    return ctx;
}

GrContext::~GrContext() {
    this->flush();

    // Since the gpu can hold scratch textures, give it a chance to let go
    // of them before freeing the texture cache
    fGpu->purgeResources();

    delete fTextureCache;
    delete fFontCache;
    delete fDrawBuffer;
    delete fDrawBufferVBAllocPool;
    delete fDrawBufferIBAllocPool;

    GrSafeUnref(fAAFillRectIndexBuffer);
    GrSafeUnref(fAAStrokeRectIndexBuffer);
    fGpu->unref();
    GrSafeUnref(fPathRendererChain);
    GrSafeUnref(fSoftwarePathRenderer);
    fDrawState->unref();
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

    GrSafeSetNull(fAAFillRectIndexBuffer);
    GrSafeSetNull(fAAStrokeRectIndexBuffer);

    fTextureCache->removeAll();
    fFontCache->freeAll();
    fGpu->markContextDirty();
}

void GrContext::resetContext() {
    fGpu->markContextDirty();
}

void GrContext::freeGpuResources() {
    this->flush();
    
    fGpu->purgeResources();

    fTextureCache->removeAll();
    fFontCache->freeAll();
    // a path renderer may be holding onto resources
    GrSafeSetNull(fPathRendererChain);
    GrSafeSetNull(fSoftwarePathRenderer);
}

size_t GrContext::getGpuTextureCacheBytes() const {
  return fTextureCache->getCachedResourceBytes();
}

////////////////////////////////////////////////////////////////////////////////

int GrContext::PaintStageVertexLayoutBits(
                            const GrPaint& paint,
                            const bool hasTexCoords[GrPaint::kTotalStages]) {
    int stageMask = paint.getActiveStageMask();
    int layout = 0;
    for (int i = 0; i < GrPaint::kTotalStages; ++i) {
        if ((1 << i) & stageMask) {
            if (NULL != hasTexCoords && hasTexCoords[i]) {
                layout |= GrDrawTarget::StageTexCoordVertexLayoutBit(i, i);
            } else {
                layout |= GrDrawTarget::StagePosAsTexCoordVertexLayoutBit(i);
            }
        }
    }
    return layout;
}


////////////////////////////////////////////////////////////////////////////////

enum {
    // flags for textures
    kNPOTBit            = 0x1,
    kFilterBit          = 0x2,
    kScratchBit         = 0x4,

    // resource type
    kTextureBit         = 0x8,
    kStencilBufferBit   = 0x10
};

GrTexture* GrContext::TextureCacheEntry::texture() const {
    if (NULL == fEntry) {
        return NULL; 
    } else {
        return (GrTexture*) fEntry->resource();
    }
}

namespace {
// returns true if this is a "special" texture because of gpu NPOT limitations
bool gen_texture_key_values(const GrGpu* gpu,
                            const GrSamplerState* sampler,
                            GrContext::TextureKey clientKey,
                            int width,
                            int height,
                            int sampleCnt,
                            bool scratch,
                            uint32_t v[4]) {
    GR_STATIC_ASSERT(sizeof(GrContext::TextureKey) == sizeof(uint64_t));
    // we assume we only need 16 bits of width and height
    // assert that texture creation will fail anyway if this assumption
    // would cause key collisions.
    GrAssert(gpu->getCaps().fMaxTextureSize <= SK_MaxU16);
    v[0] = clientKey & 0xffffffffUL;
    v[1] = (clientKey >> 32) & 0xffffffffUL;
    v[2] = width | (height << 16);

    v[3] = (sampleCnt << 24);
    GrAssert(sampleCnt >= 0 && sampleCnt < 256);

    if (!gpu->getCaps().fNPOTTextureTileSupport) {
        bool isPow2 = GrIsPow2(width) && GrIsPow2(height);

        bool tiled = NULL != sampler &&
                     ((sampler->getWrapX() != GrSamplerState::kClamp_WrapMode) ||
                      (sampler->getWrapY() != GrSamplerState::kClamp_WrapMode));

        if (tiled && !isPow2) {
            v[3] |= kNPOTBit;
            if (GrSamplerState::kNearest_Filter != sampler->getFilter()) {
                v[3] |= kFilterBit;
            }
        }
    }

    if (scratch) {
        v[3] |= kScratchBit;
    }

    v[3] |= kTextureBit;

    return v[3] & kNPOTBit;
}

// we should never have more than one stencil buffer with same combo of
// (width,height,samplecount)
void gen_stencil_key_values(int width, int height,
                            int sampleCnt, uint32_t v[4]) {
    v[0] = width;
    v[1] = height;
    v[2] = sampleCnt;
    v[3] = kStencilBufferBit;
}

void gen_stencil_key_values(const GrStencilBuffer* sb,
                            uint32_t v[4]) {
    gen_stencil_key_values(sb->width(), sb->height(),
                           sb->numSamples(), v);
}

void build_kernel(float sigma, float* kernel, int kernelWidth) {
    int halfWidth = (kernelWidth - 1) / 2;
    float sum = 0.0f;
    float denom = 1.0f / (2.0f * sigma * sigma);
    for (int i = 0; i < kernelWidth; ++i) {
        float x = static_cast<float>(i - halfWidth);
        // Note that the constant term (1/(sqrt(2*pi*sigma^2)) of the Gaussian
        // is dropped here, since we renormalize the kernel below.
        kernel[i] = sk_float_exp(- x * x * denom);
        sum += kernel[i];
    }
    // Normalize the kernel
    float scale = 1.0f / sum;
    for (int i = 0; i < kernelWidth; ++i)
        kernel[i] *= scale;
}

void scale_rect(SkRect* rect, float xScale, float yScale) {
    rect->fLeft = SkScalarMul(rect->fLeft, SkFloatToScalar(xScale));
    rect->fTop = SkScalarMul(rect->fTop, SkFloatToScalar(yScale));
    rect->fRight = SkScalarMul(rect->fRight, SkFloatToScalar(xScale));
    rect->fBottom = SkScalarMul(rect->fBottom, SkFloatToScalar(yScale));
}

float adjust_sigma(float sigma, int *scaleFactor, int *halfWidth,
                          int *kernelWidth) {
    *scaleFactor = 1;
    while (sigma > MAX_BLUR_SIGMA) {
        *scaleFactor *= 2;
        sigma *= 0.5f;
    }
    *halfWidth = static_cast<int>(ceilf(sigma * 3.0f));
    *kernelWidth = *halfWidth * 2 + 1;
    return sigma;
}

void apply_morphology(GrGpu* gpu,
                      GrTexture* texture,
                      const SkRect& rect,
                      int radius,
                      GrSamplerState::Filter filter,
                      GrSamplerState::FilterDirection direction) {
    GrAssert(filter == GrSamplerState::kErode_Filter ||
             filter == GrSamplerState::kDilate_Filter);

    GrRenderTarget* target = gpu->drawState()->getRenderTarget();
    GrDrawTarget::AutoStateRestore asr(gpu, GrDrawTarget::kReset_ASRInit);
    GrDrawState* drawState = gpu->drawState();
    drawState->setRenderTarget(target);
    GrMatrix sampleM;
    sampleM.setIDiv(texture->width(), texture->height());
    drawState->sampler(0)->reset(GrSamplerState::kClamp_WrapMode, filter,
                                 sampleM);
    drawState->sampler(0)->setMorphologyRadius(radius);
    drawState->sampler(0)->setFilterDirection(direction);
    drawState->setTexture(0, texture);
    gpu->drawSimpleRect(rect, NULL, 1 << 0);
}

void convolve(GrGpu* gpu,
              GrTexture* texture,
              const SkRect& rect,
              const float* kernel,
              int kernelWidth,
              GrSamplerState::FilterDirection direction) {
    GrRenderTarget* target = gpu->drawState()->getRenderTarget();
    GrDrawTarget::AutoStateRestore asr(gpu, GrDrawTarget::kReset_ASRInit);
    GrDrawState* drawState = gpu->drawState();
    drawState->setRenderTarget(target);
    GrMatrix sampleM;
    sampleM.setIDiv(texture->width(), texture->height());
    drawState->sampler(0)->reset(GrSamplerState::kClamp_WrapMode,
                                 GrSamplerState::kConvolution_Filter,
                                 sampleM);
    drawState->sampler(0)->setCustomStage(
        new GrConvolutionEffect(direction, kernelWidth, kernel));
    drawState->setTexture(0, texture);
    gpu->drawSimpleRect(rect, NULL, 1 << 0);
}

}

GrContext::TextureCacheEntry GrContext::findAndLockTexture(
        TextureKey key,
        int width,
        int height,
        const GrSamplerState* sampler) {
    uint32_t v[4];
    gen_texture_key_values(fGpu, sampler, key, width, height, 0, false, v);
    GrResourceKey resourceKey(v);
    return TextureCacheEntry(fTextureCache->findAndLock(resourceKey,
                                            GrResourceCache::kNested_LockType));
}

bool GrContext::isTextureInCache(TextureKey key,
                                 int width,
                                 int height,
                                 const GrSamplerState* sampler) const {
    uint32_t v[4];
    gen_texture_key_values(fGpu, sampler, key, width, height, 0, false, v);
    GrResourceKey resourceKey(v);
    return fTextureCache->hasKey(resourceKey);
}

GrResourceEntry* GrContext::addAndLockStencilBuffer(GrStencilBuffer* sb) {
    ASSERT_OWNED_RESOURCE(sb);
    uint32_t v[4];
    gen_stencil_key_values(sb, v);
    GrResourceKey resourceKey(v);
    return fTextureCache->createAndLock(resourceKey, sb);
}

GrStencilBuffer* GrContext::findStencilBuffer(int width, int height,
                                              int sampleCnt) {
    uint32_t v[4];
    gen_stencil_key_values(width, height, sampleCnt, v);
    GrResourceKey resourceKey(v);
    GrResourceEntry* entry = fTextureCache->findAndLock(resourceKey,
                                            GrResourceCache::kSingle_LockType);
    if (NULL != entry) {
        GrStencilBuffer* sb = (GrStencilBuffer*) entry->resource();
        return sb;
    } else {
        return NULL;
    }
}

void GrContext::unlockStencilBuffer(GrResourceEntry* sbEntry) {
    ASSERT_OWNED_RESOURCE(sbEntry->resource());
    fTextureCache->unlock(sbEntry);
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

GrContext::TextureCacheEntry GrContext::createAndLockTexture(
        TextureKey key,
        const GrSamplerState* sampler,
        const GrTextureDesc& desc,
        void* srcData,
        size_t rowBytes) {
    SK_TRACE_EVENT0("GrContext::createAndLockTexture");

#if GR_DUMP_TEXTURE_UPLOAD
    GrPrintf("GrContext::createAndLockTexture [%d %d]\n", desc.fWidth, desc.fHeight);
#endif

    TextureCacheEntry entry;
    uint32_t v[4];
    bool special = gen_texture_key_values(fGpu, sampler, key,
                                          desc.fWidth, desc.fHeight,
                                          desc.fSampleCnt, false, v);
    GrResourceKey resourceKey(v);

    if (special) {
        GrAssert(NULL != sampler);
        TextureCacheEntry clampEntry = this->findAndLockTexture(key,
                                                                desc.fWidth,
                                                                desc.fHeight,
                                                                NULL);

        if (NULL == clampEntry.texture()) {
            clampEntry = this->createAndLockTexture(key, NULL, desc,
                                                    srcData, rowBytes);
            GrAssert(NULL != clampEntry.texture());
            if (NULL == clampEntry.texture()) {
                return entry;
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
            GrDrawTarget::AutoStateRestore asr(fGpu,
                                               GrDrawTarget::kReset_ASRInit);
            GrDrawState* drawState = fGpu->drawState();
            drawState->setRenderTarget(texture->asRenderTarget());
            drawState->setTexture(0, clampEntry.texture());

            GrSamplerState::Filter filter;
            // if filtering is not desired then we want to ensure all
            // texels in the resampled image are copies of texels from
            // the original.
            if (GrSamplerState::kNearest_Filter == sampler->getFilter()) {
                filter = GrSamplerState::kNearest_Filter;
            } else {
                filter = GrSamplerState::kBilinear_Filter;
            }
            drawState->sampler(0)->reset(GrSamplerState::kClamp_WrapMode,
                                         filter);

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
                fGpu->drawNonIndexed(kTriangleFan_PrimitiveType,
                                     0, 4);
                entry.set(fTextureCache->createAndLock(resourceKey, texture));
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
            entry.set(fTextureCache->createAndLock(resourceKey, texture));
        }
        fTextureCache->unlock(clampEntry.cacheEntry());

    } else {
        GrTexture* texture = fGpu->createTexture(desc, srcData, rowBytes);
        if (NULL != texture) {
            entry.set(fTextureCache->createAndLock(resourceKey, texture));
        }
    }
    return entry;
}

namespace {
inline void gen_scratch_tex_key_values(const GrGpu* gpu, 
                                       const GrTextureDesc& desc,
                                       uint32_t v[4]) {
    // Instead of a client-provided key of the texture contents
    // we create a key of from the descriptor.
    GrContext::TextureKey descKey = (desc.fFlags << 8) |
                                    ((uint64_t) desc.fConfig << 32);
    // this code path isn't friendly to tiling with NPOT restricitons
    // We just pass ClampNoFilter()
    gen_texture_key_values(gpu, NULL, descKey, desc.fWidth,
                           desc.fHeight, desc.fSampleCnt, true, v);
}
}

GrContext::TextureCacheEntry GrContext::lockScratchTexture(
                                                const GrTextureDesc& inDesc,
                                                ScratchTexMatch match) {

    GrTextureDesc desc = inDesc;
    if (kExact_ScratchTexMatch != match) {
        // bin by pow2 with a reasonable min
        static const int MIN_SIZE = 256;
        desc.fWidth  = GrMax(MIN_SIZE, GrNextPow2(desc.fWidth));
        desc.fHeight = GrMax(MIN_SIZE, GrNextPow2(desc.fHeight));
    }

    GrResourceEntry* entry;
    int origWidth = desc.fWidth;
    int origHeight = desc.fHeight;
    bool doubledW = false;
    bool doubledH = false;

    do {
        uint32_t v[4];
        gen_scratch_tex_key_values(fGpu, desc, v);
        GrResourceKey key(v);
        entry = fTextureCache->findAndLock(key,
                                           GrResourceCache::kNested_LockType);
        // if we miss, relax the fit of the flags...
        // then try doubling width... then height.
        if (NULL != entry || kExact_ScratchTexMatch == match) {
            break;
        }
        if (!(desc.fFlags & kRenderTarget_GrTextureFlagBit)) {
            desc.fFlags = desc.fFlags | kRenderTarget_GrTextureFlagBit;
        } else if (desc.fFlags & kNoStencil_GrTextureFlagBit) {
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

    if (NULL == entry) {
        desc.fFlags = inDesc.fFlags;
        desc.fWidth = origWidth;
        desc.fHeight = origHeight;
        GrTexture* texture = fGpu->createTexture(desc, NULL, 0);
        if (NULL != texture) {
            uint32_t v[4];
            gen_scratch_tex_key_values(fGpu, desc, v);
            GrResourceKey key(v);
            entry = fTextureCache->createAndLock(key, texture);
        }
    }

    // If the caller gives us the same desc/sampler twice we don't want
    // to return the same texture the second time (unless it was previously
    // released). So we detach the entry from the cache and reattach at release.
    if (NULL != entry) {
        fTextureCache->detach(entry);
    }
    return TextureCacheEntry(entry);
}

void GrContext::unlockTexture(TextureCacheEntry entry) {
    ASSERT_OWNED_RESOURCE(entry.texture());
    // If this is a scratch texture we detached it from the cache
    // while it was locked (to avoid two callers simultaneously getting
    // the same texture).
    if (kScratchBit & entry.cacheEntry()->key().getValue32(3)) {
        fTextureCache->reattachAndUnlock(entry.cacheEntry());
    } else {
        fTextureCache->unlock(entry.cacheEntry());
    }
}

GrTexture* GrContext::createUncachedTexture(const GrTextureDesc& desc,
                                            void* srcData,
                                            size_t rowBytes) {
    return fGpu->createTexture(desc, srcData, rowBytes);
}

void GrContext::getTextureCacheLimits(int* maxTextures,
                                      size_t* maxTextureBytes) const {
    fTextureCache->getLimits(maxTextures, maxTextureBytes);
}

void GrContext::setTextureCacheLimits(int maxTextures, size_t maxTextureBytes) {
    fTextureCache->setLimits(maxTextures, maxTextureBytes);
}

int GrContext::getMaxTextureSize() const {
    return fGpu->getCaps().fMaxTextureSize;
}

int GrContext::getMaxRenderTargetSize() const {
    return fGpu->getCaps().fMaxRenderTargetSize;
}

///////////////////////////////////////////////////////////////////////////////

GrTexture* GrContext::createPlatformTexture(const GrPlatformTextureDesc& desc) {
    return fGpu->createPlatformTexture(desc);
}

GrRenderTarget* GrContext::createPlatformRenderTarget(const GrPlatformRenderTargetDesc& desc) {
    return fGpu->createPlatformRenderTarget(desc);
}

///////////////////////////////////////////////////////////////////////////////

bool GrContext::supportsIndex8PixelConfig(const GrSamplerState* sampler,
                                          int width, int height) const {
    const GrDrawTarget::Caps& caps = fGpu->getCaps();
    if (!caps.f8BitPaletteSupport) {
        return false;
    }

    bool isPow2 = GrIsPow2(width) && GrIsPow2(height);

    if (!isPow2) {
        bool tiled = NULL != sampler &&
                     (sampler->getWrapX() != GrSamplerState::kClamp_WrapMode ||
                      sampler->getWrapY() != GrSamplerState::kClamp_WrapMode);
        if (tiled && !caps.fNPOTTextureTileSupport) {
            return false;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

const GrClip& GrContext::getClip() const { return fGpu->getClip(); }

void GrContext::setClip(const GrClip& clip) {
    fGpu->setClip(clip);
    fDrawState->enableState(GrDrawState::kClip_StateBit);
}

void GrContext::setClip(const GrIRect& rect) {
    GrClip clip;
    clip.setFromIRect(rect);
    fGpu->setClip(clip);
}

////////////////////////////////////////////////////////////////////////////////

void GrContext::clear(const GrIRect* rect, const GrColor color) {
    this->flush();
    fGpu->clear(rect, color);
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
    GrAutoMatrix am;

    // We attempt to map r by the inverse matrix and draw that. mapRect will
    // map the four corners and bound them with a new rect. This will not
    // produce a correct result for some perspective matrices.
    if (!this->getMatrix().hasPerspective()) {
        if (!fDrawState->getViewInverse(&inverse)) {
            GrPrintf("Could not invert matrix");
            return;
        }
        inverse.mapRect(&r);
    } else {
        if (paint.getActiveMaskStageMask() || paint.getActiveStageMask()) {
            if (!fDrawState->getViewInverse(&inverse)) {
                GrPrintf("Could not invert matrix");
                return;
            }
            tmpPaint.set(paint);
            tmpPaint.get()->preConcatActiveSamplerMatrices(inverse);
            p = tmpPaint.get();
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

static void setInsetFan(GrPoint* pts, size_t stride,
                        const GrRect& r, GrScalar dx, GrScalar dy) {
    pts->setRectFan(r.fLeft + dx, r.fTop + dy, r.fRight - dx, r.fBottom - dy, stride);
}

static const uint16_t gFillAARectIdx[] = {
    0, 1, 5, 5, 4, 0,
    1, 2, 6, 6, 5, 1,
    2, 3, 7, 7, 6, 2,
    3, 0, 4, 4, 7, 3,
    4, 5, 6, 6, 7, 4,
};

int GrContext::aaFillRectIndexCount() const {
    return GR_ARRAY_COUNT(gFillAARectIdx);
}

GrIndexBuffer* GrContext::aaFillRectIndexBuffer() {
    if (NULL == fAAFillRectIndexBuffer) {
        fAAFillRectIndexBuffer = fGpu->createIndexBuffer(sizeof(gFillAARectIdx),
                                                         false);
        if (NULL != fAAFillRectIndexBuffer) {
    #if GR_DEBUG
            bool updated =
    #endif
            fAAFillRectIndexBuffer->updateData(gFillAARectIdx,
                                               sizeof(gFillAARectIdx));
            GR_DEBUGASSERT(updated);
        }
    }
    return fAAFillRectIndexBuffer;
}

static const uint16_t gStrokeAARectIdx[] = {
    0 + 0, 1 + 0, 5 + 0, 5 + 0, 4 + 0, 0 + 0,
    1 + 0, 2 + 0, 6 + 0, 6 + 0, 5 + 0, 1 + 0,
    2 + 0, 3 + 0, 7 + 0, 7 + 0, 6 + 0, 2 + 0,
    3 + 0, 0 + 0, 4 + 0, 4 + 0, 7 + 0, 3 + 0,

    0 + 4, 1 + 4, 5 + 4, 5 + 4, 4 + 4, 0 + 4,
    1 + 4, 2 + 4, 6 + 4, 6 + 4, 5 + 4, 1 + 4,
    2 + 4, 3 + 4, 7 + 4, 7 + 4, 6 + 4, 2 + 4,
    3 + 4, 0 + 4, 4 + 4, 4 + 4, 7 + 4, 3 + 4,

    0 + 8, 1 + 8, 5 + 8, 5 + 8, 4 + 8, 0 + 8,
    1 + 8, 2 + 8, 6 + 8, 6 + 8, 5 + 8, 1 + 8,
    2 + 8, 3 + 8, 7 + 8, 7 + 8, 6 + 8, 2 + 8,
    3 + 8, 0 + 8, 4 + 8, 4 + 8, 7 + 8, 3 + 8,
};

int GrContext::aaStrokeRectIndexCount() const {
    return GR_ARRAY_COUNT(gStrokeAARectIdx);
}

GrIndexBuffer* GrContext::aaStrokeRectIndexBuffer() {
    if (NULL == fAAStrokeRectIndexBuffer) {
        fAAStrokeRectIndexBuffer = fGpu->createIndexBuffer(sizeof(gStrokeAARectIdx),
                                                           false);
        if (NULL != fAAStrokeRectIndexBuffer) {
    #if GR_DEBUG
            bool updated =
    #endif
            fAAStrokeRectIndexBuffer->updateData(gStrokeAARectIdx,
                                                 sizeof(gStrokeAARectIdx));
            GR_DEBUGASSERT(updated);
        }
    }
    return fAAStrokeRectIndexBuffer;
}

static GrVertexLayout aa_rect_layout(const GrDrawTarget* target,
                                     bool useCoverage) {
    GrVertexLayout layout = 0;
    for (int s = 0; s < GrDrawState::kNumStages; ++s) {
        if (NULL != target->getDrawState().getTexture(s)) {
            layout |= GrDrawTarget::StagePosAsTexCoordVertexLayoutBit(s);
        }
    }
    if (useCoverage) {
        layout |= GrDrawTarget::kCoverage_VertexLayoutBit;
    } else {
        layout |= GrDrawTarget::kColor_VertexLayoutBit;
    }
    return layout;
}

void GrContext::fillAARect(GrDrawTarget* target,
                           const GrRect& devRect,
                           bool useVertexCoverage) {
    GrVertexLayout layout = aa_rect_layout(target, useVertexCoverage);

    size_t vsize = GrDrawTarget::VertexSize(layout);

    GrDrawTarget::AutoReleaseGeometry geo(target, layout, 8, 0);
    if (!geo.succeeded()) {
        GrPrintf("Failed to get space for vertices!\n");
        return;
    }
    GrIndexBuffer* indexBuffer = this->aaFillRectIndexBuffer();
    if (NULL == indexBuffer) {
        GrPrintf("Failed to create index buffer!\n");
        return;
    }

    intptr_t verts = reinterpret_cast<intptr_t>(geo.vertices());

    GrPoint* fan0Pos = reinterpret_cast<GrPoint*>(verts);
    GrPoint* fan1Pos = reinterpret_cast<GrPoint*>(verts + 4 * vsize);

    setInsetFan(fan0Pos, vsize, devRect, -GR_ScalarHalf, -GR_ScalarHalf);
    setInsetFan(fan1Pos, vsize, devRect,  GR_ScalarHalf,  GR_ScalarHalf);

    verts += sizeof(GrPoint);
    for (int i = 0; i < 4; ++i) {
        *reinterpret_cast<GrColor*>(verts + i * vsize) = 0;
    }

    GrColor innerColor;
    if (useVertexCoverage) {
        innerColor = 0xffffffff;
    } else {
        innerColor = target->getDrawState().getColor();
    }

    verts += 4 * vsize;
    for (int i = 0; i < 4; ++i) {
        *reinterpret_cast<GrColor*>(verts + i * vsize) = innerColor;
    }

    target->setIndexSourceToBuffer(indexBuffer);

    target->drawIndexed(kTriangles_PrimitiveType, 0,
                         0, 8, this->aaFillRectIndexCount());
}

void GrContext::strokeAARect(GrDrawTarget* target,
                             const GrRect& devRect,
                             const GrVec& devStrokeSize,
                             bool useVertexCoverage) {
    const GrScalar& dx = devStrokeSize.fX;
    const GrScalar& dy = devStrokeSize.fY;
    const GrScalar rx = GrMul(dx, GR_ScalarHalf);
    const GrScalar ry = GrMul(dy, GR_ScalarHalf);

    GrScalar spare;
    {
        GrScalar w = devRect.width() - dx;
        GrScalar h = devRect.height() - dy;
        spare = GrMin(w, h);
    }

    if (spare <= 0) {
        GrRect r(devRect);
        r.inset(-rx, -ry);
        fillAARect(target, r, useVertexCoverage);
        return;
    }
    GrVertexLayout layout = aa_rect_layout(target, useVertexCoverage);
    size_t vsize = GrDrawTarget::VertexSize(layout);

    GrDrawTarget::AutoReleaseGeometry geo(target, layout, 16, 0);
    if (!geo.succeeded()) {
        GrPrintf("Failed to get space for vertices!\n");
        return;
    }
    GrIndexBuffer* indexBuffer = this->aaStrokeRectIndexBuffer();
    if (NULL == indexBuffer) {
        GrPrintf("Failed to create index buffer!\n");
        return;
    }

    intptr_t verts = reinterpret_cast<intptr_t>(geo.vertices());

    GrPoint* fan0Pos = reinterpret_cast<GrPoint*>(verts);
    GrPoint* fan1Pos = reinterpret_cast<GrPoint*>(verts + 4 * vsize);
    GrPoint* fan2Pos = reinterpret_cast<GrPoint*>(verts + 8 * vsize);
    GrPoint* fan3Pos = reinterpret_cast<GrPoint*>(verts + 12 * vsize);

    setInsetFan(fan0Pos, vsize, devRect, -rx - GR_ScalarHalf, -ry - GR_ScalarHalf);
    setInsetFan(fan1Pos, vsize, devRect, -rx + GR_ScalarHalf, -ry + GR_ScalarHalf);
    setInsetFan(fan2Pos, vsize, devRect,  rx - GR_ScalarHalf,  ry - GR_ScalarHalf);
    setInsetFan(fan3Pos, vsize, devRect,  rx + GR_ScalarHalf,  ry + GR_ScalarHalf);

    verts += sizeof(GrPoint);
    for (int i = 0; i < 4; ++i) {
        *reinterpret_cast<GrColor*>(verts + i * vsize) = 0;
    }

    GrColor innerColor;
    if (useVertexCoverage) {
        innerColor = 0xffffffff;
    } else {
        innerColor = target->getDrawState().getColor();
    }
    verts += 4 * vsize;
    for (int i = 0; i < 8; ++i) {
        *reinterpret_cast<GrColor*>(verts + i * vsize) = innerColor;
    }

    verts += 8 * vsize;
    for (int i = 0; i < 8; ++i) {
        *reinterpret_cast<GrColor*>(verts + i * vsize) = 0;
    }

    target->setIndexSourceToBuffer(indexBuffer);
    target->drawIndexed(kTriangles_PrimitiveType,
                        0, 0, 16, aaStrokeRectIndexCount());
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

    GrDrawTarget* target = this->prepareToDraw(paint, kUnbuffered_DrawCategory);
    int stageMask = paint.getActiveStageMask();

    GrRect devRect = rect;
    GrMatrix combinedMatrix;
    bool useVertexCoverage;
    bool needAA = paint.fAntiAlias &&
                  !this->getRenderTarget()->isMultisampled();
    bool doAA = needAA && apply_aa_to_rect(target, rect, width, matrix,
                                           &combinedMatrix, &devRect,
                                           &useVertexCoverage);

    if (doAA) {
        GrDrawTarget::AutoDeviceCoordDraw adcd(target, stageMask);
        if (width >= 0) {
            GrVec strokeSize;;
            if (width > 0) {
                strokeSize.set(width, width);
                combinedMatrix.mapVectors(&strokeSize, 1);
                strokeSize.setAbs(strokeSize);
            } else {
                strokeSize.set(GR_Scalar1, GR_Scalar1);
            }
            strokeAARect(target, devRect, strokeSize, useVertexCoverage);
        } else {
            fillAARect(target, devRect, useVertexCoverage);
        }
        return;
    }

    if (width >= 0) {
        // TODO: consider making static vertex buffers for these cases.
        // Hairline could be done by just adding closing vertex to
        // unitSquareVertexBuffer()
        GrVertexLayout layout =  PaintStageVertexLayoutBits(paint, NULL);

        static const int worstCaseVertCount = 10;
        GrDrawTarget::AutoReleaseGeometry geo(target, layout, worstCaseVertCount, 0);

        if (!geo.succeeded()) {
            GrPrintf("Failed to get space for vertices!\n");
            return;
        }

        GrPrimitiveType primType;
        int vertCount;
        GrPoint* vertex = geo.positions();

        if (width > 0) {
            vertCount = 10;
            primType = kTriangleStrip_PrimitiveType;
            setStrokeRectStrip(vertex, rect, width);
        } else {
            // hairline
            vertCount = 5;
            primType = kLineStrip_PrimitiveType;
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
            drawState->preConcatSamplerMatrices(stageMask, *matrix);
        }

        target->drawNonIndexed(primType, 0, vertCount);
    } else {
#if GR_STATIC_RECT_VB
            GrVertexLayout layout = PaintStageVertexLayoutBits(paint, NULL);
            const GrVertexBuffer* sqVB = fGpu->getUnitSquareVertexBuffer();
            if (NULL == sqVB) {
                GrPrintf("Failed to create static rect vb.\n");
                return;
            }
            target->setVertexSourceToBuffer(layout, sqVB);
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
            drawState->preConcatSamplerMatrices(stageMask, m);
 
            target->drawNonIndexed(kTriangleFan_PrimitiveType, 0, 4);
#else
            target->drawSimpleRect(rect, matrix, stageMask);
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
    if (NULL == paint.getTexture(0)) {
        drawRect(paint, dstRect, -1, dstMatrix);
        return;
    }

    GR_STATIC_ASSERT(!BATCH_RECT_TO_RECT || !GR_STATIC_RECT_VB);

#if GR_STATIC_RECT_VB
    GrDrawTarget* target = this->prepareToDraw(paint, kUnbuffered_DrawCategory);
    GrDrawState* drawState = target->drawState();
    GrVertexLayout layout = PaintStageVertexLayoutBits(paint, NULL);
    GrDrawState::AutoViewMatrixRestore avmr(drawState);

    GrMatrix m;

    m.setAll(dstRect.width(), 0,                dstRect.fLeft,
             0,               dstRect.height(), dstRect.fTop,
             0,               0,                GrMatrix::I()[8]);
    if (NULL != dstMatrix) {
        m.postConcat(*dstMatrix);
    }
    drawState->preConcatViewMatrix(m);

    // srcRect refers to first stage
    int otherStageMask = paint.getActiveStageMask() & 
                         (~(1 << GrPaint::kFirstTextureStage));
    if (otherStageMask) {
        drawState->preConcatSamplerMatrices(otherStageMask, m);
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
    target->setVertexSourceToBuffer(layout, sqVB);
    target->drawNonIndexed(kTriangleFan_PrimitiveType, 0, 4);
#else

    GrDrawTarget* target;
#if BATCH_RECT_TO_RECT
    target = this->prepareToDraw(paint, kBuffered_DrawCategory);
#else
    target = this->prepareToDraw(paint, kUnbuffered_DrawCategory);
#endif

    const GrRect* srcRects[GrDrawState::kNumStages] = {NULL};
    const GrMatrix* srcMatrices[GrDrawState::kNumStages] = {NULL};
    srcRects[0] = &srcRect;
    srcMatrices[0] = srcMatrix;

    target->drawRect(dstRect, dstMatrix, 1, srcRects, srcMatrices);
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

    GrDrawTarget* target = this->prepareToDraw(paint, kUnbuffered_DrawCategory);

    bool hasTexCoords[GrPaint::kTotalStages] = {
        NULL != texCoords,   // texCoordSrc provides explicit stage 0 coords
        0                    // remaining stages use positions
    };

    GrVertexLayout layout = PaintStageVertexLayoutBits(paint, hasTexCoords);

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
    DrawCategory category = (DEFER_PATHS) ? kBuffered_DrawCategory :
                                            kUnbuffered_DrawCategory;
    GrDrawTarget* target = this->prepareToDraw(paint, category);
    GrDrawState* drawState = target->drawState();
    GrMatrix vm = drawState->getViewMatrix();

    if (!isSimilarityTransformation(vm) ||
        !paint.fAntiAlias ||
        rect.height() != rect.width()) {
        SkPath path;
        path.addOval(rect);
        GrPathFill fill = (strokeWidth == 0) ?
                            kHairLine_PathFill : kWinding_PathFill;
        this->internalDrawPath(paint, path, fill, NULL);
        return;
    }

    const GrRenderTarget* rt = drawState->getRenderTarget();
    if (NULL == rt) {
        return;
    }

    GrDrawTarget::AutoDeviceCoordDraw adcd(target, paint.getActiveStageMask());

    GrVertexLayout layout = PaintStageVertexLayoutBits(paint, NULL);
    layout |= GrDrawTarget::kEdge_VertexLayoutBit;
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

    SkScalar L = center.fX - outerRadius;
    SkScalar R = center.fX + outerRadius;
    SkScalar T = center.fY - outerRadius;
    SkScalar B = center.fY + outerRadius;

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
    target->drawNonIndexed(kTriangleStrip_PrimitiveType, 0, 4);
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
        SkScalar width = (fill == kHairLine_PathFill) ? 0 : -SK_Scalar1;
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
    DrawCategory category = (DEFER_PATHS) ? kBuffered_DrawCategory :
                                            kUnbuffered_DrawCategory;
    GrDrawTarget* target = this->prepareToDraw(paint, category);
    GrDrawState::StageMask stageMask = paint.getActiveStageMask();

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

    pr->drawPath(path, fill, translate, target, stageMask, prAA);
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

void GrContext::internalWriteTexturePixels(GrTexture* texture,
                                           int left, int top,
                                           int width, int height,
                                           GrPixelConfig config,
                                           const void* buffer,
                                           size_t rowBytes,
                                           uint32_t flags) {
    SK_TRACE_EVENT0("GrContext::writeTexturePixels");
    ASSERT_OWNED_RESOURCE(texture);

    if (!(kDontFlush_PixelOpsFlag & flags)) {
        this->flush();
    }
    // TODO: use scratch texture to perform conversion
    if (GrPixelConfigIsUnpremultiplied(texture->config()) !=
        GrPixelConfigIsUnpremultiplied(config)) {
        return;
    }

    fGpu->writeTexturePixels(texture, left, top, width, height, 
                             config, buffer, rowBytes);
}

bool GrContext::internalReadTexturePixels(GrTexture* texture,
                                          int left, int top,
                                          int width, int height,
                                          GrPixelConfig config,
                                          void* buffer,
                                          size_t rowBytes,
                                          uint32_t flags) {
    SK_TRACE_EVENT0("GrContext::readTexturePixels");
    ASSERT_OWNED_RESOURCE(texture);

    // TODO: code read pixels for textures that aren't also rendertargets
    GrRenderTarget* target = texture->asRenderTarget();
    if (NULL != target) {
        return this->internalReadRenderTargetPixels(target,
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
                            SkCanvas::Config8888* config8888) {
    switch (config) {
        case kRGBA_8888_PM_GrPixelConfig:
            *config8888 = SkCanvas::kRGBA_Premul_Config8888;
            return true;
        case kRGBA_8888_UPM_GrPixelConfig:
            *config8888 = SkCanvas::kRGBA_Unpremul_Config8888;
            return true;
        case kBGRA_8888_PM_GrPixelConfig:
            *config8888 = SkCanvas::kBGRA_Premul_Config8888;
            return true;
        case kBGRA_8888_UPM_GrPixelConfig:
            *config8888 = SkCanvas::kBGRA_Unpremul_Config8888;
            return true;
        default:
            return false;
    }
}
}

bool GrContext::internalReadRenderTargetPixels(GrRenderTarget* target,
                                               int left, int top,
                                               int width, int height,
                                               GrPixelConfig config,
                                               void* buffer,
                                               size_t rowBytes,
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

    if (!GrPixelConfigIsUnpremultiplied(target->config()) &&
        GrPixelConfigIsUnpremultiplied(config) &&
        !fGpu->canPreserveReadWriteUnpremulPixels()) {
        SkCanvas::Config8888 srcConfig8888, dstConfig8888;
        if (!grconfig_to_config8888(target->config(), &srcConfig8888) ||
            !grconfig_to_config8888(config, &dstConfig8888)) {
            return false;
        }
        // do read back using target's own config
        this->internalReadRenderTargetPixels(target,
                                             left, top,
                                             width, height,
                                             target->config(),
                                             buffer, rowBytes,
                                             kDontFlush_PixelOpsFlag);
        // sw convert the pixels to unpremul config
        uint32_t* pixels = reinterpret_cast<uint32_t*>(buffer);
        SkConvertConfig8888Pixels(pixels, rowBytes, dstConfig8888,
                                  pixels, rowBytes, srcConfig8888,
                                  width, height);
        return true;
    }

    GrTexture* src = target->asTexture();
    bool swapRAndB = NULL != src &&
                     fGpu->preferredReadPixelsConfig(config) ==
                     GrPixelConfigSwapRAndB(config);

    bool flipY = NULL != src &&
                 fGpu->readPixelsWillPayForYFlip(target, left, top,
                                                 width, height, config,
                                                 rowBytes);
    bool alphaConversion = (!GrPixelConfigIsUnpremultiplied(target->config()) &&
                             GrPixelConfigIsUnpremultiplied(config));

    if (NULL == src && alphaConversion) {
        // we should fallback to cpu conversion here. This could happen when
        // we were given an external render target by the client that is not
        // also a texture (e.g. FBO 0 in GL)
        return false;
    }
    // we draw to a scratch texture if any of these conversion are applied
    GrAutoScratchTexture ast;
    if (flipY || swapRAndB || alphaConversion) {
        GrAssert(NULL != src);
        if (swapRAndB) {
            config = GrPixelConfigSwapRAndB(config);
            GrAssert(kUnknown_GrPixelConfig != config);
        }
        // Make the scratch a render target because we don't have a robust
        // readTexturePixels as of yet (it calls this function).
        const GrTextureDesc desc = {
            kRenderTarget_GrTextureFlagBit,
            width, height,
            config,
            0 // samples
        };

        // When a full readback is faster than a partial we could always make
        // the scratch exactly match the passed rect. However, if we see many
        // different size rectangles we will trash our texture cache and pay the
        // cost of creating and destroying many textures. So, we only request
        // an exact match when the caller is reading an entire RT.
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
        if (!texture) {
            return false;
        }
        target = texture->asRenderTarget();
        GrAssert(NULL != target);

        GrDrawTarget::AutoStateRestore asr(fGpu,
                                           GrDrawTarget::kReset_ASRInit);
        GrDrawState* drawState = fGpu->drawState();
        drawState->setRenderTarget(target);

        GrMatrix matrix;
        if (flipY) {
            matrix.setTranslate(SK_Scalar1 * left,
                                SK_Scalar1 * (top + height));
            matrix.set(GrMatrix::kMScaleY, -GR_Scalar1);
        } else {
            matrix.setTranslate(SK_Scalar1 *left, SK_Scalar1 *top);
        }
        matrix.postIDiv(src->width(), src->height());
        drawState->sampler(0)->reset(matrix);
        drawState->sampler(0)->setRAndBSwap(swapRAndB);
        drawState->setTexture(0, src);
        GrRect rect;
        rect.setXYWH(0, 0, SK_Scalar1 * width, SK_Scalar1 * height);
        fGpu->drawSimpleRect(rect, NULL, 0x1);
        left = 0;
        top = 0;
    }
    return fGpu->readPixels(target,
                            left, top, width, height,
                            config, buffer, rowBytes, flipY);
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
    drawState->setTexture(0, src);
    drawState->sampler(0)->reset(sampleM);
    SkRect rect = SkRect::MakeXYWH(0, 0,
                                   SK_Scalar1 * src->width(),
                                   SK_Scalar1 * src->height());
    fGpu->drawSimpleRect(rect, NULL, 1 << 0);
}

void GrContext::internalWriteRenderTargetPixels(GrRenderTarget* target, 
                                                int left, int top,
                                                int width, int height,
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

    // TODO: when underlying api has a direct way to do this we should use it
    // (e.g. glDrawPixels on desktop GL).

    // If the RT is also a texture and we don't have to do PM/UPM conversion
    // then take the texture path, which we expect to be at least as fast or
    // faster since it doesn't use an intermediate texture as we do below.
    
#if !GR_MAC_BUILD
    // At least some drivers on the Mac get confused when glTexImage2D is called
    // on a texture attached to an FBO. The FBO still sees the old image. TODO:
    // determine what OS versions and/or HW is affected.
    if (NULL != target->asTexture() &&
        GrPixelConfigIsUnpremultiplied(target->config()) ==
        GrPixelConfigIsUnpremultiplied(config)) {

        this->internalWriteTexturePixels(target->asTexture(),
                                         left, top, width, height,
                                         config, buffer, rowBytes, flags);
        return;
    }
#endif
    if (!GrPixelConfigIsUnpremultiplied(target->config()) &&
        GrPixelConfigIsUnpremultiplied(config) &&
        !fGpu->canPreserveReadWriteUnpremulPixels()) {
        SkCanvas::Config8888 srcConfig8888, dstConfig8888;
        if (!grconfig_to_config8888(config, &srcConfig8888) ||
            !grconfig_to_config8888(target->config(), &dstConfig8888)) {
            return;
        }
        // allocate a tmp buffer and sw convert the pixels to premul
        SkAutoSTMalloc<128 * 128, uint32_t> tmpPixels(width * height);
        const uint32_t* src = reinterpret_cast<const uint32_t*>(buffer);
        SkConvertConfig8888Pixels(tmpPixels.get(), 4 * width, dstConfig8888,
                                  src, rowBytes, srcConfig8888,
                                  width, height);
        // upload the already premul pixels
        this->internalWriteRenderTargetPixels(target,
                                             left, top,
                                             width, height,
                                             target->config(),
                                             tmpPixels, 4 * width, flags);
        return;
    }

    bool swapRAndB = fGpu->preferredReadPixelsConfig(config) ==
                     GrPixelConfigSwapRAndB(config);
    if (swapRAndB) {
        config = GrPixelConfigSwapRAndB(config);
    }

    const GrTextureDesc desc = {
        kNone_GrTextureFlags, width, height, config, 0
    };
    GrAutoScratchTexture ast(this, desc);
    GrTexture* texture = ast.texture();
    if (NULL == texture) {
        return;
    }
    this->internalWriteTexturePixels(texture, 0, 0, width, height,
                                     config, buffer, rowBytes, flags);

    GrDrawTarget::AutoStateRestore  asr(fGpu, GrDrawTarget::kReset_ASRInit);
    GrDrawState* drawState = fGpu->drawState();

    GrMatrix matrix;
    matrix.setTranslate(GrIntToScalar(left), GrIntToScalar(top));
    drawState->setViewMatrix(matrix);
    drawState->setRenderTarget(target);
    drawState->setTexture(0, texture);

    matrix.setIDiv(texture->width(), texture->height());
    drawState->sampler(0)->reset(GrSamplerState::kClamp_WrapMode,
                                 GrSamplerState::kNearest_Filter,
                                 matrix);
    drawState->sampler(0)->setRAndBSwap(swapRAndB);

    GrVertexLayout layout = GrDrawTarget::StagePosAsTexCoordVertexLayoutBit(0);
    static const int VCOUNT = 4;
    // TODO: Use GrGpu::drawRect here
    GrDrawTarget::AutoReleaseGeometry geo(fGpu, layout, VCOUNT, 0);
    if (!geo.succeeded()) {
        GrPrintf("Failed to get space for vertices!\n");
        return;
    }
    ((GrPoint*)geo.vertices())->setIRectFan(0, 0, width, height);
    fGpu->drawNonIndexed(kTriangleFan_PrimitiveType, 0, VCOUNT);
}
////////////////////////////////////////////////////////////////////////////////

void GrContext::setPaint(const GrPaint& paint) {

    for (int i = 0; i < GrPaint::kMaxTextures; ++i) {
        int s = i + GrPaint::kFirstTextureStage;
        fDrawState->setTexture(s, paint.getTexture(i));
        ASSERT_OWNED_RESOURCE(paint.getTexture(i));
        if (paint.getTexture(i)) {
            *fDrawState->sampler(s) = paint.getTextureSampler(i);
        }
    }

    fDrawState->setFirstCoverageStage(GrPaint::kFirstMaskStage);

    for (int i = 0; i < GrPaint::kMaxMasks; ++i) {
        int s = i + GrPaint::kFirstMaskStage;
        fDrawState->setTexture(s, paint.getMask(i));
        ASSERT_OWNED_RESOURCE(paint.getMask(i));
        if (paint.getMask(i)) {
            *fDrawState->sampler(s) = paint.getMaskSampler(i);
        }
    }
    
    // disable all stages not accessible via the paint
    for (int s = GrPaint::kTotalStages; s < GrDrawState::kNumStages; ++s) {
        fDrawState->setTexture(s, NULL);
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
    if ((paint.getActiveMaskStageMask() || 0xff != paint.fCoverage) &&
        !fGpu->canApplyCoverage()) {
        GrPrintf("Partial pixel coverage will be incorrectly blended.\n");
    }
#endif
}

GrDrawTarget* GrContext::prepareToDraw(const GrPaint& paint,
                                       DrawCategory category) {
    if (category != fLastDrawCategory) {
        this->flushDrawBuffer();
        fLastDrawCategory = category;
    }
    this->setPaint(paint);
    GrDrawTarget* target = fGpu;
    switch (category) {
        case kUnbuffered_DrawCategory:
            target = fGpu;
            break;
        case kBuffered_DrawCategory:
            target = fDrawBuffer;
            fDrawBuffer->setClip(fGpu->getClip());
            break;
        default:
            GrCrash("Unexpected DrawCategory.");
            break;
    }
    return target;
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
            new GrPathRendererChain(this, GrPathRendererChain::kNone_UsageFlag);
    }

    GrPathRenderer* pr = fPathRendererChain->getPathRenderer(path, fill,
                                                             target,
                                                             antiAlias);

    if (NULL == pr && allowSW) {
        if (NULL == fSoftwarePathRenderer) {
            fSoftwarePathRenderer = new GrSoftwarePathRenderer(this);
        }

        pr = fSoftwarePathRenderer;
    }

    return pr;
}

////////////////////////////////////////////////////////////////////////////////

void GrContext::setRenderTarget(GrRenderTarget* target) {
    ASSERT_OWNED_RESOURCE(target);
    if (fDrawState->getRenderTarget() != target) {
        this->flush(false);
        fDrawState->setRenderTarget(target);
    }
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

void GrContext::resetStats() {
    fGpu->resetStats();
}

const GrGpuStats& GrContext::getStats() const {
    return fGpu->getStats();
}

void GrContext::printStats() const {
    fGpu->printStats();
}

GrContext::GrContext(GrGpu* gpu) {
    fGpu = gpu;
    fGpu->ref();
    fGpu->setContext(this);

    fDrawState = new GrDrawState();
    fGpu->setDrawState(fDrawState);

    fPathRendererChain = NULL;
    fSoftwarePathRenderer = NULL;

    fTextureCache = new GrResourceCache(MAX_TEXTURE_CACHE_COUNT,
                                        MAX_TEXTURE_CACHE_BYTES);
    fFontCache = new GrFontCache(fGpu);

    fLastDrawCategory = kUnbuffered_DrawCategory;

    fDrawBuffer = NULL;
    fDrawBufferVBAllocPool = NULL;
    fDrawBufferIBAllocPool = NULL;

    fAAFillRectIndexBuffer = NULL;
    fAAStrokeRectIndexBuffer = NULL;

    this->setupDrawBuffer();
}

void GrContext::setupDrawBuffer() {

    GrAssert(NULL == fDrawBuffer);
    GrAssert(NULL == fDrawBufferVBAllocPool);
    GrAssert(NULL == fDrawBufferIBAllocPool);

#if DEFER_TEXT_RENDERING || BATCH_RECT_TO_RECT || DEFER_PATHS
    fDrawBufferVBAllocPool =
        new GrVertexBufferAllocPool(fGpu, false,
                                    DRAW_BUFFER_VBPOOL_BUFFER_SIZE,
                                    DRAW_BUFFER_VBPOOL_PREALLOC_BUFFERS);
    fDrawBufferIBAllocPool =
        new GrIndexBufferAllocPool(fGpu, false,
                                   DRAW_BUFFER_IBPOOL_BUFFER_SIZE,
                                   DRAW_BUFFER_IBPOOL_PREALLOC_BUFFERS);

    fDrawBuffer = new GrInOrderDrawBuffer(fGpu,
                                          fDrawBufferVBAllocPool,
                                          fDrawBufferIBAllocPool);
#endif

#if BATCH_RECT_TO_RECT
    fDrawBuffer->setQuadIndexBuffer(this->getQuadIndexBuffer());
#endif
    fDrawBuffer->setAutoFlushTarget(fGpu);
    fDrawBuffer->setDrawState(fDrawState);
}

GrDrawTarget* GrContext::getTextTarget(const GrPaint& paint) {
#if DEFER_TEXT_RENDERING
    return prepareToDraw(paint, kBuffered_DrawCategory);
#else
    return prepareToDraw(paint, kUnbuffered_DrawCategory);
#endif
}

const GrIndexBuffer* GrContext::getQuadIndexBuffer() const {
    return fGpu->getQuadIndexBuffer();
}

GrTexture* GrContext::gaussianBlur(GrTexture* srcTexture,
                                   GrAutoScratchTexture* temp1,
                                   GrAutoScratchTexture* temp2,
                                   const SkRect& rect,
                                   float sigmaX, float sigmaY) {
    ASSERT_OWNED_RESOURCE(srcTexture);
    GrRenderTarget* oldRenderTarget = this->getRenderTarget();
    GrClip oldClip = this->getClip();
    GrTexture* origTexture = srcTexture;
    GrAutoMatrix avm(this, GrMatrix::I());
    SkIRect clearRect;
    int scaleFactorX, halfWidthX, kernelWidthX;
    int scaleFactorY, halfWidthY, kernelWidthY;
    sigmaX = adjust_sigma(sigmaX, &scaleFactorX, &halfWidthX, &kernelWidthX);
    sigmaY = adjust_sigma(sigmaY, &scaleFactorY, &halfWidthY, &kernelWidthY);

    SkRect srcRect(rect);
    scale_rect(&srcRect, 1.0f / scaleFactorX, 1.0f / scaleFactorY);
    srcRect.roundOut();
    scale_rect(&srcRect, static_cast<float>(scaleFactorX), 
                         static_cast<float>(scaleFactorY));
    this->setClip(srcRect);

    GrAssert(kBGRA_8888_PM_GrPixelConfig == srcTexture->config() ||
             kRGBA_8888_PM_GrPixelConfig == srcTexture->config() ||
             kAlpha_8_GrPixelConfig == srcTexture->config());

    const GrTextureDesc desc = {
        kRenderTarget_GrTextureFlagBit | kNoStencil_GrTextureFlagBit,
        SkScalarFloorToInt(srcRect.width()),
        SkScalarFloorToInt(srcRect.height()),
        srcTexture->config(), 
        0 // samples 
    };

    temp1->set(this, desc);
    if (temp2) {
        temp2->set(this, desc);
    }

    GrTexture* dstTexture = temp1->texture();
    GrPaint paint;
    paint.reset();
    paint.textureSampler(0)->setFilter(GrSamplerState::kBilinear_Filter);

    for (int i = 1; i < scaleFactorX || i < scaleFactorY; i *= 2) {
        paint.textureSampler(0)->matrix()->setIDiv(srcTexture->width(),
                                                   srcTexture->height());
        this->setRenderTarget(dstTexture->asRenderTarget());
        SkRect dstRect(srcRect);
        scale_rect(&dstRect, i < scaleFactorX ? 0.5f : 1.0f,
                            i < scaleFactorY ? 0.5f : 1.0f);
        paint.setTexture(0, srcTexture);
        this->drawRectToRect(paint, dstRect, srcRect);
        srcRect = dstRect;
        SkTSwap(srcTexture, dstTexture);
        // If temp2 is non-NULL, don't render back to origTexture
        if (temp2 && dstTexture == origTexture) dstTexture = temp2->texture();
    }

    SkIRect srcIRect;
    srcRect.roundOut(&srcIRect);

    if (sigmaX > 0.0f) {
        SkAutoTMalloc<float> kernelStorageX(kernelWidthX);
        float* kernelX = kernelStorageX.get();
        build_kernel(sigmaX, kernelX, kernelWidthX);

        if (scaleFactorX > 1) {
            // Clear out a halfWidth to the right of the srcRect to prevent the
            // X convolution from reading garbage.
            clearRect = SkIRect::MakeXYWH(srcIRect.fRight, srcIRect.fTop, 
                                          halfWidthX, srcIRect.height());
            this->clear(&clearRect, 0x0);
        }

        this->setRenderTarget(dstTexture->asRenderTarget());
        convolve(fGpu, srcTexture, srcRect, kernelX, kernelWidthX,
                 GrSamplerState::kX_FilterDirection);
        SkTSwap(srcTexture, dstTexture);
        if (temp2 && dstTexture == origTexture) {
            dstTexture = temp2->texture();
        }
    }

    if (sigmaY > 0.0f) {
        SkAutoTMalloc<float> kernelStorageY(kernelWidthY);
        float* kernelY = kernelStorageY.get();
        build_kernel(sigmaY, kernelY, kernelWidthY);

        if (scaleFactorY > 1 || sigmaX > 0.0f) {
            // Clear out a halfWidth below the srcRect to prevent the Y
            // convolution from reading garbage.
            clearRect = SkIRect::MakeXYWH(srcIRect.fLeft, srcIRect.fBottom, 
                                          srcIRect.width(), halfWidthY);
            this->clear(&clearRect, 0x0);
        }

        this->setRenderTarget(dstTexture->asRenderTarget());
        convolve(fGpu, srcTexture, srcRect, kernelY, kernelWidthY,
                 GrSamplerState::kY_FilterDirection);
        SkTSwap(srcTexture, dstTexture);
        if (temp2 && dstTexture == origTexture) {
            dstTexture = temp2->texture();
        }
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
        paint.textureSampler(0)->setFilter(GrSamplerState::kBilinear_Filter);
        paint.textureSampler(0)->matrix()->setIDiv(srcTexture->width(),
                                                   srcTexture->height());
        this->setRenderTarget(dstTexture->asRenderTarget());
        paint.setTexture(0, srcTexture);
        SkRect dstRect(srcRect);
        scale_rect(&dstRect, (float) scaleFactorX, (float) scaleFactorY);
        this->drawRectToRect(paint, dstRect, srcRect);
        srcRect = dstRect;
        SkTSwap(srcTexture, dstTexture);
    }
    this->setRenderTarget(oldRenderTarget);
    this->setClip(oldClip);
    return srcTexture;
}

GrTexture* GrContext::applyMorphology(GrTexture* srcTexture,
                                      const GrRect& rect,
                                      GrTexture* temp1, GrTexture* temp2,
                                      GrSamplerState::Filter filter,
                                      SkISize radius) {
    ASSERT_OWNED_RESOURCE(srcTexture);
    GrRenderTarget* oldRenderTarget = this->getRenderTarget();
    GrAutoMatrix avm(this, GrMatrix::I());
    GrClip oldClip = this->getClip();
    this->setClip(GrRect::MakeWH(SkIntToScalar(srcTexture->width()), 
                                 SkIntToScalar(srcTexture->height())));
    if (radius.fWidth > 0) {
        this->setRenderTarget(temp1->asRenderTarget());
        apply_morphology(fGpu, srcTexture, rect, radius.fWidth, filter,
                         GrSamplerState::kX_FilterDirection);
        SkIRect clearRect = SkIRect::MakeXYWH(
                    SkScalarFloorToInt(rect.fLeft), 
                    SkScalarFloorToInt(rect.fBottom),
                    SkScalarFloorToInt(rect.width()), 
                    radius.fHeight);
        this->clear(&clearRect, 0x0);
        srcTexture = temp1;
    }
    if (radius.fHeight > 0) {
        this->setRenderTarget(temp2->asRenderTarget());
        apply_morphology(fGpu, srcTexture, rect, radius.fHeight, filter,
                         GrSamplerState::kY_FilterDirection);
        srcTexture = temp2;
    }
    this->setRenderTarget(oldRenderTarget);
    this->setClip(oldClip);
    return srcTexture;
}

void GrContext::postClipPush() {
    fGpu->postClipPush();
}

void GrContext::preClipPop() {
    fGpu->preClipPop();
};

///////////////////////////////////////////////////////////////////////////////
