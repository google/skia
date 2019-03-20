/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DebugCanvas.h"
#include "DrawCommand.h"
#include "SkCanvasPriv.h"
#include "SkClipOpPriv.h"
#include "SkJSONWriter.h"
#include "SkPaintFilterCanvas.h"
#include "SkPicture.h"
#include "SkRectPriv.h"
#include "SkTextBlob.h"

#include "GrAuditTrail.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrRenderTargetContext.h"

#define SKDEBUGCANVAS_VERSION 1
#define SKDEBUGCANVAS_ATTRIBUTE_VERSION "version"
#define SKDEBUGCANVAS_ATTRIBUTE_COMMANDS "commands"
#define SKDEBUGCANVAS_ATTRIBUTE_AUDITTRAIL "auditTrail"

class DebugPaintFilterCanvas : public SkPaintFilterCanvas {
public:
    DebugPaintFilterCanvas(SkCanvas* canvas, bool overdrawViz)
            : INHERITED(canvas), fOverdrawViz(overdrawViz) {}

protected:
    bool onFilter(SkTCopyOnFirstWrite<SkPaint>* paint, Type) const override {
        if (*paint) {
            if (fOverdrawViz) {
                paint->writable()->setColor(SK_ColorRED);
                paint->writable()->setAlpha(0x08);
                paint->writable()->setBlendMode(SkBlendMode::kSrcOver);
            }
        }
        return true;
    }

    void onDrawPicture(const SkPicture* picture,
                       const SkMatrix*  matrix,
                       const SkPaint*   paint) override {
        // We need to replay the picture onto this canvas in order to filter its internal paints.
        this->SkCanvas::onDrawPicture(picture, matrix, paint);
    }

private:
    bool fOverdrawViz;

    typedef SkPaintFilterCanvas INHERITED;
};

DebugCanvas::DebugCanvas(int width, int height)
        : INHERITED(width, height)
        , fOverdrawViz(false)
        , fClipVizColor(SK_ColorTRANSPARENT)
        , fDrawGpuOpBounds(false) {
    // SkPicturePlayback uses the base-class' quickReject calls to cull clipped
    // operations. This can lead to problems in the debugger which expects all
    // the operations in the captured skp to appear in the debug canvas. To
    // circumvent this we create a wide open clip here (an empty clip rect
    // is not sufficient).
    // Internally, the SkRect passed to clipRect is converted to an SkIRect and
    // rounded out. The following code creates a nearly maximal rect that will
    // not get collapsed by the coming conversions (Due to precision loss the
    // inset has to be surprisingly large).
    SkIRect largeIRect = SkRectPriv::MakeILarge();
    largeIRect.inset(1024, 1024);
    SkRect large = SkRect::Make(largeIRect);
#ifdef SK_DEBUG
    SkASSERT(!large.roundOut().isEmpty());
#endif
    // call the base class' version to avoid adding a draw command
    this->INHERITED::onClipRect(large, kReplace_SkClipOp, kHard_ClipEdgeStyle);
}

DebugCanvas::DebugCanvas(SkIRect bounds) { DebugCanvas(bounds.width(), bounds.height()); }

DebugCanvas::~DebugCanvas() { fCommandVector.deleteAll(); }

void DebugCanvas::addDrawCommand(DrawCommand* command) { fCommandVector.push_back(command); }

void DebugCanvas::draw(SkCanvas* canvas) {
    if (!fCommandVector.isEmpty()) {
        this->drawTo(canvas, fCommandVector.count() - 1);
    }
}

