/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/pdf/SkPDFDevice.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkRRect.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/docs/SkPDFDocument.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/pathops/SkPathOps.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkAnnotationKeys.h"
#include "src/core/SkBitmapDevice.h"
#include "src/core/SkClipOpPriv.h"
#include "src/core/SkDraw.h"
#include "src/core/SkGlyphRun.h"
#include "src/core/SkImageFilterCache.h"
#include "src/core/SkMakeUnique.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkScopeExit.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTextFormatParams.h"
#include "src/core/SkXfermodeInterpretation.h"
#include "src/pdf/SkBitmapKey.h"
#include "src/pdf/SkClusterator.h"
#include "src/pdf/SkPDFBitmap.h"
#include "src/pdf/SkPDFDocumentPriv.h"
#include "src/pdf/SkPDFFont.h"
#include "src/pdf/SkPDFFormXObject.h"
#include "src/pdf/SkPDFGraphicState.h"
#include "src/pdf/SkPDFResourceDict.h"
#include "src/pdf/SkPDFShader.h"
#include "src/pdf/SkPDFTypes.h"
#include "src/pdf/SkPDFUtils.h"
#include "src/utils/SkUTF.h"

#include <vector>

#ifndef SK_PDF_MASK_QUALITY
    // If MASK_QUALITY is in [0,100], will be used for JpegEncoder.
    // Otherwise, just encode masks losslessly.
    #define SK_PDF_MASK_QUALITY 50
    // Since these masks are used for blurry shadows, we shouldn't need
    // high quality.  Raise this value if your shadows have visible JPEG
    // artifacts.
    // If SkJpegEncoder::Encode fails, we will fall back to the lossless
    // encoding.
#endif

// Utility functions

static SkPath to_path(const SkRect& r) {
    SkPath p;
    p.addRect(r);
    return p;
}

// This function destroys the mask and either frees or takes the pixels.
sk_sp<SkImage> mask_to_greyscale_image(SkMask* mask) {
    sk_sp<SkImage> img;
    SkPixmap pm(SkImageInfo::Make(mask->fBounds.width(), mask->fBounds.height(),
                                  kGray_8_SkColorType, kOpaque_SkAlphaType),
                mask->fImage, mask->fRowBytes);
    const int imgQuality = SK_PDF_MASK_QUALITY;
    if (imgQuality <= 100 && imgQuality >= 0) {
        SkDynamicMemoryWStream buffer;
        SkJpegEncoder::Options jpegOptions;
        jpegOptions.fQuality = imgQuality;
        if (SkJpegEncoder::Encode(&buffer, pm, jpegOptions)) {
            img = SkImage::MakeFromEncoded(buffer.detachAsData());
            SkASSERT(img);
            if (img) {
                SkMask::FreeImage(mask->fImage);
            }
        }
    }
    if (!img) {
        img = SkImage::MakeFromRaster(pm, [](const void* p, void*) { SkMask::FreeImage((void*)p); },
                                      nullptr);
    }
    *mask = SkMask();  // destructive;
    return img;
}

sk_sp<SkImage> alpha_image_to_greyscale_image(const SkImage* mask) {
    int w = mask->width(), h = mask->height();
    SkBitmap greyBitmap;
    greyBitmap.allocPixels(SkImageInfo::Make(w, h, kGray_8_SkColorType, kOpaque_SkAlphaType));
    if (!mask->readPixels(SkImageInfo::MakeA8(w, h),
                          greyBitmap.getPixels(), greyBitmap.rowBytes(), 0, 0)) {
        return nullptr;
    }
    return SkImage::MakeFromBitmap(greyBitmap);
}

static int add_resource(SkTHashSet<SkPDFIndirectReference>& resources, SkPDFIndirectReference ref) {
    resources.add(ref);
    return ref.fValue;
}

static void draw_points(SkCanvas::PointMode mode,
                        size_t count,
                        const SkPoint* points,
                        const SkPaint& paint,
                        const SkIRect& bounds,
                        const SkMatrix& ctm,
                        SkBaseDevice* device) {
    SkRasterClip rc(bounds);
    SkDraw draw;
    draw.fDst = SkPixmap(SkImageInfo::MakeUnknown(bounds.right(), bounds.bottom()), nullptr, 0);
    draw.fMatrix = &ctm;
    draw.fRC = &rc;
    draw.drawPoints(mode, count, points, paint, device);
}

// A shader's matrix is:  CTMM x LocalMatrix x WrappingLocalMatrix.  We want to
// switch to device space, where CTM = I, while keeping the original behavior.
//
//               I * LocalMatrix * NewWrappingMatrix = CTM * LocalMatrix
//                   LocalMatrix * NewWrappingMatrix = CTM * LocalMatrix
//  InvLocalMatrix * LocalMatrix * NewWrappingMatrix = InvLocalMatrix * CTM * LocalMatrix
//                                 NewWrappingMatrix = InvLocalMatrix * CTM * LocalMatrix
//
static void transform_shader(SkPaint* paint, const SkMatrix& ctm) {
    SkASSERT(!ctm.isIdentity());
    SkMatrix lm = SkPDFUtils::GetShaderLocalMatrix(paint->getShader());
    SkMatrix lmInv;
    if (lm.invert(&lmInv)) {
        SkMatrix m = SkMatrix::Concat(SkMatrix::Concat(lmInv, ctm), lm);
        paint->setShader(paint->getShader()->makeWithLocalMatrix(m));
    }
}

static SkTCopyOnFirstWrite<SkPaint> clean_paint(const SkPaint& srcPaint) {
    SkTCopyOnFirstWrite<SkPaint> paint(srcPaint);
    // If the paint will definitely draw opaquely, replace kSrc with
    // kSrcOver.  http://crbug.com/473572
    if (SkBlendMode::kSrcOver != paint->getBlendMode() &&
        kSrcOver_SkXfermodeInterpretation == SkInterpretXfermode(*paint, false))
    {
        paint.writable()->setBlendMode(SkBlendMode::kSrcOver);
    }
    // If the paint has a color filter, apply the color filter to the shader or the
    // paint color.  Remove the color filter.
    if (SkColorFilter* cf = paint->getColorFilter()) {
        SkPaint* p = paint.writable();
        if (SkShader* shader = paint->getShader()) {
            p->setShader(shader->makeWithColorFilter(paint->refColorFilter()));
        } else {
            p->setColor4f(cf->filterColor4f(paint->getColor4f(), nullptr), nullptr);
        }
        p->setColorFilter(nullptr);
    }
    return paint;
}

static void set_style(SkTCopyOnFirstWrite<SkPaint>* paint, SkPaint::Style style) {
    if (paint->get()->getStyle() != style) {
        paint->writable()->setStyle(style);
    }
}

/* Calculate an inverted path's equivalent non-inverted path, given the
 * canvas bounds.
 * outPath may alias with invPath (since this is supported by PathOps).
 */
static bool calculate_inverse_path(const SkRect& bounds, const SkPath& invPath,
                                   SkPath* outPath) {
    SkASSERT(invPath.isInverseFillType());
    return Op(to_path(bounds), invPath, kIntersect_SkPathOp, outPath);
}

SkBaseDevice* SkPDFDevice::onCreateDevice(const CreateInfo& cinfo, const SkPaint* layerPaint) {
    // PDF does not support image filters, so render them on CPU.
    // Note that this rendering is done at "screen" resolution (100dpi), not
    // printer resolution.

    // TODO: It may be possible to express some filters natively using PDF
    // to improve quality and file size (https://bug.skia.org/3043)
    if (layerPaint && (layerPaint->getImageFilter() || layerPaint->getColorFilter())) {
        // need to return a raster device, which we will detect in drawDevice()
        return SkBitmapDevice::Create(cinfo.fInfo, SkSurfaceProps(0, kUnknown_SkPixelGeometry));
    }
    return new SkPDFDevice(cinfo.fInfo.dimensions(), fDocument);
}

// A helper class to automatically finish a ContentEntry at the end of a
// drawing method and maintain the state needed between set up and finish.
class ScopedContentEntry {
public:
    ScopedContentEntry(SkPDFDevice* device,
                       const SkClipStack* clipStack,
                       const SkMatrix& matrix,
                       const SkPaint& paint,
                       SkScalar textScale = 0)
        : fDevice(device)
        , fBlendMode(SkBlendMode::kSrcOver)
        , fClipStack(clipStack)
    {
        if (matrix.hasPerspective()) {
            NOT_IMPLEMENTED(!matrix.hasPerspective(), false);
            return;
        }
        fBlendMode = paint.getBlendMode();
        fContentStream =
            fDevice->setUpContentEntry(clipStack, matrix, paint, textScale, &fDstFormXObject);
    }
    ScopedContentEntry(SkPDFDevice* dev, const SkPaint& paint, SkScalar textScale = 0)
        : ScopedContentEntry(dev, &dev->cs(), dev->ctm(), paint, textScale) {}

