
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"

#include "GrAARectRenderer.h"
#include "GrBufferAllocPool.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrFontCache.h"
#include "GrGpuResource.h"
#include "GrGpuResourcePriv.h"
#include "GrDistanceFieldTextContext.h"
#include "GrDrawTargetCaps.h"
#include "GrGpu.h"
#include "GrIndexBuffer.h"
#include "GrInOrderDrawBuffer.h"
#include "GrLayerCache.h"
#include "GrOvalRenderer.h"
#include "GrPathRenderer.h"
#include "GrPathUtils.h"
#include "GrResourceCache.h"
#include "GrSoftwarePathRenderer.h"
#include "GrStencilAndCoverTextContext.h"
#include "GrStrokeInfo.h"
#include "GrSurfacePriv.h"
#include "GrTexturePriv.h"
#include "GrTraceMarker.h"
#include "GrTracing.h"
#include "SkDashPathPriv.h"
#include "SkConfig8888.h"
#include "SkGr.h"
#include "SkRRect.h"
#include "SkStrokeRec.h"
#include "SkTLazy.h"
#include "SkTLS.h"
#include "SkTraceEvent.h"

#include "effects/GrConfigConversionEffect.h"
#include "effects/GrDashingEffect.h"
#include "effects/GrSingleTextureEffect.h"

static const size_t DRAW_BUFFER_VBPOOL_BUFFER_SIZE = 1 << 15;
static const int DRAW_BUFFER_VBPOOL_PREALLOC_BUFFERS = 4;

static const size_t DRAW_BUFFER_IBPOOL_BUFFER_SIZE = 1 << 11;
static const int DRAW_BUFFER_IBPOOL_PREALLOC_BUFFERS = 4;

#define ASSERT_OWNED_RESOURCE(R) SkASSERT(!(R) || (R)->getContext() == this)

class GrContext::AutoCheckFlush {
public:
    AutoCheckFlush(GrContext* context) : fContext(context) { SkASSERT(context); }

    ~AutoCheckFlush() {
        if (fContext->fFlushToReduceCacheSize) {
            fContext->flush();
        }
    }

private:
    GrContext* fContext;
};

GrContext* GrContext::Create(GrBackend backend, GrBackendContext backendContext,
                             const Options* opts) {
    GrContext* context;
    if (NULL == opts) {
        context = SkNEW_ARGS(GrContext, (Options()));
    } else {
        context = SkNEW_ARGS(GrContext, (*opts));
    }

    if (context->init(backend, backendContext)) {
        return context;
    } else {
        context->unref();
        return NULL;
    }
}

GrContext::GrContext(const Options& opts) : fOptions(opts) {
    fGpu = NULL;
    fClip = NULL;
    fPathRendererChain = NULL;
    fSoftwarePathRenderer = NULL;
    fResourceCache = NULL;
    fFontCache = NULL;
    fDrawBuffer = NULL;
    fDrawBufferVBAllocPool = NULL;
    fDrawBufferIBAllocPool = NULL;
    fFlushToReduceCacheSize = false;
    fAARectRenderer = NULL;
    fOvalRenderer = NULL;
    fMaxTextureSizeOverride = 1 << 20;
}

bool GrContext::init(GrBackend backend, GrBackendContext backendContext) {
    SkASSERT(NULL == fGpu);

    fGpu = GrGpu::Create(backend, backendContext, this);
    if (NULL == fGpu) {
        return false;
    }
    this->initCommon();
    return true;
}

void GrContext::initCommon() {
    fResourceCache = SkNEW(GrResourceCache);
    fResourceCache->setOverBudgetCallback(OverBudgetCB, this);

    fFontCache = SkNEW_ARGS(GrFontCache, (fGpu));

    fLayerCache.reset(SkNEW_ARGS(GrLayerCache, (this)));

    fAARectRenderer = SkNEW_ARGS(GrAARectRenderer, (fGpu));
    fOvalRenderer = SkNEW_ARGS(GrOvalRenderer, (fGpu));

    fDidTestPMConversions = false;

    this->setupDrawBuffer();
}

GrContext::~GrContext() {
    if (NULL == fGpu) {
        return;
    }

    this->flush();

    for (int i = 0; i < fCleanUpData.count(); ++i) {
        (*fCleanUpData[i].fFunc)(this, fCleanUpData[i].fInfo);
    }

    SkDELETE(fResourceCache);
    SkDELETE(fFontCache);
    SkDELETE(fDrawBuffer);
    SkDELETE(fDrawBufferVBAllocPool);
    SkDELETE(fDrawBufferIBAllocPool);

    fAARectRenderer->unref();
    fOvalRenderer->unref();

    fGpu->unref();
    SkSafeUnref(fPathRendererChain);
    SkSafeUnref(fSoftwarePathRenderer);
}

void GrContext::abandonContext() {
    // abandon first to so destructors
    // don't try to free the resources in the API.
    fResourceCache->abandonAll();

    fGpu->contextAbandoned();

    // a path renderer may be holding onto resources that
    // are now unusable
    SkSafeSetNull(fPathRendererChain);
    SkSafeSetNull(fSoftwarePathRenderer);

    delete fDrawBuffer;
    fDrawBuffer = NULL;

    delete fDrawBufferVBAllocPool;
    fDrawBufferVBAllocPool = NULL;

    delete fDrawBufferIBAllocPool;
    fDrawBufferIBAllocPool = NULL;

    fAARectRenderer->reset();
    fOvalRenderer->reset();

    fFontCache->freeAll();
    fLayerCache->freeAll();
}

void GrContext::resetContext(uint32_t state) {
    fGpu->markContextDirty(state);
}

void GrContext::freeGpuResources() {
    this->flush();

    if (fDrawBuffer) {
        fDrawBuffer->purgeResources();
    }

    fAARectRenderer->reset();
    fOvalRenderer->reset();

    fFontCache->freeAll();
    fLayerCache->freeAll();
    // a path renderer may be holding onto resources
    SkSafeSetNull(fPathRendererChain);
    SkSafeSetNull(fSoftwarePathRenderer);
}

void GrContext::getResourceCacheUsage(int* resourceCount, size_t* resourceBytes) const {
    if (resourceCount) {
        *resourceCount = fResourceCache->getBudgetedResourceCount();
    }
    if (resourceBytes) {
        *resourceBytes = fResourceCache->getBudgetedResourceBytes();
    }
}

GrTextContext* GrContext::createTextContext(GrRenderTarget* renderTarget,
                                            const SkDeviceProperties&
                                            leakyProperties,
                                            bool enableDistanceFieldFonts) {
    if (fGpu->caps()->pathRenderingSupport() && renderTarget->getStencilBuffer() && 
                                                renderTarget->isMultisampled()) {
        return GrStencilAndCoverTextContext::Create(this, leakyProperties);
    } 

    return GrDistanceFieldTextContext::Create(this, leakyProperties, enableDistanceFieldFonts);
}

