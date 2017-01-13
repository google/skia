/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDocument.h"

#ifndef SK_BUILD_FOR_WIN

/*
  Note:  Very little of this file *needs* _WIN32 (That's all in SkXPS.cpp), but no
  clients would want to compile this withot that.
*/

sk_sp<SkDocument> SkDocument::MakeXPS(SkWStream*, SkScalar) { return nullptr; }
sk_sp<SkDocument> SkDocument::MakeXPS(const char[], SkScalar) { return nullptr; }

#else

#include <ObjBase.h>
#include <XpsObjectModel.h>

#include "SkAutoCoInitialize.h"
#include "SkClipStack.h"
#include "SkColorFilter.h"
#include "SkDrawable.h"
#include "SkImage.h"
#include "SkLatticeIter.h"
#include "SkMakeUnique.h"
#include "SkMatrix.h"
#include "SkNoDrawCanvas.h"
#include "SkPatchUtils.h"
#include "SkPath.h"
#include "SkPathMeasure.h"
#include "SkPathPriv.h"
#include "SkRSXform.h"
#include "SkTArray.h"
#include "SkTScopedComPtr.h"
#include "SkTextBlob.h"
#include "SkTextBlobRunIterator.h"
#include "SkTextToPathIter.h"
#include "SkUtils.h"
#include "SkXPS.h"

namespace {

class XPSCanvas final : public SkNoDrawCanvas {
public:
    XPSCanvas(SkXPS* xps, SkISize size)
        : SkNoDrawCanvas(size.width(), size.height()), fXPS(xps) {}
    ~XPSCanvas() {}

protected:
    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override;
    void onDrawDrawable(SkDrawable*, const SkMatrix*) override;
    void onDrawText(const void*, size_t, SkScalar, SkScalar, const SkPaint&) override;
    void onDrawPosText(const void*, size_t, const SkPoint[], const SkPaint&) override;
    void onDrawPosTextH(const void*, size_t, const SkScalar[], SkScalar, const SkPaint&) override;
    void onDrawTextOnPath(const void*, size_t, const SkPath&, const SkMatrix*,
                          const SkPaint&) override;
    void onDrawTextRSXform(const void*, size_t, const SkRSXform[], const SkRect*,
                           const SkPaint&) override;
    void onDrawTextBlob(const SkTextBlob*, SkScalar, SkScalar, const SkPaint&) override;
    void onDrawPatch(const SkPoint[12], const SkColor[4], const SkPoint[4], SkBlendMode,
                     const SkPaint&) override;
    void onDrawPaint(const SkPaint&) override;
    void onDrawPoints(PointMode, size_t, const SkPoint[], const SkPaint&) override;
    void onDrawRect(const SkRect&, const SkPaint&) override;
    void onDrawRegion(const SkRegion&, const SkPaint&) override;
    void onDrawOval(const SkRect&, const SkPaint&) override;
    void onDrawArc(const SkRect&, SkScalar, SkScalar, bool, const SkPaint&) override;
    void onDrawRRect(const SkRRect&, const SkPaint&) override;
    void onDrawPath(const SkPath&, const SkPaint&) override;
    void onDrawBitmap(const SkBitmap&, SkScalar, SkScalar, const SkPaint*) override;
    void onDrawBitmapRect(const SkBitmap&, const SkRect*, const SkRect&, const SkPaint*,
                          SrcRectConstraint) override;
    void onDrawImage(const SkImage*, SkScalar, SkScalar, const SkPaint*) override;
    void onDrawImageRect(const SkImage*, const SkRect*, const SkRect&, const SkPaint*,
                         SrcRectConstraint) override;
    void onDrawImageNine(const SkImage*, const SkIRect&, const SkRect&, const SkPaint*) override;
    void onDrawBitmapNine(const SkBitmap&, const SkIRect&, const SkRect&, const SkPaint*) override;
    void onDrawImageLattice(const SkImage*, const Lattice&, const SkRect&, const SkPaint*) override;
    void onDrawBitmapLattice(const SkBitmap&, const Lattice&, const SkRect&,
                             const SkPaint*) override;
    void onDrawVertices(VertexMode, int, const SkPoint[], const SkPoint[], const SkColor[],
                        SkBlendMode, const uint16_t[], int, const SkPaint&) override;
    void onDrawAtlas(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[],
                     int, SkBlendMode, const SkRect*, const SkPaint*) override;
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec&) override;
    void willRestore() override;

private:
    void internalSaveLayer(const SkCanvas::SaveLayerRec& rec);
    void internalRestoreLayer();
    void internalDrawText(const void* text, size_t textBytes,
                          SkTextBlob::GlyphPositioning,
                          SkPoint, const SkScalar*, const SkPaint&);
    void updateState();