    ~ScopedContentEntry() {
        if (fContentStream) {
            SkPath* shape = &fShape;
            if (shape->isEmpty()) {
                shape = nullptr;
            }
            fDevice->finishContentEntry(fClipStack, fBlendMode, fDstFormXObject, shape);
        }
    }

    explicit operator bool() const { return fContentStream != nullptr; }
    SkDynamicMemoryWStream* stream() { return fContentStream; }

    /* Returns true when we explicitly need the shape of the drawing. */
    bool needShape() {
        switch (fBlendMode) {
            case SkBlendMode::kClear:
            case SkBlendMode::kSrc:
            case SkBlendMode::kSrcIn:
            case SkBlendMode::kSrcOut:
            case SkBlendMode::kDstIn:
            case SkBlendMode::kDstOut:
            case SkBlendMode::kSrcATop:
            case SkBlendMode::kDstATop:
            case SkBlendMode::kModulate:
                return true;
            default:
                return false;
        }
    }

    /* Returns true unless we only need the shape of the drawing. */
    bool needSource() {
        if (fBlendMode == SkBlendMode::kClear) {
            return false;
        }
        return true;
    }

    /* If the shape is different than the alpha component of the content, then
     * setShape should be called with the shape.  In particular, images and
     * devices have rectangular shape.
     */
    void setShape(const SkPath& shape) {
        fShape = shape;
    }

private:
    SkPDFDevice* fDevice = nullptr;
    SkDynamicMemoryWStream* fContentStream = nullptr;
    SkBlendMode fBlendMode;
    SkPDFIndirectReference fDstFormXObject;
    SkPath fShape;
    const SkClipStack* fClipStack;
};

////////////////////////////////////////////////////////////////////////////////

SkPDFDevice::SkPDFDevice(SkISize pageSize, SkPDFDocument* doc, const SkMatrix& transform)
    : INHERITED(SkImageInfo::MakeUnknown(pageSize.width(), pageSize.height()),
                SkSurfaceProps(0, kUnknown_SkPixelGeometry))
    , fInitialTransform(transform)
    , fNodeId(0)
    , fDocument(doc)
{
    SkASSERT(!pageSize.isEmpty());
}

SkPDFDevice::~SkPDFDevice() = default;

void SkPDFDevice::reset() {
    fGraphicStateResources.reset();
    fXObjectResources.reset();
    fShaderResources.reset();
    fFontResources.reset();
    fContent.reset();
    fActiveStackState = SkPDFGraphicStackState();
}

void SkPDFDevice::drawAnnotation(const SkRect& rect, const char key[], SkData* value) {
    if (!value) {
        return;
    }
    const SkMatrix& pageXform = fDocument->currentPageTransform();
    SkPoint deviceOffset = {(float)this->getOrigin().x(), (float)this->getOrigin().y()};
    if (rect.isEmpty()) {
        if (!strcmp(key, SkPDFGetNodeIdKey())) {
            int nodeID;
            if (value->size() != sizeof(nodeID)) { return; }
            memcpy(&nodeID, value->data(), sizeof(nodeID));
            fNodeId = nodeID;
            return;
        }
        if (!strcmp(SkAnnotationKeys::Define_Named_Dest_Key(), key)) {
            SkPoint p = deviceOffset + this->ctm().mapXY(rect.x(), rect.y());
            pageXform.mapPoints(&p, 1);
            auto pg = fDocument->currentPage();
            fDocument->fNamedDestinations.push_back(SkPDFNamedDestination{sk_ref_sp(value), p, pg});
        }
        return;
    }
    // Convert to path to handle non-90-degree rotations.
    SkPath path = to_path(rect);
    path.transform(this->ctm(), &path);
    SkPath clip;
    (void)this->cs().asPath(&clip);
    Op(clip, path, kIntersect_SkPathOp, &path);
    // PDF wants a rectangle only.
    SkRect transformedRect =
        pageXform.mapRect(path.getBounds().makeOffset(deviceOffset.x(), deviceOffset.y()));
    if (transformedRect.isEmpty()) {
        return;
    }
    if (!strcmp(SkAnnotationKeys::URL_Key(), key)) {
        fDocument->fCurrentPageLinkToURLs.push_back(
                std::make_pair(sk_ref_sp(value), transformedRect));
    } else if (!strcmp(SkAnnotationKeys::Link_Named_Dest_Key(), key)) {
        fDocument->fCurrentPageLinkToDestinations.emplace_back(
                std::make_pair(sk_ref_sp(value), transformedRect));
    }
}

void SkPDFDevice::drawPaint(const SkPaint& srcPaint) {
    SkMatrix inverse;
    if (!this->ctm().invert(&inverse)) {
        return;
    }
    SkRect bbox = this->cs().bounds(this->bounds());
    inverse.mapRect(&bbox);
    bbox.roundOut(&bbox);
    if (this->hasEmptyClip()) {
        return;
    }
    SkPaint newPaint = srcPaint;
    newPaint.setStyle(SkPaint::kFill_Style);
    this->drawRect(bbox, newPaint);
}

void SkPDFDevice::drawPoints(SkCanvas::PointMode mode,
                             size_t count,
                             const SkPoint* points,
                             const SkPaint& srcPaint) {
    if (this->hasEmptyClip()) {
        return;
    }
    if (count == 0) {
        return;
    }
    SkTCopyOnFirstWrite<SkPaint> paint(clean_paint(srcPaint));



    if (SkCanvas::kPoints_PointMode != mode) {
        set_style(&paint, SkPaint::kStroke_Style);
    }

    // SkDraw::drawPoints converts to multiple calls to fDevice->drawPath.
    // We only use this when there's a path effect because of the overhead
    // of multiple calls to setUpContentEntry it causes.
    if (paint->getPathEffect()) {
        draw_points(mode, count, points, *paint,
                    this->devClipBounds(), this->ctm(), this);
        return;
    }


    if (mode == SkCanvas::kPoints_PointMode && paint->getStrokeCap() != SkPaint::kRound_Cap) {
        if (paint->getStrokeWidth()) {
            // PDF won't draw a single point with square/butt caps because the
            // orientation is ambiguous.  Draw a rectangle instead.
            set_style(&paint, SkPaint::kFill_Style);
            SkScalar strokeWidth = paint->getStrokeWidth();
            SkScalar halfStroke = SkScalarHalf(strokeWidth);
            for (size_t i = 0; i < count; i++) {
                SkRect r = SkRect::MakeXYWH(points[i].fX, points[i].fY, 0, 0);
                r.inset(-halfStroke, -halfStroke);
                this->drawRect(r, *paint);
            }
            return;
        } else {
            if (paint->getStrokeCap() != SkPaint::kRound_Cap) {
                paint.writable()->setStrokeCap(SkPaint::kRound_Cap);
            }
        }
    }

    ScopedContentEntry content(this, *paint);
    if (!content) {
        return;
    }
    SkDynamicMemoryWStream* contentStream = content.stream();
    switch (mode) {
        case SkCanvas::kPolygon_PointMode:
            SkPDFUtils::MoveTo(points[0].fX, points[0].fY, contentStream);
            for (size_t i = 1; i < count; i++) {
                SkPDFUtils::AppendLine(points[i].fX, points[i].fY, contentStream);
            }
            SkPDFUtils::StrokePath(contentStream);
            break;
        case SkCanvas::kLines_PointMode:
            for (size_t i = 0; i < count/2; i++) {
                SkPDFUtils::MoveTo(points[i * 2].fX, points[i * 2].fY, contentStream);
                SkPDFUtils::AppendLine(points[i * 2 + 1].fX, points[i * 2 + 1].fY, contentStream);
                SkPDFUtils::StrokePath(contentStream);
            }
            break;
        case SkCanvas::kPoints_PointMode:
            SkASSERT(paint->getStrokeCap() == SkPaint::kRound_Cap);
            for (size_t i = 0; i < count; i++) {
                SkPDFUtils::MoveTo(points[i].fX, points[i].fY, contentStream);
                SkPDFUtils::ClosePath(contentStream);
                SkPDFUtils::StrokePath(contentStream);
            }
            break;
        default:
            SkASSERT(false);
    }
}

void SkPDFDevice::drawRect(const SkRect& rect, const SkPaint& paint) {
    SkRect r = rect;
    r.sort();
    this->internalDrawPath(this->cs(), this->ctm(), to_path(r), paint, true);
}

void SkPDFDevice::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    SkPath path;
    path.addRRect(rrect);
    this->internalDrawPath(this->cs(), this->ctm(), path, paint, true);
}

void SkPDFDevice::drawOval(const SkRect& oval, const SkPaint& paint) {
    SkPath path;
    path.addOval(oval);
    this->internalDrawPath(this->cs(), this->ctm(), path, paint, true);
}

void SkPDFDevice::drawPath(const SkPath& path, const SkPaint& paint, bool pathIsMutable) {
    this->internalDrawPath(this->cs(), this->ctm(), path, paint, pathIsMutable);
}

