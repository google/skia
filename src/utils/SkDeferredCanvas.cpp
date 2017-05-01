/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDeferredCanvas.h"
#include "SkDrawable.h"
#include "SkPath.h"
#include "SkRSXform.h"
#include "SkRRect.h"
#include "SkSurface.h"
#include "SkTextBlob.h"
#include "SkClipOpPriv.h"

bool SkDeferredCanvas::Rec::isConcat(SkMatrix* m) const {
    switch (fType) {
        case kTrans_Type:
            m->setTranslate(fData.fTranslate.x(), fData.fTranslate.y());
            return true;
        case kScaleTrans_Type:
            m->setScaleTranslate(fData.fScaleTrans.fScale.x(),
                                 fData.fScaleTrans.fScale.y(),
                                 fData.fScaleTrans.fTrans.x(),
                                 fData.fScaleTrans.fTrans.y());
            return true;
        default:
            break;
    }
    return false;
}

void SkDeferredCanvas::Rec::setConcat(const SkMatrix& m) {
    SkASSERT(m.getType() <= (SkMatrix::kScale_Mask | SkMatrix::kTranslate_Mask));

    if (m.getType() <= SkMatrix::kTranslate_Mask) {
        fType = kTrans_Type;
        fData.fTranslate.set(m.getTranslateX(), m.getTranslateY());
    } else {
        fType = kScaleTrans_Type;
        fData.fScaleTrans.fScale.set(m.getScaleX(), m.getScaleY());
        fData.fScaleTrans.fTrans.set(m.getTranslateX(), m.getTranslateY());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkDeferredCanvas::SkDeferredCanvas(SkCanvas* canvas, EvalType evalType)
    : INHERITED(canvas->getBaseLayerSize().width(), canvas->getBaseLayerSize().height())
    , fCanvas(nullptr)  // must be here for reset to work.
    , fEvalType(evalType)
{
    this->reset(canvas);
}

SkDeferredCanvas::~SkDeferredCanvas() {}

void SkDeferredCanvas::reset(SkCanvas* canvas) {
    if (fCanvas) {
        this->flush();
        fCanvas = nullptr;
    }
    fRecs.reset();
    if (canvas) {
        this->resetCanvas(canvas->getBaseLayerSize().width(),
                          canvas->getBaseLayerSize().height());
        fCanvas = canvas;
    }
}

void SkDeferredCanvas::push_save() {
    Rec* r = fRecs.append();
    r->fType = kSave_Type;
}

void SkDeferredCanvas::push_cliprect(const SkRect& bounds) {
    int index = fRecs.count() - 1;
    if (index >= 0 && fRecs[index].fType == kClipRect_Type) {
        if (!fRecs[index].fData.fBounds.intersect(bounds)) {
            fRecs[index].fData.fBounds.setEmpty();
        }
    } else {
        Rec* r = fRecs.append();
        r->fType = kClipRect_Type;
        r->fData.fBounds = bounds;
    }
}

bool SkDeferredCanvas::push_concat(const SkMatrix& mat) {
    if (mat.getType() > (SkMatrix::kScale_Mask | SkMatrix::kTranslate_Mask)) {
        return false;
    }
    // At the moment, we don't know which ops can scale and which can also flip, so
    // we reject negative scales for now
    if (mat.getScaleX() < 0 || mat.getScaleY() < 0) {
        return false;
    }

    int index = fRecs.count() - 1;
    SkMatrix m;
    if (index >= 0 && fRecs[index].isConcat(&m)) {
        m.preConcat(mat);
        fRecs[index].setConcat(m);
    } else {
        fRecs.append()->setConcat(mat);
    }
    return true;
}

void SkDeferredCanvas::emit(const Rec& rec) {
    switch (rec.fType) {
        case kSave_Type:
            fCanvas->save();
            this->INHERITED::willSave();
            break;
        case kClipRect_Type:
            fCanvas->clipRect(rec.fData.fBounds);
            this->INHERITED::onClipRect(rec.fData.fBounds,
                                        kIntersect_SkClipOp, kHard_ClipEdgeStyle);
            break;
        case kTrans_Type:
        case kScaleTrans_Type: {
            SkMatrix mat;
            rec.getConcat(&mat);
            fCanvas->concat(mat);
            this->INHERITED::didConcat(mat);
        } break;
    }
}

void SkDeferredCanvas::flush_le(int index) {
    SkASSERT(index >= -1 && index < fRecs.count());

    int count = index + 1;
    for (int i = 0; i < count; ++i) {
        this->emit(fRecs[i]);
    }
    fRecs.remove(0, count);
}

void SkDeferredCanvas::flush_all() {
    this->flush_le(fRecs.count() - 1);
}

void SkDeferredCanvas::flush_before_saves() {
    int i;
    for (i = fRecs.count() - 1; i >= 0; --i) {
        if (kSave_Type != fRecs[i].fType) {
            break;
        }
    }
    this->flush_le(i);
}

enum Flags {
    kNoTranslate_Flag   = 1 << 0,
    kNoClip_Flag        = 1 << 1,
    kNoCull_Flag        = 1 << 2,
    kNoScale_Flag       = 1 << 3,
};

void SkDeferredCanvas::flush_check(SkRect* bounds, const SkPaint* paint, unsigned flags) {
    if (paint) {
        if (paint->getShader() || paint->getImageFilter()) {
            flags |= kNoTranslate_Flag | kNoScale_Flag;
        }
        // TODO: replace these with code to enlarge the bounds conservatively?
        if (paint->getStyle() != SkPaint::kFill_Style || paint->getMaskFilter() ||
            paint->getImageFilter() || paint->getPathEffect())
        {
            flags |= kNoCull_Flag | kNoScale_Flag | kNoClip_Flag;
        }
        if (paint->getLooper()) {
            // to be conservative, we disable both, since embedded layers could have shaders
            // or strokes etc.
            flags |= kNoTranslate_Flag | kNoCull_Flag | kNoScale_Flag;
        }
    }
    bool canClip = !(flags & kNoClip_Flag);
    bool canTranslate = !(flags & kNoTranslate_Flag);
    bool canCull = !(flags & kNoCull_Flag);
    bool canScale = !(flags & kNoScale_Flag);

    int i;
    for (i = fRecs.count() - 1; i >= 0; --i) {
        const Rec& rec = fRecs[i];
        switch (rec.fType) {
            case kSave_Type:
                // continue to the next rec
                break;
            case kClipRect_Type:
                if (!canCull) {
                    goto STOP;
                }
                if (canClip) {
                    if (!bounds->intersect(rec.fData.fBounds)) {
                        bounds->setEmpty();
                        return;
                    }
                    // continue to the next rec
                } else {
                    if (!rec.fData.fBounds.contains(*bounds)) {
                        goto STOP;
                    }
                    // continue to the next rec
                }
                break;
            case kTrans_Type:
                if (canTranslate) {
                    bounds->offset(rec.fData.fTranslate.x(), rec.fData.fTranslate.y());
                    // continue to the next rec
                } else {
                    goto STOP;
                }
                break;
            case kScaleTrans_Type:
                if (canScale) {
                    SkMatrix m;
                    rec.getConcat(&m);
                    m.mapRectScaleTranslate(bounds, *bounds);
                } else {
                    goto STOP;
                }
                break;
        }
    }
STOP:
    this->flush_le(i);
}

void SkDeferredCanvas::flush_translate(SkScalar* x, SkScalar* y, const SkRect& bounds,
                                       const SkPaint* paint) {
    SkRect tmp = bounds;
    this->flush_check(&tmp, paint, kNoClip_Flag | kNoScale_Flag);
    *x += tmp.x() - bounds.x();
    *y += tmp.y() - bounds.y();
}

void SkDeferredCanvas::flush_translate(SkScalar* x, SkScalar* y, const SkPaint& paint) {
    SkRect tmp = SkRect::MakeXYWH(*x, *y, 1, 1);
    this->flush_check(&tmp, &paint, kNoClip_Flag | kNoCull_Flag | kNoScale_Flag);
    *x = tmp.x();
    *y = tmp.y();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkDeferredCanvas::willSave() {
    this->push_save();
}

SkCanvas::SaveLayerStrategy SkDeferredCanvas::getSaveLayerStrategy(const SaveLayerRec& rec) {
    this->flush_all();
    fCanvas->saveLayer(rec);
    this->INHERITED::getSaveLayerStrategy(rec);
    // No need for a layer.
    return kNoLayer_SaveLayerStrategy;
}

void SkDeferredCanvas::willRestore() {
    for (int i = fRecs.count() - 1; i >= 0; --i) {
        if (kSave_Type == fRecs[i].fType) {
            fRecs.setCount(i);  // pop off everything here and later
            return;
        }
    }
    for (int i = 0; i < fRecs.count(); ++i) {
        SkASSERT(kSave_Type != fRecs[i].fType);
    }
    fRecs.setCount(0);
    fCanvas->restore();
    this->INHERITED::willRestore();
}

void SkDeferredCanvas::didConcat(const SkMatrix& matrix) {
    if (matrix.isIdentity()) {
        return;
    }
    if (!this->push_concat(matrix)) {
        this->flush_all();
        fCanvas->concat(matrix);
        this->INHERITED::didConcat(matrix);
    }
}

void SkDeferredCanvas::didSetMatrix(const SkMatrix& matrix) {
    this->flush_all();
    fCanvas->setMatrix(matrix);
    this->INHERITED::didSetMatrix(matrix);
}

void SkDeferredCanvas::onClipRect(const SkRect& rect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    if (kIntersect_SkClipOp == op) {
        this->push_cliprect(rect);
    } else {
        this->flush_all();
        fCanvas->clipRect(rect, op, kSoft_ClipEdgeStyle == edgeStyle);
        this->INHERITED::onClipRect(rect, op, edgeStyle);
    }
}

void SkDeferredCanvas::onClipRRect(const SkRRect& rrect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    this->flush_all();
    fCanvas->clipRRect(rrect, op, kSoft_ClipEdgeStyle == edgeStyle);
    this->INHERITED::onClipRRect(rrect, op, edgeStyle);
}

void SkDeferredCanvas::onClipPath(const SkPath& path, SkClipOp op, ClipEdgeStyle edgeStyle) {
    this->flush_all();
    fCanvas->clipPath(path, op, kSoft_ClipEdgeStyle == edgeStyle);
    this->INHERITED::onClipPath(path, op, edgeStyle);
}

void SkDeferredCanvas::onClipRegion(const SkRegion& deviceRgn, SkClipOp op) {
    this->flush_all();
    fCanvas->clipRegion(deviceRgn, op);
    this->INHERITED::onClipRegion(deviceRgn, op);
}

void SkDeferredCanvas::onDrawPaint(const SkPaint& paint) {
    // TODO: Can we turn this into drawRect?
    this->flush_all();
    fCanvas->drawPaint(paint);
}

void SkDeferredCanvas::onDrawPoints(PointMode mode, size_t count, const SkPoint pts[],
                                const SkPaint& paint) {
    this->flush_all();
    fCanvas->drawPoints(mode, count, pts, paint);
}

void SkDeferredCanvas::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    SkRect modRect = rect;
    this->flush_check(&modRect, &paint);
    fCanvas->drawRect(modRect, paint);
}

void SkDeferredCanvas::onDrawRegion(const SkRegion& region, const SkPaint& paint) {
    this->flush_all();  // can we do better?
    fCanvas->drawRegion(region, paint);
}

void SkDeferredCanvas::onDrawOval(const SkRect& rect, const SkPaint& paint) {
    SkRect modRect = rect;
    this->flush_check(&modRect, &paint, kNoClip_Flag);
    fCanvas->drawOval(modRect, paint);
}

void SkDeferredCanvas::onDrawArc(const SkRect& rect, SkScalar startAngle, SkScalar sweepAngle,
                                 bool useCenter, const SkPaint& paint) {
    SkRect modRect = rect;
    this->flush_check(&modRect, &paint, kNoClip_Flag);
    fCanvas->drawArc(modRect, startAngle, sweepAngle, useCenter, paint);
}

static SkRRect make_offset(const SkRRect& src, SkScalar dx, SkScalar dy) {
    SkRRect dst = src;
    dst.offset(dx, dy);
    return dst;
}

void SkDeferredCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    SkRect modRect = rrect.getBounds();
    this->flush_check(&modRect, &paint, kNoClip_Flag);
    fCanvas->drawRRect(make_offset(rrect,
                                   modRect.x() - rrect.getBounds().x(),
                                   modRect.y() - rrect.getBounds().y()), paint);
}

void SkDeferredCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    this->flush_all();
    fCanvas->drawDRRect(outer, inner, paint);
}

void SkDeferredCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    if (path.isInverseFillType()) {
        this->flush_before_saves();
    } else {
        SkRect modRect = path.getBounds();
        this->flush_check(&modRect, &paint, kNoClip_Flag | kNoTranslate_Flag | kNoScale_Flag);
    }
    fCanvas->drawPath(path, paint);
}

void SkDeferredCanvas::onDrawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y,
                                const SkPaint* paint) {
    const SkScalar w = SkIntToScalar(bitmap.width());
    const SkScalar h = SkIntToScalar(bitmap.height());
    SkRect bounds = SkRect::MakeXYWH(x, y, w, h);
    this->flush_check(&bounds, paint, kNoClip_Flag);
    if (bounds.width() == w && bounds.height() == h) {
        fCanvas->drawBitmap(bitmap, bounds.x(), bounds.y(), paint);
    } else {
        fCanvas->drawBitmapRect(bitmap, bounds, paint);
    }
}

void SkDeferredCanvas::onDrawBitmapRect(const SkBitmap& bitmap, const SkRect* src, const SkRect& dst,
                                    const SkPaint* paint, SrcRectConstraint constraint) {
    SkRect modRect = dst;
    this->flush_check(&modRect, paint, kNoClip_Flag);
    fCanvas->legacy_drawBitmapRect(bitmap, src, modRect, paint, constraint);
}

