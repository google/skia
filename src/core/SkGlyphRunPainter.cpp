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

    const SkPaint& runPaint = glyphRunList.paint();
    // The bitmap blitters can only draw lcd text to a N32 bitmap in srcOver. Otherwise,
    // convert the lcd text into A8 text. The props communicates this to the scaler.
    auto& props = (kN32_SkColorType == fColorType && runPaint.isSrcOver())
                  ? fDeviceProps
                  : fBitmapFallbackProps;

    SkPoint origin = glyphRunList.origin();
    for (auto& glyphRun : glyphRunList) {
        const SkFont& runFont = glyphRun.font();

        if (SkStrikeSpec::ShouldDrawAsPath(runPaint, runFont, deviceMatrix)) {

            SkStrikeSpec strikeSpec = SkStrikeSpec::MakePath(
                    runFont, runPaint, props, fScalerContextFlags);

            auto strike = strikeSpec.findOrCreateExclusiveStrike();

            fDrawable.startSource(glyphRun.source(), origin);
            strike->prepareForDrawingPathsCPU(&fDrawable);

            // The paint we draw paths with must have the same anti-aliasing state as the runFont
            // allowing the paths to have the same edging as the glyph masks.
            SkPaint pathPaint = runPaint;
            pathPaint.setAntiAlias(runFont.hasSomeAntiAliasing());

            bitmapDevice->paintPaths(&fDrawable, strikeSpec.strikeToSourceRatio(), pathPaint);
        } else {
            SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
                    runFont, runPaint, props, fScalerContextFlags, deviceMatrix);

            auto strike = strikeSpec.findOrCreateExclusiveStrike();

            fDrawable.startDevice(glyphRun.source(), origin, deviceMatrix, strike->roundingSpec());
            strike->prepareForDrawingMasksCPU(&fDrawable);
            bitmapDevice->paintMasks(&fDrawable, runPaint);
        }
    }
}