void SkPDFDevice::internalDrawPathWithFilter(const SkClipStack& clipStack,
                                             const SkMatrix& ctm,
                                             const SkPath& origPath,
                                             const SkPaint& origPaint) {
    SkASSERT(origPaint.getMaskFilter());
    SkPath path(origPath);
    SkTCopyOnFirstWrite<SkPaint> paint(origPaint);

    SkStrokeRec::InitStyle initStyle = paint->getFillPath(path, &path)
                                     ? SkStrokeRec::kFill_InitStyle
                                     : SkStrokeRec::kHairline_InitStyle;
    path.transform(ctm, &path);

    SkIRect bounds = clipStack.bounds(this->bounds()).roundOut();
    SkMask sourceMask;
    if (!SkDraw::DrawToMask(path, &bounds, paint->getMaskFilter(), &SkMatrix::I(),
                            &sourceMask, SkMask::kComputeBoundsAndRenderImage_CreateMode,
                            initStyle)) {
        return;
    }
    SkAutoMaskFreeImage srcAutoMaskFreeImage(sourceMask.fImage);
    SkMask dstMask;
    SkIPoint margin;
    if (!as_MFB(paint->getMaskFilter())->filterMask(&dstMask, sourceMask, ctm, &margin)) {
        return;
    }
    SkIRect dstMaskBounds = dstMask.fBounds;
    sk_sp<SkImage> mask = mask_to_greyscale_image(&dstMask);
    // PDF doesn't seem to allow masking vector graphics with an Image XObject.
    // Must mask with a Form XObject.
    sk_sp<SkPDFDevice> maskDevice = this->makeCongruentDevice();
    {
        SkCanvas canvas(maskDevice);
        canvas.drawImage(mask, dstMaskBounds.x(), dstMaskBounds.y());
    }
    if (!ctm.isIdentity() && paint->getShader()) {
        transform_shader(paint.writable(), ctm); // Since we are using identity matrix.
    }
    ScopedContentEntry content(this, &clipStack, SkMatrix::I(), *paint);
    if (!content) {
        return;
    }
    this->setGraphicState(SkPDFGraphicState::GetSMaskGraphicState(
            maskDevice->makeFormXObjectFromDevice(dstMaskBounds, true), false,
            SkPDFGraphicState::kLuminosity_SMaskMode, fDocument), content.stream());
    SkPDFUtils::AppendRectangle(SkRect::Make(dstMaskBounds), content.stream());
    SkPDFUtils::PaintPath(SkPaint::kFill_Style, path.getFillType(), content.stream());
    this->clearMaskOnGraphicState(content.stream());
}

void SkPDFDevice::setGraphicState(SkPDFIndirectReference gs, SkDynamicMemoryWStream* content) {
    SkPDFUtils::ApplyGraphicState(add_resource(fGraphicStateResources, gs), content);
}

void SkPDFDevice::clearMaskOnGraphicState(SkDynamicMemoryWStream* contentStream) {
    // The no-softmask graphic state is used to "turn off" the mask for later draw calls.
    SkPDFIndirectReference& noSMaskGS = fDocument->fNoSmaskGraphicState;
    if (!noSMaskGS) {
        SkPDFDict tmp("ExtGState");
        tmp.insertName("SMask", "None");
        noSMaskGS = fDocument->emit(tmp);
    }
    this->setGraphicState(noSMaskGS, contentStream);
}

void SkPDFDevice::internalDrawPath(const SkClipStack& clipStack,
                                   const SkMatrix& ctm,
                                   const SkPath& origPath,
                                   const SkPaint& srcPaint,
                                   bool pathIsMutable) {
    if (clipStack.isEmpty(this->bounds())) {
        return;
    }
    SkTCopyOnFirstWrite<SkPaint> paint(clean_paint(srcPaint));
    SkPath modifiedPath;
    SkPath* pathPtr = const_cast<SkPath*>(&origPath);

    if (paint->getMaskFilter()) {
        this->internalDrawPathWithFilter(clipStack, ctm, origPath, *paint);
        return;
    }

    SkMatrix matrix = ctm;

    if (paint->getPathEffect()) {
        if (clipStack.isEmpty(this->bounds())) {
            return;
        }
        if (!pathIsMutable) {
            modifiedPath = origPath;
            pathPtr = &modifiedPath;
            pathIsMutable = true;
        }
        if (paint->getFillPath(*pathPtr, pathPtr)) {
            set_style(&paint, SkPaint::kFill_Style);
        } else {
            set_style(&paint, SkPaint::kStroke_Style);
            if (paint->getStrokeWidth() != 0) {
                paint.writable()->setStrokeWidth(0);
            }
        }
        paint.writable()->setPathEffect(nullptr);
    }

    if (this->handleInversePath(*pathPtr, *paint, pathIsMutable)) {
        return;
    }
    if (matrix.getType() & SkMatrix::kPerspective_Mask) {
        if (!pathIsMutable) {
            modifiedPath = origPath;
            pathPtr = &modifiedPath;
            pathIsMutable = true;
        }
        pathPtr->transform(matrix);
        if (paint->getShader()) {
            transform_shader(paint.writable(), matrix);
        }
        matrix = SkMatrix::I();
    }

    ScopedContentEntry content(this, &clipStack, matrix, *paint);
    if (!content) {
        return;
    }
    constexpr SkScalar kToleranceScale = 0.0625f;  // smaller = better conics (circles).
    SkScalar matrixScale = matrix.mapRadius(1.0f);
    SkScalar tolerance = matrixScale > 0.0f ? kToleranceScale / matrixScale : kToleranceScale;
    bool consumeDegeratePathSegments =
           paint->getStyle() == SkPaint::kFill_Style ||
           (paint->getStrokeCap() != SkPaint::kRound_Cap &&
            paint->getStrokeCap() != SkPaint::kSquare_Cap);
    SkPDFUtils::EmitPath(*pathPtr, paint->getStyle(), consumeDegeratePathSegments, content.stream(),
                         tolerance);
    SkPDFUtils::PaintPath(paint->getStyle(), pathPtr->getFillType(), content.stream());
}

////////////////////////////////////////////////////////////////////////////////

void SkPDFDevice::drawImageRect(const SkImage* image,
                                const SkRect* src,
                                const SkRect& dst,
                                const SkPaint& paint,
                                SkCanvas::SrcRectConstraint) {
    SkASSERT(image);
    this->internalDrawImageRect(SkKeyedImage(sk_ref_sp(const_cast<SkImage*>(image))),
                                src, dst, paint, this->ctm());
}

void SkPDFDevice::drawBitmapRect(const SkBitmap& bm,
                                 const SkRect* src,
                                 const SkRect& dst,
                                 const SkPaint& paint,
                                 SkCanvas::SrcRectConstraint) {
    SkASSERT(!bm.drawsNothing());
    this->internalDrawImageRect(SkKeyedImage(bm), src, dst, paint, this->ctm());
}

void SkPDFDevice::drawSprite(const SkBitmap& bm, int x, int y, const SkPaint& paint) {
    SkASSERT(!bm.drawsNothing());
    auto r = SkRect::MakeXYWH(x, y, bm.width(), bm.height());
    this->internalDrawImageRect(SkKeyedImage(bm), nullptr, r, paint, SkMatrix::I());
}

////////////////////////////////////////////////////////////////////////////////

namespace {
class GlyphPositioner {
public:
    GlyphPositioner(SkDynamicMemoryWStream* content,
                    SkScalar textSkewX,
                    SkPoint origin)
        : fContent(content)
        , fCurrentMatrixOrigin(origin)
        , fTextSkewX(textSkewX) {
    }
    ~GlyphPositioner() { this->flush(); }
    void flush() {
        if (fInText) {
            fContent->writeText("> Tj\n");
            fInText = false;
        }
    }
    void setWideChars(bool wide) {
        this->flush();
        fWideChars = wide;
    }
    void writeGlyph(SkPoint xy,
                    SkScalar advanceWidth,
                    uint16_t glyph) {
        if (!fInitialized) {
            // Flip the text about the x-axis to account for origin swap and include
            // the passed parameters.
            fContent->writeText("1 0 ");
            SkPDFUtils::AppendScalar(-fTextSkewX, fContent);
            fContent->writeText(" -1 ");
            SkPDFUtils::AppendScalar(fCurrentMatrixOrigin.x(), fContent);
            fContent->writeText(" ");
            SkPDFUtils::AppendScalar(fCurrentMatrixOrigin.y(), fContent);
            fContent->writeText(" Tm\n");
            fCurrentMatrixOrigin.set(0.0f, 0.0f);
            fInitialized = true;
        }
        SkPoint position = xy - fCurrentMatrixOrigin;
        if (position != SkPoint{fXAdvance, 0}) {
            this->flush();
            SkPDFUtils::AppendScalar(position.x() - position.y() * fTextSkewX, fContent);
            fContent->writeText(" ");
            SkPDFUtils::AppendScalar(-position.y(), fContent);
            fContent->writeText(" Td ");
            fCurrentMatrixOrigin = xy;
            fXAdvance = 0;
        }
        fXAdvance += advanceWidth;
        if (!fInText) {
            fContent->writeText("<");
            fInText = true;
        }
        if (fWideChars) {
            SkPDFUtils::WriteUInt16BE(fContent, glyph);
        } else {
            SkASSERT(0 == glyph >> 8);
            SkPDFUtils::WriteUInt8(fContent, static_cast<uint8_t>(glyph));
        }
    }

private:
    SkDynamicMemoryWStream* fContent;
    SkPoint fCurrentMatrixOrigin;
    SkScalar fXAdvance = 0.0f;
    SkScalar fTextSkewX;
    bool fWideChars = true;
    bool fInText = false;
    bool fInitialized = false;
};
}  // namespace

