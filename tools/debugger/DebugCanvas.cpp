/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPicture.h"
#include "include/core/SkTextBlob.h"
#include "include/utils/SkPaintFilterCanvas.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkClipOpPriv.h"
#include "src/core/SkRectPriv.h"
#include "src/utils/SkJSONWriter.h"
#include "tools/debugger/DebugCanvas.h"
#include "tools/debugger/DebugLayerManager.h"
#include "tools/debugger/DrawCommand.h"

#include "include/gpu/GrContext.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"

#include <string>

#define SKDEBUGCANVAS_VERSION 1
#define SKDEBUGCANVAS_ATTRIBUTE_VERSION "version"
#define SKDEBUGCANVAS_ATTRIBUTE_COMMANDS "commands"
#define SKDEBUGCANVAS_ATTRIBUTE_AUDITTRAIL "auditTrail"

namespace {
    // Constants used in Annotations by Android for keeping track of layers
    static constexpr char kOffscreenLayerDraw[] = "OffscreenLayerDraw";
    static constexpr char kSurfaceID[] = "SurfaceID";
    static constexpr char kAndroidClip[] = "AndroidDeviceClipRestriction";
} // namespace

class DebugPaintFilterCanvas : public SkPaintFilterCanvas {
public:
    DebugPaintFilterCanvas(SkCanvas* canvas) : INHERITED(canvas) {}

protected:
    bool onFilter(SkPaint& paint) const override {
        paint.setColor(SK_ColorRED);
        paint.setAlpha(0x08);
        paint.setBlendMode(SkBlendMode::kSrcOver);
        return true;
    }

    void onDrawPicture(const SkPicture* picture,
                       const SkMatrix*  matrix,
                       const SkPaint*   paint) override {
        // We need to replay the picture onto this canvas in order to filter its internal paints.
        this->SkCanvas::onDrawPicture(picture, matrix, paint);
    }

private:

    typedef SkPaintFilterCanvas INHERITED;
};

