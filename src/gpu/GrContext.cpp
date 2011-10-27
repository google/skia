
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrBufferAllocPool.h"
#include "GrClipIterator.h"
#include "GrContext.h"
#include "GrGpu.h"
#include "GrIndexBuffer.h"
#include "GrInOrderDrawBuffer.h"
#include "GrPathRenderer.h"
#include "GrPathUtils.h"
#include "GrResourceCache.h"
#include "GrStencilBuffer.h"
#include "GrTextStrike.h"
#include "SkTLazy.h"
#include "SkTrace.h"

// Using MSAA seems to be slower for some yet unknown reason.
#define PREFER_MSAA_OFFSCREEN_AA 0
#define OFFSCREEN_SSAA_SCALE 4 // super sample at 4x4

#define DEFER_TEXT_RENDERING 1

#define BATCH_RECT_TO_RECT (1 && !GR_STATIC_RECT_VB)

// When we're using coverage AA but the blend is incompatible (given gpu
// limitations) should we disable AA or draw wrong?
#define DISABLE_COVERAGE_AA_FOR_BLEND 1

static const size_t MAX_TEXTURE_CACHE_COUNT = 256;
static const size_t MAX_TEXTURE_CACHE_BYTES = 16 * 1024 * 1024;

static const size_t DRAW_BUFFER_VBPOOL_BUFFER_SIZE = 1 << 18;
static const int DRAW_BUFFER_VBPOOL_PREALLOC_BUFFERS = 4;

// We are currently only batching Text and drawRectToRect, both
// of which use the quad index buffer.
static const size_t DRAW_BUFFER_IBPOOL_BUFFER_SIZE = 0;
static const int DRAW_BUFFER_IBPOOL_PREALLOC_BUFFERS = 0;

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

GrContext* GrContext::CreateGLShaderContext() {
    return GrContext::Create(kOpenGL_Shaders_GrEngine, 0);
}

