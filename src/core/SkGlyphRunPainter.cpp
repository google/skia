/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkGlyphRunPainter.h"

#if SK_SUPPORT_GPU
#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/text/GrTextBlobCache.h"
#include "src/gpu/text/GrTextContext.h"
#endif

#include "include/core/SkColorFilter.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPathEffect.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkDevice.h"
#include "src/core/SkDistanceFieldGen.h"
#include "src/core/SkDraw.h"
#include "src/core/SkEnumerate.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeForGPU.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTraceEvent.h"

#include <climits>

// -- SkGlyphRunListPainter ------------------------------------------------------------------------
SkGlyphRunListPainter::SkGlyphRunListPainter(const SkSurfaceProps& props,
                                             SkColorType colorType,
                                             SkScalerContextFlags flags,
                                             SkStrikeForGPUCacheInterface* strikeCache)
        : fDeviceProps{props}
        ,  fBitmapFallbackProps{SkSurfaceProps{props.flags(), kUnknown_SkPixelGeometry}}
        ,  fColorType{colorType}, fScalerContextFlags{flags}
        ,  fStrikeCache{strikeCache} {}

// TODO: unify with code in GrTextContext.cpp
static SkScalerContextFlags compute_scaler_context_flags(const SkColorSpace* cs) {
    // If we're doing linear blending, then we can disable the gamma hacks.
    // Otherwise, leave them on. In either case, we still want the contrast boost:
    // TODO: Can we be even smarter about mask gamma based on the dest transfer function?
    if (cs && cs->gammaIsLinear()) {
        return SkScalerContextFlags::kBoostContrast;
    } else {
        return SkScalerContextFlags::kFakeGammaAndBoostContrast;
    }
}

SkGlyphRunListPainter::SkGlyphRunListPainter(const SkSurfaceProps& props,
                                             SkColorType colorType,
                                             SkColorSpace* cs,
                                             SkStrikeForGPUCacheInterface* strikeCache)
        : SkGlyphRunListPainter(props, colorType, compute_scaler_context_flags(cs), strikeCache) {}

#if SK_SUPPORT_GPU
SkGlyphRunListPainter::SkGlyphRunListPainter(const SkSurfaceProps& props, const GrColorInfo& csi)
        : SkGlyphRunListPainter(props,
                                kUnknown_SkColorType,
                                compute_scaler_context_flags(csi.colorSpace()),
                                SkStrikeCache::GlobalStrikeCache()) {}

SkGlyphRunListPainter::SkGlyphRunListPainter(const GrRenderTargetContext& rtc)
        : SkGlyphRunListPainter{rtc.surfaceProps(), rtc.colorInfo()} {}

#endif

void SkGlyphRunListPainter::drawForBitmapDevice(
        const SkGlyphRunList& glyphRunList, const SkMatrix& deviceMatrix,
        const BitmapDevicePainter* bitmapDevice) {
    ScopedBuffers _ = this->ensureBuffers(glyphRunList);

    // TODO: fStrikeCache is only used for GPU, and some compilers complain about it during the no
    //  gpu build. Remove when SkGlyphRunListPainter is split into GPU and CPU version.
    (void)fStrikeCache;

    const SkPaint& runPaint = glyphRunList.paint();
    // The bitmap blitters can only draw lcd text to a N32 bitmap in srcOver. Otherwise,
    // convert the lcd text into A8 text. The props communicates this to the scaler.
    auto& props = (kN32_SkColorType == fColorType && runPaint.isSrcOver())
                  ? fDeviceProps
                  : fBitmapFallbackProps;

    SkPoint origin = glyphRunList.origin();
    for (auto& glyphRun : glyphRunList) {
        const SkFont& runFont = glyphRun.font();

        fRejects.setSource(glyphRun.source());

        if (SkStrikeSpec::ShouldDrawAsPath(runPaint, runFont, deviceMatrix)) {

            SkStrikeSpec strikeSpec = SkStrikeSpec::MakePath(
                    runFont, runPaint, props, fScalerContextFlags);

            auto strike = strikeSpec.findOrCreateExclusiveStrike();

            fDrawable.startSource(fRejects.source(), origin);
            strike->prepareForPathDrawing(&fDrawable, &fRejects);
            fRejects.flipRejectsToSource();

            // The paint we draw paths with must have the same anti-aliasing state as the runFont
            // allowing the paths to have the same edging as the glyph masks.
            SkPaint pathPaint = runPaint;
            pathPaint.setAntiAlias(runFont.hasSomeAntiAliasing());

            bitmapDevice->paintPaths(&fDrawable, strikeSpec.strikeToSourceRatio(), pathPaint);
        }
        if (!fRejects.source().empty()) {
            SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
                    runFont, runPaint, props, fScalerContextFlags, deviceMatrix);

            auto strike = strikeSpec.findOrCreateExclusiveStrike();

            fDrawable.startDevice(fRejects.source(), origin, deviceMatrix, strike->roundingSpec());
            strike->prepareForDrawingMasksCPU(&fDrawable);
            bitmapDevice->paintMasks(&fDrawable, runPaint);
        }

        // TODO: have the mask stage above reject the glyphs that are too big, and handle the
        //  rejects in a more sophisticated stage.
    }
}

