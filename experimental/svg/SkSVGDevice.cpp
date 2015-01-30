/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSVGDevice.h"

#include "SkBitmap.h"
#include "SkDraw.h"
#include "SkPaint.h"
#include "SkParsePath.h"
#include "SkStream.h"
#include "SkXMLWriter.h"

namespace {

class AutoElement {
public:
    AutoElement(const char name[], SkXMLWriter* writer)
        : fWriter(writer) {
        fWriter->startElement(name);
    }

    ~AutoElement() {
        fWriter->endElement();
    }

private:
    SkXMLWriter* fWriter;
};

}

SkBaseDevice* SkSVGDevice::Create(const SkISize& size, SkWStream* wstream) {
    if (!SkToBool(wstream)) {
        return NULL;
    }

    return SkNEW_ARGS(SkSVGDevice, (size, wstream));
}

SkSVGDevice::SkSVGDevice(const SkISize& size, SkWStream* wstream)
    : fWriter(SkNEW_ARGS(SkXMLStreamWriter, (wstream))) {

    fLegacyBitmap.setInfo(SkImageInfo::MakeUnknown(size.width(), size.height()));

    fWriter->writeHeader();
    fWriter->startElement("svg");
    fWriter->addAttribute("xmlns", "http://www.w3.org/2000/svg");
    fWriter->addAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
    fWriter->addS32Attribute("width", size.width());
    fWriter->addS32Attribute("height", size.height());
}

SkSVGDevice::~SkSVGDevice() {
    fWriter->endElement();
    fWriter->flush();
    SkDELETE(fWriter);
}

SkImageInfo SkSVGDevice::imageInfo() const {
    return fLegacyBitmap.info();
}

const SkBitmap& SkSVGDevice::onAccessBitmap() {
    return fLegacyBitmap;
}

void SkSVGDevice::addPaint(const SkPaint& paint) {
    SkColor color = paint.getColor();
    SkString colorStr;
    colorStr.appendf("rgb(%u,%u,%u)", SkColorGetR(color), SkColorGetG(color), SkColorGetB(color));

    SkPaint::Style style = paint.getStyle();
    if (style == SkPaint::kFill_Style || style == SkPaint::kStrokeAndFill_Style) {
        fWriter->addAttribute("fill", colorStr.c_str());
    } else {
        fWriter->addAttribute("fill", "none");
    }

    if (style == SkPaint::kStroke_Style || style == SkPaint::kStrokeAndFill_Style) {
        fWriter->addAttribute("stroke", colorStr.c_str());
        fWriter->addScalarAttribute("stroke-width", paint.getStrokeWidth());
    } else {
        fWriter->addAttribute("stroke", "none");
    }
}

void SkSVGDevice::addTransform(const SkMatrix &t) {
    if (t.isIdentity()) {
        return;
    }

    SkString tstr;
    tstr.appendf("matrix(%g %g %g %g %g %g)",
                 SkScalarToFloat(t.getScaleX()), SkScalarToFloat(t.getSkewY()),
                 SkScalarToFloat(t.getSkewX()), SkScalarToFloat(t.getScaleY()),
                 SkScalarToFloat(t.getTranslateX()), SkScalarToFloat(t.getTranslateY()));
    fWriter->addAttribute("transform", tstr.c_str());
}

void SkSVGDevice::drawPaint(const SkDraw&, const SkPaint& paint) {
    // todo
}

void SkSVGDevice::drawPoints(const SkDraw&, SkCanvas::PointMode mode, size_t count,
                             const SkPoint[], const SkPaint& paint) {
    // todo
}

void SkSVGDevice::drawRect(const SkDraw& draw, const SkRect& r, const SkPaint& paint) {
    AutoElement elem("rect", fWriter);

    fWriter->addScalarAttribute("x", r.fLeft);
    fWriter->addScalarAttribute("y", r.fTop);
    fWriter->addScalarAttribute("width", r.width());
    fWriter->addScalarAttribute("height", r.height());

    this->addPaint(paint);
    this->addTransform(*draw.fMatrix);
}

void SkSVGDevice::drawOval(const SkDraw&, const SkRect& oval, const SkPaint& paint) {
    // todo
}

void SkSVGDevice::drawRRect(const SkDraw&, const SkRRect& rr, const SkPaint& paint) {
    // todo
}

void SkSVGDevice::drawPath(const SkDraw& draw, const SkPath& path, const SkPaint& paint,
                           const SkMatrix* prePathMatrix, bool pathIsMutable) {
    AutoElement elem("path", fWriter);

    SkString pathStr;
    SkParsePath::ToSVGString(path, &pathStr);
    fWriter->addAttribute("d", pathStr.c_str());

    this->addPaint(paint);
    this->addTransform(*draw.fMatrix);
}

void SkSVGDevice::drawBitmap(const SkDraw&, const SkBitmap& bitmap,
                             const SkMatrix& matrix, const SkPaint& paint) {
    // todo
}

void SkSVGDevice::drawSprite(const SkDraw&, const SkBitmap& bitmap,
                             int x, int y, const SkPaint& paint) {
    // todo
}

void SkSVGDevice::drawBitmapRect(const SkDraw&, const SkBitmap&, const SkRect* srcOrNull,
                                 const SkRect& dst, const SkPaint& paint,
                                 SkCanvas::DrawBitmapRectFlags flags) {
    // todo
}

void SkSVGDevice::drawText(const SkDraw&, const void* text, size_t len,
                           SkScalar x, SkScalar y, const SkPaint& paint) {
    // todo
}

void SkSVGDevice::drawPosText(const SkDraw&, const void* text, size_t len,const SkScalar pos[],
                              int scalarsPerPos, const SkPoint& offset, const SkPaint& paint) {
    // todo
}

void SkSVGDevice::drawTextOnPath(const SkDraw&, const void* text, size_t len, const SkPath& path,
                                 const SkMatrix* matrix, const SkPaint& paint) {
    // todo
}

void SkSVGDevice::drawVertices(const SkDraw&, SkCanvas::VertexMode, int vertexCount,
                               const SkPoint verts[], const SkPoint texs[],
                               const SkColor colors[], SkXfermode* xmode,
                               const uint16_t indices[], int indexCount,
                               const SkPaint& paint) {
    // todo
}

void SkSVGDevice::drawDevice(const SkDraw&, SkBaseDevice*, int x, int y,
                             const SkPaint&) {
    // todo
}
