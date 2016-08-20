/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPDFDevice.h"
#include "SkAnnotationKeys.h"
#include "SkBitmapDevice.h"
#include "SkBitmapKey.h"
#include "SkColor.h"
#include "SkColorFilter.h"
#include "SkDraw.h"
#include "SkGlyphCache.h"
#include "SkPath.h"
#include "SkPathEffect.h"
#include "SkPathOps.h"
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
#include "SkRasterClip.h"
#include "SkRRect.h"
#include "SkString.h"
#include "SkSurface.h"
#include "SkTextFormatParams.h"
#include "SkTemplates.h"
#include "SkTypefacePriv.h"
#include "SkXfermodeInterpretation.h"

#define DPI_FOR_RASTER_SCALE_ONE 72

// Utility functions

// If the paint will definitely draw opaquely, replace kSrc_Mode with
// kSrcOver_Mode.  http://crbug.com/473572
static void replace_srcmode_on_opaque_paint(SkPaint* paint) {
    if (kSrcOver_SkXfermodeInterpretation
        == SkInterpretXfermode(*paint, false)) {
        paint->setXfermode(nullptr);
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
        SkScalar width = SkScalarMul(result.getTextSize(), fakeBoldScale);
        if (result.getStyle() == SkPaint::kFill_Style) {
            result.setStyle(SkPaint::kStrokeAndFill_Style);
        } else {
            width += result.getStrokeWidth();
        }
        result.setStrokeWidth(width);
    }
    return result;
}

static void set_text_transform(SkScalar x, SkScalar y, SkScalar textSkewX,
                               SkWStream* content) {
    // Flip the text about the x-axis to account for origin swap and include
    // the passed parameters.
    content->writeText("1 0 ");
    SkPDFUtils::AppendScalar(0 - textSkewX, content);
    content->writeText(" -1 ");
    SkPDFUtils::AppendScalar(x, content);
    content->writeText(" ");
    SkPDFUtils::AppendScalar(y, content);
    content->writeText(" Tm\n");
}

SkPDFDevice::GraphicStateEntry::GraphicStateEntry()
    : fColor(SK_ColorBLACK)
    , fTextScaleX(SK_Scalar1)
    , fTextFill(SkPaint::kFill_Style)
    , fShaderIndex(-1)
    , fGraphicStateIndex(-1)
    , fFont(nullptr)
    , fTextSize(SK_ScalarNaN) {
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
                      const SkRegion& existingClipRegion,
                      SkWStream* contentStream)
            : fStackDepth(0),
              fContentStream(contentStream) {
        fEntries[0].fClipStack = existingClipStack;
        fEntries[0].fClipRegion = existingClipRegion;
    }

    void updateClip(const SkClipStack& clipStack, const SkRegion& clipRegion,
                    const SkPoint& translation);
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

// This function initializes iter to be an iterator on the "stack" argument
// and then skips over the leading entries as specified in prefix.  It requires
// and asserts that "prefix" will be a prefix to "stack."
static void skip_clip_stack_prefix(const SkClipStack& prefix,
                                   const SkClipStack& stack,
                                   SkClipStack::Iter* iter) {
    SkClipStack::B2TIter prefixIter(prefix);
    iter->reset(stack, SkClipStack::Iter::kBottom_IterStart);

    const SkClipStack::Element* prefixEntry;
    const SkClipStack::Element* iterEntry;

    for (prefixEntry = prefixIter.next(); prefixEntry;
            prefixEntry = prefixIter.next()) {
        iterEntry = iter->next();
        SkASSERT(iterEntry);
        // Because of SkClipStack does internal intersection, the last clip
        // entry may differ.
        if (*prefixEntry != *iterEntry) {
            SkASSERT(prefixEntry->getOp() == SkRegion::kIntersect_Op);
            SkASSERT(iterEntry->getOp() == SkRegion::kIntersect_Op);
            SkASSERT(iterEntry->getType() == prefixEntry->getType());
            // back up the iterator by one
            iter->prev();
            prefixEntry = prefixIter.next();
            break;
        }
    }

    SkASSERT(prefixEntry == nullptr);
}

