
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDumpCanvas.h"

#ifdef SK_DEVELOPER
#include "SkPatchUtils.h"
#include "SkPicture.h"
#include "SkPixelRef.h"
#include "SkRRect.h"
#include "SkString.h"
#include "SkTextBlob.h"
#include <stdarg.h>
#include <stdio.h>

// needed just to know that these are all subclassed from SkFlattenable
#include "SkShader.h"
#include "SkPathEffect.h"
#include "SkXfermode.h"
#include "SkColorFilter.h"
#include "SkPathEffect.h"
#include "SkMaskFilter.h"

static void toString(const SkRect& r, SkString* str) {
    str->appendf("[%g,%g %g:%g]",
                 SkScalarToFloat(r.fLeft), SkScalarToFloat(r.fTop),
                 SkScalarToFloat(r.width()), SkScalarToFloat(r.height()));
}

static void toString(const SkIRect& r, SkString* str) {
    str->appendf("[%d,%d %d:%d]", r.fLeft, r.fTop, r.width(), r.height());
}

static void toString(const SkRRect& rrect, SkString* str) {
    SkRect r = rrect.getBounds();
    str->appendf("[%g,%g %g:%g]",
                 SkScalarToFloat(r.fLeft), SkScalarToFloat(r.fTop),
                 SkScalarToFloat(r.width()), SkScalarToFloat(r.height()));
    if (rrect.isOval()) {
        str->append("()");
    } else if (rrect.isSimple()) {
        const SkVector& rad = rrect.getSimpleRadii();
        str->appendf("(%g,%g)", rad.x(), rad.y());
    } else if (rrect.isComplex()) {
        SkVector radii[4] = {
            rrect.radii(SkRRect::kUpperLeft_Corner),
            rrect.radii(SkRRect::kUpperRight_Corner),
            rrect.radii(SkRRect::kLowerRight_Corner),
            rrect.radii(SkRRect::kLowerLeft_Corner),
        };
        str->appendf("(%g,%g %g,%g %g,%g %g,%g)",
                     radii[0].x(), radii[0].y(),
                     radii[1].x(), radii[1].y(),
                     radii[2].x(), radii[2].y(),
                     radii[3].x(), radii[3].y());
    }
}

static void dumpVerbs(const SkPath& path, SkString* str) {
    SkPath::Iter iter(path, false);
    SkPoint pts[4];
    for (;;) {
        switch (iter.next(pts, false)) {
            case SkPath::kMove_Verb:
                str->appendf(" M%g,%g", pts[0].fX, pts[0].fY);
                break;
            case SkPath::kLine_Verb:
                str->appendf(" L%g,%g", pts[0].fX, pts[0].fY);
                break;
            case SkPath::kQuad_Verb:
                str->appendf(" Q%g,%g,%g,%g", pts[1].fX, pts[1].fY,
                             pts[2].fX, pts[2].fY);
                break;
            case SkPath::kCubic_Verb:
                str->appendf(" C%g,%g,%g,%g,%g,%g", pts[1].fX, pts[1].fY,
                             pts[2].fX, pts[2].fY, pts[3].fX, pts[3].fY);
                break;
            case SkPath::kClose_Verb:
                str->append("X");
                break;
            case SkPath::kDone_Verb:
                return;
            case SkPath::kConic_Verb:
                SkASSERT(0);
                break;
        }
    }
}

static void toString(const SkPath& path, SkString* str) {
    if (path.isEmpty()) {
        str->append("path:empty");
    } else {
        toString(path.getBounds(), str);
#if 1
        SkString s;
        dumpVerbs(path, &s);
        str->append(s.c_str());
#endif
        str->append("]");
        str->prepend("path:[");
    }
}

static const char* toString(SkRegion::Op op) {
    static const char* gOpNames[] = {
        "DIFF", "SECT", "UNION", "XOR", "RDIFF", "REPLACE"
    };
    return gOpNames[op];
}

static void toString(const SkRegion& rgn, SkString* str) {
    str->append("Region:[");
    toString(rgn.getBounds(), str);
    str->append("]");
    if (rgn.isComplex()) {
        str->append(".complex");
    }
}

static const char* toString(SkCanvas::VertexMode vm) {
    static const char* gVMNames[] = {
        "TRIANGLES", "STRIP", "FAN"
    };
    return gVMNames[vm];
}