    SkXPS* fXPS;
    SkTArray<int> fLayerCounts;
    SkMatrix fMatrix = SkMatrix::I();
    SkPath fClip;
    int32_t fClipGenID = SkClipStack::kInvalidGenID;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

// The following helper functions implement complex canvas commands in terms of
// simpler commands.  They are ripe for plucking into a collection used by
// multiple backends.

static void draw_drrect(SkCanvas* canvas, const SkRRect& outer, const SkRRect& inner,
                        const SkPaint& paint) {
    SkPath path;
    path.addRRect(outer);
    path.addRRect(inner);
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.setIsVolatile(true);
    canvas->drawPath(path, paint);
}

static void morphpoints(SkPoint dst[], const SkPoint src[], int count,
                        SkPathMeasure& meas, const SkMatrix& matrix) {
    SkMatrix::MapXYProc proc = matrix.getMapXYProc();

    for (int i = 0; i < count; i++) {
        SkPoint pos;
        SkVector tangent;

        proc(matrix, src[i].fX, src[i].fY, &pos);
        SkScalar sx = pos.fX;
        SkScalar sy = pos.fY;

        if (!meas.getPosTan(sx, &pos, &tangent)) {
            // set to 0 if the measure failed, so that we just set dst == pos
            tangent.set(0, 0);
        }

        /*  This is the old way (that explains our approach but is way too slow
         SkMatrix    matrix;
         SkPoint     pt;

         pt.set(sx, sy);
         matrix.setSinCos(tangent.fY, tangent.fX);
         matrix.preTranslate(-sx, 0);
         matrix.postTranslate(pos.fX, pos.fY);
         matrix.mapPoints(&dst[i], &pt, 1);
         */
        dst[i].set(pos.fX - SkScalarMul(tangent.fY, sy),
                   pos.fY + SkScalarMul(tangent.fX, sy));
    }
}

static void morphpath(SkPath* dst, const SkPath& src, SkPathMeasure& meas,
                      const SkMatrix& matrix) {
    SkPath::Iter    iter(src, false);
    SkPoint         srcP[4], dstP[3];
    SkPath::Verb    verb;

    while ((verb = iter.next(srcP)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                morphpoints(dstP, srcP, 1, meas, matrix);
                dst->moveTo(dstP[0]);
                break;
            case SkPath::kLine_Verb:
                // turn lines into quads to look bendy
                srcP[0].fX = SkScalarAve(srcP[0].fX, srcP[1].fX);
                srcP[0].fY = SkScalarAve(srcP[0].fY, srcP[1].fY);
                morphpoints(dstP, srcP, 2, meas, matrix);
                dst->quadTo(dstP[0], dstP[1]);
                break;
            case SkPath::kQuad_Verb:
                morphpoints(dstP, &srcP[1], 2, meas, matrix);
                dst->quadTo(dstP[0], dstP[1]);
                break;
            case SkPath::kCubic_Verb:
                morphpoints(dstP, &srcP[1], 3, meas, matrix);
                dst->cubicTo(dstP[0], dstP[1], dstP[2]);
                break;
            case SkPath::kClose_Verb:
                dst->close();
                break;
            default:
                SkDEBUGFAIL("unknown verb");
                break;
        }
    }
}

static void draw_text_on_path(SkCanvas* canvas, const void* text, size_t textBytes,
                              const SkPath& path, const SkMatrix* matrix, const SkPaint& paint) {
    if (!text || !textBytes) {
        return;
    }
    SkTextToPathIter iter((const char*)text, textBytes, paint, true);
    SkPathMeasure meas(path, false);
    SkScalar hOffset = 0;
    if (paint.getTextAlign() != SkPaint::kLeft_Align) {
        SkScalar pathLen = meas.getLength();
        if (paint.getTextAlign() == SkPaint::kCenter_Align) {
            pathLen = SkScalarHalf(pathLen);
        }
        hOffset += pathLen;
    }
    const SkPath* iterPath;
    SkScalar      xpos;
    SkMatrix      scaledMatrix;
    SkScalar      scale = iter.getPathScale();
    scaledMatrix.setScale(scale, scale);
    while (iter.next(&iterPath, &xpos)) {
        if (iterPath) {
            SkPath      tmp;
            SkMatrix    m(scaledMatrix);
            tmp.setIsVolatile(true);
            m.postTranslate(xpos + hOffset, 0);
            if (matrix) {
                m.postConcat(*matrix);
            }
            morphpath(&tmp, *iterPath, meas, m);
            canvas->drawPath(tmp, iter.getPaint());
        }
    }
}

static int count_text(SkPaint::TextEncoding enc, const char* text) {
    switch (enc) {
        case SkPaint::kUTF8_TextEncoding:
            return SkUTF8_CountUTF8Bytes(text);
        case SkPaint::kUTF16_TextEncoding: {
            const uint16_t* next = (uint16_t*)text;
            (void)SkUTF16_NextUnichar(&next);
            return SkToInt((const char*)next - text);
        }
        case SkPaint::kUTF32_TextEncoding:
            return 4;
        case SkPaint::kGlyphID_TextEncoding:
            return 2;
        default:
            SkASSERT(false);
            return 1;
    }
}

static void draw_text_rsxform(SkCanvas* canvas, const void* textPtr, size_t textBytes,
                              const SkRSXform xform[], const SkRect* cullRect,
                              const SkPaint& paint) {
    if (cullRect && canvas->quickReject(*cullRect)) {
        return;
    }
    const char* text = reinterpret_cast<const char*>(textPtr);
    SkPaint::TextEncoding encoding = paint.getTextEncoding();
    SkMatrix localM;
    const char* stopText = text + textBytes;
    while (text < stopText) {
        localM.setRSXform(*xform++);
        SkAutoCanvasRestore autoSaveCanvas(canvas, true);
        canvas->concat(localM);
        int subLen = count_text(encoding, text);
        canvas->drawText(text, subLen, 0, 0, paint);
        text += subLen;
    }
}

static void draw_text_blob(SkCanvas* canvas, const SkTextBlob* blob,
                           SkScalar x, SkScalar y, const SkPaint& p) {
    SkPaint runPaint(p);
    for (SkTextBlobRunIterator it(blob); !it.done(); it.next()) {
        size_t textLen = it.glyphCount() * sizeof(uint16_t);
        SkPoint xy = it.offset() + SkPoint{x, y};
        it.applyFontToPaint(&runPaint);
        switch (it.positioning()) {
            case SkTextBlob::kDefault_Positioning:
                canvas->drawText(it.glyphs(), textLen, xy.x(), xy.y(), runPaint);
                break;
            case SkTextBlob::kHorizontal_Positioning:
                canvas->drawPosTextH(it.glyphs(), textLen, it.pos(), xy.y(), runPaint);
                break;
            case SkTextBlob::kFull_Positioning:
                canvas->drawPosText(it.glyphs(), textLen, (const SkPoint*)it.pos(), runPaint);
                break;
            default:
                SkASSERT(false);
                return;
        }
    }
}

static void draw_patch(SkCanvas* canvas, const SkPoint cubics[12], const SkColor colors[4],
                       const SkPoint texCoords[4], SkBlendMode bmode, const SkPaint& paint) {
    SkPatchUtils::VertexData data;
    SkISize lod = SkPatchUtils::GetLevelOfDetail(cubics, &SkMatrix::I());
    if (SkPatchUtils::getVertexData(&data, cubics, colors, texCoords, lod.width(), lod.height())) {
        canvas->drawVertices(SkCanvas::kTriangles_VertexMode, data.fVertexCount,
                             data.fPoints, data.fTexCoords, data.fColors, bmode,
                             data.fIndices, data.fIndexCount, paint);
    }
}

static void draw_bitmap_rect(SkCanvas* canvas, const SkBitmap& bitmap,
                             const SkRect* src, const SkRect& dst,
                             const SkPaint* paint, SkCanvas::SrcRectConstraint) {
    SkRect bitmapBounds = SkRect::Make(bitmap.bounds());
    if (src && !bitmapBounds.intersect(*src)) {
        return;
    }
    SkBitmap subset;
    SkIRect isrc = bitmapBounds.roundOut();
    if (!bitmap.extractSubset(&subset, isrc)) {
       return;
    }
    SkAutoCanvasRestore autoCanvasRestore(canvas, true);
    if (bitmapBounds != SkRect::Make(isrc)) {
        canvas->clipRect(bitmapBounds);
    }
    SkMatrix matrix = SkMatrix::MakeRectToRect(bitmapBounds, dst, SkMatrix::kFill_ScaleToFit);
    canvas->concat(matrix);
    canvas->drawBitmap(subset, isrc.x(), isrc.y(), paint);
}

static SkBitmap to_bitmap(const SkImage* img) {
    SkBitmap result;
    if (!img) {
        return result;
    }
    if (img->asLegacyBitmap(&result, SkImage::kRO_LegacyBitmapMode)) {
        return result;
    }
    SkISize size = img->dimensions();
    result.allocPixels(SkImageInfo::MakeN32(size.width(), size.height(), img->alphaType()));
    result.lockPixels();
    SkPixmap pixmap;
    SkAssertResult(result.peekPixels(&pixmap));
    if (!img->readPixels(pixmap, 0, 0)) {
        result.reset();
    }
    return result;
}

static void draw_bitmap_nine(SkCanvas* canvas, const SkBitmap& bitmap,
                             const SkIRect& center, const SkRect& dst, const SkPaint* p) {
    SkLatticeIter iter(bitmap.width(), bitmap.height(), center, dst);
    SkRect srcR, dstR;
    while (iter.next(&srcR, &dstR)) {
        canvas->drawBitmapRect(bitmap, srcR, dstR, p);
    }
}

static void draw_bitmap_lattice(SkCanvas* canvas, const SkBitmap& bitmap,
                                const SkCanvas::Lattice& lattice,
                                const SkRect& dst, const SkPaint* paint) {
    SkLatticeIter iter(lattice, dst);
    SkRect srcR, dstR;
    while (iter.next(&srcR, &dstR)) {
        canvas->drawBitmapRect(bitmap, srcR, dstR, paint);
    }
}

static void draw_atlas(SkCanvas* canvas, const SkImage* atlas, const SkRSXform xform[],
                       const SkRect tex[], const SkColor colors[], int count, SkBlendMode mode,
                       const SkRect* cull, const SkPaint* paintPtr) {
    if (!count || !atlas || !tex || !xform) {
        return;
    }
    SkPaint paint = paintPtr ? *paintPtr : SkPaint();
    SkPath path;
    path.setIsVolatile(true);
    for (int i = 0; i < count; ++i) {
        SkPoint quad[4];
        xform[i].toQuad(tex[i].width(), tex[i].height(), quad);

        SkMatrix localM;
        localM.setRSXform(xform[i]);
        localM.preTranslate(-tex[i].left(), -tex[i].top());

        SkPaint pnt(paint);
        pnt.setShader(atlas->makeShader(SkShader::kClamp_TileMode,
                                        SkShader::kClamp_TileMode,
                                        &localM));
        if (!pnt.getShader()) {
            break;
        }
        if (colors) {
            pnt.setColorFilter(SkColorFilter::MakeModeFilter(colors[i], (SkBlendMode)mode));
        }
        path.rewind();
        path.addPoly(quad, 4, true);
        path.setConvexity(SkPath::kConvex_Convexity);
        canvas->drawPath(path, pnt);
    }
}

static void draw_paint(SkCanvas* canvas, const SkPaint& p) {
    SkRect bounds;
    if (canvas->getClipBounds(&bounds)) {
        SkPaint paint(p);
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawRect(bounds, paint);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SkCanvas::SaveLayerStrategy XPSCanvas::getSaveLayerStrategy(const SkCanvas::SaveLayerRec& rec) {
    (void)this->SkNoDrawCanvas::getSaveLayerStrategy(rec);
    fLayerCounts.push_back(this->getSaveCount());
    this->internalSaveLayer(rec);
    return kNoLayer_SaveLayerStrategy;
}

void XPSCanvas::willRestore() {
    // FIXME: check to see if off-by-one.
    if (fLayerCounts.count() > 0 &&  this->getSaveCount() == fLayerCounts.back()) {
        fLayerCounts.pop_back();
        this->internalRestoreLayer();
    }
}

void XPSCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    draw_drrect(this, outer, inner, paint);
}

void XPSCanvas::onDrawDrawable(SkDrawable* dr, const SkMatrix* matrix) {
    dr->draw(this, matrix);
}

void XPSCanvas::onDrawTextOnPath(const void* text, size_t textBytes, const SkPath& path,
                                   const SkMatrix* matrix, const SkPaint& paint) {
    draw_text_on_path(this, text, textBytes, path, matrix, paint);
}

void XPSCanvas::onDrawTextRSXform(const void* text, size_t textBytes,
                                    const SkRSXform xform[], const SkRect* cullRect,
                                    const SkPaint& paint) {
    draw_text_rsxform(this, text, textBytes, xform, cullRect, paint);
}

void XPSCanvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y, const SkPaint& p) {
    draw_text_blob(this, blob, x, y, p);
}

void XPSCanvas::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                              const SkPoint texCoords[4], SkBlendMode bmode,
                              const SkPaint& paint) {
    draw_patch(this, cubics, colors, texCoords, bmode, paint);
}

