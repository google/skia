
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDumpCanvas.h"

#ifdef SK_DEVELOPER
#include "SkPicture.h"
#include "SkPixelRef.h"
#include "SkRRect.h"
#include "SkString.h"
#include <stdarg.h>
#include <stdio.h>

// needed just to know that these are all subclassed from SkFlattenable
#include "SkShader.h"
#include "SkPathEffect.h"
#include "SkXfermode.h"
#include "SkColorFilter.h"
#include "SkPathEffect.h"
#include "SkMaskFilter.h"

SK_DEFINE_INST_COUNT(SkDumpCanvas::Dumper)

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

static SkBitmap make_wideopen_bm() {
    static const int WIDE_OPEN = 16384;

    SkBitmap bm;
    bm.setConfig(SkBitmap::kNo_Config, WIDE_OPEN, WIDE_OPEN);
    return bm;
}

SkDumpCanvas::SkDumpCanvas(Dumper* dumper) : INHERITED(make_wideopen_bm()) {
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

int SkDumpCanvas::save(SaveFlags flags) {
    this->dump(kSave_Verb, NULL, "save(0x%X)", flags);
    return this->INHERITED::save(flags);
}

int SkDumpCanvas::saveLayer(const SkRect* bounds, const SkPaint* paint,
                             SaveFlags flags) {
    SkString str;
    str.printf("saveLayer(0x%X)", flags);
    if (bounds) {
        str.append(" bounds");
        toString(*bounds, &str);
    }
    if (paint) {
        if (paint->getAlpha() != 0xFF) {
            str.appendf(" alpha:0x%02X", paint->getAlpha());
        }
        if (paint->getXfermode()) {
            str.appendf(" xfermode:%p", paint->getXfermode());
        }
    }
    this->dump(kSave_Verb, paint, str.c_str());
    return this->INHERITED::saveLayer(bounds, paint, flags);
}

void SkDumpCanvas::restore() {
    this->INHERITED::restore();
    this->dump(kRestore_Verb, NULL, "restore");
}

bool SkDumpCanvas::translate(SkScalar dx, SkScalar dy) {
    this->dump(kMatrix_Verb, NULL, "translate(%g %g)",
               SkScalarToFloat(dx), SkScalarToFloat(dy));
    return this->INHERITED::translate(dx, dy);
}

bool SkDumpCanvas::scale(SkScalar sx, SkScalar sy) {
    this->dump(kMatrix_Verb, NULL, "scale(%g %g)",
               SkScalarToFloat(sx), SkScalarToFloat(sy));
    return this->INHERITED::scale(sx, sy);
}

bool SkDumpCanvas::rotate(SkScalar degrees) {
    this->dump(kMatrix_Verb, NULL, "rotate(%g)", SkScalarToFloat(degrees));
    return this->INHERITED::rotate(degrees);
}

bool SkDumpCanvas::skew(SkScalar sx, SkScalar sy) {
    this->dump(kMatrix_Verb, NULL, "skew(%g %g)",
               SkScalarToFloat(sx), SkScalarToFloat(sy));
    return this->INHERITED::skew(sx, sy);
}

bool SkDumpCanvas::concat(const SkMatrix& matrix) {
    SkString str;
    matrix.toString(&str);
    this->dump(kMatrix_Verb, NULL, "concat(%s)", str.c_str());
    return this->INHERITED::concat(matrix);
}

void SkDumpCanvas::setMatrix(const SkMatrix& matrix) {
    SkString str;
    matrix.toString(&str);
    this->dump(kMatrix_Verb, NULL, "setMatrix(%s)", str.c_str());
    this->INHERITED::setMatrix(matrix);
}

///////////////////////////////////////////////////////////////////////////////

static const char* bool_to_aastring(bool doAA) {
    return doAA ? "AA" : "BW";
}

bool SkDumpCanvas::clipRect(const SkRect& rect, SkRegion::Op op, bool doAA) {
    SkString str;
    toString(rect, &str);
    this->dump(kClip_Verb, NULL, "clipRect(%s %s %s)", str.c_str(), toString(op),
               bool_to_aastring(doAA));
    return this->INHERITED::clipRect(rect, op, doAA);
}

bool SkDumpCanvas::clipRRect(const SkRRect& rrect, SkRegion::Op op, bool doAA) {
    SkString str;
    toString(rrect, &str);
    this->dump(kClip_Verb, NULL, "clipRRect(%s %s %s)", str.c_str(), toString(op),
               bool_to_aastring(doAA));
    return this->INHERITED::clipRRect(rrect, op, doAA);
}

bool SkDumpCanvas::clipPath(const SkPath& path, SkRegion::Op op, bool doAA) {
    SkString str;
    toString(path, &str);
    this->dump(kClip_Verb, NULL, "clipPath(%s %s %s)", str.c_str(), toString(op),
               bool_to_aastring(doAA));
    return this->INHERITED::clipPath(path, op, doAA);
}

bool SkDumpCanvas::clipRegion(const SkRegion& deviceRgn, SkRegion::Op op) {
    SkString str;
    toString(deviceRgn, &str);
    this->dump(kClip_Verb, NULL, "clipRegion(%s %s)", str.c_str(),
               toString(op));
    return this->INHERITED::clipRegion(deviceRgn, op);
}

///////////////////////////////////////////////////////////////////////////////

void SkDumpCanvas::drawPaint(const SkPaint& paint) {
    this->dump(kDrawPaint_Verb, &paint, "drawPaint()");
}

void SkDumpCanvas::drawPoints(PointMode mode, size_t count,
                               const SkPoint pts[], const SkPaint& paint) {
    this->dump(kDrawPoints_Verb, &paint, "drawPoints(%s, %d)", toString(mode),
               count);
}

void SkDumpCanvas::drawOval(const SkRect& rect, const SkPaint& paint) {
    SkString str;
    toString(rect, &str);
    this->dump(kDrawOval_Verb, &paint, "drawOval(%s)", str.c_str());
}

void SkDumpCanvas::drawRect(const SkRect& rect, const SkPaint& paint) {
    SkString str;
    toString(rect, &str);
    this->dump(kDrawRect_Verb, &paint, "drawRect(%s)", str.c_str());
}

void SkDumpCanvas::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    SkString str;
    toString(rrect, &str);
    this->dump(kDrawRRect_Verb, &paint, "drawRRect(%s)", str.c_str());
}

void SkDumpCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    SkString str;
    toString(path, &str);
    this->dump(kDrawPath_Verb, &paint, "drawPath(%s)", str.c_str());
}

void SkDumpCanvas::drawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y,
                               const SkPaint* paint) {
    SkString str;
    bitmap.toString(&str);
    this->dump(kDrawBitmap_Verb, paint, "drawBitmap(%s %g %g)", str.c_str(),
               SkScalarToFloat(x), SkScalarToFloat(y));
}

void SkDumpCanvas::drawBitmapRectToRect(const SkBitmap& bitmap, const SkRect* src,
                                        const SkRect& dst, const SkPaint* paint,
                                        DrawBitmapRectFlags flags) {
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

    this->dump(kDrawBitmap_Verb, paint, "drawBitmapRectToRect(%s %s)",
               bs.c_str(), rs.c_str());
}

void SkDumpCanvas::drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& m,
                                     const SkPaint* paint) {
    SkString bs, ms;
    bitmap.toString(&bs);
    m.toString(&ms);
    this->dump(kDrawBitmap_Verb, paint, "drawBitmapMatrix(%s %s)",
               bs.c_str(), ms.c_str());
}

void SkDumpCanvas::drawSprite(const SkBitmap& bitmap, int x, int y,
                               const SkPaint* paint) {
    SkString str;
    bitmap.toString(&str);
    this->dump(kDrawBitmap_Verb, paint, "drawSprite(%s %d %d)", str.c_str(),
               x, y);
}