// Getting glyphs to the screen in a fallback situation can be complex. Here is the set of
// transformations that have to happen. Normally, they would all be accommodated by the font
// scaler, but the atlas has an upper limit to the glyphs it can handle. So the GPU is used to
// make up the difference from the smaller atlas size to the larger size needed by the final
// transform. Here are the transformations that are applied.
//
// final transform = [view matrix] * [text scale] * [text size]
//
// There are three cases:
// * Go Fast - view matrix is scale and translate, and all the glyphs are small enough
//   Just scale the positions, and have the glyph cache handle the view matrix transformation.
//   The text scale is 1.
// * It's complicated - view matrix is not scale and translate, and the glyphs are small enough
//   The glyph cache does not handle the view matrix, but stores the glyphs at the text size
//   specified by the run paint. The GPU handles the rotation, etc. specified by the view matrix.
//   The text scale is 1.
// * Too big - The glyphs are too big to fit in the atlas
//   Reduce the text size so the glyphs will fit in the atlas, but don't apply any
//   transformations from the view matrix. Calculate a text scale based on that reduction. This
//   scale factor is used to increase the size of the destination rectangles. The destination
//   rectangles are then scaled, rotated, etc. by the GPU using the view matrix.
void SkGlyphRunListPainter::processARGBFallback(SkScalar maxSourceGlyphDimension,
                                                const SkPaint& runPaint,
                                                const SkFont& runFont,
                                                SkPoint origin,
                                                const SkMatrix& viewMatrix,
                                                SkGlyphRunPainterInterface* process) {
    // if maxSourceGlyphDimension then no pixels will change.
    if (maxSourceGlyphDimension == 0) { return; }

    SkScalar maxScale = viewMatrix.getMaxScale();

    // This is a linear estimate of the longest dimension among all the glyph widths and heights.
    SkScalar conservativeMaxGlyphDimension = maxSourceGlyphDimension * maxScale;

    // If the situation that the matrix is simple, and all the glyphs are small enough. Go fast!
    // N.B. If the matrix has scale, that will be reflected in the strike through the viewMatrix
    // in the useFastPath case.
    bool useDeviceCache =
            viewMatrix.isScaleTranslate()
            && conservativeMaxGlyphDimension <= SkStrikeCommon::kSkSideTooBigForAtlas;

    // A scaled and translated transform is the common case, and is handled directly in fallback.
    // Even if the transform is scale and translate, fallback must be careful to use glyphs that
    // fit in the atlas. If a glyph will not fit in the atlas, then the general transform case is
    // used to render the glyphs.
    if (useDeviceCache) {
        SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
                runFont, runPaint, fDeviceProps, fScalerContextFlags, viewMatrix);

        SkScopedStrikeForGPU strike = strikeSpec.findOrCreateScopedStrike(fStrikeCache);

        fDrawable.startDevice(fRejects.source(), origin, viewMatrix, strike->roundingSpec());

        strike->prepareForDrawing(
                SkStrikeCommon::kSkSideTooBigForAtlas, &fDrawable);

        if (process) {
            process->processDeviceFallback(fDrawable.drawable(), strikeSpec);
        }

    } else {
        // If the matrix is complicated or if scaling is used to fit the glyphs in the cache,
        // then this case is used.

        SkStrikeSpec strikeSpec = SkStrikeSpec::MakeSourceFallback(
                runFont, runPaint, fDeviceProps, fScalerContextFlags, maxSourceGlyphDimension);

        SkScopedStrikeForGPU strike = strikeSpec.findOrCreateScopedStrike(fStrikeCache);

        fDrawable.startSource(fRejects.source(), origin);

        strike->prepareForDrawing(
                SkStrikeCommon::kSkSideTooBigForAtlas, &fDrawable);

        if (process) {
            process->processSourceFallback(
                    fDrawable.drawable(),
                    strikeSpec,
                    viewMatrix.hasPerspective());
        }
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
        fPaths.clear();
        const SkFont& runFont = glyphRun.font();


        bool useSDFT = GrTextContext::CanDrawAsDistanceFields(
                runPaint, runFont, viewMatrix, props, contextSupportsDistanceFieldText, options);
        if (process) {
            process->startRun(glyphRun, useSDFT);
        }

        if (useSDFT) {
            SkScalar minScale, maxScale;
            SkStrikeSpec strikeSpec;
            std::tie(strikeSpec, minScale, maxScale) =
                    SkStrikeSpec::MakeSDFT(
                            runFont, runPaint,fDeviceProps, viewMatrix, options);

            SkScopedStrikeForGPU strike = strikeSpec.findOrCreateScopedStrike(fStrikeCache);

            fDrawable.startSource(fRejects.source(), origin);
            strike->prepareForDrawing(SkStrikeCommon::kSkSideTooBigForAtlas, &fDrawable);

            fDrawable.flipDrawableToInput();
            for (auto t : SkMakeEnumerate(fDrawable.input())) {
                size_t i; SkGlyphVariant glyphVariant; SkPoint pos;
                std::forward_as_tuple(i, std::tie(glyphVariant, pos)) = t;
                const SkGlyph& glyph = *glyphVariant.glyph();

                // The SDF scaler context system ensures that a glyph is empty, kSDF_Format, or
                // kARGB32_Format. The following if statements use this assumption.
                SkASSERT(glyph.maskFormat() == SkMask::kSDF_Format
                         || glyph.isColor()
                         || glyph.isEmpty());

                if (!glyph.isEmpty()) {
                    if (SkStrikeForGPU::CanDrawAsSDFT(glyph)) {
                        // SDF mask will work.
                        fDrawable.push_back(i);
                    } else if (SkStrikeForGPU::CanDrawAsPath(glyph)) {
                        // If not color but too big, use a path.
                        fPaths.push_back(SkGlyphPos{i, &glyph, pos});
                    } else {
                        // If no path, or it is color, then fallback.
                        fRejects.reject(i, glyph.maxDimension());
                    }
                }
            }

            fRejects.flipRejectsToSource();

            if (process) {
                bool hasWCoord =
                        viewMatrix.hasPerspective() || options.fDistanceFieldVerticesAlwaysHaveW;

                // processSourceSDFT must be called even if there are no glyphs to make sure runs
                // are set correctly.
                process->processSourceSDFT(
                        fDrawable.drawable(),
                        strikeSpec,
                        runFont,
                        minScale,
                        maxScale,
                        hasWCoord);

                if (!fPaths.empty()) {
                    process->processSourcePaths(SkMakeSpan(fPaths), strikeSpec);
                }
            }

            // fGlyphPos will be reused here.
            if (!fRejects.source().empty()) {
                this->processARGBFallback(
                        fRejects.rejectedMaxDimension() * strikeSpec.strikeToSourceRatio(),
                        runPaint, runFont, origin, viewMatrix, process);
            }
        } else if (SkStrikeSpec::ShouldDrawAsPath(runPaint, runFont, viewMatrix)) {
            SkStrikeSpec strikeSpec = SkStrikeSpec::MakePath(
                            runFont, runPaint, fDeviceProps, fScalerContextFlags);

            SkScopedStrikeForGPU strike = strikeSpec.findOrCreateScopedStrike(fStrikeCache);

            fDrawable.startSource(fRejects.source(), origin);
            strike->prepareForDrawing(0, &fDrawable);

            fDrawable.flipDrawableToInput();
            for (auto t : SkMakeEnumerate(fDrawable.input())) {
                size_t i; SkGlyphVariant glyphVariant; SkPoint pos;
                std::forward_as_tuple(i, std::tie(glyphVariant, pos)) = t;
                const SkGlyph& glyph = *glyphVariant.glyph();
                if (!glyph.isEmpty()) {
                    if (SkStrikeForGPU::CanDrawAsPath(glyph)) {
                        // Place paths in fGlyphPos
                        fPaths.push_back(SkGlyphPos{i, &glyph, pos});
                    } else {
                        fRejects.reject(i, glyph.maxDimension());
                    }
                }
            }

            fRejects.flipRejectsToSource();

            if (process) {
                // processSourcePaths must be called even if there are no glyphs to make sure runs
                // are set correctly.
                process->processSourcePaths(SkMakeSpan(fPaths), strikeSpec);
            }

            // fGlyphPos will be reused here.
            if (!fRejects.source().empty()) {
                this->processARGBFallback(
                        fRejects.rejectedMaxDimension() * strikeSpec.strikeToSourceRatio(),
                        runPaint, runFont, origin, viewMatrix, process);
            }
        } else {
            SkStrikeSpec strikeSpec =
                    SkStrikeSpec::MakeMask(runFont, runPaint,
                            fDeviceProps, fScalerContextFlags, viewMatrix);

            SkScopedStrikeForGPU strike = strikeSpec.findOrCreateScopedStrike(fStrikeCache);

            fDrawable.startDevice(fRejects.source(), origin, viewMatrix, strike->roundingSpec());

            strike->prepareForDrawing(
                    SkStrikeCommon::kSkSideTooBigForAtlas, &fDrawable);

            // Sort glyphs into the three bins: mask (fGlyphPos), path (fPaths), and fallback.
            fDrawable.flipDrawableToInput();
            for (auto t : SkMakeEnumerate(fDrawable.input())) {
                size_t i; SkGlyphVariant glyphVariant; SkPoint pos;
                std::forward_as_tuple(i, std::tie(glyphVariant, pos)) = t;
                SkGlyph* glyph = glyphVariant.glyph();
                if (!glyph->isEmpty()) {
                    // Does the glyph have work to do or is the code able to position the glyph?
                    if (!SkScalarsAreFinite(pos.x(), pos.y())) {
                        // Do nothing;
                    } else if (SkStrikeForGPU::CanDrawAsMask(*glyph)) {
                        fDrawable.push_back(i);
                    } else if (SkStrikeForGPU::CanDrawAsPath(*glyph)) {
                        fPaths.push_back(SkGlyphPos{i, glyph, pos});
                    } else {
                        fRejects.reject(i, glyph->maxDimension());
                    }
                }
            }

            fRejects.flipRejectsToSource();

            if (process) {
                // processDeviceMasks must be called even if there are no glyphs to make sure runs
                // are set correctly.
                process->processDeviceMasks(fDrawable.drawable(), strikeSpec);
                if (!fPaths.empty()) {
                    process->processDevicePaths(SkMakeSpan(fPaths));
                }
            }

            // fGlyphPos will be reused here.
            if (!fRejects.source().empty()) {
                this->processARGBFallback(
                        fRejects.rejectedMaxDimension() / viewMatrix.getMaxScale(),
                        runPaint, runFont, origin, viewMatrix, process);
            }
        }  // Mask case
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

    if (cacheBlob) {
        if (cacheBlob->mustRegenerate(listPaint, glyphRunList.anyRunsSubpixelPositioned(),
                                      blurRec, viewMatrix, origin.x(),origin.y())) {
            // We have to remake the blob because changes may invalidate our masks.
            // TODO we could probably get away reuse most of the time if the pointer is unique,
            // but we'd have to clear the subrun information
            textBlobCache->remove(cacheBlob.get());
            cacheBlob = textBlobCache->makeCachedBlob(
                    glyphRunList, key, blurRec, listPaint, color, grStrikeCache);
            cacheBlob->generateFromGlyphRunList(
                    *context->priv().caps()->shaderCaps(), fOptions,
                    listPaint, scalerContextFlags, viewMatrix, props,
                    glyphRunList, target->glyphPainter());
        } else {
            textBlobCache->makeMRU(cacheBlob.get());

            if (CACHE_SANITY_CHECK) {
                sk_sp<GrTextBlob> sanityBlob(textBlobCache->makeBlob(
                        glyphRunList, color, grStrikeCache));
                sanityBlob->setupKey(key, blurRec, listPaint);
                cacheBlob->generateFromGlyphRunList(
                        *context->priv().caps()->shaderCaps(), fOptions,
                        listPaint, scalerContextFlags, viewMatrix, props, glyphRunList,
                        target->glyphPainter());
                GrTextBlob::AssertEqual(*sanityBlob, *cacheBlob);
            }
        }
    } else {
        if (canCache) {
            cacheBlob = textBlobCache->makeCachedBlob(
                    glyphRunList, key, blurRec, listPaint, color, grStrikeCache);
        } else {
            cacheBlob = textBlobCache->makeBlob(glyphRunList, color, grStrikeCache);
        }
        cacheBlob->generateFromGlyphRunList(
                *context->priv().caps()->shaderCaps(), fOptions, listPaint,
                scalerContextFlags, viewMatrix, props, glyphRunList,
                target->glyphPainter());
    }

    cacheBlob->flush(target, props, fDistanceAdjustTable.get(), listPaint, filteredColor,
                     clip, viewMatrix, origin.x(), origin.y());
}