void XPSCanvas::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    SkPath path;
    path.addRect(rect);
    this->drawPath(path, paint);
}

void XPSCanvas::onDrawRegion(const SkRegion& region, const SkPaint& paint) {
    SkPath path;
    region.getBoundaryPath(&path);
    this->drawPath(path, paint);
}

void XPSCanvas::onDrawOval(const SkRect& oval, const SkPaint& paint) {
    SkPath path;
    path.addOval(oval);
    this->drawPath(path, paint);
}

void XPSCanvas::onDrawArc(const SkRect& oval, SkScalar start, SkScalar sweep, bool useCenter,
                            const SkPaint& paint) {
    SkPath path;
    bool isFillNoPathEffect = SkPaint::kFill_Style == paint.getStyle() && !paint.getPathEffect();
    SkPathPriv::CreateDrawArcPath(&path, oval, start, sweep, useCenter, isFillNoPathEffect);
    this->drawPath(path, paint);
}

void XPSCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    SkPath path;
    path.addRRect(rrect);
    this->drawPath(path, paint);
}

void XPSCanvas::onDrawBitmapRect(const SkBitmap& bitmap, const SkRect* src, const SkRect& dst,
                                   const SkPaint* paint, SrcRectConstraint constraint) {
    draw_bitmap_rect(this, bitmap, src, dst, paint, constraint);
}

