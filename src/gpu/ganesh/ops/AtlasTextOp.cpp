/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/ops/AtlasTextOp.h"

#include "include/core/SkRRect.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/ganesh/GrBufferAllocPool.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrClip.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrMeshDrawTarget.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrPipeline.h"
#include "src/gpu/ganesh/GrProcessorAnalysis.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrSimpleMesh.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrUserStencilSettings.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/effects/GrBitmapTextGeoProc.h"
#include "src/gpu/ganesh/effects/GrDistanceFieldGeoProc.h"
#include "src/gpu/ganesh/ops/GrDrawOp.h"
#include "src/gpu/ganesh/ops/GrSimpleMeshDrawOpHelper.h"
#include "src/gpu/ganesh/text/GrAtlasManager.h"
#include "src/text/gpu/DistanceFieldAdjustTable.h"
#include "src/text/gpu/GlyphVector.h"
#include "src/text/gpu/SubRunContainer.h"

#if defined(SK_GAMMA_APPLY_TO_A8)
#include "include/private/base/SkCPUTypes.h"
#include "src/core/SkMaskGamma.h"
#endif

#include <algorithm>
#include <functional>
#include <new>
#include <tuple>
#include <utility>

struct GrShaderCaps;

using MaskFormat = skgpu::MaskFormat;