void GrTextBlob::SubRun::appendGlyph(GrGlyph* glyph, SkRect dstRect) {

    this->joinGlyphBounds(dstRect);

    GrTextBlob* blob = fRun->fBlob;

    bool hasW = this->hasWCoord();
    // glyphs drawn in perspective must always have a w coord.
    SkASSERT(hasW || !blob->fInitialViewMatrix.hasPerspective());
    auto maskFormat = this->maskFormat();
    size_t vertexStride = GetVertexStride(maskFormat, hasW);

    intptr_t vertex = reinterpret_cast<intptr_t>(blob->fVertices + fVertexEndIndex);

    // We always write the third position component used by SDFs. If it is unused it gets
    // overwritten. Similarly, we always write the color and the blob will later overwrite it
    // with texture coords if it is unused.
    size_t colorOffset = hasW ? sizeof(SkPoint3) : sizeof(SkPoint);
    // V0
    *reinterpret_cast<SkPoint3*>(vertex) = {dstRect.fLeft, dstRect.fTop, 1.f};
    *reinterpret_cast<GrColor*>(vertex + colorOffset) = fColor;
    vertex += vertexStride;

    // V1
    *reinterpret_cast<SkPoint3*>(vertex) = {dstRect.fLeft, dstRect.fBottom, 1.f};
    *reinterpret_cast<GrColor*>(vertex + colorOffset) = fColor;
    vertex += vertexStride;

    // V2
    *reinterpret_cast<SkPoint3*>(vertex) = {dstRect.fRight, dstRect.fTop, 1.f};
    *reinterpret_cast<GrColor*>(vertex + colorOffset) = fColor;
    vertex += vertexStride;

    // V3
    *reinterpret_cast<SkPoint3*>(vertex) = {dstRect.fRight, dstRect.fBottom, 1.f};
    *reinterpret_cast<GrColor*>(vertex + colorOffset) = fColor;

    fVertexEndIndex += vertexStride * kVerticesPerGlyph;
    blob->fGlyphs[fGlyphEndIndex++] = glyph;
}