void XPSCanvas::onDrawImage(const SkImage* img, SkScalar x, SkScalar y, const SkPaint* p) {
    SkBitmap bitmap = to_bitmap(img);
    if (!bitmap.drawsNothing()) {
        this->drawBitmap(bitmap, x, y, p);
    }
}

void XPSCanvas::onDrawImageRect(const SkImage* img, const SkRect* s, const SkRect& d,
                                  const SkPaint* p, SrcRectConstraint) {
    SkASSERT(s);
    SkBitmap bitmap = to_bitmap(img);
    if (!bitmap.drawsNothing()) {
        this->drawBitmapRect(bitmap, s ? *s : SkRect::Make(img->bounds()), d, p);
    }
}

void XPSCanvas::onDrawImageNine(const SkImage* img, const SkIRect& center, const SkRect& dst,
                                  const SkPaint* paint) {
    SkBitmap bitmap = to_bitmap(img);
    if (!bitmap.drawsNothing()) {
        this->drawBitmapNine(bitmap, center, dst, paint);
    }
}

void XPSCanvas::onDrawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                   const SkRect& dst, const SkPaint* p) {
    draw_bitmap_nine(this, bitmap, center, dst, p);
}

void XPSCanvas::onDrawImageLattice(const SkImage* img, const SkCanvas::Lattice& lattice,
                                     const SkRect& dst, const SkPaint* paint) {
    SkBitmap bitmap = to_bitmap(img);
    if (!bitmap.drawsNothing()) {
        this->drawBitmapLattice(bitmap, lattice, dst, paint);
    }
}

