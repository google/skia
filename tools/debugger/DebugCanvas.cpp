/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/debugger/DebugCanvas.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkData.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkVertices.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "include/utils/SkPaintFilterCanvas.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkStringUtils.h"
#include "src/gpu/ganesh/GrCanvas.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/utils/SkJSONWriter.h"
#include "tools/debugger/DebugLayerManager.h"
#include "tools/debugger/DrawCommand.h"

#include <cstring>
#include <string>
#include <utility>

class SkDrawable;
class SkImage;
class SkRRect;
class SkRegion;
class UrlDataManager;
struct SkDrawShadowRec;

#if defined(SK_GANESH)
#include "src/gpu/ganesh/GrAuditTrail.h"
#endif

using namespace skia_private;

#define SKDEBUGCANVAS_VERSION 1
#define SKDEBUGCANVAS_ATTRIBUTE_VERSION "version"
#define SKDEBUGCANVAS_ATTRIBUTE_COMMANDS "commands"
#define SKDEBUGCANVAS_ATTRIBUTE_AUDITTRAIL "auditTrail"

namespace {
    // Constants used in Annotations by Android for keeping track of layers
    static constexpr char kOffscreenLayerDraw[] = "OffscreenLayerDraw";
    static constexpr char kSurfaceID[] = "SurfaceID";
    static constexpr char kAndroidClip[] = "AndroidDeviceClipRestriction";

    static SkPath arrowHead = SkPath::Polygon({
        { 0,   0},
        { 6, -15},
        { 0,  -12},
        {-6, -15},
    }, true);

    void drawArrow(SkCanvas* canvas, const SkPoint& a, const SkPoint& b, const SkPaint& paint) {
        canvas->translate(0.5, 0.5);
        canvas->drawLine(a, b, paint);
        canvas->save();
        canvas->translate(b.fX, b.fY);
        SkScalar angle = SkScalarATan2((b.fY - a.fY), b.fX - a.fX);
        canvas->rotate(angle * 180 / SK_ScalarPI - 90);
        // arrow head
        canvas->drawPath(arrowHead, paint);
        canvas->restore();
        canvas->restore();
    }
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

    using INHERITED = SkPaintFilterCanvas;
};

DebugCanvas::DebugCanvas(int width, int height)
        : INHERITED(width, height)
        , fOverdrawViz(false)
        , fClipVizColor(SK_ColorTRANSPARENT)
        , fDrawGpuOpBounds(false)
        , fShowAndroidClip(false)
        , fShowOrigin(false)
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
    this->INHERITED::onClipRect(large, SkClipOp::kIntersect, kHard_ClipEdgeStyle);
}

DebugCanvas::DebugCanvas(SkIRect bounds)
        : DebugCanvas(bounds.width(), bounds.height()) {}

DebugCanvas::~DebugCanvas() {
    for (DrawCommand* p : fCommandVector) {
        delete p;
    }
    fCommandVector.reset();
}

void DebugCanvas::addDrawCommand(DrawCommand* command) { fCommandVector.push_back(command); }

void DebugCanvas::draw(SkCanvas* canvas) {
    if (!fCommandVector.empty()) {
        this->drawTo(canvas, fCommandVector.size() - 1);
    }
}