void GrTextBlob::Run::switchSubRunIfNeededAndAppendGlyph(GrGlyph* glyph,
                                                         const sk_sp<GrTextStrike>& strike,
                                                         const SkRect& destRect,
                                                         bool needsTransform) {
    GrMaskFormat format = glyph->fMaskFormat;

    SubRun* subRun = &fSubRunInfo.back();
    if (fInitialized && subRun->maskFormat() != format) {
        subRun = pushBackSubRun(fStrikeSpec, fColor);
        subRun->setStrike(strike);
    } else if (!fInitialized) {
        subRun->setStrike(strike);
    }

    fInitialized = true;
    subRun->setMaskFormat(format);
    subRun->setNeedsTransform(needsTransform);
    subRun->appendGlyph(glyph, destRect);
}

void GrTextBlob::Run::appendDeviceSpaceGlyph(const sk_sp<GrTextStrike>& strike,
                                             const SkGlyph& skGlyph, SkPoint origin) {
    if (GrGlyph* glyph = strike->getGlyph(skGlyph)) {

        SkRect glyphRect = glyph->destRect(origin);

        if (!glyphRect.isEmpty()) {
            this->switchSubRunIfNeededAndAppendGlyph(glyph, strike, glyphRect, false);
        }
    }
}

void GrTextBlob::Run::appendSourceSpaceGlyph(const sk_sp<GrTextStrike>& strike,
                                             const SkGlyph& skGlyph,
                                             SkPoint origin,
                                             SkScalar textScale) {
    if (GrGlyph* glyph = strike->getGlyph(skGlyph)) {

        SkRect glyphRect = glyph->destRect(origin, textScale);

        if (!glyphRect.isEmpty()) {
            this->switchSubRunIfNeededAndAppendGlyph(glyph, strike, glyphRect, true);
        }
    }
}