////////////////////////////////////////////////////////////////////////////////
enum ScratchTextureFlags {
    kExact_ScratchTextureFlag           = 0x1,
    kNoPendingIO_ScratchTextureFlag     = 0x2,
    kNoCreate_ScratchTextureFlag        = 0x4,
};

bool GrContext::isConfigTexturable(GrPixelConfig config) const {
    return fGpu->caps()->isConfigTexturable(config);
}

bool GrContext::npotTextureTileSupport() const {
    return fGpu->caps()->npotTextureTileSupport();
}

GrTexture* GrContext::createTexture(const GrSurfaceDesc& desc, bool budgeted, const void* srcData,
                                    size_t rowBytes) {
    if ((desc.fFlags & kRenderTarget_GrSurfaceFlag) &&
        !this->isConfigRenderable(desc.fConfig, desc.fSampleCnt > 0)) {
        return NULL;
    }
    if (!GrPixelConfigIsCompressed(desc.fConfig)) {
        static const uint32_t kFlags = kExact_ScratchTextureFlag |
                                       kNoCreate_ScratchTextureFlag;
        if (GrTexture* texture = this->internalRefScratchTexture(desc, kFlags)) {
            if (!srcData || texture->writePixels(0, 0, desc.fWidth, desc.fHeight, desc.fConfig,
                                                 srcData, rowBytes)) {
                if (!budgeted) {
                    texture->resourcePriv().makeUnbudgeted();
                }
                return texture;
            }
            texture->unref();
        }
    }
    return fGpu->createTexture(desc, budgeted, srcData, rowBytes);
}

GrTexture* GrContext::refScratchTexture(const GrSurfaceDesc& desc, ScratchTexMatch match,
                                        bool calledDuringFlush) {
    // Currently we don't recycle compressed textures as scratch.
    if (GrPixelConfigIsCompressed(desc.fConfig)) {
        return NULL;
    } else {
        uint32_t flags = 0;
        if (kExact_ScratchTexMatch == match) {
            flags |= kExact_ScratchTextureFlag;
        }
        if (calledDuringFlush) {
            flags |= kNoPendingIO_ScratchTextureFlag;
        }
        return this->internalRefScratchTexture(desc, flags);
    }
}

GrTexture* GrContext::internalRefScratchTexture(const GrSurfaceDesc& inDesc, uint32_t flags) {
    SkASSERT(!GrPixelConfigIsCompressed(inDesc.fConfig));
    // kNoStencil has no meaning if kRT isn't set.
    SkASSERT((inDesc.fFlags & kRenderTarget_GrSurfaceFlag) ||
             !(inDesc.fFlags & kNoStencil_GrSurfaceFlag));

    SkTCopyOnFirstWrite<GrSurfaceDesc> desc(inDesc);

    if (fGpu->caps()->reuseScratchTextures() || (desc->fFlags & kRenderTarget_GrSurfaceFlag)) {
        GrSurfaceFlags origFlags = desc->fFlags;
        if (!(kExact_ScratchTextureFlag & flags)) {
            // bin by pow2 with a reasonable min
            static const int MIN_SIZE = 16;
            GrSurfaceDesc* wdesc = desc.writable();
            wdesc->fWidth  = SkTMax(MIN_SIZE, GrNextPow2(desc->fWidth));
            wdesc->fHeight = SkTMax(MIN_SIZE, GrNextPow2(desc->fHeight));
        }

        do {
            GrScratchKey key;
            GrTexturePriv::ComputeScratchKey(*desc, &key);
            uint32_t scratchFlags = 0;
            if (kNoPendingIO_ScratchTextureFlag & flags) {
                scratchFlags = GrResourceCache::kRequireNoPendingIO_ScratchFlag;
            } else  if (!(desc->fFlags & kRenderTarget_GrSurfaceFlag)) {
                // If it is not a render target then it will most likely be populated by
                // writePixels() which will trigger a flush if the texture has pending IO.
                scratchFlags = GrResourceCache::kPreferNoPendingIO_ScratchFlag;
            }
            GrGpuResource* resource = fResourceCache->findAndRefScratchResource(key, scratchFlags);
            if (resource) {
                GrSurface* surface = static_cast<GrSurface*>(resource);
                GrRenderTarget* rt = surface->asRenderTarget();
                if (rt && fGpu->caps()->discardRenderTargetSupport()) {
                    rt->discard();
                }
                return surface->asTexture();
            }

            if (kExact_ScratchTextureFlag & flags) {
                break;
            }
            // We had a cache miss and we are in approx mode, relax the fit of the flags.

            // We no longer try to reuse textures that were previously used as render targets in
            // situations where no RT is needed; doing otherwise can confuse the video driver and
            // cause significant performance problems in some cases.
            if (desc->fFlags & kNoStencil_GrSurfaceFlag) {
                desc.writable()->fFlags = desc->fFlags & ~kNoStencil_GrSurfaceFlag;
            } else {
                break;
            }

        } while (true);

        desc.writable()->fFlags = origFlags;
    }

    if (!(kNoCreate_ScratchTextureFlag & flags)) {
        return fGpu->createTexture(*desc, true, NULL, 0);
    }

    return NULL;
}

void GrContext::OverBudgetCB(void* data) {
    SkASSERT(data);

    GrContext* context = reinterpret_cast<GrContext*>(data);

    // Flush the InOrderDrawBuffer to possibly free up some textures
    context->fFlushToReduceCacheSize = true;
}

int GrContext::getMaxTextureSize() const {
    return SkTMin(fGpu->caps()->maxTextureSize(), fMaxTextureSizeOverride);
}

int GrContext::getMaxRenderTargetSize() const {
    return fGpu->caps()->maxRenderTargetSize();
}

int GrContext::getMaxSampleCount() const {
    return fGpu->caps()->maxSampleCount();
}

///////////////////////////////////////////////////////////////////////////////

GrTexture* GrContext::wrapBackendTexture(const GrBackendTextureDesc& desc) {
    return fGpu->wrapBackendTexture(desc);
}

GrRenderTarget* GrContext::wrapBackendRenderTarget(const GrBackendRenderTargetDesc& desc) {
    return fGpu->wrapBackendRenderTarget(desc);
}

////////////////////////////////////////////////////////////////////////////////

void GrContext::clear(const SkIRect* rect,
                      const GrColor color,
                      bool canIgnoreRect,
                      GrRenderTarget* renderTarget) {
    ASSERT_OWNED_RESOURCE(renderTarget);
    SkASSERT(renderTarget);

    AutoCheckFlush acf(this);
    GR_CREATE_TRACE_MARKER_CONTEXT("GrContext::clear", this);
    GrDrawTarget* target = this->prepareToDraw(NULL, NULL, &acf);
    if (NULL == target) {
        return;
    }
    target->clear(rect, color, canIgnoreRect, renderTarget);
}