void XPSCanvas::onDrawBitmapLattice(const SkBitmap& bitmap, const SkCanvas::Lattice& lattice,
                                      const SkRect& dst, const SkPaint* paint) {
    draw_bitmap_lattice(this, bitmap, lattice, dst, paint);
}

void XPSCanvas::onDrawAtlas(const SkImage* atlas, const SkRSXform xform[], const SkRect tex[],
                              const SkColor colors[], int count, SkBlendMode mode,
                              const SkRect* cull, const SkPaint* paint) {
    draw_atlas(this, atlas, xform, tex, colors, count, mode, cull, paint);
}

void XPSCanvas::onDrawText(const void* text, size_t textBytes,
                             SkScalar x, SkScalar y, const SkPaint& paint) {
    this->internalDrawText(text, textBytes, SkTextBlob::kDefault_Positioning,
                           SkPoint{x, y}, nullptr, paint);
}

void XPSCanvas::onDrawPosText(const void* text, size_t textBytes,
                                const SkPoint pos[], const SkPaint& paint) {
    this->internalDrawText(text, textBytes, SkTextBlob::kFull_Positioning,
                           SkPoint{0, 0}, reinterpret_cast<const SkScalar*>(pos), paint);
}

void XPSCanvas::onDrawPosTextH(const void* text, size_t textBytes,
                                 const SkScalar posx[], SkScalar y, const SkPaint& paint) {
    this->internalDrawText(text, textBytes, SkTextBlob::kHorizontal_Positioning,
                           SkPoint{0, y}, posx, paint);
}