namespace skgpu::ganesh {

inline static constexpr int kVerticesPerGlyph = 4;
inline static constexpr int kIndicesPerGlyph = 6;

// If we have thread local, then cache memory for a single AtlasTextOp.
static thread_local void* gCache = nullptr;
void* AtlasTextOp::operator new(size_t s) {
    if (gCache != nullptr) {
        return std::exchange(gCache, nullptr);
    }

    return ::operator new(s);
}

void AtlasTextOp::operator delete(void* bytes) noexcept {
    if (gCache == nullptr) {
        gCache = bytes;
        return;
    }
    ::operator delete(bytes);
}

void AtlasTextOp::ClearCache() {
    ::operator delete(gCache);
    gCache = nullptr;
}

namespace {
SkPMColor4f calculate_colors(skgpu::ganesh::SurfaceDrawContext* sdc,
                             const SkPaint& paint,
                             const SkMatrix& matrix,
                             MaskFormat maskFormat,
                             GrPaint* grPaint) {
    if (maskFormat == MaskFormat::kARGB) {
        SkPaintToGrPaintReplaceShader(sdc, paint, matrix, nullptr, grPaint);
        float a = grPaint->getColor4f().fA;
        return {a, a, a, a};
    }
    SkPaintToGrPaint(sdc, paint, matrix, grPaint);
    return grPaint->getColor4f();
}

SkMatrix position_matrix(const SkMatrix& drawMatrix, SkPoint drawOrigin) {
    SkMatrix position_matrix = drawMatrix;
    return position_matrix.preTranslate(drawOrigin.x(), drawOrigin.y());
}

AtlasTextOp::MaskType op_mask_type(MaskFormat maskFormat) {
    switch (maskFormat) {
        case MaskFormat::kA8:   return AtlasTextOp::MaskType::kGrayscaleCoverage;
        case MaskFormat::kA565: return AtlasTextOp::MaskType::kLCDCoverage;
        case MaskFormat::kARGB: return AtlasTextOp::MaskType::kColorBitmap;
    }
    SkUNREACHABLE;
}

enum ClipMethod {
    kClippedOut,
    kUnclipped,
    kGPUClipped,
    kGeometryClipped
};

std::tuple<ClipMethod, SkIRect>
calculate_clip(const GrClip* clip, SkRect deviceBounds, SkRect glyphBounds) {
    if (clip == nullptr && !deviceBounds.intersects(glyphBounds)) {
        return {kClippedOut, SkIRect::MakeEmpty()};
    } else if (clip != nullptr) {
        switch (auto result = clip->preApply(glyphBounds, GrAA::kNo); result.fEffect) {
            case GrClip::Effect::kClippedOut:
                return {kClippedOut, SkIRect::MakeEmpty()};
            case GrClip::Effect::kUnclipped:
                return {kUnclipped, SkIRect::MakeEmpty()};
            case GrClip::Effect::kClipped: {
                if (result.fIsRRect && result.fRRect.isRect()) {
                    SkRect r = result.fRRect.rect();
                    if (result.fAA == GrAA::kNo || GrClip::IsPixelAligned(r)) {
                        SkIRect clipRect = SkIRect::MakeEmpty();
                        // Clip geometrically during onPrepare using clipRect.
                        r.round(&clipRect);
                        if (clipRect.contains(glyphBounds)) {
                            // If fully within the clip, signal no clipping using the empty rect.
                            return {kUnclipped, SkIRect::MakeEmpty()};
                        }
                        // Use the clipRect to clip the geometry.
                        return {kGeometryClipped, clipRect};
                    }
                    // Partial pixel clipped at this point. Have the GPU handle it.
                }
            }
            break;
        }
    }
    return {kGPUClipped, SkIRect::MakeEmpty()};
}

#if !defined(SK_DISABLE_SDF_TEXT)
static std::tuple<AtlasTextOp::MaskType, uint32_t, bool> calculate_sdf_parameters(
        const skgpu::ganesh::SurfaceDrawContext& sdc,
        const SkMatrix& drawMatrix,
        bool useLCDText,
        bool isAntiAliased) {
    const GrColorInfo& colorInfo = sdc.colorInfo();
    const SkSurfaceProps& props = sdc.surfaceProps();
    using MT = AtlasTextOp::MaskType;
    bool isLCD = useLCDText && props.pixelGeometry() != kUnknown_SkPixelGeometry;
    MT maskType = !isAntiAliased ? MT::kAliasedDistanceField
                                 : isLCD ? MT::kLCDDistanceField
                                         : MT::kGrayscaleDistanceField;

    bool useGammaCorrectDistanceTable = colorInfo.isLinearlyBlended();
    uint32_t DFGPFlags = drawMatrix.isSimilarity() ? kSimilarity_DistanceFieldEffectFlag : 0;
    DFGPFlags |= drawMatrix.isScaleTranslate() ? kScaleOnly_DistanceFieldEffectFlag : 0;
    DFGPFlags |= useGammaCorrectDistanceTable ? kGammaCorrect_DistanceFieldEffectFlag : 0;
    DFGPFlags |= MT::kAliasedDistanceField == maskType ? kAliased_DistanceFieldEffectFlag : 0;
    DFGPFlags |= drawMatrix.hasPerspective() ? kPerspective_DistanceFieldEffectFlag : 0;

    if (isLCD) {
        bool isBGR = SkPixelGeometryIsBGR(props.pixelGeometry());
        bool isVertical = SkPixelGeometryIsV(props.pixelGeometry());
        DFGPFlags |= kUseLCD_DistanceFieldEffectFlag;
        DFGPFlags |= isBGR ? kBGR_DistanceFieldEffectFlag : 0;
        DFGPFlags |= isVertical ? kPortrait_DistanceFieldEffectFlag : 0;
    }
    return {maskType, DFGPFlags, useGammaCorrectDistanceTable};
}
#endif
} // anonymous namespace

std::tuple<const GrClip*, GrOp::Owner> AtlasTextOp::Make(SurfaceDrawContext* sdc,
                                                         const sktext::gpu::AtlasSubRun* subrun,
                                                         const GrClip* clip,
                                                         const SkMatrix& viewMatrix,
                                                         SkPoint drawOrigin,
                                                         const SkPaint& paint,
                                                         sk_sp<SkRefCnt>&& subRunStorage) {
    SkASSERT(subrun->glyphCount() != 0);
    const SkMatrix& positionMatrix = position_matrix(viewMatrix, drawOrigin);

    auto [needsTransform, subRunDeviceBounds] =
            subrun->deviceRectAndNeedsTransform(positionMatrix);
    if (subRunDeviceBounds.isEmpty()) {
        return {nullptr, nullptr};
    }

    SkIRect geometricClipRect = SkIRect::MakeEmpty();
    if (!needsTransform) {
        // We can clip geometrically using clipRect and ignore clip when an axis-aligned
        // rectangular non-AA clip is used. If clipRect is empty, and clip is nullptr, then
        // there is no clipping needed.
        const SkRect deviceBounds = SkRect::MakeWH(sdc->width(), sdc->height());
        auto [clipMethod, clipRect] = calculate_clip(clip, deviceBounds, subRunDeviceBounds);

        switch (clipMethod) {
            case kClippedOut:
                // Returning nullptr as op means skip this op.
                return {nullptr, nullptr};
            case kUnclipped:
            case kGeometryClipped:
                // GPU clip is not needed.
                clip = nullptr;
                break;
            case kGPUClipped:
                // Use the GPU clip; clipRect is ignored.
                break;
        }
        geometricClipRect = clipRect;

        if (!geometricClipRect.isEmpty()) { SkASSERT(clip == nullptr); }
    }

    GrPaint grPaint;
    const SkPMColor4f drawingColor = calculate_colors(sdc,
                                                      paint,
                                                      viewMatrix,
                                                      subrun->maskFormat(),
                                                      &grPaint);

    auto geometry = AtlasTextOp::Geometry::Make(*subrun,
                                                viewMatrix,
                                                drawOrigin,
                                                geometricClipRect,
                                                std::move(subRunStorage),
                                                drawingColor,
                                                sdc->arenaAlloc());

    GrRecordingContext* const rContext = sdc->recordingContext();

    GrOp::Owner op;
#if !defined(SK_DISABLE_SDF_TEXT)
    auto glyphParams = subrun->glyphParams();
    if (glyphParams.isSDF) {
         auto [maskType, DFGPFlags, useGammaCorrectDistanceTable] =
                 calculate_sdf_parameters(*sdc, viewMatrix, glyphParams.isLCD, glyphParams.isAA);
         op = GrOp::Make<AtlasTextOp>(rContext,
                                      maskType,
                                      true,
                                      subrun->glyphCount(),
                                      subRunDeviceBounds,
                                      SkPaintPriv::ComputeLuminanceColor(paint),
                                      useGammaCorrectDistanceTable,
                                      DFGPFlags,
                                      geometry,
                                      std::move(grPaint));
     } else
#endif
     {
        op = GrOp::Make<AtlasTextOp>(rContext,
                                     op_mask_type(subrun->maskFormat()),
                                     needsTransform,
                                     subrun->glyphCount(),
                                     subRunDeviceBounds,
                                     geometry,
                                     sdc->colorInfo(),
                                     std::move(grPaint));
     }

     return {clip, std::move(op)};
}

AtlasTextOp::AtlasTextOp(MaskType maskType,
                         bool needsTransform,
                         int glyphCount,
                         SkRect deviceRect,
                         Geometry* geo,
                         const GrColorInfo& dstColorInfo,
                         GrPaint&& paint)
        : INHERITED{ClassID()}
        , fProcessors(std::move(paint))
        , fNumGlyphs(glyphCount)
        , fDFGPFlags(0)
        , fMaskType(static_cast<uint32_t>(maskType))
        , fUsesLocalCoords(false)
        , fNeedsGlyphTransform(needsTransform)
        , fHasPerspective(needsTransform && geo->fDrawMatrix.hasPerspective())
        , fUseGammaCorrectDistanceTable(false)
        , fHead{geo}
        , fTail{&fHead->fNext} {
    // We don't have tight bounds on the glyph paths in device space. For the purposes of bounds
    // we treat this as a set of non-AA rects rendered with a texture.
    this->setBounds(deviceRect, HasAABloat::kNo, IsHairline::kNo);
    if (maskType == MaskType::kColorBitmap) {
        // We assume that color emoji use the sRGB colorspace
        fColorSpaceXform = dstColorInfo.refColorSpaceXformFromSRGB();
    }
}

AtlasTextOp::AtlasTextOp(MaskType maskType,
                         bool needsTransform,
                         int glyphCount,
                         SkRect deviceRect,
                         SkColor luminanceColor,
                         bool useGammaCorrectDistanceTable,
                         uint32_t DFGPFlags,
                         Geometry* geo,
                         GrPaint&& paint)
        : INHERITED{ClassID()}
        , fProcessors(std::move(paint))
        , fNumGlyphs(glyphCount)
        , fDFGPFlags(DFGPFlags)
        , fMaskType(static_cast<uint32_t>(maskType))
        , fUsesLocalCoords(false)
        , fNeedsGlyphTransform(needsTransform)
        , fHasPerspective(needsTransform && geo->fDrawMatrix.hasPerspective())
        , fUseGammaCorrectDistanceTable(useGammaCorrectDistanceTable)
        , fLuminanceColor(luminanceColor)
        , fHead{geo}
        , fTail{&fHead->fNext} {
    // We don't have tight bounds on the glyph paths in device space. For the purposes of bounds
    // we treat this as a set of non-AA rects rendered with a texture.
    this->setBounds(deviceRect, HasAABloat::kNo, IsHairline::kNo);
}

auto AtlasTextOp::Geometry::Make(const sktext::gpu::AtlasSubRun& subRun,
                                 const SkMatrix& drawMatrix,
                                 SkPoint drawOrigin,
                                 SkIRect clipRect,
                                 sk_sp<SkRefCnt>&& supportData,
                                 const SkPMColor4f& color,
                                 SkArenaAlloc* alloc) -> Geometry* {
    // Bypass the automatic dtor behavior in SkArenaAlloc. I'm leaving this up to the Op to run
    // all geometry dtors for now.
    void* geo = alloc->makeBytesAlignedTo(sizeof(Geometry), alignof(Geometry));
    return new(geo) Geometry{subRun,
                             drawMatrix,
                             drawOrigin,
                             clipRect,
                             std::move(supportData),
                             color};
}

void AtlasTextOp::Geometry::fillVertexData(void *dst, int offset, int count) const {
    fSubRun.fillVertexData(
            dst, offset, count, fColor, fDrawMatrix, fDrawOrigin, fClipRect);
}

void AtlasTextOp::visitProxies(const GrVisitProxyFunc& func) const {
    fProcessors.visitProxies(func);
}

#if defined(GPU_TEST_UTILS)
SkString AtlasTextOp::onDumpInfo() const {
    SkString str;
    int i = 0;
    for(Geometry* geom = fHead; geom != nullptr; geom = geom->fNext) {
        str.appendf("%d: Color: 0x%08x Trans: %.2f,%.2f\n",
                    i++,
                    geom->fColor.toBytes_RGBA(),
                    geom->fDrawOrigin.x(),
                    geom->fDrawOrigin.y());
    }

    str += fProcessors.dumpProcessors();
    return str;
}
#endif

GrDrawOp::FixedFunctionFlags AtlasTextOp::fixedFunctionFlags() const {
    return FixedFunctionFlags::kNone;
}

GrProcessorSet::Analysis AtlasTextOp::finalize(const GrCaps& caps,
                                               const GrAppliedClip* clip,
                                               GrClampType clampType) {
    GrProcessorAnalysisCoverage coverage;
    GrProcessorAnalysisColor color;
    if (this->maskType() == MaskType::kColorBitmap) {
        color.setToUnknown();
    } else {
        // finalize() is called before any merging is done, so at this point there's at most one
        // Geometry with a color. Later, for non-bitmap ops, we may have mixed colors.
        color.setToConstant(fHead->fColor);
    }

    switch (this->maskType()) {
        case MaskType::kGrayscaleCoverage:
#if !defined(SK_DISABLE_SDF_TEXT)
        case MaskType::kAliasedDistanceField:
        case MaskType::kGrayscaleDistanceField:
#endif
            coverage = GrProcessorAnalysisCoverage::kSingleChannel;
            break;
        case MaskType::kLCDCoverage:
#if !defined(SK_DISABLE_SDF_TEXT)
        case MaskType::kLCDDistanceField:
#endif
            coverage = GrProcessorAnalysisCoverage::kLCD;
            break;
        case MaskType::kColorBitmap:
            coverage = GrProcessorAnalysisCoverage::kNone;
            break;
    }

    auto analysis = fProcessors.finalize(color, coverage, clip, &GrUserStencilSettings::kUnused,
                                         caps, clampType, &fHead->fColor);
    // TODO(michaelludwig): Once processor analysis can be done external to op creation/finalization
    // the atlas op metadata can be fully const. This is okay for now since finalize() happens
    // before the op is merged, so during combineIfPossible, metadata is effectively const.
    fUsesLocalCoords = analysis.usesLocalCoords();
    return analysis;
}

void AtlasTextOp::onPrepareDraws(GrMeshDrawTarget* target) {
    auto resourceProvider = target->resourceProvider();

    // If we need local coordinates, compute an inverse view matrix. If this is solid color, the
    // processor analysis will not require local coords and the GPs will skip local coords when
    // the matrix is identity. When the shaders require local coords, combineIfPossible requires all
    // all geometries to have same draw matrix.
    SkMatrix localMatrix = SkMatrix::I();
    if (fUsesLocalCoords && !fHead->fDrawMatrix.invert(&localMatrix)) {
        return;
    }

    GrAtlasManager* atlasManager = target->atlasManager();

    MaskFormat maskFormat = this->maskFormat();

    unsigned int numActiveViews;
    const GrSurfaceProxyView* views = atlasManager->getViews(maskFormat, &numActiveViews);
    if (!views) {
        SkDebugf("Could not allocate backing texture for atlas\n");
        return;
    }
    SkASSERT(views[0].proxy());

    static constexpr int kMaxTextures = GrBitmapTextGeoProc::kMaxTextures;
#if !defined(SK_DISABLE_SDF_TEXT)
    static_assert(GrDistanceFieldA8TextGeoProc::kMaxTextures == kMaxTextures);
    static_assert(GrDistanceFieldLCDTextGeoProc::kMaxTextures == kMaxTextures);
#endif

    auto primProcProxies = target->allocPrimProcProxyPtrs(kMaxTextures);
    for (unsigned i = 0; i < numActiveViews; ++i) {
        primProcProxies[i] = views[i].proxy();
        // This op does not know its atlas proxies when it is added to a OpsTasks, so the proxies
        // don't get added during the visitProxies call. Thus we add them here.
        target->sampledProxyArray()->push_back(views[i].proxy());
    }

    FlushInfo flushInfo;
    flushInfo.fPrimProcProxies = primProcProxies;
    flushInfo.fIndexBuffer = resourceProvider->refNonAAQuadIndexBuffer();

#if !defined(SK_DISABLE_SDF_TEXT)
    if (this->usesDistanceFields()) {
        flushInfo.fGeometryProcessor = this->setupDfProcessor(target->allocator(),
                                                              *target->caps().shaderCaps(),
                                                              localMatrix, views, numActiveViews);
    } else
#endif
    {
        auto filter = fNeedsGlyphTransform ? GrSamplerState::Filter::kLinear
                                           : GrSamplerState::Filter::kNearest;
        // Bitmap text uses a single color, combineIfPossible ensures all geometries have the same
        // color, so we can use the first's without worry.
        flushInfo.fGeometryProcessor = GrBitmapTextGeoProc::Make(
                target->allocator(), *target->caps().shaderCaps(), fHead->fColor,
                /*wideColor=*/false, fColorSpaceXform, views, numActiveViews, filter,
                maskFormat, localMatrix, fHasPerspective);
    }

    const int vertexStride = (int)flushInfo.fGeometryProcessor->vertexStride();

    // Ensure we don't request an insanely large contiguous vertex allocation.
    static const int kMaxVertexBytes = GrBufferAllocPool::kDefaultBufferSize;
    const int quadSize = vertexStride * kVerticesPerGlyph;
    const int maxQuadsPerBuffer = kMaxVertexBytes / quadSize;

    int allGlyphsCursor = 0;
    const int allGlyphsEnd = fNumGlyphs;
    int quadCursor;
    int quadEnd;
    char* vertices;

    auto resetVertexBuffer = [&] {
        quadCursor = 0;
        quadEnd = std::min(maxQuadsPerBuffer, allGlyphsEnd - allGlyphsCursor);

        vertices = (char*)target->makeVertexSpace(
                vertexStride,
                kVerticesPerGlyph * quadEnd,
                &flushInfo.fVertexBuffer,
                &flushInfo.fVertexOffset);

        if (!vertices || !flushInfo.fVertexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return false;
        }
        return true;
    };

    if (!resetVertexBuffer()) {
        return;
    }

    for (const Geometry* geo = fHead; geo != nullptr; geo = geo->fNext) {
        const sktext::gpu::AtlasSubRun& subRun = geo->fSubRun;
        SkASSERTF((int) subRun.vertexStride(geo->fDrawMatrix) == vertexStride,
                  "subRun stride: %d vertex buffer stride: %d\n",
                  (int)subRun.vertexStride(geo->fDrawMatrix), vertexStride);

        const int subRunEnd = subRun.glyphCount();
        auto regenerateDelegate = [&](sktext::gpu::GlyphVector* glyphs,
                                      int begin,
                                      int end,
                                      skgpu::MaskFormat maskFormat,
                                      int padding) {
            return glyphs->regenerateAtlasForGanesh(begin, end, maskFormat, padding, target);
        };
        for (int subRunCursor = 0; subRunCursor < subRunEnd;) {
            // Regenerate the atlas for the remainder of the glyphs in the run, or the remainder
            // of the glyphs to fill the vertex buffer.
            int regenEnd = subRunCursor + std::min(subRunEnd - subRunCursor, quadEnd - quadCursor);
            auto[ok, glyphsRegenerated] = subRun.regenerateAtlas(subRunCursor, regenEnd,
                                                                 regenerateDelegate);
            // There was a problem allocating the glyph in the atlas. Bail.
            if (!ok) {
                return;
            }

            geo->fillVertexData(vertices + quadCursor * quadSize, subRunCursor, glyphsRegenerated);

            subRunCursor += glyphsRegenerated;
            quadCursor += glyphsRegenerated;
            allGlyphsCursor += glyphsRegenerated;
            flushInfo.fGlyphsToFlush += glyphsRegenerated;

            if (quadCursor == quadEnd || subRunCursor < subRunEnd) {
                // Flush if not all the glyphs are drawn because either the quad buffer is full or
                // the atlas is out of space.
                if (subRunCursor < subRunEnd) {
                    ATRACE_ANDROID_FRAMEWORK_ALWAYS("Atlas full");
                }
                this->createDrawForGeneratedGlyphs(target, &flushInfo);
                if (quadCursor == quadEnd && allGlyphsCursor < allGlyphsEnd) {
                    // If the vertex buffer is full and there are still glyphs to draw then
                    // get a new buffer.
                    if(!resetVertexBuffer()) {
                        return;
                    }
                }
            }
        }
    }
}

void AtlasTextOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    auto pipeline = GrSimpleMeshDrawOpHelper::CreatePipeline(flushState,
                                                             std::move(fProcessors),
                                                             GrPipeline::InputFlags::kNone);