void DebugCanvas::drawTo(SkCanvas* originalCanvas, int index, int m) {
    SkASSERT(!fCommandVector.empty());
    SkASSERT(index < fCommandVector.size());

    int saveCount = originalCanvas->save();

    originalCanvas->resetMatrix();
    SkCanvasPriv::ResetClip(originalCanvas);

    DebugPaintFilterCanvas filterCanvas(originalCanvas);
    SkCanvas* finalCanvas = fOverdrawViz ? &filterCanvas : originalCanvas;

#if defined(SK_GANESH)
    auto dContext = GrAsDirectContext(finalCanvas->recordingContext());

    // If we have a GPU backend we can also visualize the op information
    GrAuditTrail* at = nullptr;
    if (fDrawGpuOpBounds || m != -1) {
        // The audit trail must be obtained from the original canvas.
        at = this->getAuditTrail(originalCanvas);
    }
#endif

    for (int i = 0; i <= index; i++) {
#if defined(SK_GANESH)
        GrAuditTrail::AutoCollectOps* acb = nullptr;
        if (at) {
            // We need to flush any pending operations, or they might combine with commands below.
            // Previous operations were not registered with the audit trail when they were
            // created, so if we allow them to combine, the audit trail will fail to find them.
            if (dContext) {
                dContext->flush();
            }
            acb = new GrAuditTrail::AutoCollectOps(at, i);
        }
#endif
        if (fCommandVector[i]->isVisible()) {
            fCommandVector[i]->execute(finalCanvas);
        }
#if defined(SK_GANESH)
        if (at && acb) {
            delete acb;
        }
#endif
    }

    if (SkColorGetA(fClipVizColor) != 0) {
        finalCanvas->save();
        SkPaint clipPaint;
        clipPaint.setColor(fClipVizColor);
        finalCanvas->drawPaint(clipPaint);
        finalCanvas->restore();
    }

    fMatrix = finalCanvas->getLocalToDevice();
    fClip   = finalCanvas->getDeviceClipBounds();
    if (fShowOrigin) {
        const SkPaint originXPaint = SkPaint({1.0, 0, 0, 1.0});
        const SkPaint originYPaint = SkPaint({0, 1.0, 0, 1.0});
        // Draw an origin cross at the origin before restoring to assist in visualizing the
        // current matrix.
        drawArrow(finalCanvas, {-50, 0}, {50, 0}, originXPaint);
        drawArrow(finalCanvas, {0, -50}, {0, 50}, originYPaint);
    }
    finalCanvas->restoreToCount(saveCount);

    if (fShowAndroidClip) {
        // Draw visualization of android device clip restriction
        SkPaint androidClipPaint;
        androidClipPaint.setARGB(80, 255, 100, 0);
        finalCanvas->drawRect(fAndroidClip, androidClipPaint);
    }

#if defined(SK_GANESH)
    // draw any ops if required and issue a full reset onto GrAuditTrail
    if (at) {
        // just in case there is global reordering, we flush the canvas before querying
        // GrAuditTrail
        GrAuditTrail::AutoEnable ae(at);
        if (dContext) {
            dContext->flush();
        }

        // we pick three colorblind-safe colors, 75% alpha
        static const SkColor kTotalBounds     = SkColorSetARGB(0xC0, 0x6A, 0x3D, 0x9A);
        static const SkColor kCommandOpBounds = SkColorSetARGB(0xC0, 0xE3, 0x1A, 0x1C);
        static const SkColor kOtherOpBounds   = SkColorSetARGB(0xC0, 0xFF, 0x7F, 0x00);

        // get the render target of the top device (from the original canvas) so we can ignore ops
        // drawn offscreen
        GrRenderTargetProxy* rtp = skgpu::ganesh::TopDeviceTargetProxy(originalCanvas);
        GrSurfaceProxy::UniqueID proxyID = rtp->uniqueID();

        // get the bounding boxes to draw
        TArray<GrAuditTrail::OpInfo> childrenBounds;
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
        for (int i = 0; i < childrenBounds.size(); i++) {
            if (childrenBounds[i].fProxyUniqueID != proxyID) {
                // offscreen draw, ignore for now
                continue;
            }
            paint.setColor(kTotalBounds);
            finalCanvas->drawRect(childrenBounds[i].fBounds, paint);
            for (int j = 0; j < childrenBounds[i].fOps.size(); j++) {
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
        this->cleanupAuditTrail(at);
    }
#endif
}

void DebugCanvas::deleteDrawCommandAt(int index) {
    SkASSERT(index < fCommandVector.size());
    delete fCommandVector[index];
    fCommandVector.remove(index);
}

DrawCommand* DebugCanvas::getDrawCommandAt(int index) const {
    SkASSERT(index < fCommandVector.size());
    return fCommandVector[index];
}

#if defined(SK_GANESH)
GrAuditTrail* DebugCanvas::getAuditTrail(SkCanvas* canvas) {
    GrAuditTrail* at  = nullptr;
    auto ctx = canvas->recordingContext();
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

            auto dContext = GrAsDirectContext(canvas->recordingContext());
            if (dContext) {
                dContext->flush();
            }
        }
    }
}

void DebugCanvas::cleanupAuditTrail(GrAuditTrail* at) {
    if (at) {
        GrAuditTrail::AutoEnable ae(at);
        at->fullReset();
    }
}
#endif // defined(SK_GANESH)

void DebugCanvas::toJSON(SkJSONWriter&   writer,
                         UrlDataManager& urlDataManager,
                         SkCanvas*       canvas) {
#if defined(SK_GANESH)
    this->drawAndCollectOps(canvas);

    // now collect json
    GrAuditTrail* at = this->getAuditTrail(canvas);
#endif
    writer.appendS32(SKDEBUGCANVAS_ATTRIBUTE_VERSION, SKDEBUGCANVAS_VERSION);
    writer.beginArray(SKDEBUGCANVAS_ATTRIBUTE_COMMANDS);

    for (int i = 0; i < this->getSize(); i++) {
        writer.beginObject();  // command
        this->getDrawCommandAt(i)->toJSON(writer, urlDataManager);

#if defined(SK_GANESH)
        if (at && at->isEnabled()) {
            writer.appendName(SKDEBUGCANVAS_ATTRIBUTE_AUDITTRAIL);
            at->toJson(writer, i);
        }
#endif
        writer.endObject();  // command
    }

    writer.endArray();  // commands
#if defined(SK_GANESH)
    this->cleanupAuditTrail(at);
#endif
}