void DebugCanvas::drawTo(SkCanvas* originalCanvas, int index, int m) {
    SkASSERT(!fCommandVector.isEmpty());
    SkASSERT(index < fCommandVector.count());

    int saveCount = originalCanvas->save();

    SkRect windowRect = SkRect::MakeWH(SkIntToScalar(originalCanvas->getBaseLayerSize().width()),
                                       SkIntToScalar(originalCanvas->getBaseLayerSize().height()));

    originalCanvas->clear(SK_ColorWHITE);
    originalCanvas->resetMatrix();
    if (!windowRect.isEmpty()) {
        originalCanvas->clipRect(windowRect, kReplace_SkClipOp);
    }

    DebugPaintFilterCanvas filterCanvas(originalCanvas, fOverdrawViz);

    // If we have a GPU backend we can also visualize the op information
    GrAuditTrail* at = nullptr;
    if (fDrawGpuOpBounds || m != -1) {
        // The audit trail must be obtained from the original canvas.
        at = this->getAuditTrail(originalCanvas);
    }

    for (int i = 0; i <= index; i++) {
        // We need to flush any pending operations, or they might combine with commands below.
        // Previous operations were not registered with the audit trail when they were
        // created, so if we allow them to combine, the audit trail will fail to find them.
        filterCanvas.flush();

        GrAuditTrail::AutoCollectOps* acb = nullptr;
        if (at) {
            acb = new GrAuditTrail::AutoCollectOps(at, i);
        }

        if (fCommandVector[i]->isVisible()) {
            fCommandVector[i]->execute(&filterCanvas);
        }
        if (at && acb) {
            delete acb;
        }
    }

    if (SkColorGetA(fClipVizColor) != 0) {
        filterCanvas.save();
#define LARGE_COORD 1000000000
        filterCanvas.clipRect(
                SkRect::MakeLTRB(-LARGE_COORD, -LARGE_COORD, LARGE_COORD, LARGE_COORD),
                kReverseDifference_SkClipOp);
        SkPaint clipPaint;
        clipPaint.setColor(fClipVizColor);
        filterCanvas.drawPaint(clipPaint);
        filterCanvas.restore();
    }

    fMatrix = filterCanvas.getTotalMatrix();
    fClip   = filterCanvas.getDeviceClipBounds();
    filterCanvas.restoreToCount(saveCount);

    // draw any ops if required and issue a full reset onto GrAuditTrail
    if (at) {
        // just in case there is global reordering, we flush the canvas before querying
        // GrAuditTrail
        GrAuditTrail::AutoEnable ae(at);
        filterCanvas.flush();

        // we pick three colorblind-safe colors, 75% alpha
        static const SkColor kTotalBounds     = SkColorSetARGB(0xC0, 0x6A, 0x3D, 0x9A);
        static const SkColor kCommandOpBounds = SkColorSetARGB(0xC0, 0xE3, 0x1A, 0x1C);
        static const SkColor kOtherOpBounds   = SkColorSetARGB(0xC0, 0xFF, 0x7F, 0x00);

        // get the render target of the top device (from the original canvas) so we can ignore ops
        // drawn offscreen
        GrRenderTargetContext* rtc =
                originalCanvas->internal_private_accessTopLayerRenderTargetContext();
        GrSurfaceProxy::UniqueID proxyID = rtc->asSurfaceProxy()->uniqueID();

        // get the bounding boxes to draw
        SkTArray<GrAuditTrail::OpInfo> childrenBounds;
        if (m == -1) {
            at->getBoundsByClientID(&childrenBounds, index);
        } else {
            // the client wants us to draw the mth op
            at->getBoundsByOpListID(&childrenBounds.push_back(), m);
        }
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(1);
        for (int i = 0; i < childrenBounds.count(); i++) {
            if (childrenBounds[i].fProxyUniqueID != proxyID) {
                // offscreen draw, ignore for now
                continue;
            }
            paint.setColor(kTotalBounds);
            filterCanvas.drawRect(childrenBounds[i].fBounds, paint);
            for (int j = 0; j < childrenBounds[i].fOps.count(); j++) {
                const GrAuditTrail::OpInfo::Op& op = childrenBounds[i].fOps[j];
                if (op.fClientID != index) {
                    paint.setColor(kOtherOpBounds);
                } else {
                    paint.setColor(kCommandOpBounds);
                }
                filterCanvas.drawRect(op.fBounds, paint);
            }
        }
    }
    this->cleanupAuditTrail(originalCanvas);
}

void DebugCanvas::deleteDrawCommandAt(int index) {
    SkASSERT(index < fCommandVector.count());
    delete fCommandVector[index];
    fCommandVector.remove(index);
}