#if SK_SUPPORT_GPU
void SkGlyphRunListPainter::processGlyphRunList(const SkGlyphRunList& glyphRunList,
                                                const SkMatrix& viewMatrix,
                                                const SkSurfaceProps& props,
                                                bool contextSupportsDistanceFieldText,
                                                const GrTextContext::Options& options,
                                                SkGlyphRunPainterInterface* process) {

    SkPoint origin = glyphRunList.origin();
    const SkPaint& runPaint = glyphRunList.paint();
    ScopedBuffers _ = this->ensureBuffers(glyphRunList);

    for (const auto& glyphRun : glyphRunList) {
        fRejects.setSource(glyphRun.source());
        const SkFont& runFont = glyphRun.font();


        bool useSDFT = GrTextContext::CanDrawAsDistanceFields(
                runPaint, runFont, viewMatrix, props, contextSupportsDistanceFieldText, options);

        bool usePaths =
                useSDFT ? false : SkStrikeSpec::ShouldDrawAsPath(runPaint, runFont, viewMatrix);

        if (!useSDFT && !usePaths) {
            // Process masks - this should be the 99.99% case.

            SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
                    runFont, runPaint, fDeviceProps, fScalerContextFlags, viewMatrix);

            SkScopedStrikeForGPU strike = strikeSpec.findOrCreateScopedStrike(fStrikeCache);

            fDrawable.startDevice(fRejects.source(), origin, viewMatrix, strike->roundingSpec());
            strike->prepareForMaskDrawing(&fDrawable, &fRejects);
            fRejects.flipRejectsToSource();

            if (process) {
                // processDeviceMasks must be called even if there are no glyphs to make sure runs
                // are set correctly.
                process->processDeviceMasks(fDrawable.drawable(), strikeSpec);
            }
        } else if (useSDFT) {
            // Process SDFT - This should be the .009% case.
            SkScalar minScale, maxScale;
            SkStrikeSpec strikeSpec;
            std::tie(strikeSpec, minScale, maxScale) =
                    SkStrikeSpec::MakeSDFT(runFont, runPaint, fDeviceProps, viewMatrix, options);

            if (!strikeSpec.isEmpty()) {
                SkScopedStrikeForGPU strike = strikeSpec.findOrCreateScopedStrike(fStrikeCache);

                fDrawable.startSource(fRejects.source(), origin);
                strike->prepareForSDFTDrawing(&fDrawable, &fRejects);
                fRejects.flipRejectsToSource();

                if (process) {
                    // processSourceSDFT must be called even if there are no glyphs to make sure
                    // runs are set correctly.
                    process->processSourceSDFT(
                            fDrawable.drawable(), strikeSpec, runFont, minScale, maxScale);
                }
            }
        }

        // Glyphs are generated in different scales relative to the source space. Masks are drawn
        // in device space, and SDFT and Paths are draw in a fixed constant space. This is the
        // factor used to scale the generated glyphs back to source space.
        SkScalar maxDimensionInSourceSpace = 0.0;
        if (!fRejects.source().empty()) {
            // Path case
            SkStrikeSpec strikeSpec = SkStrikeSpec::MakePath(
                    runFont, runPaint, fDeviceProps, fScalerContextFlags);

            if (!strikeSpec.isEmpty()) {
                SkScopedStrikeForGPU strike = strikeSpec.findOrCreateScopedStrike(fStrikeCache);

                fDrawable.startSource(fRejects.source(), origin);
                strike->prepareForPathDrawing(&fDrawable, &fRejects);
                fRejects.flipRejectsToSource();
                maxDimensionInSourceSpace =
                        fRejects.rejectedMaxDimension() * strikeSpec.strikeToSourceRatio();

                if (process) {
                    // processSourcePaths must be called even if there are no glyphs to make sure
                    // runs are set correctly.
                    process->processSourcePaths(fDrawable.drawable(), runFont, strikeSpec);
                }
            }
        }

        // Getting glyphs to the screen in a fallback situation can be complex. Here is the set of
        // transformations that have to happen. Normally, they would all be accommodated by the font
        // scaler, but the atlas has an upper limit to the glyphs it can handle. So the GPU is used
        // to make up the difference from the smaller atlas size to the larger size needed by the
        // final transform. Here are the transformations that are applied.
        //
        // final transform = [view matrix] * [text scale] * [text size]
        //
        // There are three cases:
        // * Go Fast - view matrix is scale and translate, and all the glyphs are small enough
        //   Just scale the positions, and have the glyph cache handle the view matrix
        //   transformation.
        //   The text scale is 1.
        // * It's complicated - view matrix is not scale and translate, and the glyphs are small
        //   enough The glyph cache does not handle the view matrix, but stores the glyphs at the
        //   text size specified by the run paint. The GPU handles the rotation, etc. specified
        //   by the view matrix.
        //   The text scale is 1.
        // * Too big - The glyphs are too big to fit in the atlas
        //   Reduce the text size so the glyphs will fit in the atlas, but don't apply any
        //   transformations from the view matrix. Calculate a text scale based on that reduction.
        //   This scale factor is used to increase the size of the destination rectangles. The
        //   destination rectangles are then scaled, rotated, etc. by the GPU using the view matrix.

        if (!fRejects.source().empty() && maxDimensionInSourceSpace != 0) {

            SkScalar maxScale = viewMatrix.getMaxScale();

            // This is a linear estimate of the longest dimension among all the glyph widths and
            // heights.
            SkScalar conservativeMaxGlyphDimension = maxDimensionInSourceSpace * maxScale;

            // If the situation that the matrix is simple, and all the glyphs are small enough.
            // Go fast!
            // N.B. If the matrix has scale, that will be reflected in the strike through the
            // viewMatrix in the useFastPath case.
            bool useDeviceCache =
                    viewMatrix.isScaleTranslate()
                    && conservativeMaxGlyphDimension <= SkStrikeCommon::kSkSideTooBigForAtlas;

            // A scaled and translated transform is the common case, and is handled directly in
            // fallback. Even if the transform is scale and translate, fallback must be careful
            // to use glyphs that fit in the atlas. If a glyph will not fit in the atlas, then
            // the general transform case is used to render the glyphs.
            if (useDeviceCache) {
                SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
                        runFont, runPaint, fDeviceProps, fScalerContextFlags, viewMatrix);

                SkScopedStrikeForGPU strike = strikeSpec.findOrCreateScopedStrike(fStrikeCache);

                fDrawable.startDevice(
                        fRejects.source(), origin, viewMatrix, strike->roundingSpec());

                strike->prepareForMaskDrawing(&fDrawable, &fRejects);
                fRejects.flipRejectsToSource();
                SkASSERT(fRejects.source().empty());

                if (process) {
                    process->processDeviceMasks(fDrawable.drawable(), strikeSpec);
                }
            } else {
                // If the matrix is complicated or if scaling is used to fit the glyphs in the
                // atlas, then this case is used.

                SkStrikeSpec strikeSpec = SkStrikeSpec::MakeSourceFallback(
                        runFont, runPaint, fDeviceProps,
                        fScalerContextFlags, maxDimensionInSourceSpace);

                if (!strikeSpec.isEmpty()) {
                    SkScopedStrikeForGPU strike = strikeSpec.findOrCreateScopedStrike(fStrikeCache);

                    fDrawable.startSource(fRejects.source(), origin);
                    strike->prepareForMaskDrawing(&fDrawable, &fRejects);
                    fRejects.flipRejectsToSource();
                    SkASSERT(fRejects.source().empty());

                    if (process) {
                        process->processSourceMasks(fDrawable.drawable(), strikeSpec);
                    }
                }
            }
        }
    }  // For all glyph runs
}
#endif  // SK_SUPPORT_GPU