static SkUnichar map_glyph(const std::vector<SkUnichar>& glyphToUnicode, SkGlyphID glyph) {
    return glyph < glyphToUnicode.size() ? glyphToUnicode[SkToInt(glyph)] : -1;
}

namespace {
struct PositionedGlyph {
    SkPoint fPos;
    SkGlyphID fGlyph;
};
}

static SkRect get_glyph_bounds_device_space(SkGlyphID gid, SkStrike* cache,
                                            SkScalar xScale, SkScalar yScale,
                                            SkPoint xy, const SkMatrix& ctm) {
    const SkGlyph& glyph = cache->getGlyphIDMetrics(gid);
    SkRect glyphBounds = {glyph.fLeft * xScale,
                          glyph.fTop * yScale,
                          (glyph.fLeft + glyph.fWidth) * xScale,
                          (glyph.fTop + glyph.fHeight) * yScale};
    glyphBounds.offset(xy);
    ctm.mapRect(&glyphBounds); // now in dev space.
    return glyphBounds;
}

static bool contains(const SkRect& r, SkPoint p) {
   return r.left() <= p.x() && p.x() <= r.right() &&
          r.top()  <= p.y() && p.y() <= r.bottom();
}

void SkPDFDevice::drawGlyphRunAsPath(
        const SkGlyphRun& glyphRun, SkPoint offset, const SkPaint& runPaint) {
    const SkFont& font = glyphRun.font();
    SkPath path;

    struct Rec {
        SkPath* fPath;
        SkPoint fOffset;
        const SkPoint* fPos;
    } rec = {&path, offset, glyphRun.positions().data()};

    font.getPaths(glyphRun.glyphsIDs().data(), glyphRun.glyphsIDs().size(),
                  [](const SkPath* path, const SkMatrix& mx, void* ctx) {
                      Rec* rec = reinterpret_cast<Rec*>(ctx);
                      if (path) {
                          SkMatrix total = mx;
                          total.postTranslate(rec->fPos->fX + rec->fOffset.fX,
                                              rec->fPos->fY + rec->fOffset.fY);
                          rec->fPath->addPath(*path, total);
                      }
                      rec->fPos += 1; // move to the next glyph's position
                  }, &rec);
    this->internalDrawPath(this->cs(), this->ctm(), path, runPaint, true);

    SkFont transparentFont = glyphRun.font();
    transparentFont.setEmbolden(false); // Stop Recursion
    SkGlyphRun tmpGlyphRun(glyphRun, transparentFont);

    SkPaint transparent;
    transparent.setColor(SK_ColorTRANSPARENT);

    if (this->ctm().hasPerspective()) {
        SkMatrix prevCTM = this->ctm();
        this->setCTM(SkMatrix::I());
        this->internalDrawGlyphRun(tmpGlyphRun, offset, transparent);
        this->setCTM(prevCTM);
    } else {
        this->internalDrawGlyphRun(tmpGlyphRun, offset, transparent);
    }
}

static bool needs_new_font(SkPDFFont* font, SkGlyphID gid, SkStrike* cache,
                           SkAdvancedTypefaceMetrics::FontType fontType) {
    if (!font || !font->hasGlyph(gid)) {
        return true;
    }
    if (fontType == SkAdvancedTypefaceMetrics::kOther_Font) {
        return false;
    }
    const SkGlyph& glyph = cache->getGlyphIDMetrics(gid);
    if (glyph.isEmpty()) {
        return false;
    }

    bool bitmapOnly = nullptr == cache->findPath(glyph);
    bool convertedToType3 = (font->getType() == SkAdvancedTypefaceMetrics::kOther_Font);
    return convertedToType3 != bitmapOnly;
}

namespace {
constexpr int kTypicalGlyphCount = 20;
using SmallPointsArray = SkAutoSTArray<kTypicalGlyphCount, SkPoint>;
}

void SkPDFDevice::internalDrawGlyphRun(
        const SkGlyphRun& glyphRun, SkPoint offset, const SkPaint& runPaint) {

    const SkGlyphID* glyphs = glyphRun.glyphsIDs().data();
    uint32_t glyphCount = SkToU32(glyphRun.glyphsIDs().size());
    const SkFont& glyphRunFont = glyphRun.font();

    if (!glyphCount || !glyphs || glyphRunFont.getSize() <= 0 || this->hasEmptyClip()) {
        return;
    }
    if (runPaint.getPathEffect()
        || runPaint.getMaskFilter()
        || glyphRunFont.isEmbolden()
        || this->ctm().hasPerspective()
        || SkPaint::kFill_Style != runPaint.getStyle()) {
        // Stroked Text doesn't work well with Type3 fonts.
        this->drawGlyphRunAsPath(glyphRun, offset, runPaint);
        return;
    }
    SkTypeface* typeface = glyphRunFont.getTypefaceOrDefault();
    if (!typeface) {
        SkDebugf("SkPDF: SkTypeface::MakeDefault() returned nullptr.\n");
        return;
    }

    const SkAdvancedTypefaceMetrics* metrics = SkPDFFont::GetMetrics(typeface, fDocument);
    if (!metrics) {
        return;
    }
    SkAdvancedTypefaceMetrics::FontType fontType = SkPDFFont::FontType(*metrics);

    const std::vector<SkUnichar>& glyphToUnicode = SkPDFFont::GetUnicodeMap(typeface, fDocument);

    SkClusterator clusterator(glyphRun);

    int emSize;
    SkStrikeSpec strikeSpec = SkStrikeSpec::MakePDFVector(*typeface, &emSize);
    auto glyphCache = strikeSpec.findOrCreateExclusiveStrike();

    SkScalar textSize = glyphRunFont.getSize();
    SkScalar advanceScale = textSize * glyphRunFont.getScaleX() / emSize;

    // textScaleX and textScaleY are used to get a conservative bounding box for glyphs.
    SkScalar textScaleY = textSize / emSize;
    SkScalar textScaleX = advanceScale + glyphRunFont.getSkewX() * textScaleY;

    SkRect clipStackBounds = this->cs().bounds(this->bounds());

    SkTCopyOnFirstWrite<SkPaint> paint(clean_paint(runPaint));
    ScopedContentEntry content(this, *paint, glyphRunFont.getScaleX());
    if (!content) {
        return;
    }
    SkDynamicMemoryWStream* out = content.stream();

    out->writeText("BT\n");

    int markId = -1;
    if (fNodeId) {
        markId = fDocument->getMarkIdForNodeId(fNodeId);
    }

    if (markId != -1) {
        out->writeText("/P <</MCID ");
        out->writeDecAsText(markId);
        out->writeText(" >>BDC\n");
    }
    SK_AT_SCOPE_EXIT(if (markId != -1) out->writeText("EMC\n"));

    SK_AT_SCOPE_EXIT(out->writeText("ET\n"));

    const SkGlyphID maxGlyphID = SkToU16(typeface->countGlyphs() - 1);

    if (clusterator.reversedChars()) {
        out->writeText("/ReversedChars BMC\n");
    }
    SK_AT_SCOPE_EXIT(if (clusterator.reversedChars()) { out->writeText("EMC\n"); } );
    GlyphPositioner glyphPositioner(out, glyphRunFont.getSkewX(), offset);
    SkPDFFont* font = nullptr;

    SmallPointsArray advances(glyphRun.runSize());
    glyphCache->getAdvances(glyphRun.glyphsIDs(), advances.get());

    while (SkClusterator::Cluster c = clusterator.next()) {
        int index = c.fGlyphIndex;
        int glyphLimit = index + c.fGlyphCount;

        bool actualText = false;
        SK_AT_SCOPE_EXIT(if (actualText) {
                             glyphPositioner.flush();
                             out->writeText("EMC\n");
                         });
        if (c.fUtf8Text) {  // real cluster
            // Check if `/ActualText` needed.
            const char* textPtr = c.fUtf8Text;
            const char* textEnd = c.fUtf8Text + c.fTextByteLength;
            SkUnichar unichar = SkUTF::NextUTF8(&textPtr, textEnd);
            if (unichar < 0) {
                return;
            }
            if (textPtr < textEnd ||                                  // more characters left
                glyphLimit > index + 1 ||                             // toUnicode wouldn't work
                unichar != map_glyph(glyphToUnicode, glyphs[index]))  // test single Unichar map
            {
                glyphPositioner.flush();
                out->writeText("/Span<</ActualText <");
                SkPDFUtils::WriteUTF16beHex(out, 0xFEFF);  // U+FEFF = BYTE ORDER MARK
                // the BOM marks this text as UTF-16BE, not PDFDocEncoding.
                SkPDFUtils::WriteUTF16beHex(out, unichar);  // first char
                while (textPtr < textEnd) {
                    unichar = SkUTF::NextUTF8(&textPtr, textEnd);
                    if (unichar < 0) {
                        break;
                    }
                    SkPDFUtils::WriteUTF16beHex(out, unichar);
                }
                out->writeText("> >> BDC\n");  // begin marked-content sequence
                                               // with an associated property list.
                actualText = true;
            }
        }
        for (; index < glyphLimit; ++index) {
            SkGlyphID gid = glyphs[index];
            if (gid > maxGlyphID) {
                continue;
            }
            SkPoint xy = glyphRun.positions()[index];
            // Do a glyph-by-glyph bounds-reject if positions are absolute.
            SkRect glyphBounds = get_glyph_bounds_device_space(
                    gid, glyphCache.get(), textScaleX, textScaleY,
                    xy + offset, this->ctm());
            if (glyphBounds.isEmpty()) {
                if (!contains(clipStackBounds, {glyphBounds.x(), glyphBounds.y()})) {
                    continue;
                }
            } else {
                if (!clipStackBounds.intersects(glyphBounds)) {
                    continue;  // reject glyphs as out of bounds
                }
            }
            if (needs_new_font(font, gid, glyphCache.get(), fontType)) {
                // Not yet specified font or need to switch font.
                font = SkPDFFont::GetFontResource(fDocument, glyphCache.get(), typeface, gid);
                SkASSERT(font);  // All preconditions for SkPDFFont::GetFontResource are met.
                glyphPositioner.flush();
                glyphPositioner.setWideChars(font->multiByteGlyphs());
                SkPDFWriteResourceName(out, SkPDFResourceType::kFont,
                                       add_resource(fFontResources, font->indirectReference()));
                out->writeText(" ");
                SkPDFUtils::AppendScalar(textSize, out);
                out->writeText(" Tf\n");

            }
            font->noteGlyphUsage(gid);
            SkGlyphID encodedGlyph = font->multiByteGlyphs()
                                   ? gid : font->glyphToPDFFontEncoding(gid);
            SkScalar advance = advanceScale * advances[index].x();
            glyphPositioner.writeGlyph(xy, advance, encodedGlyph);
        }
    }
}

