/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/SkGpuDevice_nga.h"

#ifdef SK_NGA

#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkImageFilterCache.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrTracing.h"

#define ASSERT_SINGLE_OWNER GR_ASSERT_SINGLE_OWNER(fContext->priv().singleOwner())

SkGpuDevice_nga::SkGpuDevice_nga(GrRecordingContext* rContext,
                                 const SkImageInfo& ii,
                                 const SkSurfaceProps& props)
    : INHERITED(ii, props)
    , fContext(SkRef(rContext)) {
}

SkGpuDevice_nga::~SkGpuDevice_nga() {}

void SkGpuDevice_nga::onSave() {
}

void SkGpuDevice_nga::onRestore() {
}

void SkGpuDevice_nga::onClipRect(const SkRect& rect, SkClipOp op, bool aa) {
}

void SkGpuDevice_nga::onClipRRect(const SkRRect& rrect, SkClipOp op, bool aa) {
}

void SkGpuDevice_nga::onClipPath(const SkPath& path, SkClipOp op, bool aa) {
    SkASSERT(op == SkClipOp::kIntersect || op == SkClipOp::kDifference);
}

void SkGpuDevice_nga::onClipShader(sk_sp<SkShader> shader) {
}

void SkGpuDevice_nga::onClipRegion(const SkRegion& globalRgn, SkClipOp op) {
    SkASSERT(op == SkClipOp::kIntersect || op == SkClipOp::kDifference);
}

void SkGpuDevice_nga::onReplaceClip(const SkIRect& rect) {
}

void SkGpuDevice_nga::onSetDeviceClipRestriction(SkIRect* mutableClipRestriction) {
}

bool SkGpuDevice_nga::onClipIsAA() const {
    return false;
}

bool SkGpuDevice_nga::onClipIsWideOpen() const {
    return false;
}

void SkGpuDevice_nga::onAsRgnClip(SkRegion* region) const {
}

SkBaseDevice::ClipType SkGpuDevice_nga::onGetClipType() const {
    return ClipType::kEmpty;
}

SkIRect SkGpuDevice_nga::onDevClipBounds() const {
    return SkIRect::MakeEmpty();
}

void SkGpuDevice_nga::drawPaint(const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice_nga", "drawPaint", fContext.get());
}

void SkGpuDevice_nga::drawPoints(SkCanvas::PointMode mode,
                                 size_t count,
                                 const SkPoint points[],
                                 const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice_nga", "drawPoints", fContext.get());

}

void SkGpuDevice_nga::drawRect(const SkRect& rect, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice_nga", "drawRect", fContext.get());
}

void SkGpuDevice_nga::drawRegion(const SkRegion& r, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice_nga", "drawRegion", fContext.get());
}

void SkGpuDevice_nga::drawOval(const SkRect& oval, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice_nga", "drawOval", fContext.get());
}

void SkGpuDevice_nga::drawArc(const SkRect& oval,
                              SkScalar startAngle,
                              SkScalar sweepAngle,
                              bool useCenter,
                              const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice_nga", "drawArc", fContext.get());
}

void SkGpuDevice_nga::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice_nga", "drawRRect", fContext.get());
}

void SkGpuDevice_nga::drawDRRect(const SkRRect& outer,
                                 const SkRRect& inner,
                                 const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice_nga", "drawDRRect", fContext.get());
}

void SkGpuDevice_nga::drawPath(const SkPath& path, const SkPaint& paint, bool pathIsMutable) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice_nga", "drawPath", fContext.get());
}

void SkGpuDevice_nga::drawImageRect(const SkImage* image,
                                    const SkRect* src,
                                    const SkRect& dst,
                                    const SkSamplingOptions& sampling,
                                    const SkPaint& paint,
                                    SkCanvas::SrcRectConstraint constraint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice_nga", "drawImageRect", fContext.get());
}

void SkGpuDevice_nga::drawImageLattice(const SkImage* image,
                                       const SkCanvas::Lattice& lattice,
                                       const SkRect& dst,
                                       SkFilterMode filter,
                                       const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice_nga", "drawImageLattice", fContext.get());
}