DrawCommand* DebugCanvas::getDrawCommandAt(int index) {
    SkASSERT(index < fCommandVector.count());
    return fCommandVector[index];
}

GrAuditTrail* DebugCanvas::getAuditTrail(SkCanvas* canvas) {
    GrAuditTrail* at  = nullptr;
    GrContext*    ctx = canvas->getGrContext();
    if (ctx) {
        at = ctx->priv().auditTrail();
    }
    return at;
}

void DebugCanvas::drawAndCollectOps(int n, SkCanvas* canvas) {
    GrAuditTrail* at = this->getAuditTrail(canvas);
    if (at) {
        // loop over all of the commands and draw them, this is to collect reordering
        // information
        for (int i = 0; i < this->getSize() && i <= n; i++) {
            GrAuditTrail::AutoCollectOps enable(at, i);
            fCommandVector[i]->execute(canvas);
        }

        // in case there is some kind of global reordering
        {
            GrAuditTrail::AutoEnable ae(at);
            canvas->flush();
        }
    }
}

void DebugCanvas::cleanupAuditTrail(SkCanvas* canvas) {
    GrAuditTrail* at = this->getAuditTrail(canvas);
    if (at) {
        GrAuditTrail::AutoEnable ae(at);
        at->fullReset();
    }
}

void DebugCanvas::toJSON(SkJSONWriter&   writer,
                         UrlDataManager& urlDataManager,
                         int             n,
                         SkCanvas*       canvas) {
    this->drawAndCollectOps(n, canvas);

    // now collect json
    GrAuditTrail* at = this->getAuditTrail(canvas);
    writer.appendS32(SKDEBUGCANVAS_ATTRIBUTE_VERSION, SKDEBUGCANVAS_VERSION);
    writer.beginArray(SKDEBUGCANVAS_ATTRIBUTE_COMMANDS);

    for (int i = 0; i < this->getSize() && i <= n; i++) {
        writer.beginObject();  // command
        this->getDrawCommandAt(i)->toJSON(writer, urlDataManager);

        if (at) {
            writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_AUDITTRAIL);
            at->toJson(writer, i);
        }
        writer.endObject();  // command
    }

    writer.endArray();  // commands
    this->cleanupAuditTrail(canvas);
}

void DebugCanvas::toJSONOpList(SkJSONWriter& writer, int n, SkCanvas* canvas) {
    this->drawAndCollectOps(n, canvas);

    GrAuditTrail* at = this->getAuditTrail(canvas);
    if (at) {
        GrAuditTrail::AutoManageOpList enable(at);
        at->toJson(writer);
    } else {
        writer.beginObject();
        writer.endObject();
    }
    this->cleanupAuditTrail(canvas);
}

void DebugCanvas::setOverdrawViz(bool overdrawViz) { fOverdrawViz = overdrawViz; }

void DebugCanvas::onClipPath(const SkPath& path, SkClipOp op, ClipEdgeStyle edgeStyle) {
    this->addDrawCommand(new SkClipPathCommand(path, op, kSoft_ClipEdgeStyle == edgeStyle));
}

void DebugCanvas::onClipRect(const SkRect& rect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    this->addDrawCommand(new SkClipRectCommand(rect, op, kSoft_ClipEdgeStyle == edgeStyle));
}

void DebugCanvas::onClipRRect(const SkRRect& rrect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    this->addDrawCommand(new SkClipRRectCommand(rrect, op, kSoft_ClipEdgeStyle == edgeStyle));
}

void DebugCanvas::onClipRegion(const SkRegion& region, SkClipOp op) {
    this->addDrawCommand(new SkClipRegionCommand(region, op));
}

void DebugCanvas::didConcat(const SkMatrix& matrix) {
    this->addDrawCommand(new SkConcatCommand(matrix));
    this->INHERITED::didConcat(matrix);
}

void DebugCanvas::onDrawAnnotation(const SkRect& rect, const char key[], SkData* value) {
    this->addDrawCommand(new SkDrawAnnotationCommand(rect, key, sk_ref_sp(value)));
}