void SkPDFDevice::drawGlyphRunList(const SkGlyphRunList& glyphRunList) {
    for (const SkGlyphRun& glyphRun : glyphRunList) {
        this->internalDrawGlyphRun(glyphRun, glyphRunList.origin(), glyphRunList.paint());
    }
}

void SkPDFDevice::drawVertices(const SkVertices*, const SkVertices::Bone[], int, SkBlendMode,
                               const SkPaint&) {
    if (this->hasEmptyClip()) {
        return;
    }
    // TODO: implement drawVertices
}

void SkPDFDevice::drawFormXObject(SkPDFIndirectReference xObject, SkDynamicMemoryWStream* content) {
    SkASSERT(xObject);
    SkPDFWriteResourceName(content, SkPDFResourceType::kXObject,
                           add_resource(fXObjectResources, xObject));
    content->writeText(" Do\n");
}

void SkPDFDevice::drawDevice(SkBaseDevice* device, int x, int y, const SkPaint& paint) {
    SkASSERT(!paint.getImageFilter());

    // Check if the source device is really a bitmapdevice (because that's what we returned
    // from createDevice (likely due to an imagefilter)
    SkPixmap pmap;
    if (device->peekPixels(&pmap)) {
        SkBitmap bitmap;
        bitmap.installPixels(pmap);
        this->drawSprite(bitmap, x, y, paint);
        return;
    }

    // our onCreateCompatibleDevice() always creates SkPDFDevice subclasses.
    SkPDFDevice* pdfDevice = static_cast<SkPDFDevice*>(device);

    if (pdfDevice->isContentEmpty()) {
        return;
    }

    SkMatrix matrix = SkMatrix::MakeTrans(SkIntToScalar(x), SkIntToScalar(y));
    ScopedContentEntry content(this, &this->cs(), matrix, paint);
    if (!content) {
        return;
    }
    if (content.needShape()) {
        SkISize dim = device->imageInfo().dimensions();
        content.setShape(to_path(SkRect::Make(SkIRect::MakeXYWH(x, y, dim.width(), dim.height()))));
    }
    if (!content.needSource()) {
        return;
    }
    this->drawFormXObject(pdfDevice->makeFormXObjectFromDevice(), content.stream());
}

sk_sp<SkSurface> SkPDFDevice::makeSurface(const SkImageInfo& info, const SkSurfaceProps& props) {
    return SkSurface::MakeRaster(info, &props);
}

static std::vector<SkPDFIndirectReference> sort(const SkTHashSet<SkPDFIndirectReference>& src) {
    std::vector<SkPDFIndirectReference> dst;
    dst.reserve(src.count());
    src.foreach([&dst](SkPDFIndirectReference ref) { dst.push_back(ref); } );
    std::sort(dst.begin(), dst.end(),
            [](SkPDFIndirectReference a, SkPDFIndirectReference b) { return a.fValue < b.fValue; });
    return dst;
}

std::unique_ptr<SkPDFDict> SkPDFDevice::makeResourceDict() {
    return SkPDFMakeResourceDict(sort(fGraphicStateResources),
                                 sort(fShaderResources),
                                 sort(fXObjectResources),
                                 sort(fFontResources));
}

std::unique_ptr<SkStreamAsset> SkPDFDevice::content() {
    if (fActiveStackState.fContentStream) {
        fActiveStackState.drainStack();
        fActiveStackState = SkPDFGraphicStackState();
    }
    if (fContent.bytesWritten() == 0) {
        return skstd::make_unique<SkMemoryStream>();
    }
    SkDynamicMemoryWStream buffer;
    if (fInitialTransform.getType() != SkMatrix::kIdentity_Mask) {
        SkPDFUtils::AppendTransform(fInitialTransform, &buffer);
    }
    if (fNeedsExtraSave) {
        buffer.writeText("q\n");
    }
    fContent.writeToAndReset(&buffer);
    if (fNeedsExtraSave) {
        buffer.writeText("Q\n");
    }
    fNeedsExtraSave = false;
    return std::unique_ptr<SkStreamAsset>(buffer.detachAsStream());
}

/* Draws an inverse filled path by using Path Ops to compute the positive
 * inverse using the current clip as the inverse bounds.
 * Return true if this was an inverse path and was properly handled,
 * otherwise returns false and the normal drawing routine should continue,
 * either as a (incorrect) fallback or because the path was not inverse
 * in the first place.
 */
bool SkPDFDevice::handleInversePath(const SkPath& origPath,
                                    const SkPaint& paint,
                                    bool pathIsMutable) {
    if (!origPath.isInverseFillType()) {
        return false;
    }

    if (this->hasEmptyClip()) {
        return false;
    }

    SkPath modifiedPath;
    SkPath* pathPtr = const_cast<SkPath*>(&origPath);
    SkPaint noInversePaint(paint);

    // Merge stroking operations into final path.
    if (SkPaint::kStroke_Style == paint.getStyle() ||
        SkPaint::kStrokeAndFill_Style == paint.getStyle()) {
        bool doFillPath = paint.getFillPath(origPath, &modifiedPath);
        if (doFillPath) {
            noInversePaint.setStyle(SkPaint::kFill_Style);
            noInversePaint.setStrokeWidth(0);
            pathPtr = &modifiedPath;
        } else {
            // To be consistent with the raster output, hairline strokes
            // are rendered as non-inverted.
            modifiedPath.toggleInverseFillType();
            this->internalDrawPath(this->cs(), this->ctm(), modifiedPath, paint, true);
            return true;
        }
    }

    // Get bounds of clip in current transform space
    // (clip bounds are given in device space).
    SkMatrix transformInverse;
    SkMatrix totalMatrix = this->ctm();

    if (!totalMatrix.invert(&transformInverse)) {
        return false;
    }
    SkRect bounds = this->cs().bounds(this->bounds());
    transformInverse.mapRect(&bounds);

    // Extend the bounds by the line width (plus some padding)
    // so the edge doesn't cause a visible stroke.
    bounds.outset(paint.getStrokeWidth() + SK_Scalar1,
                  paint.getStrokeWidth() + SK_Scalar1);

    if (!calculate_inverse_path(bounds, *pathPtr, &modifiedPath)) {
        return false;
    }

    this->internalDrawPath(this->cs(), this->ctm(), modifiedPath, noInversePaint, true);
    return true;
}