auto SkGlyphRunListPainter::ensureBuffers(const SkGlyphRunList& glyphRunList) -> ScopedBuffers {
    size_t size = 0;
    for (const SkGlyphRun& run : glyphRunList) {
        size = std::max(run.runSize(), size);
    }
    return ScopedBuffers(this, size);
}

SkGlyphRunListPainter::ScopedBuffers
SkGlyphRunListPainter::ensureBuffers(const SkGlyphRun& glyphRun) {
    return ScopedBuffers(this, glyphRun.runSize());
}

#if SK_SUPPORT_GPU
// -- GrTextContext --------------------------------------------------------------------------------
SkPMColor4f generate_filtered_color(const SkPaint& paint, const GrColorInfo& colorInfo) {
    SkColor4f filteredColor = paint.getColor4f();
    if (auto* xform = colorInfo.colorSpaceXformFromSRGB()) {
        filteredColor = xform->apply(filteredColor);
    }
    if (paint.getColorFilter() != nullptr) {
        filteredColor = paint.getColorFilter()->filterColor4f(filteredColor, colorInfo.colorSpace(),
                                                              colorInfo.colorSpace());
    }
    return filteredColor.premul();
}

void GrTextContext::drawGlyphRunList(
        GrRecordingContext* context, GrTextTarget* target, const GrClip& clip,
        const SkMatrix& viewMatrix, const SkSurfaceProps& props,
        const SkGlyphRunList& glyphRunList) {
    SkPoint origin = glyphRunList.origin();

    // Get the first paint to use as the key paint.
    const SkPaint& listPaint = glyphRunList.paint();

    SkPMColor4f filteredColor = generate_filtered_color(listPaint, target->colorInfo());
    GrColor color = generate_filtered_color(listPaint, target->colorInfo()).toBytes_RGBA();

    // If we have been abandoned, then don't draw
    if (context->priv().abandoned()) {
        return;
    }

    SkMaskFilterBase::BlurRec blurRec;
    // It might be worth caching these things, but its not clear at this time
    // TODO for animated mask filters, this will fill up our cache.  We need a safeguard here
    const SkMaskFilter* mf = listPaint.getMaskFilter();
    bool canCache = glyphRunList.canCache() && !(listPaint.getPathEffect() ||
                                                 (mf && !as_MFB(mf)->asABlur(&blurRec)));
    SkScalerContextFlags scalerContextFlags = ComputeScalerContextFlags(target->colorInfo());

    auto grStrikeCache = context->priv().getGrStrikeCache();
    GrTextBlobCache* textBlobCache = context->priv().getTextBlobCache();

    sk_sp<GrTextBlob> cacheBlob;
    GrTextBlob::Key key;
    if (canCache) {
        bool hasLCD = glyphRunList.anyRunsLCD();

        // We canonicalize all non-lcd draws to use kUnknown_SkPixelGeometry
        SkPixelGeometry pixelGeometry = hasLCD ? props.pixelGeometry() :
                                        kUnknown_SkPixelGeometry;

        // TODO we want to figure out a way to be able to use the canonical color on LCD text,
        // see the note on ComputeCanonicalColor above.  We pick a dummy value for LCD text to
        // ensure we always match the same key
        GrColor canonicalColor = hasLCD ? SK_ColorTRANSPARENT :
                                 ComputeCanonicalColor(listPaint, hasLCD);

        key.fPixelGeometry = pixelGeometry;
        key.fUniqueID = glyphRunList.uniqueID();
        key.fStyle = listPaint.getStyle();
        key.fHasBlur = SkToBool(mf);
        key.fCanonicalColor = canonicalColor;
        key.fScalerContextFlags = scalerContextFlags;
        cacheBlob = textBlobCache->find(key);
    }

    bool forceW = fOptions.fDistanceFieldVerticesAlwaysHaveW;
    if (cacheBlob) {
        if (cacheBlob->mustRegenerate(listPaint, glyphRunList.anyRunsSubpixelPositioned(),
                                      blurRec, viewMatrix, origin.x(),origin.y())) {
            // We have to remake the blob because changes may invalidate our masks.
            // TODO we could probably get away reuse most of the time if the pointer is unique,
            // but we'd have to clear the subrun information
            textBlobCache->remove(cacheBlob.get());
            cacheBlob = textBlobCache->makeCachedBlob(
                    glyphRunList, grStrikeCache, key, blurRec, viewMatrix, color, forceW);
            cacheBlob->generateFromGlyphRunList(
                    *context->priv().caps()->shaderCaps(), fOptions,
                    listPaint, viewMatrix, props,
                    glyphRunList, target->glyphPainter());
        } else {
            textBlobCache->makeMRU(cacheBlob.get());

            if (CACHE_SANITY_CHECK) {
                sk_sp<GrTextBlob> sanityBlob(textBlobCache->makeBlob(
                        glyphRunList, grStrikeCache, viewMatrix, color, forceW));
                sanityBlob->setupKey(key, blurRec, listPaint);
                cacheBlob->generateFromGlyphRunList(
                        *context->priv().caps()->shaderCaps(), fOptions,
                        listPaint, viewMatrix, props, glyphRunList,
                        target->glyphPainter());
                GrTextBlob::AssertEqual(*sanityBlob, *cacheBlob);
            }
        }
    } else {
        if (canCache) {
            cacheBlob = textBlobCache->makeCachedBlob(
                    glyphRunList, grStrikeCache, key, blurRec, viewMatrix, color, forceW);
        } else {
            cacheBlob = textBlobCache->makeBlob(
                    glyphRunList, grStrikeCache, viewMatrix, color, forceW);
        }
        cacheBlob->generateFromGlyphRunList(
                *context->priv().caps()->shaderCaps(), fOptions, listPaint,
                viewMatrix, props, glyphRunList,
                target->glyphPainter());
    }

    cacheBlob->flush(target, props, fDistanceAdjustTable.get(), listPaint, filteredColor,
                     clip, viewMatrix, origin.x(), origin.y());
}