void DebugCanvas::onDrawBitmap(const SkBitmap& bitmap,
                               SkScalar        left,
                               SkScalar        top,
                               const SkPaint*  paint) {
    this->addDrawCommand(new SkDrawBitmapCommand(bitmap, left, top, paint));
}

void DebugCanvas::onDrawBitmapLattice(const SkBitmap& bitmap,
                                      const Lattice&  lattice,
                                      const SkRect&   dst,
                                      const SkPaint*  paint) {
    this->addDrawCommand(new SkDrawBitmapLatticeCommand(bitmap, lattice, dst, paint));
}

void DebugCanvas::onDrawBitmapRect(const SkBitmap&   bitmap,
                                   const SkRect*     src,
                                   const SkRect&     dst,
                                   const SkPaint*    paint,
                                   SrcRectConstraint constraint) {
    this->addDrawCommand(
            new SkDrawBitmapRectCommand(bitmap, src, dst, paint, (SrcRectConstraint)constraint));
}

void DebugCanvas::onDrawBitmapNine(const SkBitmap& bitmap,
                                   const SkIRect&  center,
                                   const SkRect&   dst,
                                   const SkPaint*  paint) {
    this->addDrawCommand(new SkDrawBitmapNineCommand(bitmap, center, dst, paint));
}

void DebugCanvas::onDrawImage(const SkImage* image,
                              SkScalar       left,
                              SkScalar       top,
                              const SkPaint* paint) {
    this->addDrawCommand(new SkDrawImageCommand(image, left, top, paint));
}

void DebugCanvas::onDrawImageLattice(const SkImage* image,
                                     const Lattice& lattice,
                                     const SkRect&  dst,
                                     const SkPaint* paint) {
    this->addDrawCommand(new SkDrawImageLatticeCommand(image, lattice, dst, paint));
}

void DebugCanvas::onDrawImageRect(const SkImage*    image,
                                  const SkRect*     src,
                                  const SkRect&     dst,
                                  const SkPaint*    paint,
                                  SrcRectConstraint constraint) {
    this->addDrawCommand(new SkDrawImageRectCommand(image, src, dst, paint, constraint));
}

void DebugCanvas::onDrawImageNine(const SkImage* image,
                                  const SkIRect& center,
                                  const SkRect&  dst,
                                  const SkPaint* paint) {
    this->addDrawCommand(new SkDrawImageNineCommand(image, center, dst, paint));
}

void DebugCanvas::onDrawOval(const SkRect& oval, const SkPaint& paint) {
    this->addDrawCommand(new SkDrawOvalCommand(oval, paint));
}

void DebugCanvas::onDrawArc(const SkRect&  oval,
                            SkScalar       startAngle,
                            SkScalar       sweepAngle,
                            bool           useCenter,
                            const SkPaint& paint) {
    this->addDrawCommand(new SkDrawArcCommand(oval, startAngle, sweepAngle, useCenter, paint));
}

void DebugCanvas::onDrawPaint(const SkPaint& paint) {
    this->addDrawCommand(new SkDrawPaintCommand(paint));
}

void DebugCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    this->addDrawCommand(new SkDrawPathCommand(path, paint));
}

void DebugCanvas::onDrawRegion(const SkRegion& region, const SkPaint& paint) {
    this->addDrawCommand(new SkDrawRegionCommand(region, paint));
}

void DebugCanvas::onDrawPicture(const SkPicture* picture,
                                const SkMatrix*  matrix,
                                const SkPaint*   paint) {
    this->addDrawCommand(new SkBeginDrawPictureCommand(picture, matrix, paint));
    SkAutoCanvasMatrixPaint acmp(this, matrix, paint, picture->cullRect());
    picture->playback(this);
    this->addDrawCommand(new SkEndDrawPictureCommand(SkToBool(matrix) || SkToBool(paint)));
}

void DebugCanvas::onDrawPoints(PointMode      mode,
                               size_t         count,
                               const SkPoint  pts[],
                               const SkPaint& paint) {
    this->addDrawCommand(new SkDrawPointsCommand(mode, count, pts, paint));
}