GrContext::~GrContext() {
    this->flush();
    delete fTextureCache;
    delete fFontCache;
    delete fDrawBuffer;
    delete fDrawBufferVBAllocPool;
    delete fDrawBufferIBAllocPool;

    GrSafeUnref(fAAFillRectIndexBuffer);
    GrSafeUnref(fAAStrokeRectIndexBuffer);
    fGpu->unref();
    GrSafeUnref(fPathRendererChain);
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
    fTextureCache->removeAll();
    fFontCache->freeAll();
    // a path renderer may be holding onto resources
    GrSafeSetNull(fPathRendererChain);
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
                            const GrSamplerState& sampler,
                            GrContext::TextureKey clientKey,
                            int width,
                            int height,
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

    v[3] = 0;
    if (!gpu->getCaps().fNPOTTextureTileSupport) {
        bool isPow2 = GrIsPow2(width) && GrIsPow2(height);

        bool tiled = (sampler.getWrapX() != GrSamplerState::kClamp_WrapMode) ||
                     (sampler.getWrapY() != GrSamplerState::kClamp_WrapMode);

        if (tiled && !isPow2) {
            v[3] |= kNPOTBit;
            if (GrSamplerState::kNearest_Filter != sampler.getFilter()) {
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
}

GrContext::TextureCacheEntry GrContext::findAndLockTexture(TextureKey key,
                                                           int width,
                                                           int height,
                                                const GrSamplerState& sampler) {
    uint32_t v[4];
    gen_texture_key_values(fGpu, sampler, key, width, height, false, v);
    GrResourceKey resourceKey(v);
    return TextureCacheEntry(fTextureCache->findAndLock(resourceKey,
                                            GrResourceCache::kNested_LockType));
}

GrResourceEntry* GrContext::addAndLockStencilBuffer(GrStencilBuffer* sb) {
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

GrContext::TextureCacheEntry GrContext::createAndLockTexture(TextureKey key,
                                                const GrSamplerState& sampler,
                                                const GrTextureDesc& desc,
                                                void* srcData, size_t rowBytes) {
    SK_TRACE_EVENT0("GrContext::createAndLockTexture");

#if GR_DUMP_TEXTURE_UPLOAD
    GrPrintf("GrContext::createAndLockTexture [%d %d]\n", desc.fWidth, desc.fHeight);
#endif

    TextureCacheEntry entry;
    uint32_t v[4];
    bool special = gen_texture_key_values(fGpu, sampler, key,
                                          desc.fWidth, desc.fHeight, false, v);
    GrResourceKey resourceKey(v);

    if (special) {
        TextureCacheEntry clampEntry = 
                            findAndLockTexture(key, desc.fWidth, desc.fHeight,
                                               GrSamplerState::ClampNoFilter());

        if (NULL == clampEntry.texture()) {
            clampEntry = createAndLockTexture(key,
                                              GrSamplerState::ClampNoFilter(),
                                              desc, srcData, rowBytes);
            GrAssert(NULL != clampEntry.texture());
            if (NULL == clampEntry.texture()) {
                return entry;
            }
        }
        GrTextureDesc rtDesc = desc;
        rtDesc.fFlags =  rtDesc.fFlags |
                         kRenderTarget_GrTextureFlagBit |
                         kNoStencil_GrTextureFlagBit;
        rtDesc.fWidth  =
            GrNextPow2(GrMax<int>(desc.fWidth,
                                  fGpu->getCaps().fMinRenderTargetWidth));
        rtDesc.fHeight =
            GrNextPow2(GrMax<int>(desc.fHeight,
                                  fGpu->getCaps().fMinRenderTargetHeight));

        GrTexture* texture = fGpu->createTexture(rtDesc, NULL, 0);

        if (NULL != texture) {
            GrDrawTarget::AutoStateRestore asr(fGpu);
            fGpu->setRenderTarget(texture->asRenderTarget());
            fGpu->setTexture(0, clampEntry.texture());
            fGpu->disableStencil();
            fGpu->setViewMatrix(GrMatrix::I());
            fGpu->setAlpha(0xff);
            fGpu->setBlendFunc(kOne_BlendCoeff, kZero_BlendCoeff);
            fGpu->disableState(GrDrawTarget::kDither_StateBit |
                               GrDrawTarget::kClip_StateBit   |
                               GrDrawTarget::kHWAntialias_StateBit);
            GrSamplerState::Filter filter;
            // if filtering is not desired then we want to ensure all
            // texels in the resampled image are copies of texels from
            // the original.
            if (GrSamplerState::kNearest_Filter == sampler.getFilter()) {
                filter = GrSamplerState::kNearest_Filter;
            } else {
                filter = GrSamplerState::kBilinear_Filter;
            }
            GrSamplerState stretchSampler(GrSamplerState::kClamp_WrapMode,
                                          GrSamplerState::kClamp_WrapMode,
                                          filter);
            fGpu->setSamplerState(0, stretchSampler);

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
            int bpp = GrBytesPerPixel(desc.fFormat);
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
    GrContext::TextureKey descKey = desc.fAALevel |
                                    (desc.fFlags << 8) |
                                    ((uint64_t) desc.fFormat << 32);
    // this code path isn't friendly to tiling with NPOT restricitons
    // We just pass ClampNoFilter()
    gen_texture_key_values(gpu, GrSamplerState::ClampNoFilter(), descKey,
                            desc.fWidth, desc.fHeight, true, v);
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

    uint32_t p0 = desc.fFormat;
    uint32_t p1 = (desc.fAALevel << 16) | desc.fFlags;
    
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

GrResource* GrContext::createPlatformSurface(const GrPlatformSurfaceDesc& desc) {
    // validate flags here so that GrGpu subclasses don't have to check
    if (kTexture_GrPlatformSurfaceType == desc.fSurfaceType &&
        0 != desc.fRenderTargetFlags) {
        return NULL;
    }
    if (desc.fSampleCnt &&
        (kGrCanResolve_GrPlatformRenderTargetFlagBit & desc.fRenderTargetFlags)) {
        return NULL;
    }
    if (kTextureRenderTarget_GrPlatformSurfaceType == desc.fSurfaceType &&
        desc.fSampleCnt &&
        !(kGrCanResolve_GrPlatformRenderTargetFlagBit & desc.fRenderTargetFlags)) {
        return NULL;
    }
    return fGpu->createPlatformSurface(desc);
}

///////////////////////////////////////////////////////////////////////////////

bool GrContext::supportsIndex8PixelConfig(const GrSamplerState& sampler,
                                          int width, int height) const {
    const GrDrawTarget::Caps& caps = fGpu->getCaps();
    if (!caps.f8BitPaletteSupport) {
        return false;
    }

    bool isPow2 = GrIsPow2(width) && GrIsPow2(height);

    if (!isPow2) {
        if (!caps.fNPOTTextureSupport) {
            return false;
        }

        bool tiled = sampler.getWrapX() != GrSamplerState::kClamp_WrapMode ||
                     sampler.getWrapY() != GrSamplerState::kClamp_WrapMode;
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
    fGpu->enableState(GrDrawTarget::kClip_StateBit);
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
    GrAutoMatrix am;
    GrMatrix inverse;
    SkTLazy<GrPaint> tmpPaint;
    const GrPaint* p = &paint;
    // We attempt to map r by the inverse matrix and draw that. mapRect will
    // map the four corners and bound them with a new rect. This will not
    // produce a correct result for some perspective matrices.
    if (!this->getMatrix().hasPerspective()) {
        if (!fGpu->getViewInverse(&inverse)) {
            GrPrintf("Could not invert matrix");
            return;
        }
        inverse.mapRect(&r);
    } else {
        if (paint.getActiveMaskStageMask() || paint.getActiveStageMask()) {
            if (!fGpu->getViewInverse(&inverse)) {
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

struct GrContext::OffscreenRecord {
    enum Downsample {
        k4x4TwoPass_Downsample,
        k4x4SinglePass_Downsample,
        kFSAA_Downsample
    }                              fDownsample;
    int                            fTileSizeX;
    int                            fTileSizeY;
    int                            fTileCountX;
    int                            fTileCountY;
    int                            fScale;
    GrAutoScratchTexture           fOffscreen0;
    GrAutoScratchTexture           fOffscreen1;
    GrDrawTarget::SavedDrawState   fSavedState;
    GrClip                         fClip;
};

bool GrContext::doOffscreenAA(GrDrawTarget* target,
                              bool isHairLines) const {
#if !GR_USE_OFFSCREEN_AA
    return false;
#else
    // Line primitves are always rasterized as 1 pixel wide.
    // Super-sampling would make them too thin but MSAA would be OK.
    if (isHairLines &&
        (!PREFER_MSAA_OFFSCREEN_AA || !fGpu->getCaps().fFSAASupport)) {
        return false;
    }
    if (target->getRenderTarget()->isMultisampled()) {
        return false;
    }
    if (disable_coverage_aa_for_blend(target)) {
#if GR_DEBUG
        GrPrintf("Turning off AA to correctly apply blend.\n");
#endif
        return false;
    }
    return true;
#endif
}

bool GrContext::prepareForOffscreenAA(GrDrawTarget* target,
                                      bool requireStencil,
                                      const GrIRect& boundRect,
                                      GrPathRenderer* pr,
                                      OffscreenRecord* record) {

    GrAssert(GR_USE_OFFSCREEN_AA);

    GrAssert(NULL == record->fOffscreen0.texture());
    GrAssert(NULL == record->fOffscreen1.texture());
    GrAssert(!boundRect.isEmpty());

    int boundW = boundRect.width();
    int boundH = boundRect.height();

    GrTextureDesc desc;

    desc.fWidth  = GrMin(fMaxOffscreenAASize, boundW);
    desc.fHeight = GrMin(fMaxOffscreenAASize, boundH);

    if (requireStencil) {
        desc.fFlags = kRenderTarget_GrTextureFlagBit;
    } else {
        desc.fFlags = kRenderTarget_GrTextureFlagBit | 
                      kNoStencil_GrTextureFlagBit;
    }

    desc.fFormat = kRGBA_8888_GrPixelConfig;

    if (PREFER_MSAA_OFFSCREEN_AA && fGpu->getCaps().fFSAASupport) {
        record->fDownsample = OffscreenRecord::kFSAA_Downsample;
        record->fScale = 1;
        desc.fAALevel = kMed_GrAALevel;
    } else {
        record->fDownsample = fGpu->getCaps().fShaderSupport ?
                                OffscreenRecord::k4x4SinglePass_Downsample :
                                OffscreenRecord::k4x4TwoPass_Downsample;
        record->fScale = OFFSCREEN_SSAA_SCALE;
        // both downsample paths assume this
        GR_STATIC_ASSERT(4 == OFFSCREEN_SSAA_SCALE);
        desc.fAALevel = kNone_GrAALevel;
    }
    
    desc.fWidth *= record->fScale;
    desc.fHeight *= record->fScale;
    record->fOffscreen0.set(this, desc);
    if (NULL == record->fOffscreen0.texture()) {
        return false;
    }
    // the approximate lookup might have given us some slop space, might as well
    // use it when computing the tiles size.
    // these are scale values, will adjust after considering
    // the possible second offscreen.
    record->fTileSizeX = record->fOffscreen0.texture()->width();
    record->fTileSizeY = record->fOffscreen0.texture()->height();

    if (OffscreenRecord::k4x4TwoPass_Downsample == record->fDownsample) {
        desc.fWidth /= 2;
        desc.fHeight /= 2;
        record->fOffscreen1.set(this, desc);
        if (NULL == record->fOffscreen1.texture()) {
            return false;
        }
        record->fTileSizeX = GrMin(record->fTileSizeX, 
                                   2 * record->fOffscreen0.texture()->width());
        record->fTileSizeY = GrMin(record->fTileSizeY, 
                                   2 * record->fOffscreen0.texture()->height());
    }
    record->fTileSizeX /= record->fScale;
    record->fTileSizeY /= record->fScale;

    record->fTileCountX = GrIDivRoundUp(boundW, record->fTileSizeX);
    record->fTileCountY = GrIDivRoundUp(boundH, record->fTileSizeY);

    record->fClip = target->getClip();

    target->saveCurrentDrawState(&record->fSavedState);
    return true;
}

void GrContext::setupOffscreenAAPass1(GrDrawTarget* target,
                                      const GrIRect& boundRect,
                                      int tileX, int tileY,
                                      OffscreenRecord* record) {

    GrRenderTarget* offRT0 = record->fOffscreen0.texture()->asRenderTarget();
    GrAssert(NULL != offRT0);

    GrPaint tempPaint;
    tempPaint.reset();
    SetPaint(tempPaint, target);
    target->setRenderTarget(offRT0);
#if PREFER_MSAA_OFFSCREEN_AA
    target->enableState(GrDrawTarget::kHWAntialias_StateBit);
#endif

    GrMatrix transM;
    int left = boundRect.fLeft + tileX * record->fTileSizeX;
    int top =  boundRect.fTop  + tileY * record->fTileSizeY;
    transM.setTranslate(-left * GR_Scalar1, -top * GR_Scalar1);
    target->postConcatViewMatrix(transM);
    GrMatrix scaleM;
    scaleM.setScale(record->fScale * GR_Scalar1, record->fScale * GR_Scalar1);
    target->postConcatViewMatrix(scaleM);

    int w = (tileX == record->fTileCountX-1) ? boundRect.fRight - left :
                                               record->fTileSizeX;
    int h = (tileY == record->fTileCountY-1) ? boundRect.fBottom - top :
                                               record->fTileSizeY;
    GrIRect clear = SkIRect::MakeWH(record->fScale * w, 
                                    record->fScale * h);
    target->setClip(GrClip(clear));
#if 0
    // visualize tile boundaries by setting edges of offscreen to white
    // and interior to tranparent. black.
    target->clear(&clear, 0xffffffff);

    static const int gOffset = 2;
    GrIRect clear2 = SkIRect::MakeLTRB(gOffset, gOffset,
                                       record->fScale * w - gOffset,
                                       record->fScale * h - gOffset);
    target->clear(&clear2, 0x0);
#else
    target->clear(&clear, 0x0);
#endif
}

void GrContext::doOffscreenAAPass2(GrDrawTarget* target,
                                 const GrPaint& paint,
                                 const GrIRect& boundRect,
                                 int tileX, int tileY,
                                 OffscreenRecord* record) {
    SK_TRACE_EVENT0("GrContext::doOffscreenAAPass2");
    GrAssert(NULL != record->fOffscreen0.texture());
    GrDrawTarget::AutoGeometryPush agp(target);
    GrIRect tileRect;
    tileRect.fLeft = boundRect.fLeft + tileX * record->fTileSizeX;
    tileRect.fTop  = boundRect.fTop  + tileY * record->fTileSizeY,
    tileRect.fRight = (tileX == record->fTileCountX-1) ? 
                        boundRect.fRight :
                        tileRect.fLeft + record->fTileSizeX;
    tileRect.fBottom = (tileY == record->fTileCountY-1) ? 
                        boundRect.fBottom :
                        tileRect.fTop + record->fTileSizeY;

    GrSamplerState::Filter filter;
    if (OffscreenRecord::k4x4SinglePass_Downsample == record->fDownsample) {
        filter = GrSamplerState::k4x4Downsample_Filter;
    } else {
        filter = GrSamplerState::kBilinear_Filter;
    }

    GrMatrix sampleM;
    GrSamplerState sampler(GrSamplerState::kClamp_WrapMode, 
                           GrSamplerState::kClamp_WrapMode, filter);

    GrTexture* src = record->fOffscreen0.texture();
    int scale;

    enum {
        kOffscreenStage = GrPaint::kTotalStages,
    };

    if (OffscreenRecord::k4x4TwoPass_Downsample == record->fDownsample) {
        GrAssert(NULL != record->fOffscreen1.texture());
        scale = 2;
        GrRenderTarget* dst = record->fOffscreen1.texture()->asRenderTarget();
        
        // Do 2x2 downsample from first to second
        target->setTexture(kOffscreenStage, src);
        target->setRenderTarget(dst);
        target->setViewMatrix(GrMatrix::I());
        sampleM.setScale(scale * GR_Scalar1 / src->width(),
                         scale * GR_Scalar1 / src->height());
        sampler.setMatrix(sampleM);
        target->setSamplerState(kOffscreenStage, sampler);
        GrRect rect = SkRect::MakeWH(SkIntToScalar(scale * tileRect.width()),
                                     SkIntToScalar(scale * tileRect.height()));
        target->drawSimpleRect(rect, NULL, 1 << kOffscreenStage);
        
        src = record->fOffscreen1.texture();
    } else if (OffscreenRecord::kFSAA_Downsample == record->fDownsample) {
        scale = 1;
        GrIRect rect = SkIRect::MakeWH(tileRect.width(), tileRect.height());
        src->asRenderTarget()->overrideResolveRect(rect);
    } else {
        GrAssert(OffscreenRecord::k4x4SinglePass_Downsample == 
                 record->fDownsample);
        scale = 4;
    }

    // setup for draw back to main RT, we use the original
    // draw state setup by the caller plus an additional coverage
    // stage to handle the AA resolve. Also, we use an identity
    // view matrix and so pre-concat sampler matrices with view inv.
    int stageMask = paint.getActiveStageMask();

    target->restoreDrawState(record->fSavedState);
    target->setClip(record->fClip);

    if (stageMask) {
        GrMatrix invVM;
        if (target->getViewInverse(&invVM)) {
            target->preConcatSamplerMatrices(stageMask, invVM);
        }
    }
    // This is important when tiling, otherwise second tile's 
    // pass 1 view matrix will be incorrect.
    GrDrawTarget::AutoViewMatrixRestore avmr(target);

    target->setViewMatrix(GrMatrix::I());

    target->setTexture(kOffscreenStage, src);
    sampleM.setScale(scale * GR_Scalar1 / src->width(),
                     scale * GR_Scalar1 / src->height());
    sampler.setMatrix(sampleM);
    sampleM.setTranslate(SkIntToScalar(-tileRect.fLeft),
                         SkIntToScalar(-tileRect.fTop));
    sampler.preConcatMatrix(sampleM);
    target->setSamplerState(kOffscreenStage, sampler);

    GrRect dstRect;
    int stages = (1 << kOffscreenStage) | stageMask;
    dstRect.set(tileRect);
    target->drawSimpleRect(dstRect, NULL, stages);
}

void GrContext::cleanupOffscreenAA(GrDrawTarget* target,
                                   GrPathRenderer* pr,
                                   OffscreenRecord* record) {
    target->restoreDrawState(record->fSavedState);
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
        if (NULL != target->getTexture(s)) {
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
        innerColor = target->getColor();
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
        innerColor = target->getColor();
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
    // we use a simple alpha ramp to do aa on axis-aligned rects
    // do AA with alpha ramp if the caller requested AA, the rect 
    // will be axis-aligned, and the rect won't land on integer coords.

    // we are keeping around the "tweak the alpha" trick because
    // it is our only hope for the fixed-pipe implementation.
    // In a shader implementation we can give a separate coverage input
    // TODO: remove this ugliness when we drop the fixed-pipe impl
    *useVertexCoverage = false;
    if (!target->canTweakAlphaForCoverage()) {
        if (target->getCaps().fSupportPerVertexCoverage) {
            if (disable_coverage_aa_for_blend(target)) {
#if GR_DEBUG
                GrPrintf("Turning off AA to correctly apply blend.\n");
#endif
                return false;
            } else {
                *useVertexCoverage = true;
            }
        } else {
            GrPrintf("Rect AA dropped because no support for coverage.\n");
            return false;
        }
    }

    if (target->getRenderTarget()->isMultisampled()) {
        return false;
    }

    if (0 == width && target->willUseHWAALines()) {
        return false;
    }

    if (!target->getViewMatrix().preservesAxisAlignment()) {
        return false;
    }

    if (NULL != matrix && 
        !matrix->preservesAxisAlignment()) {
        return false;
    }

    *combinedMatrix = target->getViewMatrix();
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
        GrDrawTarget::AutoViewMatrixRestore avm(target);
        if (stageMask) {
            GrMatrix inv;
            if (combinedMatrix.invert(&inv)) {
                target->preConcatSamplerMatrices(stageMask, inv);
            }
        }
        target->setViewMatrix(GrMatrix::I());
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

        GrDrawTarget::AutoViewMatrixRestore avmr;
        if (NULL != matrix) {
            avmr.set(target);
            target->preConcatViewMatrix(*matrix);
            target->preConcatSamplerMatrices(stageMask, *matrix);
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
            GrDrawTarget::AutoViewMatrixRestore avmr(target);
            GrMatrix m;
            m.setAll(rect.width(),    0,             rect.fLeft,
                        0,            rect.height(), rect.fTop,
                        0,            0,             GrMatrix::I()[8]);

            if (NULL != matrix) {
                m.postConcat(*matrix);
            }

            target->preConcatViewMatrix(m);
            target->preConcatSamplerMatrices(stageMask, m);
 
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
    
    GrVertexLayout layout = PaintStageVertexLayoutBits(paint, NULL);
    GrDrawTarget::AutoViewMatrixRestore avmr(target);

    GrMatrix m;

    m.setAll(dstRect.width(), 0,                dstRect.fLeft,
             0,               dstRect.height(), dstRect.fTop,
             0,               0,                GrMatrix::I()[8]);
    if (NULL != dstMatrix) {
        m.postConcat(*dstMatrix);
    }
    target->preConcatViewMatrix(m);

    // srcRect refers to first stage
    int otherStageMask = paint.getActiveStageMask() & 
                         (~(1 << GrPaint::kFirstTextureStage));
    if (otherStageMask) {
        target->preConcatSamplerMatrices(otherStageMask, m);
    }

    m.setAll(srcRect.width(), 0,                srcRect.fLeft,
             0,               srcRect.height(), srcRect.fTop,
             0,               0,                GrMatrix::I()[8]);
    if (NULL != srcMatrix) {
        m.postConcat(*srcMatrix);
    }
    target->preConcatSamplerMatrix(GrPaint::kFirstTextureStage, m);

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

void GrContext::drawPath(const GrPaint& paint, const GrPath& path,
                         GrPathFill fill, const GrPoint* translate) {

    if (path.isEmpty()) {
#if GR_DEBUG
       GrPrintf("Empty path should have been caught by canvas.\n");
#endif
       if (GrIsFillInverted(fill)) {
           this->drawPaint(paint);
       }
       return;
    }

    GrDrawTarget* target = this->prepareToDraw(paint, kUnbuffered_DrawCategory);

    bool prAA = paint.fAntiAlias && !this->getRenderTarget()->isMultisampled();

    // An Assumption here is that path renderer would use some form of tweaking
    // the src color (either the input alpha or in the frag shader) to implement
    // aa. If we have some future driver-mojo path AA that can do the right
    // thing WRT to the blend then we'll need some query on the PR.
    if (disable_coverage_aa_for_blend(target)) {
#if GR_DEBUG
        GrPrintf("Turning off AA to correctly apply blend.\n");
#endif
        prAA = false;
    }

    bool doOSAA = false;
    GrPathRenderer* pr = NULL;
    if (prAA) {
        pr = this->getPathRenderer(path, fill, true);
        if (NULL == pr) {
            prAA = false;
            doOSAA = this->doOffscreenAA(target, kHairLine_PathFill == fill);
            pr = this->getPathRenderer(path, fill, false);
        }
    } else {
        pr = this->getPathRenderer(path, fill, false);
    }

    if (NULL == pr) {
#if GR_DEBUG
        GrPrintf("Unable to find path renderer compatible with path.\n");
#endif
        return;
    }

    GrPathRenderer::AutoClearPath arp(pr, target, &path, fill, prAA, translate);
    GrDrawTarget::StageBitfield stageMask = paint.getActiveStageMask();

    if (doOSAA) {
        bool needsStencil = pr->requiresStencilPass(target, path, fill);

        // compute bounds as intersection of rt size, clip, and path
        GrIRect bound = SkIRect::MakeWH(target->getRenderTarget()->width(), 
                                        target->getRenderTarget()->height());
        GrIRect clipIBounds;
        if (target->getClip().hasConservativeBounds()) {
            target->getClip().getConservativeBounds().roundOut(&clipIBounds);
            if (!bound.intersect(clipIBounds)) {
                return;
            }
        }

        GrRect pathBounds = path.getBounds();
        if (!pathBounds.isEmpty()) {
            if (NULL != translate) {
                pathBounds.offset(*translate);
            }
            target->getViewMatrix().mapRect(&pathBounds, pathBounds);
            GrIRect pathIBounds;
            pathBounds.roundOut(&pathIBounds);
            if (!bound.intersect(pathIBounds)) {
                return;
            }
        }
        OffscreenRecord record;
        if (this->prepareForOffscreenAA(target, needsStencil, bound,
                                        pr, &record)) {
            for (int tx = 0; tx < record.fTileCountX; ++tx) {
                for (int ty = 0; ty < record.fTileCountY; ++ty) {
                    this->setupOffscreenAAPass1(target, bound, tx, ty, &record);
                    pr->drawPath(0);
                    this->doOffscreenAAPass2(target, paint, bound, tx, ty, &record);
                }
            }
            this->cleanupOffscreenAA(target, pr, &record);
            if (GrIsFillInverted(fill) && bound != clipIBounds) {
                GrDrawTarget::AutoDeviceCoordDraw adcd(target, stageMask);
                GrRect rect;
                if (clipIBounds.fTop < bound.fTop) {
                    rect.iset(clipIBounds.fLeft, clipIBounds.fTop, 
                              clipIBounds.fRight, bound.fTop);
                    target->drawSimpleRect(rect, NULL, stageMask);
                }
                if (clipIBounds.fLeft < bound.fLeft) {
                    rect.iset(clipIBounds.fLeft, bound.fTop, 
                              bound.fLeft, bound.fBottom);
                    target->drawSimpleRect(rect, NULL, stageMask);
                }
                if (clipIBounds.fRight > bound.fRight) {
                    rect.iset(bound.fRight, bound.fTop, 
                              clipIBounds.fRight, bound.fBottom);
                    target->drawSimpleRect(rect, NULL, stageMask);
                }
                if (clipIBounds.fBottom > bound.fBottom) {
                    rect.iset(clipIBounds.fLeft, bound.fBottom, 
                              clipIBounds.fRight, clipIBounds.fBottom);
                    target->drawSimpleRect(rect, NULL, stageMask);
                }
            }
            return;
        }
    } 
    pr->drawPath(stageMask);
}

////////////////////////////////////////////////////////////////////////////////

bool GrContext::supportsShaders() const {
    return fGpu->getCaps().fShaderSupport;
}

void GrContext::flush(int flagsBitfield) {
    if (kDiscard_FlushBit & flagsBitfield) {
        fDrawBuffer->reset();
    } else {
        flushDrawBuffer();
    }

    if (kForceCurrentRenderTarget_FlushBit & flagsBitfield) {
        fGpu->forceRenderTargetFlush();
    }
}

void GrContext::flushText() {
    if (kText_DrawCategory == fLastDrawCategory) {
        flushDrawBuffer();
    }
}

void GrContext::flushDrawBuffer() {
#if BATCH_RECT_TO_RECT || DEFER_TEXT_RENDERING
    if (fDrawBuffer) {
        fDrawBuffer->playback(fGpu);
        fDrawBuffer->reset();
    }
#endif
}

bool GrContext::readTexturePixels(GrTexture* texture,
                                  int left, int top, int width, int height,
                                  GrPixelConfig config, void* buffer) {
    SK_TRACE_EVENT0("GrContext::readTexturePixels");

    // TODO: code read pixels for textures that aren't rendertargets

    this->flush();
    GrRenderTarget* target = texture->asRenderTarget();
    if (NULL != target) {
        return fGpu->readPixels(target,
                                left, top, width, height, 
                                config, buffer);
    } else {
        return false;
    }
}

bool GrContext::readRenderTargetPixels(GrRenderTarget* target,
                                      int left, int top, int width, int height,
                                      GrPixelConfig config, void* buffer) {
    SK_TRACE_EVENT0("GrContext::readRenderTargetPixels");
    uint32_t flushFlags = 0;
    if (NULL == target) { 
        flushFlags |= GrContext::kForceCurrentRenderTarget_FlushBit;
    }

    this->flush(flushFlags);
    return fGpu->readPixels(target,
                            left, top, width, height, 
                            config, buffer);
}

void GrContext::writePixels(int left, int top, int width, int height,
                            GrPixelConfig config, const void* buffer,
                            size_t stride) {
    SK_TRACE_EVENT0("GrContext::writePixels");

    // TODO: when underlying api has a direct way to do this we should use it
    // (e.g. glDrawPixels on desktop GL).

    this->flush(true);

    const GrTextureDesc desc = {
        kNone_GrTextureFlags, kNone_GrAALevel, width, height, config
    };
    GrAutoScratchTexture ast(this, desc);
    GrTexture* texture = ast.texture();
    if (NULL == texture) {
        return;
    }
    texture->uploadTextureData(0, 0, width, height, buffer, stride);

    GrDrawTarget::AutoStateRestore  asr(fGpu);

    GrMatrix matrix;
    matrix.setTranslate(GrIntToScalar(left), GrIntToScalar(top));
    fGpu->setViewMatrix(matrix);

    fGpu->setColorFilter(0, SkXfermode::kDst_Mode);
    fGpu->disableState(GrDrawTarget::kClip_StateBit);
    fGpu->setAlpha(0xFF);
    fGpu->setBlendFunc(kOne_BlendCoeff,
                       kZero_BlendCoeff);
    fGpu->setTexture(0, texture);

    GrSamplerState sampler;
    sampler.setClampNoFilter();
    matrix.setIDiv(texture->width(), texture->height());
    sampler.setMatrix(matrix);
    fGpu->setSamplerState(0, sampler);

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

void GrContext::SetPaint(const GrPaint& paint, GrDrawTarget* target) {

    for (int i = 0; i < GrPaint::kMaxTextures; ++i) {
        int s = i + GrPaint::kFirstTextureStage;
        target->setTexture(s, paint.getTexture(i));
        target->setSamplerState(s, *paint.getTextureSampler(i));
    }

    target->setFirstCoverageStage(GrPaint::kFirstMaskStage);

    for (int i = 0; i < GrPaint::kMaxMasks; ++i) {
        int s = i + GrPaint::kFirstMaskStage;
        target->setTexture(s, paint.getMask(i));
        target->setSamplerState(s, *paint.getMaskSampler(i));
    }

    target->setColor(paint.fColor);

    if (paint.fDither) {
        target->enableState(GrDrawTarget::kDither_StateBit);
    } else {
        target->disableState(GrDrawTarget::kDither_StateBit);
    }
    if (paint.fAntiAlias) {
        target->enableState(GrDrawTarget::kHWAntialias_StateBit);
    } else {
        target->disableState(GrDrawTarget::kHWAntialias_StateBit);
    }
    target->setBlendFunc(paint.fSrcBlendCoeff, paint.fDstBlendCoeff);
    target->setColorFilter(paint.fColorFilterColor, paint.fColorFilterXfermode);

    if (paint.getActiveMaskStageMask() && !target->canApplyCoverage()) {
        GrPrintf("Partial pixel coverage will be incorrectly blended.\n");
    }
}

GrDrawTarget* GrContext::prepareToDraw(const GrPaint& paint,
                                       DrawCategory category) {
    if (category != fLastDrawCategory) {
        flushDrawBuffer();
        fLastDrawCategory = category;
    }
    SetPaint(paint, fGpu);
    GrDrawTarget* target = fGpu;
    switch (category) {
    case kText_DrawCategory:
#if DEFER_TEXT_RENDERING
        target = fDrawBuffer;
        fDrawBuffer->initializeDrawStateAndClip(*fGpu);
#else
        target = fGpu;
#endif
        break;
    case kUnbuffered_DrawCategory:
        target = fGpu;
        break;
    case kBuffered_DrawCategory:
        target = fDrawBuffer;
        fDrawBuffer->initializeDrawStateAndClip(*fGpu);
        break;
    }
    return target;
}

GrPathRenderer* GrContext::getPathRenderer(const GrPath& path,
                                           GrPathFill fill,
                                           bool antiAlias) {
    if (NULL == fPathRendererChain) {
        fPathRendererChain = 
            new GrPathRendererChain(this, GrPathRendererChain::kNone_UsageFlag);
    }
    return fPathRendererChain->getPathRenderer(fGpu->getCaps(), path,
                                               fill, antiAlias);
}

////////////////////////////////////////////////////////////////////////////////

void GrContext::setRenderTarget(GrRenderTarget* target) {
    this->flush(false);
    fGpu->setRenderTarget(target);
}

GrRenderTarget* GrContext::getRenderTarget() {
    return fGpu->getRenderTarget();
}

const GrRenderTarget* GrContext::getRenderTarget() const {
    return fGpu->getRenderTarget();
}

const GrMatrix& GrContext::getMatrix() const {
    return fGpu->getViewMatrix();
}

void GrContext::setMatrix(const GrMatrix& m) {
    fGpu->setViewMatrix(m);
}

void GrContext::concatMatrix(const GrMatrix& m) const {
    fGpu->preConcatViewMatrix(m);
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

    fPathRendererChain = NULL;

    fTextureCache = new GrResourceCache(MAX_TEXTURE_CACHE_COUNT,
                                        MAX_TEXTURE_CACHE_BYTES);
    fFontCache = new GrFontCache(fGpu);

    fLastDrawCategory = kUnbuffered_DrawCategory;

    fDrawBuffer = NULL;
    fDrawBufferVBAllocPool = NULL;
    fDrawBufferIBAllocPool = NULL;

    fAAFillRectIndexBuffer = NULL;
    fAAStrokeRectIndexBuffer = NULL;
    
    int gpuMaxOffscreen = gpu->getCaps().fMaxRenderTargetSize;
    if (!PREFER_MSAA_OFFSCREEN_AA || !gpu->getCaps().fFSAASupport) {
        gpuMaxOffscreen /= OFFSCREEN_SSAA_SCALE;
    }
    fMaxOffscreenAASize = GrMin(GR_MAX_OFFSCREEN_AA_SIZE, gpuMaxOffscreen);

    this->setupDrawBuffer();
}

void GrContext::setupDrawBuffer() {

    GrAssert(NULL == fDrawBuffer);
    GrAssert(NULL == fDrawBufferVBAllocPool);
    GrAssert(NULL == fDrawBufferIBAllocPool);

#if DEFER_TEXT_RENDERING || BATCH_RECT_TO_RECT
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
}

GrDrawTarget* GrContext::getTextTarget(const GrPaint& paint) {
    GrDrawTarget* target;
#if DEFER_TEXT_RENDERING
    target = prepareToDraw(paint, kText_DrawCategory);
#else
    target = prepareToDraw(paint, kUnbuffered_DrawCategory);
#endif
    SetPaint(paint, target);
    return target;
}

const GrIndexBuffer* GrContext::getQuadIndexBuffer() const {
    return fGpu->getQuadIndexBuffer();
}

void GrContext::convolveInX(GrTexture* texture,
                            const SkRect& rect,
                            const float* kernel,
                            int kernelWidth) {
    float imageIncrement[2] = {1.0f / texture->width(), 0.0f};
    convolve(texture, rect, imageIncrement, kernel, kernelWidth);
}

void GrContext::convolveInY(GrTexture* texture,
                            const SkRect& rect,
                            const float* kernel,
                            int kernelWidth) {
    float imageIncrement[2] = {0.0f, 1.0f / texture->height()};
    convolve(texture, rect, imageIncrement, kernel, kernelWidth);
}

void GrContext::convolve(GrTexture* texture,
                         const SkRect& rect,
                         float imageIncrement[2],
                         const float* kernel,
                         int kernelWidth) {
    GrDrawTarget::AutoStateRestore asr(fGpu);
    GrMatrix sampleM;
    GrSamplerState sampler(GrSamplerState::kClamp_WrapMode, 
                           GrSamplerState::kClamp_WrapMode,
                           GrSamplerState::kConvolution_Filter);
    sampler.setConvolutionParams(kernelWidth, kernel, imageIncrement);
    sampleM.setScale(GR_Scalar1 / texture->width(),
                     GR_Scalar1 / texture->height());
    sampler.setMatrix(sampleM);
    fGpu->setSamplerState(0, sampler);
    fGpu->setViewMatrix(GrMatrix::I());
    fGpu->setTexture(0, texture);
    fGpu->setBlendFunc(kOne_BlendCoeff, kZero_BlendCoeff);
    fGpu->drawSimpleRect(rect, NULL, 1 << 0);
}
