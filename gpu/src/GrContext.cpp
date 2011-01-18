/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "GrContext.h"
#include "GrTextureCache.h"
#include "GrTextStrike.h"
#include "GrMemory.h"
#include "GrPathIter.h"
#include "GrClipIterator.h"
#include "GrIndexBuffer.h"

#define DEFER_TEXT_RENDERING 1

static const size_t MAX_TEXTURE_CACHE_COUNT = 128;
static const size_t MAX_TEXTURE_CACHE_BYTES = 8 * 1024 * 1024;

#if DEFER_TEXT_RENDERING
    static const uint32_t POOL_VB_SIZE = 2048 *
            GrDrawTarget::VertexSize(
                GrDrawTarget::kTextFormat_VertexLayoutBit |
                GrDrawTarget::StageTexCoordVertexLayoutBit(0,0));
    static const uint32_t NUM_POOL_VBS = 8;
#else
    static const uint32_t POOL_VB_SIZE = 0;
    static const uint32_t NUM_POOL_VBS = 0;

#endif

GrContext* GrContext::Create(GrGpu::Engine engine,
                             GrGpu::Platform3DContext context3D) {
    GrContext* ctx = NULL;
    GrGpu* fGpu = GrGpu::Create(engine, context3D);
    if (NULL != fGpu) {
        ctx = new GrContext(fGpu);
        fGpu->unref();
    }
    return ctx;
}

GrContext* GrContext::CreateGLShaderContext() {
    return GrContext::Create(GrGpu::kOpenGL_Shaders_Engine, NULL);
}

GrContext::~GrContext() {
    fGpu->unref();
    delete fTextureCache;
    delete fFontCache;
}

void GrContext::abandonAllTextures() {
    fTextureCache->deleteAll(GrTextureCache::kAbandonTexture_DeleteMode);
    fFontCache->abandonAll();
}