static const char* toString(SkCanvas::PointMode pm) {
    static const char* gPMNames[] = {
        "POINTS", "LINES", "POLYGON"
    };
    return gPMNames[pm];
}

static void toString(const void* text, size_t byteLen, SkPaint::TextEncoding enc,
                     SkString* str) {
    // FIXME: this code appears to be untested - and probably unused - and probably wrong
    switch (enc) {
        case SkPaint::kUTF8_TextEncoding:
            str->appendf("\"%.*s\"%s", (int)SkTMax<size_t>(byteLen, 32), (const char*) text,
                        byteLen > 32 ? "..." : "");
            break;
        case SkPaint::kUTF16_TextEncoding:
            str->appendf("\"%.*ls\"%s", (int)SkTMax<size_t>(byteLen, 32), (const wchar_t*) text,
                        byteLen > 64 ? "..." : "");
            break;
        case SkPaint::kUTF32_TextEncoding:
            str->appendf("\"%.*ls\"%s", (int)SkTMax<size_t>(byteLen, 32), (const wchar_t*) text,
                        byteLen > 128 ? "..." : "");
            break;
        case SkPaint::kGlyphID_TextEncoding:
            str->append("<glyphs>");
            break;

        default:
            SkASSERT(false);
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////

#define WIDE_OPEN   16384

SkDumpCanvas::SkDumpCanvas(Dumper* dumper) : INHERITED(WIDE_OPEN, WIDE_OPEN) {
    fNestLevel = 0;
    SkSafeRef(dumper);
    fDumper = dumper;
}

SkDumpCanvas::~SkDumpCanvas() {
    SkSafeUnref(fDumper);
}

void SkDumpCanvas::dump(Verb verb, const SkPaint* paint,
                        const char format[], ...) {
    static const size_t BUFFER_SIZE = 1024;

    char    buffer[BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, BUFFER_SIZE, format, args);
    va_end(args);

    if (fDumper) {
        fDumper->dump(this, verb, buffer, paint);
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkDumpCanvas::willSave() {
    this->dump(kSave_Verb, nullptr, "save()");
    this->INHERITED::willSave();
}

SkCanvas::SaveLayerStrategy SkDumpCanvas::getSaveLayerStrategy(const SaveLayerRec& rec) {
    SkString str;
    str.printf("saveLayer(0x%X)", rec.fSaveLayerFlags);
    if (rec.fBounds) {
        str.append(" bounds");
        toString(*rec.fBounds, &str);
    }
    const SkPaint* paint = rec.fPaint;
    if (paint) {
        if (paint->getAlpha() != 0xFF) {
            str.appendf(" alpha:0x%02X", paint->getAlpha());
        }
        if (paint->getXfermode()) {
            str.appendf(" xfermode:%p", paint->getXfermode());
        }
    }
    this->dump(kSave_Verb, paint, str.c_str());
    return this->INHERITED::getSaveLayerStrategy(rec);
}

void SkDumpCanvas::willRestore() {
    this->dump(kRestore_Verb, nullptr, "restore");
    this->INHERITED::willRestore();
}

void SkDumpCanvas::didConcat(const SkMatrix& matrix) {
    SkString str;

    switch (matrix.getType()) {
        case SkMatrix::kTranslate_Mask:
            this->dump(kMatrix_Verb, nullptr, "translate(%g %g)",
                       SkScalarToFloat(matrix.getTranslateX()),
                       SkScalarToFloat(matrix.getTranslateY()));
            break;
        case SkMatrix::kScale_Mask:
            this->dump(kMatrix_Verb, nullptr, "scale(%g %g)",
                       SkScalarToFloat(matrix.getScaleX()),
                       SkScalarToFloat(matrix.getScaleY()));
            break;
        default:
            matrix.toString(&str);
            this->dump(kMatrix_Verb, nullptr, "concat(%s)", str.c_str());
            break;
    }

    this->INHERITED::didConcat(matrix);
}

void SkDumpCanvas::didSetMatrix(const SkMatrix& matrix) {
    SkString str;
    matrix.toString(&str);
    this->dump(kMatrix_Verb, nullptr, "setMatrix(%s)", str.c_str());
    this->INHERITED::didSetMatrix(matrix);
}

///////////////////////////////////////////////////////////////////////////////

const char* SkDumpCanvas::EdgeStyleToAAString(ClipEdgeStyle edgeStyle) {
    return kSoft_ClipEdgeStyle == edgeStyle ? "AA" : "BW";
}

void SkDumpCanvas::onClipRect(const SkRect& rect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    SkString str;
    toString(rect, &str);
    this->dump(kClip_Verb, nullptr, "clipRect(%s %s %s)", str.c_str(), toString(op),
               EdgeStyleToAAString(edgeStyle));
    this->INHERITED::onClipRect(rect, op, edgeStyle);
}

void SkDumpCanvas::onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    SkString str;
    toString(rrect, &str);
    this->dump(kClip_Verb, nullptr, "clipRRect(%s %s %s)", str.c_str(), toString(op),
               EdgeStyleToAAString(edgeStyle));
    this->INHERITED::onClipRRect(rrect, op, edgeStyle);
}

void SkDumpCanvas::onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    SkString str;
    toString(path, &str);
    this->dump(kClip_Verb, nullptr, "clipPath(%s %s %s)", str.c_str(), toString(op),
               EdgeStyleToAAString(edgeStyle));
    this->INHERITED::onClipPath(path, op, edgeStyle);
}

void SkDumpCanvas::onClipRegion(const SkRegion& deviceRgn, SkRegion::Op op) {
    SkString str;
    toString(deviceRgn, &str);
    this->dump(kClip_Verb, nullptr, "clipRegion(%s %s)", str.c_str(),
               toString(op));
    this->INHERITED::onClipRegion(deviceRgn, op);
}

///////////////////////////////////////////////////////////////////////////////

void SkDumpCanvas::onDrawPaint(const SkPaint& paint) {
    this->dump(kDrawPaint_Verb, &paint, "drawPaint()");
}

void SkDumpCanvas::onDrawPoints(PointMode mode, size_t count,
                               const SkPoint pts[], const SkPaint& paint) {
    this->dump(kDrawPoints_Verb, &paint, "drawPoints(%s, %d)", toString(mode),
               count);
}

void SkDumpCanvas::onDrawOval(const SkRect& rect, const SkPaint& paint) {
    SkString str;
    toString(rect, &str);
    this->dump(kDrawOval_Verb, &paint, "drawOval(%s)", str.c_str());
}

void SkDumpCanvas::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    SkString str;
    toString(rect, &str);
    this->dump(kDrawRect_Verb, &paint, "drawRect(%s)", str.c_str());
}

void SkDumpCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    SkString str;
    toString(rrect, &str);
    this->dump(kDrawDRRect_Verb, &paint, "drawRRect(%s)", str.c_str());
}

void SkDumpCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                                const SkPaint& paint) {
    SkString str0, str1;
    toString(outer, &str0);
    toString(inner, &str0);
    this->dump(kDrawRRect_Verb, &paint, "drawDRRect(%s,%s)",
               str0.c_str(), str1.c_str());
}

void SkDumpCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    SkString str;
    toString(path, &str);
    this->dump(kDrawPath_Verb, &paint, "drawPath(%s)", str.c_str());
}

void SkDumpCanvas::onDrawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y,
                                const SkPaint* paint) {
    SkString str;
    bitmap.toString(&str);
    this->dump(kDrawBitmap_Verb, paint, "drawBitmap(%s %g %g)", str.c_str(),
               SkScalarToFloat(x), SkScalarToFloat(y));
}

void SkDumpCanvas::onDrawBitmapRect(const SkBitmap& bitmap, const SkRect* src, const SkRect& dst,
                                    const SkPaint* paint, SrcRectConstraint) {
    SkString bs, rs;
    bitmap.toString(&bs);
    toString(dst, &rs);
    // show the src-rect only if its not everything
    if (src && (src->fLeft > 0 || src->fTop > 0 ||
                src->fRight < SkIntToScalar(bitmap.width()) ||
                src->fBottom < SkIntToScalar(bitmap.height()))) {
        SkString ss;
        toString(*src, &ss);
        rs.prependf("%s ", ss.c_str());
    }

    this->dump(kDrawBitmap_Verb, paint, "drawBitmapRect(%s %s)", bs.c_str(), rs.c_str());
}