void SkDumpCanvas::drawText(const void* text, size_t byteLength, SkScalar x,
                             SkScalar y, const SkPaint& paint) {
    SkString str;
    toString(text, byteLength, paint.getTextEncoding(), &str);
    this->dump(kDrawText_Verb, &paint, "drawText(%s [%d] %g %g)", str.c_str(),
               byteLength, SkScalarToFloat(x), SkScalarToFloat(y));
}

void SkDumpCanvas::drawPosText(const void* text, size_t byteLength,
                                const SkPoint pos[], const SkPaint& paint) {
    SkString str;
    toString(text, byteLength, paint.getTextEncoding(), &str);
    this->dump(kDrawText_Verb, &paint, "drawPosText(%s [%d] %g %g ...)",
               str.c_str(), byteLength, SkScalarToFloat(pos[0].fX),
               SkScalarToFloat(pos[0].fY));
}

void SkDumpCanvas::drawPosTextH(const void* text, size_t byteLength,
                                 const SkScalar xpos[], SkScalar constY,
                                 const SkPaint& paint) {
    SkString str;
    toString(text, byteLength, paint.getTextEncoding(), &str);
    this->dump(kDrawText_Verb, &paint, "drawPosTextH(%s [%d] %g %g ...)",
               str.c_str(), byteLength, SkScalarToFloat(xpos[0]),
               SkScalarToFloat(constY));
}

void SkDumpCanvas::drawTextOnPath(const void* text, size_t byteLength,
                                   const SkPath& path, const SkMatrix* matrix,
                                   const SkPaint& paint) {
    SkString str;
    toString(text, byteLength, paint.getTextEncoding(), &str);
    this->dump(kDrawText_Verb, &paint, "drawTextOnPath(%s [%d])",
               str.c_str(), byteLength);
}

void SkDumpCanvas::drawPicture(SkPicture& picture) {
    this->dump(kDrawPicture_Verb, NULL, "drawPicture(%p) %d:%d", &picture,
               picture.width(), picture.height());
    fNestLevel += 1;
    this->INHERITED::drawPicture(picture);
    fNestLevel -= 1;
    this->dump(kDrawPicture_Verb, NULL, "endPicture(%p) %d:%d", &picture,
               picture.width(), picture.height());
}

void SkDumpCanvas::drawVertices(VertexMode vmode, int vertexCount,
                                 const SkPoint vertices[], const SkPoint texs[],
                                 const SkColor colors[], SkXfermode* xmode,
                                 const uint16_t indices[], int indexCount,
                                 const SkPaint& paint) {
    this->dump(kDrawVertices_Verb, &paint, "drawVertices(%s [%d] %g %g ...)",
               toString(vmode), vertexCount, SkScalarToFloat(vertices[0].fX),
               SkScalarToFloat(vertices[0].fY));
}

void SkDumpCanvas::drawData(const void* data, size_t length) {
//    this->dump(kDrawData_Verb, NULL, "drawData(%d)", length);
    this->dump(kDrawData_Verb, NULL, "drawData(%d) %.*s", length,
               SkTMin<size_t>(length, 64), data);
}

void SkDumpCanvas::beginCommentGroup(const char* description) {
    this->dump(kBeginCommentGroup_Verb, NULL, "beginCommentGroup(%s)", description);
}

void SkDumpCanvas::addComment(const char* kywd, const char* value) {
    this->dump(kAddComment_Verb, NULL, "addComment(%s, %s)", kywd, value);
}

void SkDumpCanvas::endCommentGroup() {
    this->dump(kEndCommentGroup_Verb, NULL, "endCommentGroup()");
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

SkDebugfDumper::SkDebugfDumper() : INHERITED(dumpToDebugf, NULL) {}

#endif