void GrTextBlob::generateFromGlyphRunList(const GrShaderCaps& shaderCaps,
                                          const GrTextContext::Options& options,
                                          const SkPaint& paint,
                                          const SkMatrix& viewMatrix,
                                          const SkSurfaceProps& props,
                                          const SkGlyphRunList& glyphRunList,
                                          SkGlyphRunListPainter* glyphPainter) {
    const SkPaint& runPaint = glyphRunList.paint();
    this->initReusableBlob(SkPaintPriv::ComputeLuminanceColor(runPaint));

    glyphPainter->processGlyphRunList(glyphRunList,
                                      viewMatrix,
                                      props,
                                      shaderCaps.supportsDistanceFieldText(),
                                      options,
                                      this);
}

GrTextBlob::SubRun::SubRun(SubRunType type, GrTextBlob* textBlob, const SkStrikeSpec& strikeSpec,
                           GrMaskFormat format, const GrTextBlob::SubRunBufferSpec& bufferSpec,
                           sk_sp<GrTextStrike>&& grStrike)
        : fType{type}
        , fBlob{textBlob}
        , fMaskFormat{format}
        , fGlyphStartIndex{std::get<0>(bufferSpec)}
        , fGlyphEndIndex{std::get<1>(bufferSpec)}
        , fVertexStartIndex{std::get<2>(bufferSpec)}
        , fVertexEndIndex{std::get<3>(bufferSpec)}
        , fStrikeSpec{strikeSpec}
        , fStrike{grStrike}
        , fColor{textBlob->fColor}
        , fX{textBlob->fInitialOrigin.x()}
        , fY{textBlob->fInitialOrigin.y()}
        , fCurrentViewMatrix{textBlob->fInitialViewMatrix} {
    SkASSERT(type != kTransformedPath);
}