void DebugCanvas::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    // NOTE(chudy): Messing up when renamed to DrawRect... Why?
    addDrawCommand(new SkDrawRectCommand(rect, paint));
}

void DebugCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    this->addDrawCommand(new SkDrawRRectCommand(rrect, paint));
}

void DebugCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    this->addDrawCommand(new SkDrawDRRectCommand(outer, inner, paint));
}

void DebugCanvas::onDrawTextBlob(const SkTextBlob* blob,
                                 SkScalar          x,
                                 SkScalar          y,
                                 const SkPaint&    paint) {
    this->addDrawCommand(
            new SkDrawTextBlobCommand(sk_ref_sp(const_cast<SkTextBlob*>(blob)), x, y, paint));
}

void DebugCanvas::onDrawPatch(const SkPoint  cubics[12],
                              const SkColor  colors[4],
                              const SkPoint  texCoords[4],
                              SkBlendMode    bmode,
                              const SkPaint& paint) {
    this->addDrawCommand(new SkDrawPatchCommand(cubics, colors, texCoords, bmode, paint));
}

void DebugCanvas::onDrawVerticesObject(const SkVertices*      vertices,
                                       const SkVertices::Bone bones[],
                                       int                    boneCount,
                                       SkBlendMode            bmode,
                                       const SkPaint&         paint) {
    // TODO: ANIMATION NOT LOGGED
    this->addDrawCommand(
            new SkDrawVerticesCommand(sk_ref_sp(const_cast<SkVertices*>(vertices)), bmode, paint));
}

void DebugCanvas::onDrawAtlas(const SkImage*  image,
                              const SkRSXform xform[],
                              const SkRect    tex[],
                              const SkColor   colors[],
                              int             count,
                              SkBlendMode     bmode,
                              const SkRect*   cull,
                              const SkPaint*  paint) {
    this->addDrawCommand(
            new SkDrawAtlasCommand(image, xform, tex, colors, count, bmode, cull, paint));
}

void DebugCanvas::onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) {
    this->addDrawCommand(new SkDrawShadowCommand(path, rec));
}

void DebugCanvas::onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) {
    this->addDrawCommand(new SkDrawDrawableCommand(drawable, matrix));
}

void DebugCanvas::onDrawEdgeAAQuad(const SkRect& rect,
                                   const SkPoint clip[4],
                                   QuadAAFlags   aa,
                                   SkColor       color,
                                   SkBlendMode   mode) {
    this->addDrawCommand(new SkDrawEdgeAAQuadCommand(rect, clip, aa, color, mode));
}

void DebugCanvas::onDrawEdgeAAImageSet(const ImageSetEntry set[],
                                       int                 count,
                                       const SkPoint       dstClips[],
                                       const SkMatrix      preViewMatrices[],
                                       const SkPaint*      paint,
                                       SrcRectConstraint   constraint) {
    this->addDrawCommand(new SkDrawEdgeAAImageSetCommand(
            set, count, dstClips, preViewMatrices, paint, constraint));
}

void DebugCanvas::willRestore() {
    this->addDrawCommand(new SkRestoreCommand());
    this->INHERITED::willRestore();
}

void DebugCanvas::willSave() {
    this->addDrawCommand(new SkSaveCommand());
    this->INHERITED::willSave();
}

SkCanvas::SaveLayerStrategy DebugCanvas::getSaveLayerStrategy(const SaveLayerRec& rec) {
    this->addDrawCommand(new SkSaveLayerCommand(rec));
    (void)this->INHERITED::getSaveLayerStrategy(rec);
    // No need for a full layer.
    return kNoLayer_SaveLayerStrategy;
}

bool DebugCanvas::onDoSaveBehind(const SkRect* subset) {
    // TODO
    return false;
}

void DebugCanvas::didSetMatrix(const SkMatrix& matrix) {
    this->addDrawCommand(new SkSetMatrixCommand(matrix));
    this->INHERITED::didSetMatrix(matrix);
}

void DebugCanvas::toggleCommand(int index, bool toggle) {
    SkASSERT(index < fCommandVector.count());
    fCommandVector[index]->setVisible(toggle);
}