void XPSCanvas::onDrawPaint(const SkPaint& paint) {
    draw_paint(this, paint);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void XPSCanvas::updateState() {
    fMatrix = this->getTotalMatrix();
    const SkClipStack* clipStack = this->getClipStack();
    SkASSERT(clipStack);
    if (clipStack->getTopmostGenID() != fClipGenID) {
        fClip.rewind();
        (void)clipStack->asPath(&fClip);
        fClipGenID = clipStack->getTopmostGenID();
    }
}

void XPSCanvas::internalDrawText(const void* text, size_t textBytes,
                                   SkTextBlob::GlyphPositioning positioning,
                                   SkPoint origin, const SkScalar* pos, const SkPaint& paint) {
    this->updateState();
    fXPS->drawText(fMatrix, fClip, text, textBytes, positioning, origin, pos, paint);
}

void XPSCanvas::onDrawPoints(PointMode mode, size_t count, const SkPoint pts[],
                               const SkPaint& paint) {
    this->updateState();
    fXPS->drawPoints(fMatrix, fClip, mode, count, pts, paint);
}

void XPSCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    this->updateState();
    fXPS->drawPath(fMatrix, fClip, path, paint);
}

void XPSCanvas::onDrawBitmap(const SkBitmap& bm, SkScalar x, SkScalar y, const SkPaint* p) {
    this->updateState();
    fXPS->drawBitmap(fMatrix, fClip, bm, SkPoint{x, y}, p);
}

