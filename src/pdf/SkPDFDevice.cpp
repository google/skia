/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPDFDevice.h"

#include "SkAdvancedTypefaceMetrics.h"
#include "SkAnnotationKeys.h"
#include "SkBitmapDevice.h"
#include "SkBitmapKey.h"
#include "SkCanvas.h"
#include "SkClipOpPriv.h"
#include "SkColor.h"
#include "SkColorFilter.h"
#include "SkDraw.h"
#include "SkDrawFilter.h"
#include "SkGlyphCache.h"
#include "SkImageFilterCache.h"
#include "SkJpegEncoder.h"
#include "SkMakeUnique.h"
#include "SkMaskFilter.h"
#include "SkPDFBitmap.h"
#include "SkPDFCanon.h"
#include "SkPDFDocument.h"
#include "SkPDFFont.h"
#include "SkPDFFormXObject.h"
#include "SkPDFGraphicState.h"
#include "SkPDFResourceDict.h"
#include "SkPDFShader.h"
#include "SkPDFTypes.h"
#include "SkPDFUtils.h"
#include "SkPath.h"
#include "SkPathEffect.h"
#include "SkPathOps.h"
#include "SkPixelRef.h"
#include "SkRRect.h"
#include "SkRasterClip.h"
#include "SkScopeExit.h"
#include "SkString.h"
#include "SkSurface.h"
#include "SkTemplates.h"
#include "SkTextBlobRunIterator.h"
#include "SkTextFormatParams.h"
#include "SkUtils.h"
#include "SkXfermodeInterpretation.h"

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

// If the paint will definitely draw opaquely, replace kSrc with
// kSrcOver.  http://crbug.com/473572
static void replace_srcmode_on_opaque_paint(SkPaint* paint) {
    if (kSrcOver_SkXfermodeInterpretation == SkInterpretXfermode(*paint, false)) {
        paint->setBlendMode(SkBlendMode::kSrcOver);
    }
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
    SkMatrix lm = SkPDFUtils::GetShaderLocalMatrix(paint->getShader());
    SkMatrix lmInv;
    if (lm.invert(&lmInv)) {
        SkMatrix m = SkMatrix::Concat(SkMatrix::Concat(lmInv, ctm), lm);
        paint->setShader(paint->getShader()->makeWithLocalMatrix(m));
    }
}

static void emit_pdf_color(SkColor color, SkWStream* result) {
    SkASSERT(SkColorGetA(color) == 0xFF);  // We handle alpha elsewhere.
    SkPDFUtils::AppendColorComponent(SkColorGetR(color), result);
    result->writeText(" ");
    SkPDFUtils::AppendColorComponent(SkColorGetG(color), result);
    result->writeText(" ");
    SkPDFUtils::AppendColorComponent(SkColorGetB(color), result);
    result->writeText(" ");
}

static SkPaint calculate_text_paint(const SkPaint& paint) {
    SkPaint result = paint;
    if (result.isFakeBoldText()) {
        SkScalar fakeBoldScale = SkScalarInterpFunc(result.getTextSize(),
                                                    kStdFakeBoldInterpKeys,
                                                    kStdFakeBoldInterpValues,
                                                    kStdFakeBoldInterpLength);
        SkScalar width = result.getTextSize() * fakeBoldScale;
        if (result.getStyle() == SkPaint::kFill_Style) {
            result.setStyle(SkPaint::kStrokeAndFill_Style);
        } else {
            width += result.getStrokeWidth();
        }
        result.setStrokeWidth(width);
    }
    return result;
}


// If the paint has a color filter, apply the color filter to the shader or the
// paint color.  Remove the color filter.
void remove_color_filter(SkPaint* paint) {
    if (SkColorFilter* cf = paint->getColorFilter()) {
        if (SkShader* shader = paint->getShader()) {
            paint->setShader(shader->makeWithColorFilter(paint->refColorFilter()));
        } else {
            paint->setColor(cf->filterColor(paint->getColor()));
        }
        paint->setColorFilter(nullptr);
    }
}

SkPDFDevice::GraphicStateEntry::GraphicStateEntry()
    : fColor(SK_ColorBLACK)
    , fTextScaleX(SK_Scalar1)
    , fTextFill(SkPaint::kFill_Style)
    , fShaderIndex(-1)
    , fGraphicStateIndex(-1) {
    fMatrix.reset();
}

bool SkPDFDevice::GraphicStateEntry::compareInitialState(
        const GraphicStateEntry& cur) {
    return fColor == cur.fColor &&
           fShaderIndex == cur.fShaderIndex &&
           fGraphicStateIndex == cur.fGraphicStateIndex &&
           fMatrix == cur.fMatrix &&
           fClipStack == cur.fClipStack &&
           (fTextScaleX == 0 ||
               (fTextScaleX == cur.fTextScaleX && fTextFill == cur.fTextFill));
}

class GraphicStackState {
public:
    GraphicStackState(const SkClipStack& existingClipStack,
                      SkWStream* contentStream)
            : fStackDepth(0),
              fContentStream(contentStream) {
        fEntries[0].fClipStack = existingClipStack;
    }

    void updateClip(const SkClipStack& clipStack,
                    const SkPoint& translation, const SkRect& bounds);
    void updateMatrix(const SkMatrix& matrix);
    void updateDrawingState(const SkPDFDevice::GraphicStateEntry& state);

    void drainStack();

private:
    void push();
    void pop();
    SkPDFDevice::GraphicStateEntry* currentEntry() { return &fEntries[fStackDepth]; }

    // Conservative limit on save depth, see impl. notes in PDF 1.4 spec.
    static const int kMaxStackDepth = 12;
    SkPDFDevice::GraphicStateEntry fEntries[kMaxStackDepth + 1];
    int fStackDepth;
    SkWStream* fContentStream;
};

void GraphicStackState::drainStack() {
    while (fStackDepth) {
        pop();
    }
}

void GraphicStackState::push() {
    SkASSERT(fStackDepth < kMaxStackDepth);
    fContentStream->writeText("q\n");
    fStackDepth++;
    fEntries[fStackDepth] = fEntries[fStackDepth - 1];
}

void GraphicStackState::pop() {
    SkASSERT(fStackDepth > 0);
    fContentStream->writeText("Q\n");
    fStackDepth--;
}

/* Calculate an inverted path's equivalent non-inverted path, given the
 * canvas bounds.
 * outPath may alias with invPath (since this is supported by PathOps).
 */
static bool calculate_inverse_path(const SkRect& bounds, const SkPath& invPath,
                                   SkPath* outPath) {
    SkASSERT(invPath.isInverseFillType());

    SkPath clipPath;
    clipPath.addRect(bounds);

    return Op(clipPath, invPath, kIntersect_SkPathOp, outPath);
}

bool apply_clip(SkClipOp op, const SkPath& u, const SkPath& v, SkPath* r)  {
    switch (op) {
        case SkClipOp::kDifference:
            return Op(u, v, kDifference_SkPathOp, r);
        case SkClipOp::kIntersect:
            return Op(u, v, kIntersect_SkPathOp, r);
#ifdef SK_SUPPORT_DEPRECATED_CLIPOPS
        case SkClipOp::kUnion_deprecated:
            return Op(u, v, kUnion_SkPathOp, r);
        case SkClipOp::kXOR_deprecated:
            return Op(u, v, kXOR_SkPathOp, r);
        case SkClipOp::kReverseDifference_deprecated:
            return Op(u, v, kReverseDifference_SkPathOp, r);
        case SkClipOp::kReplace_deprecated:
            *r = v;
            return true;
#endif
        default:
            return false;
    }
}

/* Uses Path Ops to calculate a vector SkPath clip from a clip stack.
 * Returns true if successful, or false if not successful.
 * If successful, the resulting clip is stored in outClipPath.
 * If not successful, outClipPath is undefined, and a fallback method
 * should be used.
 */
static bool get_clip_stack_path(const SkMatrix& transform,
                                const SkClipStack& clipStack,
                                const SkRect& bounds,
                                SkPath* outClipPath) {
    outClipPath->reset();
    outClipPath->setFillType(SkPath::kInverseWinding_FillType);

    const SkClipStack::Element* clipEntry;
    SkClipStack::Iter iter;
    iter.reset(clipStack, SkClipStack::Iter::kBottom_IterStart);
    for (clipEntry = iter.next(); clipEntry; clipEntry = iter.next()) {
        SkPath entryPath;
        if (SkClipStack::Element::kEmpty_Type == clipEntry->getType()) {
            outClipPath->reset();
            outClipPath->setFillType(SkPath::kInverseWinding_FillType);
            continue;
        } else {
            clipEntry->asPath(&entryPath);
        }
        entryPath.transform(transform);
        if (!apply_clip(clipEntry->getOp(), *outClipPath, entryPath, outClipPath)) {
            return false;
        }
    }

    if (outClipPath->isInverseFillType()) {
        // The bounds are slightly outset to ensure this is correct in the
        // face of floating-point accuracy and possible SkRegion bitmap
        // approximations.
        SkRect clipBounds = bounds;
        clipBounds.outset(SK_Scalar1, SK_Scalar1);
        if (!calculate_inverse_path(clipBounds, *outClipPath, outClipPath)) {
            return false;
        }
    }
    return true;
}

// TODO(vandebo): Take advantage of SkClipStack::getSaveCount(), the PDF
// graphic state stack, and the fact that we can know all the clips used
// on the page to optimize this.
void GraphicStackState::updateClip(const SkClipStack& clipStack,
                                   const SkPoint& translation,
                                   const SkRect& bounds) {
    if (clipStack == currentEntry()->fClipStack) {
        return;
    }

    while (fStackDepth > 0) {
        pop();
        if (clipStack == currentEntry()->fClipStack) {
            return;
        }
    }
    push();

    currentEntry()->fClipStack = clipStack;

    SkMatrix transform;
    transform.setTranslate(translation.fX, translation.fY);

    SkPath clipPath;
    if (get_clip_stack_path(transform, clipStack, bounds, &clipPath)) {
        SkPDFUtils::EmitPath(clipPath, SkPaint::kFill_Style, fContentStream);
        SkPath::FillType clipFill = clipPath.getFillType();
        NOT_IMPLEMENTED(clipFill == SkPath::kInverseEvenOdd_FillType, false);
        NOT_IMPLEMENTED(clipFill == SkPath::kInverseWinding_FillType, false);
        if (clipFill == SkPath::kEvenOdd_FillType) {
            fContentStream->writeText("W* n\n");
        } else {
            fContentStream->writeText("W n\n");
        }
    }
    // If Op() fails (pathological case; e.g. input values are
    // extremely large or NaN), emit no clip at all.
}