void DebugCanvas::toJSONOpsTask(SkJSONWriter& writer, SkCanvas* canvas) {
#if defined(SK_GANESH)
    this->drawAndCollectOps(canvas);

    GrAuditTrail* at = this->getAuditTrail(canvas);
    if (at) {
        GrAuditTrail::AutoManageOpsTask enable(at);
        at->toJson(writer);
        this->cleanupAuditTrail(at);
        return;
    }
#endif

    writer.beginObject();
    writer.endObject();
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

void DebugCanvas::onResetClip() {
    this->addDrawCommand(new ResetClipCommand());
}

void DebugCanvas::didConcat44(const SkM44& m) {
    this->addDrawCommand(new Concat44Command(m));
    this->INHERITED::didConcat44(m);
}

void DebugCanvas::didScale(SkScalar x, SkScalar y) {
    this->didConcat44(SkM44::Scale(x, y));
}

void DebugCanvas::didTranslate(SkScalar x, SkScalar y) {
    this->didConcat44(SkM44::Translate(x, y));
}

void DebugCanvas::onDrawAnnotation(const SkRect& rect, const char key[], SkData* value) {
    // Parse layer-releated annotations added in SkiaPipeline.cpp and RenderNodeDrawable.cpp
    // the format of the annotations is <Indicator|RenderNodeId>
    TArray<SkString> tokens;
    SkStrSplit(key, "|", kStrict_SkStrSplitMode, &tokens);
    if (tokens.size() == 2) {
        if (tokens[0].equals(kOffscreenLayerDraw)) {
            // Indicates that the next drawPicture command contains the SkPicture to render the
            // node at this id in an offscreen buffer.
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

void DebugCanvas::onDrawImage2(const SkImage*           image,
                               SkScalar                 left,
                               SkScalar                 top,
                               const SkSamplingOptions& sampling,
                               const SkPaint*           paint) {
    this->addDrawCommand(new DrawImageCommand(image, left, top, sampling, paint));
}

void DebugCanvas::onDrawImageLattice2(const SkImage* image,
                                      const Lattice& lattice,
                                      const SkRect&  dst,
                                      SkFilterMode filter,   // todo
                                      const SkPaint* paint) {
    this->addDrawCommand(new DrawImageLatticeCommand(image, lattice, dst, filter, paint));
}

void DebugCanvas::onDrawImageRect2(const SkImage*           image,
                                   const SkRect&            src,
                                   const SkRect&            dst,
                                   const SkSamplingOptions& sampling,
                                   const SkPaint*           paint,
                                   SrcRectConstraint        constraint) {
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
            fLayerManager, fnextDrawImageRectLayerId, fFrame, src, dst, sampling,
                                                           paint, constraint));
    } else {
        this->addDrawCommand(new DrawImageRectCommand(image, src, dst, sampling, paint, constraint));
    }
    // Reset expectation so next drawImageRect is not special.
    fnextDrawImageRectLayerId = -1;
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

void DebugCanvas::onDrawAtlas2(const SkImage*           image,
                               const SkRSXform          xform[],
                               const SkRect             tex[],
                               const SkColor            colors[],
                               int                      count,
                               SkBlendMode              bmode,
                               const SkSamplingOptions& sampling,
                               const SkRect*            cull,
                               const SkPaint*           paint) {
    this->addDrawCommand(
            new DrawAtlasCommand(image, xform, tex, colors, count, bmode, sampling, cull, paint));
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

void DebugCanvas::onDrawEdgeAAImageSet2(const ImageSetEntry set[],
                                        int                 count,
                                        const SkPoint       dstClips[],
                                        const SkMatrix      preViewMatrices[],
                                        const SkSamplingOptions& sampling,
                                        const SkPaint*      paint,
                                        SrcRectConstraint   constraint) {
    this->addDrawCommand(new DrawEdgeAAImageSetCommand(
            set, count, dstClips, preViewMatrices, sampling, paint, constraint));
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

void DebugCanvas::didSetM44(const SkM44& matrix) {
    this->addDrawCommand(new SetM44Command(matrix));
    this->INHERITED::didSetM44(matrix);
}

void DebugCanvas::toggleCommand(int index, bool toggle) {
    SkASSERT(index < fCommandVector.size());
    fCommandVector[index]->setVisible(toggle);
}

std::map<int, std::vector<int>> DebugCanvas::getImageIdToCommandMap(UrlDataManager& udm) const {
    // map from image ids to list of commands that reference them.
    std::map<int, std::vector<int>> m;

    for (int i = 0; i < this->getSize(); i++) {
        const DrawCommand* command = this->getDrawCommandAt(i);
        int imageIndex = -1;
        // this is not an exaustive list of where images can be used, they show up in paints too.
        switch (command->getOpType()) {
            case DrawCommand::OpType::kDrawImage_OpType: {
                imageIndex = static_cast<const DrawImageCommand*>(command)->imageId(udm);
                break;
            }
            case DrawCommand::OpType::kDrawImageRect_OpType: {
                imageIndex = static_cast<const DrawImageRectCommand*>(command)->imageId(udm);
                break;
            }
            case DrawCommand::OpType::kDrawImageLattice_OpType: {
                imageIndex = static_cast<const DrawImageLatticeCommand*>(command)->imageId(udm);
                break;
            }
            default: break;
        }
        if (imageIndex >= 0) {
            m[imageIndex].push_back(i);
        }
    }
    return m;
}