void GrContext::drawPaint(const GrPaint& origPaint, const SkMatrix& viewMatrix) {
    // set rect to be big enough to fill the space, but not super-huge, so we
    // don't overflow fixed-point implementations
    SkRect r;
    r.setLTRB(0, 0,
              SkIntToScalar(getRenderTarget()->width()),
              SkIntToScalar(getRenderTarget()->height()));
    SkTCopyOnFirstWrite<GrPaint> paint(origPaint);

    // by definition this fills the entire clip, no need for AA
    if (paint->isAntiAlias()) {
        paint.writable()->setAntiAlias(false);
    }

    bool isPerspective = viewMatrix.hasPerspective();

    // We attempt to map r by the inverse matrix and draw that. mapRect will
    // map the four corners and bound them with a new rect. This will not
    // produce a correct result for some perspective matrices.
    if (!isPerspective) {
        SkMatrix inverse;
        if (!viewMatrix.invert(&inverse)) {
            SkDebugf("Could not invert matrix\n");
            return;
        }
        inverse.mapRect(&r);
        this->drawRect(*paint, viewMatrix, r);
    } else {
        SkMatrix localMatrix;
        if (!viewMatrix.invert(&localMatrix)) {
            SkDebugf("Could not invert matrix\n");
            return;
        }

        AutoCheckFlush acf(this);
        GrPipelineBuilder pipelineBuilder;
        GrDrawTarget* target = this->prepareToDraw(&pipelineBuilder, paint, &acf);
        if (NULL == target) {
            return;
        }

        GR_CREATE_TRACE_MARKER("GrContext::drawPaintWithPerspective", target);
        target->drawRect(&pipelineBuilder, paint->getColor(), SkMatrix::I(), r, NULL, &localMatrix);
    }
}

#ifdef SK_DEVELOPER
void GrContext::dumpFontCache() const {
    fFontCache->dump();
}
#endif

////////////////////////////////////////////////////////////////////////////////

/*  create a triangle strip that strokes the specified triangle. There are 8
 unique vertices, but we repreat the last 2 to close up. Alternatively we
 could use an indices array, and then only send 8 verts, but not sure that
 would be faster.
 */