void GraphicStackState::updateMatrix(const SkMatrix& matrix) {
    if (matrix == currentEntry()->fMatrix) {
        return;
    }

    if (currentEntry()->fMatrix.getType() != SkMatrix::kIdentity_Mask) {
        SkASSERT(fStackDepth > 0);
        SkASSERT(fEntries[fStackDepth].fClipStack ==
                 fEntries[fStackDepth -1].fClipStack);
        pop();

        SkASSERT(currentEntry()->fMatrix.getType() == SkMatrix::kIdentity_Mask);
    }
    if (matrix.getType() == SkMatrix::kIdentity_Mask) {
        return;
    }

    push();
    SkPDFUtils::AppendTransform(matrix, fContentStream);
    currentEntry()->fMatrix = matrix;
}

void GraphicStackState::updateDrawingState(const SkPDFDevice::GraphicStateEntry& state) {
    // PDF treats a shader as a color, so we only set one or the other.
    if (state.fShaderIndex >= 0) {
        if (state.fShaderIndex != currentEntry()->fShaderIndex) {
            SkPDFUtils::ApplyPattern(state.fShaderIndex, fContentStream);
            currentEntry()->fShaderIndex = state.fShaderIndex;
        }
    } else {
        if (state.fColor != currentEntry()->fColor ||
                currentEntry()->fShaderIndex >= 0) {
            emit_pdf_color(state.fColor, fContentStream);
            fContentStream->writeText("RG ");
            emit_pdf_color(state.fColor, fContentStream);
            fContentStream->writeText("rg\n");
            currentEntry()->fColor = state.fColor;
            currentEntry()->fShaderIndex = -1;
        }
    }

    if (state.fGraphicStateIndex != currentEntry()->fGraphicStateIndex) {
        SkPDFUtils::ApplyGraphicState(state.fGraphicStateIndex, fContentStream);
        currentEntry()->fGraphicStateIndex = state.fGraphicStateIndex;
    }

    if (state.fTextScaleX) {
        if (state.fTextScaleX != currentEntry()->fTextScaleX) {
            SkScalar pdfScale = state.fTextScaleX * 1000;
            SkPDFUtils::AppendScalar(pdfScale, fContentStream);
            fContentStream->writeText(" Tz\n");
            currentEntry()->fTextScaleX = state.fTextScaleX;
        }
        if (state.fTextFill != currentEntry()->fTextFill) {
            static_assert(SkPaint::kFill_Style == 0, "enum_must_match_value");
            static_assert(SkPaint::kStroke_Style == 1, "enum_must_match_value");
            static_assert(SkPaint::kStrokeAndFill_Style == 2, "enum_must_match_value");
            fContentStream->writeDecAsText(state.fTextFill);
            fContentStream->writeText(" Tr\n");
            currentEntry()->fTextFill = state.fTextFill;
        }
    }
}

static bool not_supported_for_layers(const SkPaint& layerPaint) {
    // PDF does not support image filters, so render them on CPU.
    // Note that this rendering is done at "screen" resolution (100dpi), not
    // printer resolution.
    // TODO: It may be possible to express some filters natively using PDF
    // to improve quality and file size (https://bug.skia.org/3043)

    // TODO: should we return true if there is a colorfilter?
    return layerPaint.getImageFilter() != nullptr;
}

SkBaseDevice* SkPDFDevice::onCreateDevice(const CreateInfo& cinfo, const SkPaint* layerPaint) {
    if (layerPaint && not_supported_for_layers(*layerPaint)) {
        // need to return a raster device, which we will detect in drawDevice()
        return SkBitmapDevice::Create(cinfo.fInfo, SkSurfaceProps(0, kUnknown_SkPixelGeometry));
    }
    return new SkPDFDevice(cinfo.fInfo.dimensions(), fDocument);
}

SkPDFCanon* SkPDFDevice::getCanon() const { return fDocument->canon(); }

// A helper class to automatically finish a ContentEntry at the end of a
// drawing method and maintain the state needed between set up and finish.
class ScopedContentEntry {
public:
    ScopedContentEntry(SkPDFDevice* device,
                       const SkClipStack& clipStack,
                       const SkMatrix& matrix,
                       const SkPaint& paint,
                       bool hasText = false)
        : fDevice(device)
        , fContentEntry(nullptr)
        , fBlendMode(SkBlendMode::kSrcOver)
        , fDstFormXObject(nullptr)
    {
        if (matrix.hasPerspective()) {
            NOT_IMPLEMENTED(!matrix.hasPerspective(), false);
            return;
        }
        fBlendMode = paint.getBlendMode();
        fContentEntry =
            fDevice->setUpContentEntry(clipStack, matrix, paint, hasText, &fDstFormXObject);
    }
    ScopedContentEntry(SkPDFDevice* dev, const SkPaint& paint, bool hasText = false)
        : ScopedContentEntry(dev, dev->cs(), dev->ctm(), paint, hasText) {}

    ~ScopedContentEntry() {
        if (fContentEntry) {
            SkPath* shape = &fShape;
            if (shape->isEmpty()) {
                shape = nullptr;
            }
            fDevice->finishContentEntry(fBlendMode, std::move(fDstFormXObject), shape);
        }
    }

    SkPDFDevice::ContentEntry* entry() { return fContentEntry; }
    SkDynamicMemoryWStream* stream() { return &fContentEntry->fContent; }

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
    SkPDFDevice* fDevice;
    SkPDFDevice::ContentEntry* fContentEntry;
    SkBlendMode fBlendMode;
    sk_sp<SkPDFObject> fDstFormXObject;
    SkPath fShape;
};

////////////////////////////////////////////////////////////////////////////////

SkPDFDevice::SkPDFDevice(SkISize pageSize, SkPDFDocument* doc)
    : INHERITED(SkImageInfo::MakeUnknown(pageSize.width(), pageSize.height()),
                SkSurfaceProps(0, kUnknown_SkPixelGeometry))
    , fPageSize(pageSize)
    , fInitialTransform(SkMatrix::I())
    , fDocument(doc)
{
    SkASSERT(!pageSize.isEmpty());
}

void SkPDFDevice::setFlip() {
    // Skia generally uses the top left as the origin but PDF
    // natively has the origin at the bottom left. This matrix
    // corrects for that.  But that only needs to be done once, we
    // don't do it when layering.
    fInitialTransform.setTranslate(0, SkIntToScalar(fPageSize.fHeight));
    fInitialTransform.preScale(SK_Scalar1, -SK_Scalar1);
}

SkPDFDevice::~SkPDFDevice() {
    this->cleanUp();
}

void SkPDFDevice::init() {
    fContentEntries.reset();
}

void SkPDFDevice::cleanUp() {
    fGraphicStateResources.unrefAll();
    fXObjectResources.unrefAll();
    fFontResources.unrefAll();
    fShaderResources.unrefAll();
}

void SkPDFDevice::drawAnnotation(const SkRect& rect, const char key[], SkData* value) {
    if (!value) {
        return;
    }
    if (rect.isEmpty()) {
        if (!strcmp(SkAnnotationKeys::Define_Named_Dest_Key(), key)) {
            SkPoint transformedPoint;
            this->ctm().mapXY(rect.x(), rect.y(), &transformedPoint);
            fNamedDestinations.emplace_back(NamedDestination{sk_ref_sp(value), transformedPoint});
        }
        return;
    }
    // Convert to path to handle non-90-degree rotations.
    SkPath path;
    path.addRect(rect);
    path.transform(this->ctm(), &path);
    SkPath clip;
    (void)this->cs().asPath(&clip);
    Op(clip, path, kIntersect_SkPathOp, &path);
    // PDF wants a rectangle only.
    SkRect transformedRect = path.getBounds();
    if (transformedRect.isEmpty()) {
        return;
    }
    if (!strcmp(SkAnnotationKeys::URL_Key(), key)) {
        fLinkToURLs.emplace_back(RectWithData{transformedRect, sk_ref_sp(value)});
    } else if (!strcmp(SkAnnotationKeys::Link_Named_Dest_Key(), key)) {
        fLinkToDestinations.emplace_back(RectWithData{transformedRect, sk_ref_sp(value)});
    }
}

void SkPDFDevice::drawPaint(const SkPaint& srcPaint) {
    SkPaint newPaint = srcPaint;
    remove_color_filter(&newPaint);
    replace_srcmode_on_opaque_paint(&newPaint);
    newPaint.setStyle(SkPaint::kFill_Style);

    SkMatrix ctm = this->ctm();
    if (ctm.getType() & SkMatrix::kPerspective_Mask) {
        if (newPaint.getShader()) {
            transform_shader(&newPaint, ctm);
        }
        ctm = SkMatrix::I();
    }
    ScopedContentEntry content(this, this->cs(), ctm, newPaint);
    this->internalDrawPaint(newPaint, content.entry());
}

void SkPDFDevice::internalDrawPaint(const SkPaint& paint,
                                    SkPDFDevice::ContentEntry* contentEntry) {
    if (!contentEntry) {
        return;
    }
    SkRect bbox = SkRect::Make(fPageSize);
    SkMatrix inverse;
    if (!contentEntry->fState.fMatrix.invert(&inverse)) {
        return;
    }
    inverse.mapRect(&bbox);

    SkPDFUtils::AppendRectangle(bbox, &contentEntry->fContent);
    SkPDFUtils::PaintPath(paint.getStyle(), SkPath::kWinding_FillType,
                          &contentEntry->fContent);
}