GrTextureEntry* GrContext::findAndLockTexture(GrTextureKey* key,
                                              const GrSamplerState& sampler) {
    finalizeTextureKey(key, sampler);
    return fTextureCache->findAndLock(*key);
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

GrTextureEntry* GrContext::createAndLockTexture(GrTextureKey* key,
                                                const GrSamplerState& sampler,
                                                const GrGpu::TextureDesc& desc,
                                                void* srcData, size_t rowBytes) {
    GrAssert(key->width() == desc.fWidth);
    GrAssert(key->height() == desc.fHeight);

#if GR_DUMP_TEXTURE_UPLOAD
    GrPrintf("GrContext::createAndLockTexture [%d %d]\n", desc.fWidth, desc.fHeight);
#endif

    GrTextureEntry* entry = NULL;
    bool special = finalizeTextureKey(key, sampler);
    if (special) {
        GrTextureEntry* clampEntry;
        GrTextureKey clampKey(*key);
        clampEntry = findAndLockTexture(&clampKey, GrSamplerState::ClampNoFilter());

        if (NULL == clampEntry) {
            clampEntry = createAndLockTexture(&clampKey,
                                              GrSamplerState::ClampNoFilter(),
                                              desc, srcData, rowBytes);
            GrAssert(NULL != clampEntry);
            if (NULL == clampEntry) {
                return NULL;
            }
        }
        GrTexture* clampTexture = clampEntry->texture();
        GrGpu::TextureDesc rtDesc = desc;
        rtDesc.fFlags |= GrGpu::kRenderTarget_TextureFlag |
                         GrGpu::kNoPathRendering_TextureFlag;
        rtDesc.fWidth  = GrNextPow2(GrMax<int>(desc.fWidth,
                                               fGpu->minRenderTargetWidth()));
        rtDesc.fHeight = GrNextPow2(GrMax<int>(desc.fHeight,
                                               fGpu->minRenderTargetHeight()));

        GrTexture* texture = fGpu->createTexture(rtDesc, NULL, 0);

        if (NULL != texture) {
            GrGpu::AutoStateRestore asr(fGpu);
            fGpu->setRenderTarget(texture->asRenderTarget());
            fGpu->setTexture(0, clampEntry->texture());
            fGpu->setStencilPass(GrGpu::kNone_StencilPass);
            fGpu->setTextureMatrix(0, GrMatrix::I());
            fGpu->setViewMatrix(GrMatrix::I());
            fGpu->setAlpha(0xff);
            fGpu->setBlendFunc(GrGpu::kOne_BlendCoeff, GrGpu::kZero_BlendCoeff);
            fGpu->disableState(GrGpu::kDither_StateBit |
                               GrGpu::kClip_StateBit   |
                               GrGpu::kAntialias_StateBit);
            GrSamplerState stretchSampler(GrSamplerState::kClamp_WrapMode,
                                          GrSamplerState::kClamp_WrapMode,
                                          sampler.isFilter());
            fGpu->setSamplerState(0, stretchSampler);

            static const GrVertexLayout layout =
                                GrDrawTarget::StageTexCoordVertexLayoutBit(0,0);
            GrDrawTarget::AutoReleaseGeometry arg(fGpu, layout, 4, 0);

            if (arg.succeeded()) {
                GrPoint* verts = (GrPoint*) arg.vertices();
                verts[0].setIRectFan(0, 0,
                                     texture->contentWidth(),
                                     texture->contentHeight(),
                                     2*sizeof(GrPoint));
                GrScalar tw = GrFixedToScalar(GR_Fixed1 *
                                              clampTexture->contentWidth() /
                                              clampTexture->allocWidth());
                GrScalar th = GrFixedToScalar(GR_Fixed1 *
                                              clampTexture->contentHeight() /
                                              clampTexture->allocHeight());
                verts[1].setRectFan(0, 0, tw, th, 2*sizeof(GrPoint));
                fGpu->drawNonIndexed(GrGpu::kTriangleFan_PrimitiveType,
                                     0, 4);
                entry = fTextureCache->createAndLock(*key, texture);
            }
            texture->removeRenderTarget();
        } else {
            // TODO: Our CPU stretch doesn't filter. But we create separate
            // stretched textures when the sampler state is either filtered or
            // not. Either implement filtered stretch blit on CPU or just create
            // one when FBO case fails.

            rtDesc.fFlags = 0;
            // no longer need to clamp at min RT size.
            rtDesc.fWidth  = GrNextPow2(desc.fWidth);
            rtDesc.fHeight = GrNextPow2(desc.fHeight);
            int bpp = GrTexture::BytesPerPixel(desc.fFormat);
            GrAutoSMalloc<128*128*4> stretchedPixels(bpp *
                                                     rtDesc.fWidth *
                                                     rtDesc.fHeight);
            stretchImage(stretchedPixels.get(), rtDesc.fWidth, rtDesc.fHeight,
                         srcData, desc.fWidth, desc.fHeight, bpp);

            size_t stretchedRowBytes = rtDesc.fWidth * bpp;

            GrTexture* texture = fGpu->createTexture(rtDesc,
                                                     stretchedPixels.get(),
                                                     stretchedRowBytes);
            GrAssert(NULL != texture);
            entry = fTextureCache->createAndLock(*key, texture);
        }
        fTextureCache->unlock(clampEntry);

    } else {
        GrTexture* texture = fGpu->createTexture(desc, srcData, rowBytes);
        if (NULL != texture) {
            entry = fTextureCache->createAndLock(*key, texture);
        } else {
            entry = NULL;
        }
    }
    return entry;
}

void GrContext::unlockTexture(GrTextureEntry* entry) {
    fTextureCache->unlock(entry);
}

void GrContext::detachCachedTexture(GrTextureEntry* entry) {
    fTextureCache->detach(entry);
}

void GrContext::reattachAndUnlockCachedTexture(GrTextureEntry* entry) {
    fTextureCache->reattachAndUnlock(entry);
}

GrTexture* GrContext::createUncachedTexture(const GrGpu::TextureDesc& desc,
                                            void* srcData,
                                            size_t rowBytes) {
    return fGpu->createTexture(desc, srcData, rowBytes);
}

GrRenderTarget* GrContext::createPlatformRenderTarget(intptr_t platformRenderTarget,
                                                      int width, int height) {
    return fGpu->createPlatformRenderTarget(platformRenderTarget,
                                            width, height);
}

bool GrContext::supportsIndex8PixelConfig(const GrSamplerState& sampler,
                                          int width, int height) {
    if (!fGpu->supports8BitPalette()) {
        return false;
    }

    bool needsRepeat = sampler.getWrapX() != GrSamplerState::kClamp_WrapMode ||
                       sampler.getWrapY() != GrSamplerState::kClamp_WrapMode;
    bool isPow2 = GrIsPow2(width) && GrIsPow2(height);

    switch (fGpu->npotTextureSupport()) {
        case GrGpu::kNone_NPOTTextureType:
            return isPow2;
        case GrGpu::kNoRepeat_NPOTTextureType:
            return isPow2 || !needsRepeat;
        case GrGpu::kNonRendertarget_NPOTTextureType:
        case GrGpu::kFull_NPOTTextureType:
            return true;
    }
    // should never get here
    GrAssert(!"Bad enum from fGpu->npotTextureSupport");
    return false;
}

////////////////////////////////////////////////////////////////////////////////

void GrContext::eraseColor(GrColor color) {
    fGpu->eraseColor(color);
}

void GrContext::drawFull(bool useTexture) {
    // set rect to be big enough to fill the space, but not super-huge, so we
    // don't overflow fixed-point implementations
    GrRect r(fGpu->getClip().getBounds());
    GrMatrix inverse;
    if (fGpu->getViewInverse(&inverse)) {
        inverse.mapRect(&r);
    } else {
        GrPrintf("---- fGpu->getViewInverse failed\n");
    }

    this->fillRect(r, useTexture);
}

/*  create a triangle strip that strokes the specified triangle. There are 8
 unique vertices, but we repreat the last 2 to close up. Alternatively we
 could use an indices array, and then only send 8 verts, but not sure that
 would be faster.
 */
static void setStrokeRectStrip(GrPoint verts[10], const GrRect& rect,
                               GrScalar width) {
    const GrScalar rad = GrScalarHalf(width);

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

void GrContext::drawRect(const GrRect& rect, bool useTexture, GrScalar width) {
    GrVertexLayout layout = useTexture ?
                            GrDrawTarget::StagePosAsTexCoordVertexLayoutBit(0) :
                            0;

    static const int worstCaseVertCount = 10;
    GrDrawTarget::AutoReleaseGeometry geo(fGpu, layout, worstCaseVertCount, 0);
    if (!geo.succeeded()) {
        return;
    }

    this->flushText();

    int vertCount;
    GrGpu::PrimitiveType primType;
    GrPoint* vertex = geo.positions();

    if (width >= 0) {
        if (width > 0) {
            vertCount = 10;
            primType = GrGpu::kTriangleStrip_PrimitiveType;
            setStrokeRectStrip(vertex, rect, width);
        } else {
            // hairline
            vertCount = 5;
            primType = GrGpu::kLineStrip_PrimitiveType;
            vertex[0].set(rect.fLeft, rect.fTop);
            vertex[1].set(rect.fRight, rect.fTop);
            vertex[2].set(rect.fRight, rect.fBottom);
            vertex[3].set(rect.fLeft, rect.fBottom);
            vertex[4].set(rect.fLeft, rect.fTop);
        }
    } else {
        vertCount = 4;
        primType = GrGpu::kTriangleFan_PrimitiveType;
        vertex->setRectFan(rect.fLeft, rect.fTop, rect.fRight, rect.fBottom);
    }

    fGpu->drawNonIndexed(primType, 0, vertCount);
}

////////////////////////////////////////////////////////////////////////////////

#define NEW_EVAL        1   // Use adaptive path tesselation
#define STENCIL_OFF     0   // Always disable stencil (even when needed)
#define CPU_TRANSFORM   0   // Transform path verts on CPU

#if NEW_EVAL

#define EVAL_TOL GR_Scalar1

static uint32_t quadratic_point_count(const GrPoint points[], GrScalar tol) {
    GrScalar d = points[1].distanceToLineSegmentBetween(points[0], points[2]);
    // TODO: fixed points sqrt
    if (d < tol) {
        return 1;
    } else {
        // Each time we subdivide, d should be cut in 4. So we need to
        // subdivide x = log4(d/tol) times. x subdivisions creates 2^(x)
        // points.
        // 2^(log4(x)) = sqrt(x);
        d = ceilf(sqrtf(d/tol));
        return GrNextPow2((uint32_t)d);
    }
}

static uint32_t generate_quadratic_points(const GrPoint& p0,
                                          const GrPoint& p1,
                                          const GrPoint& p2,
                                          GrScalar tolSqd,
                                          GrPoint** points,
                                          uint32_t pointsLeft) {
    if (pointsLeft < 2 ||
        (p1.distanceToLineSegmentBetweenSqd(p0, p2)) < tolSqd) {
        (*points)[0] = p2;
        *points += 1;
        return 1;
    }

    GrPoint q[] = {
        GrPoint(GrScalarAve(p0.fX, p1.fX), GrScalarAve(p0.fY, p1.fY)),
        GrPoint(GrScalarAve(p1.fX, p2.fX), GrScalarAve(p1.fY, p2.fY)),
    };
    GrPoint r(GrScalarAve(q[0].fX, q[1].fX), GrScalarAve(q[0].fY, q[1].fY));

    pointsLeft >>= 1;
    uint32_t a = generate_quadratic_points(p0, q[0], r, tolSqd, points, pointsLeft);
    uint32_t b = generate_quadratic_points(r, q[1], p2, tolSqd, points, pointsLeft);
    return a + b;
}

static uint32_t cubic_point_count(const GrPoint points[], GrScalar tol) {
    GrScalar d = GrMax(points[1].distanceToLineSegmentBetweenSqd(points[0], points[3]),
                       points[2].distanceToLineSegmentBetweenSqd(points[0], points[3]));
    d = sqrtf(d);
    if (d < tol) {
        return 1;
    } else {
        d = ceilf(sqrtf(d/tol));
        return GrNextPow2((uint32_t)d);
    }
}

static uint32_t generate_cubic_points(const GrPoint& p0,
                                      const GrPoint& p1,
                                      const GrPoint& p2,
                                      const GrPoint& p3,
                                      GrScalar tolSqd,
                                      GrPoint** points,
                                      uint32_t pointsLeft) {
    if (pointsLeft < 2 ||
        (p1.distanceToLineSegmentBetweenSqd(p0, p3) < tolSqd &&
         p2.distanceToLineSegmentBetweenSqd(p0, p3) < tolSqd)) {
            (*points)[0] = p3;
            *points += 1;
            return 1;
        }
    GrPoint q[] = {
        GrPoint(GrScalarAve(p0.fX, p1.fX), GrScalarAve(p0.fY, p1.fY)),
        GrPoint(GrScalarAve(p1.fX, p2.fX), GrScalarAve(p1.fY, p2.fY)),
        GrPoint(GrScalarAve(p2.fX, p3.fX), GrScalarAve(p2.fY, p3.fY))
    };
    GrPoint r[] = {
        GrPoint(GrScalarAve(q[0].fX, q[1].fX), GrScalarAve(q[0].fY, q[1].fY)),
        GrPoint(GrScalarAve(q[1].fX, q[2].fX), GrScalarAve(q[1].fY, q[2].fY))
    };
    GrPoint s(GrScalarAve(r[0].fX, r[1].fX), GrScalarAve(r[0].fY, r[1].fY));
    pointsLeft >>= 1;
    uint32_t a = generate_cubic_points(p0, q[0], r[0], s, tolSqd, points, pointsLeft);
    uint32_t b = generate_cubic_points(s, r[1], q[2], p3, tolSqd, points, pointsLeft);
    return a + b;
}

#else // !NEW_EVAL

static GrScalar gr_eval_quad(const GrScalar coord[], GrScalar t) {
    GrScalar A = coord[0] - 2 * coord[2] + coord[4];
    GrScalar B = 2 * (coord[2] - coord[0]);
    GrScalar C = coord[0];

    return GrMul(GrMul(A, t) + B, t) + C;
}

static void gr_eval_quad_at(const GrPoint src[3], GrScalar t, GrPoint* pt) {
    GrAssert(src);
    GrAssert(pt);
    GrAssert(t >= 0 && t <= GR_Scalar1);
    pt->set(gr_eval_quad(&src[0].fX, t), gr_eval_quad(&src[0].fY, t));
}

static GrScalar gr_eval_cubic(const GrScalar coord[], GrScalar t) {
    GrScalar A = coord[6] - coord[0] + 3 * (coord[2] - coord[4]);
    GrScalar B = 3 * (coord[0] - 2 * coord[2] + coord[4]);
    GrScalar C = 3 * (coord[2] - coord[0]);
    GrScalar D = coord[0];

    return GrMul(GrMul(GrMul(A, t) + B, t) + C, t) + D;
}

static void gr_eval_cubic_at(const GrPoint src[4], GrScalar t, GrPoint* pt) {
    GrAssert(src);
    GrAssert(pt);
    GrAssert(t >= 0 && t <= GR_Scalar1);

    pt->set(gr_eval_cubic(&src[0].fX, t), gr_eval_cubic(&src[0].fY, t));
}

#endif // !NEW_EVAL

static int worst_case_point_count(GrPathIter* path,
                                  int* subpaths,
                                  const GrMatrix& matrix,
                                  GrScalar tol) {
    int pointCount = 0;
    *subpaths = 1;

    bool first = true;

    GrPathIter::Command cmd;

    GrPoint pts[4];
    while ((cmd = path->next(pts)) != GrPathIter::kEnd_Command) {

        switch (cmd) {
            case GrPathIter::kLine_Command:
                pointCount += 1;
                break;
            case GrPathIter::kQuadratic_Command:
#if NEW_EVAL
                matrix.mapPoints(pts, pts, 3);
                pointCount += quadratic_point_count(pts, tol);
#else
                pointCount += 9;
#endif
                break;
            case GrPathIter::kCubic_Command:
#if NEW_EVAL
                matrix.mapPoints(pts, pts, 4);
                pointCount += cubic_point_count(pts, tol);
#else
                pointCount += 17;
#endif
                break;
            case GrPathIter::kMove_Command:
                pointCount += 1;
                if (!first) {
                    ++(*subpaths);
                }
                break;
            default:
                break;
        }
        first = false;
    }
    return pointCount;
}

static inline bool single_pass_path(const GrPathIter& path,
                                    GrContext::PathFills fill,
                                    bool useTex,
                                    const GrGpu& gpu) {
#if STENCIL_OFF
    return true;
#else
    if (GrContext::kEvenOdd_PathFill == fill) {
        GrPathIter::ConvexHint hint = path.hint();
        return hint == GrPathIter::kConvex_ConvexHint ||
               hint == GrPathIter::kNonOverlappingConvexPieces_ConvexHint;
    } else if (GrContext::kWinding_PathFill == fill) {
        GrPathIter::ConvexHint hint = path.hint();
        return hint == GrPathIter::kConvex_ConvexHint ||
               hint == GrPathIter::kNonOverlappingConvexPieces_ConvexHint ||
               (hint == GrPathIter::kSameWindingConvexPieces_ConvexHint &&
                gpu.canDisableBlend() && !gpu.isDitherState());

    }
    return false;
#endif
}

void GrContext::drawPath(GrPathIter* path, PathFills fill,
                         bool useTexture, const GrPoint* translate) {

    flushText();

    GrGpu::AutoStateRestore asr(fGpu);

#if NEW_EVAL
    GrMatrix viewM;
    fGpu->getViewMatrix(&viewM);
    // In order to tesselate the path we get a bound on how much the matrix can
    // stretch when mapping to screen coordinates.
    GrScalar stretch = viewM.getMaxStretch();
    bool useStretch = stretch > 0;
    GrScalar tol = EVAL_TOL;
    if (!useStretch) {
        // TODO: deal with perspective in some better way.
        tol /= 10;
    } else {
        // TODO: fixed point divide
        GrScalar sinv = 1 / stretch;
        tol = GrMul(tol, sinv);
        viewM = GrMatrix::I();
    }
    GrScalar tolSqd = GrMul(tol, tol);
#else
    // pass to worst_case... but won't be used.
    static const GrScalar tol = -1;
#endif

    int subpathCnt;
    int maxPts = worst_case_point_count(path,
                                        &subpathCnt,
#if CPU_TRANSFORM
                                        cpuMatrix,
#else
                                        GrMatrix::I(),
#endif
                                        tol);
    GrVertexLayout layout = 0;
    if (useTexture) {
        layout = GrDrawTarget::StagePosAsTexCoordVertexLayoutBit(0);
    }
    // add 4 to hold the bounding rect
    GrDrawTarget::AutoReleaseGeometry arg(fGpu, layout, maxPts + 4, 0);

    GrPoint* base = (GrPoint*) arg.vertices();
    GrPoint* vert = base;
    GrPoint* subpathBase = base;

    GrAutoSTMalloc<8, uint16_t> subpathVertCount(subpathCnt);

    path->rewind();

    // TODO: use primitve restart if available rather than multiple draws
    GrGpu::PrimitiveType  type;
    int                   passCount = 0;
    GrGpu::StencilPass    passes[3];
    bool                  reverse = false;

    if (kHairLine_PathFill == fill) {
        type = GrGpu::kLineStrip_PrimitiveType;
        passCount = 1;
        passes[0] = GrGpu::kNone_StencilPass;
    } else {
        type = GrGpu::kTriangleFan_PrimitiveType;
        if (single_pass_path(*path, fill, useTexture, *fGpu)) {
            passCount = 1;
            passes[0] = GrGpu::kNone_StencilPass;
        } else {
            switch (fill) {
                case kInverseEvenOdd_PathFill:
                    reverse = true;
                    // fallthrough
                case kEvenOdd_PathFill:
                    passCount = 2;
                    passes[0] = GrGpu::kEvenOddStencil_StencilPass;
                    passes[1] = GrGpu::kEvenOddColor_StencilPass;
                    break;

                case kInverseWinding_PathFill:
                    reverse = true;
                    // fallthrough
                case kWinding_PathFill:
                    passes[0] = GrGpu::kWindingStencil1_StencilPass;
                    if (fGpu->supportsSingleStencilPassWinding()) {
                        passes[1] = GrGpu::kWindingColor_StencilPass;
                        passCount = 2;
                    } else {
                        passes[1] = GrGpu::kWindingStencil2_StencilPass;
                        passes[2] = GrGpu::kWindingColor_StencilPass;
                        passCount = 3;
                    }
                    break;
                default:
                    GrAssert(!"Unknown path fill!");
                    return;
            }
        }
    }
    fGpu->setReverseFill(reverse);
#if CPU_TRANSFORM
    GrMatrix cpuMatrix;
    fGpu->getViewMatrix(&cpuMatrix);
    fGpu->setViewMatrix(GrMatrix::I());
#endif

    GrPoint pts[4];

    bool first = true;
    int subpath = 0;

    for (;;) {
        GrPathIter::Command cmd = path->next(pts);
#if CPU_TRANSFORM
        int numPts = GrPathIter::NumCommandPoints(cmd);
        cpuMatrix.mapPoints(pts, pts, numPts);
#endif
        switch (cmd) {
            case GrPathIter::kMove_Command:
                if (!first) {
                    subpathVertCount[subpath] = vert-subpathBase;
                    subpathBase = vert;
                    ++subpath;
                }
                *vert = pts[0];
                vert++;
                break;
            case GrPathIter::kLine_Command:
                *vert = pts[1];
                vert++;
                break;
            case GrPathIter::kQuadratic_Command: {
#if NEW_EVAL

                generate_quadratic_points(pts[0], pts[1], pts[2],
                                          tolSqd, &vert,
                                          quadratic_point_count(pts, tol));
#else
                const int n = 8;
                const GrScalar dt = GR_Scalar1 / n;
                GrScalar t = dt;
                for (int i = 1; i < n; i++) {
                    gr_eval_quad_at(pts, t, (GrPoint*)vert);
                    t += dt;
                    vert++;
                }
                vert->set(pts[2].fX, pts[2].fY);
                vert++;
#endif
                break;
            }
            case GrPathIter::kCubic_Command: {
#if NEW_EVAL
                generate_cubic_points(pts[0], pts[1], pts[2], pts[3],
                                      tolSqd, &vert,
                                      cubic_point_count(pts, tol));
#else
                const int n = 16;
                const GrScalar dt = GR_Scalar1 / n;
                GrScalar t = dt;
                for (int i = 1; i < n; i++) {
                    gr_eval_cubic_at(pts, t, (GrPoint*)vert);
                    t += dt;
                    vert++;
                }
                vert->set(pts[3].fX, pts[3].fY);
                vert++;
#endif
                break;
            }
            case GrPathIter::kClose_Command:
                break;
            case GrPathIter::kEnd_Command:
                subpathVertCount[subpath] = vert-subpathBase;
                ++subpath; // this could be only in debug
                goto FINISHED;
        }
        first = false;
    }
FINISHED:
    GrAssert(subpath == subpathCnt);
    GrAssert((vert - base) <= maxPts);

    if (translate) {
        int count = vert - base;
        for (int i = 0; i < count; i++) {
            base[i].offset(translate->fX, translate->fY);
        }
    }

    // arbitrary path complexity cutoff
    bool useBounds = fill != kHairLine_PathFill &&
                    (reverse || (vert - base) > 8);
    GrPoint* boundsVerts = base + maxPts;
    if (useBounds) {
        GrRect bounds;
        if (reverse) {
            GrAssert(NULL != fGpu->currentRenderTarget());
            // draw over the whole world.
            bounds.setLTRB(0, 0,
                           GrIntToScalar(fGpu->currentRenderTarget()->width()),
                           GrIntToScalar(fGpu->currentRenderTarget()->height()));
        } else {
            bounds.setBounds((GrPoint*)base, vert - base);
        }
        boundsVerts[0].setRectFan(bounds.fLeft, bounds.fTop, bounds.fRight,
                                  bounds.fBottom);
    }

    for (int p = 0; p < passCount; ++p) {
        fGpu->setStencilPass(passes[p]);
        if (useBounds && (GrGpu::kEvenOddColor_StencilPass == passes[p] ||
                          GrGpu::kWindingColor_StencilPass == passes[p])) {
            fGpu->drawNonIndexed(GrGpu::kTriangleFan_PrimitiveType,
                                 maxPts, 4);
        } else {
            int baseVertex = 0;
            for (int sp = 0; sp < subpathCnt; ++sp) {
                fGpu->drawNonIndexed(type,
                                     baseVertex,
                                     subpathVertCount[sp]);
                baseVertex += subpathVertCount[sp];
            }
        }
    }
}

void GrContext::flush(bool flushRenderTarget) {
    flushText();
    if (flushRenderTarget) {
        fGpu->forceRenderTargetFlush();
    }
}

void GrContext::flushText() {
    fTextDrawBuffer.playback(fGpu);
    fTextDrawBuffer.reset();
}

bool GrContext::readPixels(int left, int top, int width, int height,
                           GrTexture::PixelConfig config, void* buffer) {
    this->flush(true);
    return fGpu->readPixels(left, top, width, height, config, buffer);
}

void GrContext::writePixels(int left, int top, int width, int height,
                            GrTexture::PixelConfig config, const void* buffer,
                            size_t stride) {
    const GrGpu::TextureDesc desc = {
        0, GrGpu::kNone_AALevel, width, height, config
    };
    GrTexture* texture = fGpu->createTexture(desc, buffer, stride);
    if (NULL == texture) {
        return;
    }

    this->flush(true);

    GrAutoUnref                     aur(texture);
    GrDrawTarget::AutoStateRestore  asr(fGpu);

    GrMatrix matrix;
    matrix.setTranslate(GrIntToScalar(left), GrIntToScalar(top));
    fGpu->setViewMatrix(matrix);
    matrix.setScale(GR_Scalar1 / texture->allocWidth(),
                    GR_Scalar1 / texture->allocHeight());
    fGpu->setTextureMatrix(0, matrix);

    fGpu->disableState(GrDrawTarget::kClip_StateBit);
    fGpu->setAlpha(0xFF);
    fGpu->setBlendFunc(GrDrawTarget::kOne_BlendCoeff,
                       GrDrawTarget::kZero_BlendCoeff);
    fGpu->setTexture(0, texture);
    fGpu->setSamplerState(0, GrSamplerState::ClampNoFilter());

    this->fillRect(GrRect(0, 0, GrIntToScalar(width), GrIntToScalar(height)),
                   true);
}

////////////////////////////////////////////////////////////////////////////////


/* -------------------------------------------------------
 * Mimicking the GrGpu interface for now
 * TODO: define appropriate higher-level API for context
 */

void GrContext::resetContext() {
    fGpu->resetContext();
}

GrVertexBuffer* GrContext::createVertexBuffer(uint32_t size, bool dynamic) {
    return fGpu->createVertexBuffer(size, dynamic);
}

GrIndexBuffer* GrContext::createIndexBuffer(uint32_t size, bool dynamic) {
    return fGpu->createIndexBuffer(size, dynamic);
}

void GrContext::setTexture(int stage, GrTexture* texture) {
    fGpu->setTexture(stage, texture);
}

void GrContext::setRenderTarget(GrRenderTarget* target) {
    flushText();
    fGpu->setRenderTarget(target);
}

GrRenderTarget* GrContext::currentRenderTarget() const {
    return fGpu->currentRenderTarget();
}

void GrContext::setSamplerState(int stage, const GrSamplerState& samplerState) {
    fGpu->setSamplerState(stage, samplerState);
}

void GrContext::setTextureMatrix(int stage, const GrMatrix& m) {
    fGpu->setTextureMatrix(stage, m);
}

void GrContext::getViewMatrix(GrMatrix* m) const {
    fGpu->getViewMatrix(m);
}

void GrContext::setViewMatrix(const GrMatrix& m) {
    fGpu->setViewMatrix(m);
}

bool GrContext::reserveAndLockGeometry(GrVertexLayout    vertexLayout,
                                       uint32_t          vertexCount,
                                       uint32_t          indexCount,
                                       void**            vertices,
                                       void**            indices) {
    return fGpu->reserveAndLockGeometry(vertexLayout,
                                        vertexCount,
                                        indexCount,
                                        vertices,
                                        indices);
}

void GrContext::drawIndexed(GrGpu::PrimitiveType type,
                            uint32_t startVertex,
                            uint32_t startIndex,
                            uint32_t vertexCount,
                            uint32_t indexCount) {
    flushText();
    fGpu->drawIndexed(type,
                      startVertex,
                      startIndex,
                      vertexCount,
                      indexCount);
}

void GrContext::drawNonIndexed(GrGpu::PrimitiveType type,
                               uint32_t startVertex,
                               uint32_t vertexCount) {
    flushText();
    fGpu->drawNonIndexed(type,
                         startVertex,
                         vertexCount);
}

void GrContext::setVertexSourceToArray(const void* array,
                                       GrVertexLayout vertexLayout) {
    fGpu->setVertexSourceToArray(array, vertexLayout);
}

void GrContext::setIndexSourceToArray(const void* array) {
    fGpu->setIndexSourceToArray(array);
}

void GrContext::setVertexSourceToBuffer(GrVertexBuffer* buffer,
                                       GrVertexLayout vertexLayout) {
    fGpu->setVertexSourceToBuffer(buffer, vertexLayout);
}

void GrContext::setIndexSourceToBuffer(GrIndexBuffer* buffer) {
    fGpu->setIndexSourceToBuffer(buffer);
}

void GrContext::releaseReservedGeometry() {
    fGpu->releaseReservedGeometry();
}

void GrContext::setClip(const GrClip& clip) {
    fGpu->setClip(clip);
    fGpu->enableState(GrDrawTarget::kClip_StateBit);
}

void GrContext::setAlpha(uint8_t alpha) {
    fGpu->setAlpha(alpha);
}

void GrContext::setColor(GrColor color) {
    fGpu->setColor(color);
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

void GrContext::setAntiAlias(bool aa) {
    if (aa) {
        fGpu->enableState(GrGpu::kAntialias_StateBit);
    } else {
        fGpu->disableState(GrGpu::kAntialias_StateBit);
    }
}

void GrContext::setDither(bool dither) {
    // hack for now, since iPad dither is hella-slow
    dither = false;

    if (dither) {
        fGpu->enableState(GrGpu::kDither_StateBit);
    } else {
        fGpu->disableState(GrGpu::kDither_StateBit);
    }
}

void GrContext::setPointSize(float size) {
    fGpu->setPointSize(size);
}

void GrContext::setBlendFunc(GrGpu::BlendCoeff srcCoef,
                             GrGpu::BlendCoeff dstCoef) {
    fGpu->setBlendFunc(srcCoef, dstCoef);
}

void GrContext::resetStats() {
    fGpu->resetStats();
}

const GrGpu::Stats& GrContext::getStats() const {
    return fGpu->getStats();
}

void GrContext::printStats() const {
    fGpu->printStats();
}

GrContext::GrContext(GrGpu* gpu) :
        fVBAllocPool(gpu,
                     gpu->supportsBufferLocking() ? POOL_VB_SIZE : 0,
                     gpu->supportsBufferLocking() ? NUM_POOL_VBS : 0),
        fTextDrawBuffer(gpu->supportsBufferLocking() ? &fVBAllocPool : NULL) {
    fGpu = gpu;
    fGpu->ref();
    fTextureCache = new GrTextureCache(MAX_TEXTURE_CACHE_COUNT,
                                       MAX_TEXTURE_CACHE_BYTES);
    fFontCache = new GrFontCache(fGpu);
}

bool GrContext::finalizeTextureKey(GrTextureKey* key,
                                   const GrSamplerState& sampler) const {
    uint32_t bits = 0;
    uint16_t width = key->width();
    uint16_t height = key->height();
    if (fGpu->npotTextureSupport() < GrGpu::kNonRendertarget_NPOTTextureType) {
        if ((sampler.getWrapX() != GrSamplerState::kClamp_WrapMode ||
             sampler.getWrapY() != GrSamplerState::kClamp_WrapMode) &&
            (!GrIsPow2(width) || !GrIsPow2(height))) {
            bits |= 1;
            bits |= sampler.isFilter() ? 2 : 0;
        }
    }
    key->finalize(bits);
    return 0 != bits;
}

GrDrawTarget* GrContext::getTextTarget() {
#if DEFER_TEXT_RENDERING
    fTextDrawBuffer.initializeDrawStateAndClip(*fGpu);
    return &fTextDrawBuffer;
#else
    return fGpu;
#endif
}

const GrIndexBuffer* GrContext::quadIndexBuffer() const {
    return fGpu->quadIndexBuffer();
}

int GrContext::maxQuadsInIndexBuffer() const {
    return fGpu->maxQuadsInIndexBuffer();
}