void SkDeferredCanvas::onDrawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                    const SkRect& dst, const SkPaint* paint) {
    SkRect modRect = dst;
    this->flush_check(&modRect, paint, kNoClip_Flag);
    fCanvas->drawBitmapNine(bitmap, center, modRect, paint);
}

void SkDeferredCanvas::onDrawBitmapLattice(const SkBitmap& bitmap, const Lattice& lattice,
                                           const SkRect& dst, const SkPaint* paint) {
    SkRect modRect = dst;
    this->flush_check(&modRect, paint, kNoClip_Flag);
    fCanvas->drawBitmapLattice(bitmap, lattice, modRect, paint);
}

void SkDeferredCanvas::onDrawImageNine(const SkImage* image, const SkIRect& center,
                                       const SkRect& dst, const SkPaint* paint) {
    SkRect modRect = dst;
    this->flush_check(&modRect, paint, kNoClip_Flag);
    fCanvas->drawImageNine(image, center, modRect, paint);
}

void SkDeferredCanvas::onDrawImage(const SkImage* image, SkScalar x, SkScalar y,
                                   const SkPaint* paint) {
    const SkScalar w = SkIntToScalar(image->width());
    const SkScalar h = SkIntToScalar(image->height());
    SkRect bounds = SkRect::MakeXYWH(x, y, w, h);
    this->flush_check(&bounds, paint, kNoClip_Flag);
    if (bounds.width() == w && bounds.height() == h) {
        fCanvas->drawImage(image, bounds.x(), bounds.y(), paint);
    } else {
        fCanvas->drawImageRect(image, bounds, paint);
    }
}

