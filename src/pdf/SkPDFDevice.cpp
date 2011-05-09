/*
 * Copyright (C) 2011 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SkPDFDevice.h"

#include "SkColor.h"
#include "SkClipStack.h"
#include "SkDraw.h"
#include "SkGlyphCache.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPDFFont.h"
#include "SkPDFFormXObject.h"
#include "SkPDFGraphicState.h"
#include "SkPDFImage.h"
#include "SkPDFShader.h"
#include "SkPDFStream.h"
#include "SkPDFTypes.h"
#include "SkPDFUtils.h"
#include "SkRect.h"
#include "SkString.h"
#include "SkTextFormatParams.h"
#include "SkTypeface.h"
#include "SkTypes.h"

// Utility functions

static void emit_pdf_color(SkColor color, SkWStream* result) {
    SkASSERT(SkColorGetA(color) == 0xFF);  // We handle alpha elsewhere.
    SkScalar colorMax = SkIntToScalar(0xFF);
    SkPDFScalar::Append(
            SkScalarDiv(SkIntToScalar(SkColorGetR(color)), colorMax), result);
    result->writeText(" ");
    SkPDFScalar::Append(
            SkScalarDiv(SkIntToScalar(SkColorGetG(color)), colorMax), result);
    result->writeText(" ");
    SkPDFScalar::Append(
            SkScalarDiv(SkIntToScalar(SkColorGetB(color)), colorMax), result);
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
        if (result.getStyle() == SkPaint::kFill_Style)
            result.setStyle(SkPaint::kStrokeAndFill_Style);
        else
            width += result.getStrokeWidth();
        result.setStrokeWidth(width);
    }
    return result;
}

// Stolen from measure_text in SkDraw.cpp and then tweaked.
static void align_text(SkDrawCacheProc glyphCacheProc, const SkPaint& paint,
                       const uint16_t* glyphs, size_t len, SkScalar* x,
                       SkScalar* y, SkScalar* width) {
    if (paint.getTextAlign() == SkPaint::kLeft_Align && width == NULL)
        return;

    SkMatrix ident;
    ident.reset();
    SkAutoGlyphCache autoCache(paint, &ident);
    SkGlyphCache* cache = autoCache.getCache();

    const char* start = (char*)glyphs;
    const char* stop = (char*)(glyphs + len);
    SkFixed xAdv = 0, yAdv = 0;

    // TODO(vandebo) This probably needs to take kerning into account.
    while (start < stop) {
        const SkGlyph& glyph = glyphCacheProc(cache, &start, 0, 0);
        xAdv += glyph.fAdvanceX;
        yAdv += glyph.fAdvanceY;
    };
    if (width)
        *width = SkFixedToScalar(xAdv);
    if (paint.getTextAlign() == SkPaint::kLeft_Align)
        return;

    SkScalar xAdj = SkFixedToScalar(xAdv);
    SkScalar yAdj = SkFixedToScalar(yAdv);
    if (paint.getTextAlign() == SkPaint::kCenter_Align) {
        xAdj = SkScalarHalf(xAdj);
        yAdj = SkScalarHalf(yAdj);
    }
    *x = *x - xAdj;
    *y = *y - yAdj;
}

// It is important to not confuse GraphicStateEntry with SkPDFGraphicState, the
// later being our representation of an object in the PDF file.
struct GraphicStateEntry {
    GraphicStateEntry();

    // Compare the fields we care about when setting up a new content entry.
    bool compareInitialState(const GraphicStateEntry& b);

    SkMatrix fMatrix;
    // We can't do set operations on Paths, though PDF natively supports
    // intersect.  If the clip stack does anything other than intersect,
    // we have to fall back to the region.  Treat fClipStack as authoritative.
    // See http://code.google.com/p/skia/issues/detail?id=221
    SkClipStack fClipStack;
    SkRegion fClipRegion;

    // When emitting the content entry, we will ensure the graphic state
    // is set to these values first.
    SkColor fColor;
    SkScalar fTextScaleX;  // Zero means we don't care what the value is.
    SkPaint::Style fTextFill;  // Only if TextScaleX is non-zero.
    int fShaderIndex;
    int fGraphicStateIndex;

    // We may change the font (i.e. for Type1 support) within a
    // ContentEntry.  This is the one currently in effect, or NULL if none.
    SkPDFFont* fFont;
    // In PDF, text size has no default value. It is only valid if fFont is
    // not NULL.
    SkScalar fTextSize;
};

GraphicStateEntry::GraphicStateEntry() : fColor(SK_ColorBLACK),
                                         fTextScaleX(SK_Scalar1),
                                         fTextFill(SkPaint::kFill_Style),
                                         fShaderIndex(-1),
                                         fGraphicStateIndex(-1),
                                         fFont(NULL),
                                         fTextSize(SK_ScalarNaN) {
    fMatrix.reset();
}

bool GraphicStateEntry::compareInitialState(const GraphicStateEntry& b) {
    return fColor == b.fColor &&
           fShaderIndex == b.fShaderIndex &&
           fGraphicStateIndex == b.fGraphicStateIndex &&
           fMatrix == b.fMatrix &&
           fClipStack == b.fClipStack &&
               (fTextScaleX == 0 ||
                b.fTextScaleX == 0 ||
                (fTextScaleX == b.fTextScaleX && fTextFill == b.fTextFill));
}

struct ContentEntry {
    GraphicStateEntry fState;
    SkDynamicMemoryWStream fContent;
    SkTScopedPtr<ContentEntry> fNext;
};

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
                    const SkIPoint& translation);
    void updateMatrix(const SkMatrix& matrix);
    void updateDrawingState(const GraphicStateEntry& state);

    void drainStack();

private:
    void push();
    void pop();
    GraphicStateEntry* currentEntry() { return &fEntries[fStackDepth]; }

    // Conservative limit on save depth, see impl. notes in PDF 1.4 spec.
    static const int kMaxStackDepth = 12;
    GraphicStateEntry fEntries[kMaxStackDepth + 1];
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

// This function initializes iter to be an interator on the "stack" argument
// and then skips over the leading entries as specified in prefix.  It requires
// and asserts that "prefix" will be a prefix to "stack."
static void skip_clip_stack_prefix(const SkClipStack& prefix,
                                   const SkClipStack& stack,
                                   SkClipStack::B2FIter* iter) {
    SkClipStack::B2FIter prefixIter(prefix);
    iter->reset(stack);

    const SkClipStack::B2FIter::Clip* prefixEntry;
    const SkClipStack::B2FIter::Clip* iterEntry;

    for (prefixEntry = prefixIter.next(); prefixEntry;
            prefixEntry = prefixIter.next()) {
        iterEntry = iter->next();
        SkASSERT(iterEntry);
        SkASSERT(*prefixEntry == *iterEntry);
    }

    SkASSERT(prefixEntry == NULL);
}

static void emit_clip(SkPath* clipPath, SkRect* clipRect,
                      SkWStream* contentStream) {
    SkASSERT(clipPath || clipRect);

    SkPath::FillType clipFill;
    if (clipPath) {
        SkPDFUtils::EmitPath(*clipPath, contentStream);
        clipFill = clipPath->getFillType();
    } else if (clipRect) {
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

// TODO(vandebo) Take advantage of SkClipStack::getSaveCount(), the PDF
// graphic state stack, and the fact that we can know all the clips used
// on the page to optimize this.
void GraphicStackState::updateClip(const SkClipStack& clipStack,
                                   const SkRegion& clipRegion,
                                   const SkIPoint& translation) {
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

    // gsState->initialEntry()->fClipStack/Region specifies the clip that has
    // already been applied.  (If this is a top level device, then it specifies
    // a clip to the content area.  If this is a layer, then it specifies
    // the clip in effect when the layer was created.)  There's no need to
    // reapply that clip; SKCanvas's SkDrawIter will draw anything outside the
    // initial clip on the parent layer.  (This means there's a bug if the user
    // expands the clip and then uses any xfer mode that uses dst:
    // http://code.google.com/p/skia/issues/detail?id=228 )
    SkClipStack::B2FIter iter;
    skip_clip_stack_prefix(fEntries[0].fClipStack, clipStack, &iter);

    // If the clip stack does anything other than intersect or if it uses
    // an inverse fill type, we have to fall back to the clip region.
    bool needRegion = false;
    const SkClipStack::B2FIter::Clip* clipEntry;
    for (clipEntry = iter.next(); clipEntry; clipEntry = iter.next()) {
        if (clipEntry->fOp != SkRegion::kIntersect_Op ||
                (clipEntry->fPath && clipEntry->fPath->isInverseFillType())) {
            needRegion = true;
            break;
        }
    }

    if (needRegion) {
        SkPath clipPath;
        SkAssertResult(clipRegion.getBoundaryPath(&clipPath));
        emit_clip(&clipPath, NULL, fContentStream);
    } else {
        skip_clip_stack_prefix(fEntries[0].fClipStack, clipStack, &iter);
        SkMatrix transform;
        transform.setTranslate(translation.fX, translation.fY);
        const SkClipStack::B2FIter::Clip* clipEntry;
        for (clipEntry = iter.next(); clipEntry; clipEntry = iter.next()) {
            SkASSERT(clipEntry->fOp == SkRegion::kIntersect_Op);
            if (clipEntry->fRect) {
                SkRect translatedClip;
                transform.mapRect(&translatedClip, *clipEntry->fRect);
                emit_clip(NULL, &translatedClip, fContentStream);
            } else if (clipEntry->fPath) {
                SkPath translatedPath;
                clipEntry->fPath->transform(transform, &translatedPath);
                emit_clip(&translatedPath, NULL, fContentStream);
            } else {
                SkASSERT(false);
            }
        }
    }
    currentEntry()->fClipStack = clipStack;
    currentEntry()->fClipRegion = clipRegion;
}

static void append_clip_stack(const SkClipStack& src, SkClipStack* dst) {
    SkClipStack::B2FIter iter(src);
    const SkClipStack::B2FIter::Clip* clipEntry;
    for (clipEntry = iter.next(); clipEntry; clipEntry = iter.next()) {
        if (clipEntry->fRect) {
            dst->clipDevRect(*clipEntry->fRect, clipEntry->fOp);
        } else if (clipEntry->fPath) {
            dst->clipDevPath(*clipEntry->fPath, clipEntry->fOp);
        } else {
            // There's not an easy way to add an empty clip with an arbitrary
            // op, but an empty path (we've already used an empty rect) will
            // work just fine for our purposes.
            SkPath empty;
            dst->clipDevPath(empty, clipEntry->fOp);
        }
    }
}

static void apply_inverse_clip_to_content_entries(
        const SkClipStack& clipStack,
        const SkRegion& clipRegion,
        SkTScopedPtr<ContentEntry>* entries) {
    // I don't see how to find the inverse of a clip stack and apply it to
    // another clip stack.  So until we can do polygon boolean operations,
    // we combine the two clip stacks plus a sentinal with a non-intersect
    // operator to annotate the intent and force use of the clip region while
    // keeping the stacks comparable.
    // Our sentinal is an empty SkRect with kReplace_Op.  This sentinal should
    // not appear in any reasonable real clip stack.
    SkRect empty = SkRect::MakeEmpty();
    SkTScopedPtr<ContentEntry>* entry = entries;
    while (entry->get()) {
        entry->get()->fState.fClipRegion.op(clipRegion,
                                            SkRegion::kDifference_Op);
        if (entry->get()->fState.fClipRegion.isEmpty()) {
            // TODO(vandebo) The content entry we are removing may have been
            // the only one referencing specific resources, we should remove
            // those from our resource lists.
            entry->reset(entry->get()->fNext.release());
            continue;
        }
        entry->get()->fState.fClipStack.save();
        entry->get()->fState.fClipStack.clipDevRect(empty,
                                                    SkRegion::kReplace_Op);
        append_clip_stack(clipStack, &entry->get()->fState.fClipStack);

        entry = &entry->get()->fNext;
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

void GraphicStackState::updateDrawingState(const GraphicStateEntry& state) {
    // PDF treats a shader as a color, so we only set one or the other.
    if (state.fShaderIndex >= 0) {
        if (state.fShaderIndex != currentEntry()->fShaderIndex) {
            fContentStream->writeText("/Pattern CS /Pattern cs /P");
            fContentStream->writeDecAsText(state.fShaderIndex);
            fContentStream->writeText(" SCN /P");
            fContentStream->writeDecAsText(state.fShaderIndex);
            fContentStream->writeText(" scn\n");
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
        fContentStream->writeText("/G");
        fContentStream->writeDecAsText(state.fGraphicStateIndex);
        fContentStream->writeText(" gs\n");
        currentEntry()->fGraphicStateIndex = state.fGraphicStateIndex;
    }

    if (state.fTextScaleX) {
        if (state.fTextScaleX != currentEntry()->fTextScaleX) {
            SkScalar pdfScale = SkScalarMul(state.fTextScaleX,
                                            SkIntToScalar(100));
            SkPDFScalar::Append(pdfScale, fContentStream);
            fContentStream->writeText(" Tz\n");
            currentEntry()->fTextScaleX = state.fTextScaleX;
        }
        if (state.fTextFill != currentEntry()->fTextFill) {
            SK_COMPILE_ASSERT(SkPaint::kFill_Style == 0, enum_must_match_value);
            SK_COMPILE_ASSERT(SkPaint::kStroke_Style == 1,
                              enum_must_match_value);
            SK_COMPILE_ASSERT(SkPaint::kStrokeAndFill_Style == 2,
                              enum_must_match_value);
            fContentStream->writeDecAsText(state.fTextFill);
            fContentStream->writeText(" Tr\n");
            currentEntry()->fTextFill = state.fTextFill;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

SkDevice* SkPDFDeviceFactory::newDevice(SkCanvas* c, SkBitmap::Config config,
                                        int width, int height, bool isOpaque,
                                        bool isForLayer) {
    SkMatrix initialTransform;
    initialTransform.reset();
    SkISize size = SkISize::Make(width, height);
    if (isForLayer) {
        return SkNEW_ARGS(SkPDFDevice, (size, c->getTotalClipStack(),
                                        c->getTotalClip()));
    } else {
        return SkNEW_ARGS(SkPDFDevice, (size, size, initialTransform));
    }
}

static inline SkBitmap makeContentBitmap(const SkISize& contentSize,
                                         const SkMatrix* initialTransform) {
    SkBitmap bitmap;
    if (initialTransform) {
        // Compute the size of the drawing area.
        SkVector drawingSize;
        SkMatrix inverse;
        drawingSize.set(contentSize.fWidth, contentSize.fHeight);
        initialTransform->invert(&inverse);
        inverse.mapVectors(&drawingSize, 1);
        SkISize size = SkSize::Make(drawingSize.fX, drawingSize.fY).toRound();
        bitmap.setConfig(SkBitmap::kNo_Config, abs(size.fWidth),
                         abs(size.fHeight));
    } else {
        bitmap.setConfig(SkBitmap::kNo_Config, abs(contentSize.fWidth),
                         abs(contentSize.fHeight));
    }

    return bitmap;
}

SkPDFDevice::SkPDFDevice(const SkISize& pageSize, const SkISize& contentSize,
                         const SkMatrix& initialTransform)
    : SkDevice(NULL, makeContentBitmap(contentSize, &initialTransform), false),
      fPageSize(pageSize),
      fContentSize(contentSize),
      fCurrentContentEntry(NULL) {
    // Skia generally uses the top left as the origin but PDF natively has the
    // origin at the bottom left. This matrix corrects for that.  But that only
    // needs to be done once, we don't do it when layering.
    fInitialTransform.setTranslate(0, pageSize.fHeight);
    fInitialTransform.preScale(1, -1);
    fInitialTransform.preConcat(initialTransform);

    SkIRect existingClip = SkIRect::MakeWH(this->width(), this->height());
    fExistingClipStack.clipDevRect(existingClip);
    fExistingClipRegion.setRect(existingClip);

    this->init();
}

SkPDFDevice::SkPDFDevice(const SkISize& layerSize,
                         const SkClipStack& existingClipStack,
                         const SkRegion& existingClipRegion)
    : SkDevice(NULL, makeContentBitmap(layerSize, NULL), false),
      fPageSize(layerSize),
      fContentSize(layerSize),
      fExistingClipStack(existingClipStack),
      fExistingClipRegion(existingClipRegion),
      fCurrentContentEntry(NULL) {
    fInitialTransform.reset();
    this->init();
}

SkPDFDevice::~SkPDFDevice() {
    this->cleanUp();
}

void SkPDFDevice::init() {
    fResourceDict = NULL;
    fContentEntries.reset();
    fCurrentContentEntry = NULL;
}

SkDeviceFactory* SkPDFDevice::onNewDeviceFactory() {
    return SkNEW(SkPDFDeviceFactory);
}

void SkPDFDevice::cleanUp() {
    fGraphicStateResources.unrefAll();
    fXObjectResources.unrefAll();
    fFontResources.unrefAll();
    fShaderResources.unrefAll();
}

void SkPDFDevice::clear(SkColor color) {
    this->cleanUp();
    this->init();

    SkPaint paint;
    paint.setColor(color);
    paint.setStyle(SkPaint::kFill_Style);
    SkMatrix identity;
    identity.reset();
    if (!setUpContentEntry(fExistingClipStack, fExistingClipRegion, identity,
                           paint)) {
        return;
    }

    internalDrawPaint(paint);
}

void SkPDFDevice::drawPaint(const SkDraw& d, const SkPaint& paint) {
    SkPaint newPaint = paint;
    newPaint.setStyle(SkPaint::kFill_Style);
    if (!setUpContentEntry(*d.fClipStack, *d.fClip, *d.fMatrix, newPaint)) {
        return;
    }

    internalDrawPaint(newPaint);
}

void SkPDFDevice::internalDrawPaint(const SkPaint& paint) {
    SkRect bbox = SkRect::MakeWH(SkIntToScalar(this->width()),
                                 SkIntToScalar(this->height()));
    SkMatrix totalTransform = fInitialTransform;
    totalTransform.preConcat(fCurrentContentEntry->fState.fMatrix);
    SkMatrix inverse;
    inverse.reset();
    totalTransform.invert(&inverse);
    inverse.mapRect(&bbox);

    SkPDFUtils::AppendRectangle(bbox, &fCurrentContentEntry->fContent);
    SkPDFUtils::PaintPath(paint.getStyle(), SkPath::kWinding_FillType,
                          &fCurrentContentEntry->fContent);
}

void SkPDFDevice::drawPoints(const SkDraw& d, SkCanvas::PointMode mode,
                             size_t count, const SkPoint* points,
                             const SkPaint& passedPaint) {
    if (count == 0) {
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


    if (!setUpContentEntry(*d.fClipStack, *d.fClip, *d.fMatrix, *paint)) {
        return;
    }

    switch (mode) {
        case SkCanvas::kPolygon_PointMode:
            SkPDFUtils::MoveTo(points[0].fX, points[0].fY,
                               &fCurrentContentEntry->fContent);
            for (size_t i = 1; i < count; i++) {
                SkPDFUtils::AppendLine(points[i].fX, points[i].fY,
                                       &fCurrentContentEntry->fContent);
            }
            SkPDFUtils::StrokePath(&fCurrentContentEntry->fContent);
            break;
        case SkCanvas::kLines_PointMode:
            for (size_t i = 0; i < count/2; i++) {
                SkPDFUtils::MoveTo(points[i * 2].fX, points[i * 2].fY,
                                   &fCurrentContentEntry->fContent);
                SkPDFUtils::AppendLine(points[i * 2 + 1].fX,
                                       points[i * 2 + 1].fY,
                                       &fCurrentContentEntry->fContent);
                SkPDFUtils::StrokePath(&fCurrentContentEntry->fContent);
            }
            break;
        case SkCanvas::kPoints_PointMode:
            SkASSERT(paint->getStrokeCap() == SkPaint::kRound_Cap);
            for (size_t i = 0; i < count; i++) {
                SkPDFUtils::MoveTo(points[i].fX, points[i].fY,
                                   &fCurrentContentEntry->fContent);
                SkPDFUtils::ClosePath(&fCurrentContentEntry->fContent);
                SkPDFUtils::StrokePath(&fCurrentContentEntry->fContent);
            }
            break;
        default:
            SkASSERT(false);
    }
}

void SkPDFDevice::drawRect(const SkDraw& d, const SkRect& r,
                           const SkPaint& paint) {
    if (paint.getPathEffect()) {
        if (d.fClip->isEmpty()) {
            return;
        }
        // Create a path for the rectangle and apply the path effect to it.
        SkPath path;
        path.addRect(r);
        paint.getFillPath(path, &path);

        SkPaint noEffectPaint(paint);
        SkSafeUnref(noEffectPaint.setPathEffect(NULL));
        drawPath(d, path, noEffectPaint, NULL, true);
        return;
    }
    if (!setUpContentEntry(*d.fClipStack, *d.fClip, *d.fMatrix, paint)) {
        return;
    }

    SkPDFUtils::AppendRectangle(r, &fCurrentContentEntry->fContent);
    SkPDFUtils::PaintPath(paint.getStyle(), SkPath::kWinding_FillType,
                          &fCurrentContentEntry->fContent);
}

void SkPDFDevice::drawPath(const SkDraw& d, const SkPath& path,
                           const SkPaint& paint, const SkMatrix* prePathMatrix,
                           bool pathIsMutable) {
    NOT_IMPLEMENTED(prePathMatrix != NULL, true);

    if (paint.getPathEffect()) {
        if (d.fClip->isEmpty()) {
            return;
        }
        // Apply the path effect to path and draw it that way.
        SkPath noEffectPath;
        paint.getFillPath(path, &noEffectPath);

        SkPaint noEffectPaint(paint);
        SkSafeUnref(noEffectPaint.setPathEffect(NULL));
        drawPath(d, noEffectPath, noEffectPaint, NULL, true);
        return;
    }
    if (!setUpContentEntry(*d.fClipStack, *d.fClip, *d.fMatrix, paint)) {
        return;
    }

    SkPDFUtils::EmitPath(path, &fCurrentContentEntry->fContent);
    SkPDFUtils::PaintPath(paint.getStyle(), path.getFillType(),
                          &fCurrentContentEntry->fContent);
}

void SkPDFDevice::drawBitmap(const SkDraw& d, const SkBitmap& bitmap,
                             const SkIRect* srcRect, const SkMatrix& matrix,
                             const SkPaint& paint) {
    if (d.fClip->isEmpty()) {
        return;
    }

    SkMatrix transform = matrix;
    transform.postConcat(*d.fMatrix);
    internalDrawBitmap(transform, *d.fClipStack, *d.fClip, bitmap, srcRect,
                       paint);
}

void SkPDFDevice::drawSprite(const SkDraw& d, const SkBitmap& bitmap,
                             int x, int y, const SkPaint& paint) {
    if (d.fClip->isEmpty()) {
        return;
    }

    SkMatrix matrix;
    matrix.setTranslate(SkIntToScalar(x), SkIntToScalar(y));
    internalDrawBitmap(matrix, *d.fClipStack, *d.fClip, bitmap, NULL, paint);
}

void SkPDFDevice::drawText(const SkDraw& d, const void* text, size_t len,
                           SkScalar x, SkScalar y, const SkPaint& paint) {
    SkPaint textPaint = calculate_text_paint(paint);
    if (!setUpContentEntryForText(*d.fClipStack, *d.fClip, *d.fMatrix,
                                  textPaint)) {
        return;
    }

    // We want the text in glyph id encoding and a writable buffer, so we end
    // up making a copy either way.
    size_t numGlyphs = paint.textToGlyphs(text, len, NULL);
    uint16_t* glyphIDs =
        (uint16_t*)sk_malloc_flags(numGlyphs * 2,
                                   SK_MALLOC_TEMP | SK_MALLOC_THROW);
    SkAutoFree autoFreeGlyphIDs(glyphIDs);
    if (paint.getTextEncoding() != SkPaint::kGlyphID_TextEncoding) {
        paint.textToGlyphs(text, len, glyphIDs);
        textPaint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    } else {
        SkASSERT((len & 1) == 0);
        SkASSERT(len / 2 == numGlyphs);
        memcpy(glyphIDs, text, len);
    }

    SkScalar width;
    SkScalar* widthPtr = NULL;
    if (textPaint.isUnderlineText() || textPaint.isStrikeThruText())
        widthPtr = &width;

    SkDrawCacheProc glyphCacheProc = textPaint.getDrawCacheProc();
    align_text(glyphCacheProc, textPaint, glyphIDs, numGlyphs, &x, &y,
               widthPtr);
    fCurrentContentEntry->fContent.writeText("BT\n");
    setTextTransform(x, y, textPaint.getTextSkewX());
    size_t consumedGlyphCount = 0;
    while (numGlyphs > consumedGlyphCount) {
        updateFont(textPaint, glyphIDs[consumedGlyphCount]);
        SkPDFFont* font = fCurrentContentEntry->fState.fFont;
        size_t availableGlyphs =
            font->glyphsToPDFFontEncoding(glyphIDs + consumedGlyphCount,
                                          numGlyphs - consumedGlyphCount);
        SkString encodedString =
            SkPDFString::formatString(glyphIDs + consumedGlyphCount,
                                      availableGlyphs, font->multiByteGlyphs());
        fCurrentContentEntry->fContent.writeText(encodedString.c_str());
        consumedGlyphCount += availableGlyphs;
        fCurrentContentEntry->fContent.writeText(" Tj\n");
    }
    fCurrentContentEntry->fContent.writeText("ET\n");

    // Draw underline and/or strikethrough if the paint has them.
    // drawPosText() and drawTextOnPath() don't draw underline or strikethrough
    // because the raster versions don't.  Use paint instead of textPaint
    // because we may have changed strokeWidth to do fakeBold text.
    if (paint.isUnderlineText() || paint.isStrikeThruText()) {
        SkScalar textSize = paint.getTextSize();
        SkScalar height = SkScalarMul(textSize, kStdUnderline_Thickness);

        if (paint.isUnderlineText()) {
            SkScalar top = SkScalarMulAdd(textSize, kStdUnderline_Offset, y);
            SkRect r = SkRect::MakeXYWH(x, top - height, width, height);
            drawRect(d, r, paint);
        }
        if (paint.isStrikeThruText()) {
            SkScalar top = SkScalarMulAdd(textSize, kStdStrikeThru_Offset, y);
            SkRect r = SkRect::MakeXYWH(x, top - height, width, height);
            drawRect(d, r, paint);
        }
    }
}

void SkPDFDevice::drawPosText(const SkDraw& d, const void* text, size_t len,
                              const SkScalar pos[], SkScalar constY,
                              int scalarsPerPos, const SkPaint& paint) {
    SkASSERT(1 == scalarsPerPos || 2 == scalarsPerPos);
    SkPaint textPaint = calculate_text_paint(paint);
    if (!setUpContentEntryForText(*d.fClipStack, *d.fClip, *d.fMatrix,
                                  textPaint)) {
        return;
    }

    // Make sure we have a glyph id encoding.
    SkAutoFree glyphStorage;
    uint16_t* glyphIDs;
    size_t numGlyphs;
    if (paint.getTextEncoding() != SkPaint::kGlyphID_TextEncoding) {
        numGlyphs = paint.textToGlyphs(text, len, NULL);
        glyphIDs = (uint16_t*)sk_malloc_flags(numGlyphs * 2,
                                              SK_MALLOC_TEMP | SK_MALLOC_THROW);
        glyphStorage.set(glyphIDs);
        paint.textToGlyphs(text, len, glyphIDs);
        textPaint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    } else {
        SkASSERT((len & 1) == 0);
        numGlyphs = len / 2;
        glyphIDs = (uint16_t*)text;
    }

    SkDrawCacheProc glyphCacheProc = textPaint.getDrawCacheProc();
    fCurrentContentEntry->fContent.writeText("BT\n");
    updateFont(textPaint, glyphIDs[0]);
    for (size_t i = 0; i < numGlyphs; i++) {
        SkPDFFont* font = fCurrentContentEntry->fState.fFont;
        uint16_t encodedValue = glyphIDs[i];
        if (font->glyphsToPDFFontEncoding(&encodedValue, 1) != 1) {
            updateFont(textPaint, glyphIDs[i]);
            i--;
            continue;
        }
        SkScalar x = pos[i * scalarsPerPos];
        SkScalar y = scalarsPerPos == 1 ? constY : pos[i * scalarsPerPos + 1];
        align_text(glyphCacheProc, textPaint, glyphIDs + i, 1, &x, &y, NULL);
        setTextTransform(x, y, textPaint.getTextSkewX());
        SkString encodedString =
            SkPDFString::formatString(&encodedValue, 1,
                                      font->multiByteGlyphs());
        fCurrentContentEntry->fContent.writeText(encodedString.c_str());
        fCurrentContentEntry->fContent.writeText(" Tj\n");
    }
    fCurrentContentEntry->fContent.writeText("ET\n");
}

void SkPDFDevice::drawTextOnPath(const SkDraw& d, const void* text, size_t len,
                                 const SkPath& path, const SkMatrix* matrix,
                                 const SkPaint& paint) {
    if (d.fClip->isEmpty()) {
        return;
    }
    NOT_IMPLEMENTED("drawTextOnPath", true);
}

void SkPDFDevice::drawVertices(const SkDraw& d, SkCanvas::VertexMode,
                               int vertexCount, const SkPoint verts[],
                               const SkPoint texs[], const SkColor colors[],
                               SkXfermode* xmode, const uint16_t indices[],
                               int indexCount, const SkPaint& paint) {
    if (d.fClip->isEmpty()) {
        return;
    }
    NOT_IMPLEMENTED("drawVerticies", true);
}

void SkPDFDevice::drawDevice(const SkDraw& d, SkDevice* device, int x, int y,
                             const SkPaint& paint) {
    if ((device->getDeviceCapabilities() & kVector_Capability) == 0) {
        // If we somehow get a raster device, do what our parent would do.
        SkDevice::drawDevice(d, device, x, y, paint);
        return;
    }

    SkMatrix matrix;
    matrix.setTranslate(SkIntToScalar(x), SkIntToScalar(y));
    if (!setUpContentEntry(*d.fClipStack, *d.fClip, matrix, paint)) {
        return;
    }

    // Assume that a vector capable device means that it's a PDF Device.
    SkPDFDevice* pdfDevice = static_cast<SkPDFDevice*>(device);
    SkPDFFormXObject* xobject = new SkPDFFormXObject(pdfDevice);
    fXObjectResources.push(xobject);  // Transfer reference.
    fCurrentContentEntry->fContent.writeText("/X");
    fCurrentContentEntry->fContent.writeDecAsText(
            fXObjectResources.count() - 1);
    fCurrentContentEntry->fContent.writeText(" Do\n");
}

const SkRefPtr<SkPDFDict>& SkPDFDevice::getResourceDict() {
    if (fResourceDict.get() == NULL) {
        fResourceDict = new SkPDFDict;
        fResourceDict->unref();  // SkRefPtr and new both took a reference.

        if (fGraphicStateResources.count()) {
            SkRefPtr<SkPDFDict> extGState = new SkPDFDict();
            extGState->unref();  // SkRefPtr and new both took a reference.
            for (int i = 0; i < fGraphicStateResources.count(); i++) {
                SkString nameString("G");
                nameString.appendS32(i);
                extGState->insert(
                        nameString.c_str(),
                        new SkPDFObjRef(fGraphicStateResources[i]))->unref();
            }
            fResourceDict->insert("ExtGState", extGState.get());
        }

        if (fXObjectResources.count()) {
            SkRefPtr<SkPDFDict> xObjects = new SkPDFDict();
            xObjects->unref();  // SkRefPtr and new both took a reference.
            for (int i = 0; i < fXObjectResources.count(); i++) {
                SkString nameString("X");
                nameString.appendS32(i);
                xObjects->insert(
                        nameString.c_str(),
                        new SkPDFObjRef(fXObjectResources[i]))->unref();
            }
            fResourceDict->insert("XObject", xObjects.get());
        }

        if (fFontResources.count()) {
            SkRefPtr<SkPDFDict> fonts = new SkPDFDict();
            fonts->unref();  // SkRefPtr and new both took a reference.
            for (int i = 0; i < fFontResources.count(); i++) {
                SkString nameString("F");
                nameString.appendS32(i);
                fonts->insert(nameString.c_str(),
                              new SkPDFObjRef(fFontResources[i]))->unref();
            }
            fResourceDict->insert("Font", fonts.get());
        }

        if (fShaderResources.count()) {
            SkRefPtr<SkPDFDict> patterns = new SkPDFDict();
            patterns->unref();  // SkRefPtr and new both took a reference.
            for (int i = 0; i < fShaderResources.count(); i++) {
                SkString nameString("P");
                nameString.appendS32(i);
                patterns->insert(nameString.c_str(),
                                 new SkPDFObjRef(fShaderResources[i]))->unref();
            }
            fResourceDict->insert("Pattern", patterns.get());
        }

        // For compatibility, add all proc sets (only used for output to PS
        // devices).
        const char procs[][7] = {"PDF", "Text", "ImageB", "ImageC", "ImageI"};
        SkRefPtr<SkPDFArray> procSets = new SkPDFArray();
        procSets->unref();  // SkRefPtr and new both took a reference.
        procSets->reserve(SK_ARRAY_COUNT(procs));
        for (size_t i = 0; i < SK_ARRAY_COUNT(procs); i++)
            procSets->append(new SkPDFName(procs[i]))->unref();
        fResourceDict->insert("ProcSet", procSets.get());
    }
    return fResourceDict;
}

void SkPDFDevice::getResources(SkTDArray<SkPDFObject*>* resourceList) const {
    resourceList->setReserve(resourceList->count() +
                             fGraphicStateResources.count() +
                             fXObjectResources.count() +
                             fFontResources.count() +
                             fShaderResources.count());
    for (int i = 0; i < fGraphicStateResources.count(); i++) {
        resourceList->push(fGraphicStateResources[i]);
        fGraphicStateResources[i]->ref();
        fGraphicStateResources[i]->getResources(resourceList);
    }
    for (int i = 0; i < fXObjectResources.count(); i++) {
        resourceList->push(fXObjectResources[i]);
        fXObjectResources[i]->ref();
        fXObjectResources[i]->getResources(resourceList);
    }
    for (int i = 0; i < fFontResources.count(); i++) {
        resourceList->push(fFontResources[i]);
        fFontResources[i]->ref();
        fFontResources[i]->getResources(resourceList);
    }
    for (int i = 0; i < fShaderResources.count(); i++) {
        resourceList->push(fShaderResources[i]);
        fShaderResources[i]->ref();
        fShaderResources[i]->getResources(resourceList);
    }
}

SkRefPtr<SkPDFArray> SkPDFDevice::getMediaBox() const {
    SkRefPtr<SkPDFInt> zero = new SkPDFInt(0);
    zero->unref();  // SkRefPtr and new both took a reference.

    SkRefPtr<SkPDFArray> mediaBox = new SkPDFArray();
    mediaBox->unref();  // SkRefPtr and new both took a reference.
    mediaBox->reserve(4);
    mediaBox->append(zero.get());
    mediaBox->append(zero.get());
    mediaBox->append(new SkPDFInt(fPageSize.fWidth))->unref();
    mediaBox->append(new SkPDFInt(fPageSize.fHeight))->unref();
    return mediaBox;
}

SkStream* SkPDFDevice::content() const {
    SkDynamicMemoryWStream data;
    if (fInitialTransform.getType() != SkMatrix::kIdentity_Mask) {
        SkPDFUtils::AppendTransform(fInitialTransform, &data);
    }
    // If the content area is the entire page, then we don't need to clip
    // the content area (PDF area clips to the page size).  Otherwise,
    // we have to clip to the content area; we've already applied the
    // initial transform, so just clip to the device size.
    if (fPageSize != fContentSize) {
        SkRect r = SkRect::MakeWH(this->width(), this->height());
        emit_clip(NULL, &r, &data);
    }

    GraphicStackState gsState(fExistingClipStack, fExistingClipRegion, &data);
    for (ContentEntry* entry = fContentEntries.get();
            entry != NULL;
            entry = entry->fNext.get()) {
        SkIPoint translation = this->getOrigin();
        translation.negate();
        gsState.updateClip(entry->fState.fClipStack, entry->fState.fClipRegion,
                           translation);
        gsState.updateMatrix(entry->fState.fMatrix);
        gsState.updateDrawingState(entry->fState);
        data.write(entry->fContent.getStream(), entry->fContent.getOffset());
    }
    gsState.drainStack();

    SkMemoryStream* result = new SkMemoryStream;
    result->setMemoryOwned(data.detach(), data.getOffset());
    return result;
}

bool SkPDFDevice::setUpContentEntry(const SkClipStack& clipStack,
                                    const SkRegion& clipRegion,
                                    const SkMatrix& matrix,
                                    const SkPaint& paint,
                                    bool hasText) {
    if (clipRegion.isEmpty()) {
        return false;
    }

    SkXfermode::Mode xfermode = SkXfermode::kSrcOver_Mode;
    if (paint.getXfermode()) {
        paint.getXfermode()->asMode(&xfermode);
    }

    // For Clear and Src modes, we need to push down the inverse of the
    // current clip.
    if (xfermode == SkXfermode::kClear_Mode ||
            xfermode == SkXfermode::kSrc_Mode) {
        apply_inverse_clip_to_content_entries(clipStack, clipRegion,
                                              &fContentEntries);
        // apply_inverse_clip_to_content_entries may have removed entries
        // from fContentEntries and this may have invalidated
        // fCurrentContentEntry. Set it to a known good value.
        fCurrentContentEntry = fContentEntries.get();
    }

    // TODO(vandebo) For the following modes, we need to calculate various
    // operations between the source and destination shape:
    // SrcIn, DstIn, SrcOut, DstOut, SrcAtop, DestAtop, Xor.

    // These xfer modes don't draw source at all.
    if (xfermode == SkXfermode::kClear_Mode ||
            xfermode == SkXfermode::kDst_Mode) {
        return false;
    }

    // If the previous content entry was for DstOver reset fCurrentContentEntry.
    if (fCurrentContentEntry && xfermode != SkXfermode::kDstOver_Mode) {
        while (fCurrentContentEntry->fNext.get()) {
            fCurrentContentEntry = fCurrentContentEntry->fNext.get();
        }
    }

    ContentEntry* entry;
    SkTScopedPtr<ContentEntry> newEntry;
    if (fCurrentContentEntry &&
            fCurrentContentEntry->fContent.getOffset() == 0) {
        entry = fCurrentContentEntry;
    } else {
        newEntry.reset(new ContentEntry);
        entry = newEntry.get();
    }

    populateGraphicStateEntryFromPaint(matrix, clipStack, clipRegion, paint,
                                       hasText, &entry->fState);
    if (fCurrentContentEntry && xfermode != SkXfermode::kDstOver_Mode &&
            entry->fState.compareInitialState(fCurrentContentEntry->fState)) {
        return true;
    }

    if (!fCurrentContentEntry) {
        fContentEntries.reset(entry);
    } else if (xfermode == SkXfermode::kDstOver_Mode) {
        entry->fNext.reset(fContentEntries.release());
        fContentEntries.reset(entry);
    } else {
        fCurrentContentEntry->fNext.reset(entry);
    }
    newEntry.release();
    fCurrentContentEntry = entry;
    return true;
}

bool SkPDFDevice::setUpContentEntryForText(const SkClipStack& clipStack,
                                           const SkRegion& clipRegion,
                                           const SkMatrix& matrix,
                                           const SkPaint& paint) {
    return setUpContentEntry(clipStack, clipRegion, matrix, paint, true);
}

void SkPDFDevice::populateGraphicStateEntryFromPaint(
        const SkMatrix& matrix,
        const SkClipStack& clipStack,
        const SkRegion& clipRegion,
        const SkPaint& paint,
        bool hasText,
        GraphicStateEntry* entry) {
    SkASSERT(paint.getPathEffect() == NULL);

    NOT_IMPLEMENTED(paint.getMaskFilter() != NULL, false);
    NOT_IMPLEMENTED(paint.getColorFilter() != NULL, false);

    entry->fMatrix = matrix;
    entry->fClipStack = clipStack;
    entry->fClipRegion = clipRegion;

    // PDF treats a shader as a color, so we only set one or the other.
    SkRefPtr<SkPDFShader> pdfShader;
    const SkShader* shader = paint.getShader();
    SkColor color = paint.getColor();
    if (shader) {
        // PDF positions patterns relative to the initial transform, so
        // we need to apply the current transform to the shader parameters.
        SkMatrix transform = matrix;
        transform.postConcat(fInitialTransform);

        // PDF doesn't support kClamp_TileMode, so we simulate it by making
        // a pattern the size of the current clip.
        SkIRect bounds = fCurrentContentEntry->fState.fClipRegion.getBounds();
        pdfShader = SkPDFShader::getPDFShader(*shader, transform, bounds);
        SkSafeUnref(pdfShader.get());  // getShader and SkRefPtr both took a ref

        // A color shader is treated as an invalid shader so we don't have
        // to set a shader just for a color.
        if (pdfShader.get() == NULL) {
            entry->fColor = 0;
            color = 0;

            // Check for a color shader.
            SkShader::GradientInfo gradientInfo;
            SkColor gradientColor;
            gradientInfo.fColors = &gradientColor;
            gradientInfo.fColorOffsets = NULL;
            gradientInfo.fColorCount = 1;
            if (shader->asAGradient(&gradientInfo) ==
                    SkShader::kColor_GradientType) {
                entry->fColor = SkColorSetA(gradientColor, 0xFF);
                color = gradientColor;
            }
        }
    }

    if (pdfShader) {
        // pdfShader has been canonicalized so we can directly compare
        // pointers.
        int resourceIndex = fShaderResources.find(pdfShader.get());
        if (resourceIndex < 0) {
            resourceIndex = fShaderResources.count();
            fShaderResources.push(pdfShader.get());
            pdfShader->ref();
        }
        entry->fShaderIndex = resourceIndex;
    } else {
        entry->fShaderIndex = -1;
        entry->fColor = SkColorSetA(paint.getColor(), 0xFF);
        color = paint.getColor();
    }

    SkRefPtr<SkPDFGraphicState> newGraphicState;
    if (color == paint.getColor()) {
        newGraphicState = SkPDFGraphicState::getGraphicStateForPaint(paint);
    } else {
        SkPaint newPaint = paint;
        newPaint.setColor(color);
        newGraphicState = SkPDFGraphicState::getGraphicStateForPaint(newPaint);
    }
    newGraphicState->unref();  // getGraphicState and SkRefPtr both took a ref.
    // newGraphicState has been canonicalized so we can directly compare
    // pointers.
    int resourceIndex = fGraphicStateResources.find(newGraphicState.get());
    if (resourceIndex < 0) {
        resourceIndex = fGraphicStateResources.count();
        fGraphicStateResources.push(newGraphicState.get());
        newGraphicState->ref();
    }
    entry->fGraphicStateIndex = resourceIndex;

    if (hasText) {
        entry->fTextScaleX = paint.getTextScaleX();
        entry->fTextFill = paint.getStyle();
    } else {
        entry->fTextScaleX = 0;
    }
}

void SkPDFDevice::updateFont(const SkPaint& paint, uint16_t glyphID) {
    SkTypeface* typeface = paint.getTypeface();
    if (fCurrentContentEntry->fState.fFont == NULL ||
            !fCurrentContentEntry->fState.fFont->hasGlyph(glyphID)) {
        int fontIndex = getFontResourceIndex(typeface, glyphID);
        fCurrentContentEntry->fContent.writeText("/F");
        fCurrentContentEntry->fContent.writeDecAsText(fontIndex);
        fCurrentContentEntry->fContent.writeText(" ");
        SkPDFScalar::Append(paint.getTextSize(),
                            &fCurrentContentEntry->fContent);
        fCurrentContentEntry->fContent.writeText(" Tf\n");
        fCurrentContentEntry->fState.fFont = fFontResources[fontIndex];
    }
}

int SkPDFDevice::getFontResourceIndex(SkTypeface* typeface, uint16_t glyphID) {
    SkRefPtr<SkPDFFont> newFont = SkPDFFont::getFontResource(typeface, glyphID);
    newFont->unref();  // getFontResource and SkRefPtr both took a ref.
    int resourceIndex = fFontResources.find(newFont.get());
    if (resourceIndex < 0) {
        resourceIndex = fFontResources.count();
        fFontResources.push(newFont.get());
        newFont->ref();
    }
    return resourceIndex;
}

void SkPDFDevice::setTextTransform(SkScalar x, SkScalar y, SkScalar textSkewX) {
    // Flip the text about the x-axis to account for origin swap and include
    // the passed parameters.
    fCurrentContentEntry->fContent.writeText("1 0 ");
    SkPDFScalar::Append(0 - textSkewX, &fCurrentContentEntry->fContent);
    fCurrentContentEntry->fContent.writeText(" -1 ");
    SkPDFScalar::Append(x, &fCurrentContentEntry->fContent);
    fCurrentContentEntry->fContent.writeText(" ");
    SkPDFScalar::Append(y, &fCurrentContentEntry->fContent);
    fCurrentContentEntry->fContent.writeText(" Tm\n");
}

void SkPDFDevice::internalDrawBitmap(const SkMatrix& matrix,
                                     const SkClipStack& clipStack,
                                     const SkRegion& clipRegion,
                                     const SkBitmap& bitmap,
                                     const SkIRect* srcRect,
                                     const SkPaint& paint) {
    SkMatrix scaled;
    // Adjust for origin flip.
    scaled.setScale(1, -1);
    scaled.postTranslate(0, 1);
    // Scale the image up from 1x1 to WxH.
    SkIRect subset = SkIRect::MakeWH(bitmap.width(), bitmap.height());
    scaled.postScale(SkIntToScalar(subset.width()),
                     SkIntToScalar(subset.height()));
    scaled.postConcat(matrix);
    if (!setUpContentEntry(clipStack, clipRegion, scaled, paint)) {
        return;
    }

    if (srcRect && !subset.intersect(*srcRect)) {
        return;
    }

    SkPDFImage* image = SkPDFImage::CreateImage(bitmap, subset, paint);
    if (!image) {
        return;
    }

    fXObjectResources.push(image);  // Transfer reference.
    fCurrentContentEntry->fContent.writeText("/X");
    fCurrentContentEntry->fContent.writeDecAsText(
            fXObjectResources.count() - 1);
    fCurrentContentEntry->fContent.writeText(" Do\n");
}