DebugCanvas::DebugCanvas(int width, int height)
        : INHERITED(width, height)
        , fOverdrawViz(false)
        , fClipVizColor(SK_ColorTRANSPARENT)
        , fDrawGpuOpBounds(false)
        , fShowAndroidClip(false)
        , fnextDrawPictureLayerId(-1)
        , fnextDrawImageRectLayerId(-1)
        , fAndroidClip(SkRect::MakeEmpty()) {
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

DebugCanvas::DebugCanvas(SkIRect bounds) {
    DebugCanvas(bounds.width(), bounds.height());
}

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

    originalCanvas->resetMatrix();
    if (!windowRect.isEmpty()) {
        originalCanvas->clipRect(windowRect, kReplace_SkClipOp);
    }

    DebugPaintFilterCanvas filterCanvas(originalCanvas);
    SkCanvas* finalCanvas = fOverdrawViz ? &filterCanvas : originalCanvas;

    // If we have a GPU backend we can also visualize the op information
    GrAuditTrail* at = nullptr;
    if (fDrawGpuOpBounds || m != -1) {
        // The audit trail must be obtained from the original canvas.
        at = this->getAuditTrail(originalCanvas);
    }

    for (int i = 0; i <= index; i++) {
        GrAuditTrail::AutoCollectOps* acb = nullptr;
        if (at) {
            // We need to flush any pending operations, or they might combine with commands below.
            // Previous operations were not registered with the audit trail when they were
            // created, so if we allow them to combine, the audit trail will fail to find them.
            finalCanvas->flush();
            acb = new GrAuditTrail::AutoCollectOps(at, i);
        }
        if (fCommandVector[i]->isVisible()) {
            fCommandVector[i]->execute(finalCanvas);
        }
        if (at && acb) {
            delete acb;
        }
    }

    if (SkColorGetA(fClipVizColor) != 0) {
        finalCanvas->save();
        SkPaint clipPaint;
        clipPaint.setColor(fClipVizColor);
        finalCanvas->drawPaint(clipPaint);
        finalCanvas->restore();
    }

    fMatrix = finalCanvas->getTotalMatrix();
    fClip   = finalCanvas->getDeviceClipBounds();
    finalCanvas->restoreToCount(saveCount);

    if (fShowAndroidClip) {
        // Draw visualization of android device clip restriction
        SkPaint androidClipPaint;
        androidClipPaint.setARGB(80, 255, 100, 0);
        finalCanvas->drawRect(fAndroidClip, androidClipPaint);
    }

    // draw any ops if required and issue a full reset onto GrAuditTrail
    if (at) {
        // just in case there is global reordering, we flush the canvas before querying
        // GrAuditTrail
        GrAuditTrail::AutoEnable ae(at);
        finalCanvas->flush();

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
            at->getBoundsByOpsTaskID(&childrenBounds.push_back(), m);
        }
        // Shift the rects half a pixel, so they appear as exactly 1px thick lines.
        finalCanvas->save();
        finalCanvas->translate(0.5, -0.5);
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(1);
        for (int i = 0; i < childrenBounds.count(); i++) {
            if (childrenBounds[i].fProxyUniqueID != proxyID) {
                // offscreen draw, ignore for now
                continue;
            }
            paint.setColor(kTotalBounds);
            finalCanvas->drawRect(childrenBounds[i].fBounds, paint);
            for (int j = 0; j < childrenBounds[i].fOps.count(); j++) {
                const GrAuditTrail::OpInfo::Op& op = childrenBounds[i].fOps[j];
                if (op.fClientID != index) {
                    paint.setColor(kOtherOpBounds);
                } else {
                    paint.setColor(kCommandOpBounds);
                }
                finalCanvas->drawRect(op.fBounds, paint);
            }
        }
        finalCanvas->restore();
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

void DebugCanvas::drawAndCollectOps(SkCanvas* canvas) {
    GrAuditTrail* at = this->getAuditTrail(canvas);
    if (at) {
        // loop over all of the commands and draw them, this is to collect reordering
        // information
        for (int i = 0; i < this->getSize(); i++) {
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
                         SkCanvas*       canvas) {
    this->drawAndCollectOps(canvas);

    // now collect json
    GrAuditTrail* at = this->getAuditTrail(canvas);
    writer.appendS32(SKDEBUGCANVAS_ATTRIBUTE_VERSION, SKDEBUGCANVAS_VERSION);
    writer.beginArray(SKDEBUGCANVAS_ATTRIBUTE_COMMANDS);

    for (int i = 0; i < this->getSize(); i++) {
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

void DebugCanvas::toJSONOpsTask(SkJSONWriter& writer, SkCanvas* canvas) {
    this->drawAndCollectOps(canvas);

    GrAuditTrail* at = this->getAuditTrail(canvas);
    if (at) {
        GrAuditTrail::AutoManageOpsTask enable(at);
        at->toJson(writer);
    } else {
        writer.beginObject();
        writer.endObject();
    }
    this->cleanupAuditTrail(canvas);
}

void DebugCanvas::setOverdrawViz(bool overdrawViz) { fOverdrawViz = overdrawViz; }

void DebugCanvas::onClipPath(const SkPath& path, SkClipOp op, ClipEdgeStyle edgeStyle) {
    this->addDrawCommand(new ClipPathCommand(path, op, kSoft_ClipEdgeStyle == edgeStyle));
}

void DebugCanvas::onClipRect(const SkRect& rect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    this->addDrawCommand(new ClipRectCommand(rect, op, kSoft_ClipEdgeStyle == edgeStyle));
}

void DebugCanvas::onClipRRect(const SkRRect& rrect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    this->addDrawCommand(new ClipRRectCommand(rrect, op, kSoft_ClipEdgeStyle == edgeStyle));
}

void DebugCanvas::onClipRegion(const SkRegion& region, SkClipOp op) {
    this->addDrawCommand(new ClipRegionCommand(region, op));
}

void DebugCanvas::onClipShader(sk_sp<SkShader> cs, SkClipOp op) {
    this->addDrawCommand(new ClipShaderCommand(std::move(cs), op));
}

void DebugCanvas::didConcat44(const SkScalar m[16]) {
    // TODO
    this->INHERITED::didConcat44(m);
}

void DebugCanvas::didScale(SkScalar x, SkScalar y) {
    this->didConcat(SkMatrix::MakeScale(x, y));
}

void DebugCanvas::didTranslate(SkScalar x, SkScalar y) {
    this->didConcat(SkMatrix::MakeTrans(x, y));
}

void DebugCanvas::didConcat(const SkMatrix& matrix) {
    this->addDrawCommand(new ConcatCommand(matrix));
    this->INHERITED::didConcat(matrix);
}

void DebugCanvas::onDrawAnnotation(const SkRect& rect, const char key[], SkData* value) {
    // Parse layer-releated annotations added in SkiaPipeline.cpp and RenderNodeDrawable.cpp
    // the format of the annotations is <Indicator|RenderNodeId>
    SkTArray<SkString> tokens;
    SkStrSplit(key, "|", kStrict_SkStrSplitMode, &tokens);
    if (tokens.size() == 2) {
        if (tokens[0].equals(kOffscreenLayerDraw)) {
            // Indicates that the next drawPicture command contains the SkPicture to render the node
            // at this id in an offscreen buffer.
            fnextDrawPictureLayerId = std::stoi(tokens[1].c_str());
            fnextDrawPictureDirtyRect = rect.roundOut();
            return; // don't record it
        } else if (tokens[0].equals(kSurfaceID)) {
            // Indicates that the following drawImageRect should draw the offscreen buffer.
            fnextDrawImageRectLayerId = std::stoi(tokens[1].c_str());
            return; // don't record it
        }
    }
    if (strcmp(kAndroidClip, key) == 0) {
        // Store this frame's android device clip restriction for visualization later.
        // This annotation stands in place of the androidFramework_setDeviceClipRestriction
        // which is unrecordable.
        fAndroidClip = rect;
    }
    this->addDrawCommand(new DrawAnnotationCommand(rect, key, sk_ref_sp(value)));
}

void DebugCanvas::onDrawImage(const SkImage* image,
                              SkScalar       left,
                              SkScalar       top,
                              const SkPaint* paint) {
    this->addDrawCommand(new DrawImageCommand(image, left, top, paint));
}

void DebugCanvas::onDrawImageLattice(const SkImage* image,
                                     const Lattice& lattice,
                                     const SkRect&  dst,
                                     const SkPaint* paint) {
    this->addDrawCommand(new DrawImageLatticeCommand(image, lattice, dst, paint));
}

void DebugCanvas::onDrawImageRect(const SkImage*    image,
                                  const SkRect*     src,
                                  const SkRect&     dst,
                                  const SkPaint*    paint,
                                  SrcRectConstraint constraint) {
    if (fnextDrawImageRectLayerId != -1 && fLayerManager) {
        // This drawImageRect command would have drawn the offscreen buffer for a layer.
        // On Android, we recorded an SkPicture of the commands that drew to the layer.
        // To render the layer as it would have looked on the frame this DebugCanvas draws, we need
        // to call fLayerManager->getLayerAsImage(id). This must be done just before
        // drawTo(command), since it depends on the index into the layer's commands
        // (managed by fLayerManager)
        // Instead of adding a DrawImageRectCommand, we need a deferred command, that when
        // executed, will call drawImageRect(fLayerManager->getLayerAsImage())
        this->addDrawCommand(new DrawImageRectLayerCommand(
            fLayerManager, fnextDrawImageRectLayerId, fFrame, src, dst, paint, constraint));
    } else {
        this->addDrawCommand(new DrawImageRectCommand(image, src, dst, paint, constraint));
    }
    // Reset expectation so next drawImageRect is not special.
    fnextDrawImageRectLayerId = -1;
}

void DebugCanvas::onDrawImageNine(const SkImage* image,
                                  const SkIRect& center,
                                  const SkRect&  dst,
                                  const SkPaint* paint) {
    this->addDrawCommand(new DrawImageNineCommand(image, center, dst, paint));
}

void DebugCanvas::onDrawOval(const SkRect& oval, const SkPaint& paint) {
    this->addDrawCommand(new DrawOvalCommand(oval, paint));
}

void DebugCanvas::onDrawArc(const SkRect&  oval,
                            SkScalar       startAngle,
                            SkScalar       sweepAngle,
                            bool           useCenter,
                            const SkPaint& paint) {
    this->addDrawCommand(new DrawArcCommand(oval, startAngle, sweepAngle, useCenter, paint));
}

void DebugCanvas::onDrawPaint(const SkPaint& paint) {
    this->addDrawCommand(new DrawPaintCommand(paint));
}

void DebugCanvas::onDrawBehind(const SkPaint& paint) {
    this->addDrawCommand(new DrawBehindCommand(paint));
}

void DebugCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    this->addDrawCommand(new DrawPathCommand(path, paint));
}

void DebugCanvas::onDrawRegion(const SkRegion& region, const SkPaint& paint) {
    this->addDrawCommand(new DrawRegionCommand(region, paint));
}

void DebugCanvas::onDrawPicture(const SkPicture* picture,
                                const SkMatrix*  matrix,
                                const SkPaint*   paint) {
    if (fnextDrawPictureLayerId != -1 && fLayerManager) {
        fLayerManager->storeSkPicture(fnextDrawPictureLayerId, fFrame, sk_ref_sp(picture),
           fnextDrawPictureDirtyRect);
    } else {
        this->addDrawCommand(new BeginDrawPictureCommand(picture, matrix, paint));
        SkAutoCanvasMatrixPaint acmp(this, matrix, paint, picture->cullRect());
        picture->playback(this);
        this->addDrawCommand(new EndDrawPictureCommand(SkToBool(matrix) || SkToBool(paint)));
    }
    fnextDrawPictureLayerId = -1;
}

void DebugCanvas::onDrawPoints(PointMode      mode,
                               size_t         count,
                               const SkPoint  pts[],
                               const SkPaint& paint) {
    this->addDrawCommand(new DrawPointsCommand(mode, count, pts, paint));
}

void DebugCanvas::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    // NOTE(chudy): Messing up when renamed to DrawRect... Why?
    addDrawCommand(new DrawRectCommand(rect, paint));
}

void DebugCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    this->addDrawCommand(new DrawRRectCommand(rrect, paint));
}

void DebugCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    this->addDrawCommand(new DrawDRRectCommand(outer, inner, paint));
}

void DebugCanvas::onDrawTextBlob(const SkTextBlob* blob,
                                 SkScalar          x,
                                 SkScalar          y,
                                 const SkPaint&    paint) {
    this->addDrawCommand(
            new DrawTextBlobCommand(sk_ref_sp(const_cast<SkTextBlob*>(blob)), x, y, paint));
}

void DebugCanvas::onDrawPatch(const SkPoint  cubics[12],
                              const SkColor  colors[4],
                              const SkPoint  texCoords[4],
                              SkBlendMode    bmode,
                              const SkPaint& paint) {
    this->addDrawCommand(new DrawPatchCommand(cubics, colors, texCoords, bmode, paint));
}

void DebugCanvas::onDrawVerticesObject(const SkVertices*      vertices,
                                       SkBlendMode            bmode,
                                       const SkPaint&         paint) {
    this->addDrawCommand(
            new DrawVerticesCommand(sk_ref_sp(const_cast<SkVertices*>(vertices)), bmode, paint));
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
            new DrawAtlasCommand(image, xform, tex, colors, count, bmode, cull, paint));
}

