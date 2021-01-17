/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkDevice.h"

#include "include/core/SkColorFilter.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPathMeasure.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkShader.h"
#include "include/core/SkVertices.h"
#include "include/private/SkTo.h"
#include "src/core/SkDraw.h"
#include "src/core/SkGlyphRun.h"
#include "src/core/SkImageFilterCache.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkLatticeIter.h"
#include "src/core/SkMarkerStack.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkTLazy.h"
#include "src/core/SkTextBlobPriv.h"
#include "src/core/SkUtils.h"
#include "src/image/SkImage_Base.h"
#include "src/shaders/SkLocalMatrixShader.h"
#include "src/utils/SkPatchUtils.h"

SkBaseDevice::SkBaseDevice(const SkImageInfo& info, const SkSurfaceProps& surfaceProps)
        : SkMatrixProvider(/* localToDevice = */ SkMatrix::I())
        , fInfo(info)
        , fSurfaceProps(surfaceProps) {
    fDeviceToGlobal.reset();
    fGlobalToDevice.reset();
}

void SkBaseDevice::setDeviceCoordinateSystem(const SkMatrix& deviceToGlobal,
                                             const SkM44& localToDevice,
                                             int bufferOriginX,
                                             int bufferOriginY) {
    fDeviceToGlobal = deviceToGlobal;
    fDeviceToGlobal.normalizePerspective();
    SkAssertResult(deviceToGlobal.invert(&fGlobalToDevice));

    fLocalToDevice = localToDevice;
    fLocalToDevice.normalizePerspective();
    if (bufferOriginX | bufferOriginY) {
        fDeviceToGlobal.preTranslate(bufferOriginX, bufferOriginY);
        fGlobalToDevice.postTranslate(-bufferOriginX, -bufferOriginY);
        fLocalToDevice.postTranslate(-bufferOriginX, -bufferOriginY);
    }
    fLocalToDevice33 = fLocalToDevice.asM33();
}

void SkBaseDevice::setGlobalCTM(const SkM44& ctm) {
    fLocalToDevice = ctm;
    fLocalToDevice.normalizePerspective();
    if (!fGlobalToDevice.isIdentity()) {
        // Map from the global CTM state to this device's coordinate system.
        fLocalToDevice.postConcat(SkM44(fGlobalToDevice));
    }
    fLocalToDevice33 = fLocalToDevice.asM33();
}

bool SkBaseDevice::isPixelAlignedToGlobal() const {
    return fDeviceToGlobal.isTranslate() &&
           SkScalarIsInt(fDeviceToGlobal.getTranslateX()) &&
           SkScalarIsInt(fDeviceToGlobal.getTranslateY());
}

SkIPoint SkBaseDevice::getOrigin() const {
    // getOrigin() is deprecated, the old origin has been moved into the fDeviceToGlobal matrix.
    // This extracts the origin from the matrix, but asserts that a more complicated coordinate
    // space hasn't been set of the device. This function can be removed once existing use cases
    // have been updated to use the device-to-global matrix instead or have themselves been removed
    // (e.g. Android's device-space clip regions are going away, and are not compatible with the
    // generalized device coordinate system).
    SkASSERT(this->isPixelAlignedToGlobal());
    return SkIPoint::Make(SkScalarFloorToInt(fDeviceToGlobal.getTranslateX()),
                          SkScalarFloorToInt(fDeviceToGlobal.getTranslateY()));
}

SkMatrix SkBaseDevice::getRelativeTransform(const SkBaseDevice& dstDevice) const {
    // To get the transform from this space to the other device's, transform from our space to
    // global and then from global to the other device.
    return SkMatrix::Concat(dstDevice.fGlobalToDevice, fDeviceToGlobal);
}

