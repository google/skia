/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkDevice.h"

#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPathMeasure.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkShader.h"
#include "include/core/SkVertices.h"
#include "include/private/base/SkTo.h"
#include "include/private/chromium/Slug.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkEnumerate.h"
#include "src/core/SkImageFilterCache.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkLatticeIter.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkMemset.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkTextBlobPriv.h"
#include "src/image/SkImage_Base.h"
#include "src/shaders/SkLocalMatrixShader.h"
#include "src/text/GlyphRun.h"
#include "src/utils/SkPatchUtils.h"

SkBaseDevice::SkBaseDevice(const SkImageInfo& info, const SkSurfaceProps& surfaceProps)
        : fInfo(info)
        , fSurfaceProps(surfaceProps) {
    fDeviceToGlobal.setIdentity();
    fGlobalToDevice.setIdentity();
}

void SkBaseDevice::setDeviceCoordinateSystem(const SkM44& deviceToGlobal,
                                             const SkM44& globalToDevice,
                                             const SkM44& localToDevice,
                                             int bufferOriginX,
                                             int bufferOriginY) {
    fDeviceToGlobal = deviceToGlobal;
    fDeviceToGlobal.normalizePerspective();
    fGlobalToDevice = globalToDevice;
    fGlobalToDevice.normalizePerspective();

    fLocalToDevice = localToDevice;
    fLocalToDevice.normalizePerspective();
    if (bufferOriginX | bufferOriginY) {
        fDeviceToGlobal.preTranslate(bufferOriginX, bufferOriginY);
        fGlobalToDevice.postTranslate(-bufferOriginX, -bufferOriginY);
        fLocalToDevice.postTranslate(-bufferOriginX, -bufferOriginY);
    }
    fLocalToDevice33 = fLocalToDevice.asM33();
    fLocalToDeviceDirty = true;
}

void SkBaseDevice::setGlobalCTM(const SkM44& ctm) {
    fLocalToDevice = ctm;
    fLocalToDevice.normalizePerspective();
    // Map from the global CTM state to this device's coordinate system.
    fLocalToDevice.postConcat(fGlobalToDevice);
    fLocalToDevice33 = fLocalToDevice.asM33();
    fLocalToDeviceDirty = true;
}

bool SkBaseDevice::isPixelAlignedToGlobal() const {
    // pixelAligned is set to the identity + integer translation of the device-to-global matrix.
    // If they are equal then the device is by definition pixel aligned.
    SkM44 pixelAligned = SkM44();
    pixelAligned.setRC(0, 3, SkScalarFloorToScalar(fDeviceToGlobal.rc(0, 3)));
    pixelAligned.setRC(1, 3, SkScalarFloorToScalar(fDeviceToGlobal.rc(1, 3)));
    return pixelAligned == fDeviceToGlobal;
}

SkIPoint SkBaseDevice::getOrigin() const {
    // getOrigin() is deprecated, the old origin has been moved into the fDeviceToGlobal matrix.
    // This extracts the origin from the matrix, but asserts that a more complicated coordinate
    // space hasn't been set of the device. This function can be removed once existing use cases
    // have been updated to use the device-to-global matrix instead or have themselves been removed
    // (e.g. Android's device-space clip regions are going away, and are not compatible with the
    // generalized device coordinate system).
    SkASSERT(this->isPixelAlignedToGlobal());
    return SkIPoint::Make(SkScalarFloorToInt(fDeviceToGlobal.rc(0, 3)),
                          SkScalarFloorToInt(fDeviceToGlobal.rc(1, 3)));
}

SkMatrix SkBaseDevice::getRelativeTransform(const SkBaseDevice& dstDevice) const {
    // To get the transform from this space to the other device's, transform from our space to
    // global and then from global to the other device.
    return (dstDevice.fGlobalToDevice * fDeviceToGlobal).asM33();
}

static inline bool is_int(float x) {
    return x == (float) sk_float_round2int(x);
}

void SkBaseDevice::drawRegion(const SkRegion& region, const SkPaint& paint) {
    const SkMatrix& localToDevice = this->localToDevice();
    bool isNonTranslate = localToDevice.getType() & ~(SkMatrix::kTranslate_Mask);
    bool complexPaint = paint.getStyle() != SkPaint::kFill_Style || paint.getMaskFilter() ||
                        paint.getPathEffect();
    bool antiAlias = paint.isAntiAlias() && (!is_int(localToDevice.getTranslateX()) ||
                                             !is_int(localToDevice.getTranslateY()));
    if (isNonTranslate || complexPaint || antiAlias) {
        SkPath path;
        region.getBoundaryPath(&path);
        path.setIsVolatile(true);
        return this->drawPath(path, paint, true);
    }

    SkRegion::Iterator it(region);
    while (!it.done()) {
        this->drawRect(SkRect::Make(it.rect()), paint);
        it.next();
    }
}