GrTextBlob::SubRun::SubRun(GrTextBlob* textBlob, const SkStrikeSpec& strikeSpec)
        : fType{kTransformedPath}
        , fBlob{textBlob}
        , fMaskFormat{kA8_GrMaskFormat}
        , fGlyphStartIndex{0}
        , fGlyphEndIndex{0}
        , fVertexStartIndex{0}
        , fVertexEndIndex{0}
        , fStrikeSpec{strikeSpec}
        , fStrike{nullptr}
        , fColor{textBlob->fColor}
        , fPaths{} { }

class GrTextBlob::SubRun* GrTextBlob::makeSubRun(SubRunType type,
                                                 const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                                 const SkStrikeSpec& strikeSpec,
                                                 GrMaskFormat format) {
    bool hasW = this->hasW(type);
    uint32_t glyphsStart = fGlyphsCursor;
    fGlyphsCursor += drawables.size();
    uint32_t glyphsEnd = fGlyphsCursor;
    size_t verticesStart = fVerticesCursor;
    fVerticesCursor += drawables.size() * GetVertexStride(format, hasW) * kVerticesPerGlyph;
    size_t verticesEnd = fVerticesCursor;

    SubRunBufferSpec bufferSpec = std::make_tuple(
            glyphsStart, glyphsEnd, verticesStart, verticesEnd);

    sk_sp<GrTextStrike> grStrike = strikeSpec.findOrCreateGrStrike(fStrikeCache);

    SubRun& subRun = fSubRuns.emplace_back(
            type, this, strikeSpec, format, bufferSpec, std::move(grStrike));

    subRun.appendGlyphs(drawables);

    return &subRun;
}