    flushState->executeDrawsAndUploadsForMeshDrawOp(this, chainBounds, pipeline,
                                                    &GrUserStencilSettings::kUnused);
}

void AtlasTextOp::createDrawForGeneratedGlyphs(GrMeshDrawTarget* target,
                                               FlushInfo* flushInfo) const {
    if (!flushInfo->fGlyphsToFlush) {
        return;
    }

    auto atlasManager = target->atlasManager();

    GrGeometryProcessor* gp = flushInfo->fGeometryProcessor;
    MaskFormat maskFormat = this->maskFormat();

    unsigned int numActiveViews;
    const GrSurfaceProxyView* views = atlasManager->getViews(maskFormat, &numActiveViews);
    SkASSERT(views);
    // Something has gone terribly wrong, bail
    if (!views || 0 == numActiveViews) {
        return;
    }
    if (gp->numTextureSamplers() != (int) numActiveViews) {
        // During preparation the number of atlas pages has increased.
        // Update the proxies used in the GP to match.
        for (unsigned i = gp->numTextureSamplers(); i < numActiveViews; ++i) {
            flushInfo->fPrimProcProxies[i] = views[i].proxy();
            // This op does not know its atlas proxies when it is added to a OpsTasks, so the
            // proxies don't get added during the visitProxies call. Thus we add them here.
            target->sampledProxyArray()->push_back(views[i].proxy());
            // These will get unreffed when the previously recorded draws destruct.
            for (int d = 0; d < flushInfo->fNumDraws; ++d) {
                flushInfo->fPrimProcProxies[i]->ref();
            }
        }
#if !defined(SK_DISABLE_SDF_TEXT)
        if (this->usesDistanceFields()) {
            if (this->isLCD()) {
                reinterpret_cast<GrDistanceFieldLCDTextGeoProc*>(gp)->addNewViews(
                        views, numActiveViews, GrSamplerState::Filter::kLinear);
            } else {
                reinterpret_cast<GrDistanceFieldA8TextGeoProc*>(gp)->addNewViews(
                        views, numActiveViews, GrSamplerState::Filter::kLinear);
            }
        } else
#endif
        {
            auto filter = fNeedsGlyphTransform ? GrSamplerState::Filter::kLinear
                                               : GrSamplerState::Filter::kNearest;
            reinterpret_cast<GrBitmapTextGeoProc*>(gp)->addNewViews(views, numActiveViews, filter);
        }
    }
    int maxGlyphsPerDraw = static_cast<int>(flushInfo->fIndexBuffer->size() / sizeof(uint16_t) / 6);
    GrSimpleMesh* mesh = target->allocMesh();
    mesh->setIndexedPatterned(flushInfo->fIndexBuffer, kIndicesPerGlyph, flushInfo->fGlyphsToFlush,
                              maxGlyphsPerDraw, flushInfo->fVertexBuffer, kVerticesPerGlyph,
                              flushInfo->fVertexOffset);
    target->recordDraw(flushInfo->fGeometryProcessor, mesh, 1, flushInfo->fPrimProcProxies,
                       GrPrimitiveType::kTriangles);
    flushInfo->fVertexOffset += kVerticesPerGlyph * flushInfo->fGlyphsToFlush;
    flushInfo->fGlyphsToFlush = 0;
    ++flushInfo->fNumDraws;
}

