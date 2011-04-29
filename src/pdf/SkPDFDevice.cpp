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

namespace {

void emitPDFColor(SkColor color, SkWStream* result) {
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

SkPaint calculateTextPaint(const SkPaint& paint) {
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
void alignText(SkDrawCacheProc glyphCacheProc, const SkPaint& paint,
               const uint16_t* glyphs, size_t len, SkScalar* x, SkScalar* y,
               SkScalar* width) {
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

}  // namespace

////////////////////////////////////////////////////////////////////////////////

SkDevice* SkPDFDeviceFactory::newDevice(SkCanvas*, SkBitmap::Config config,
                                        int width, int height, bool isOpaque,
                                        bool isForLayer) {
    SkMatrix initialTransform;
    initialTransform.reset();
    if (isForLayer) {
        initialTransform.setTranslate(0, height);
        initialTransform.preScale(1, -1);
    }
    SkISize size = SkISize::Make(width, height);
    return SkNEW_ARGS(SkPDFDevice, (size, size, initialTransform));
}

static inline SkBitmap makeContentBitmap(const SkISize& contentSize,
                                         const SkMatrix& initialTransform) {
    // Compute the size of the drawing area.
    SkVector drawingSize;
    SkMatrix inverse;
    drawingSize.set(contentSize.fWidth, contentSize.fHeight);
    initialTransform.invert(&inverse);
    inverse.mapVectors(&drawingSize, 1);
    SkISize size = SkSize::Make(drawingSize.fX, drawingSize.fY).toRound();

    SkBitmap bitmap;
    bitmap.setConfig(SkBitmap::kNo_Config, size.fWidth, size.fHeight);
    return bitmap;
}

SkPDFDevice::SkPDFDevice(const SkISize& pageSize, const SkISize& contentSize,
                         const SkMatrix& initialTransform)
    : SkDevice(NULL, makeContentBitmap(contentSize, initialTransform), false),
      fPageSize(pageSize),
      fGraphicStackIndex(0) {
    // Skia generally uses the top left as the origin but PDF natively has the
    // origin at the bottom left. This matrix corrects for that.  When layering,
    // we specify an inverse correction to cancel this out.
    fInitialTransform.setTranslate(0, pageSize.fHeight);
    fInitialTransform.preScale(1, -1);
    fInitialTransform.preConcat(initialTransform);

    this->init();
}

SkPDFDevice::~SkPDFDevice() {
    this->cleanUp();
}

void SkPDFDevice::init() {
    fGraphicStack[0].fColor = SK_ColorBLACK;
    fGraphicStack[0].fTextSize = SK_ScalarNaN;  // This has no default value.
    fGraphicStack[0].fTextScaleX = SK_Scalar1;
    fGraphicStack[0].fTextFill = SkPaint::kFill_Style;
    fGraphicStack[0].fFont = NULL;
    fGraphicStack[0].fShader = NULL;
    fGraphicStack[0].fGraphicState = NULL;
    fGraphicStack[0].fClip.setRect(0, 0, this->width(), this->height());
    fGraphicStack[0].fTransform.reset();
    fGraphicStackIndex = 0;
    fResourceDict = NULL;
    fContent.reset();

    if (fInitialTransform.getType() != SkMatrix::kIdentity_Mask) {
        SkPDFUtils::AppendTransform(fInitialTransform, &fContent);
    }
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
    SkMatrix curTransform = fGraphicStack[fGraphicStackIndex].fTransform;
    SkRegion curClip = fGraphicStack[fGraphicStackIndex].fClip;

    this->cleanUp();
    this->init();

    SkPaint paint;
    paint.setColor(color);
    paint.setStyle(SkPaint::kFill_Style);
    updateGSFromPaint(paint, false);
    internalDrawPaint(paint);

    SkClipStack clipStack;
    setMatrixClip(curTransform, curClip, clipStack);
}

void SkPDFDevice::setMatrixClip(const SkMatrix& matrix,
                                const SkRegion& region,
                                const SkClipStack&) {
    if (region.isEmpty()) {
        return;
    }

    // TODO(vandebo) SkCanvas may not draw anything after doing this call, we
    // should defer writing anything out to fContent until we actually get
    // a draw command.  In fact SkDraw contains the clip and transform, so
    // this method should do nothing and we should have an update
    // transform-and-clip method, like update paint.

    // See the comment in the header file above GraphicStackEntry.
    if (region != fGraphicStack[fGraphicStackIndex].fClip) {
        while (fGraphicStackIndex > 0)
            popGS();

        if (region != fGraphicStack[fGraphicStackIndex].fClip) {
            pushGS();

            SkPath clipPath;
            SkAssertResult(region.getBoundaryPath(&clipPath));

            SkPDFUtils::EmitPath(clipPath, &fContent);
            SkPath::FillType clipFill = clipPath.getFillType();
            NOT_IMPLEMENTED(clipFill == SkPath::kInverseEvenOdd_FillType,
                            false);
            NOT_IMPLEMENTED(clipFill == SkPath::kInverseWinding_FillType,
                            false);
            if (clipFill == SkPath::kEvenOdd_FillType)
                fContent.writeText("W* n ");
            else
                fContent.writeText("W n ");

            fGraphicStack[fGraphicStackIndex].fClip = region;
        }
    }
    setTransform(matrix);
}

void SkPDFDevice::drawPaint(const SkDraw& d, const SkPaint& paint) {
    if (d.fClip->isEmpty()) {
        return;
    }

    SkPaint newPaint = paint;
    newPaint.setStyle(SkPaint::kFill_Style);
    updateGSFromPaint(newPaint, false);

    internalDrawPaint(newPaint);
}

void SkPDFDevice::internalDrawPaint(const SkPaint& paint) {
    SkRect bbox = SkRect::MakeWH(SkIntToScalar(this->width()),
                                 SkIntToScalar(this->height()));
    SkMatrix totalTransform = fInitialTransform;
    totalTransform.preConcat(fGraphicStack[fGraphicStackIndex].fTransform);
    SkMatrix inverse;
    inverse.reset();
    totalTransform.invert(&inverse);
    inverse.mapRect(&bbox);

    internalDrawRect(bbox, paint);
}

void SkPDFDevice::drawPoints(const SkDraw& d, SkCanvas::PointMode mode,
                             size_t count, const SkPoint* points,
                             const SkPaint& paint) {
    if (count == 0 || d.fClip->isEmpty()) {
        return;
    }

    switch (mode) {
        case SkCanvas::kPolygon_PointMode:
            updateGSFromPaint(paint, false);
            SkPDFUtils::MoveTo(points[0].fX, points[0].fY, &fContent);
            for (size_t i = 1; i < count; i++) {
                SkPDFUtils::AppendLine(points[i].fX, points[i].fY, &fContent);
            }
            SkPDFUtils::StrokePath(&fContent);
            break;
        case SkCanvas::kLines_PointMode:
            updateGSFromPaint(paint, false);
            for (size_t i = 0; i < count/2; i++) {
                SkPDFUtils::MoveTo(points[i * 2].fX, points[i * 2].fY,
                                   &fContent);
                SkPDFUtils::AppendLine(points[i * 2 + 1].fX,
                                       points[i * 2 + 1].fY, &fContent);
                SkPDFUtils::StrokePath(&fContent);
            }
            break;
        case SkCanvas::kPoints_PointMode:
            if (paint.getStrokeCap() == SkPaint::kRound_Cap) {
                updateGSFromPaint(paint, false);
                for (size_t i = 0; i < count; i++) {
                    SkPDFUtils::MoveTo(points[i].fX, points[i].fY, &fContent);
                    SkPDFUtils::StrokePath(&fContent);
                }
            } else {
                // PDF won't draw a single point with square/butt caps because
                // the orientation is ambiguous.  Draw a rectangle instead.
                SkPaint newPaint = paint;
                newPaint.setStyle(SkPaint::kFill_Style);
                SkScalar strokeWidth = paint.getStrokeWidth();
                SkScalar halfStroke = strokeWidth * SK_ScalarHalf;
                for (size_t i = 0; i < count; i++) {
                    SkRect r = SkRect::MakeXYWH(points[i].fX, points[i].fY,
                                                0, 0);
                    r.inset(-halfStroke, -halfStroke);
                    drawRect(d, r, newPaint);
                }
            }
            break;
        default:
            SkASSERT(false);
    }
}

void SkPDFDevice::drawRect(const SkDraw& d, const SkRect& r,
                           const SkPaint& paint) {
    if (d.fClip->isEmpty()) {
        return;
    }

    if (paint.getPathEffect()) {
        // Create a path for the rectangle and apply the path effect to it.
        SkPath path;
        path.addRect(r);
        paint.getFillPath(path, &path);

        SkPaint noEffectPaint(paint);
        SkSafeUnref(noEffectPaint.setPathEffect(NULL));
        drawPath(d, path, noEffectPaint, NULL, true);
        return;
    }
    updateGSFromPaint(paint, false);

    internalDrawRect(r, paint);
}

void SkPDFDevice::internalDrawRect(const SkRect& r, const SkPaint& paint) {
    // Skia has 0,0 at top left, pdf at bottom left.  Do the right thing.
    SkScalar bottom = r.fBottom < r.fTop ? r.fBottom : r.fTop;
    SkPDFUtils::AppendRectangle(r.fLeft, bottom, r.width(), r.height(),
                                &fContent);
    SkPDFUtils::PaintPath(paint.getStyle(), SkPath::kWinding_FillType,
                          &fContent);
}

void SkPDFDevice::drawPath(const SkDraw& d, const SkPath& path,
                           const SkPaint& paint, const SkMatrix* prePathMatrix,
                           bool pathIsMutable) {
    if (d.fClip->isEmpty()) {
        return;
    }
    NOT_IMPLEMENTED(prePathMatrix != NULL, true);

    if (paint.getPathEffect()) {
        // Apply the path effect to path and draw it that way.
        SkPath noEffectPath;
        paint.getFillPath(path, &noEffectPath);

        SkPaint noEffectPaint(paint);
        SkSafeUnref(noEffectPaint.setPathEffect(NULL));
        drawPath(d, noEffectPath, noEffectPaint, NULL, true);
        return;
    }
    updateGSFromPaint(paint, false);

    SkPDFUtils::EmitPath(path, &fContent);
    SkPDFUtils::PaintPath(paint.getStyle(), path.getFillType(), &fContent);
}

void SkPDFDevice::drawBitmap(const SkDraw& d, const SkBitmap& bitmap,
                             const SkIRect* srcRect,
                             const SkMatrix& matrix, const SkPaint& paint) {
    if (d.fClip->isEmpty()) {
        return;
    }

    SkMatrix transform = matrix;
    transform.postConcat(fGraphicStack[fGraphicStackIndex].fTransform);
    internalDrawBitmap(transform, bitmap, srcRect, paint);
}

void SkPDFDevice::drawSprite(const SkDraw& d, const SkBitmap& bitmap,
                             int x, int y, const SkPaint& paint) {
    if (d.fClip->isEmpty()) {
        return;
    }

    SkMatrix matrix;
    matrix.setTranslate(SkIntToScalar(x), SkIntToScalar(y));
    internalDrawBitmap(matrix, bitmap, NULL, paint);
}

void SkPDFDevice::drawText(const SkDraw& d, const void* text, size_t len,
                           SkScalar x, SkScalar y, const SkPaint& paint) {
    if (d.fClip->isEmpty()) {
        return;
    }

    SkPaint textPaint = calculateTextPaint(paint);
    updateGSFromPaint(textPaint, true);

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
    alignText(glyphCacheProc, textPaint, glyphIDs, numGlyphs, &x, &y, widthPtr);
    fContent.writeText("BT\n");
    setTextTransform(x, y, textPaint.getTextSkewX());
    size_t consumedGlyphCount = 0;
    while (numGlyphs > consumedGlyphCount) {
        updateFont(textPaint, glyphIDs[consumedGlyphCount]);
        SkPDFFont* font = fGraphicStack[fGraphicStackIndex].fFont;
        size_t availableGlyphs =
            font->glyphsToPDFFontEncoding(glyphIDs + consumedGlyphCount,
                                          numGlyphs - consumedGlyphCount);
        SkString encodedString =
            SkPDFString::formatString(glyphIDs + consumedGlyphCount,
                                      availableGlyphs, font->multiByteGlyphs());
        fContent.writeText(encodedString.c_str());
        consumedGlyphCount += availableGlyphs;
        fContent.writeText(" Tj\n");
    }
    fContent.writeText("ET\n");

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
    if (d.fClip->isEmpty()) {
        return;
    }

    SkASSERT(1 == scalarsPerPos || 2 == scalarsPerPos);
    SkPaint textPaint = calculateTextPaint(paint);
    updateGSFromPaint(textPaint, true);

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
    fContent.writeText("BT\n");
    updateFont(textPaint, glyphIDs[0]);
    for (size_t i = 0; i < numGlyphs; i++) {
        SkPDFFont* font = fGraphicStack[fGraphicStackIndex].fFont;
        uint16_t encodedValue = glyphIDs[i];
        if (font->glyphsToPDFFontEncoding(&encodedValue, 1) != 1) {
            updateFont(textPaint, glyphIDs[i]);
            i--;
            continue;
        }
        SkScalar x = pos[i * scalarsPerPos];
        SkScalar y = scalarsPerPos == 1 ? constY : pos[i * scalarsPerPos + 1];
        alignText(glyphCacheProc, textPaint, glyphIDs + i, 1, &x, &y, NULL);
        setTextTransform(x, y, textPaint.getTextSkewX());
        SkString encodedString =
            SkPDFString::formatString(&encodedValue, 1,
                                      font->multiByteGlyphs());
        fContent.writeText(encodedString.c_str());
        fContent.writeText(" Tj\n");
    }
    fContent.writeText("ET\n");
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
    if (d.fClip->isEmpty()) {
        return;
    }

    if ((device->getDeviceCapabilities() & kVector_Capability) == 0) {
        // If we somehow get a raster device, do what our parent would do.
        SkDevice::drawDevice(d, device, x, y, paint);
        return;
    }
    // Assume that a vector capable device means that it's a PDF Device.
    SkPDFDevice* pdfDevice = static_cast<SkPDFDevice*>(device);

    SkMatrix matrix;
    matrix.setTranslate(SkIntToScalar(x), SkIntToScalar(y));
    SkMatrix curTransform = setTransform(matrix);
    updateGSFromPaint(paint, false);

    SkPDFFormXObject* xobject = new SkPDFFormXObject(pdfDevice);
    fXObjectResources.push(xobject);  // Transfer reference.
    fContent.writeText("/X");
    fContent.writeDecAsText(fXObjectResources.count() - 1);
    fContent.writeText(" Do\n");
    setTransform(curTransform);
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
    size_t offset = fContent.getOffset();
    char* data = (char*)sk_malloc_throw(offset + fGraphicStackIndex * 2);
    fContent.copyTo(data);
    for (int i = 0; i < fGraphicStackIndex; i++) {
        data[offset++] = 'Q';
        data[offset++] = '\n';
    }
    SkMemoryStream* result = new SkMemoryStream;
    result->setMemoryOwned(data, offset);
    return result;
}

void SkPDFDevice::updateGSFromPaint(const SkPaint& paint, bool forText) {
    SkASSERT(paint.getPathEffect() == NULL);

    NOT_IMPLEMENTED(paint.getMaskFilter() != NULL, false);
    NOT_IMPLEMENTED(paint.getColorFilter() != NULL, false);

    SkPaint newPaint = paint;

    // PDF treats a shader as a color, so we only set one or the other.
    SkRefPtr<SkPDFShader> pdfShader;
    const SkShader* shader = newPaint.getShader();
    if (shader) {
        // PDF positions patterns relative to the initial transform, so
        // we need to apply the current transform to the shader parameters.
        SkMatrix transform = fGraphicStack[fGraphicStackIndex].fTransform;
        transform.postConcat(fInitialTransform);

        // PDF doesn't support kClamp_TileMode, so we simulate it by making
        // a pattern the size of the drawing surface.
        SkIRect bounds = fGraphicStack[fGraphicStackIndex].fClip.getBounds();
        pdfShader = SkPDFShader::getPDFShader(*shader, transform, bounds);
        SkSafeUnref(pdfShader.get());  // getShader and SkRefPtr both took a ref

        // A color shader is treated as an invalid shader so we don't have
        // to set a shader just for a color.
        if (pdfShader.get() == NULL) {
            newPaint.setColor(0);

            // Check for a color shader.
            SkShader::GradientInfo gradientInfo;
            SkColor gradientColor;
            gradientInfo.fColors = &gradientColor;
            gradientInfo.fColorOffsets = NULL;
            gradientInfo.fColorCount = 1;
            if (shader->asAGradient(&gradientInfo) ==
                    SkShader::kColor_GradientType) {
                newPaint.setColor(gradientColor);
            }
        }
    }

    if (pdfShader) {
        // pdfShader has been canonicalized so we can directly compare
        // pointers.
        if (fGraphicStack[fGraphicStackIndex].fShader != pdfShader.get()) {
            int resourceIndex = fShaderResources.find(pdfShader.get());
            if (resourceIndex < 0) {
                resourceIndex = fShaderResources.count();
                fShaderResources.push(pdfShader.get());
                pdfShader->ref();
            }
            fContent.writeText("/Pattern CS /Pattern cs /P");
            fContent.writeDecAsText(resourceIndex);
            fContent.writeText(" SCN /P");
            fContent.writeDecAsText(resourceIndex);
            fContent.writeText(" scn\n");
            fGraphicStack[fGraphicStackIndex].fShader = pdfShader.get();
        }
    } else {
        SkColor newColor = newPaint.getColor();
        newColor = SkColorSetA(newColor, 0xFF);
        if (fGraphicStack[fGraphicStackIndex].fShader ||
                fGraphicStack[fGraphicStackIndex].fColor != newColor) {
            emitPDFColor(newColor, &fContent);
            fContent.writeText("RG ");
            emitPDFColor(newColor, &fContent);
            fContent.writeText("rg\n");
            fGraphicStack[fGraphicStackIndex].fColor = newColor;
            fGraphicStack[fGraphicStackIndex].fShader = NULL;
        }
    }

    SkRefPtr<SkPDFGraphicState> newGraphicState =
        SkPDFGraphicState::getGraphicStateForPaint(newPaint);
    newGraphicState->unref();  // getGraphicState and SkRefPtr both took a ref.
    // newGraphicState has been canonicalized so we can directly compare
    // pointers.
    if (fGraphicStack[fGraphicStackIndex].fGraphicState !=
            newGraphicState.get()) {
        int resourceIndex = fGraphicStateResources.find(newGraphicState.get());
        if (resourceIndex < 0) {
            resourceIndex = fGraphicStateResources.count();
            fGraphicStateResources.push(newGraphicState.get());
            newGraphicState->ref();
        }
        fContent.writeText("/G");
        fContent.writeDecAsText(resourceIndex);
        fContent.writeText(" gs\n");
        fGraphicStack[fGraphicStackIndex].fGraphicState = newGraphicState.get();
    }

    if (forText) {
        if (fGraphicStack[fGraphicStackIndex].fTextScaleX !=
                newPaint.getTextScaleX()) {
            SkScalar scale = newPaint.getTextScaleX();
            SkScalar pdfScale = SkScalarMul(scale, SkIntToScalar(100));
            SkPDFScalar::Append(pdfScale, &fContent);
            fContent.writeText(" Tz\n");
            fGraphicStack[fGraphicStackIndex].fTextScaleX = scale;
        }

        if (fGraphicStack[fGraphicStackIndex].fTextFill !=
                newPaint.getStyle()) {
            SK_COMPILE_ASSERT(SkPaint::kFill_Style == 0, enum_must_match_value);
            SK_COMPILE_ASSERT(SkPaint::kStroke_Style == 1,
                              enum_must_match_value);
            SK_COMPILE_ASSERT(SkPaint::kStrokeAndFill_Style == 2,
                              enum_must_match_value);
            fContent.writeDecAsText(newPaint.getStyle());
            fContent.writeText(" Tr\n");
            fGraphicStack[fGraphicStackIndex].fTextFill = newPaint.getStyle();
        }
    }
}

void SkPDFDevice::updateFont(const SkPaint& paint, uint16_t glyphID) {
    SkTypeface* typeface = paint.getTypeface();
    if (fGraphicStack[fGraphicStackIndex].fTextSize != paint.getTextSize() ||
            fGraphicStack[fGraphicStackIndex].fFont == NULL ||
            fGraphicStack[fGraphicStackIndex].fFont->typeface() != typeface ||
            !fGraphicStack[fGraphicStackIndex].fFont->hasGlyph(glyphID)) {
        int fontIndex = getFontResourceIndex(typeface, glyphID);
        fContent.writeText("/F");
        fContent.writeDecAsText(fontIndex);
        fContent.writeText(" ");
        SkPDFScalar::Append(paint.getTextSize(), &fContent);
        fContent.writeText(" Tf\n");
        fGraphicStack[fGraphicStackIndex].fTextSize = paint.getTextSize();
        fGraphicStack[fGraphicStackIndex].fFont = fFontResources[fontIndex];
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

void SkPDFDevice::pushGS() {
    SkASSERT(fGraphicStackIndex < 2);
    fContent.writeText("q\n");
    fGraphicStackIndex++;
    fGraphicStack[fGraphicStackIndex] = fGraphicStack[fGraphicStackIndex - 1];
}

void SkPDFDevice::popGS() {
    SkASSERT(fGraphicStackIndex > 0);
    fContent.writeText("Q\n");
    fGraphicStackIndex--;
}

void SkPDFDevice::setTextTransform(SkScalar x, SkScalar y, SkScalar textSkewX) {
    // Flip the text about the x-axis to account for origin swap and include
    // the passed parameters.
    fContent.writeText("1 0 ");
    SkPDFScalar::Append(0 - textSkewX, &fContent);
    fContent.writeText(" -1 ");
    SkPDFScalar::Append(x, &fContent);
    fContent.writeText(" ");
    SkPDFScalar::Append(y, &fContent);
    fContent.writeText(" Tm\n");
}

void SkPDFDevice::internalDrawBitmap(const SkMatrix& matrix,
                                     const SkBitmap& bitmap,
                                     const SkIRect* srcRect,
                                     const SkPaint& paint) {
    SkIRect subset = SkIRect::MakeWH(bitmap.width(), bitmap.height());
    if (srcRect && !subset.intersect(*srcRect))
        return;

    SkPDFImage* image = SkPDFImage::CreateImage(bitmap, subset, paint);
    if (!image)
        return;

    SkMatrix scaled;
    // Adjust for origin flip.
    scaled.setScale(1, -1);
    scaled.postTranslate(0, 1);
    // Scale the image up from 1x1 to WxH.
    scaled.postScale(SkIntToScalar(subset.width()),
                     SkIntToScalar(subset.height()));
    scaled.postConcat(matrix);
    SkMatrix curTransform = setTransform(scaled);
    updateGSFromPaint(paint, false);

    fXObjectResources.push(image);  // Transfer reference.
    fContent.writeText("/X");
    fContent.writeDecAsText(fXObjectResources.count() - 1);
    fContent.writeText(" Do\n");
    setTransform(curTransform);
}

SkMatrix SkPDFDevice::setTransform(const SkMatrix& m) {
    SkMatrix old = fGraphicStack[fGraphicStackIndex].fTransform;
    if (old == m)
        return old;

    if (old.getType() != SkMatrix::kIdentity_Mask) {
        SkASSERT(fGraphicStackIndex > 0);
        SkASSERT(fGraphicStack[fGraphicStackIndex - 1].fTransform.getType() ==
                 SkMatrix::kIdentity_Mask);
        SkASSERT(fGraphicStack[fGraphicStackIndex].fClip ==
                 fGraphicStack[fGraphicStackIndex - 1].fClip);
        popGS();
    }
    if (m.getType() == SkMatrix::kIdentity_Mask)
        return old;

    if (fGraphicStackIndex == 0 || fGraphicStack[fGraphicStackIndex].fClip !=
            fGraphicStack[fGraphicStackIndex - 1].fClip)
        pushGS();

    SkPDFUtils::AppendTransform(m, &fContent);
    fGraphicStack[fGraphicStackIndex].fTransform = m;

    return old;
}