void GrTextBlob::SubRun::appendGlyphs(const SkZip<SkGlyphVariant, SkPoint>& drawables) {
    GrTextStrike* grStrike = fStrike.get();
    SkScalar strikeToSource = fStrikeSpec.strikeToSourceRatio();
    uint32_t glyphCursor = fGlyphStartIndex;
    size_t vertexCursor = fVertexStartIndex;
    bool hasW = this->hasW();
    GrColor color = this->color();
    // glyphs drawn in perspective must always have a w coord.
    SkASSERT(hasW || !fBlob->fInitialViewMatrix.hasPerspective());
    size_t vertexStride = GetVertexStride(fMaskFormat, hasW);
    // We always write the third position component used by SDFs. If it is unused it gets
    // overwritten. Similarly, we always write the color and the blob will later overwrite it
    // with texture coords if it is unused.
    size_t colorOffset = hasW ? sizeof(SkPoint3) : sizeof(SkPoint);
    for (auto [variant, pos] : drawables) {
        SkGlyph* skGlyph = variant;
        GrGlyph* grGlyph = grStrike->getGlyph(*skGlyph);
        // Only floor the device coordinates.
        SkRect dstRect;
        if (!this->needsTransform()) {
            pos = {SkScalarFloorToScalar(pos.x()), SkScalarFloorToScalar(pos.y())};
            dstRect = grGlyph->destRect(pos);
        } else {
            dstRect = grGlyph->destRect(pos, strikeToSource);
        }

        this->joinGlyphBounds(dstRect);

        intptr_t vertex = reinterpret_cast<intptr_t>(fBlob->fVertices + vertexCursor);

        // V0
        *reinterpret_cast<SkPoint3*>(vertex) = {dstRect.fLeft, dstRect.fTop, 1.f};
        *reinterpret_cast<GrColor*>(vertex + colorOffset) = color;
        vertex += vertexStride;

        // V1
        *reinterpret_cast<SkPoint3*>(vertex) = {dstRect.fLeft, dstRect.fBottom, 1.f};
        *reinterpret_cast<GrColor*>(vertex + colorOffset) = color;
        vertex += vertexStride;

        // V2
        *reinterpret_cast<SkPoint3*>(vertex) = {dstRect.fRight, dstRect.fTop, 1.f};
        *reinterpret_cast<GrColor*>(vertex + colorOffset) = color;
        vertex += vertexStride;

        // V3
        *reinterpret_cast<SkPoint3*>(vertex) = {dstRect.fRight, dstRect.fBottom, 1.f};
        *reinterpret_cast<GrColor*>(vertex + colorOffset) = color;

        vertexCursor += vertexStride * kVerticesPerGlyph;
        fBlob->fGlyphs[glyphCursor++] = grGlyph;
    }
    SkASSERT(glyphCursor == fGlyphEndIndex);
    SkASSERT(vertexCursor == fVertexEndIndex);
}