GrOp::CombineResult AtlasTextOp::onCombineIfPossible(GrOp* t, SkArenaAlloc*, const GrCaps& caps) {
    auto that = t->cast<AtlasTextOp>();

    if (fDFGPFlags != that->fDFGPFlags ||
        fMaskType != that->fMaskType ||
        fUsesLocalCoords != that->fUsesLocalCoords ||
        fNeedsGlyphTransform != that->fNeedsGlyphTransform ||
        fHasPerspective != that->fHasPerspective ||
        fUseGammaCorrectDistanceTable != that->fUseGammaCorrectDistanceTable) {
        // All flags must match for an op to be combined
        return CombineResult::kCannotCombine;
    }

    if (fProcessors != that->fProcessors) {
        return CombineResult::kCannotCombine;
    }

    if (fUsesLocalCoords) {
        // If the fragment processors use local coordinates, the GPs compute them using the inverse
        // of the view matrix stored in a uniform, so all geometries must have the same matrix.
        const SkMatrix& thisFirstMatrix = fHead->fDrawMatrix;
        const SkMatrix& thatFirstMatrix = that->fHead->fDrawMatrix;
        if (!SkMatrixPriv::CheapEqual(thisFirstMatrix, thatFirstMatrix)) {
            return CombineResult::kCannotCombine;
        }
    }

#if !defined(SK_DISABLE_SDF_TEXT)
   if (this->usesDistanceFields()) {
        SkASSERT(that->usesDistanceFields());
        if (fLuminanceColor != that->fLuminanceColor) {
            return CombineResult::kCannotCombine;
        }
    } else
#endif
    {
        if (this->maskType() == MaskType::kColorBitmap &&
            fHead->fColor != that->fHead->fColor) {
            // This ensures all merged bitmap color text ops have a constant color
            return CombineResult::kCannotCombine;
        }
    }

    fNumGlyphs += that->fNumGlyphs;

    // After concat, that's geometry list is emptied so it will not unref the blobs when destructed
    this->addGeometry(that->fHead);
    that->fHead = nullptr;
    return CombineResult::kMerged;
}