bool SkBaseDevice::getLocalToMarker(uint32_t id, SkM44* localToMarker) const {
    // The marker stack stores CTM snapshots, which are "marker to global" matrices.
    // We ask for the (cached) inverse, which is a "global to marker" matrix.
    SkM44 globalToMarker;
    // ID 0 is special, and refers to the CTM (local-to-global)
    if (fMarkerStack && (id == 0 || fMarkerStack->findMarkerInverse(id, &globalToMarker))) {
        if (localToMarker) {
            // globalToMarker will still be the identity if id is zero
            *localToMarker = globalToMarker * SkM44(fDeviceToGlobal) * fLocalToDevice;
        }
        return true;
    }
    return false;
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
                             const SkPoint texCoords[4], SkBlendMode bmode, const SkPaint& paint) {
    SkISize lod = SkPatchUtils::GetLevelOfDetail(cubics, &this->localToDevice());
    auto vertices = SkPatchUtils::MakeVertices(cubics, colors, texCoords, lod.width(), lod.height(),
                                               this->imageInfo().colorSpace());
    if (vertices) {
        this->drawVertices(vertices.get(), bmode, paint);
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

void SkBaseDevice::drawAtlas(const SkImage* atlas, const SkRSXform xform[],
                             const SkRect tex[], const SkColor colors[], int quadCount,
                             SkBlendMode mode, const SkSamplingOptions& sampling,
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
            sk_memset32(vCol, colors[i], 6);
            vCol += 6;
        }
    }
    SkPaint p(paint);
    p.setShader(atlas->makeShader(sampling));
    this->drawVertices(builder.detach().get(), mode, p);
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

void SkBaseDevice::drawDrawable(SkDrawable* drawable, const SkMatrix* matrix, SkCanvas* canvas) {
    drawable->draw(canvas, matrix);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkBaseDevice::drawSpecial(SkSpecialImage*, const SkMatrix&, const SkSamplingOptions&,
                               const SkPaint&) {}
sk_sp<SkSpecialImage> SkBaseDevice::makeSpecial(const SkBitmap&) { return nullptr; }
sk_sp<SkSpecialImage> SkBaseDevice::makeSpecial(const SkImage*) { return nullptr; }
sk_sp<SkSpecialImage> SkBaseDevice::snapSpecial(const SkIRect&, bool) { return nullptr; }
sk_sp<SkSpecialImage> SkBaseDevice::snapSpecial() {
    return this->snapSpecial(SkIRect::MakeWH(this->width(), this->height()));
}

void SkBaseDevice::drawDevice(SkBaseDevice* device, const SkSamplingOptions& sampling,
                              const SkPaint& paint) {
    sk_sp<SkSpecialImage> deviceImage = device->snapSpecial();
    if (deviceImage) {
        this->drawSpecial(deviceImage.get(), device->getRelativeTransform(*this), sampling, paint);
    }
}

void SkBaseDevice::drawFilteredImage(const skif::Mapping& mapping, SkSpecialImage* src,
                                     const SkImageFilter* filter, const SkSamplingOptions& sampling,
                                     const SkPaint& paint) {
    SkASSERT(!paint.getImageFilter() && !paint.getMaskFilter());
    using For = skif::Usage;

    skif::LayerSpace<SkIRect> targetOutput = mapping.deviceToLayer(
            skif::DeviceSpace<SkIRect>(this->devClipBounds()));

    // FIXME If the saved layer (so src) was created to use F16, should we do all image filtering
    // in F16 and then only flatten to the destination color encoding at the end?
    // Currently, this context converts everything to the dst color type ASAP.
    SkColorType colorType = this->imageInfo().colorType();
    if (colorType == kUnknown_SkColorType) {
        colorType = kRGBA_8888_SkColorType;
    }

    // getImageFilterCache returns a bare image filter cache pointer that must be ref'ed until the
    // filter's filterImage(ctx) function returns.
    sk_sp<SkImageFilterCache> cache(this->getImageFilterCache());
    skif::Context ctx(mapping, targetOutput, cache.get(), colorType, this->imageInfo().colorSpace(),
                      skif::FilterResult<For::kInput>(sk_ref_sp(src)));

    SkIPoint offset;
    sk_sp<SkSpecialImage> result = as_IFB(filter)->filterImage(ctx).imageAndOffset(&offset);
    if (result) {
        SkMatrix deviceMatrixWithOffset = mapping.deviceMatrix();
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

#include "src/core/SkUtils.h"


// TODO: This does not work for arbitrary shader DAGs (when there is no single leaf local matrix).
// What we really need is proper post-LM plumbing for shaders.
static sk_sp<SkShader> make_post_inverse_lm(const SkShader* shader, const SkMatrix& m) {
    SkMatrix inverse;
    if (!shader || !m.invert(&inverse)) {
        return nullptr;
    }

    // Normal LMs pre-compose.  In order to push a post local matrix, we shoot for
    // something along these lines (where all new components are pre-composed):
    //
    //   new_lm X current_lm == current_lm X inv(current_lm) X new_lm X current_lm
    //
    // We also have two sources of local matrices:
    //   - the actual shader lm
    //   - outer lms applied via SkLocalMatrixShader

    SkMatrix outer_lm;
    const auto nested_shader = as_SB(shader)->makeAsALocalMatrixShader(&outer_lm);
    if (nested_shader) {
        // unfurl the shader
        shader = nested_shader.get();
    } else {
        outer_lm.reset();
    }

    const auto lm = *as_SB(shader)->totalLocalMatrix(nullptr);
    SkMatrix lm_inv;
    if (!lm.invert(&lm_inv)) {
        return nullptr;
    }

    // Note: since we unfurled the shader above, we don't need to apply an outer_lm inverse
    return shader->makeWithLocalMatrix(lm_inv * inverse * lm * outer_lm);
}

void SkBaseDevice::drawGlyphRunRSXform(const SkFont& font, const SkGlyphID glyphs[],
                                       const SkRSXform xform[], int count, SkPoint origin,
                                       const SkPaint& paint) {
    const SkM44 originalLocalToDevice = this->localToDevice44();
    if (!originalLocalToDevice.isFinite() || !SkScalarIsFinite(font.getSize()) ||
        !SkScalarIsFinite(font.getScaleX()) ||
        !SkScalarIsFinite(font.getSkewX())) {
        return;
    }

    SkPoint sharedPos{0, 0};    // we're at the origin
    SkGlyphID glyphID;
    SkGlyphRun glyphRun{
        font,
        SkSpan<const SkPoint>{&sharedPos, 1},
        SkSpan<const SkGlyphID>{&glyphID, 1},
        SkSpan<const char>{},
        SkSpan<const uint32_t>{}
    };

    for (int i = 0; i < count; i++) {
        glyphID = glyphs[i];
        // now "glyphRun" is pointing at the current glyphID

        SkMatrix glyphToDevice;
        glyphToDevice.setRSXform(xform[i]).postTranslate(origin.fX, origin.fY);

        // We want to rotate each glyph by the rsxform, but we don't want to rotate "space"
        // (i.e. the shader that cares about the ctm) so we have to undo our little ctm trick
        // with a localmatrixshader so that the shader draws as if there was no change to the ctm.
        SkPaint transformingPaint{paint};
        transformingPaint.setShader(make_post_inverse_lm(paint.getShader(), glyphToDevice));

        this->setLocalToDevice(originalLocalToDevice * SkM44(glyphToDevice));

        this->drawGlyphRunList(SkGlyphRunList{glyphRun, transformingPaint});
    }
    this->setLocalToDevice(originalLocalToDevice);
}

//////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkSurface> SkBaseDevice::makeSurface(SkImageInfo const&, SkSurfaceProps const&) {
    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////

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

SkConservativeClip& SkNoPixelsDevice::writableClip() {
    SkASSERT(!fClipStack.empty());
    ClipState& current = fClipStack.back();
    if (current.fDeferredSaveCount > 0) {
        current.fDeferredSaveCount--;
        return fClipStack.push_back(ClipState(current.fClip)).fClip;
    } else {
        return current.fClip;
    }
}

void SkNoPixelsDevice::onClipRect(const SkRect& rect, SkClipOp op, bool aa) {
    this->writableClip().opRect(rect, this->localToDevice(), this->bounds(), (SkRegion::Op) op, aa);
}

void SkNoPixelsDevice::onClipRRect(const SkRRect& rrect, SkClipOp op, bool aa) {
    this->writableClip().opRRect(rrect, this->localToDevice(), this->bounds(),
                                 (SkRegion::Op) op, aa);
}

void SkNoPixelsDevice::onClipPath(const SkPath& path, SkClipOp op, bool aa) {
    this->writableClip().opPath(path, this->localToDevice(), this->bounds(),
                            (SkRegion::Op) op, aa);
}

void SkNoPixelsDevice::onClipRegion(const SkRegion& globalRgn, SkClipOp op) {
    if (globalRgn.isEmpty()) {
        this->writableClip().setEmpty();
    } else if (this->isPixelAlignedToGlobal()) {
        SkIPoint origin = this->getOrigin();
        SkRegion deviceRgn(globalRgn);
        deviceRgn.translate(-origin.fX, -origin.fY);
        this->writableClip().opRegion(deviceRgn, (SkRegion::Op) op);
    } else {
        this->writableClip().opRect(SkRect::Make(globalRgn.getBounds()), this->globalToDevice(),
                                    this->bounds(), (SkRegion::Op) op, false);
    }
}

void SkNoPixelsDevice::onClipShader(sk_sp<SkShader> shader) {
    this->writableClip().opShader(std::move(shader));
}

void SkNoPixelsDevice::onReplaceClip(const SkIRect& rect) {
    SkIRect deviceRect = this->globalToDevice().mapRect(SkRect::Make(rect)).round();
    if (!deviceRect.intersect(this->bounds())) {
        deviceRect.setEmpty();
    }
    this->writableClip().setRect(deviceRect);
}

void SkNoPixelsDevice::onSetDeviceClipRestriction(SkIRect* mutableClipRestriction) {
    if (!mutableClipRestriction || mutableClipRestriction->isEmpty()) {
        // The subset clip restriction is gone, so just store the actual device bounds as the limit
        fDeviceClipRestriction.setEmpty();
    } else {
        fDeviceClipRestriction =
                this->globalToDevice().mapRect(SkRect::Make(*mutableClipRestriction)).round();
        // Besides affecting future ops, it acts as an immediate intersection
        this->writableClip().opIRect(fDeviceClipRestriction, SkRegion::kIntersect_Op);
    }
}

SkBaseDevice::ClipType SkNoPixelsDevice::onGetClipType() const {
    const auto& clip = this->clip();
    if (clip.isEmpty()) {
        return ClipType::kEmpty;
    } else if (clip.isRect()) {
        return ClipType::kRect;
    } else {
        return ClipType::kComplex;
    }
}