void SkDeferredCanvas::onDrawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                                   const SkPaint* paint, SrcRectConstraint constraint) {
    SkRect modRect = dst;
    this->flush_check(&modRect, paint, kNoClip_Flag);
    fCanvas->legacy_drawImageRect(image, src, modRect, paint, constraint);
}

void SkDeferredCanvas::onDrawImageLattice(const SkImage* image, const Lattice& lattice,
                                          const SkRect& dst, const SkPaint* paint) {
    SkRect modRect = dst;
    this->flush_check(&modRect, paint, kNoClip_Flag);
    fCanvas->drawImageLattice(image, lattice, modRect, paint);
}

void SkDeferredCanvas::onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                                  const SkPaint& paint) {
    this->flush_translate(&x, &y, paint);
    fCanvas->drawText(text, byteLength, x, y, paint);
}

void SkDeferredCanvas::onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                                 const SkPaint& paint) {
    this->flush_before_saves();
    fCanvas->drawPosText(text, byteLength, pos, paint);
}

void SkDeferredCanvas::onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                                  SkScalar constY, const SkPaint& paint) {
    this->flush_before_saves();
    fCanvas->drawPosTextH(text, byteLength, xpos, constY, paint);
}

void SkDeferredCanvas::onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                                    const SkMatrix* matrix, const SkPaint& paint) {
    this->flush_before_saves();
    fCanvas->drawTextOnPath(text, byteLength, path, matrix, paint);
}