void GrTextBlob::addSingleMaskFormat(
        SubRunType type,
        const SkZip<SkGlyphVariant, SkPoint>& drawables,
        const SkStrikeSpec& strikeSpec,
        GrMaskFormat format) {
    this->makeSubRun(type, drawables, strikeSpec, format);
}

void GrTextBlob::addMultiMaskFormat(
        SubRunType type,
        const SkZip<SkGlyphVariant, SkPoint>& drawables,
        const SkStrikeSpec& strikeSpec) {
    this->setHasBitmap();
    if (drawables.empty()) { return; }

    auto glyphSpan = drawables.get<0>();
    SkGlyph* glyph = glyphSpan[0];
    GrMaskFormat format = GrGlyph::FormatFromSkGlyph(glyph->maskFormat());
    size_t startIndex = 0;
    for (size_t i = 1; i < drawables.size(); i++) {
        glyph = glyphSpan[i];
        GrMaskFormat nextFormat = GrGlyph::FormatFromSkGlyph(glyph->maskFormat());
        if (format != nextFormat) {
            auto sameFormat = drawables.subspan(startIndex, i - startIndex);
            this->addSingleMaskFormat(type, sameFormat, strikeSpec, format);
            format = nextFormat;
            startIndex = i;
        }
    }
    auto sameFormat = drawables.last(drawables.size() - startIndex);
    this->addSingleMaskFormat(type, sameFormat, strikeSpec, format);
}

void GrTextBlob::addSDFT(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                         const SkStrikeSpec& strikeSpec,
                         const SkFont& runFont,
                         SkScalar minScale,
                         SkScalar maxScale) {
    this->setHasDistanceField();
    this->setMinAndMaxScale(minScale, maxScale);

    SubRun* subRun = this->makeSubRun(kTransformedSDFT, drawables, strikeSpec, kA8_GrMaskFormat);
    subRun->setUseLCDText(runFont.getEdging() == SkFont::Edging::kSubpixelAntiAlias);
    subRun->setAntiAliased(runFont.hasSomeAntiAliasing());
}

void GrTextBlob::processDeviceMasks(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    const SkStrikeSpec& strikeSpec) {
    this->addMultiMaskFormat(kDirectMask, drawables, strikeSpec);
}

void GrTextBlob::processSourcePaths(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    const SkFont& runFont,
                                    const SkStrikeSpec& strikeSpec) {
    this->setHasBitmap();
    SubRun& subRun = fSubRuns.emplace_back(this, strikeSpec);
    subRun.setAntiAliased(runFont.hasSomeAntiAliasing());
    for (auto [variant, pos] : drawables) {
        subRun.fPaths.emplace_back(*variant.path(), pos);
    }
}

void GrTextBlob::processSourceSDFT(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                   const SkStrikeSpec& strikeSpec,
                                   const SkFont& runFont,
                                   SkScalar minScale,
                                   SkScalar maxScale) {
    this->addSDFT(drawables, strikeSpec, runFont, minScale, maxScale);
}

void GrTextBlob::processSourceMasks(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    const SkStrikeSpec& strikeSpec) {
    this->addMultiMaskFormat(kTransformedMask, drawables, strikeSpec);
}

#if GR_TEST_UTILS

#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"