void XPSCanvas::onDrawVertices(VertexMode, int, const SkPoint[], const SkPoint[], const SkColor[],
                                 SkBlendMode, const uint16_t[], int, const SkPaint&) {
    SkDebugf("Unimplmented: XPSCanvas::onDrawVertices\n");
}

void XPSCanvas::internalSaveLayer(const SkCanvas::SaveLayerRec& rec) {
    fXPS->saveLayer(rec);
}

void XPSCanvas::internalRestoreLayer() {
    fXPS->restoreLayer();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class XPSDocument final : public SkDocument {
public:
    XPSDocument(SkWStream* wStream, void (*release)(SkWStream*, bool),
                IXpsOMObjectFactory* f, SkScalar dpi)
        : SkDocument(wStream, release), fXPS(wStream, f, dpi) {}
    ~XPSDocument() { this->close(); }

protected:
    SkCanvas* onBeginPage(SkScalar w, SkScalar h, const SkRect& trimBox) override {
        SkSize size = SkSize::Make(w, h);
        fXPS.newPage(size);
        fCanvas = skstd::make_unique<XPSCanvas>(&fXPS, size.toCeil());
        if (trimBox != SkRect::MakeSize(size)) {
            fCanvas->clipRect(trimBox);
            fCanvas->translate(trimBox.x(), trimBox.y());
        }
        return fCanvas.get();
    }
    void onEndPage() override {}
    void onClose(SkWStream*) override {
        fCanvas = nullptr;
        fXPS.endPortfolio();
    }
    void onAbort() override { /* TODO: implement */ }

private:
    SkXPS fXPS;
    std::unique_ptr<XPSCanvas> fCanvas;
};

static sk_sp<SkDocument> make_xps_document(SkWStream* wStream,
                                           void (*release)(SkWStream*, bool),
                                           SkScalar dpi) {
    if (!wStream) {
        return nullptr;
    }
    SkTScopedComPtr<IXpsOMObjectFactory> xpsFactory;
    (void)CoCreateInstance(CLSID_XpsOMObjectFactory,
                           nullptr,
                           CLSCTX_INPROC_SERVER,
                           IID_PPV_ARGS(&xpsFactory));
    if (!xpsFactory.get() || dpi <= 0) {
        if (release) {
            release(wStream, true);
        }
    }
    return sk_make_sp<XPSDocument>(wStream, release, xpsFactory.get(), dpi);
}

} // namespace

sk_sp<SkDocument> SkDocument::MakeXPS(SkWStream* wStream, SkScalar dpi) {
    return make_xps_document(wStream, nullptr, dpi);
}

sk_sp<SkDocument> SkDocument::MakeXPS(const char path[], SkScalar dpi) {
    auto wStream = skstd::make_unique<SkFILEWStream>(path);
    if (!wStream->isValid() || dpi <= 0) {
        return nullptr;
    }
    return make_xps_document(wStream.release(), [](SkWStream* p, bool) { delete p; }, dpi);
}

#endif  //  SK_BUILD_FOR_WIN