void SkBaseDevice::drawArc(const SkRect& oval, SkScalar startAngle,
                           SkScalar sweepAngle, bool useCenter, const SkPaint& paint) {
    SkPath path;
    bool isFillNoPathEffect = SkPaint::kFill_Style == paint.getStyle() && !paint.getPathEffect();
    SkPathPriv::CreateDrawArcPath(&path, oval, startAngle, sweepAngle, useCenter,
                                  isFillNoPathEffect);
    this->drawPath(path, paint);
}

void SkBaseDevice::drawDRRect(const SkRRect& outer,
                              const SkRRect& inner, const SkPaint& paint) {
    SkPath path;
    path.addRRect(outer);
    path.addRRect(inner);
    path.setFillType(SkPathFillType::kEvenOdd);
    path.setIsVolatile(true);

    this->drawPath(path, paint, true);
}

void SkBaseDevice::drawPatch(const SkPoint cubics[12], const SkColor colors[4],
                             const SkPoint texCoords[4], sk_sp<SkBlender> blender,
                             const SkPaint& paint) {
    SkISize lod = SkPatchUtils::GetLevelOfDetail(cubics, &this->localToDevice());
    auto vertices = SkPatchUtils::MakeVertices(cubics, colors, texCoords, lod.width(), lod.height(),
                                               this->imageInfo().colorSpace());
    if (vertices) {
        this->drawVertices(vertices.get(), std::move(blender), paint);
    }
}

void SkBaseDevice::drawImageLattice(const SkImage* image, const SkCanvas::Lattice& lattice,
                                    const SkRect& dst, SkFilterMode filter, const SkPaint& paint) {
    SkLatticeIter iter(lattice, dst);

    SkRect srcR, dstR;
    SkColor c;
    bool isFixedColor = false;
    const SkImageInfo info = SkImageInfo::Make(1, 1, kBGRA_8888_SkColorType, kUnpremul_SkAlphaType);

    while (iter.next(&srcR, &dstR, &isFixedColor, &c)) {
        // TODO: support this fast-path for GPU images
        if (isFixedColor || (srcR.width() <= 1.0f && srcR.height() <= 1.0f &&
                             image->readPixels(nullptr, info, &c, 4, srcR.fLeft, srcR.fTop))) {
              // Fast draw with drawRect, if this is a patch containing a single color
              // or if this is a patch containing a single pixel.
              if (0 != c || !paint.isSrcOver()) {
                   SkPaint paintCopy(paint);
                   int alpha = SkAlphaMul(SkColorGetA(c), SkAlpha255To256(paint.getAlpha()));
                   paintCopy.setColor(SkColorSetA(c, alpha));
                   this->drawRect(dstR, paintCopy);
              }
        } else {
            this->drawImageRect(image, &srcR, dstR, SkSamplingOptions(filter), paint,
                                SkCanvas::kStrict_SrcRectConstraint);
        }
    }
}

static SkPoint* quad_to_tris(SkPoint tris[6], const SkPoint quad[4]) {
    tris[0] = quad[0];
    tris[1] = quad[1];
    tris[2] = quad[2];

    tris[3] = quad[0];
    tris[4] = quad[2];
    tris[5] = quad[3];

    return tris + 6;
}

void SkBaseDevice::drawAtlas(const SkRSXform xform[],
                             const SkRect tex[],
                             const SkColor colors[],
                             int quadCount,
                             sk_sp<SkBlender> blender,
                             const SkPaint& paint) {
    const int triCount = quadCount << 1;
    const int vertexCount = triCount * 3;
    uint32_t flags = SkVertices::kHasTexCoords_BuilderFlag;
    if (colors) {
        flags |= SkVertices::kHasColors_BuilderFlag;
    }
    SkVertices::Builder builder(SkVertices::kTriangles_VertexMode, vertexCount, 0, flags);

    SkPoint* vPos = builder.positions();
    SkPoint* vTex = builder.texCoords();
    SkColor* vCol = builder.colors();
    for (int i = 0; i < quadCount; ++i) {
        SkPoint tmp[4];
        xform[i].toQuad(tex[i].width(), tex[i].height(), tmp);
        vPos = quad_to_tris(vPos, tmp);

        tex[i].toQuad(tmp);
        vTex = quad_to_tris(vTex, tmp);

        if (colors) {
            SkOpts::memset32(vCol, colors[i], 6);
            vCol += 6;
        }
    }
    this->drawVertices(builder.detach().get(), std::move(blender), paint);
}