SkPDFIndirectReference SkPDFDevice::makeFormXObjectFromDevice(SkIRect bounds, bool alpha) {
    SkMatrix inverseTransform = SkMatrix::I();
    if (!fInitialTransform.isIdentity()) {
        if (!fInitialTransform.invert(&inverseTransform)) {
            SkDEBUGFAIL("Layer initial transform should be invertible.");
            inverseTransform.reset();
        }
    }
    const char* colorSpace = alpha ? "DeviceGray" : nullptr;

    SkPDFIndirectReference xobject =
        SkPDFMakeFormXObject(fDocument, this->content(),
                             SkPDFMakeArray(bounds.left(), bounds.top(),
                                            bounds.right(), bounds.bottom()),
                             this->makeResourceDict(), inverseTransform, colorSpace);
    // We always draw the form xobjects that we create back into the device, so
    // we simply preserve the font usage instead of pulling it out and merging
    // it back in later.
    this->reset();
    return xobject;
}

SkPDFIndirectReference SkPDFDevice::makeFormXObjectFromDevice(bool alpha) {
    return this->makeFormXObjectFromDevice(SkIRect{0, 0, this->width(), this->height()}, alpha);
}

void SkPDFDevice::drawFormXObjectWithMask(SkPDFIndirectReference xObject,
                                          SkPDFIndirectReference sMask,
                                          SkBlendMode mode,
                                          bool invertClip) {
    SkASSERT(sMask);
    SkPaint paint;
    paint.setBlendMode(mode);
    ScopedContentEntry content(this, nullptr, SkMatrix::I(), paint);
    if (!content) {
        return;
    }
    this->setGraphicState(SkPDFGraphicState::GetSMaskGraphicState(
            sMask, invertClip, SkPDFGraphicState::kAlpha_SMaskMode,
            fDocument), content.stream());
    this->drawFormXObject(xObject, content.stream());
    this->clearMaskOnGraphicState(content.stream());
}


static bool treat_as_regular_pdf_blend_mode(SkBlendMode blendMode) {
    return nullptr != SkPDFUtils::BlendModeName(blendMode);
}

static void populate_graphic_state_entry_from_paint(
        SkPDFDocument* doc,
        const SkMatrix& matrix,
        const SkClipStack* clipStack,
        SkIRect deviceBounds,
        const SkPaint& paint,
        const SkMatrix& initialTransform,
        SkScalar textScale,
        SkPDFGraphicStackState::Entry* entry,
        SkTHashSet<SkPDFIndirectReference>* shaderResources,
        SkTHashSet<SkPDFIndirectReference>* graphicStateResources) {
    NOT_IMPLEMENTED(paint.getPathEffect() != nullptr, false);
    NOT_IMPLEMENTED(paint.getMaskFilter() != nullptr, false);
    NOT_IMPLEMENTED(paint.getColorFilter() != nullptr, false);

    entry->fMatrix = matrix;
    entry->fClipStackGenID = clipStack ? clipStack->getTopmostGenID()
                                       : SkClipStack::kWideOpenGenID;
    SkColor4f color = paint.getColor4f();
    entry->fColor = {color.fR, color.fG, color.fB, 1};
    entry->fShaderIndex = -1;

    // PDF treats a shader as a color, so we only set one or the other.
    SkShader* shader = paint.getShader();
    if (shader) {
        if (SkShader::kColor_GradientType == shader->asAGradient(nullptr)) {
            // We don't have to set a shader just for a color.
            SkShader::GradientInfo gradientInfo;
            SkColor gradientColor = SK_ColorBLACK;
            gradientInfo.fColors = &gradientColor;
            gradientInfo.fColorOffsets = nullptr;
            gradientInfo.fColorCount = 1;
            SkAssertResult(shader->asAGradient(&gradientInfo) == SkShader::kColor_GradientType);
            color = SkColor4f::FromColor(gradientColor);
            entry->fColor ={color.fR, color.fG, color.fB, 1};

        } else {
            // PDF positions patterns relative to the initial transform, so
            // we need to apply the current transform to the shader parameters.
            SkMatrix transform = matrix;
            transform.postConcat(initialTransform);

            // PDF doesn't support kClamp_TileMode, so we simulate it by making
            // a pattern the size of the current clip.
            SkRect clipStackBounds = clipStack ? clipStack->bounds(deviceBounds)
                                               : SkRect::Make(deviceBounds);

            // We need to apply the initial transform to bounds in order to get
            // bounds in a consistent coordinate system.
            initialTransform.mapRect(&clipStackBounds);
            SkIRect bounds;
            clipStackBounds.roundOut(&bounds);

            SkPDFIndirectReference pdfShader
                = SkPDFMakeShader(doc, shader, transform, bounds, paint.getColor4f());

            if (pdfShader) {
                // pdfShader has been canonicalized so we can directly compare pointers.
                entry->fShaderIndex = add_resource(*shaderResources, pdfShader);
            }
        }
    }

    SkPDFIndirectReference newGraphicState;
    if (color == paint.getColor4f()) {
        newGraphicState = SkPDFGraphicState::GetGraphicStateForPaint(doc, paint);
    } else {
        SkPaint newPaint = paint;
        newPaint.setColor4f(color, nullptr);
        newGraphicState = SkPDFGraphicState::GetGraphicStateForPaint(doc, newPaint);
    }
    entry->fGraphicStateIndex = add_resource(*graphicStateResources, newGraphicState);
    entry->fTextScaleX = textScale;
}

SkDynamicMemoryWStream* SkPDFDevice::setUpContentEntry(const SkClipStack* clipStack,
                                                       const SkMatrix& matrix,
                                                       const SkPaint& paint,
                                                       SkScalar textScale,
                                                       SkPDFIndirectReference* dst) {
    SkASSERT(!*dst);
    SkBlendMode blendMode = paint.getBlendMode();

    // Dst xfer mode doesn't draw source at all.
    if (blendMode == SkBlendMode::kDst) {
        return nullptr;
    }

    // For the following modes, we want to handle source and destination
    // separately, so make an object of what's already there.
    if (!treat_as_regular_pdf_blend_mode(blendMode) && blendMode != SkBlendMode::kDstOver) {
        if (!isContentEmpty()) {
            *dst = this->makeFormXObjectFromDevice();
            SkASSERT(isContentEmpty());
        } else if (blendMode != SkBlendMode::kSrc &&
                   blendMode != SkBlendMode::kSrcOut) {
            // Except for Src and SrcOut, if there isn't anything already there,
            // then we're done.
            return nullptr;
        }
    }
    // TODO(vandebo): Figure out how/if we can handle the following modes:
    // Xor, Plus.  For now, we treat them as SrcOver/Normal.

    if (treat_as_regular_pdf_blend_mode(blendMode)) {
        if (!fActiveStackState.fContentStream) {
            if (fContent.bytesWritten() != 0) {
                fContent.writeText("Q\nq\n");
                fNeedsExtraSave = true;
            }
            fActiveStackState = SkPDFGraphicStackState(&fContent);
        } else {
            SkASSERT(fActiveStackState.fContentStream = &fContent);
        }
    } else {
        fActiveStackState.drainStack();
        fActiveStackState = SkPDFGraphicStackState(&fContentBuffer);
    }
    SkASSERT(fActiveStackState.fContentStream);
    SkPDFGraphicStackState::Entry entry;
    populate_graphic_state_entry_from_paint(
            fDocument,
            matrix,
            clipStack,
            this->bounds(),
            paint,
            fInitialTransform,
            textScale,
            &entry,
            &fShaderResources,
            &fGraphicStateResources);
    fActiveStackState.updateClip(clipStack, this->bounds());
    fActiveStackState.updateMatrix(entry.fMatrix);
    fActiveStackState.updateDrawingState(entry);

    return fActiveStackState.fContentStream;
}