static void setStrokeRectStrip(SkPoint verts[10], SkRect rect,
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

static inline bool is_irect(const SkRect& r) {
  return SkScalarIsInt(r.fLeft)  && SkScalarIsInt(r.fTop) &&
         SkScalarIsInt(r.fRight) && SkScalarIsInt(r.fBottom);
}

static bool apply_aa_to_rect(GrDrawTarget* target,
                             GrPipelineBuilder* pipelineBuilder,
                             SkRect* devBoundRect,
                             const SkRect& rect,
                             SkScalar strokeWidth,
                             const SkMatrix& combinedMatrix,
                             GrColor color) {
    if (!pipelineBuilder->canTweakAlphaForCoverage() &&
        !pipelineBuilder->canUseFracCoveragePrimProc(color, *target->caps())) {
#ifdef SK_DEBUG
        //SkDebugf("Turning off AA to correctly apply blend.\n");
#endif
        return false;
    }

    if (pipelineBuilder->getRenderTarget()->isMultisampled()) {
        return false;
    }

#if defined(SHADER_AA_FILL_RECT) || !defined(IGNORE_ROT_AA_RECT_OPT)
    if (strokeWidth >= 0) {
#endif
        if (!combinedMatrix.preservesAxisAlignment()) {
            return false;
        }

#if defined(SHADER_AA_FILL_RECT) || !defined(IGNORE_ROT_AA_RECT_OPT)
    } else {
        if (!combinedMatrix.preservesRightAngles()) {
            return false;
        }
    }
#endif

    combinedMatrix.mapRect(devBoundRect, rect);
    if (!combinedMatrix.rectStaysRect()) {
        return true;
    }

    if (strokeWidth < 0) {
        return !is_irect(*devBoundRect);
    }

    return true;
}

static inline bool rect_contains_inclusive(const SkRect& rect, const SkPoint& point) {
    return point.fX >= rect.fLeft && point.fX <= rect.fRight &&
           point.fY >= rect.fTop && point.fY <= rect.fBottom;
}

void GrContext::drawRect(const GrPaint& paint,
                         const SkMatrix& viewMatrix,
                         const SkRect& rect,
                         const GrStrokeInfo* strokeInfo) {
    if (strokeInfo && strokeInfo->isDashed()) {
        SkPath path;
        path.addRect(rect);
        this->drawPath(paint, viewMatrix, path, *strokeInfo);
        return;
    }

    AutoCheckFlush acf(this);
    GrPipelineBuilder pipelineBuilder;
    GrDrawTarget* target = this->prepareToDraw(&pipelineBuilder, &paint, &acf);
    if (NULL == target) {
        return;
    }

    GR_CREATE_TRACE_MARKER("GrContext::drawRect", target);
    SkScalar width = NULL == strokeInfo ? -1 : strokeInfo->getStrokeRec().getWidth();

    // Check if this is a full RT draw and can be replaced with a clear. We don't bother checking
    // cases where the RT is fully inside a stroke.
    if (width < 0) {
        SkRect rtRect;
        pipelineBuilder.getRenderTarget()->getBoundsRect(&rtRect);
        SkRect clipSpaceRTRect = rtRect;
        bool checkClip = false;
        if (this->getClip()) {
            checkClip = true;
            clipSpaceRTRect.offset(SkIntToScalar(this->getClip()->fOrigin.fX),
                                   SkIntToScalar(this->getClip()->fOrigin.fY));
        }
        // Does the clip contain the entire RT?
        if (!checkClip || target->getClip()->fClipStack->quickContains(clipSpaceRTRect)) {
            SkMatrix invM;
            if (!viewMatrix.invert(&invM)) {
                return;
            }
            // Does the rect bound the RT?
            SkPoint srcSpaceRTQuad[4];
            invM.mapRectToQuad(srcSpaceRTQuad, rtRect);
            if (rect_contains_inclusive(rect, srcSpaceRTQuad[0]) &&
                rect_contains_inclusive(rect, srcSpaceRTQuad[1]) &&
                rect_contains_inclusive(rect, srcSpaceRTQuad[2]) &&
                rect_contains_inclusive(rect, srcSpaceRTQuad[3])) {
                // Will it blend?
                GrColor clearColor;
                if (paint.isOpaqueAndConstantColor(&clearColor)) {
                    target->clear(NULL, clearColor, true, fRenderTarget);
                    return;
                }
            }
        }
    }

    GrColor color = paint.getColor();
    SkRect devBoundRect;
    bool needAA = paint.isAntiAlias() && !pipelineBuilder.getRenderTarget()->isMultisampled();
    bool doAA = needAA && apply_aa_to_rect(target, &pipelineBuilder, &devBoundRect, rect, width,
                                           viewMatrix, color);

    if (doAA) {
        if (width >= 0) {
            const SkStrokeRec& strokeRec = strokeInfo->getStrokeRec();
            fAARectRenderer->strokeAARect(target,
                                          &pipelineBuilder,
                                          color,
                                          viewMatrix,
                                          rect,
                                          devBoundRect,
                                          strokeRec);
        } else {
            // filled AA rect
            fAARectRenderer->fillAARect(target,
                                        &pipelineBuilder,
                                        color,
                                        viewMatrix,
                                        rect,
                                        devBoundRect);
        }
        return;
    }

    if (width >= 0) {
        // TODO: consider making static vertex buffers for these cases.
        // Hairline could be done by just adding closing vertex to
        // unitSquareVertexBuffer()

        static const int worstCaseVertCount = 10;
        SkAutoTUnref<const GrGeometryProcessor> gp(
                GrDefaultGeoProcFactory::Create(GrDefaultGeoProcFactory::kPosition_GPType,
                                                color,
                                                viewMatrix,
                                                SkMatrix::I()));
        GrDrawTarget::AutoReleaseGeometry geo(target,
                                              worstCaseVertCount,
                                              gp->getVertexStride(),
                                              0);
        SkASSERT(gp->getVertexStride() == sizeof(SkPoint));

        if (!geo.succeeded()) {
            SkDebugf("Failed to get space for vertices!\n");
            return;
        }

        GrPrimitiveType primType;
        int vertCount;
        SkPoint* vertex = geo.positions();

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

        target->drawNonIndexed(&pipelineBuilder, gp, primType, 0, vertCount);
    } else {
        // filled BW rect
        target->drawSimpleRect(&pipelineBuilder, color, viewMatrix, rect);
    }
}

void GrContext::drawNonAARectToRect(const GrPaint& paint,
                                    const SkMatrix& viewMatrix,
                                    const SkRect& rectToDraw,
                                    const SkRect& localRect,
                                    const SkMatrix* localMatrix) {
    AutoCheckFlush acf(this);
    GrPipelineBuilder pipelineBuilder;
    GrDrawTarget* target = this->prepareToDraw(&pipelineBuilder, &paint, &acf);
    if (NULL == target) {
        return;
    }

    GR_CREATE_TRACE_MARKER("GrContext::drawRectToRect", target);

    target->drawRect(&pipelineBuilder, paint.getColor(), viewMatrix, rectToDraw, &localRect,
                     localMatrix);
}

static const GrGeometryProcessor* set_vertex_attributes(const SkPoint* texCoords,
                                                        const GrColor* colors,
                                                        int* colorOffset,
                                                        int* texOffset,
                                                        GrColor color,
                                                        const SkMatrix& viewMatrix) {
    *texOffset = -1;
    *colorOffset = -1;
    uint32_t flags = GrDefaultGeoProcFactory::kPosition_GPType;
    if (texCoords && colors) {
        *colorOffset = sizeof(SkPoint);
        *texOffset = sizeof(SkPoint) + sizeof(GrColor);
        flags |= GrDefaultGeoProcFactory::kColor_GPType |
                 GrDefaultGeoProcFactory::kLocalCoord_GPType;
    } else if (texCoords) {
        *texOffset = sizeof(SkPoint);
        flags |= GrDefaultGeoProcFactory::kLocalCoord_GPType;
    } else if (colors) {
        *colorOffset = sizeof(SkPoint);
        flags |= GrDefaultGeoProcFactory::kColor_GPType;
    }
    return GrDefaultGeoProcFactory::Create(flags, color, viewMatrix, SkMatrix::I());
}

void GrContext::drawVertices(const GrPaint& paint,
                             const SkMatrix& viewMatrix,
                             GrPrimitiveType primitiveType,
                             int vertexCount,
                             const SkPoint positions[],
                             const SkPoint texCoords[],
                             const GrColor colors[],
                             const uint16_t indices[],
                             int indexCount) {
    AutoCheckFlush acf(this);
    GrPipelineBuilder pipelineBuilder;
    GrDrawTarget::AutoReleaseGeometry geo; // must be inside AutoCheckFlush scope

    GrDrawTarget* target = this->prepareToDraw(&pipelineBuilder, &paint, &acf);
    if (NULL == target) {
        return;
    }

    GR_CREATE_TRACE_MARKER("GrContext::drawVertices", target);

    int colorOffset = -1, texOffset = -1;
    SkAutoTUnref<const GrGeometryProcessor> gp(
            set_vertex_attributes(texCoords, colors, &colorOffset, &texOffset,
                                  paint.getColor(), viewMatrix));

    size_t vertexStride = gp->getVertexStride();
    SkASSERT(vertexStride == sizeof(SkPoint) + (SkToBool(texCoords) ? sizeof(SkPoint) : 0)
                                             + (SkToBool(colors) ? sizeof(GrColor) : 0));
    if (!geo.set(target, vertexCount, vertexStride, indexCount)) {
        SkDebugf("Failed to get space for vertices!\n");
        return;
    }
    void* curVertex = geo.vertices();

    for (int i = 0; i < vertexCount; ++i) {
        *((SkPoint*)curVertex) = positions[i];

        if (texOffset >= 0) {
            *(SkPoint*)((intptr_t)curVertex + texOffset) = texCoords[i];
        }
        if (colorOffset >= 0) {
            *(GrColor*)((intptr_t)curVertex + colorOffset) = colors[i];
        }
        curVertex = (void*)((intptr_t)curVertex + vertexStride);
    }

    // we don't currently apply offscreen AA to this path. Need improved
    // management of GrDrawTarget's geometry to avoid copying points per-tile.
    if (indices) {
        uint16_t* curIndex = (uint16_t*)geo.indices();
        for (int i = 0; i < indexCount; ++i) {
            curIndex[i] = indices[i];
        }
        target->drawIndexed(&pipelineBuilder, gp, primitiveType, 0, 0, vertexCount, indexCount);
    } else {
        target->drawNonIndexed(&pipelineBuilder, gp, primitiveType, 0, vertexCount);
    }
}

///////////////////////////////////////////////////////////////////////////////

void GrContext::drawRRect(const GrPaint& paint,
                          const SkMatrix& viewMatrix,
                          const SkRRect& rrect,
                          const GrStrokeInfo& strokeInfo) {
    if (rrect.isEmpty()) {
       return;
    }

    if (strokeInfo.isDashed()) {
        SkPath path;
        path.addRRect(rrect);
        this->drawPath(paint, viewMatrix, path, strokeInfo);
        return;
    }

    AutoCheckFlush acf(this);
    GrPipelineBuilder pipelineBuilder;
    GrDrawTarget* target = this->prepareToDraw(&pipelineBuilder, &paint, &acf);
    if (NULL == target) {
        return;
    }

    GR_CREATE_TRACE_MARKER("GrContext::drawRRect", target);

    const SkStrokeRec& strokeRec = strokeInfo.getStrokeRec();

    GrColor color = paint.getColor();
    if (!fOvalRenderer->drawRRect(target, &pipelineBuilder, color, viewMatrix, paint.isAntiAlias(),
                                  rrect, strokeRec)) {
        SkPath path;
        path.addRRect(rrect);
        this->internalDrawPath(target, &pipelineBuilder, viewMatrix, color, paint.isAntiAlias(),
                               path, strokeInfo);
    }
}

///////////////////////////////////////////////////////////////////////////////

void GrContext::drawDRRect(const GrPaint& paint,
                           const SkMatrix& viewMatrix,
                           const SkRRect& outer,
                           const SkRRect& inner) {
    if (outer.isEmpty()) {
       return;
    }

    AutoCheckFlush acf(this);
    GrPipelineBuilder pipelineBuilder;
    GrDrawTarget* target = this->prepareToDraw(&pipelineBuilder, &paint, &acf);

    GR_CREATE_TRACE_MARKER("GrContext::drawDRRect", target);

    GrColor color = paint.getColor();
    if (!fOvalRenderer->drawDRRect(target, &pipelineBuilder, color, viewMatrix, paint.isAntiAlias(),
                                   outer, inner)) {
        SkPath path;
        path.addRRect(inner);
        path.addRRect(outer);
        path.setFillType(SkPath::kEvenOdd_FillType);

        GrStrokeInfo fillRec(SkStrokeRec::kFill_InitStyle);
        this->internalDrawPath(target, &pipelineBuilder, viewMatrix, color, paint.isAntiAlias(),
                               path, fillRec);
    }
}

///////////////////////////////////////////////////////////////////////////////

void GrContext::drawOval(const GrPaint& paint,
                         const SkMatrix& viewMatrix,
                         const SkRect& oval,
                         const GrStrokeInfo& strokeInfo) {
    if (oval.isEmpty()) {
       return;
    }

    if (strokeInfo.isDashed()) {
        SkPath path;
        path.addOval(oval);
        this->drawPath(paint, viewMatrix, path, strokeInfo);
        return;
    }

    AutoCheckFlush acf(this);
    GrPipelineBuilder pipelineBuilder;
    GrDrawTarget* target = this->prepareToDraw(&pipelineBuilder, &paint, &acf);
    if (NULL == target) {
        return;
    }

    GR_CREATE_TRACE_MARKER("GrContext::drawOval", target);

    const SkStrokeRec& strokeRec = strokeInfo.getStrokeRec();

    GrColor color = paint.getColor();
    if (!fOvalRenderer->drawOval(target, &pipelineBuilder, color, viewMatrix, paint.isAntiAlias(),
                                 oval, strokeRec)) {
        SkPath path;
        path.addOval(oval);
        this->internalDrawPath(target, &pipelineBuilder, viewMatrix, color, paint.isAntiAlias(),
                               path, strokeInfo);
    }
}

// Can 'path' be drawn as a pair of filled nested rectangles?
static bool is_nested_rects(GrDrawTarget* target,
                            GrPipelineBuilder* pipelineBuilder,
                            GrColor color,
                            const SkMatrix& viewMatrix,
                            const SkPath& path,
                            const SkStrokeRec& stroke,
                            SkRect rects[2]) {
    SkASSERT(stroke.isFillStyle());

    if (path.isInverseFillType()) {
        return false;
    }

    // TODO: this restriction could be lifted if we were willing to apply
    // the matrix to all the points individually rather than just to the rect
    if (!viewMatrix.preservesAxisAlignment()) {
        return false;
    }

    if (!pipelineBuilder->canTweakAlphaForCoverage() &&
        !pipelineBuilder->canUseFracCoveragePrimProc(color, *target->caps())) {
        return false;
    }

    SkPath::Direction dirs[2];
    if (!path.isNestedRects(rects, dirs)) {
        return false;
    }

    if (SkPath::kWinding_FillType == path.getFillType() && dirs[0] == dirs[1]) {
        // The two rects need to be wound opposite to each other
        return false;
    }

    // Right now, nested rects where the margin is not the same width
    // all around do not render correctly
    const SkScalar* outer = rects[0].asScalars();
    const SkScalar* inner = rects[1].asScalars();

    bool allEq = true;

    SkScalar margin = SkScalarAbs(outer[0] - inner[0]);
    bool allGoE1 = margin >= SK_Scalar1;

    for (int i = 1; i < 4; ++i) {
        SkScalar temp = SkScalarAbs(outer[i] - inner[i]);
        if (temp < SK_Scalar1) {
            allGoE1 = false;
        }
        if (!SkScalarNearlyEqual(margin, temp)) {
            allEq = false;
        }
    }

    return allEq || allGoE1;
}

void GrContext::drawPath(const GrPaint& paint,
                         const SkMatrix& viewMatrix,
                         const SkPath& path,
                         const GrStrokeInfo& strokeInfo) {

    if (path.isEmpty()) {
       if (path.isInverseFillType()) {
           this->drawPaint(paint, viewMatrix);
       }
       return;
    }

    GrColor color = paint.getColor();
    if (strokeInfo.isDashed()) {
        SkPoint pts[2];
        if (path.isLine(pts)) {
            AutoCheckFlush acf(this);
            GrPipelineBuilder pipelineBuilder;
            GrDrawTarget* target = this->prepareToDraw(&pipelineBuilder, &paint, &acf);
            if (NULL == target) {
                return;
            }

            if (GrDashingEffect::DrawDashLine(fGpu, target, &pipelineBuilder, color, viewMatrix,
                                              pts, paint, strokeInfo)) {
                return;
            }
        }

        // Filter dashed path into new path with the dashing applied
        const SkPathEffect::DashInfo& info = strokeInfo.getDashInfo();
        SkTLazy<SkPath> effectPath;
        GrStrokeInfo newStrokeInfo(strokeInfo, false);
        SkStrokeRec* stroke = newStrokeInfo.getStrokeRecPtr();
        if (SkDashPath::FilterDashPath(effectPath.init(), path, stroke, NULL, info)) {
            this->drawPath(paint, viewMatrix, *effectPath.get(), newStrokeInfo);
            return;
        }

        this->drawPath(paint, viewMatrix, path, newStrokeInfo);
        return;
    }

    // Note that internalDrawPath may sw-rasterize the path into a scratch texture.
    // Scratch textures can be recycled after they are returned to the texture
    // cache. This presents a potential hazard for buffered drawing. However,
    // the writePixels that uploads to the scratch will perform a flush so we're
    // OK.
    AutoCheckFlush acf(this);
    GrPipelineBuilder pipelineBuilder;
    GrDrawTarget* target = this->prepareToDraw(&pipelineBuilder, &paint, &acf);
    if (NULL == target) {
        return;
    }

    GR_CREATE_TRACE_MARKER1("GrContext::drawPath", target, "Is Convex", path.isConvex());

    const SkStrokeRec& strokeRec = strokeInfo.getStrokeRec();

    bool useCoverageAA = paint.isAntiAlias() &&
        !pipelineBuilder.getRenderTarget()->isMultisampled();

    if (useCoverageAA && strokeRec.getWidth() < 0 && !path.isConvex()) {
        // Concave AA paths are expensive - try to avoid them for special cases
        SkRect rects[2];

        if (is_nested_rects(target, &pipelineBuilder, color, viewMatrix, path, strokeRec, rects)) {
            fAARectRenderer->fillAANestedRects(target, &pipelineBuilder, color, viewMatrix,rects);
            return;
        }
    }

    SkRect ovalRect;
    bool isOval = path.isOval(&ovalRect);

    if (!isOval || path.isInverseFillType() ||
        !fOvalRenderer->drawOval(target, &pipelineBuilder, color, viewMatrix, paint.isAntiAlias(),
                                 ovalRect, strokeRec)) {
        this->internalDrawPath(target, &pipelineBuilder, viewMatrix, color, paint.isAntiAlias(),
                               path, strokeInfo);
    }
}

void GrContext::internalDrawPath(GrDrawTarget* target,
                                 GrPipelineBuilder* pipelineBuilder,
                                 const SkMatrix& viewMatrix,
                                 GrColor color,
                                 bool useAA,
                                 const SkPath& path,
                                 const GrStrokeInfo& strokeInfo) {
    SkASSERT(!path.isEmpty());

    GR_CREATE_TRACE_MARKER("GrContext::internalDrawPath", target);


    // An Assumption here is that path renderer would use some form of tweaking
    // the src color (either the input alpha or in the frag shader) to implement
    // aa. If we have some future driver-mojo path AA that can do the right
    // thing WRT to the blend then we'll need some query on the PR.
    bool useCoverageAA = useAA &&
        !pipelineBuilder->getRenderTarget()->isMultisampled() &&
        pipelineBuilder->canUseFracCoveragePrimProc(color, *target->caps());


    GrPathRendererChain::DrawType type =
        useCoverageAA ? GrPathRendererChain::kColorAntiAlias_DrawType :
                        GrPathRendererChain::kColor_DrawType;

    const SkPath* pathPtr = &path;
    SkTLazy<SkPath> tmpPath;
    SkTCopyOnFirstWrite<SkStrokeRec> stroke(strokeInfo.getStrokeRec());

    // Try a 1st time without stroking the path and without allowing the SW renderer
    GrPathRenderer* pr = this->getPathRenderer(target, pipelineBuilder, viewMatrix, *pathPtr,
                                               *stroke, false, type);

    if (NULL == pr) {
        if (!GrPathRenderer::IsStrokeHairlineOrEquivalent(*stroke, viewMatrix, NULL)) {
            // It didn't work the 1st time, so try again with the stroked path
            if (stroke->applyToPath(tmpPath.init(), *pathPtr)) {
                pathPtr = tmpPath.get();
                stroke.writable()->setFillStyle();
                if (pathPtr->isEmpty()) {
                    return;
                }
            }
        }

        // This time, allow SW renderer
        pr = this->getPathRenderer(target, pipelineBuilder, viewMatrix, *pathPtr, *stroke, true,
                                   type);
    }

    if (NULL == pr) {
#ifdef SK_DEBUG
        SkDebugf("Unable to find path renderer compatible with path.\n");
#endif
        return;
    }

    pr->drawPath(target, pipelineBuilder, color, viewMatrix, *pathPtr, *stroke, useCoverageAA);
}

////////////////////////////////////////////////////////////////////////////////

void GrContext::flush(int flagsBitfield) {
    if (NULL == fDrawBuffer) {
        return;
    }

    if (kDiscard_FlushBit & flagsBitfield) {
        fDrawBuffer->reset();
    } else {
        fDrawBuffer->flush();
    }
    fFlushToReduceCacheSize = false;
}

bool sw_convert_to_premul(GrPixelConfig srcConfig, int width, int height, size_t inRowBytes,
                          const void* inPixels, size_t outRowBytes, void* outPixels) {
    SkSrcPixelInfo srcPI;
    if (!GrPixelConfig2ColorAndProfileType(srcConfig, &srcPI.fColorType, NULL)) {
        return false;
    }
    srcPI.fAlphaType = kUnpremul_SkAlphaType;
    srcPI.fPixels = inPixels;
    srcPI.fRowBytes = inRowBytes;

    SkDstPixelInfo dstPI;
    dstPI.fColorType = srcPI.fColorType;
    dstPI.fAlphaType = kPremul_SkAlphaType;
    dstPI.fPixels = outPixels;
    dstPI.fRowBytes = outRowBytes;

    return srcPI.convertPixelsTo(&dstPI, width, height);
}

bool GrContext::writeSurfacePixels(GrSurface* surface,
                                   int left, int top, int width, int height,
                                   GrPixelConfig srcConfig, const void* buffer, size_t rowBytes,
                                   uint32_t pixelOpsFlags) {

    {
        GrTexture* texture = NULL;
        if (!(kUnpremul_PixelOpsFlag & pixelOpsFlags) && (texture = surface->asTexture()) &&
            fGpu->canWriteTexturePixels(texture, srcConfig)) {

            if (!(kDontFlush_PixelOpsFlag & pixelOpsFlags) &&
                surface->surfacePriv().hasPendingIO()) {
                this->flush();
            }
            return fGpu->writeTexturePixels(texture, left, top, width, height,
                                            srcConfig, buffer, rowBytes);
            // Don't need to check kFlushWrites_PixelOp here, we just did a direct write so the
            // upload is already flushed.
        }
    }

    // If we didn't do a direct texture write then we upload the pixels to a texture and draw.
    GrRenderTarget* renderTarget = surface->asRenderTarget();
    if (NULL == renderTarget) {
        return false;
    }

    // We ignore the preferred config unless it is a R/B swap of the src config. In that case
    // we will upload the original src data to a scratch texture but we will spoof it as the swapped
    // config. This scratch will then have R and B swapped. We correct for this by swapping again
    // when drawing the scratch to the dst using a conversion effect.
    bool swapRAndB = false;
    GrPixelConfig writeConfig = srcConfig;
    if (GrPixelConfigSwapRAndB(srcConfig) ==
        fGpu->preferredWritePixelsConfig(srcConfig, renderTarget->config())) {
        writeConfig = GrPixelConfigSwapRAndB(srcConfig);
        swapRAndB = true;
    }

    GrSurfaceDesc desc;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = writeConfig;
    SkAutoTUnref<GrTexture> texture(this->refScratchTexture(desc, kApprox_ScratchTexMatch));
    if (!texture) {
        return false;
    }

    SkAutoTUnref<const GrFragmentProcessor> fp;
    SkMatrix textureMatrix;
    textureMatrix.setIDiv(texture->width(), texture->height());

    // allocate a tmp buffer and sw convert the pixels to premul
    SkAutoSTMalloc<128 * 128, uint32_t> tmpPixels(0);

    if (kUnpremul_PixelOpsFlag & pixelOpsFlags) {
        if (!GrPixelConfigIs8888(srcConfig)) {
            return false;
        }
        fp.reset(this->createUPMToPMEffect(texture, swapRAndB, textureMatrix));
        // handle the unpremul step on the CPU if we couldn't create an effect to do it.
        if (NULL == fp) {
            size_t tmpRowBytes = 4 * width;
            tmpPixels.reset(width * height);
            if (!sw_convert_to_premul(srcConfig, width, height, rowBytes, buffer, tmpRowBytes,
                                      tmpPixels.get())) {
                return false;
            }
            rowBytes = tmpRowBytes;
            buffer = tmpPixels.get();
        }
    }
    if (NULL == fp) {
        fp.reset(GrConfigConversionEffect::Create(texture,
                                                  swapRAndB,
                                                  GrConfigConversionEffect::kNone_PMConversion,
                                                  textureMatrix));
    }

    // Even if the client told us not to flush, we still flush here. The client may have known that
    // writes to the original surface caused no data hazards, but they can't know that the scratch
    // we just got is safe.
    if (texture->surfacePriv().hasPendingIO()) {
        this->flush();
    }
    if (!fGpu->writeTexturePixels(texture, 0, 0, width, height,
                                  writeConfig, buffer, rowBytes)) {
        return false;
    }

    SkMatrix matrix;
    matrix.setTranslate(SkIntToScalar(left), SkIntToScalar(top));

    // This function can be called in the midst of drawing another object (e.g., when uploading a
    // SW-rasterized clip while issuing a draw). So we push the current geometry state before
    // drawing a rect to the render target.
    // The bracket ensures we pop the stack if we wind up flushing below.
    {
        GrDrawTarget* drawTarget = this->prepareToDraw(NULL, NULL, NULL);
        GrDrawTarget::AutoGeometryPush agp(drawTarget);

        GrPipelineBuilder pipelineBuilder;
        pipelineBuilder.addColorProcessor(fp);
        pipelineBuilder.setRenderTarget(renderTarget);
        drawTarget->drawSimpleRect(&pipelineBuilder, GrColor_WHITE, matrix,
                                   SkRect::MakeWH(SkIntToScalar(width), SkIntToScalar(height)));
    }

    if (kFlushWrites_PixelOp & pixelOpsFlags) {
        this->flushSurfaceWrites(surface);
    }

    return true;
}

// toggles between RGBA and BGRA
static SkColorType toggle_colortype32(SkColorType ct) {
    if (kRGBA_8888_SkColorType == ct) {
        return kBGRA_8888_SkColorType;
    } else {
        SkASSERT(kBGRA_8888_SkColorType == ct);
        return kRGBA_8888_SkColorType;
    }
}

bool GrContext::readRenderTargetPixels(GrRenderTarget* target,
                                       int left, int top, int width, int height,
                                       GrPixelConfig dstConfig, void* buffer, size_t rowBytes,
                                       uint32_t flags) {
    ASSERT_OWNED_RESOURCE(target);
    SkASSERT(target);

    if (!(kDontFlush_PixelOpsFlag & flags) && target->surfacePriv().hasPendingWrite()) {
        this->flush();
    }

    // Determine which conversions have to be applied: flipY, swapRAnd, and/or unpremul.

    // If fGpu->readPixels would incur a y-flip cost then we will read the pixels upside down. We'll
    // either do the flipY by drawing into a scratch with a matrix or on the cpu after the read.
    bool flipY = fGpu->readPixelsWillPayForYFlip(target, left, top,
                                                 width, height, dstConfig,
                                                 rowBytes);
    // We ignore the preferred config if it is different than our config unless it is an R/B swap.
    // In that case we'll perform an R and B swap while drawing to a scratch texture of the swapped
    // config. Then we will call readPixels on the scratch with the swapped config. The swaps during
    // the draw cancels out the fact that we call readPixels with a config that is R/B swapped from
    // dstConfig.
    GrPixelConfig readConfig = dstConfig;
    bool swapRAndB = false;
    if (GrPixelConfigSwapRAndB(dstConfig) ==
        fGpu->preferredReadPixelsConfig(dstConfig, target->config())) {
        readConfig = GrPixelConfigSwapRAndB(readConfig);
        swapRAndB = true;
    }

    bool unpremul = SkToBool(kUnpremul_PixelOpsFlag & flags);

    if (unpremul && !GrPixelConfigIs8888(dstConfig)) {
        // The unpremul flag is only allowed for these two configs.
        return false;
    }

    SkAutoTUnref<GrTexture> tempTexture;

    // If the src is a texture and we would have to do conversions after read pixels, we instead
    // do the conversions by drawing the src to a scratch texture. If we handle any of the
    // conversions in the draw we set the corresponding bool to false so that we don't reapply it
    // on the read back pixels.
    GrTexture* src = target->asTexture();
    if (src && (swapRAndB || unpremul || flipY)) {
        // Make the scratch a render so we can read its pixels.
        GrSurfaceDesc desc;
        desc.fFlags = kRenderTarget_GrSurfaceFlag;
        desc.fWidth = width;
        desc.fHeight = height;
        desc.fConfig = readConfig;
        desc.fOrigin = kTopLeft_GrSurfaceOrigin;

        // When a full read back is faster than a partial we could always make the scratch exactly
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
        tempTexture.reset(this->refScratchTexture(desc, match));
        if (tempTexture) {
            // compute a matrix to perform the draw
            SkMatrix textureMatrix;
            textureMatrix.setTranslate(SK_Scalar1 *left, SK_Scalar1 *top);
            textureMatrix.postIDiv(src->width(), src->height());

            SkAutoTUnref<const GrFragmentProcessor> fp;
            if (unpremul) {
                fp.reset(this->createPMToUPMEffect(src, swapRAndB, textureMatrix));
                if (fp) {
                    unpremul = false; // we no longer need to do this on CPU after the read back.
                }
            }
            // If we failed to create a PM->UPM effect and have no other conversions to perform then
            // there is no longer any point to using the scratch.
            if (fp || flipY || swapRAndB) {
                if (!fp) {
                    fp.reset(GrConfigConversionEffect::Create(
                            src, swapRAndB, GrConfigConversionEffect::kNone_PMConversion,
                            textureMatrix));
                }
                swapRAndB = false; // we will handle the swap in the draw.

                // We protect the existing geometry here since it may not be
                // clear to the caller that a draw operation (i.e., drawSimpleRect)
                // can be invoked in this method
                {
                    GrDrawTarget::AutoGeometryPush agp(fDrawBuffer);
                    GrPipelineBuilder pipelineBuilder;
                    SkASSERT(fp);
                    pipelineBuilder.addColorProcessor(fp);

                    pipelineBuilder.setRenderTarget(tempTexture->asRenderTarget());
                    SkRect rect = SkRect::MakeWH(SkIntToScalar(width), SkIntToScalar(height));
                    fDrawBuffer->drawSimpleRect(&pipelineBuilder, GrColor_WHITE, SkMatrix::I(),
                                                rect);
                    // we want to read back from the scratch's origin
                    left = 0;
                    top = 0;
                    target = tempTexture->asRenderTarget();
                }
                this->flushSurfaceWrites(target);
            }
        }
    }

    if (!fGpu->readPixels(target,
                          left, top, width, height,
                          readConfig, buffer, rowBytes)) {
        return false;
    }
    // Perform any conversions we weren't able to perform using a scratch texture.
    if (unpremul || swapRAndB) {
        SkDstPixelInfo dstPI;
        if (!GrPixelConfig2ColorAndProfileType(dstConfig, &dstPI.fColorType, NULL)) {
            return false;
        }
        dstPI.fAlphaType = kUnpremul_SkAlphaType;
        dstPI.fPixels = buffer;
        dstPI.fRowBytes = rowBytes;

        SkSrcPixelInfo srcPI;
        srcPI.fColorType = swapRAndB ? toggle_colortype32(dstPI.fColorType) : dstPI.fColorType;
        srcPI.fAlphaType = kPremul_SkAlphaType;
        srcPI.fPixels = buffer;
        srcPI.fRowBytes = rowBytes;

        return srcPI.convertPixelsTo(&dstPI, width, height);
    }
    return true;
}

void GrContext::prepareSurfaceForExternalRead(GrSurface* surface) {
    SkASSERT(surface);
    ASSERT_OWNED_RESOURCE(surface);
    if (surface->surfacePriv().hasPendingIO()) {
        this->flush();
    }
    GrRenderTarget* rt = surface->asRenderTarget();
    if (fGpu && rt) {
        fGpu->resolveRenderTarget(rt);
    }
}

void GrContext::discardRenderTarget(GrRenderTarget* renderTarget) {
    SkASSERT(renderTarget);
    ASSERT_OWNED_RESOURCE(renderTarget);
    AutoCheckFlush acf(this);
    GrDrawTarget* target = this->prepareToDraw(NULL, NULL, &acf);
    if (NULL == target) {
        return;
    }
    target->discard(renderTarget);
}

void GrContext::copySurface(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                            const SkIPoint& dstPoint, uint32_t pixelOpsFlags) {
    if (NULL == src || NULL == dst) {
        return;
    }
    ASSERT_OWNED_RESOURCE(src);
    ASSERT_OWNED_RESOURCE(dst);

    // Since we're going to the draw target and not GPU, no need to check kNoFlush
    // here.

    GrDrawTarget* target = this->prepareToDraw(NULL, NULL, NULL);
    if (NULL == target) {
        return;
    }
    target->copySurface(dst, src, srcRect, dstPoint);

    if (kFlushWrites_PixelOp & pixelOpsFlags) {
        this->flush();
    }
}

void GrContext::flushSurfaceWrites(GrSurface* surface) {
    if (surface->surfacePriv().hasPendingWrite()) {
        this->flush();
    }
}

GrDrawTarget* GrContext::prepareToDraw(GrPipelineBuilder* pipelineBuilder,
                                       const GrPaint* paint,
                                       const AutoCheckFlush* acf) {
    if (NULL == fGpu) {
        return NULL;
    }

    ASSERT_OWNED_RESOURCE(fRenderTarget.get());
    if (pipelineBuilder) {
        SkASSERT(paint && acf);
        pipelineBuilder->setFromPaint(*paint, fRenderTarget.get());
        pipelineBuilder->setState(GrPipelineBuilder::kClip_StateBit,
                                  fClip && !fClip->fClipStack->isWideOpen());
    }
    fDrawBuffer->setClip(fClip);
    return fDrawBuffer;
}

/*
 * This method finds a path renderer that can draw the specified path on
 * the provided target.
 * Due to its expense, the software path renderer has split out so it can
 * can be individually allowed/disallowed via the "allowSW" boolean.
 */
GrPathRenderer* GrContext::getPathRenderer(const GrDrawTarget* target,
                                           const GrPipelineBuilder* pipelineBuilder,
                                           const SkMatrix& viewMatrix,
                                           const SkPath& path,
                                           const SkStrokeRec& stroke,
                                           bool allowSW,
                                           GrPathRendererChain::DrawType drawType,
                                           GrPathRendererChain::StencilSupport* stencilSupport) {

    if (NULL == fPathRendererChain) {
        fPathRendererChain = SkNEW_ARGS(GrPathRendererChain, (this));
    }

    GrPathRenderer* pr = fPathRendererChain->getPathRenderer(target,
                                                             pipelineBuilder,
                                                             viewMatrix,
                                                             path,
                                                             stroke,
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
bool GrContext::isConfigRenderable(GrPixelConfig config, bool withMSAA) const {
    return fGpu->caps()->isConfigRenderable(config, withMSAA);
}

int GrContext::getRecommendedSampleCount(GrPixelConfig config,
                                         SkScalar dpi) const {
    if (!this->isConfigRenderable(config, true)) {
        return 0;
    }
    int chosenSampleCount = 0;
    if (fGpu->caps()->pathRenderingSupport()) {
        if (dpi >= 250.0f) {
            chosenSampleCount = 4;
        } else {
            chosenSampleCount = 16;
        }
    }
    return chosenSampleCount <= fGpu->caps()->maxSampleCount() ?
        chosenSampleCount : 0;
}

void GrContext::setupDrawBuffer() {
    SkASSERT(NULL == fDrawBuffer);
    SkASSERT(NULL == fDrawBufferVBAllocPool);
    SkASSERT(NULL == fDrawBufferIBAllocPool);

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
}

GrDrawTarget* GrContext::getTextTarget() {
    return this->prepareToDraw(NULL, NULL, NULL);
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

const GrFragmentProcessor* GrContext::createPMToUPMEffect(GrTexture* texture,
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

const GrFragmentProcessor* GrContext::createUPMToPMEffect(GrTexture* texture,
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

//////////////////////////////////////////////////////////////////////////////

void GrContext::getResourceCacheLimits(int* maxTextures, size_t* maxTextureBytes) const {
    if (maxTextures) {
        *maxTextures = fResourceCache->getMaxResourceCount();
    }
    if (maxTextureBytes) {
        *maxTextureBytes = fResourceCache->getMaxResourceBytes();
    }
}

void GrContext::setResourceCacheLimits(int maxTextures, size_t maxTextureBytes) {
    fResourceCache->setLimits(maxTextures, maxTextureBytes);
}

bool GrContext::addResourceToCache(const GrContentKey& key, GrGpuResource* resource) {
    ASSERT_OWNED_RESOURCE(resource);
    if (!resource || resource->wasDestroyed()) {
        return false;
    }
    return resource->resourcePriv().setContentKey(key);
}

bool GrContext::isResourceInCache(const GrContentKey& key) const {
    return fResourceCache->hasContentKey(key);
}

GrGpuResource* GrContext::findAndRefCachedResource(const GrContentKey& key) {
    return fResourceCache->findAndRefContentResource(key);
}

//////////////////////////////////////////////////////////////////////////////

void GrContext::addGpuTraceMarker(const GrGpuTraceMarker* marker) {
    fGpu->addGpuTraceMarker(marker);
    if (fDrawBuffer) {
        fDrawBuffer->addGpuTraceMarker(marker);
    }
}

void GrContext::removeGpuTraceMarker(const GrGpuTraceMarker* marker) {
    fGpu->removeGpuTraceMarker(marker);
    if (fDrawBuffer) {
        fDrawBuffer->removeGpuTraceMarker(marker);
    }
}

