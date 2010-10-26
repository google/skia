/*
 * Copyright (C) 2010 The Android Open Source Project
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
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPDFImage.h"
#include "SkPDFGraphicState.h"
#include "SkPDFFormXObject.h"
#include "SkPDFTypes.h"
#include "SkPDFStream.h"
#include "SkRect.h"
#include "SkString.h"

#define NOT_IMPLEMENTED(condition, assert)                         \
    do {                                                           \
        if (condition) {                                           \
            fprintf(stderr, "NOT_IMPLEMENTED: " #condition "\n");  \
            SkASSERT(!assert);                                     \
        }                                                          \
    } while(0)

// Utility functions

namespace {

SkString toPDFColor(SkColor color) {
    SkASSERT(SkColorGetA(color) == 0xFF);  // We handle alpha elsewhere.
    SkScalar colorMax = SkIntToScalar(0xFF);
    SkString result;
    result.appendScalar(SkScalarDiv(SkIntToScalar(SkColorGetR(color)),
                                    colorMax));
    result.append(" ");
    result.appendScalar(SkScalarDiv(SkIntToScalar(SkColorGetG(color)),
                                    colorMax));
    result.append(" ");
    result.appendScalar(SkScalarDiv(SkIntToScalar(SkColorGetB(color)),
                                    colorMax));
    result.append(" ");
    return result;
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////

SkDevice* SkPDFDeviceFactory::newDevice(SkBitmap::Config config,
                                        int width, int height,
                                        bool isOpaque, bool isForLayer) {
    return SkNEW_ARGS(SkPDFDevice, (width, height));
}

SkPDFDevice::SkPDFDevice(int width, int height)
    : fWidth(width),
      fHeight(height),
      fCurrentColor(0),
      fCurrentTextScaleX(SK_Scalar1) {
    fContent.append("q\n");
    fCurTransform.reset();
    fActiveTransform.reset();
}

SkPDFDevice::~SkPDFDevice() {
    fGraphicStateResources.unrefAll();
    fXObjectResources.unrefAll();
}

void SkPDFDevice::setMatrixClip(const SkMatrix& matrix,
                                const SkRegion& region) {
    // TODO(vandebo) handle clipping
    setTransform(matrix);
    fCurTransform = matrix;
}

void SkPDFDevice::drawPaint(const SkDraw& d, const SkPaint& paint) {
    setNoTransform();

    SkPaint newPaint = paint;
    newPaint.setStyle(SkPaint::kFill_Style);
    updateGSFromPaint(newPaint, NULL);

    SkRect all = SkRect::MakeWH(width() + 1, height() + 1);
    drawRect(d, all, newPaint);
    setTransform(fCurTransform);
}

void SkPDFDevice::drawPoints(const SkDraw& d, SkCanvas::PointMode mode,
                             size_t count, const SkPoint* points,
                             const SkPaint& paint) {
    if (count == 0)
        return;

    switch (mode) {
        case SkCanvas::kPolygon_PointMode:
            updateGSFromPaint(paint, NULL);
            moveTo(points[0].fX, points[0].fY);
            for (size_t i = 1; i < count; i++)
                appendLine(points[i].fX, points[i].fY);
            strokePath();
            break;
        case SkCanvas::kLines_PointMode:
            updateGSFromPaint(paint, NULL);
            for (size_t i = 0; i < count/2; i++) {
                moveTo(points[i * 2].fX, points[i * 2].fY);
                appendLine(points[i * 2 + 1].fX, points[i * 2 + 1].fY);
                strokePath();
            }
            break;
        case SkCanvas::kPoints_PointMode:
            if (paint.getStrokeCap() == SkPaint::kRound_Cap) {
                updateGSFromPaint(paint, NULL);
                for (size_t i = 0; i < count; i++) {
                    moveTo(points[i].fX, points[i].fY);
                    strokePath();
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
    if (paint.getPathEffect()) {
        // Create a path for the rectangle and apply the path effect to it.
        SkPath path;
        path.addRect(r);
        paint.getFillPath(path, &path);

        SkPaint no_effect_paint(paint);
        SkSafeUnref(no_effect_paint.setPathEffect(NULL));
        drawPath(d, path, no_effect_paint);
        return;
    }
    updateGSFromPaint(paint, NULL);

    // Skia has 0,0 at top left, pdf at bottom left.  Do the right thing.
    SkScalar bottom = r.fBottom < r.fTop ? r.fBottom : r.fTop;
    appendRectangle(r.fLeft, bottom, r.width(), r.height());
    paintPath(paint.getStyle(), SkPath::kWinding_FillType);
}

void SkPDFDevice::drawPath(const SkDraw& d, const SkPath& path,
                           const SkPaint& paint) {
    if (paint.getPathEffect()) {
        // Apply the path effect to path and draw it that way.
        SkPath no_effect_path;
        paint.getFillPath(path, &no_effect_path);

        SkPaint no_effect_paint(paint);
        SkSafeUnref(no_effect_paint.setPathEffect(NULL));
        drawPath(d, no_effect_path, no_effect_paint);
        return;
    }
    updateGSFromPaint(paint, NULL);

    SkPoint args[4];
    SkPath::Iter iter(path, false);
    for (SkPath::Verb verb = iter.next(args);
         verb != SkPath::kDone_Verb;
         verb = iter.next(args)) {
        // args gets all the points, even the implicit first point.
        switch (verb) {
            case SkPath::kMove_Verb:
                moveTo(args[0].fX, args[0].fY);
                break;
            case SkPath::kLine_Verb:
                appendLine(args[1].fX, args[1].fY);
                break;
            case SkPath::kQuad_Verb: {
                // Convert quad to cubic (degree elevation). http://goo.gl/vS4i
                const SkScalar three = SkIntToScalar(3);
                args[1].scale(SkIntToScalar(2));
                SkScalar ctl1X = SkScalarDiv(args[0].fX + args[1].fX, three);
                SkScalar ctl1Y = SkScalarDiv(args[0].fY + args[1].fY, three);
                SkScalar ctl2X = SkScalarDiv(args[2].fX + args[1].fX, three);
                SkScalar ctl2Y = SkScalarDiv(args[2].fY + args[1].fY, three);
                appendCubic(ctl1X, ctl1Y, ctl2X, ctl2Y, args[2].fX, args[2].fY);
                break;
            }
            case SkPath::kCubic_Verb:
                appendCubic(args[1].fX, args[1].fY, args[2].fX, args[2].fY,
                            args[3].fX, args[3].fY);
                break;
            case SkPath::kClose_Verb:
                closePath();
                break;
            case SkPath::kDone_Verb:
                break;
            default:
                SkASSERT(false);
                break;
        }
    }
    paintPath(paint.getStyle(), path.getFillType());
}

void SkPDFDevice::drawBitmap(const SkDraw&, const SkBitmap& bitmap,
                             const SkMatrix& matrix, const SkPaint& paint) {
    SkMatrix scaled;
    // Adjust for origin flip.
    scaled.setScale(1, -1);
    scaled.postTranslate(0, 1);
    scaled.postConcat(fCurTransform);
    // Scale the image up from 1x1 to WxH.
    scaled.postScale(bitmap.width(), bitmap.height());
    scaled.postConcat(matrix);
    internalDrawBitmap(scaled, bitmap, paint);
}

void SkPDFDevice::drawSprite(const SkDraw&, const SkBitmap& bitmap,
                             int x, int y, const SkPaint& paint) {
    SkMatrix scaled;
    // Adjust for origin flip.
    scaled.setScale(1, -1);
    scaled.postTranslate(0, 1);
    // Scale the image up from 1x1 to WxH.
    scaled.postScale(bitmap.width(), -bitmap.height());
    scaled.postTranslate(x, y);
    internalDrawBitmap(scaled, bitmap, paint);
}

void SkPDFDevice::drawText(const SkDraw&, const void* text, size_t len,
                           SkScalar x, SkScalar y, const SkPaint& paint) {
    NOT_IMPLEMENTED("drawText", true);
}

void SkPDFDevice::drawPosText(const SkDraw&, const void* text, size_t len,
                              const SkScalar pos[], SkScalar constY,
                              int scalarsPerPos, const SkPaint& paint) {
    NOT_IMPLEMENTED("drawPosText", false);
}

void SkPDFDevice::drawTextOnPath(const SkDraw&, const void* text, size_t len,
                                 const SkPath& path, const SkMatrix* matrix,
                                 const SkPaint& paint) {
    NOT_IMPLEMENTED("drawTextOnPath", true);
}

void SkPDFDevice::drawVertices(const SkDraw&, SkCanvas::VertexMode,
                               int vertexCount, const SkPoint verts[],
                               const SkPoint texs[], const SkColor colors[],
                               SkXfermode* xmode, const uint16_t indices[],
                               int indexCount, const SkPaint& paint) {
    NOT_IMPLEMENTED("drawVerticies", true);
}

void SkPDFDevice::drawDevice(const SkDraw& d, SkDevice* device, int x, int y,
                             const SkPaint& paint) {
    if ((device->getDeviceCapabilities() & kVector_Capability) == 0) {
        // If we somehow get a raster device, do what our parent would do.
        SkDevice::drawDevice(d, device, x, y, paint);
        return;
    }

    // Assume that a vector capable device means that it's a PDF Device.
    // TODO(vandebo) handle the paint (alpha and compositing mode).
    SkMatrix matrix;
    matrix.setTranslate(x, y);
    SkPDFDevice* pdfDevice = static_cast<SkPDFDevice*>(device);

    SkPDFFormXObject* xobject = new SkPDFFormXObject(pdfDevice, matrix);
    fXObjectResources.push(xobject);  // Transfer reference.
    fContent.append("/X");
    fContent.appendS32(fXObjectResources.count() - 1);
    fContent.append(" Do\n");
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
                SkRefPtr<SkPDFName> name = new SkPDFName(nameString);
                name->unref();  // SkRefPtr and new both took a reference.
                SkRefPtr<SkPDFObjRef> gsRef =
                    new SkPDFObjRef(fGraphicStateResources[i]);
                gsRef->unref();  // SkRefPtr and new both took a reference.
                extGState->insert(name.get(), gsRef.get());
            }
            fResourceDict->insert("ExtGState", extGState.get());
        }

        if (fXObjectResources.count()) {
            SkRefPtr<SkPDFDict> xObjects = new SkPDFDict();
            xObjects->unref();  // SkRefPtr and new both took a reference.
            for (int i = 0; i < fXObjectResources.count(); i++) {
                SkString nameString("X");
                nameString.appendS32(i);
                SkRefPtr<SkPDFName> name = new SkPDFName(nameString);
                name->unref();  // SkRefPtr and new both took a reference.
                SkRefPtr<SkPDFObjRef> xObjRef =
                    new SkPDFObjRef(fXObjectResources[i]);
                xObjRef->unref();  // SkRefPtr and new both took a reference.
                xObjects->insert(name.get(), xObjRef.get());
            }
            fResourceDict->insert("XObject", xObjects.get());
        }
    }
    return fResourceDict;
}

void SkPDFDevice::getResources(SkTDArray<SkPDFObject*>* resourceList) const {
    resourceList->setReserve(resourceList->count() +
                             fGraphicStateResources.count() +
                             fXObjectResources.count());
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
}

SkRefPtr<SkPDFArray> SkPDFDevice::getMediaBox() const {
    SkRefPtr<SkPDFInt> zero = new SkPDFInt(0);
    zero->unref();  // SkRefPtr and new both took a reference.
    SkRefPtr<SkPDFInt> width = new SkPDFInt(fWidth);
    width->unref();  // SkRefPtr and new both took a reference.
    SkRefPtr<SkPDFInt> height = new SkPDFInt(fHeight);
    height->unref();  // SkRefPtr and new both took a reference.
    SkRefPtr<SkPDFArray> mediaBox = new SkPDFArray();
    mediaBox->unref();  // SkRefPtr and new both took a reference.
    mediaBox->reserve(4);
    mediaBox->append(zero.get());
    mediaBox->append(zero.get());
    mediaBox->append(width.get());
    mediaBox->append(height.get());
    return mediaBox;
}

SkString SkPDFDevice::content(bool flipOrigin) const {
    SkString result;
    // Scale and translate to move the origin from the lower left to the
    // upper left.
    if (flipOrigin)
        result.printf("1 0 0 -1 0 %d cm\n", fHeight);
    result.append(fContent);
    result.append("Q");
    return result;
}

// Private

// TODO(vandebo) handle these cases.
#define PAINTCHECK(x,y) NOT_IMPLEMENTED(newPaint.x() y, false)

void SkPDFDevice::updateGSFromPaint(const SkPaint& newPaint,
                                    SkString* textStateUpdate) {
    PAINTCHECK(getXfermode, != NULL);
    PAINTCHECK(getPathEffect, != NULL);
    PAINTCHECK(getMaskFilter, != NULL);
    PAINTCHECK(getShader, != NULL);
    PAINTCHECK(getColorFilter, != NULL);
    PAINTCHECK(isFakeBoldText, == true);
    PAINTCHECK(isUnderlineText, == true);
    PAINTCHECK(isStrikeThruText, == true);
    PAINTCHECK(getTextSkewX, != 0);

    SkRefPtr<SkPDFGraphicState> newGraphicState =
        SkPDFGraphicState::getGraphicStateForPaint(newPaint);
    newGraphicState->unref();  // getGraphicState and SkRefPtr both took a ref.
    // newGraphicState has been canonicalized so we can directly compare
    // pointers.
    if (fCurrentGraphicState.get() != newGraphicState.get()) {
        int resourceIndex = fGraphicStateResources.find(newGraphicState.get());
        if (resourceIndex < 0) {
            resourceIndex = fGraphicStateResources.count();
            fGraphicStateResources.push(newGraphicState.get());
            newGraphicState->ref();
        }
        fContent.append("/G");
        fContent.appendS32(resourceIndex);
        fContent.append(" gs\n");
        fCurrentGraphicState = newGraphicState;
    }

    SkColor newColor = newPaint.getColor();
    newColor = SkColorSetA(newColor, 0xFF);
    if (fCurrentColor != newColor) {
        SkString colorString = toPDFColor(newColor);
        fContent.append(colorString);
        fContent.append("RG ");
        fContent.append(colorString);
        fContent.append("rg\n");
        fCurrentColor = newColor;
    }

    if (textStateUpdate != NULL &&
            fCurrentTextScaleX != newPaint.getTextScaleX()) {
        SkScalar scale = newPaint.getTextScaleX();
        SkScalar pdfScale = scale * 100;
        textStateUpdate->appendScalar(pdfScale);
        textStateUpdate->append(" Tz\n");
        fCurrentTextScaleX = scale;
    }
}

void SkPDFDevice::moveTo(SkScalar x, SkScalar y) {
    fContent.appendScalar(x);
    fContent.append(" ");
    fContent.appendScalar(y);
    fContent.append(" m\n");
}

void SkPDFDevice::appendLine(SkScalar x, SkScalar y) {
    fContent.appendScalar(x);
    fContent.append(" ");
    fContent.appendScalar(y);
    fContent.append(" l\n");
}

void SkPDFDevice::appendCubic(SkScalar ctl1X, SkScalar ctl1Y,
                              SkScalar ctl2X, SkScalar ctl2Y,
                              SkScalar dstX, SkScalar dstY) {
    SkString cmd("y\n");
    fContent.appendScalar(ctl1X);
    fContent.append(" ");
    fContent.appendScalar(ctl1Y);
    fContent.append(" ");
    if (ctl2X != dstX || ctl2Y != dstY) {
        cmd.set("c\n");
        fContent.appendScalar(ctl2X);
        fContent.append(" ");
        fContent.appendScalar(ctl2Y);
        fContent.append(" ");
    }
    fContent.appendScalar(dstX);
    fContent.append(" ");
    fContent.appendScalar(dstY);
    fContent.append(cmd);
}

void SkPDFDevice::appendRectangle(SkScalar x, SkScalar y,
                                  SkScalar w, SkScalar h) {
    fContent.appendScalar(x);
    fContent.append(" ");
    fContent.appendScalar(y);
    fContent.append(" ");
    fContent.appendScalar(w);
    fContent.append(" ");
    fContent.appendScalar(h);
    fContent.append(" re\n");
}

void SkPDFDevice::closePath() {
    fContent.append("h\n");
}

void SkPDFDevice::paintPath(SkPaint::Style style, SkPath::FillType fill) {
    if (style == SkPaint::kFill_Style)
        fContent.append("f");
    else if (style == SkPaint::kStrokeAndFill_Style)
        fContent.append("B");
    else if (style == SkPaint::kStroke_Style)
        fContent.append("S");

    if (style != SkPaint::kStroke_Style) {
        // Not supported yet.
        NOT_IMPLEMENTED(fill == SkPath::kInverseEvenOdd_FillType, false);
        NOT_IMPLEMENTED(fill == SkPath::kInverseWinding_FillType, false);
        if (fill == SkPath::kEvenOdd_FillType)
            fContent.append("*");
    }
    fContent.append("\n");
}

void SkPDFDevice::strokePath() {
    paintPath(SkPaint::kStroke_Style, SkPath::kWinding_FillType);
}

void SkPDFDevice::internalDrawBitmap(const SkMatrix& matrix,
                                     const SkBitmap& bitmap,
                                     const SkPaint& paint) {
    setTransform(matrix);
    SkPDFImage* image = new SkPDFImage(bitmap, paint);
    fXObjectResources.push(image);  // Transfer reference.
    fContent.append("/X");
    fContent.appendS32(fXObjectResources.count() - 1);
    fContent.append(" Do\n");
    setTransform(fCurTransform);
}

void SkPDFDevice::setTransform(const SkMatrix& m) {
    setNoTransform();
    applyTransform(m);
}

void SkPDFDevice::setNoTransform() {
    if (fActiveTransform.getType() == SkMatrix::kIdentity_Mask)
        return;
    fContent.append("Q q ");  // Restore the default transform and save it.
    fCurrentGraphicState = NULL;
    fActiveTransform.reset();
}

void SkPDFDevice::applyTempTransform(const SkMatrix& m) {
    fContent.append("q ");
    applyTransform(m);
}

void SkPDFDevice::removeTempTransform() {
    fContent.append("Q\n");
    fActiveTransform = fCurTransform;
}

void SkPDFDevice::applyTransform(const SkMatrix& m) {
    if (m == fActiveTransform)
        return;
    SkScalar transform[6];
    SkAssertResult(m.pdfTransform(transform));
    for (size_t i = 0; i < SK_ARRAY_COUNT(transform); i++) {
        fContent.appendScalar(transform[i]);
        fContent.append(" ");
    }
    fContent.append("cm\n");
    fActiveTransform = m;
}