void SkPDFDevice::drawPoints(SkCanvas::PointMode mode,
                             size_t count,
                             const SkPoint* points,
                             const SkPaint& srcPaint) {
    SkPaint passedPaint = srcPaint;
    remove_color_filter(&passedPaint);
    replace_srcmode_on_opaque_paint(&passedPaint);
    if (SkCanvas::kPoints_PointMode != mode) {
        passedPaint.setStyle(SkPaint::kStroke_Style);
    }
    if (count == 0) {
        return;
    }

    // SkDraw::drawPoints converts to multiple calls to fDevice->drawPath.
    // We only use this when there's a path effect because of the overhead
    // of multiple calls to setUpContentEntry it causes.
    if (passedPaint.getPathEffect()) {
        if (this->cs().isEmpty(this->bounds())) {
            return;
        }
        draw_points(mode, count, points, passedPaint,
                    this->devClipBounds(), this->ctm(), this);
        return;
    }

    const SkPaint* paint = &passedPaint;
    SkPaint modifiedPaint;

    if (mode == SkCanvas::kPoints_PointMode &&
            paint->getStrokeCap() != SkPaint::kRound_Cap) {
        modifiedPaint = *paint;
        paint = &modifiedPaint;
        if (paint->getStrokeWidth()) {
            // PDF won't draw a single point with square/butt caps because the
            // orientation is ambiguous.  Draw a rectangle instead.
            modifiedPaint.setStyle(SkPaint::kFill_Style);
            SkScalar strokeWidth = paint->getStrokeWidth();
            SkScalar halfStroke = SkScalarHalf(strokeWidth);
            for (size_t i = 0; i < count; i++) {
                SkRect r = SkRect::MakeXYWH(points[i].fX, points[i].fY, 0, 0);
                r.inset(-halfStroke, -halfStroke);
                this->drawRect(r, modifiedPaint);
            }
            return;
        } else {
            modifiedPaint.setStrokeCap(SkPaint::kRound_Cap);
        }
    }

    ScopedContentEntry content(this, *paint);
    if (!content.entry()) {
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

static sk_sp<SkPDFDict> create_link_annotation(const SkRect& translatedRect) {
    auto annotation = sk_make_sp<SkPDFDict>("Annot");
    annotation->insertName("Subtype", "Link");
    annotation->insertInt("F", 4);  // required by ISO 19005

    auto border = sk_make_sp<SkPDFArray>();
    border->reserve(3);
    border->appendInt(0);  // Horizontal corner radius.
    border->appendInt(0);  // Vertical corner radius.
    border->appendInt(0);  // Width, 0 = no border.
    annotation->insertObject("Border", std::move(border));

    auto rect = sk_make_sp<SkPDFArray>();
    rect->reserve(4);
    rect->appendScalar(translatedRect.fLeft);
    rect->appendScalar(translatedRect.fTop);
    rect->appendScalar(translatedRect.fRight);
    rect->appendScalar(translatedRect.fBottom);
    annotation->insertObject("Rect", std::move(rect));

    return annotation;
}

static sk_sp<SkPDFDict> create_link_to_url(const SkData* urlData, const SkRect& r) {
    sk_sp<SkPDFDict> annotation = create_link_annotation(r);
    SkString url(static_cast<const char *>(urlData->data()),
                 urlData->size() - 1);
    auto action = sk_make_sp<SkPDFDict>("Action");
    action->insertName("S", "URI");
    action->insertString("URI", url);
    annotation->insertObject("A", std::move(action));
    return annotation;
}

static sk_sp<SkPDFDict> create_link_named_dest(const SkData* nameData,
                                               const SkRect& r) {
    sk_sp<SkPDFDict> annotation = create_link_annotation(r);
    SkString name(static_cast<const char *>(nameData->data()),
                  nameData->size() - 1);
    annotation->insertName("Dest", name);
    return annotation;
}

void SkPDFDevice::drawRect(const SkRect& rect,
                           const SkPaint& srcPaint) {
    SkPaint paint = srcPaint;
    remove_color_filter(&paint);
    replace_srcmode_on_opaque_paint(&paint);
    SkRect r = rect;
    r.sort();

    if (paint.getPathEffect() || paint.getMaskFilter()) {
        if (this->cs().isEmpty(this->bounds())) {
            return;
        }
        SkPath path;
        path.addRect(r);
        this->drawPath(path, paint, nullptr, true);
        return;
    }

    ScopedContentEntry content(this, paint);
    if (!content.entry()) {
        return;
    }
    SkPDFUtils::AppendRectangle(r, content.stream());
    SkPDFUtils::PaintPath(paint.getStyle(), SkPath::kWinding_FillType, content.stream());
}

void SkPDFDevice::drawRRect(const SkRRect& rrect,
                            const SkPaint& srcPaint) {
    SkPaint paint = srcPaint;
    remove_color_filter(&paint);
    replace_srcmode_on_opaque_paint(&paint);
    SkPath  path;
    path.addRRect(rrect);
    this->drawPath(path, paint, nullptr, true);
}

void SkPDFDevice::drawOval(const SkRect& oval,
                           const SkPaint& srcPaint) {
    SkPaint paint = srcPaint;
    remove_color_filter(&paint);
    replace_srcmode_on_opaque_paint(&paint);
    SkPath  path;
    path.addOval(oval);
    this->drawPath(path, paint, nullptr, true);
}

void SkPDFDevice::drawPath(const SkPath& origPath,
                           const SkPaint& srcPaint,
                           const SkMatrix* prePathMatrix,
                           bool pathIsMutable) {
    this->internalDrawPath(
            this->cs(), this->ctm(), origPath, srcPaint, prePathMatrix, pathIsMutable);
}

void SkPDFDevice::internalDrawPathWithFilter(const SkClipStack& clipStack,
                                             const SkMatrix& ctm,
                                             const SkPath& origPath,
                                             const SkPaint& origPaint,
                                             const SkMatrix* prePathMatrix) {
    SkASSERT(origPaint.getMaskFilter());
    SkPath path(origPath);
    SkTCopyOnFirstWrite<SkPaint> paint(origPaint);
    if (prePathMatrix) {
        path.transform(*prePathMatrix, &path);
    }
    SkStrokeRec::InitStyle initStyle = paint->getFillPath(path, &path)
                                     ? SkStrokeRec::kFill_InitStyle
                                     : SkStrokeRec::kHairline_InitStyle;
    path.transform(ctm, &path);

    // TODO(halcanary): respect fDocument->rasterDpi().
    //        SkScalar rasterScale = (float)rasterDpi / SkPDFUtils::kDpiForRasterScaleOne;
    // Would it be easier to just change the device size (and pre-scale the canvas)?
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
    if (!paint->getMaskFilter()->filterMask(&dstMask, sourceMask, ctm, &margin)) {
        return;
    }
    SkIRect dstMaskBounds = dstMask.fBounds;
    sk_sp<SkImage> mask = mask_to_greyscale_image(&dstMask);
    // PDF doesn't seem to allow masking vector graphics with an Image XObject.
    // Must mask with a Form XObject.
    sk_sp<SkPDFDevice> maskDevice = this->makeCongruentDevice();
    {
        SkCanvas canvas(maskDevice.get());
        canvas.drawImage(mask, dstMaskBounds.x(), dstMaskBounds.y());
    }
    if (!ctm.isIdentity() && paint->getShader()) {
        transform_shader(paint.writable(), ctm); // Since we are using identity matrix.
    }
    ScopedContentEntry content(this, clipStack, SkMatrix::I(), *paint);
    if (!content.entry()) {
        return;
    }
    this->addSMaskGraphicState(std::move(maskDevice), content.stream());
    SkPDFUtils::AppendRectangle(SkRect::Make(dstMaskBounds), content.stream());
    SkPDFUtils::PaintPath(SkPaint::kFill_Style, path.getFillType(), content.stream());
    this->clearMaskOnGraphicState(content.stream());
}

void SkPDFDevice::addSMaskGraphicState(sk_sp<SkPDFDevice> maskDevice,
                                       SkDynamicMemoryWStream* contentStream) {
    sk_sp<SkPDFDict> sMaskGS = SkPDFGraphicState::GetSMaskGraphicState(
            maskDevice->makeFormXObjectFromDevice(true), false,
            SkPDFGraphicState::kLuminosity_SMaskMode, this->getCanon());
    SkPDFUtils::ApplyGraphicState(this->addGraphicStateResource(sMaskGS.get()), contentStream);
}

void SkPDFDevice::clearMaskOnGraphicState(SkDynamicMemoryWStream* contentStream) {
    // The no-softmask graphic state is used to "turn off" the mask for later draw calls.
    sk_sp<SkPDFDict>& noSMaskGS = this->getCanon()->fNoSmaskGraphicState;
    if (!noSMaskGS) {
        noSMaskGS = sk_make_sp<SkPDFDict>("ExtGState");
        noSMaskGS->insertName("SMask", "None");
    }
    SkPDFUtils::ApplyGraphicState(this->addGraphicStateResource(noSMaskGS.get()), contentStream);
}

void SkPDFDevice::internalDrawPath(const SkClipStack& clipStack,
                                   const SkMatrix& ctm,
                                   const SkPath& origPath,
                                   const SkPaint& srcPaint,
                                   const SkMatrix* prePathMatrix,
                                   bool pathIsMutable) {
    SkPaint paint = srcPaint;
    remove_color_filter(&paint);
    replace_srcmode_on_opaque_paint(&paint);
    SkPath modifiedPath;
    SkPath* pathPtr = const_cast<SkPath*>(&origPath);

    if (paint.getMaskFilter()) {
        this->internalDrawPathWithFilter(clipStack, ctm, origPath, paint, prePathMatrix);
        return;
    }

    SkMatrix matrix = ctm;
    if (prePathMatrix) {
        if (paint.getPathEffect() || paint.getStyle() != SkPaint::kFill_Style) {
            if (!pathIsMutable) {
                pathPtr = &modifiedPath;
                pathIsMutable = true;
            }
            origPath.transform(*prePathMatrix, pathPtr);
        } else {
            matrix.preConcat(*prePathMatrix);
        }
    }

    if (paint.getPathEffect()) {
        if (clipStack.isEmpty(this->bounds())) {
            return;
        }
        if (!pathIsMutable) {
            modifiedPath = origPath;
            pathPtr = &modifiedPath;
            pathIsMutable = true;
        }
        if (paint.getFillPath(*pathPtr, pathPtr)) {
            paint.setStyle(SkPaint::kFill_Style);
        } else {
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(0);
        }
        paint.setPathEffect(nullptr);
    }

    if (this->handleInversePath(*pathPtr, paint, pathIsMutable, prePathMatrix)) {
        return;
    }
    if (matrix.getType() & SkMatrix::kPerspective_Mask) {
        if (!pathIsMutable) {
            modifiedPath = origPath;
            pathPtr = &modifiedPath;
            pathIsMutable = true;
        }
        pathPtr->transform(matrix);
        if (paint.getShader()) {
            transform_shader(&paint, matrix);
        }
        matrix = SkMatrix::I();
    }

    ScopedContentEntry content(this, clipStack, matrix, paint);
    if (!content.entry()) {
        return;
    }
    SkScalar matrixScale = matrix.mapRadius(1.0f);
    SkScalar tolerance = matrixScale > 0.0f ? 0.25f / matrixScale : 0.25f;
    bool consumeDegeratePathSegments =
           paint.getStyle() == SkPaint::kFill_Style ||
           (paint.getStrokeCap() != SkPaint::kRound_Cap &&
            paint.getStrokeCap() != SkPaint::kSquare_Cap);
    SkPDFUtils::EmitPath(*pathPtr, paint.getStyle(), consumeDegeratePathSegments, content.stream(),
                         tolerance);
    SkPDFUtils::PaintPath(paint.getStyle(), pathPtr->getFillType(), content.stream());
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

void SkPDFDevice::drawBitmap(const SkBitmap& bm, SkScalar x, SkScalar y, const SkPaint& paint) {
    SkASSERT(!bm.drawsNothing());
    auto r = SkRect::MakeXYWH(x, y, bm.width(), bm.height());
    this->internalDrawImageRect(SkKeyedImage(bm), nullptr, r, paint, this->ctm());
}

void SkPDFDevice::drawSprite(const SkBitmap& bm, int x, int y, const SkPaint& paint) {
    SkASSERT(!bm.drawsNothing());
    auto r = SkRect::MakeXYWH(x, y, bm.width(), bm.height());
    this->internalDrawImageRect(SkKeyedImage(bm), nullptr, r, paint, SkMatrix::I());
}

void SkPDFDevice::drawImage(const SkImage* image, SkScalar x, SkScalar y, const SkPaint& paint) {
    SkASSERT(image);
    auto r = SkRect::MakeXYWH(x, y, image->width(), image->height());
    this->internalDrawImageRect(SkKeyedImage(sk_ref_sp(const_cast<SkImage*>(image))),
                                nullptr, r, paint, this->ctm());
}

////////////////////////////////////////////////////////////////////////////////

namespace {
class GlyphPositioner {
public:
    GlyphPositioner(SkDynamicMemoryWStream* content,
                    SkScalar textSkewX,
                    bool wideChars,
                    bool defaultPositioning,
                    SkPoint origin)
        : fContent(content)
        , fCurrentMatrixOrigin(origin)
        , fTextSkewX(textSkewX)
        , fWideChars(wideChars)
        , fDefaultPositioning(defaultPositioning) {
    }
    ~GlyphPositioner() { this->flush(); }
    void flush() {
        if (fInText) {
            fContent->writeText("> Tj\n");
            fInText = false;
        }
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
        if (!fDefaultPositioning) {
            SkPoint position = xy - fCurrentMatrixOrigin;
            if (position != SkPoint{fXAdvance, 0}) {
                this->flush();
                SkPDFUtils::AppendScalar(position.x(), fContent);
                fContent->writeText(" ");
                SkPDFUtils::AppendScalar(-position.y(), fContent);
                fContent->writeText(" Td ");
                fCurrentMatrixOrigin = xy;
                fXAdvance = 0;
            }
            fXAdvance += advanceWidth;
        }
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
    bool fWideChars;
    bool fInText = false;
    bool fInitialized = false;
    const bool fDefaultPositioning;
};

/** Given the m-to-n glyph-to-character mapping data (as returned by
    harfbuzz), iterate over the clusters. */
class Clusterator {
public:
    Clusterator() : fClusters(nullptr), fUtf8Text(nullptr), fGlyphCount(0), fTextByteLength(0) {}
    explicit Clusterator(uint32_t glyphCount)
        : fClusters(nullptr)
        , fUtf8Text(nullptr)
        , fGlyphCount(glyphCount)
        , fTextByteLength(0) {}
    // The clusters[] array is an array of offsets into utf8Text[],
    // one offset for each glyph.  See SkTextBlobBuilder for more info.
    Clusterator(const uint32_t* clusters,
                const char* utf8Text,
                uint32_t glyphCount,
                uint32_t textByteLength)
        : fClusters(clusters)
        , fUtf8Text(utf8Text)
        , fGlyphCount(glyphCount)
        , fTextByteLength(textByteLength) {
        // This is a cheap heuristic for /ReversedChars which seems to
        // work for clusters produced by HarfBuzz, which either
        // increase from zero (LTR) or decrease to zero (RTL).
        // "ReversedChars" is how PDF deals with RTL text.
        fReversedChars =
            fUtf8Text && fClusters && fGlyphCount && fClusters[0] != 0;
    }
    struct Cluster {
        const char* fUtf8Text;
        uint32_t fTextByteLength;
        uint32_t fGlyphIndex;
        uint32_t fGlyphCount;
        explicit operator bool() const { return fGlyphCount != 0; }
    };
    // True if this looks like right-to-left text.
    bool reversedChars() const { return fReversedChars; }
    Cluster next() {
        if ((!fUtf8Text || !fClusters) && fGlyphCount) {
            // These glyphs have no text.  Treat as one "cluster".
            uint32_t glyphCount = fGlyphCount;
            fGlyphCount = 0;
            return Cluster{nullptr, 0, 0, glyphCount};
        }
        if (fGlyphCount == 0 || fTextByteLength == 0) {
            return Cluster{nullptr, 0, 0, 0};  // empty
        }
        SkASSERT(fUtf8Text);
        SkASSERT(fClusters);
        uint32_t cluster = fClusters[0];
        if (cluster >= fTextByteLength) {
            return Cluster{nullptr, 0, 0, 0};  // bad input.
        }
        uint32_t glyphsInCluster = 1;
        while (glyphsInCluster < fGlyphCount &&
               fClusters[glyphsInCluster] == cluster) {
            ++glyphsInCluster;
        }
        SkASSERT(glyphsInCluster <= fGlyphCount);
        uint32_t textLength = 0;
        if (glyphsInCluster == fGlyphCount) {
            // consumes rest of glyphs and rest of text
            if (kInvalidCluster == fPreviousCluster) { // LTR text or single cluster
                textLength = fTextByteLength - cluster;
            } else { // RTL text; last cluster.
                SkASSERT(fPreviousCluster < fTextByteLength);
                if (fPreviousCluster <= cluster) {  // bad input.
                    return Cluster{nullptr, 0, 0, 0};
                }
                textLength = fPreviousCluster - cluster;
            }
            fGlyphCount = 0;
            return Cluster{fUtf8Text + cluster,
                           textLength,
                           fGlyphIndex,
                           glyphsInCluster};
        }
        SkASSERT(glyphsInCluster < fGlyphCount);
        uint32_t nextCluster = fClusters[glyphsInCluster];
        if (nextCluster >= fTextByteLength) {
            return Cluster{nullptr, 0, 0, 0};  // bad input.
        }
        if (nextCluster > cluster) { // LTR text
            if (kInvalidCluster != fPreviousCluster) {
                return Cluster{nullptr, 0, 0, 0};  // bad input.
            }
            textLength = nextCluster - cluster;
        } else { // RTL text
            SkASSERT(nextCluster < cluster);
            if (kInvalidCluster == fPreviousCluster) { // first cluster
                textLength = fTextByteLength - cluster;
            } else { // later cluster
                if (fPreviousCluster <= cluster) {
                    return Cluster{nullptr, 0, 0, 0}; // bad input.
                }
                textLength = fPreviousCluster - cluster;
            }
            fPreviousCluster = cluster;
        }
        uint32_t glyphIndex = fGlyphIndex;
        fGlyphCount -= glyphsInCluster;
        fGlyphIndex += glyphsInCluster;
        fClusters   += glyphsInCluster;
        return Cluster{fUtf8Text + cluster,
                       textLength,
                       glyphIndex,
                       glyphsInCluster};
    }

private:
    static constexpr uint32_t kInvalidCluster = 0xFFFFFFFF;
    const uint32_t* fClusters;
    const char* fUtf8Text;
    uint32_t fGlyphCount;
    uint32_t fTextByteLength;
    uint32_t fGlyphIndex = 0;
    uint32_t fPreviousCluster = kInvalidCluster;
    bool fReversedChars = false;
};

struct TextStorage {
    SkAutoTMalloc<char> fUtf8textStorage;
    SkAutoTMalloc<uint32_t> fClusterStorage;
    SkAutoTMalloc<SkGlyphID> fGlyphStorage;
};
}  // namespace

/** Given some unicode text (as passed to drawText(), convert to
    glyphs (via primitive shaping), while preserving
    glyph-to-character mapping information. */
static Clusterator make_clusterator(
        const void* sourceText,
        size_t sourceByteCount,
        const SkPaint& paint,
        TextStorage* storage,
        int glyphCount) {
    SkASSERT(SkPaint::kGlyphID_TextEncoding != paint.getTextEncoding());
    SkASSERT(glyphCount == paint.textToGlyphs(sourceText, sourceByteCount, nullptr));
    SkASSERT(glyphCount > 0);
    storage->fGlyphStorage.reset(SkToSizeT(glyphCount));
    (void)paint.textToGlyphs(sourceText, sourceByteCount, storage->fGlyphStorage.get());
    storage->fClusterStorage.reset(SkToSizeT(glyphCount));
    uint32_t* clusters = storage->fClusterStorage.get();
    uint32_t utf8ByteCount = 0;
    const char* utf8Text = nullptr;
    switch (paint.getTextEncoding()) {
        case SkPaint::kUTF8_TextEncoding: {
            const char* txtPtr = (const char*)sourceText;
            for (int i = 0; i < glyphCount; ++i) {
                clusters[i] = SkToU32(txtPtr - (const char*)sourceText);
                txtPtr += SkUTF8_LeadByteToCount(*(const unsigned char*)txtPtr);
                SkASSERT(txtPtr <= (const char*)sourceText + sourceByteCount);
            }
            SkASSERT(txtPtr == (const char*)sourceText + sourceByteCount);
            utf8ByteCount = SkToU32(sourceByteCount);
            utf8Text = (const char*)sourceText;
            break;
        }
        case SkPaint::kUTF16_TextEncoding: {
            const uint16_t* utf16ptr = (const uint16_t*)sourceText;
            int utf16count = SkToInt(sourceByteCount / sizeof(uint16_t));
            utf8ByteCount = SkToU32(SkUTF16_ToUTF8(utf16ptr, utf16count));
            storage->fUtf8textStorage.reset(utf8ByteCount);
            char* txtPtr = storage->fUtf8textStorage.get();
            utf8Text = txtPtr;
            int clusterIndex = 0;
            while (utf16ptr < (const uint16_t*)sourceText + utf16count) {
                clusters[clusterIndex++] = SkToU32(txtPtr - utf8Text);
                SkUnichar uni = SkUTF16_NextUnichar(&utf16ptr);
                txtPtr += SkUTF8_FromUnichar(uni, txtPtr);
            }
            SkASSERT(clusterIndex == glyphCount);
            SkASSERT(txtPtr == storage->fUtf8textStorage.get() + utf8ByteCount);
            SkASSERT(utf16ptr == (const uint16_t*)sourceText + utf16count);
            break;
        }
        case SkPaint::kUTF32_TextEncoding: {
            const SkUnichar* utf32 = (const SkUnichar*)sourceText;
            int utf32count = SkToInt(sourceByteCount / sizeof(SkUnichar));
            SkASSERT(glyphCount == utf32count);
            for (int i = 0; i < utf32count; ++i) {
                utf8ByteCount += SkToU32(SkUTF8_FromUnichar(utf32[i]));
            }
            storage->fUtf8textStorage.reset(SkToSizeT(utf8ByteCount));
            char* txtPtr = storage->fUtf8textStorage.get();
            utf8Text = txtPtr;
            for (int i = 0; i < utf32count; ++i) {
                clusters[i] = SkToU32(txtPtr - utf8Text);
                txtPtr += SkUTF8_FromUnichar(utf32[i], txtPtr);
            }
            break;
        }
        default:
            SkDEBUGFAIL("");
            break;
    }
    return Clusterator(clusters, utf8Text, SkToU32(glyphCount), utf8ByteCount);
}

static SkUnichar map_glyph(const SkTDArray<SkUnichar>& glyphToUnicode, SkGlyphID glyph) {
    return SkToInt(glyph) < glyphToUnicode.count() ? glyphToUnicode[SkToInt(glyph)] : -1;
}

static void update_font(SkWStream* wStream, int fontIndex, SkScalar textSize) {
    wStream->writeText("/");
    char prefix = SkPDFResourceDict::GetResourceTypePrefix(SkPDFResourceDict::kFont_ResourceType);
    wStream->write(&prefix, 1);
    wStream->writeDecAsText(fontIndex);
    wStream->writeText(" ");
    SkPDFUtils::AppendScalar(textSize, wStream);
    wStream->writeText(" Tf\n");
}

static SkPath draw_text_as_path(const void* sourceText, size_t sourceByteCount,
                               const SkScalar pos[], SkTextBlob::GlyphPositioning positioning,
                               SkPoint offset, const SkPaint& srcPaint) {
    SkPath path;
    int glyphCount;
    SkAutoTMalloc<SkPoint> tmpPoints;
    switch (positioning) {
        case SkTextBlob::kDefault_Positioning:
            srcPaint.getTextPath(sourceText, sourceByteCount, offset.x(), offset.y(), &path);
            break;
        case SkTextBlob::kHorizontal_Positioning:
            glyphCount = srcPaint.countText(sourceText, sourceByteCount);
            tmpPoints.realloc(glyphCount);
            for (int i = 0; i < glyphCount; ++i) {
                tmpPoints[i] = {pos[i] + offset.x(), offset.y()};
            }
            srcPaint.getPosTextPath(sourceText, sourceByteCount, tmpPoints.get(), &path);
            break;
        case SkTextBlob::kFull_Positioning:
            srcPaint.getPosTextPath(sourceText, sourceByteCount, (const SkPoint*)pos, &path);
            path.offset(offset.x(), offset.y());
            break;
    }
    return path;
}

static bool has_outline_glyph(SkGlyphID gid, SkGlyphCache* cache) {
    const SkGlyph& glyph = cache->getGlyphIDMetrics(gid);
    const SkPath* path = cache->findPath(glyph);
    return (path && !path->isEmpty()) || (glyph.fWidth == 0 && glyph.fHeight == 0);
}

static SkRect get_glyph_bounds_device_space(SkGlyphID gid, SkGlyphCache* cache,
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

static sk_sp<SkImage> image_from_mask(const SkMask& mask) {
    if (!mask.fImage) {
        return nullptr;
    }
    SkIRect bounds = mask.fBounds;
    SkBitmap bm;
    switch (mask.fFormat) {
        case SkMask::kBW_Format:
            bm.allocPixels(SkImageInfo::MakeA8(bounds.width(), bounds.height()));
            for (int y = 0; y < bm.height(); ++y) {
                for (int x8 = 0; x8 < bm.width(); x8 += 8) {
                    uint8_t v = *mask.getAddr1(x8 + bounds.x(), y + bounds.y());
                    int e = SkTMin(x8 + 8, bm.width());
                    for (int x = x8; x < e; ++x) {
                        *bm.getAddr8(x, y) = (v >> (x & 0x7)) & 0x1 ? 0xFF : 0x00;
                    }
                }
            }
            bm.setImmutable();
            return SkImage::MakeFromBitmap(bm);
        case SkMask::kA8_Format:
            bm.installPixels(SkImageInfo::MakeA8(bounds.width(), bounds.height()),
                             mask.fImage, mask.fRowBytes);
            return SkMakeImageFromRasterBitmap(bm, kAlways_SkCopyPixelsMode);
        case SkMask::kARGB32_Format:
            bm.installPixels(SkImageInfo::MakeN32Premul(bounds.width(), bounds.height()),
                             mask.fImage, mask.fRowBytes);
            return SkMakeImageFromRasterBitmap(bm, kAlways_SkCopyPixelsMode);
        case SkMask::k3D_Format:
            SkASSERT(false);
            return nullptr;
        case SkMask::kLCD16_Format:
            SkASSERT(false);
            return nullptr;
        default:
            SkASSERT(false);
            return nullptr;
    }
}

void SkPDFDevice::internalDrawText(
        const void* sourceText, size_t sourceByteCount,
        const SkScalar pos[], SkTextBlob::GlyphPositioning positioning,
        SkPoint offset, const SkPaint& srcPaint, const uint32_t* clusters,
        uint32_t textByteLength, const char* utf8Text) {
    if (0 == sourceByteCount || !sourceText) {
        return;
    }
    if (this->cs().isEmpty(this->bounds())) {
        return;
    }
    NOT_IMPLEMENTED(srcPaint.isVerticalText(), false);
    if (srcPaint.isVerticalText()) {
        // Don't pretend we support drawing vertical text.  It is not
        // clear to me how to switch to "vertical writing" mode in PDF.
        // Currently neither Chromium or Android set this flag.
        // https://bug.skia.org/5665
    }
    if (srcPaint.getPathEffect()
            || srcPaint.getMaskFilter()
            || SkPaint::kFill_Style != srcPaint.getStyle()) {
        // Stroked Text doesn't work well with Type3 fonts.
        SkPath path = draw_text_as_path(sourceText, sourceByteCount, pos,
                                        positioning, offset, srcPaint);
        this->drawPath(path, srcPaint, nullptr, true);
        return;
    }
    SkPaint paint = calculate_text_paint(srcPaint);
    remove_color_filter(&paint);
    replace_srcmode_on_opaque_paint(&paint);
    if (!paint.getTypeface()) {
        paint.setTypeface(SkTypeface::MakeDefault());
    }
    SkTypeface* typeface = paint.getTypeface();
    if (!typeface) {
        SkDebugf("SkPDF: SkTypeface::MakeDefault() returned nullptr.\n");
        return;
    }

    const SkAdvancedTypefaceMetrics* metrics =
        SkPDFFont::GetMetrics(typeface, fDocument->canon());
    if (!metrics) {
        return;
    }
    int glyphCount = paint.textToGlyphs(sourceText, sourceByteCount, nullptr);
    if (glyphCount <= 0) {
        return;
    }

    // These three heap buffers are only used in the case where no glyphs
    // are passed to drawText() (most clients pass glyphs or a textblob).
    TextStorage storage;
    const SkGlyphID* glyphs = nullptr;
    Clusterator clusterator;
    if (textByteLength > 0) {
        SkASSERT(glyphCount == SkToInt(sourceByteCount / sizeof(SkGlyphID)));
        glyphs = (const SkGlyphID*)sourceText;
        clusterator = Clusterator(clusters, utf8Text, SkToU32(glyphCount), textByteLength);
        SkASSERT(clusters);
        SkASSERT(utf8Text);
        SkASSERT(srcPaint.getTextEncoding() == SkPaint::kGlyphID_TextEncoding);
        SkASSERT(glyphCount == paint.textToGlyphs(sourceText, sourceByteCount, nullptr));
    } else if (SkPaint::kGlyphID_TextEncoding == srcPaint.getTextEncoding()) {
        SkASSERT(glyphCount == SkToInt(sourceByteCount / sizeof(SkGlyphID)));
        glyphs = (const SkGlyphID*)sourceText;
        clusterator = Clusterator(SkToU32(glyphCount));
        SkASSERT(glyphCount == paint.textToGlyphs(sourceText, sourceByteCount, nullptr));
        SkASSERT(nullptr == clusters);
        SkASSERT(nullptr == utf8Text);
    } else {
        SkASSERT(nullptr == clusters);
        SkASSERT(nullptr == utf8Text);
        clusterator = make_clusterator(sourceText, sourceByteCount, srcPaint,
                                       &storage, glyphCount);
        glyphs = storage.fGlyphStorage;
    }
    bool defaultPositioning = (positioning == SkTextBlob::kDefault_Positioning);
    paint.setHinting(SkPaint::kNo_Hinting);

    int emSize;
    SkAutoGlyphCache glyphCache = SkPDFFont::MakeVectorCache(typeface, &emSize);

    SkScalar textSize = paint.getTextSize();
    SkScalar advanceScale = textSize * paint.getTextScaleX() / emSize;

    // textScaleX and textScaleY are used to get a conservative bounding box for glyphs.
    SkScalar textScaleY = textSize / emSize;
    SkScalar textScaleX = advanceScale + paint.getTextSkewX() * textScaleY;

    SkPaint::Align alignment = paint.getTextAlign();
    float alignmentFactor = SkPaint::kLeft_Align   == alignment ?  0.0f :
                            SkPaint::kCenter_Align == alignment ? -0.5f :
                            /* SkPaint::kRight_Align */           -1.0f;
    if (defaultPositioning && alignment != SkPaint::kLeft_Align) {
        SkScalar advance = 0;
        for (int i = 0; i < glyphCount; ++i) {
            advance += advanceScale * glyphCache->getGlyphIDAdvance(glyphs[i]).fAdvanceX;
        }
        offset.offset(alignmentFactor * advance, 0);
    }
    SkRect clipStackBounds = this->cs().bounds(this->bounds());
    struct PositionedGlyph {
        SkPoint fPos;
        SkGlyphID fGlyph;
    };
    SkTArray<PositionedGlyph> fMissingGlyphs;
    {
        ScopedContentEntry content(this, paint, true);
        if (!content.entry()) {
            return;
        }
        SkDynamicMemoryWStream* out = content.stream();
        const SkTDArray<SkUnichar>& glyphToUnicode = metrics->fGlyphToUnicode;

        out->writeText("BT\n");
        SK_AT_SCOPE_EXIT(out->writeText("ET\n"));

        const SkGlyphID maxGlyphID = SkToU16(typeface->countGlyphs() - 1);

        bool multiByteGlyphs = SkPDFFont::IsMultiByte(SkPDFFont::FontType(*metrics));
        if (clusterator.reversedChars()) {
            out->writeText("/ReversedChars BMC\n");
        }
        SK_AT_SCOPE_EXIT(if (clusterator.reversedChars()) { out->writeText("EMC\n"); } );
        GlyphPositioner glyphPositioner(out,
                                        paint.getTextSkewX(),
                                        multiByteGlyphs,
                                        defaultPositioning,
                                        offset);
        SkPDFFont* font = nullptr;

        while (Clusterator::Cluster c = clusterator.next()) {
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
                SkUnichar unichar = SkUTF8_NextUnicharWithError(&textPtr, textEnd);
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
                        unichar = SkUTF8_NextUnicharWithError(&textPtr, textEnd);
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
                if (!font || !font->hasGlyph(gid)) {
                    // Not yet specified font or need to switch font.
                    int fontIndex = this->getFontResourceIndex(typeface, gid);
                    // All preconditions for SkPDFFont::GetFontResource are met.
                    SkASSERT(fontIndex >= 0);
                    if (fontIndex < 0) {
                        return;
                    }
                    glyphPositioner.flush();
                    update_font(out, fontIndex, textSize);
                    font = fFontResources[fontIndex];
                    SkASSERT(font);  // All preconditions for SkPDFFont::GetFontResource are met.
                    if (!font) {
                        return;
                    }
                    SkASSERT(font->multiByteGlyphs() == multiByteGlyphs);
                }
                SkPoint xy = {0, 0};
                SkScalar advance = advanceScale * glyphCache->getGlyphIDAdvance(gid).fAdvanceX;
                if (!defaultPositioning) {
                    xy = SkTextBlob::kFull_Positioning == positioning
                       ? SkPoint{pos[2 * index], pos[2 * index + 1]}
                       : SkPoint{pos[index], 0};
                    if (alignment != SkPaint::kLeft_Align) {
                        xy.offset(alignmentFactor * advance, 0);
                    }
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
                    if (!has_outline_glyph(gid, glyphCache.get())) {
                        fMissingGlyphs.push_back({xy + offset, gid});
                    }
                } else {
                    if (!has_outline_glyph(gid, glyphCache.get())) {
                        fMissingGlyphs.push_back({offset, gid});
                    }
                    offset += SkPoint{advance, 0};
                }
                font->noteGlyphUsage(gid);

                SkGlyphID encodedGlyph = multiByteGlyphs ? gid : font->glyphToPDFFontEncoding(gid);
                glyphPositioner.writeGlyph(xy, advance, encodedGlyph);
            }
        }
    }
    if (fMissingGlyphs.count() > 0) {
        // Fall back on images.
        SkPaint scaledGlyphCachePaint;
        scaledGlyphCachePaint.setTextSize(paint.getTextSize());
        scaledGlyphCachePaint.setTextScaleX(paint.getTextScaleX());
        scaledGlyphCachePaint.setTextSkewX(paint.getTextSkewX());
        scaledGlyphCachePaint.setTypeface(sk_ref_sp(typeface));
        SkAutoGlyphCache scaledGlyphCache(scaledGlyphCachePaint, nullptr, nullptr);
        SkTHashMap<SkPDFCanon::BitmapGlyphKey, SkPDFCanon::BitmapGlyph>* map =
            &this->getCanon()->fBitmapGlyphImages;
        for (PositionedGlyph positionedGlyph : fMissingGlyphs) {
            SkPDFCanon::BitmapGlyphKey key = {typeface->uniqueID(),
                                              paint.getTextSize(),
                                              paint.getTextScaleX(),
                                              paint.getTextSkewX(),
                                              positionedGlyph.fGlyph,
                                              0};
            SkImage* img = nullptr;
            SkIPoint imgOffset = {0, 0};
            if (SkPDFCanon::BitmapGlyph* ptr = map->find(key)) {
                img = ptr->fImage.get();
                imgOffset = ptr->fOffset;
            } else {
                (void)scaledGlyphCache->findImage(
                        scaledGlyphCache->getGlyphIDMetrics(positionedGlyph.fGlyph));
                SkMask mask;
                scaledGlyphCache->getGlyphIDMetrics(positionedGlyph.fGlyph).toMask(&mask);
                imgOffset = {mask.fBounds.x(), mask.fBounds.y()};
                img = map->set(key, {image_from_mask(mask), imgOffset})->fImage.get();
            }
            if (img) {
                SkPoint pt = positionedGlyph.fPos +
                             SkPoint{(SkScalar)imgOffset.x(), (SkScalar)imgOffset.y()};
                this->drawImage(img, pt.x(), pt.y(), srcPaint);
            }
        }
    }
}

void SkPDFDevice::drawText(const void* text, size_t len,
                           SkScalar x, SkScalar y, const SkPaint& paint) {
    this->internalDrawText(text, len, nullptr, SkTextBlob::kDefault_Positioning,
                           SkPoint{x, y}, paint, nullptr, 0, nullptr);
}

void SkPDFDevice::drawPosText(const void* text, size_t len,
                              const SkScalar pos[], int scalarsPerPos,
                              const SkPoint& offset, const SkPaint& paint) {
    this->internalDrawText(text, len, pos, (SkTextBlob::GlyphPositioning)scalarsPerPos,
                           offset, paint, nullptr, 0, nullptr);
}

void SkPDFDevice::drawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                               const SkPaint &paint, SkDrawFilter* drawFilter) {
    for (SkTextBlobRunIterator it(blob); !it.done(); it.next()) {
        SkPaint runPaint(paint);
        it.applyFontToPaint(&runPaint);
        if (drawFilter && !drawFilter->filter(&runPaint, SkDrawFilter::kText_Type)) {
            continue;
        }
        runPaint.setFlags(this->filterTextFlags(runPaint));
        SkPoint offset = it.offset() + SkPoint{x, y};
        this->internalDrawText(it.glyphs(), sizeof(SkGlyphID) * it.glyphCount(),
                               it.pos(), it.positioning(), offset, runPaint,
                               it.clusters(), it.textSize(), it.text());
    }
}

void SkPDFDevice::drawVertices(const SkVertices*, SkBlendMode, const SkPaint&) {
    if (this->cs().isEmpty(this->bounds())) {
        return;
    }
    // TODO: implement drawVertices
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

    SkScalar scalarX = SkIntToScalar(x);
    SkScalar scalarY = SkIntToScalar(y);
    for (const RectWithData& l : pdfDevice->fLinkToURLs) {
        SkRect r = l.rect.makeOffset(scalarX, scalarY);
        fLinkToURLs.emplace_back(RectWithData{r, l.data});
    }
    for (const RectWithData& l : pdfDevice->fLinkToDestinations) {
        SkRect r = l.rect.makeOffset(scalarX, scalarY);
        fLinkToDestinations.emplace_back(RectWithData{r, l.data});
    }
    for (const NamedDestination& d : pdfDevice->fNamedDestinations) {
        SkPoint p = d.point + SkPoint::Make(scalarX, scalarY);
        fNamedDestinations.emplace_back(NamedDestination{d.nameData, p});
    }

    if (pdfDevice->isContentEmpty()) {
        return;
    }

    SkMatrix matrix = SkMatrix::MakeTrans(SkIntToScalar(x), SkIntToScalar(y));
    ScopedContentEntry content(this, this->cs(), matrix, paint);
    if (!content.entry()) {
        return;
    }
    if (content.needShape()) {
        SkPath shape;
        shape.addRect(SkRect::MakeXYWH(SkIntToScalar(x), SkIntToScalar(y),
                                       SkIntToScalar(device->width()),
                                       SkIntToScalar(device->height())));
        content.setShape(shape);
    }
    if (!content.needSource()) {
        return;
    }

    sk_sp<SkPDFObject> xObject = pdfDevice->makeFormXObjectFromDevice();
    SkPDFUtils::DrawFormXObject(this->addXObjectResource(xObject.get()), content.stream());
}

sk_sp<SkSurface> SkPDFDevice::makeSurface(const SkImageInfo& info, const SkSurfaceProps& props) {
    return SkSurface::MakeRaster(info, &props);
}


sk_sp<SkPDFDict> SkPDFDevice::makeResourceDict() const {
    SkTDArray<SkPDFObject*> fonts;
    fonts.setReserve(fFontResources.count());
    for (SkPDFFont* font : fFontResources) {
        fonts.push(font);
    }
    return SkPDFResourceDict::Make(
            &fGraphicStateResources,
            &fShaderResources,
            &fXObjectResources,
            &fonts);
}

sk_sp<SkPDFArray> SkPDFDevice::copyMediaBox() const {
    auto mediaBox = sk_make_sp<SkPDFArray>();
    mediaBox->reserve(4);
    mediaBox->appendInt(0);
    mediaBox->appendInt(0);
    mediaBox->appendInt(fPageSize.width());
    mediaBox->appendInt(fPageSize.height());
    return mediaBox;
}

std::unique_ptr<SkStreamAsset> SkPDFDevice::content() const {
    SkDynamicMemoryWStream buffer;
    if (fInitialTransform.getType() != SkMatrix::kIdentity_Mask) {
        SkPDFUtils::AppendTransform(fInitialTransform, &buffer);
    }

    GraphicStackState gsState(fExistingClipStack, &buffer);
    for (const auto& entry : fContentEntries) {
        gsState.updateClip(entry.fState.fClipStack,
                {0, 0}, SkRect::Make(this->bounds()));
        gsState.updateMatrix(entry.fState.fMatrix);
        gsState.updateDrawingState(entry.fState);

        entry.fContent.writeToStream(&buffer);
    }
    gsState.drainStack();
    if (buffer.bytesWritten() > 0) {
        return std::unique_ptr<SkStreamAsset>(buffer.detachAsStream());
    } else {
        return skstd::make_unique<SkMemoryStream>();
    }
}

/* Draws an inverse filled path by using Path Ops to compute the positive
 * inverse using the current clip as the inverse bounds.
 * Return true if this was an inverse path and was properly handled,
 * otherwise returns false and the normal drawing routine should continue,
 * either as a (incorrect) fallback or because the path was not inverse
 * in the first place.
 */
bool SkPDFDevice::handleInversePath(const SkPath& origPath,
                                    const SkPaint& paint, bool pathIsMutable,
                                    const SkMatrix* prePathMatrix) {
    if (!origPath.isInverseFillType()) {
        return false;
    }

    if (this->cs().isEmpty(this->bounds())) {
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
            this->drawPath(modifiedPath, paint, nullptr, true);
            return true;
        }
    }

    // Get bounds of clip in current transform space
    // (clip bounds are given in device space).
    SkMatrix transformInverse;
    SkMatrix totalMatrix = this->ctm();
    if (prePathMatrix) {
        totalMatrix.preConcat(*prePathMatrix);
    }
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

    this->drawPath(modifiedPath, noInversePaint, prePathMatrix, true);
    return true;
}

void SkPDFDevice::appendAnnotations(SkPDFArray* array) const {
    array->reserve(fLinkToURLs.count() + fLinkToDestinations.count());
    for (const RectWithData& rectWithURL : fLinkToURLs) {
        SkRect r;
        fInitialTransform.mapRect(&r, rectWithURL.rect);
        array->appendObject(create_link_to_url(rectWithURL.data.get(), r));
    }
    for (const RectWithData& linkToDestination : fLinkToDestinations) {
        SkRect r;
        fInitialTransform.mapRect(&r, linkToDestination.rect);
        array->appendObject(
                create_link_named_dest(linkToDestination.data.get(), r));
    }
}

void SkPDFDevice::appendDestinations(SkPDFDict* dict, SkPDFObject* page) const {
    for (const NamedDestination& dest : fNamedDestinations) {
        auto pdfDest = sk_make_sp<SkPDFArray>();
        pdfDest->reserve(5);
        pdfDest->appendObjRef(sk_ref_sp(page));
        pdfDest->appendName("XYZ");
        SkPoint p = fInitialTransform.mapXY(dest.point.x(), dest.point.y());
        pdfDest->appendScalar(p.x());
        pdfDest->appendScalar(p.y());
        pdfDest->appendInt(0);  // Leave zoom unchanged
        SkString name(static_cast<const char*>(dest.nameData->data()));
        dict->insertObject(name, std::move(pdfDest));
    }
}

sk_sp<SkPDFObject> SkPDFDevice::makeFormXObjectFromDevice(bool alpha) {
    SkMatrix inverseTransform = SkMatrix::I();
    if (!fInitialTransform.isIdentity()) {
        if (!fInitialTransform.invert(&inverseTransform)) {
            SkDEBUGFAIL("Layer initial transform should be invertible.");
            inverseTransform.reset();
        }
    }
    const char* colorSpace = alpha ? "DeviceGray" : nullptr;
    sk_sp<SkPDFObject> xobject =
        SkPDFMakeFormXObject(this->content(), this->copyMediaBox(),
                             this->makeResourceDict(), inverseTransform, colorSpace);
    // We always draw the form xobjects that we create back into the device, so
    // we simply preserve the font usage instead of pulling it out and merging
    // it back in later.
    this->cleanUp();  // Reset this device to have no content.
    this->init();
    return xobject;
}

void SkPDFDevice::drawFormXObjectWithMask(int xObjectIndex,
                                          sk_sp<SkPDFObject> mask,
                                          const SkClipStack& clipStack,
                                          SkBlendMode mode,
                                          bool invertClip) {
    if (!invertClip && clipStack.isEmpty(this->bounds())) {
        return;
    }

    sk_sp<SkPDFDict> sMaskGS = SkPDFGraphicState::GetSMaskGraphicState(
            std::move(mask), invertClip,
            SkPDFGraphicState::kAlpha_SMaskMode, fDocument->canon());

    SkPaint paint;
    paint.setBlendMode(mode);
    ScopedContentEntry content(this, clipStack, SkMatrix::I(), paint);
    if (!content.entry()) {
        return;
    }
    SkPDFUtils::ApplyGraphicState(addGraphicStateResource(sMaskGS.get()), content.stream());
    SkPDFUtils::DrawFormXObject(xObjectIndex, content.stream());
    this->clearMaskOnGraphicState(content.stream());
}

SkPDFDevice::ContentEntry* SkPDFDevice::setUpContentEntry(const SkClipStack& clipStack,
                                                          const SkMatrix& matrix,
                                                          const SkPaint& paint,
                                                          bool hasText,
                                                          sk_sp<SkPDFObject>* dst) {
    *dst = nullptr;
    SkBlendMode blendMode = paint.getBlendMode();

    // For the following modes, we want to handle source and destination
    // separately, so make an object of what's already there.
    if (blendMode == SkBlendMode::kClear       ||
            blendMode == SkBlendMode::kSrc     ||
            blendMode == SkBlendMode::kSrcIn   ||
            blendMode == SkBlendMode::kDstIn   ||
            blendMode == SkBlendMode::kSrcOut  ||
            blendMode == SkBlendMode::kDstOut  ||
            blendMode == SkBlendMode::kSrcATop ||
            blendMode == SkBlendMode::kDstATop ||
            blendMode == SkBlendMode::kModulate) {
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
    // Xor, Plus.

    // Dst xfer mode doesn't draw source at all.
    if (blendMode == SkBlendMode::kDst) {
        return nullptr;
    }

    SkPDFDevice::ContentEntry* entry;
    if (fContentEntries.back() && fContentEntries.back()->fContent.bytesWritten() == 0) {
        entry = fContentEntries.back();
    } else if (blendMode != SkBlendMode::kDstOver) {
        entry = fContentEntries.emplace_back();
    } else {
        entry = fContentEntries.emplace_front();
    }
    populateGraphicStateEntryFromPaint(matrix, clipStack, paint, hasText, &entry->fState);
    return entry;
}

void SkPDFDevice::finishContentEntry(SkBlendMode blendMode,
                                     sk_sp<SkPDFObject> dst,
                                     SkPath* shape) {
    if (blendMode != SkBlendMode::kClear       &&
            blendMode != SkBlendMode::kSrc     &&
            blendMode != SkBlendMode::kDstOver &&
            blendMode != SkBlendMode::kSrcIn   &&
            blendMode != SkBlendMode::kDstIn   &&
            blendMode != SkBlendMode::kSrcOut  &&
            blendMode != SkBlendMode::kDstOut  &&
            blendMode != SkBlendMode::kSrcATop &&
            blendMode != SkBlendMode::kDstATop &&
            blendMode != SkBlendMode::kModulate) {
        SkASSERT(!dst);
        return;
    }
    if (blendMode == SkBlendMode::kDstOver) {
        SkASSERT(!dst);
        if (fContentEntries.front()->fContent.bytesWritten() == 0) {
            // For DstOver, an empty content entry was inserted before the rest
            // of the content entries. If nothing was drawn, it needs to be
            // removed.
            fContentEntries.pop_front();
        }
        return;
    }
    if (!dst) {
        SkASSERT(blendMode == SkBlendMode::kSrc ||
                 blendMode == SkBlendMode::kSrcOut);
        return;
    }

    SkASSERT(dst);
    SkASSERT(fContentEntries.count() == 1);
    // Changing the current content into a form-xobject will destroy the clip
    // objects which is fine since the xobject will already be clipped. However
    // if source has shape, we need to clip it too, so a copy of the clip is
    // saved.

    SkClipStack clipStack = fContentEntries.front()->fState.fClipStack;

    SkPaint stockPaint;

    sk_sp<SkPDFObject> srcFormXObject;
    if (isContentEmpty()) {
        // If nothing was drawn and there's no shape, then the draw was a
        // no-op, but dst needs to be restored for that to be true.
        // If there is shape, then an empty source with Src, SrcIn, SrcOut,
        // DstIn, DstAtop or Modulate reduces to Clear and DstOut or SrcAtop
        // reduces to Dst.
        if (shape == nullptr || blendMode == SkBlendMode::kDstOut ||
                blendMode == SkBlendMode::kSrcATop) {
            ScopedContentEntry content(this, fExistingClipStack, SkMatrix::I(), stockPaint);
            // TODO: addXObjectResource take sk_sp
            SkPDFUtils::DrawFormXObject(this->addXObjectResource(dst.get()), content.stream());
            return;
        } else {
            blendMode = SkBlendMode::kClear;
        }
    } else {
        SkASSERT(fContentEntries.count() == 1);
        srcFormXObject = this->makeFormXObjectFromDevice();
    }

    // TODO(vandebo) srcFormXObject may contain alpha, but here we want it
    // without alpha.
    if (blendMode == SkBlendMode::kSrcATop) {
        // TODO(vandebo): In order to properly support SrcATop we have to track
        // the shape of what's been drawn at all times. It's the intersection of
        // the non-transparent parts of the device and the outlines (shape) of
        // all images and devices drawn.
        drawFormXObjectWithMask(addXObjectResource(srcFormXObject.get()), dst,
                                fExistingClipStack, SkBlendMode::kSrcOver, true);
    } else {
        if (shape != nullptr) {
            // Draw shape into a form-xobject.
            SkPaint filledPaint;
            filledPaint.setColor(SK_ColorBLACK);
            filledPaint.setStyle(SkPaint::kFill_Style);
            this->internalDrawPath(clipStack, SkMatrix::I(), *shape, filledPaint, nullptr, true);
            this->drawFormXObjectWithMask(this->addXObjectResource(dst.get()),
                                          this->makeFormXObjectFromDevice(),
                                          fExistingClipStack,
                                          SkBlendMode::kSrcOver, true);
        } else {
            this->drawFormXObjectWithMask(this->addXObjectResource(dst.get()),
                                          srcFormXObject,
                                          fExistingClipStack,
                                          SkBlendMode::kSrcOver, true);
        }
    }

    if (blendMode == SkBlendMode::kClear) {
        return;
    } else if (blendMode == SkBlendMode::kSrc ||
            blendMode == SkBlendMode::kDstATop) {
        ScopedContentEntry content(this, fExistingClipStack, SkMatrix::I(), stockPaint);
        if (content.entry()) {
            SkPDFUtils::DrawFormXObject(this->addXObjectResource(srcFormXObject.get()),
                                        content.stream());
        }
        if (blendMode == SkBlendMode::kSrc) {
            return;
        }
    } else if (blendMode == SkBlendMode::kSrcATop) {
        ScopedContentEntry content(this, fExistingClipStack,
                                   SkMatrix::I(), stockPaint);
        if (content.entry()) {
            SkPDFUtils::DrawFormXObject(this->addXObjectResource(dst.get()), content.stream());
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
        drawFormXObjectWithMask(addXObjectResource(srcFormXObject.get()),
                                std::move(dst),
                                fExistingClipStack,
                                SkBlendMode::kSrcOver,
                                blendMode == SkBlendMode::kSrcOut);
        return;
    } else {
        SkBlendMode mode = SkBlendMode::kSrcOver;
        int resourceID = addXObjectResource(dst.get());
        if (blendMode == SkBlendMode::kModulate) {
            drawFormXObjectWithMask(addXObjectResource(srcFormXObject.get()),
                                    std::move(dst), fExistingClipStack,
                                    SkBlendMode::kSrcOver, false);
            mode = SkBlendMode::kMultiply;
        }
        drawFormXObjectWithMask(resourceID, std::move(srcFormXObject),
                                fExistingClipStack, mode,
                                blendMode == SkBlendMode::kDstOut);
        return;
    }
}

bool SkPDFDevice::isContentEmpty() {
    if (!fContentEntries.front() || fContentEntries.front()->fContent.bytesWritten() == 0) {
        SkASSERT(fContentEntries.count() <= 1);
        return true;
    }
    return false;
}

void SkPDFDevice::populateGraphicStateEntryFromPaint(
        const SkMatrix& matrix,
        const SkClipStack& clipStack,
        const SkPaint& paint,
        bool hasText,
        SkPDFDevice::GraphicStateEntry* entry) {
    NOT_IMPLEMENTED(paint.getPathEffect() != nullptr, false);
    NOT_IMPLEMENTED(paint.getMaskFilter() != nullptr, false);
    NOT_IMPLEMENTED(paint.getColorFilter() != nullptr, false);

    entry->fMatrix = matrix;
    entry->fClipStack = clipStack;
    entry->fColor = SkColorSetA(paint.getColor(), 0xFF);
    entry->fShaderIndex = -1;

    // PDF treats a shader as a color, so we only set one or the other.
    sk_sp<SkPDFObject> pdfShader;
    SkShader* shader = paint.getShader();
    SkColor color = paint.getColor();
    if (shader) {
        if (SkShader::kColor_GradientType == shader->asAGradient(nullptr)) {
            // We don't have to set a shader just for a color.
            SkShader::GradientInfo gradientInfo;
            SkColor gradientColor = SK_ColorBLACK;
            gradientInfo.fColors = &gradientColor;
            gradientInfo.fColorOffsets = nullptr;
            gradientInfo.fColorCount = 1;
            SkAssertResult(shader->asAGradient(&gradientInfo) == SkShader::kColor_GradientType);
            entry->fColor = SkColorSetA(gradientColor, 0xFF);
            color = gradientColor;
        } else {
            // PDF positions patterns relative to the initial transform, so
            // we need to apply the current transform to the shader parameters.
            SkMatrix transform = matrix;
            transform.postConcat(fInitialTransform);

            // PDF doesn't support kClamp_TileMode, so we simulate it by making
            // a pattern the size of the current clip.
            SkRect clipStackBounds = clipStack.bounds(this->bounds());

            // We need to apply the initial transform to bounds in order to get
            // bounds in a consistent coordinate system.
            fInitialTransform.mapRect(&clipStackBounds);
            SkIRect bounds;
            clipStackBounds.roundOut(&bounds);

            pdfShader = SkPDFMakeShader(fDocument, shader, transform, bounds, paint.getColor());

            if (pdfShader.get()) {
                // pdfShader has been canonicalized so we can directly compare
                // pointers.
                int resourceIndex = fShaderResources.find(pdfShader.get());
                if (resourceIndex < 0) {
                    resourceIndex = fShaderResources.count();
                    fShaderResources.push(pdfShader.get());
                    pdfShader.get()->ref();
                }
                entry->fShaderIndex = resourceIndex;
            }
        }
    }

    sk_sp<SkPDFDict> newGraphicState;
    if (color == paint.getColor()) {
        newGraphicState = SkPDFGraphicState::GetGraphicStateForPaint(fDocument->canon(), paint);
    } else {
        SkPaint newPaint = paint;
        newPaint.setColor(color);
        newGraphicState = SkPDFGraphicState::GetGraphicStateForPaint(fDocument->canon(), newPaint);
    }
    int resourceIndex = addGraphicStateResource(newGraphicState.get());
    entry->fGraphicStateIndex = resourceIndex;

    if (hasText) {
        entry->fTextScaleX = paint.getTextScaleX();
        entry->fTextFill = paint.getStyle();
    } else {
        entry->fTextScaleX = 0;
    }
}

int SkPDFDevice::addGraphicStateResource(SkPDFObject* gs) {
    // Assumes that gs has been canonicalized (so we can directly compare
    // pointers).
    int result = fGraphicStateResources.find(gs);
    if (result < 0) {
        result = fGraphicStateResources.count();
        fGraphicStateResources.push(gs);
        gs->ref();
    }
    return result;
}

int SkPDFDevice::addXObjectResource(SkPDFObject* xObject) {
    // TODO(halcanary): make this take a sk_sp<SkPDFObject>
    // Assumes that xobject has been canonicalized (so we can directly compare
    // pointers).
    int result = fXObjectResources.find(xObject);
    if (result < 0) {
        result = fXObjectResources.count();
        fXObjectResources.push(SkRef(xObject));
    }
    return result;
}

int SkPDFDevice::getFontResourceIndex(SkTypeface* typeface, uint16_t glyphID) {
    sk_sp<SkPDFFont> newFont = SkPDFFont::GetFontResource(fDocument->canon(), typeface, glyphID);
    if (!newFont) {
        return -1;
    }
    int resourceIndex = fFontResources.find(newFont.get());
    if (resourceIndex < 0) {
        fDocument->registerFont(newFont.get());
        resourceIndex = fFontResources.count();
        fFontResources.push(newFont.release());
    }
    return resourceIndex;
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
    if (!imageSubset) {
        return;
    }

    SkIRect bounds = imageSubset.image()->bounds();
    SkPaint paint = srcPaint;
    if (imageSubset.image()->isOpaque()) {
        replace_srcmode_on_opaque_paint(&paint);
    }
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

    // TODO(halcanary) support isAlphaOnly & getMaskFilter.
    bool imageAlphaOnly = imageSubset.image()->isAlphaOnly() && !paint.getMaskFilter();
    if (imageAlphaOnly) {
        if (SkColorFilter* colorFilter = paint.getColorFilter()) {
            sk_sp<SkImage> img = color_filter(imageSubset.image().get(), colorFilter);
            paint.setColorFilter(nullptr);
            imageSubset = SkKeyedImage(std::move(img));
            if (!imageSubset) {
                return;
            }
            imageAlphaOnly = imageSubset.image()->isAlphaOnly();
            // The colorfilter can make a alphonly image no longer be alphaonly.
        }
    }
    if (imageAlphaOnly) {
        sk_sp<SkImage> mask = alpha_image_to_greyscale_image(imageSubset.image().get());
        if (!mask) {
            return;
        }
        // PDF doesn't seem to allow masking vector graphics with an Image XObject.
        // Must mask with a Form XObject.
        sk_sp<SkPDFDevice> maskDevice = this->makeCongruentDevice();
        {
            SkCanvas canvas(maskDevice.get());
            canvas.concat(transform);
            canvas.concat(ctm);
            // TODO(halcanary): investigate sub-pixel clipping.
            canvas.drawImage(mask, 0, 0);
        }
        remove_color_filter(&paint);
        if (!ctm.isIdentity() && paint.getShader()) {
            transform_shader(&paint, ctm); // Since we are using identity matrix.
        }
        ScopedContentEntry content(this, this->cs(), SkMatrix::I(), paint);
        if (!content.entry()) {
            return;
        }
        this->addSMaskGraphicState(std::move(maskDevice), content.stream());
        SkPDFUtils::AppendRectangle(SkRect::Make(fPageSize), content.stream());
        SkPDFUtils::PaintPath(SkPaint::kFill_Style, SkPath::kWinding_FillType, content.stream());
        this->clearMaskOnGraphicState(content.stream());
        return;
    }
    if (paint.getMaskFilter()) {
        paint.setShader(imageSubset.image()->makeShader(&transform));
        SkPath path;
        path.addRect(SkRect::Make(imageSubset.image()->bounds()));
        this->internalDrawPath(this->cs(), this->ctm(), path, paint, &transform, true);
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

    #ifdef SK_PDF_IMAGE_STATS
    gDrawImageCalls.fetch_add(1);
    #endif
    SkMatrix matrix = transform;

    // Rasterize the bitmap using perspective in a new bitmap.
    if (transform.hasPerspective()) {
        SkASSERT(fDocument->rasterDpi() > 0);
        // Transform the bitmap in the new space, without taking into
        // account the initial transform.
        SkPath perspectiveOutline;
        SkRect imageBounds = SkRect::Make(imageSubset.image()->bounds());
        perspectiveOutline.addRect(imageBounds);
        perspectiveOutline.transform(transform);

        // TODO(edisonn): perf - use current clip too.
        // Retrieve the bounds of the new shape.
        SkRect bounds = perspectiveOutline.getBounds();

        // Transform the bitmap in the new space, taking into
        // account the initial transform.
        SkMatrix total = transform;
        total.postConcat(fInitialTransform);
        SkScalar dpiScale = SkIntToScalar(fDocument->rasterDpi()) /
                            SkIntToScalar(SkPDFUtils::kDpiForRasterScaleOne);
        total.postScale(dpiScale, dpiScale);

        SkPath physicalPerspectiveOutline;
        physicalPerspectiveOutline.addRect(imageBounds);
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
        canvas->flush();

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
    ScopedContentEntry content(this, this->cs(), scaled, paint);
    if (!content.entry()) {
        return;
    }
    if (content.needShape()) {
        SkPath shape;
        shape.addRect(SkRect::Make(subset));
        shape.transform(matrix);
        content.setShape(shape);
    }
    if (!content.needSource()) {
        return;
    }

    if (SkColorFilter* colorFilter = paint.getColorFilter()) {
        // TODO(https://bug.skia.org/4378): implement colorfilter on other
        // draw calls.  This code here works for all
        // drawBitmap*()/drawImage*() calls amd ImageFilters (which
        // rasterize a layer on this backend).  Fortuanely, this seems
        // to be how Chromium impements most color-filters.
        sk_sp<SkImage> img = color_filter(imageSubset.image().get(), colorFilter);
        imageSubset = SkKeyedImage(std::move(img));
        if (!imageSubset) {
            return;
        }
        // TODO(halcanary): de-dupe this by caching filtered images.
        // (maybe in the resource cache?)
    }

    SkBitmapKey key = imageSubset.key();
    sk_sp<SkPDFObject>* pdfimagePtr = fDocument->canon()->fPDFBitmapMap.find(key);
    sk_sp<SkPDFObject> pdfimage = pdfimagePtr ? *pdfimagePtr : nullptr;
    if (!pdfimage) {
        SkASSERT(imageSubset);
        pdfimage = SkPDFCreateBitmapObject(imageSubset.release(),
                                           fDocument->canon()->fPixelSerializer.get());
        if (!pdfimage) {
            return;
        }
        fDocument->serialize(pdfimage);  // serialize images early.
        SkASSERT((key != SkBitmapKey{{0, 0, 0, 0}, 0}));
        fDocument->canon()->fPDFBitmapMap.set(key, pdfimage);
    }
    // TODO(halcanary): addXObjectResource() should take a sk_sp<SkPDFObject>
    SkPDFUtils::DrawFormXObject(this->addXObjectResource(pdfimage.get()), content.stream());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkSpecialImage.h"
#include "SkImageFilter.h"

void SkPDFDevice::drawSpecial(SkSpecialImage* srcImg, int x, int y, const SkPaint& paint,
                              SkImage* clipImage, const SkMatrix& clipMatrix) {
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
        // TODO: Should PDF be operating in a specified color space? For now, run the filter
        // in the same color space as the source (this is different from all other backends).
        SkImageFilter::OutputProperties outputProperties(srcImg->getColorSpace());
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
    // TODO: See comment above in drawSpecial. The color mode we use for decode should be driven
    // by the destination where we're going to draw thing thing (ie this device). But we don't have
    // a color space, so we always decode in legacy mode for now.
    SkColorSpace* legacyColorSpace = nullptr;
    return SkSpecialImage::MakeFromImage(image->bounds(),
                                         image->makeNonTextureImage(), legacyColorSpace);
}

sk_sp<SkSpecialImage> SkPDFDevice::snapSpecial() {
    return nullptr;
}

SkImageFilterCache* SkPDFDevice::getImageFilterCache() {
    // We always return a transient cache, so it is freed after each
    // filter traversal.
    return SkImageFilterCache::Create(SkImageFilterCache::kDefaultTransientSize);
}