void SkDumpCanvas::onDrawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                    const SkRect& dst, const SkPaint* paint) {
    SkString str, centerStr, dstStr;
    bitmap.toString(&str);
    toString(center, &centerStr);
    toString(dst, &dstStr);
    this->dump(kDrawBitmap_Verb, paint, "drawBitmapNine(%s %s %s)", str.c_str(),
               centerStr.c_str(), dstStr.c_str());
}

void SkDumpCanvas::onDrawImage(const SkImage* image, SkScalar x, SkScalar y, const SkPaint* paint) {
    SkString str;
    image->toString(&str);
    this->dump(kDrawBitmap_Verb, paint, "drawImage(%s %g %g)", str.c_str(),
               SkScalarToFloat(x), SkScalarToFloat(y));
}

void SkDumpCanvas::onDrawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                                   const SkPaint* paint, SrcRectConstraint) {
    SkString bs, rs;
    image->toString(&bs);
    toString(dst, &rs);
    // show the src-rect only if its not everything
    if (src && (src->fLeft > 0 || src->fTop > 0 ||
                src->fRight < SkIntToScalar(image->width()) ||
                src->fBottom < SkIntToScalar(image->height()))) {
        SkString ss;
        toString(*src, &ss);
        rs.prependf("%s ", ss.c_str());
    }
    
    this->dump(kDrawBitmap_Verb, paint, "drawImageRectToRect(%s %s)",
               bs.c_str(), rs.c_str());
}

void SkDumpCanvas::onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                              const SkPaint& paint) {
    SkString str;
    toString(text, byteLength, paint.getTextEncoding(), &str);
    this->dump(kDrawText_Verb, &paint, "drawText(%s [%d] %g %g)", str.c_str(),
               byteLength, SkScalarToFloat(x), SkScalarToFloat(y));
}

void SkDumpCanvas::onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                                 const SkPaint& paint) {
    SkString str;
    toString(text, byteLength, paint.getTextEncoding(), &str);
    this->dump(kDrawText_Verb, &paint, "drawPosText(%s [%d] %g %g ...)",
               str.c_str(), byteLength, SkScalarToFloat(pos[0].fX),
               SkScalarToFloat(pos[0].fY));
}

void SkDumpCanvas::onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                                  SkScalar constY, const SkPaint& paint) {
    SkString str;
    toString(text, byteLength, paint.getTextEncoding(), &str);
    this->dump(kDrawText_Verb, &paint, "drawPosTextH(%s [%d] %g %g ...)",
               str.c_str(), byteLength, SkScalarToFloat(xpos[0]),
               SkScalarToFloat(constY));
}

void SkDumpCanvas::onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                                    const SkMatrix* matrix, const SkPaint& paint) {
    SkString str;
    toString(text, byteLength, paint.getTextEncoding(), &str);
    this->dump(kDrawText_Verb, &paint, "drawTextOnPath(%s [%d])",
               str.c_str(), byteLength);
}

void SkDumpCanvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                  const SkPaint& paint) {
    SkString str;
    toString(blob->bounds(), &str);
    this->dump(kDrawText_Verb, &paint, "drawTextBlob(%p) [%s]", blob, str.c_str());
    // FIXME: dump the actual blob content?
}

void SkDumpCanvas::onDrawPicture(const SkPicture* picture, const SkMatrix* matrix,
                                 const SkPaint* paint) {
    this->dump(kDrawPicture_Verb, nullptr, "drawPicture(%p) %f:%f:%f:%f", picture,
               picture->cullRect().fLeft, picture->cullRect().fTop,
               picture->cullRect().fRight, picture->cullRect().fBottom);
    fNestLevel += 1;
    this->INHERITED::onDrawPicture(picture, matrix, paint);
    fNestLevel -= 1;
    this->dump(kDrawPicture_Verb, nullptr, "endPicture(%p) %f:%f:%f:%f", &picture,
               picture->cullRect().fLeft, picture->cullRect().fTop,
               picture->cullRect().fRight, picture->cullRect().fBottom);
}