void SkGpuDevice_nga::drawVertices(const SkVertices* vertices,
                                   SkBlendMode mode,
                                   const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice_nga", "drawVertices", fContext.get());
}

void SkGpuDevice_nga::drawShadow(const SkPath& path, const SkDrawShadowRec& rec) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice_nga", "drawShadow", fContext.get());
}

void SkGpuDevice_nga::drawAtlas(const SkImage* atlas,
                                const SkRSXform xform[],
                                const SkRect texRect[],
                                const SkColor colors[],
                                int count,
                                SkBlendMode mode,
                                const SkSamplingOptions& sampling,
                                const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice_nga", "drawAtlas", fContext.get());
}

void SkGpuDevice_nga::drawEdgeAAQuad(const SkRect& rect,
                                     const SkPoint clip[4],
                                     SkCanvas::QuadAAFlags aaFlags,
                                     const SkColor4f& color,
                                     SkBlendMode mode) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice_nga", "drawEdgeAAQuad", fContext.get());
}

void SkGpuDevice_nga::drawEdgeAAImageSet(const SkCanvas::ImageSetEntry set[],
                                         int count,
                                         const SkPoint dstClips[],
                                         const SkMatrix preViewMatrices[],
                                         const SkSamplingOptions& sampling,
                                         const SkPaint& paint,
                                         SkCanvas::SrcRectConstraint constraint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice_nga", "drawEdgeAAImageSet", fContext.get());
}

void SkGpuDevice_nga::drawDrawable(SkDrawable* drawable,
                                   const SkMatrix* matrix,
                                   SkCanvas* canvas) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice_nga", "drawDrawable", fContext.get());

    this->INHERITED::drawDrawable(drawable, matrix, canvas);
}

void SkGpuDevice_nga::onDrawGlyphRunList(const SkGlyphRunList& glyphRunList, const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice_nga", "onDrawGlyphRunList", fContext.get());
}

void SkGpuDevice_nga::drawDevice(SkBaseDevice* device,
                                 const SkSamplingOptions& sampling,
                                 const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("SkGpuDevice_nga", "drawDevice", fContext.get());
}

void SkGpuDevice_nga::drawSpecial(SkSpecialImage* special,
                                  const SkMatrix& localToDevice,
                                  const SkSamplingOptions& sampling,
                                  const SkPaint& paint) {
}

sk_sp<SkSpecialImage> SkGpuDevice_nga::makeSpecial(const SkBitmap& bitmap) {
    ASSERT_SINGLE_OWNER
    return nullptr;
}

sk_sp<SkSpecialImage> SkGpuDevice_nga::makeSpecial(const SkImage* image) {
    ASSERT_SINGLE_OWNER
    return nullptr;
}

sk_sp<SkSpecialImage> SkGpuDevice_nga::snapSpecial(const SkIRect& subset, bool forceCopy) {
    ASSERT_SINGLE_OWNER
    return nullptr;
}

sk_sp<SkSurface> SkGpuDevice_nga::makeSurface(const SkImageInfo& ii,
                                              const SkSurfaceProps& props) {
    ASSERT_SINGLE_OWNER
    return nullptr;
}

bool SkGpuDevice_nga::onReadPixels(const SkPixmap& pm, int x, int y) {
    ASSERT_SINGLE_OWNER
    return false;
}

bool SkGpuDevice_nga::onWritePixels(const SkPixmap& pm, int x, int y) {
    ASSERT_SINGLE_OWNER
    return false;
}

bool SkGpuDevice_nga::onAccessPixels(SkPixmap*) {
    ASSERT_SINGLE_OWNER
    return false;
}

SkBaseDevice* SkGpuDevice_nga::onCreateDevice(const CreateInfo& cinfo, const SkPaint*) {
    ASSERT_SINGLE_OWNER
    return nullptr;
}

bool SkGpuDevice_nga::forceConservativeRasterClip() const {
    return true;
}

SkImageFilterCache* SkGpuDevice_nga::getImageFilterCache() {
    ASSERT_SINGLE_OWNER

    // We always return a transient cache, so it is freed after each filter traversal.
    return SkImageFilterCache::Create(SkImageFilterCache::kDefaultTransientSize);
}

#endif // SK_NGA