#if !defined(SK_DISABLE_SDF_TEXT)
GrGeometryProcessor* AtlasTextOp::setupDfProcessor(SkArenaAlloc* arena,
                                                   const GrShaderCaps& caps,
                                                   const SkMatrix& localMatrix,
                                                   const GrSurfaceProxyView* views,
                                                   unsigned int numActiveViews) const {
    auto dfAdjustTable = sktext::gpu::DistanceFieldAdjustTable::Get();

    // see if we need to create a new effect
    if (this->isLCD()) {
        float redCorrection = dfAdjustTable->getAdjustment(SkColorGetR(fLuminanceColor),
                                                           fUseGammaCorrectDistanceTable);
        float greenCorrection = dfAdjustTable->getAdjustment(SkColorGetG(fLuminanceColor),
                                                             fUseGammaCorrectDistanceTable);
        float blueCorrection = dfAdjustTable->getAdjustment(SkColorGetB(fLuminanceColor),
                                                            fUseGammaCorrectDistanceTable);
        GrDistanceFieldLCDTextGeoProc::DistanceAdjust widthAdjust =
                GrDistanceFieldLCDTextGeoProc::DistanceAdjust::Make(
                        redCorrection, greenCorrection, blueCorrection);
        return GrDistanceFieldLCDTextGeoProc::Make(arena, caps, views, numActiveViews,
                                                   GrSamplerState::Filter::kLinear, widthAdjust,
                                                   fDFGPFlags, localMatrix);
    } else {
#if defined(SK_GAMMA_APPLY_TO_A8)
        float correction = 0;
        if (this->maskType() != MaskType::kAliasedDistanceField) {
            U8CPU lum = SkColorSpaceLuminance::computeLuminance(SK_GAMMA_EXPONENT, fLuminanceColor);
            correction = dfAdjustTable->getAdjustment(lum, fUseGammaCorrectDistanceTable);
        }
        return GrDistanceFieldA8TextGeoProc::Make(arena, caps, views, numActiveViews,
                                                  GrSamplerState::Filter::kLinear, correction,
                                                  fDFGPFlags, localMatrix);
#else
        return GrDistanceFieldA8TextGeoProc::Make(arena, caps, views, numActiveViews,
                                                  GrSamplerState::Filter::kLinear, fDFGPFlags,
                                                  localMatrix);
#endif
    }
}
#endif // !defined(SK_DISABLE_SDF_TEXT)

} // namespace skgpu::ganesh