void GrTextBlob::generateFromGlyphRunList(const GrShaderCaps& shaderCaps,
                                          const GrTextContext::Options& options,
                                          const SkPaint& paint,
                                          SkScalerContextFlags scalerContextFlags,
                                          const SkMatrix& viewMatrix,
                                          const SkSurfaceProps& props,
                                          const SkGlyphRunList& glyphRunList,
                                          SkGlyphRunListPainter* glyphPainter) {
    SkPoint origin = glyphRunList.origin();
    const SkPaint& runPaint = glyphRunList.paint();
    this->initReusableBlob(SkPaintPriv::ComputeLuminanceColor(runPaint), viewMatrix,
                           origin.x(), origin.y());

    glyphPainter->processGlyphRunList(glyphRunList,
                                      viewMatrix,
                                      props,
                                      shaderCaps.supportsDistanceFieldText(),
                                      options,
                                      this);
}

GrTextBlob::Run* GrTextBlob::currentRun() {
    return &fRuns[fRunCount - 1];
}

void GrTextBlob::startRun(const SkGlyphRun& glyphRun, bool useSDFT) {
    if (useSDFT) {
        this->setHasDistanceField();
    }
    Run* run = this->pushBackRun();
    run->setRunFontAntiAlias(glyphRun.font().hasSomeAntiAliasing());
}

void GrTextBlob::processDeviceMasks(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    const SkStrikeSpec& strikeSpec) {
    Run* run = this->currentRun();
    this->setHasBitmap();
    run->setupFont(strikeSpec);
    sk_sp<GrTextStrike> currStrike = strikeSpec.findOrCreateGrStrike(fStrikeCache);
    for (auto t : drawables) {
        SkGlyph* glyph; SkPoint pos;
        std::tie(glyph, pos) = t;
        SkPoint pt{SkScalarFloorToScalar(pos.fX),
                   SkScalarFloorToScalar(pos.fY)};
        run->appendDeviceSpaceGlyph(currStrike, *glyph, pt);
    }
}

void GrTextBlob::processSourcePaths(SkSpan<const SkGlyphPos> paths,
                                    const SkStrikeSpec& strikeSpec) {
    Run* run = this->currentRun();
    this->setHasBitmap();
    run->setupFont(strikeSpec);
    for (const auto& path : paths) {
        if (const SkPath* glyphPath = path.glyph->path()) {
            run->appendPathGlyph(*glyphPath, path.position, strikeSpec.strikeToSourceRatio(),
                                 false);
        }
    }
}

void GrTextBlob::processDevicePaths(SkSpan<const SkGlyphPos> paths) {
    Run* run = this->currentRun();
    this->setHasBitmap();
    for (const auto& path : paths) {
        SkPoint pt{SkScalarFloorToScalar(path.position.fX),
                   SkScalarFloorToScalar(path.position.fY)};
        // TODO: path should always be set. Remove when proven.
        if (const SkPath* glyphPath = path.glyph->path()) {
            run->appendPathGlyph(*glyphPath, pt, SK_Scalar1, true);
        }
    }
}

void GrTextBlob::processSourceSDFT(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                   const SkStrikeSpec& strikeSpec,
                                   const SkFont& runFont,
                                   SkScalar minScale,
                                   SkScalar maxScale,
                                   bool hasWCoord) {

    Run* run = this->currentRun();
    run->setSubRunHasDistanceFields(
            runFont.getEdging() == SkFont::Edging::kSubpixelAntiAlias,
            runFont.hasSomeAntiAliasing(),
            hasWCoord);
    this->setMinAndMaxScale(minScale, maxScale);
    run->setupFont(strikeSpec);
    sk_sp<GrTextStrike> currStrike = strikeSpec.findOrCreateGrStrike(fStrikeCache);
    for (auto t : drawables) {
        SkGlyph* glyph; SkPoint pos;
        std::tie(glyph, pos) = t;
        run->appendSourceSpaceGlyph(currStrike, *glyph, pos, strikeSpec.strikeToSourceRatio());
    }
}