void DebugCanvas::onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) {
    this->addDrawCommand(new DrawShadowCommand(path, rec));
}

void DebugCanvas::onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) {
    this->addDrawCommand(new DrawDrawableCommand(drawable, matrix));
}

void DebugCanvas::onDrawEdgeAAQuad(const SkRect&    rect,
                                   const SkPoint    clip[4],
                                   QuadAAFlags      aa,
                                   const SkColor4f& color,
                                   SkBlendMode      mode) {
    this->addDrawCommand(new DrawEdgeAAQuadCommand(rect, clip, aa, color, mode));
}

void DebugCanvas::onDrawEdgeAAImageSet(const ImageSetEntry set[],
                                       int                 count,
                                       const SkPoint       dstClips[],
                                       const SkMatrix      preViewMatrices[],
                                       const SkPaint*      paint,
                                       SrcRectConstraint   constraint) {
    this->addDrawCommand(new DrawEdgeAAImageSetCommand(
            set, count, dstClips, preViewMatrices, paint, constraint));
}

void DebugCanvas::willRestore() {
    this->addDrawCommand(new RestoreCommand());
    this->INHERITED::willRestore();
}

void DebugCanvas::willSave() {
    this->addDrawCommand(new SaveCommand());
    this->INHERITED::willSave();
}

SkCanvas::SaveLayerStrategy DebugCanvas::getSaveLayerStrategy(const SaveLayerRec& rec) {
    this->addDrawCommand(new SaveLayerCommand(rec));
    (void)this->INHERITED::getSaveLayerStrategy(rec);
    // No need for a full layer.
    return kNoLayer_SaveLayerStrategy;
}

bool DebugCanvas::onDoSaveBehind(const SkRect* subset) {
    // TODO
    return false;
}

void DebugCanvas::didSetMatrix(const SkMatrix& matrix) {
    this->addDrawCommand(new SetMatrixCommand(matrix));
    this->INHERITED::didSetMatrix(matrix);
}

void DebugCanvas::toggleCommand(int index, bool toggle) {
    SkASSERT(index < fCommandVector.count());
    fCommandVector[index]->setVisible(toggle);
}