void SkDeferredCanvas::onDrawTextRSXform(const void* text, size_t byteLength,
                                         const SkRSXform xform[], const SkRect* cullRect,
                                         const SkPaint& paint) {
    if (cullRect) {
        SkRect modRect = *cullRect;
        // only allow culling
        this->flush_check(&modRect, &paint, kNoClip_Flag | kNoScale_Flag | kNoTranslate_Flag);
    } else {
        this->flush_before_saves();
    }
    fCanvas->drawTextRSXform(text, byteLength, xform, cullRect, paint);
}

void SkDeferredCanvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                  const SkPaint &paint) {
    this->flush_translate(&x, &y, blob->bounds(), &paint);
    fCanvas->drawTextBlob(blob, x, y, paint);
}

#include "SkPicture.h"
#include "SkCanvasPriv.h"
void SkDeferredCanvas::onDrawPicture(const SkPicture* picture, const SkMatrix* matrix,
                                 const SkPaint* paint) {
    if (kEager == fEvalType) {
        SkAutoCanvasMatrixPaint acmp(this, matrix, paint, picture->cullRect());
        picture->playback(this);
    } else {
        this->flush_before_saves();
        fCanvas->drawPicture(picture, matrix, paint);
    }
}

void SkDeferredCanvas::onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) {
    if (kEager == fEvalType) {
        // TODO: investigate culling and applying concat to the matrix
        drawable->draw(this, matrix);
    } else {
        this->flush_before_saves();
        fCanvas->drawDrawable(drawable, matrix);
    }
}

void SkDeferredCanvas::onDrawAtlas(const SkImage* image, const SkRSXform xform[],
                                   const SkRect rects[], const SkColor colors[],
                                   int count, SkBlendMode bmode,
                                   const SkRect* cull, const SkPaint* paint) {
    this->flush_before_saves();
    fCanvas->drawAtlas(image, xform, rects, colors, count, bmode, cull, paint);
}

void SkDeferredCanvas::onDrawVerticesObject(const SkVertices* vertices, SkBlendMode bmode,
                                            const SkPaint& paint) {
    this->flush_before_saves();
    fCanvas->drawVertices(vertices, bmode, paint);
}

void SkDeferredCanvas::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                               const SkPoint texCoords[4], SkBlendMode bmode,
                               const SkPaint& paint) {
    this->flush_before_saves();
    fCanvas->drawPatch(cubics, colors, texCoords, bmode, paint);
}

void SkDeferredCanvas::onDrawAnnotation(const SkRect& rect, const char key[], SkData* data) {
    SkRect modRect = rect;
    this->flush_check(&modRect, nullptr, kNoClip_Flag);
    fCanvas->drawAnnotation(modRect, key, data);
}

#ifdef SK_SUPPORT_LEGACY_DRAWFILTER
SkDrawFilter* SkDeferredCanvas::setDrawFilter(SkDrawFilter* filter) {
    fCanvas->setDrawFilter(filter);
    return this->INHERITED::setDrawFilter(filter);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkSurface> SkDeferredCanvas::onNewSurface(const SkImageInfo& info,
                                                const SkSurfaceProps& props) {
    return fCanvas->makeSurface(info, &props);
}
SkISize SkDeferredCanvas::getBaseLayerSize() const { return fCanvas->getBaseLayerSize(); }
SkRect SkDeferredCanvas::onGetLocalClipBounds() const {
    return fCanvas->getLocalClipBounds();
}
SkIRect SkDeferredCanvas::onGetDeviceClipBounds() const {
    return fCanvas->getDeviceClipBounds();
}
bool SkDeferredCanvas::isClipEmpty() const { return fCanvas->isClipEmpty(); }
bool SkDeferredCanvas::isClipRect() const { return fCanvas->isClipRect(); }
bool SkDeferredCanvas::onPeekPixels(SkPixmap* pixmap) { return fCanvas->peekPixels(pixmap); }
bool SkDeferredCanvas::onAccessTopLayerPixels(SkPixmap* pixmap) {
    SkImageInfo info;
    size_t rowBytes;
    SkIPoint* origin = nullptr;
    void* addr = fCanvas->accessTopLayerPixels(&info, &rowBytes, origin);
    if (addr) {
        *pixmap = SkPixmap(info, addr, rowBytes);
        return true;
    }
    return false;
}
SkImageInfo SkDeferredCanvas::onImageInfo() const { return fCanvas->imageInfo(); }
bool SkDeferredCanvas::onGetProps(SkSurfaceProps* props) const { return fCanvas->getProps(props); }
void SkDeferredCanvas::onFlush() {
    this->flush_all();
    return fCanvas->flush();
}