void GrTextBlob::processSourceFallback(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                       const SkStrikeSpec& strikeSpec,
                                       bool hasW) {
    Run* run = this->currentRun();

    auto subRun = run->initARGBFallback();
    sk_sp<GrTextStrike> grStrike = strikeSpec.findOrCreateGrStrike(fStrikeCache);
    subRun->setStrike(grStrike);
    subRun->setHasWCoord(hasW);

    this->setHasBitmap();
    run->setupFont(strikeSpec);
    for (auto t : drawables) {
        SkGlyph* glyph; SkPoint pos;
        std::tie(glyph, pos) = t;
        run->appendSourceSpaceGlyph(grStrike, *glyph, pos, strikeSpec.strikeToSourceRatio());
    }
}

void GrTextBlob::processDeviceFallback(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                       const SkStrikeSpec& strikeSpec) {
    Run* run = this->currentRun();
    this->setHasBitmap();
    sk_sp<GrTextStrike> grStrike = strikeSpec.findOrCreateGrStrike(fStrikeCache);
    auto subRun = run->initARGBFallback();
    run->setupFont(strikeSpec);
    subRun->setStrike(grStrike);
    for (auto t : drawables) {
        SkGlyph* glyph; SkPoint pos;
        std::tie(glyph, pos) = t;
        SkPoint pt{SkScalarFloorToScalar(pos.fX),
                   SkScalarFloorToScalar(pos.fY)};
        run->appendDeviceSpaceGlyph(grStrike, *glyph, pt);
    }
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
        blob = direct->priv().getTextBlobCache()->makeBlob(glyphRunList, color, strikeCache);
        // Use the text and textLen below, because we don't want to mess with the paint.
        SkScalerContextFlags scalerContextFlags = ComputeScalerContextFlags(rtc->colorInfo());
        blob->generateFromGlyphRunList(
                *context->priv().caps()->shaderCaps(), textContext->fOptions,
                skPaint, scalerContextFlags, viewMatrix, surfaceProps,
                glyphRunList, rtc->textTarget()->glyphPainter());
    }

    return blob->test_makeOp(textLen, 0, 0, viewMatrix, x, y, skPaint, filteredColor, surfaceProps,
                             textContext->dfAdjustTable(), rtc->textTarget());
}

#endif  // GR_TEST_UTILS
#endif  // SK_SUPPORT_GPU

SkGlyphRunListPainter::ScopedBuffers::ScopedBuffers(SkGlyphRunListPainter* painter, size_t size)
        : fPainter{painter} {
    fPainter->fDrawable.ensureSize(size);
    if (fPainter->fMaxRunSize < size) {
        fPainter->fMaxRunSize = size;
    }
}

SkGlyphRunListPainter::ScopedBuffers::~ScopedBuffers() {
    fPainter->fDrawable.reset();
    fPainter->fRejects.reset();
    fPainter->fPaths.clear();

    if (fPainter->fMaxRunSize > 200) {
        fPainter->fMaxRunSize = 0;
        fPainter->fPaths.shrink_to_fit();
    }
}

SkVector SkGlyphPositionRoundingSpec::HalfAxisSampleFreq(bool isSubpixel, SkAxisAlignment axisAlignment) {
    if (!isSubpixel) {
        return {SK_ScalarHalf, SK_ScalarHalf};
    } else {
        static constexpr SkScalar kSubpixelRounding = SkFixedToScalar(SkGlyph::kSubpixelRound);
        switch (axisAlignment) {
            case kX_SkAxisAlignment:
                return {kSubpixelRounding, SK_ScalarHalf};
            case kY_SkAxisAlignment:
                return {SK_ScalarHalf, kSubpixelRounding};
            case kNone_SkAxisAlignment:
                return {kSubpixelRounding, kSubpixelRounding};
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

SkGlyphPositionRoundingSpec::SkGlyphPositionRoundingSpec(bool isSubpixel,
                                                         SkAxisAlignment axisAlignment)
        : halfAxisSampleFreq{HalfAxisSampleFreq(isSubpixel, axisAlignment)}
        , ignorePositionMask{IgnorePositionMask(isSubpixel, axisAlignment)} {
}