void SkPDFDevice::finishContentEntry(const SkClipStack* clipStack,
                                     SkBlendMode blendMode,
                                     SkPDFIndirectReference dst,
                                     SkPath* shape) {
    SkASSERT(blendMode != SkBlendMode::kDst);
    if (treat_as_regular_pdf_blend_mode(blendMode)) {
        SkASSERT(!dst);
        return;
    }

    SkASSERT(fActiveStackState.fContentStream);

    fActiveStackState.drainStack();
    fActiveStackState = SkPDFGraphicStackState();

    if (blendMode == SkBlendMode::kDstOver) {
        SkASSERT(!dst);
        if (fContentBuffer.bytesWritten() != 0) {
            if (fContent.bytesWritten() != 0) {
                fContentBuffer.writeText("Q\nq\n");
                fNeedsExtraSave = true;
            }
            fContentBuffer.prependToAndReset(&fContent);
            SkASSERT(fContentBuffer.bytesWritten() == 0);
        }
        return;
    }
    if (fContentBuffer.bytesWritten() != 0) {
        if (fContent.bytesWritten() != 0) {
            fContent.writeText("Q\nq\n");
            fNeedsExtraSave = true;
        }
        fContentBuffer.writeToAndReset(&fContent);
        SkASSERT(fContentBuffer.bytesWritten() == 0);
    }

    if (!dst) {
        SkASSERT(blendMode == SkBlendMode::kSrc ||
                 blendMode == SkBlendMode::kSrcOut);
        return;
    }

    SkASSERT(dst);
    // Changing the current content into a form-xobject will destroy the clip
    // objects which is fine since the xobject will already be clipped. However
    // if source has shape, we need to clip it too, so a copy of the clip is
    // saved.

    SkPaint stockPaint;

    SkPDFIndirectReference srcFormXObject;
    if (this->isContentEmpty()) {
        // If nothing was drawn and there's no shape, then the draw was a
        // no-op, but dst needs to be restored for that to be true.
        // If there is shape, then an empty source with Src, SrcIn, SrcOut,
        // DstIn, DstAtop or Modulate reduces to Clear and DstOut or SrcAtop
        // reduces to Dst.
        if (shape == nullptr || blendMode == SkBlendMode::kDstOut ||
                blendMode == SkBlendMode::kSrcATop) {
            ScopedContentEntry content(this, nullptr, SkMatrix::I(), stockPaint);
            this->drawFormXObject(dst, content.stream());
            return;
        } else {
            blendMode = SkBlendMode::kClear;
        }
    } else {
        srcFormXObject = this->makeFormXObjectFromDevice();
    }

    // TODO(vandebo) srcFormXObject may contain alpha, but here we want it
    // without alpha.
    if (blendMode == SkBlendMode::kSrcATop) {
        // TODO(vandebo): In order to properly support SrcATop we have to track
        // the shape of what's been drawn at all times. It's the intersection of
        // the non-transparent parts of the device and the outlines (shape) of
        // all images and devices drawn.
        this->drawFormXObjectWithMask(srcFormXObject, dst, SkBlendMode::kSrcOver, true);
    } else {
        if (shape != nullptr) {
            // Draw shape into a form-xobject.
            SkPaint filledPaint;
            filledPaint.setColor(SK_ColorBLACK);
            filledPaint.setStyle(SkPaint::kFill_Style);
            SkClipStack empty;
            SkPDFDevice shapeDev(this->size(), fDocument, fInitialTransform);
            shapeDev.internalDrawPath(clipStack ? *clipStack : empty,
                                      SkMatrix::I(), *shape, filledPaint, true);
            this->drawFormXObjectWithMask(dst, shapeDev.makeFormXObjectFromDevice(),
                                          SkBlendMode::kSrcOver, true);
        } else {
            this->drawFormXObjectWithMask(dst, srcFormXObject, SkBlendMode::kSrcOver, true);
        }
    }

    if (blendMode == SkBlendMode::kClear) {
        return;
    } else if (blendMode == SkBlendMode::kSrc ||
            blendMode == SkBlendMode::kDstATop) {
        ScopedContentEntry content(this, nullptr, SkMatrix::I(), stockPaint);
        if (content) {
            this->drawFormXObject(srcFormXObject, content.stream());
        }
        if (blendMode == SkBlendMode::kSrc) {
            return;
        }
    } else if (blendMode == SkBlendMode::kSrcATop) {
        ScopedContentEntry content(this, nullptr, SkMatrix::I(), stockPaint);
        if (content) {
            this->drawFormXObject(dst, content.stream());
        }
    }

    SkASSERT(blendMode == SkBlendMode::kSrcIn   ||
             blendMode == SkBlendMode::kDstIn   ||
             blendMode == SkBlendMode::kSrcOut  ||
             blendMode == SkBlendMode::kDstOut  ||
             blendMode == SkBlendMode::kSrcATop ||
             blendMode == SkBlendMode::kDstATop ||
             blendMode == SkBlendMode::kModulate);

    if (blendMode == SkBlendMode::kSrcIn ||
            blendMode == SkBlendMode::kSrcOut ||
            blendMode == SkBlendMode::kSrcATop) {
        this->drawFormXObjectWithMask(srcFormXObject, dst, SkBlendMode::kSrcOver,
                                      blendMode == SkBlendMode::kSrcOut);
        return;
    } else {
        SkBlendMode mode = SkBlendMode::kSrcOver;
        if (blendMode == SkBlendMode::kModulate) {
            this->drawFormXObjectWithMask(srcFormXObject, dst, SkBlendMode::kSrcOver, false);
            mode = SkBlendMode::kMultiply;
        }
        this->drawFormXObjectWithMask(dst, srcFormXObject, mode, blendMode == SkBlendMode::kDstOut);
        return;
    }
}

bool SkPDFDevice::isContentEmpty() {
    return fContent.bytesWritten() == 0 && fContentBuffer.bytesWritten() == 0;
}

static SkSize rect_to_size(const SkRect& r) { return {r.width(), r.height()}; }

static sk_sp<SkImage> color_filter(const SkImage* image,
                                   SkColorFilter* colorFilter) {
    auto surface =
        SkSurface::MakeRaster(SkImageInfo::MakeN32Premul(image->dimensions()));
    SkASSERT(surface);
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorTRANSPARENT);
    SkPaint paint;
    paint.setColorFilter(sk_ref_sp(colorFilter));
    canvas->drawImage(image, 0, 0, &paint);
    return surface->makeImageSnapshot();
}

////////////////////////////////////////////////////////////////////////////////

static bool is_integer(SkScalar x) {
    return x == SkScalarTruncToScalar(x);
}

static bool is_integral(const SkRect& r) {
    return is_integer(r.left()) &&
           is_integer(r.top()) &&
           is_integer(r.right()) &&
           is_integer(r.bottom());
}