void SkBaseDevice::drawEdgeAAQuad(const SkRect& r, const SkPoint clip[4], SkCanvas::QuadAAFlags aa,
                                  const SkColor4f& color, SkBlendMode mode) {
    SkPaint paint;
    paint.setColor4f(color);
    paint.setBlendMode(mode);
    paint.setAntiAlias(aa == SkCanvas::kAll_QuadAAFlags);

    if (clip) {
        // Draw the clip directly as a quad since it's a filled color with no local coords
        SkPath clipPath;
        clipPath.addPoly(clip, 4, true);
        this->drawPath(clipPath, paint);
    } else {
        this->drawRect(r, paint);
    }
}

void SkBaseDevice::drawEdgeAAImageSet(const SkCanvas::ImageSetEntry images[], int count,
                                      const SkPoint dstClips[], const SkMatrix preViewMatrices[],
                                      const SkSamplingOptions& sampling, const SkPaint& paint,
                                      SkCanvas::SrcRectConstraint constraint) {
    SkASSERT(paint.getStyle() == SkPaint::kFill_Style);
    SkASSERT(!paint.getPathEffect());

    SkPaint entryPaint = paint;
    const SkM44 baseLocalToDevice = this->localToDevice44();
    int clipIndex = 0;
    for (int i = 0; i < count; ++i) {
        // TODO: Handle per-edge AA. Right now this mirrors the SkiaRenderer component of Chrome
        // which turns off antialiasing unless all four edges should be antialiased. This avoids
        // seaming in tiled composited layers.
        entryPaint.setAntiAlias(images[i].fAAFlags == SkCanvas::kAll_QuadAAFlags);
        entryPaint.setAlphaf(paint.getAlphaf() * images[i].fAlpha);

        bool needsRestore = false;
        SkASSERT(images[i].fMatrixIndex < 0 || preViewMatrices);
        if (images[i].fMatrixIndex >= 0) {
            this->save();
            this->setLocalToDevice(baseLocalToDevice *
                                   SkM44(preViewMatrices[images[i].fMatrixIndex]));
            needsRestore = true;
        }

        SkASSERT(!images[i].fHasClip || dstClips);
        if (images[i].fHasClip) {
            // Since drawImageRect requires a srcRect, the dst clip is implemented as a true clip
            if (!needsRestore) {
                this->save();
                needsRestore = true;
            }
            SkPath clipPath;
            clipPath.addPoly(dstClips + clipIndex, 4, true);
            this->clipPath(clipPath, SkClipOp::kIntersect, entryPaint.isAntiAlias());
            clipIndex += 4;
        }
        this->drawImageRect(images[i].fImage.get(), &images[i].fSrcRect, images[i].fDstRect,
                            sampling, entryPaint, constraint);
        if (needsRestore) {
            this->restoreLocal(baseLocalToDevice);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkBaseDevice::drawDrawable(SkCanvas* canvas, SkDrawable* drawable, const SkMatrix* matrix) {
    drawable->draw(canvas, matrix);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkBaseDevice::drawSpecial(SkSpecialImage*, const SkMatrix&, const SkSamplingOptions&,
                               const SkPaint&) {}
sk_sp<SkSpecialImage> SkBaseDevice::makeSpecial(const SkBitmap&) { return nullptr; }
sk_sp<SkSpecialImage> SkBaseDevice::makeSpecial(const SkImage*) { return nullptr; }
sk_sp<SkSpecialImage> SkBaseDevice::snapSpecial(const SkIRect&, bool forceCopy) { return nullptr; }
sk_sp<SkSpecialImage> SkBaseDevice::snapSpecialScaled(const SkIRect& subset,
                                                      const SkISize& dstDims) {
    return nullptr;
}
sk_sp<SkSpecialImage> SkBaseDevice::snapSpecial() {
    return this->snapSpecial(SkIRect::MakeWH(this->width(), this->height()));
}

skif::Context SkBaseDevice::createContext(const skif::ContextInfo& ctxInfo) const {
    return skif::Context::MakeRaster(ctxInfo);
}

void SkBaseDevice::drawDevice(SkBaseDevice* device, const SkSamplingOptions& sampling,
                              const SkPaint& paint) {
    sk_sp<SkSpecialImage> deviceImage = device->snapSpecial();
    if (deviceImage) {
        this->drawSpecial(deviceImage.get(), device->getRelativeTransform(*this), sampling, paint);
    }
}

void SkBaseDevice::drawFilteredImage(const skif::Mapping& mapping,
                                     SkSpecialImage* src,
                                     SkColorType colorType,
                                     const SkImageFilter* filter,
                                     const SkSamplingOptions& sampling,
                                     const SkPaint& paint) {
    SkASSERT(!paint.getImageFilter() && !paint.getMaskFilter());

    skif::LayerSpace<SkIRect> targetOutput = mapping.deviceToLayer(
            skif::DeviceSpace<SkIRect>(this->devClipBounds()));

    if (colorType == kUnknown_SkColorType) {
        colorType = kRGBA_8888_SkColorType;
    }

    // getImageFilterCache returns a bare image filter cache pointer that must be ref'ed until the
    // filter's filterImage(ctx) function returns.
    sk_sp<SkImageFilterCache> cache(this->getImageFilterCache());
    skif::Context ctx = this->createContext({mapping,
                                             targetOutput,
                                             skif::FilterResult(sk_ref_sp(src)),
                                             colorType,
                                             this->imageInfo().colorSpace(),
                                             src ? src->props() : this->surfaceProps(),
                                             cache.get()});

    SkIPoint offset;
    sk_sp<SkSpecialImage> result = as_IFB(filter)->filterImage(ctx).imageAndOffset(ctx, &offset);
    if (result) {
        SkMatrix deviceMatrixWithOffset = mapping.layerToDevice();
        deviceMatrixWithOffset.preTranslate(offset.fX, offset.fY);
        this->drawSpecial(result.get(), deviceMatrixWithOffset, sampling, paint);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SkBaseDevice::readPixels(const SkPixmap& pm, int x, int y) {
    return this->onReadPixels(pm, x, y);
}

bool SkBaseDevice::writePixels(const SkPixmap& pm, int x, int y) {
    return this->onWritePixels(pm, x, y);
}

bool SkBaseDevice::onWritePixels(const SkPixmap&, int, int) {
    return false;
}

bool SkBaseDevice::onReadPixels(const SkPixmap&, int x, int y) {
    return false;
}

bool SkBaseDevice::accessPixels(SkPixmap* pmap) {
    SkPixmap tempStorage;
    if (nullptr == pmap) {
        pmap = &tempStorage;
    }
    return this->onAccessPixels(pmap);
}

bool SkBaseDevice::peekPixels(SkPixmap* pmap) {
    SkPixmap tempStorage;
    if (nullptr == pmap) {
        pmap = &tempStorage;
    }
    return this->onPeekPixels(pmap);
}

//////////////////////////////////////////////////////////////////////////////////////////

#include "src/base/SkUtils.h"

static sk_sp<SkShader> make_post_inverse_lm(const SkShader* shader, const SkMatrix& lm) {
     SkMatrix inverse_lm;
    if (!shader || !lm.invert(&inverse_lm)) {
        return nullptr;
    }

#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)  // b/256873449
    // Legacy impl for old concat order. This does not work for arbitrary shader DAGs (when there is
    // no single leaf local matrix).

    // LMs pre-compose. In order to push a post local matrix, we peel off any existing local matrix
    // and set a new local matrix of inverse_lm * prev_local_matrix.
    SkMatrix prev_local_matrix;
    const auto nested_shader = as_SB(shader)->makeAsALocalMatrixShader(&prev_local_matrix);
    if (nested_shader) {
        // unfurl the shader
        shader = nested_shader.get();
    }

    return shader->makeWithLocalMatrix(inverse_lm * prev_local_matrix);
#endif

    return shader->makeWithLocalMatrix(inverse_lm);
}

void SkBaseDevice::drawGlyphRunList(SkCanvas* canvas,
                                    const sktext::GlyphRunList& glyphRunList,
                                    const SkPaint& initialPaint,
                                    const SkPaint& drawingPaint) {
    if (!this->localToDevice().isFinite()) {
        return;
    }

    if (!glyphRunList.hasRSXForm()) {
        this->onDrawGlyphRunList(canvas, glyphRunList, initialPaint, drawingPaint);
    } else {
        this->simplifyGlyphRunRSXFormAndRedraw(canvas, glyphRunList, initialPaint, drawingPaint);
    }
}

void SkBaseDevice::simplifyGlyphRunRSXFormAndRedraw(SkCanvas* canvas,
                                                    const sktext::GlyphRunList& glyphRunList,
                                                    const SkPaint& initialPaint,
                                                    const SkPaint& drawingPaint) {
    for (const sktext::GlyphRun& run : glyphRunList) {
        if (run.scaledRotations().empty()) {
            auto subList = glyphRunList.builder()->makeGlyphRunList(
                    run, drawingPaint, {0, 0});
            this->drawGlyphRunList(canvas, subList, initialPaint, drawingPaint);
        } else {
            SkPoint origin = glyphRunList.origin();
            SkPoint sharedPos{0, 0};    // we're at the origin
            SkGlyphID sharedGlyphID;
            sktext::GlyphRun glyphRun {
                    run.font(),
                    SkSpan<const SkPoint>{&sharedPos, 1},
                    SkSpan<const SkGlyphID>{&sharedGlyphID, 1},
                    SkSpan<const char>{},
                    SkSpan<const uint32_t>{},
                    SkSpan<const SkVector>{}
            };

            for (auto [i, glyphID, pos] : SkMakeEnumerate(run.source())) {
                sharedGlyphID = glyphID;
                auto [scos, ssin] = run.scaledRotations()[i];
                SkRSXform rsxForm = SkRSXform::Make(scos, ssin, pos.x(), pos.y());
                SkMatrix glyphToLocal;
                glyphToLocal.setRSXform(rsxForm).postTranslate(origin.x(), origin.y());

                // We want to rotate each glyph by the rsxform, but we don't want to rotate "space"
                // (i.e. the shader that cares about the ctm) so we have to undo our little ctm
                // trick with a localmatrixshader so that the shader draws as if there was no
                // change to the ctm.
                SkPaint invertingPaint{drawingPaint};
                invertingPaint.setShader(
                        make_post_inverse_lm(drawingPaint.getShader(), glyphToLocal));
                SkAutoCanvasRestore acr(canvas, true);
                canvas->concat(SkM44(glyphToLocal));
                sktext::GlyphRunList subList = glyphRunList.builder()->makeGlyphRunList(
                        glyphRun, drawingPaint, {0, 0});
                this->drawGlyphRunList(canvas, subList, initialPaint, invertingPaint);
            }
        }
    }
}

sk_sp<sktext::gpu::Slug> SkBaseDevice::convertGlyphRunListToSlug(
        const sktext::GlyphRunList& glyphRunList,
        const SkPaint& initialPaint,
        const SkPaint& drawingPaint) {
    return nullptr;
}

void SkBaseDevice::drawSlug(SkCanvas*, const sktext::gpu::Slug*, const SkPaint&) {
    SK_ABORT("Slug drawing not supported.");
}

//////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkSurface> SkBaseDevice::makeSurface(SkImageInfo const&, SkSurfaceProps const&) {
    return nullptr;
}

SkScalerContextFlags SkBaseDevice::scalerContextFlags() const {
    // If we're doing linear blending, then we can disable the gamma hacks.
    // Otherwise, leave them on. In either case, we still want the contrast boost:
    // TODO: Can we be even smarter about mask gamma based on the dest transfer function?
    const SkColorSpace* const cs = fInfo.colorSpace();
    if (cs && cs->gammaIsLinear()) {
        return SkScalerContextFlags::kBoostContrast;
    } else {
        return SkScalerContextFlags::kFakeGammaAndBoostContrast;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

SkNoPixelsDevice::SkNoPixelsDevice(const SkIRect& bounds, const SkSurfaceProps& props)
    : SkNoPixelsDevice(bounds, props, nullptr) {}

SkNoPixelsDevice::SkNoPixelsDevice(const SkIRect& bounds, const SkSurfaceProps& props,
                                   sk_sp<SkColorSpace> colorSpace)
    : SkBaseDevice(SkImageInfo::Make(bounds.size(), kUnknown_SkColorType, kUnknown_SkAlphaType,
                                     std::move(colorSpace)), props) {
    // this fails if we enable this assert: DiscardableImageMapTest.GetDiscardableImagesInRectMaxImage
    //SkASSERT(bounds.width() >= 0 && bounds.height() >= 0);

    this->setOrigin(SkM44(), bounds.left(), bounds.top());
    this->resetClipStack();
}

void SkNoPixelsDevice::onSave() {
    SkASSERT(!fClipStack.empty());
    fClipStack.back().fDeferredSaveCount++;
}

void SkNoPixelsDevice::onRestore() {
    SkASSERT(!fClipStack.empty());
    if (fClipStack.back().fDeferredSaveCount > 0) {
        fClipStack.back().fDeferredSaveCount--;
    } else {
        fClipStack.pop_back();
        SkASSERT(!fClipStack.empty());
    }
}

SkNoPixelsDevice::ClipState& SkNoPixelsDevice::writableClip() {
    SkASSERT(!fClipStack.empty());
    ClipState& current = fClipStack.back();
    if (current.fDeferredSaveCount > 0) {
        current.fDeferredSaveCount--;
        // Stash current state in case 'current' moves during a resize
        SkIRect bounds = current.fClipBounds;
        bool aa = current.fIsAA;
        bool rect = current.fIsRect;
        return fClipStack.emplace_back(bounds, aa, rect);
    } else {
        return current;
    }
}

void SkNoPixelsDevice::onClipRect(const SkRect& rect, SkClipOp op, bool aa) {
    this->writableClip().op(op, this->localToDevice44(), rect,
                            aa, /*fillsBounds=*/true);
}

void SkNoPixelsDevice::onClipRRect(const SkRRect& rrect, SkClipOp op, bool aa) {
    this->writableClip().op(op, this->localToDevice44(), rrect.getBounds(),
                            aa, /*fillsBounds=*/rrect.isRect());
}

void SkNoPixelsDevice::onClipPath(const SkPath& path, SkClipOp op, bool aa) {
    // Toggle op if the path is inverse filled
    if (path.isInverseFillType()) {
        op = (op == SkClipOp::kDifference ? SkClipOp::kIntersect : SkClipOp::kDifference);
    }
    this->writableClip().op(op, this->localToDevice44(), path.getBounds(),
                            aa, /*fillsBounds=*/false);
}

void SkNoPixelsDevice::onClipRegion(const SkRegion& globalRgn, SkClipOp op) {
    this->writableClip().op(op, this->globalToDevice(), SkRect::Make(globalRgn.getBounds()),
                            /*isAA=*/false, /*fillsBounds=*/globalRgn.isRect());
}

void SkNoPixelsDevice::onClipShader(sk_sp<SkShader> shader) {
    this->writableClip().fIsRect = false;
}

void SkNoPixelsDevice::onReplaceClip(const SkIRect& rect) {
    SkIRect deviceRect = SkMatrixPriv::MapRect(this->globalToDevice(), SkRect::Make(rect)).round();
    if (!deviceRect.intersect(this->bounds())) {
        deviceRect.setEmpty();
    }
    auto& clip = this->writableClip();
    clip.fClipBounds = deviceRect;
    clip.fIsRect = true;
    clip.fIsAA = false;
}

SkBaseDevice::ClipType SkNoPixelsDevice::onGetClipType() const {
    const auto& clip = this->clip();
    if (clip.fClipBounds.isEmpty()) {
        return ClipType::kEmpty;
    } else if (clip.fIsRect) {
        return ClipType::kRect;
    } else {
        return ClipType::kComplex;
    }
}

void SkNoPixelsDevice::ClipState::op(SkClipOp op, const SkM44& transform, const SkRect& bounds,
                                     bool isAA, bool fillsBounds) {
    const bool isRect = fillsBounds && SkMatrixPriv::IsScaleTranslateAsM33(transform);
    fIsAA |= isAA;

    SkRect devBounds = bounds.isEmpty() ? SkRect::MakeEmpty()
                                        : SkMatrixPriv::MapRect(transform, bounds);
    if (op == SkClipOp::kIntersect) {
        if (!fClipBounds.intersect(isAA ? devBounds.roundOut() : devBounds.round())) {
            fClipBounds.setEmpty();
        }
        // A rectangular clip remains rectangular if the intersection is a rect
        fIsRect &= isRect;
    } else if (isRect) {
        // Conservatively, we can leave the clip bounds unchanged and respect the difference op.
        // But, if we're subtracting out an axis-aligned rectangle that fully spans our existing
        // clip on an axis, we can shrink the clip bounds.
        SkASSERT(op == SkClipOp::kDifference);
        SkIRect difference;
        if (SkRectPriv::Subtract(fClipBounds, isAA ? devBounds.roundIn() : devBounds.round(),
                                 &difference)) {
            fClipBounds = difference;
        } else {
            // The difference couldn't be represented as a rect
            fIsRect = false;
        }
    } else {
        // A non-rect shape was applied
        fIsRect = false;
    }
}