std::unique_ptr<GrDrawOp> GrTextContext::createOp_TestingOnly(GrRecordingContext* context,
                                                              GrTextContext* textContext,
                                                              GrRenderTargetContext* rtc,
                                                              const SkPaint& skPaint,
                                                              const SkFont& font,
                                                              const SkMatrix& viewMatrix,
                                                              const char* text,
                                                              int x,
                                                              int y) {
    auto direct = context->priv().asDirectContext();
    if (!direct) {
        return nullptr;
    }

    auto strikeCache = direct->priv().getGrStrikeCache();

    static SkSurfaceProps surfaceProps(SkSurfaceProps::kLegacyFontHost_InitType);

    size_t textLen = (int)strlen(text);

    SkPMColor4f filteredColor = generate_filtered_color(skPaint, rtc->colorInfo());
    GrColor color = filteredColor.toBytes_RGBA();

    auto origin = SkPoint::Make(x, y);
    SkGlyphRunBuilder builder;
    builder.drawTextUTF8(skPaint, font, text, textLen, origin);

    auto glyphRunList = builder.useGlyphRunList();
    sk_sp<GrTextBlob> blob;
    if (!glyphRunList.empty()) {
        blob = direct->priv().getTextBlobCache()->makeBlob(
                glyphRunList, strikeCache, viewMatrix, color, false);
        blob->generateFromGlyphRunList(
                *context->priv().caps()->shaderCaps(), textContext->fOptions,
                skPaint, viewMatrix, surfaceProps,
                glyphRunList, rtc->textTarget()->glyphPainter());
    }

    return blob->test_makeOp(textLen, viewMatrix, x, y, skPaint, filteredColor, surfaceProps,
                             textContext->dfAdjustTable(), rtc->textTarget());
}

#endif  // GR_TEST_UTILS
#endif  // SK_SUPPORT_GPU

SkGlyphRunListPainter::ScopedBuffers::ScopedBuffers(SkGlyphRunListPainter* painter, size_t size)
        : fPainter{painter} {
    fPainter->fDrawable.ensureSize(size);
}

SkGlyphRunListPainter::ScopedBuffers::~ScopedBuffers() {
    fPainter->fDrawable.reset();
    fPainter->fRejects.reset();
}

SkVector SkGlyphPositionRoundingSpec::HalfAxisSampleFreq(bool isSubpixel, SkAxisAlignment axisAlignment) {
    if (!isSubpixel) {
        return {SK_ScalarHalf, SK_ScalarHalf};
    } else {
        switch (axisAlignment) {
            case kX_SkAxisAlignment:
                return {SkPackedGlyphID::kSubpixelRound, SK_ScalarHalf};
            case kY_SkAxisAlignment:
                return {SK_ScalarHalf, SkPackedGlyphID::kSubpixelRound};
            case kNone_SkAxisAlignment:
                return {SkPackedGlyphID::kSubpixelRound, SkPackedGlyphID::kSubpixelRound};
        }
    }

    // Some compilers need this.
    return {0, 0};
}

SkIPoint SkGlyphPositionRoundingSpec::IgnorePositionMask(
        bool isSubpixel, SkAxisAlignment axisAlignment) {
    return SkIPoint::Make((!isSubpixel || axisAlignment == kY_SkAxisAlignment) ? 0 : ~0,
                          (!isSubpixel || axisAlignment == kX_SkAxisAlignment) ? 0 : ~0);
}

SkIPoint SkGlyphPositionRoundingSpec::IgnorePositionFieldMask(bool isSubpixel,
                                                              SkAxisAlignment axisAlignment) {
    SkIPoint ignoreMask = IgnorePositionMask(isSubpixel, axisAlignment);
    SkIPoint answer{ignoreMask.x() & SkPackedGlyphID::kXYFieldMask.x(),
                    ignoreMask.y() & SkPackedGlyphID::kXYFieldMask.y()};
    return answer;
}

SkGlyphPositionRoundingSpec::SkGlyphPositionRoundingSpec(
        bool isSubpixel,SkAxisAlignment axisAlignment)
    : halfAxisSampleFreq{HalfAxisSampleFreq(isSubpixel, axisAlignment)}
    , ignorePositionMask{IgnorePositionMask(isSubpixel, axisAlignment)}
    , ignorePositionFieldMask {IgnorePositionFieldMask(isSubpixel, axisAlignment)}{ }