static void emit_clip(SkPath* clipPath, SkRect* clipRect,
                      SkWStream* contentStream) {
    SkASSERT(clipPath || clipRect);

    SkPath::FillType clipFill;
    if (clipPath) {
        SkPDFUtils::EmitPath(*clipPath, SkPaint::kFill_Style, contentStream);
        clipFill = clipPath->getFillType();
    } else {
        SkPDFUtils::AppendRectangle(*clipRect, contentStream);
        clipFill = SkPath::kWinding_FillType;
    }

    NOT_IMPLEMENTED(clipFill == SkPath::kInverseEvenOdd_FillType, false);
    NOT_IMPLEMENTED(clipFill == SkPath::kInverseWinding_FillType, false);
    if (clipFill == SkPath::kEvenOdd_FillType) {
        contentStream->writeText("W* n\n");
    } else {
        contentStream->writeText("W n\n");
    }
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

// Sanity check the numerical values of the SkRegion ops and PathOps ops
// enums so region_op_to_pathops_op can do a straight passthrough cast.
// If these are failing, it may be necessary to make region_op_to_pathops_op
// do more.
static_assert(SkRegion::kDifference_Op == (int)kDifference_SkPathOp, "region_pathop_mismatch");
static_assert(SkRegion::kIntersect_Op == (int)kIntersect_SkPathOp, "region_pathop_mismatch");
static_assert(SkRegion::kUnion_Op == (int)kUnion_SkPathOp, "region_pathop_mismatch");
static_assert(SkRegion::kXOR_Op == (int)kXOR_SkPathOp, "region_pathop_mismatch");
static_assert(SkRegion::kReverseDifference_Op == (int)kReverseDifference_SkPathOp,
              "region_pathop_mismatch");

static SkPathOp region_op_to_pathops_op(SkRegion::Op op) {
    SkASSERT(op >= 0);
    SkASSERT(op <= SkRegion::kReverseDifference_Op);
    return (SkPathOp)op;
}

/* Uses Path Ops to calculate a vector SkPath clip from a clip stack.
 * Returns true if successful, or false if not successful.
 * If successful, the resulting clip is stored in outClipPath.
 * If not successful, outClipPath is undefined, and a fallback method
 * should be used.
 */
static bool get_clip_stack_path(const SkMatrix& transform,
                                const SkClipStack& clipStack,
                                const SkRegion& clipRegion,
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

        if (SkRegion::kReplace_Op == clipEntry->getOp()) {
            *outClipPath = entryPath;
        } else {
            SkPathOp op = region_op_to_pathops_op(clipEntry->getOp());
            if (!Op(*outClipPath, entryPath, op, outClipPath)) {
                return false;
            }
        }
    }

    if (outClipPath->isInverseFillType()) {
        // The bounds are slightly outset to ensure this is correct in the
        // face of floating-point accuracy and possible SkRegion bitmap
        // approximations.
        SkRect clipBounds = SkRect::Make(clipRegion.getBounds());
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
                                   const SkRegion& clipRegion,
                                   const SkPoint& translation) {
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
    currentEntry()->fClipRegion = clipRegion;

    SkMatrix transform;
    transform.setTranslate(translation.fX, translation.fY);

    SkPath clipPath;
    if (get_clip_stack_path(transform, clipStack, clipRegion, &clipPath)) {
        emit_clip(&clipPath, nullptr, fContentStream);
        return;
    }

    // gsState->initialEntry()->fClipStack/Region specifies the clip that has
    // already been applied.  (If this is a top level device, then it specifies
    // a clip to the content area.  If this is a layer, then it specifies
    // the clip in effect when the layer was created.)  There's no need to
    // reapply that clip; SKCanvas's SkDrawIter will draw anything outside the
    // initial clip on the parent layer.  (This means there's a bug if the user
    // expands the clip and then uses any xfer mode that uses dst:
    // http://code.google.com/p/skia/issues/detail?id=228 )
    SkClipStack::Iter iter;
    skip_clip_stack_prefix(fEntries[0].fClipStack, clipStack, &iter);

    // If the clip stack does anything other than intersect or if it uses
    // an inverse fill type, we have to fall back to the clip region.
    bool needRegion = false;
    const SkClipStack::Element* clipEntry;
    for (clipEntry = iter.next(); clipEntry; clipEntry = iter.next()) {
        if (clipEntry->getOp() != SkRegion::kIntersect_Op ||
                clipEntry->isInverseFilled()) {
            needRegion = true;
            break;
        }
    }

    if (needRegion) {
        SkPath clipPath;
        SkAssertResult(clipRegion.getBoundaryPath(&clipPath));
        emit_clip(&clipPath, nullptr, fContentStream);
    } else {
        skip_clip_stack_prefix(fEntries[0].fClipStack, clipStack, &iter);
        const SkClipStack::Element* clipEntry;
        for (clipEntry = iter.next(); clipEntry; clipEntry = iter.next()) {
            SkASSERT(clipEntry->getOp() == SkRegion::kIntersect_Op);
            switch (clipEntry->getType()) {
                case SkClipStack::Element::kRect_Type: {
                    SkRect translatedClip;
                    transform.mapRect(&translatedClip, clipEntry->getRect());
                    emit_clip(nullptr, &translatedClip, fContentStream);
                    break;
                }
                default: {
                    SkPath translatedPath;
                    clipEntry->asPath(&translatedPath);
                    translatedPath.transform(transform, &translatedPath);
                    emit_clip(&translatedPath, nullptr, fContentStream);
                    break;
                }
            }
        }
    }
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
            SkScalar pdfScale = SkScalarMul(state.fTextScaleX,
                                            SkIntToScalar(100));
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
    SkISize size = SkISize::Make(cinfo.fInfo.width(), cinfo.fInfo.height());
    return SkPDFDevice::Create(size, fRasterDpi, fDocument);
}

SkPDFCanon* SkPDFDevice::getCanon() const { return fDocument->canon(); }



// A helper class to automatically finish a ContentEntry at the end of a
// drawing method and maintain the state needed between set up and finish.
class ScopedContentEntry {
public:
    ScopedContentEntry(SkPDFDevice* device, const SkDraw& draw,
                       const SkPaint& paint, bool hasText = false)
        : fDevice(device),
          fContentEntry(nullptr),
          fXfermode(SkXfermode::kSrcOver_Mode),
          fDstFormXObject(nullptr) {
        init(draw.fClipStack, draw.fRC->bwRgn(), *draw.fMatrix, paint, hasText);
    }
    ScopedContentEntry(SkPDFDevice* device, const SkClipStack* clipStack,
                       const SkRegion& clipRegion, const SkMatrix& matrix,
                       const SkPaint& paint, bool hasText = false)
        : fDevice(device),
          fContentEntry(nullptr),
          fXfermode(SkXfermode::kSrcOver_Mode),
          fDstFormXObject(nullptr) {
        init(clipStack, clipRegion, matrix, paint, hasText);
    }

    ~ScopedContentEntry() {
        if (fContentEntry) {
            SkPath* shape = &fShape;
            if (shape->isEmpty()) {
                shape = nullptr;
            }
            fDevice->finishContentEntry(fXfermode, std::move(fDstFormXObject), shape);
        }
    }

    SkPDFDevice::ContentEntry* entry() { return fContentEntry; }

    /* Returns true when we explicitly need the shape of the drawing. */
    bool needShape() {
        switch (fXfermode) {
            case SkXfermode::kClear_Mode:
            case SkXfermode::kSrc_Mode:
            case SkXfermode::kSrcIn_Mode:
            case SkXfermode::kSrcOut_Mode:
            case SkXfermode::kDstIn_Mode:
            case SkXfermode::kDstOut_Mode:
            case SkXfermode::kSrcATop_Mode:
            case SkXfermode::kDstATop_Mode:
            case SkXfermode::kModulate_Mode:
                return true;
            default:
                return false;
        }
    }

    /* Returns true unless we only need the shape of the drawing. */
    bool needSource() {
        if (fXfermode == SkXfermode::kClear_Mode) {
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
    SkXfermode::Mode fXfermode;
    sk_sp<SkPDFObject> fDstFormXObject;
    SkPath fShape;

    void init(const SkClipStack* clipStack, const SkRegion& clipRegion,
              const SkMatrix& matrix, const SkPaint& paint, bool hasText) {
        // Shape has to be flatten before we get here.
        if (matrix.hasPerspective()) {
            NOT_IMPLEMENTED(!matrix.hasPerspective(), false);
            return;
        }
        if (paint.getXfermode()) {
            paint.getXfermode()->asMode(&fXfermode);
        }
        fContentEntry = fDevice->setUpContentEntry(clipStack, clipRegion,
                                                   matrix, paint, hasText,
                                                   &fDstFormXObject);
    }
};

////////////////////////////////////////////////////////////////////////////////

SkPDFDevice::SkPDFDevice(SkISize pageSize, SkScalar rasterDpi, SkPDFDocument* doc, bool flip)
    : INHERITED(SkImageInfo::MakeUnknown(pageSize.width(), pageSize.height()),
                SkSurfaceProps(0, kUnknown_SkPixelGeometry))
    , fPageSize(pageSize)
    , fContentSize(pageSize)
    , fExistingClipRegion(SkIRect::MakeSize(pageSize))
    , fRasterDpi(rasterDpi)
    , fDocument(doc) {
    SkASSERT(pageSize.width() > 0);
    SkASSERT(pageSize.height() > 0);

    if (flip) {
        // Skia generally uses the top left as the origin but PDF
        // natively has the origin at the bottom left. This matrix
        // corrects for that.  But that only needs to be done once, we
        // don't do it when layering.
        fInitialTransform.setTranslate(0, SkIntToScalar(pageSize.fHeight));
        fInitialTransform.preScale(SK_Scalar1, -SK_Scalar1);
    } else {
        fInitialTransform.setIdentity();
    }
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

void SkPDFDevice::drawAnnotation(const SkDraw& d, const SkRect& rect, const char key[],
                                 SkData* value) {
    if (0 == rect.width() && 0 == rect.height()) {
        handlePointAnnotation({ rect.x(), rect.y() }, *d.fMatrix, key, value);
    } else {
        SkPath path;
        path.addRect(rect);
        handlePathAnnotation(path, d, key, value);
    }
}

void SkPDFDevice::drawPaint(const SkDraw& d, const SkPaint& paint) {
    SkPaint newPaint = paint;
    replace_srcmode_on_opaque_paint(&newPaint);

    newPaint.setStyle(SkPaint::kFill_Style);
    ScopedContentEntry content(this, d, newPaint);
    internalDrawPaint(newPaint, content.entry());
}

void SkPDFDevice::internalDrawPaint(const SkPaint& paint,
                                    SkPDFDevice::ContentEntry* contentEntry) {
    if (!contentEntry) {
        return;
    }
    SkRect bbox = SkRect::MakeWH(SkIntToScalar(this->width()),
                                 SkIntToScalar(this->height()));
    SkMatrix inverse;
    if (!contentEntry->fState.fMatrix.invert(&inverse)) {
        return;
    }
    inverse.mapRect(&bbox);

    SkPDFUtils::AppendRectangle(bbox, &contentEntry->fContent);
    SkPDFUtils::PaintPath(paint.getStyle(), SkPath::kWinding_FillType,
                          &contentEntry->fContent);
}

void SkPDFDevice::drawPoints(const SkDraw& d,
                             SkCanvas::PointMode mode,
                             size_t count,
                             const SkPoint* points,
                             const SkPaint& srcPaint) {
    SkPaint passedPaint = srcPaint;
    replace_srcmode_on_opaque_paint(&passedPaint);

    if (count == 0) {
        return;
    }

    // SkDraw::drawPoints converts to multiple calls to fDevice->drawPath.
    // We only use this when there's a path effect because of the overhead
    // of multiple calls to setUpContentEntry it causes.
    if (passedPaint.getPathEffect()) {
        if (d.fRC->isEmpty()) {
            return;
        }
        SkDraw pointDraw(d);
        pointDraw.fDevice = this;
        pointDraw.drawPoints(mode, count, points, passedPaint, true);
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
                drawRect(d, r, modifiedPaint);
            }
            return;
        } else {
            modifiedPaint.setStrokeCap(SkPaint::kRound_Cap);
        }
    }

    ScopedContentEntry content(this, d, *paint);
    if (!content.entry()) {
        return;
    }

    switch (mode) {
        case SkCanvas::kPolygon_PointMode:
            SkPDFUtils::MoveTo(points[0].fX, points[0].fY,
                               &content.entry()->fContent);
            for (size_t i = 1; i < count; i++) {
                SkPDFUtils::AppendLine(points[i].fX, points[i].fY,
                                       &content.entry()->fContent);
            }
            SkPDFUtils::StrokePath(&content.entry()->fContent);
            break;
        case SkCanvas::kLines_PointMode:
            for (size_t i = 0; i < count/2; i++) {
                SkPDFUtils::MoveTo(points[i * 2].fX, points[i * 2].fY,
                                   &content.entry()->fContent);
                SkPDFUtils::AppendLine(points[i * 2 + 1].fX,
                                       points[i * 2 + 1].fY,
                                       &content.entry()->fContent);
                SkPDFUtils::StrokePath(&content.entry()->fContent);
            }
            break;
        case SkCanvas::kPoints_PointMode:
            SkASSERT(paint->getStrokeCap() == SkPaint::kRound_Cap);
            for (size_t i = 0; i < count; i++) {
                SkPDFUtils::MoveTo(points[i].fX, points[i].fY,
                                   &content.entry()->fContent);
                SkPDFUtils::ClosePath(&content.entry()->fContent);
                SkPDFUtils::StrokePath(&content.entry()->fContent);
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

void SkPDFDevice::drawRect(const SkDraw& d,
                           const SkRect& rect,
                           const SkPaint& srcPaint) {
    SkPaint paint = srcPaint;
    replace_srcmode_on_opaque_paint(&paint);
    SkRect r = rect;
    r.sort();

    if (paint.getPathEffect()) {
        if (d.fRC->isEmpty()) {
            return;
        }
        SkPath path;
        path.addRect(r);
        drawPath(d, path, paint, nullptr, true);
        return;
    }

    ScopedContentEntry content(this, d, paint);
    if (!content.entry()) {
        return;
    }
    SkPDFUtils::AppendRectangle(r, &content.entry()->fContent);
    SkPDFUtils::PaintPath(paint.getStyle(), SkPath::kWinding_FillType,
                          &content.entry()->fContent);
}

void SkPDFDevice::drawRRect(const SkDraw& draw,
                            const SkRRect& rrect,
                            const SkPaint& srcPaint) {
    SkPaint paint = srcPaint;
    replace_srcmode_on_opaque_paint(&paint);
    SkPath  path;
    path.addRRect(rrect);
    this->drawPath(draw, path, paint, nullptr, true);
}

void SkPDFDevice::drawOval(const SkDraw& draw,
                           const SkRect& oval,
                           const SkPaint& srcPaint) {
    SkPaint paint = srcPaint;
    replace_srcmode_on_opaque_paint(&paint);
    SkPath  path;
    path.addOval(oval);
    this->drawPath(draw, path, paint, nullptr, true);
}

void SkPDFDevice::drawPath(const SkDraw& d,
                           const SkPath& origPath,
                           const SkPaint& srcPaint,
                           const SkMatrix* prePathMatrix,
                           bool pathIsMutable) {
    SkPaint paint = srcPaint;
    replace_srcmode_on_opaque_paint(&paint);
    SkPath modifiedPath;
    SkPath* pathPtr = const_cast<SkPath*>(&origPath);

    SkMatrix matrix = *d.fMatrix;
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
        if (d.fRC->isEmpty()) {
            return;
        }
        if (!pathIsMutable) {
            pathPtr = &modifiedPath;
            pathIsMutable = true;
        }
        bool fill = paint.getFillPath(origPath, pathPtr);

        SkPaint noEffectPaint(paint);
        noEffectPaint.setPathEffect(nullptr);
        if (fill) {
            noEffectPaint.setStyle(SkPaint::kFill_Style);
        } else {
            noEffectPaint.setStyle(SkPaint::kStroke_Style);
            noEffectPaint.setStrokeWidth(0);
        }
        drawPath(d, *pathPtr, noEffectPaint, nullptr, true);
        return;
    }

    if (handleInversePath(d, origPath, paint, pathIsMutable, prePathMatrix)) {
        return;
    }

    ScopedContentEntry content(this, d.fClipStack, d.fRC->bwRgn(), matrix, paint);
    if (!content.entry()) {
        return;
    }
    bool consumeDegeratePathSegments =
           paint.getStyle() == SkPaint::kFill_Style ||
           (paint.getStrokeCap() != SkPaint::kRound_Cap &&
            paint.getStrokeCap() != SkPaint::kSquare_Cap);
    SkPDFUtils::EmitPath(*pathPtr, paint.getStyle(),
                         consumeDegeratePathSegments,
                         &content.entry()->fContent);
    SkPDFUtils::PaintPath(paint.getStyle(), pathPtr->getFillType(),
                          &content.entry()->fContent);
}

void SkPDFDevice::drawBitmapRect(const SkDraw& draw,
                                 const SkBitmap& bitmap,
                                 const SkRect* src,
                                 const SkRect& dst,
                                 const SkPaint& srcPaint,
                                 SkCanvas::SrcRectConstraint constraint) {
    SkASSERT(false);
}

void SkPDFDevice::drawBitmap(const SkDraw& d,
                             const SkBitmap& bitmap,
                             const SkMatrix& matrix,
                             const SkPaint& srcPaint) {
    SkPaint paint = srcPaint;
    if (bitmap.isOpaque()) {
        replace_srcmode_on_opaque_paint(&paint);
    }

    if (d.fRC->isEmpty()) {
        return;
    }

    SkMatrix transform = matrix;
    transform.postConcat(*d.fMatrix);
    SkImageBitmap imageBitmap(bitmap);
    this->internalDrawImage(
            transform, d.fClipStack, d.fRC->bwRgn(), imageBitmap, paint);
}

void SkPDFDevice::drawSprite(const SkDraw& d,
                             const SkBitmap& bitmap,
                             int x,
                             int y,
                             const SkPaint& srcPaint) {
    SkPaint paint = srcPaint;
    if (bitmap.isOpaque()) {
        replace_srcmode_on_opaque_paint(&paint);
    }

    if (d.fRC->isEmpty()) {
        return;
    }

    SkMatrix matrix;
    matrix.setTranslate(SkIntToScalar(x), SkIntToScalar(y));
    SkImageBitmap imageBitmap(bitmap);
    this->internalDrawImage(
            matrix, d.fClipStack, d.fRC->bwRgn(), imageBitmap, paint);
}

void SkPDFDevice::drawImage(const SkDraw& draw,
                            const SkImage* image,
                            SkScalar x,
                            SkScalar y,
                            const SkPaint& srcPaint) {
    SkPaint paint = srcPaint;
    if (!image) {
        return;
    }
    if (image->isOpaque()) {
        replace_srcmode_on_opaque_paint(&paint);
    }
    if (draw.fRC->isEmpty()) {
        return;
    }
    SkMatrix transform = SkMatrix::MakeTrans(x, y);
    transform.postConcat(*draw.fMatrix);
    SkImageBitmap imageBitmap(const_cast<SkImage*>(image));
    this->internalDrawImage(
            transform, draw.fClipStack, draw.fRC->bwRgn(), imageBitmap, paint);
}

void SkPDFDevice::drawImageRect(const SkDraw& draw,
                                const SkImage* image,
                                const SkRect* src,
                                const SkRect& dst,
                                const SkPaint& srcPaint,
                                SkCanvas::SrcRectConstraint constraint) {
    SkASSERT(false);
}

namespace {
class GlyphPositioner {
public:
    GlyphPositioner(SkDynamicMemoryWStream* content,
                    SkScalar textSkewX,
                    bool wideChars,
                    bool defaultPositioning,
                    SkPoint origin)
        : fContent(content)
        , fCurrentMatrixX(0.0f)
        , fCurrentMatrixY(0.0f)
        , fXAdvance(0.0f)
        , fWideChars(wideChars)
        , fInText(false)
        , fDefaultPositioning(defaultPositioning) {
        set_text_transform(origin.x(), origin.y(), textSkewX, fContent);
    }
    ~GlyphPositioner() { SkASSERT(!fInText);  /* flush first */ }
    void flush() {
        if (fInText) {
            fContent->writeText("> Tj\n");
            fInText = false;
        }
    }
    void setWideChars(bool wideChars) {
        if (fWideChars != wideChars) {
            SkASSERT(!fInText);
            SkASSERT(fWideChars == wideChars);
            fWideChars = wideChars;
        }
    }
    void writeGlyph(SkScalar x,
                    SkScalar y,
                    SkScalar advanceWidth,
                    uint16_t glyph) {
        if (!fDefaultPositioning) {
            SkScalar xPosition = x - fCurrentMatrixX;
            SkScalar yPosition = y - fCurrentMatrixY;
            if (xPosition != fXAdvance || yPosition != 0) {
                this->flush();
                SkPDFUtils::AppendScalar(xPosition, fContent);
                fContent->writeText(" ");
                SkPDFUtils::AppendScalar(-yPosition, fContent);
                fContent->writeText(" Td ");
                fCurrentMatrixX = x;
                fCurrentMatrixY = y;
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
    SkScalar fCurrentMatrixX;
    SkScalar fCurrentMatrixY;
    SkScalar fXAdvance;
    bool fWideChars;
    bool fInText;
    const bool fDefaultPositioning;
};
}  // namespace

static void draw_transparent_text(SkPDFDevice* device,
                                  const SkDraw& d,
                                  const void* text, size_t len,
                                  SkScalar x, SkScalar y,
                                  const SkPaint& srcPaint) {
    SkPaint transparent;
    if (!SkPDFFont::CanEmbedTypeface(transparent.getTypeface(),
                                     device->getCanon())) {
        SkDebugf("SkPDF: default typeface should be embeddable");
        return;  // Avoid infinite loop in release.
    }
    transparent.setTextSize(srcPaint.getTextSize());
    transparent.setColor(SK_ColorTRANSPARENT);
    switch (srcPaint.getTextEncoding()) {
        case SkPaint::kGlyphID_TextEncoding: {
            // Since a glyphId<->Unicode mapping is typeface-specific,
            // map back to Unicode first.
            size_t glyphCount = len / 2;
            SkAutoTMalloc<SkUnichar> unichars(glyphCount);
            srcPaint.glyphsToUnichars(
                    (const uint16_t*)text, SkToInt(glyphCount), &unichars[0]);
            transparent.setTextEncoding(SkPaint::kUTF32_TextEncoding);
            // TODO(halcanary): deal with case where default typeface
            // does not have glyphs for these unicode code points.
            device->drawText(d, &unichars[0],
                             glyphCount * sizeof(SkUnichar),
                             x, y, transparent);
            break;
        }
        case SkPaint::kUTF8_TextEncoding:
        case SkPaint::kUTF16_TextEncoding:
        case SkPaint::kUTF32_TextEncoding:
            transparent.setTextEncoding(srcPaint.getTextEncoding());
            device->drawText(d, text, len, x, y, transparent);
            break;
        default:
            SkFAIL("unknown text encoding");
    }
}

void SkPDFDevice::internalDrawText(
        const SkDraw& d, const void* sourceText, size_t sourceByteCount,
        const SkScalar pos[], SkTextBlob::GlyphPositioning positioning,
        SkPoint offset, const SkPaint& srcPaint) {
    NOT_IMPLEMENTED(srcPaint.getMaskFilter() != nullptr, false);
    if (srcPaint.getMaskFilter() != nullptr) {
        // Don't pretend we support drawing MaskFilters, it makes for artifacts
        // making text unreadable (e.g. same text twice when using CSS shadows).
        return;
    }
    SkPaint paint = calculate_text_paint(srcPaint);
    replace_srcmode_on_opaque_paint(&paint);
    if (!paint.getTypeface()) {
        paint.setTypeface(SkTypeface::MakeDefault());
    }
    SkTypeface* typeface = paint.getTypeface();
    if (!typeface) {
        SkDebugf("SkPDF: SkTypeface::MakeDefault() returned nullptr.\n");
        return;
    }
    int typefaceGlyphCount = typeface->countGlyphs();
    if (typefaceGlyphCount < 1) {
        SkDebugf("SkPDF: SkTypeface has no glyphs.\n");
        return;
    }
    if (!SkPDFFont::CanEmbedTypeface(typeface, fDocument->canon())) {
        SkPath path; // https://bug.skia.org/3866
        paint.getTextPath(sourceText, sourceByteCount,
                          offset.x(), offset.y(), &path);
        this->drawPath(d, path, srcPaint, &SkMatrix::I(), true);
        // Draw text transparently to make it copyable/searchable/accessable.
        draw_transparent_text(this, d, sourceText, sourceByteCount,
                              offset.x(), offset.y(), paint);
        return;
    }
    // Always make a copy (1) to validate user-input glyphs and
    // (2) because we may modify the glyphs in place (for
    // single-byte-glyph-id PDF fonts).
    int glyphCount = paint.textToGlyphs(sourceText, sourceByteCount, nullptr);
    if (glyphCount <= 0) { return; }
    SkAutoSTMalloc<128, SkGlyphID> glyphStorage(SkToSizeT(glyphCount));
    SkGlyphID* glyphs = glyphStorage.get();
    (void)paint.textToGlyphs(sourceText, sourceByteCount, glyphs);
    if (paint.getTextEncoding() == SkPaint::kGlyphID_TextEncoding) {
        // Validate user-input glyphs.
        SkGlyphID maxGlyphID = SkToU16(typefaceGlyphCount - 1);
        for (int i = 0; i < glyphCount; ++i) {
            glyphs[i] = SkTMin(maxGlyphID, glyphs[i]);
        }
    } else {
        paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    }

    bool defaultPositioning = (positioning == SkTextBlob::kDefault_Positioning);
    SkAutoGlyphCache glyphCache(paint, nullptr, nullptr);

    SkPaint::Align alignment = paint.getTextAlign();
    bool verticalText = paint.isVerticalText();
    if (defaultPositioning && alignment != SkPaint::kLeft_Align) {
        SkScalar advance{0};
        for (int i = 0; i < glyphCount; ++i) {
            advance += glyphCache->getGlyphIDAdvance(glyphs[i]).fAdvanceX;
        }
        SkScalar m = alignment == SkPaint::kCenter_Align
            ? 0.5f * advance : advance;
        offset -= verticalText ? SkPoint{0, m} : SkPoint{m, 0};
    }
    ScopedContentEntry content(this, d, paint, true);
    if (!content.entry()) {
        return;
    }
    SkDynamicMemoryWStream* out = &content.entry()->fContent;
    out->writeText("BT\n");
    if (!this->updateFont(paint, glyphs[0], content.entry())) {
        SkDebugf("SkPDF: Font error.");
        out->writeText("ET\n%SkPDF: Font error.\n");
        return;
    }
    SkPDFFont* font = content.entry()->fState.fFont;
    GlyphPositioner glyphPositioner(out,
                                    paint.getTextSkewX(),
                                    font->multiByteGlyphs(),
                                    defaultPositioning,
                                    offset);
    const SkGlyphID* const glyphsEnd = glyphs + glyphCount;

    while (glyphs < glyphsEnd) {
        font = content.entry()->fState.fFont;
        int stretch = font->multiByteGlyphs()
            ? SkToInt(glyphsEnd - glyphs)
            : font->glyphsToPDFFontEncodingCount(glyphs, SkToInt(glyphsEnd - glyphs));
        SkASSERT(glyphs + stretch <= glyphsEnd);
        if (stretch < 1) {
            SkASSERT(!font->multiByteGlyphs());
            // The current pdf font cannot encode the next glyph.
            // Try to get a pdf font which can encode the next glyph.
            glyphPositioner.flush();
            if (!this->updateFont(paint, *glyphs, content.entry())) {
                SkDebugf("SkPDF: Font error.");
                out->writeText("ET\n%SkPDF: Font error.\n");
                return;
            }
            font = content.entry()->fState.fFont;
            glyphPositioner.setWideChars(font->multiByteGlyphs());
            // try again
            stretch = font->glyphsToPDFFontEncodingCount(glyphs,
                                                         SkToInt(glyphsEnd - glyphs));
            if (stretch < 1) {
                SkDEBUGFAIL("PDF could not encode glyph.");
                glyphPositioner.flush();
                out->writeText("ET\n%SkPDF: Font encoding error.\n");
                return;
            }
        }
        font->noteGlyphUsage(glyphs, stretch);
        if (defaultPositioning) {
            (void)font->glyphsToPDFFontEncoding(glyphs, SkToInt(glyphsEnd - glyphs));
            while (stretch-- > 0) {
                glyphPositioner.writeGlyph(0, 0, 0, *glyphs);
                ++glyphs;
            }
        } else {
            while (stretch-- > 0) {
                SkScalar advance = glyphCache->getGlyphIDAdvance(*glyphs).fAdvanceX;
                SkScalar x = *pos++;
                // evaluate x and y in order!
                SkScalar y = SkTextBlob::kFull_Positioning == positioning ? *pos++ : 0;
                SkPoint xy{x, y};
                if (alignment != SkPaint::kLeft_Align) {
                    SkScalar m = alignment == SkPaint::kCenter_Align
                        ? 0.5f * advance : advance;
                    xy -= verticalText ? SkPoint{0, m} : SkPoint{m, 0};
                }
                (void)font->glyphsToPDFFontEncoding(glyphs, 1);
                glyphPositioner.writeGlyph(xy.x(), xy.y(), advance, *glyphs);
                ++glyphs;
            }
        }
    }
    glyphPositioner.flush();
    out->writeText("ET\n");
}

void SkPDFDevice::drawText(const SkDraw& d, const void* text, size_t len,
                           SkScalar x, SkScalar y, const SkPaint& paint) {
    this->internalDrawText(d, text, len, nullptr, SkTextBlob::kDefault_Positioning,
                           SkPoint{x, y}, paint);
}

void SkPDFDevice::drawPosText(const SkDraw& d, const void* text, size_t len,
                              const SkScalar pos[], int scalarsPerPos,
                              const SkPoint& offset, const SkPaint& paint) {
    this->internalDrawText(d, text, len, pos, (SkTextBlob::GlyphPositioning)scalarsPerPos,
                           offset, paint);
}

void SkPDFDevice::drawVertices(const SkDraw& d, SkCanvas::VertexMode,
                               int vertexCount, const SkPoint verts[],
                               const SkPoint texs[], const SkColor colors[],
                               SkXfermode* xmode, const uint16_t indices[],
                               int indexCount, const SkPaint& paint) {
    if (d.fRC->isEmpty()) {
        return;
    }
    // TODO: implement drawVertices
}

void SkPDFDevice::drawDevice(const SkDraw& d, SkBaseDevice* device,
                             int x, int y, const SkPaint& paint) {
    SkASSERT(!paint.getImageFilter());

    // Check if the source device is really a bitmapdevice (because that's what we returned
    // from createDevice (likely due to an imagefilter)
    SkPixmap pmap;
    if (device->peekPixels(&pmap)) {
        SkBitmap bitmap;
        bitmap.installPixels(pmap);
        this->drawSprite(d, bitmap, x, y, paint);
        return;
    }

    // our onCreateCompatibleDevice() always creates SkPDFDevice subclasses.
    SkPDFDevice* pdfDevice = static_cast<SkPDFDevice*>(device);

    SkScalar scalarX = SkIntToScalar(x);
    SkScalar scalarY = SkIntToScalar(y);
    for (const RectWithData& l : pdfDevice->fLinkToURLs) {
        SkRect r = l.rect.makeOffset(scalarX, scalarY);
        fLinkToURLs.emplace_back(r, l.data.get());
    }
    for (const RectWithData& l : pdfDevice->fLinkToDestinations) {
        SkRect r = l.rect.makeOffset(scalarX, scalarY);
        fLinkToDestinations.emplace_back(r, l.data.get());
    }
    for (const NamedDestination& d : pdfDevice->fNamedDestinations) {
        SkPoint p = d.point + SkPoint::Make(scalarX, scalarY);
        fNamedDestinations.emplace_back(d.nameData.get(), p);
    }

    if (pdfDevice->isContentEmpty()) {
        return;
    }

    SkMatrix matrix;
    matrix.setTranslate(SkIntToScalar(x), SkIntToScalar(y));
    ScopedContentEntry content(this, d.fClipStack, d.fRC->bwRgn(), matrix, paint);
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
    SkPDFUtils::DrawFormXObject(this->addXObjectResource(xObject.get()),
                                &content.entry()->fContent);
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
    this->writeContent(&buffer);
    return std::unique_ptr<SkStreamAsset>(
            buffer.bytesWritten() > 0
            ? buffer.detachAsStream()
            : new SkMemoryStream);
}

void SkPDFDevice::writeContent(SkWStream* out) const {
    if (fInitialTransform.getType() != SkMatrix::kIdentity_Mask) {
        SkPDFUtils::AppendTransform(fInitialTransform, out);
    }

    // If the content area is the entire page, then we don't need to clip
    // the content area (PDF area clips to the page size).  Otherwise,
    // we have to clip to the content area; we've already applied the
    // initial transform, so just clip to the device size.
    if (fPageSize != fContentSize) {
        SkRect r = SkRect::MakeWH(SkIntToScalar(this->width()),
                                  SkIntToScalar(this->height()));
        emit_clip(nullptr, &r, out);
    }

    GraphicStackState gsState(fExistingClipStack, fExistingClipRegion, out);
    for (const auto& entry : fContentEntries) {
        SkPoint translation;
        translation.iset(this->getOrigin());
        translation.negate();
        gsState.updateClip(entry.fState.fClipStack, entry.fState.fClipRegion,
                           translation);
        gsState.updateMatrix(entry.fState.fMatrix);
        gsState.updateDrawingState(entry.fState);

        entry.fContent.writeToStream(out);
    }
    gsState.drainStack();
}

/* Draws an inverse filled path by using Path Ops to compute the positive
 * inverse using the current clip as the inverse bounds.
 * Return true if this was an inverse path and was properly handled,
 * otherwise returns false and the normal drawing routine should continue,
 * either as a (incorrect) fallback or because the path was not inverse
 * in the first place.
 */
bool SkPDFDevice::handleInversePath(const SkDraw& d, const SkPath& origPath,
                                    const SkPaint& paint, bool pathIsMutable,
                                    const SkMatrix* prePathMatrix) {
    if (!origPath.isInverseFillType()) {
        return false;
    }

    if (d.fRC->isEmpty()) {
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
            drawPath(d, modifiedPath, paint, nullptr, true);
            return true;
        }
    }

    // Get bounds of clip in current transform space
    // (clip bounds are given in device space).
    SkRect bounds;
    SkMatrix transformInverse;
    SkMatrix totalMatrix = *d.fMatrix;
    if (prePathMatrix) {
        totalMatrix.preConcat(*prePathMatrix);
    }
    if (!totalMatrix.invert(&transformInverse)) {
        return false;
    }
    bounds.set(d.fRC->getBounds());
    transformInverse.mapRect(&bounds);

    // Extend the bounds by the line width (plus some padding)
    // so the edge doesn't cause a visible stroke.
    bounds.outset(paint.getStrokeWidth() + SK_Scalar1,
                  paint.getStrokeWidth() + SK_Scalar1);

    if (!calculate_inverse_path(bounds, *pathPtr, &modifiedPath)) {
        return false;
    }

    drawPath(d, modifiedPath, noInversePaint, prePathMatrix, true);
    return true;
}

void SkPDFDevice::handlePointAnnotation(const SkPoint& point,
                                        const SkMatrix& matrix,
                                        const char key[], SkData* value) {
    if (!value) {
        return;
    }

    if (!strcmp(SkAnnotationKeys::Define_Named_Dest_Key(), key)) {
        SkPoint transformedPoint;
        matrix.mapXY(point.x(), point.y(), &transformedPoint);
        fNamedDestinations.emplace_back(value, transformedPoint);
    }
}

void SkPDFDevice::handlePathAnnotation(const SkPath& path,
                                       const SkDraw& d,
                                       const char key[], SkData* value) {
    if (!value) {
        return;
    }

    SkPath transformedPath = path;
    transformedPath.transform(*d.fMatrix);
    SkRasterClip clip = *d.fRC;
    clip.op(transformedPath, SkIRect::MakeWH(width(), height()), SkRegion::kIntersect_Op,
            false);
    SkRect transformedRect = SkRect::Make(clip.getBounds());

    if (!strcmp(SkAnnotationKeys::URL_Key(), key)) {
        if (!transformedRect.isEmpty()) {
            fLinkToURLs.emplace_back(transformedRect, value);
        }
    } else if (!strcmp(SkAnnotationKeys::Link_Named_Dest_Key(), key)) {
        if (!transformedRect.isEmpty()) {
            fLinkToDestinations.emplace_back(transformedRect, value);
        }
    }
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

sk_sp<SkPDFObject> SkPDFDevice::makeFormXObjectFromDevice() {
    SkMatrix inverseTransform = SkMatrix::I();
    if (!this->initialTransform().isIdentity()) {
        if (!this->initialTransform().invert(&inverseTransform)) {
            SkDEBUGFAIL("Layer initial transform should be invertible.");
            inverseTransform.reset();
        }
    }
    sk_sp<SkPDFObject> xobject =
        SkPDFMakeFormXObject(this->content(), this->copyMediaBox(),
                             this->makeResourceDict(), inverseTransform, nullptr);
    // We always draw the form xobjects that we create back into the device, so
    // we simply preserve the font usage instead of pulling it out and merging
    // it back in later.
    this->cleanUp();  // Reset this device to have no content.
    this->init();
    return xobject;
}

void SkPDFDevice::drawFormXObjectWithMask(int xObjectIndex,
                                          sk_sp<SkPDFObject> mask,
                                          const SkClipStack* clipStack,
                                          const SkRegion& clipRegion,
                                          SkXfermode::Mode mode,
                                          bool invertClip) {
    if (clipRegion.isEmpty() && !invertClip) {
        return;
    }

    sk_sp<SkPDFDict> sMaskGS = SkPDFGraphicState::GetSMaskGraphicState(
            std::move(mask), invertClip,
            SkPDFGraphicState::kAlpha_SMaskMode, fDocument->canon());

    SkMatrix identity;
    identity.reset();
    SkPaint paint;
    paint.setXfermodeMode(mode);
    ScopedContentEntry content(this, clipStack, clipRegion, identity, paint);
    if (!content.entry()) {
        return;
    }
    SkPDFUtils::ApplyGraphicState(addGraphicStateResource(sMaskGS.get()),
                                  &content.entry()->fContent);
    SkPDFUtils::DrawFormXObject(xObjectIndex, &content.entry()->fContent);

    // Call makeNoSmaskGraphicState() instead of
    // SkPDFGraphicState::MakeNoSmaskGraphicState so that the canon
    // can deduplicate.
    sMaskGS = fDocument->canon()->makeNoSmaskGraphicState();
    SkPDFUtils::ApplyGraphicState(addGraphicStateResource(sMaskGS.get()),
                                  &content.entry()->fContent);
}

SkPDFDevice::ContentEntry* SkPDFDevice::setUpContentEntry(const SkClipStack* clipStack,
                                             const SkRegion& clipRegion,
                                             const SkMatrix& matrix,
                                             const SkPaint& paint,
                                             bool hasText,
                                             sk_sp<SkPDFObject>* dst) {
    *dst = nullptr;
    if (clipRegion.isEmpty()) {
        return nullptr;
    }

    // The clip stack can come from an SkDraw where it is technically optional.
    SkClipStack synthesizedClipStack;
    if (clipStack == nullptr) {
        if (clipRegion == fExistingClipRegion) {
            clipStack = &fExistingClipStack;
        } else {
            // GraphicStackState::updateClip expects the clip stack to have
            // fExistingClip as a prefix, so start there, then set the clip
            // to the passed region.
            synthesizedClipStack = fExistingClipStack;
            SkPath clipPath;
            clipRegion.getBoundaryPath(&clipPath);
            synthesizedClipStack.clipDevPath(clipPath, SkRegion::kReplace_Op,
                                             false);
            clipStack = &synthesizedClipStack;
        }
    }

    SkXfermode::Mode xfermode = SkXfermode::kSrcOver_Mode;
    if (paint.getXfermode()) {
        paint.getXfermode()->asMode(&xfermode);
    }

    // For the following modes, we want to handle source and destination
    // separately, so make an object of what's already there.
    if (xfermode == SkXfermode::kClear_Mode       ||
            xfermode == SkXfermode::kSrc_Mode     ||
            xfermode == SkXfermode::kSrcIn_Mode   ||
            xfermode == SkXfermode::kDstIn_Mode   ||
            xfermode == SkXfermode::kSrcOut_Mode  ||
            xfermode == SkXfermode::kDstOut_Mode  ||
            xfermode == SkXfermode::kSrcATop_Mode ||
            xfermode == SkXfermode::kDstATop_Mode ||
            xfermode == SkXfermode::kModulate_Mode) {
        if (!isContentEmpty()) {
            *dst = this->makeFormXObjectFromDevice();
            SkASSERT(isContentEmpty());
        } else if (xfermode != SkXfermode::kSrc_Mode &&
                   xfermode != SkXfermode::kSrcOut_Mode) {
            // Except for Src and SrcOut, if there isn't anything already there,
            // then we're done.
            return nullptr;
        }
    }
    // TODO(vandebo): Figure out how/if we can handle the following modes:
    // Xor, Plus.

    // Dst xfer mode doesn't draw source at all.
    if (xfermode == SkXfermode::kDst_Mode) {
        return nullptr;
    }

    SkPDFDevice::ContentEntry* entry;
    if (fContentEntries.back() && fContentEntries.back()->fContent.getOffset() == 0) {
        entry = fContentEntries.back();
    } else if (xfermode != SkXfermode::kDstOver_Mode) {
        entry = fContentEntries.emplace_back();
    } else {
        entry = fContentEntries.emplace_front();
    }
    populateGraphicStateEntryFromPaint(matrix, *clipStack, clipRegion, paint,
                                       hasText, &entry->fState);
    return entry;
}

void SkPDFDevice::finishContentEntry(SkXfermode::Mode xfermode,
                                     sk_sp<SkPDFObject> dst,
                                     SkPath* shape) {
    if (xfermode != SkXfermode::kClear_Mode       &&
            xfermode != SkXfermode::kSrc_Mode     &&
            xfermode != SkXfermode::kDstOver_Mode &&
            xfermode != SkXfermode::kSrcIn_Mode   &&
            xfermode != SkXfermode::kDstIn_Mode   &&
            xfermode != SkXfermode::kSrcOut_Mode  &&
            xfermode != SkXfermode::kDstOut_Mode  &&
            xfermode != SkXfermode::kSrcATop_Mode &&
            xfermode != SkXfermode::kDstATop_Mode &&
            xfermode != SkXfermode::kModulate_Mode) {
        SkASSERT(!dst);
        return;
    }
    if (xfermode == SkXfermode::kDstOver_Mode) {
        SkASSERT(!dst);
        if (fContentEntries.front()->fContent.getOffset() == 0) {
            // For DstOver, an empty content entry was inserted before the rest
            // of the content entries. If nothing was drawn, it needs to be
            // removed.
            fContentEntries.pop_front();
        }
        return;
    }
    if (!dst) {
        SkASSERT(xfermode == SkXfermode::kSrc_Mode ||
                 xfermode == SkXfermode::kSrcOut_Mode);
        return;
    }

    SkASSERT(dst);
    SkASSERT(fContentEntries.count() == 1);
    // Changing the current content into a form-xobject will destroy the clip
    // objects which is fine since the xobject will already be clipped. However
    // if source has shape, we need to clip it too, so a copy of the clip is
    // saved.

    SkClipStack clipStack = fContentEntries.front()->fState.fClipStack;
    SkRegion clipRegion = fContentEntries.front()->fState.fClipRegion;

    SkMatrix identity;
    identity.reset();
    SkPaint stockPaint;

    sk_sp<SkPDFObject> srcFormXObject;
    if (isContentEmpty()) {
        // If nothing was drawn and there's no shape, then the draw was a
        // no-op, but dst needs to be restored for that to be true.
        // If there is shape, then an empty source with Src, SrcIn, SrcOut,
        // DstIn, DstAtop or Modulate reduces to Clear and DstOut or SrcAtop
        // reduces to Dst.
        if (shape == nullptr || xfermode == SkXfermode::kDstOut_Mode ||
                xfermode == SkXfermode::kSrcATop_Mode) {
            ScopedContentEntry content(this, &fExistingClipStack,
                                       fExistingClipRegion, identity,
                                       stockPaint);
            // TODO: addXObjectResource take sk_sp
            SkPDFUtils::DrawFormXObject(this->addXObjectResource(dst.get()),
                                        &content.entry()->fContent);
            return;
        } else {
            xfermode = SkXfermode::kClear_Mode;
        }
    } else {
        SkASSERT(fContentEntries.count() == 1);
        srcFormXObject = this->makeFormXObjectFromDevice();
    }

    // TODO(vandebo) srcFormXObject may contain alpha, but here we want it
    // without alpha.
    if (xfermode == SkXfermode::kSrcATop_Mode) {
        // TODO(vandebo): In order to properly support SrcATop we have to track
        // the shape of what's been drawn at all times. It's the intersection of
        // the non-transparent parts of the device and the outlines (shape) of
        // all images and devices drawn.
        drawFormXObjectWithMask(addXObjectResource(srcFormXObject.get()), dst,
                                &fExistingClipStack, fExistingClipRegion,
                                SkXfermode::kSrcOver_Mode, true);
    } else {
        if (shape != nullptr) {
            // Draw shape into a form-xobject.
            SkRasterClip rc(clipRegion);
            SkDraw d;
            d.fMatrix = &identity;
            d.fRC = &rc;
            d.fClipStack = &clipStack;
            SkPaint filledPaint;
            filledPaint.setColor(SK_ColorBLACK);
            filledPaint.setStyle(SkPaint::kFill_Style);
            this->drawPath(d, *shape, filledPaint, nullptr, true);
            drawFormXObjectWithMask(addXObjectResource(dst.get()),
                                    this->makeFormXObjectFromDevice(),
                                    &fExistingClipStack, fExistingClipRegion,
                                    SkXfermode::kSrcOver_Mode, true);

        } else {
            drawFormXObjectWithMask(addXObjectResource(dst.get()), srcFormXObject,
                                    &fExistingClipStack, fExistingClipRegion,
                                    SkXfermode::kSrcOver_Mode, true);
        }
    }

    if (xfermode == SkXfermode::kClear_Mode) {
        return;
    } else if (xfermode == SkXfermode::kSrc_Mode ||
            xfermode == SkXfermode::kDstATop_Mode) {
        ScopedContentEntry content(this, &fExistingClipStack,
                                   fExistingClipRegion, identity, stockPaint);
        if (content.entry()) {
            SkPDFUtils::DrawFormXObject(
                    this->addXObjectResource(srcFormXObject.get()),
                    &content.entry()->fContent);
        }
        if (xfermode == SkXfermode::kSrc_Mode) {
            return;
        }
    } else if (xfermode == SkXfermode::kSrcATop_Mode) {
        ScopedContentEntry content(this, &fExistingClipStack,
                                   fExistingClipRegion, identity, stockPaint);
        if (content.entry()) {
            SkPDFUtils::DrawFormXObject(this->addXObjectResource(dst.get()),
                                        &content.entry()->fContent);
        }
    }

    SkASSERT(xfermode == SkXfermode::kSrcIn_Mode   ||
             xfermode == SkXfermode::kDstIn_Mode   ||
             xfermode == SkXfermode::kSrcOut_Mode  ||
             xfermode == SkXfermode::kDstOut_Mode  ||
             xfermode == SkXfermode::kSrcATop_Mode ||
             xfermode == SkXfermode::kDstATop_Mode ||
             xfermode == SkXfermode::kModulate_Mode);

    if (xfermode == SkXfermode::kSrcIn_Mode ||
            xfermode == SkXfermode::kSrcOut_Mode ||
            xfermode == SkXfermode::kSrcATop_Mode) {
        drawFormXObjectWithMask(addXObjectResource(srcFormXObject.get()),
                                std::move(dst),
                                &fExistingClipStack, fExistingClipRegion,
                                SkXfermode::kSrcOver_Mode,
                                xfermode == SkXfermode::kSrcOut_Mode);
        return;
    } else {
        SkXfermode::Mode mode = SkXfermode::kSrcOver_Mode;
        int resourceID = addXObjectResource(dst.get());
        if (xfermode == SkXfermode::kModulate_Mode) {
            drawFormXObjectWithMask(addXObjectResource(srcFormXObject.get()),
                                    std::move(dst), &fExistingClipStack,
                                    fExistingClipRegion,
                                    SkXfermode::kSrcOver_Mode, false);
            mode = SkXfermode::kMultiply_Mode;
        }
        drawFormXObjectWithMask(resourceID, std::move(srcFormXObject),
                                &fExistingClipStack, fExistingClipRegion, mode,
                                xfermode == SkXfermode::kDstOut_Mode);
        return;
    }
}

bool SkPDFDevice::isContentEmpty() {
    if (!fContentEntries.front() || fContentEntries.front()->fContent.getOffset() == 0) {
        SkASSERT(fContentEntries.count() <= 1);
        return true;
    }
    return false;
}

void SkPDFDevice::populateGraphicStateEntryFromPaint(
        const SkMatrix& matrix,
        const SkClipStack& clipStack,
        const SkRegion& clipRegion,
        const SkPaint& paint,
        bool hasText,
        SkPDFDevice::GraphicStateEntry* entry) {
    NOT_IMPLEMENTED(paint.getPathEffect() != nullptr, false);
    NOT_IMPLEMENTED(paint.getMaskFilter() != nullptr, false);
    NOT_IMPLEMENTED(paint.getColorFilter() != nullptr, false);

    entry->fMatrix = matrix;
    entry->fClipStack = clipStack;
    entry->fClipRegion = clipRegion;
    entry->fColor = SkColorSetA(paint.getColor(), 0xFF);
    entry->fShaderIndex = -1;

    // PDF treats a shader as a color, so we only set one or the other.
    sk_sp<SkPDFObject> pdfShader;
    SkShader* shader = paint.getShader();
    SkColor color = paint.getColor();
    if (shader) {
        // PDF positions patterns relative to the initial transform, so
        // we need to apply the current transform to the shader parameters.
        SkMatrix transform = matrix;
        transform.postConcat(fInitialTransform);

        // PDF doesn't support kClamp_TileMode, so we simulate it by making
        // a pattern the size of the current clip.
        SkIRect bounds = clipRegion.getBounds();

        // We need to apply the initial transform to bounds in order to get
        // bounds in a consistent coordinate system.
        SkRect boundsTemp;
        boundsTemp.set(bounds);
        fInitialTransform.mapRect(&boundsTemp);
        boundsTemp.roundOut(&bounds);

        SkScalar rasterScale =
                SkIntToScalar(fRasterDpi) / DPI_FOR_RASTER_SCALE_ONE;
        pdfShader = SkPDFShader::GetPDFShader(
                fDocument, fRasterDpi, shader, transform, bounds, rasterScale);

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
        } else {
            // A color shader is treated as an invalid shader so we don't have
            // to set a shader just for a color.
            SkShader::GradientInfo gradientInfo;
            SkColor gradientColor;
            gradientInfo.fColors = &gradientColor;
            gradientInfo.fColorOffsets = nullptr;
            gradientInfo.fColorCount = 1;
            if (shader->asAGradient(&gradientInfo) ==
                    SkShader::kColor_GradientType) {
                entry->fColor = SkColorSetA(gradientColor, 0xFF);
                color = gradientColor;
            }
        }
    }

    sk_sp<SkPDFGraphicState> newGraphicState;
    if (color == paint.getColor()) {
        newGraphicState.reset(
                SkPDFGraphicState::GetGraphicStateForPaint(fDocument->canon(), paint));
    } else {
        SkPaint newPaint = paint;
        newPaint.setColor(color);
        newGraphicState.reset(
                SkPDFGraphicState::GetGraphicStateForPaint(fDocument->canon(), newPaint));
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

bool SkPDFDevice::updateFont(const SkPaint& paint, uint16_t glyphID,
                             SkPDFDevice::ContentEntry* contentEntry) {
    SkTypeface* typeface = paint.getTypeface();
    if (contentEntry->fState.fFont == nullptr ||
            contentEntry->fState.fTextSize != paint.getTextSize() ||
            !contentEntry->fState.fFont->hasGlyph(glyphID)) {
        int fontIndex = getFontResourceIndex(typeface, glyphID);
        if (fontIndex < 0) {
            return false;
        }
        contentEntry->fContent.writeText("/");
        contentEntry->fContent.writeText(SkPDFResourceDict::getResourceName(
                SkPDFResourceDict::kFont_ResourceType,
                fontIndex).c_str());
        contentEntry->fContent.writeText(" ");
        SkPDFUtils::AppendScalar(paint.getTextSize(), &contentEntry->fContent);
        contentEntry->fContent.writeText(" Tf\n");
        contentEntry->fState.fFont = fFontResources[fontIndex];
    }
    return true;
}

int SkPDFDevice::getFontResourceIndex(SkTypeface* typeface, uint16_t glyphID) {
    sk_sp<SkPDFFont> newFont(
            SkPDFFont::GetFontResource(fDocument->canon(), typeface, glyphID));
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

static SkSize rect_to_size(const SkRect& r) {
    return SkSize::Make(r.width(), r.height());
}

static sk_sp<SkImage> color_filter(const SkImageBitmap& imageBitmap,
                                   SkColorFilter* colorFilter) {
    auto surface =
        SkSurface::MakeRaster(SkImageInfo::MakeN32Premul(imageBitmap.dimensions()));
    SkASSERT(surface);
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorTRANSPARENT);
    SkPaint paint;
    paint.setColorFilter(sk_ref_sp(colorFilter));
    imageBitmap.draw(canvas, &paint);
    canvas->flush();
    return surface->makeImageSnapshot();
}

////////////////////////////////////////////////////////////////////////////////
void SkPDFDevice::internalDrawImage(const SkMatrix& origMatrix,
                                    const SkClipStack* clipStack,
                                    const SkRegion& origClipRegion,
                                    SkImageBitmap imageBitmap,
                                    const SkPaint& paint) {
    if (imageBitmap.dimensions().isZero()) {
        return;
    }
    #ifdef SK_PDF_IMAGE_STATS
    gDrawImageCalls.fetch_add(1);
    #endif
    SkMatrix matrix = origMatrix;
    SkRegion perspectiveBounds;
    const SkRegion* clipRegion = &origClipRegion;
    sk_sp<SkImage> autoImageUnref;

    // Rasterize the bitmap using perspective in a new bitmap.
    if (origMatrix.hasPerspective()) {
        if (fRasterDpi == 0) {
            return;
        }
        // Transform the bitmap in the new space, without taking into
        // account the initial transform.
        SkPath perspectiveOutline;
        SkRect imageBounds = SkRect::Make(imageBitmap.bounds());
        perspectiveOutline.addRect(imageBounds);
        perspectiveOutline.transform(origMatrix);

        // TODO(edisonn): perf - use current clip too.
        // Retrieve the bounds of the new shape.
        SkRect bounds = perspectiveOutline.getBounds();

        // Transform the bitmap in the new space, taking into
        // account the initial transform.
        SkMatrix total = origMatrix;
        total.postConcat(fInitialTransform);
        SkScalar dpiScale = SkIntToScalar(fRasterDpi) /
                            SkIntToScalar(DPI_FOR_RASTER_SCALE_ONE);
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

        auto surface(SkSurface::MakeRaster(SkImageInfo::MakeN32Premul(wh)));
        if (!surface) {
            return;
        }
        SkCanvas* canvas = surface->getCanvas();
        canvas->clear(SK_ColorTRANSPARENT);

        SkScalar deltaX = bounds.left();
        SkScalar deltaY = bounds.top();

        SkMatrix offsetMatrix = origMatrix;
        offsetMatrix.postTranslate(-deltaX, -deltaY);
        offsetMatrix.postScale(scaleX, scaleY);

        // Translate the draw in the new canvas, so we perfectly fit the
        // shape in the bitmap.
        canvas->setMatrix(offsetMatrix);
        imageBitmap.draw(canvas, nullptr);
        // Make sure the final bits are in the bitmap.
        canvas->flush();

        // In the new space, we use the identity matrix translated
        // and scaled to reflect DPI.
        matrix.setScale(1 / scaleX, 1 / scaleY);
        matrix.postTranslate(deltaX, deltaY);

        perspectiveBounds.setRect(bounds.roundOut());
        clipRegion = &perspectiveBounds;

        autoImageUnref = surface->makeImageSnapshot();
        imageBitmap = SkImageBitmap(autoImageUnref.get());
    }

    SkMatrix scaled;
    // Adjust for origin flip.
    scaled.setScale(SK_Scalar1, -SK_Scalar1);
    scaled.postTranslate(0, SK_Scalar1);
    // Scale the image up from 1x1 to WxH.
    SkIRect subset = imageBitmap.bounds();
    scaled.postScale(SkIntToScalar(imageBitmap.dimensions().width()),
                     SkIntToScalar(imageBitmap.dimensions().height()));
    scaled.postConcat(matrix);
    ScopedContentEntry content(this, clipStack, *clipRegion, scaled, paint);
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
        autoImageUnref = color_filter(imageBitmap, colorFilter);
        imageBitmap = SkImageBitmap(autoImageUnref.get());
        // TODO(halcanary): de-dupe this by caching filtered images.
        // (maybe in the resource cache?)
    }

    SkBitmapKey key = imageBitmap.getKey();
    sk_sp<SkPDFObject> pdfimage = fDocument->canon()->findPDFBitmap(key);
    if (!pdfimage) {
        sk_sp<SkImage> img = imageBitmap.makeImage();
        if (!img) {
            return;
        }
        pdfimage = SkPDFCreateBitmapObject(
                std::move(img), fDocument->canon()->getPixelSerializer());
        if (!pdfimage) {
            return;
        }
        fDocument->serialize(pdfimage);  // serialize images early.
        fDocument->canon()->addPDFBitmap(key, pdfimage);
    }
    // TODO(halcanary): addXObjectResource() should take a sk_sp<SkPDFObject>
    SkPDFUtils::DrawFormXObject(this->addXObjectResource(pdfimage.get()),
                                &content.entry()->fContent);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkSpecialImage.h"
#include "SkImageFilter.h"

void SkPDFDevice::drawSpecial(const SkDraw& draw, SkSpecialImage* srcImg, int x, int y,
                                 const SkPaint& paint) {
    SkASSERT(!srcImg->isTextureBacked());

    SkBitmap resultBM;

    SkImageFilter* filter = paint.getImageFilter();
    if (filter) {
        SkIPoint offset = SkIPoint::Make(0, 0);
        SkMatrix matrix = *draw.fMatrix;
        matrix.postTranslate(SkIntToScalar(-x), SkIntToScalar(-y));
        const SkIRect clipBounds = draw.fRC->getBounds().makeOffset(-x, -y);
//        SkAutoTUnref<SkImageFilterCache> cache(this->getImageFilterCache());
        SkImageFilter::Context ctx(matrix, clipBounds, nullptr /*cache.get()*/);

        sk_sp<SkSpecialImage> resultImg(filter->filterImage(srcImg, ctx, &offset));
        if (resultImg) {
            SkPaint tmpUnfiltered(paint);
            tmpUnfiltered.setImageFilter(nullptr);
            if (resultImg->getROPixels(&resultBM)) {
                this->drawSprite(draw, resultBM, x + offset.x(), y + offset.y(), tmpUnfiltered);
            }
        }
    } else {
        if (srcImg->getROPixels(&resultBM)) {
            this->drawSprite(draw, resultBM, x, y, paint);
        }
    }
}

sk_sp<SkSpecialImage> SkPDFDevice::makeSpecial(const SkBitmap& bitmap) {
    return SkSpecialImage::MakeFromRaster(bitmap.bounds(), bitmap);
}

sk_sp<SkSpecialImage> SkPDFDevice::makeSpecial(const SkImage* image) {
    return SkSpecialImage::MakeFromImage(SkIRect::MakeWH(image->width(), image->height()),
                                         image->makeNonTextureImage());
}

sk_sp<SkSpecialImage> SkPDFDevice::snapSpecial() {
    return nullptr;
}