void SkDumpCanvas::onDrawVertices(VertexMode vmode, int vertexCount,
                                  const SkPoint vertices[], const SkPoint texs[],
                                  const SkColor colors[], SkXfermode* xmode,
                                  const uint16_t indices[], int indexCount,
                                  const SkPaint& paint) {
    this->dump(kDrawVertices_Verb, &paint, "drawVertices(%s [%d] %g %g ...)",
               toString(vmode), vertexCount, SkScalarToFloat(vertices[0].fX),
               SkScalarToFloat(vertices[0].fY));
}

void SkDumpCanvas::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                               const SkPoint texCoords[4], SkXfermode* xmode,
                               const SkPaint& paint) {
    //dumps corner points and colors in clockwise order starting on upper-left corner
    this->dump(kDrawPatch_Verb, &paint, "drawPatch(Vertices{[%f, %f], [%f, %f], [%f, %f], [%f, %f]}\
              | Colors{[0x%x], [0x%x], [0x%x], [0x%x]} | TexCoords{[%f,%f], [%f,%f], [%f,%f], \
               [%f,%f]})",
              cubics[SkPatchUtils::kTopP0_CubicCtrlPts].fX,
              cubics[SkPatchUtils::kTopP0_CubicCtrlPts].fY,
              cubics[SkPatchUtils::kTopP3_CubicCtrlPts].fX,
              cubics[SkPatchUtils::kTopP3_CubicCtrlPts].fY,
              cubics[SkPatchUtils::kBottomP3_CubicCtrlPts].fX,
              cubics[SkPatchUtils::kBottomP3_CubicCtrlPts].fY,
              cubics[SkPatchUtils::kBottomP0_CubicCtrlPts].fX,
              cubics[SkPatchUtils::kBottomP0_CubicCtrlPts].fY,
              colors[0], colors[1], colors[2], colors[3],
              texCoords[0].x(), texCoords[0].y(), texCoords[1].x(), texCoords[1].y(),
              texCoords[2].x(), texCoords[2].y(), texCoords[3].x(), texCoords[3].y());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SkFormatDumper::SkFormatDumper(void (*proc)(const char*, void*), void* refcon) {
    fProc = proc;
    fRefcon = refcon;
}

static void appendPtr(SkString* str, const void* ptr, const char name[]) {
    if (ptr) {
        str->appendf(" %s:%p", name, ptr);
    }
}

static void appendFlattenable(SkString* str, const SkFlattenable* ptr,
                              const char name[]) {
    if (ptr) {
        str->appendf(" %s:%p", name, ptr);
    }
}

void SkFormatDumper::dump(SkDumpCanvas* canvas, SkDumpCanvas::Verb verb,
                          const char str[], const SkPaint* p) {
    SkString msg, tab;
    const int level = canvas->getNestLevel() + canvas->getSaveCount() - 1;
    SkASSERT(level >= 0);
    for (int i = 0; i < level; i++) {
#if 0
        tab.append("\t");
#else
        tab.append("    ");   // tabs are often too wide to be useful
#endif
    }
    msg.printf("%s%s", tab.c_str(), str);

    if (p) {
        msg.appendf(" color:0x%08X flags:%X", p->getColor(), p->getFlags());
        appendFlattenable(&msg, p->getShader(), "shader");
        appendFlattenable(&msg, p->getXfermode(), "xfermode");
        appendFlattenable(&msg, p->getPathEffect(), "pathEffect");
        appendFlattenable(&msg, p->getMaskFilter(), "maskFilter");
        appendFlattenable(&msg, p->getPathEffect(), "pathEffect");
        appendFlattenable(&msg, p->getColorFilter(), "filter");

        if (SkDumpCanvas::kDrawText_Verb == verb) {
            msg.appendf(" textSize:%g", SkScalarToFloat(p->getTextSize()));
            appendPtr(&msg, p->getTypeface(), "typeface");
        }

        if (p->getStyle() != SkPaint::kFill_Style) {
            msg.appendf(" strokeWidth:%g", SkScalarToFloat(p->getStrokeWidth()));
        }
    }

    fProc(msg.c_str(), fRefcon);
}

///////////////////////////////////////////////////////////////////////////////

static void dumpToDebugf(const char text[], void*) {
    SkDebugf("%s\n", text);
}

SkDebugfDumper::SkDebugfDumper() : INHERITED(dumpToDebugf, nullptr) {}

#endif