void SkPDFDevice::internalDrawImageRect(SkKeyedImage imageSubset,
                                        const SkRect* src,
                                        const SkRect& dst,
                                        const SkPaint& srcPaint,
                                        const SkMatrix& ctm) {
    if (this->hasEmptyClip()) {
        return;
    }
    if (!imageSubset) {
        return;
    }

    // First, figure out the src->dst transform and subset the image if needed.
    SkIRect bounds = imageSubset.image()->bounds();
    SkRect srcRect = src ? *src : SkRect::Make(bounds);
    SkMatrix transform;
    transform.setRectToRect(srcRect, dst, SkMatrix::kFill_ScaleToFit);
    if (src && *src != SkRect::Make(bounds)) {
        if (!srcRect.intersect(SkRect::Make(bounds))) {
            return;
        }
        srcRect.roundOut(&bounds);
        transform.preTranslate(SkIntToScalar(bounds.x()),
                               SkIntToScalar(bounds.y()));
        if (bounds != imageSubset.image()->bounds()) {
            imageSubset = imageSubset.subset(bounds);
        }
        if (!imageSubset) {
            return;
        }
    }

    // If the image is opaque and the paint's alpha is too, replace
    // kSrc blendmode with kSrcOver.  http://crbug.com/473572
    SkTCopyOnFirstWrite<SkPaint> paint(srcPaint);
    if (SkBlendMode::kSrcOver != paint->getBlendMode() &&
        imageSubset.image()->isOpaque() &&
        kSrcOver_SkXfermodeInterpretation == SkInterpretXfermode(*paint, false))
    {
        paint.writable()->setBlendMode(SkBlendMode::kSrcOver);
    }

    // Alpha-only images need to get their color from the shader, before
    // applying the colorfilter.
    if (imageSubset.image()->isAlphaOnly() && paint->getColorFilter()) {
        // must blend alpha image and shader before applying colorfilter.
        auto surface =
            SkSurface::MakeRaster(SkImageInfo::MakeN32Premul(imageSubset.image()->dimensions()));
        SkCanvas* canvas = surface->getCanvas();
        SkPaint tmpPaint;
        // In the case of alpha images with shaders, the shader's coordinate
        // system is the image's coordiantes.
        tmpPaint.setShader(sk_ref_sp(paint->getShader()));
        tmpPaint.setColor4f(paint->getColor4f(), nullptr);
        canvas->clear(0x00000000);
        canvas->drawImage(imageSubset.image().get(), 0, 0, &tmpPaint);
        if (paint->getShader() != nullptr) {
            paint.writable()->setShader(nullptr);
        }
        imageSubset = SkKeyedImage(surface->makeImageSnapshot());
        SkASSERT(!imageSubset.image()->isAlphaOnly());
    }

    if (imageSubset.image()->isAlphaOnly()) {
        // The ColorFilter applies to the paint color/shader, not the alpha layer.
        SkASSERT(nullptr == paint->getColorFilter());

        sk_sp<SkImage> mask = alpha_image_to_greyscale_image(imageSubset.image().get());
        if (!mask) {
            return;
        }
        // PDF doesn't seem to allow masking vector graphics with an Image XObject.
        // Must mask with a Form XObject.
        sk_sp<SkPDFDevice> maskDevice = this->makeCongruentDevice();
        {
            SkCanvas canvas(maskDevice);
            // This clip prevents the mask image shader from covering
            // entire device if unnecessary.
            canvas.clipRect(this->cs().bounds(this->bounds()));
            canvas.concat(ctm);
            if (paint->getMaskFilter()) {
                SkPaint tmpPaint;
                tmpPaint.setShader(mask->makeShader(&transform));
                tmpPaint.setMaskFilter(sk_ref_sp(paint->getMaskFilter()));
                canvas.drawRect(dst, tmpPaint);
            } else {
                if (src && !is_integral(*src)) {
                    canvas.clipRect(dst);
                }
                canvas.concat(transform);
                canvas.drawImage(mask, 0, 0);
            }
        }
        SkIRect maskDeviceBounds = maskDevice->cs().bounds(maskDevice->bounds()).roundOut();
        if (!ctm.isIdentity() && paint->getShader()) {
            transform_shader(paint.writable(), ctm); // Since we are using identity matrix.
        }
        ScopedContentEntry content(this, &this->cs(), SkMatrix::I(), *paint);
        if (!content) {
            return;
        }
        this->setGraphicState(SkPDFGraphicState::GetSMaskGraphicState(
                maskDevice->makeFormXObjectFromDevice(maskDeviceBounds, true), false,
                SkPDFGraphicState::kLuminosity_SMaskMode, fDocument), content.stream());
        SkPDFUtils::AppendRectangle(SkRect::Make(this->size()), content.stream());
        SkPDFUtils::PaintPath(SkPaint::kFill_Style, SkPath::kWinding_FillType, content.stream());
        this->clearMaskOnGraphicState(content.stream());
        return;
    }
    if (paint->getMaskFilter()) {
        paint.writable()->setShader(imageSubset.image()->makeShader(&transform));
        SkPath path = to_path(dst); // handles non-integral clipping.
        this->internalDrawPath(this->cs(), this->ctm(), path, *paint, true);
        return;
    }
    transform.postConcat(ctm);

    bool needToRestore = false;
    if (src && !is_integral(*src)) {
        // Need sub-pixel clipping to fix https://bug.skia.org/4374
        this->cs().save();
        this->cs().clipRect(dst, ctm, SkClipOp::kIntersect, true);
        needToRestore = true;
    }
    SK_AT_SCOPE_EXIT(if (needToRestore) { this->cs().restore(); });

    SkMatrix matrix = transform;

    // Rasterize the bitmap using perspective in a new bitmap.
    if (transform.hasPerspective()) {
        // Transform the bitmap in the new space, without taking into
        // account the initial transform.
        SkRect imageBounds = SkRect::Make(imageSubset.image()->bounds());
        SkPath perspectiveOutline = to_path(imageBounds);
        perspectiveOutline.transform(transform);

        // TODO(edisonn): perf - use current clip too.
        // Retrieve the bounds of the new shape.
        SkRect bounds = perspectiveOutline.getBounds();

        // Transform the bitmap in the new space, taking into
        // account the initial transform.
        SkMatrix total = transform;
        total.postConcat(fInitialTransform);

        SkPath physicalPerspectiveOutline = to_path(imageBounds);
        physicalPerspectiveOutline.transform(total);

        SkRect physicalPerspectiveBounds =
                physicalPerspectiveOutline.getBounds();
        SkScalar scaleX = physicalPerspectiveBounds.width() / bounds.width();
        SkScalar scaleY = physicalPerspectiveBounds.height() / bounds.height();

        // TODO(edisonn): A better approach would be to use a bitmap shader
        // (in clamp mode) and draw a rect over the entire bounding box. Then
        // intersect perspectiveOutline to the clip. That will avoid introducing
        // alpha to the image while still giving good behavior at the edge of
        // the image.  Avoiding alpha will reduce the pdf size and generation
        // CPU time some.

        SkISize wh = rect_to_size(physicalPerspectiveBounds).toCeil();

        auto surface = SkSurface::MakeRaster(SkImageInfo::MakeN32Premul(wh));
        if (!surface) {
            return;
        }
        SkCanvas* canvas = surface->getCanvas();
        canvas->clear(SK_ColorTRANSPARENT);

        SkScalar deltaX = bounds.left();
        SkScalar deltaY = bounds.top();

        SkMatrix offsetMatrix = transform;
        offsetMatrix.postTranslate(-deltaX, -deltaY);
        offsetMatrix.postScale(scaleX, scaleY);

        // Translate the draw in the new canvas, so we perfectly fit the
        // shape in the bitmap.
        canvas->setMatrix(offsetMatrix);
        canvas->drawImage(imageSubset.image(), 0, 0);
        // Make sure the final bits are in the bitmap.
        surface->flush();

        // In the new space, we use the identity matrix translated
        // and scaled to reflect DPI.
        matrix.setScale(1 / scaleX, 1 / scaleY);
        matrix.postTranslate(deltaX, deltaY);

        imageSubset = SkKeyedImage(surface->makeImageSnapshot());
        if (!imageSubset) {
            return;
        }
    }

    SkMatrix scaled;
    // Adjust for origin flip.
    scaled.setScale(SK_Scalar1, -SK_Scalar1);
    scaled.postTranslate(0, SK_Scalar1);
    // Scale the image up from 1x1 to WxH.
    SkIRect subset = imageSubset.image()->bounds();
    scaled.postScale(SkIntToScalar(subset.width()),
                     SkIntToScalar(subset.height()));
    scaled.postConcat(matrix);
    ScopedContentEntry content(this, &this->cs(), scaled, *paint);
    if (!content) {
        return;
    }
    if (content.needShape()) {
        SkPath shape = to_path(SkRect::Make(subset));
        shape.transform(matrix);
        content.setShape(shape);
    }
    if (!content.needSource()) {
        return;
    }

    if (SkColorFilter* colorFilter = paint->getColorFilter()) {
        sk_sp<SkImage> img = color_filter(imageSubset.image().get(), colorFilter);
        imageSubset = SkKeyedImage(std::move(img));
        if (!imageSubset) {
            return;
        }
        // TODO(halcanary): de-dupe this by caching filtered images.
        // (maybe in the resource cache?)
    }

    SkBitmapKey key = imageSubset.key();
    SkPDFIndirectReference* pdfimagePtr = fDocument->fPDFBitmapMap.find(key);
    SkPDFIndirectReference pdfimage = pdfimagePtr ? *pdfimagePtr : SkPDFIndirectReference();
    if (!pdfimagePtr) {
        SkASSERT(imageSubset);
        pdfimage = SkPDFSerializeImage(imageSubset.image().get(), fDocument,
                                       fDocument->metadata().fEncodingQuality);
        SkASSERT((key != SkBitmapKey{{0, 0, 0, 0}, 0}));
        fDocument->fPDFBitmapMap.set(key, pdfimage);
    }
    SkASSERT(pdfimage != SkPDFIndirectReference());
    this->drawFormXObject(pdfimage, content.stream());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "include/core/SkImageFilter.h"
#include "src/core/SkSpecialImage.h"

void SkPDFDevice::drawSpecial(SkSpecialImage* srcImg, int x, int y, const SkPaint& paint,
                              SkImage* clipImage, const SkMatrix& clipMatrix) {
    if (this->hasEmptyClip()) {
        return;
    }
    SkASSERT(!srcImg->isTextureBacked());

    //TODO: clipImage support

    SkBitmap resultBM;

    SkImageFilter* filter = paint.getImageFilter();
    if (filter) {
        SkIPoint offset = SkIPoint::Make(0, 0);
        SkMatrix matrix = this->ctm();
        matrix.postTranslate(SkIntToScalar(-x), SkIntToScalar(-y));
        const SkIRect clipBounds =
            this->cs().bounds(this->bounds()).roundOut().makeOffset(-x, -y);
        sk_sp<SkImageFilterCache> cache(this->getImageFilterCache());
        // TODO: Should PDF be operating in a specified color type/space? For now, run the filter
        // in the same color space as the source (this is different from all other backends).
        SkImageFilter::OutputProperties outputProperties(kN32_SkColorType, srcImg->getColorSpace());
        SkImageFilter::Context ctx(matrix, clipBounds, cache.get(), outputProperties);

        sk_sp<SkSpecialImage> resultImg(filter->filterImage(srcImg, ctx, &offset));
        if (resultImg) {
            SkPaint tmpUnfiltered(paint);
            tmpUnfiltered.setImageFilter(nullptr);
            if (resultImg->getROPixels(&resultBM)) {
                this->drawSprite(resultBM, x + offset.x(), y + offset.y(), tmpUnfiltered);
            }
        }
    } else {
        if (srcImg->getROPixels(&resultBM)) {
            this->drawSprite(resultBM, x, y, paint);
        }
    }
}

sk_sp<SkSpecialImage> SkPDFDevice::makeSpecial(const SkBitmap& bitmap) {
    return SkSpecialImage::MakeFromRaster(bitmap.bounds(), bitmap);
}

sk_sp<SkSpecialImage> SkPDFDevice::makeSpecial(const SkImage* image) {
    return SkSpecialImage::MakeFromImage(nullptr, image->bounds(), image->makeNonTextureImage());
}

sk_sp<SkSpecialImage> SkPDFDevice::snapSpecial() {
    return nullptr;
}

SkImageFilterCache* SkPDFDevice::getImageFilterCache() {
    // We always return a transient cache, so it is freed after each
    // filter traversal.
    return SkImageFilterCache::Create(SkImageFilterCache::kDefaultTransientSize);
}
