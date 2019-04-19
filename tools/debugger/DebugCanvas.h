/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKDEBUGCANVAS_H_
#define SKDEBUGCANVAS_H_

#include "DrawCommand.h"
#include "SkCanvas.h"
#include "SkCanvasVirtualEnforcer.h"
#include "SkPath.h"
#include "SkPathOps.h"
#include "SkString.h"
#include "SkTArray.h"
#include "SkVertices.h"
#include "UrlDataManager.h"

class GrAuditTrail;
class SkNWayCanvas;
class SkPicture;

class DebugCanvas : public SkCanvasVirtualEnforcer<SkCanvas> {
public:
    DebugCanvas(int width, int height);

    DebugCanvas(SkIRect bounds);

    ~DebugCanvas() override;

    /**
     * Enable or disable overdraw visualization
     */
    void setOverdrawViz(bool overdrawViz);

    bool getOverdrawViz() const { return fOverdrawViz; }

    /**
     * Set the color of the clip visualization. An alpha of zero renders the clip invisible.
     */
    void setClipVizColor(SkColor clipVizColor) { this->fClipVizColor = clipVizColor; }

    void setDrawGpuOpBounds(bool drawGpuOpBounds) { fDrawGpuOpBounds = drawGpuOpBounds; }

    bool getDrawGpuOpBounds() const { return fDrawGpuOpBounds; }

    /**
        Executes all draw calls to the canvas.
        @param canvas  The canvas being drawn to
     */
    void draw(SkCanvas* canvas);

    /**
        Executes the draw calls up to the specified index.
        @param canvas  The canvas being drawn to
        @param index  The index of the final command being executed
        @param m an optional Mth gpu op to highlight, or -1
     */
    void drawTo(SkCanvas* canvas, int index, int m = -1);

    /**
        Returns the most recently calculated transformation matrix
     */
    const SkMatrix& getCurrentMatrix() { return fMatrix; }

    /**
        Returns the most recently calculated clip
     */
    const SkIRect& getCurrentClip() { return fClip; }

    /**
        Removes the command at the specified index
        @param index  The index of the command to delete
     */
    void deleteDrawCommandAt(int index);

    /**
        Returns the draw command at the given index.
        @param index  The index of the command
     */
    DrawCommand* getDrawCommandAt(int index);

    /**
        Returns length of draw command vector.
     */
    int getSize() const { return fCommandVector.count(); }

    /**
        Toggles the visibility / execution of the draw command at index i with
        the value of toggle.
     */
    void toggleCommand(int index, bool toggle);

    /**
        Returns a JSON object representing up to the Nth draw, where N is less than
        DebugCanvas::getSize(). The encoder may use the UrlDataManager to store binary data such
        as images, referring to them via URLs embedded in the JSON.
     */
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager, int n, SkCanvas*);

    void toJSONOpList(SkJSONWriter& writer, int n, SkCanvas*);

    void detachCommands(SkTDArray<DrawCommand*>* dst) { fCommandVector.swap(*dst); }

protected:
    void              willSave() override;
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec&) override;
    bool              onDoSaveBehind(const SkRect*) override;
    void              willRestore() override;

    void didConcat(const SkMatrix&) override;

    void didSetMatrix(const SkMatrix&) override;

    void onDrawAnnotation(const SkRect&, const char[], SkData*) override;
    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override;
    void onDrawTextBlob(const SkTextBlob* blob,
                        SkScalar          x,
                        SkScalar          y,
                        const SkPaint&    paint) override;

    void onDrawPatch(const SkPoint cubics[12],
                     const SkColor colors[4],
                     const SkPoint texCoords[4],
                     SkBlendMode,
                     const SkPaint& paint) override;
    void onDrawPaint(const SkPaint&) override;
    void onDrawBehind(const SkPaint&) override;

    void onDrawRect(const SkRect&, const SkPaint&) override;
    void onDrawOval(const SkRect&, const SkPaint&) override;
    void onDrawArc(const SkRect&, SkScalar, SkScalar, bool, const SkPaint&) override;
    void onDrawRRect(const SkRRect&, const SkPaint&) override;
    void onDrawPoints(PointMode, size_t count, const SkPoint pts[], const SkPaint&) override;
    void onDrawVerticesObject(const SkVertices*,
                              const SkVertices::Bone bones[],
                              int                    boneCount,
                              SkBlendMode,
                              const SkPaint&) override;
    void onDrawPath(const SkPath&, const SkPaint&) override;
    void onDrawRegion(const SkRegion&, const SkPaint&) override;
    void onDrawBitmap(const SkBitmap&, SkScalar left, SkScalar top, const SkPaint*) override;
    void onDrawBitmapLattice(const SkBitmap&,
                             const Lattice&,
                             const SkRect&,
                             const SkPaint*) override;
    void onDrawBitmapRect(const SkBitmap&,
                          const SkRect* src,
                          const SkRect& dst,
                          const SkPaint*,
                          SrcRectConstraint) override;
    void onDrawImage(const SkImage*, SkScalar left, SkScalar top, const SkPaint*) override;
    void onDrawImageLattice(const SkImage* image,
                            const Lattice& lattice,
                            const SkRect&  dst,
                            const SkPaint* paint) override;
    void onDrawImageRect(const SkImage*,
                         const SkRect* src,
                         const SkRect& dst,
                         const SkPaint*,
                         SrcRectConstraint) override;
    void onDrawBitmapNine(const SkBitmap&,
                          const SkIRect& center,
                          const SkRect&  dst,
                          const SkPaint*) override;
    void onDrawImageNine(const SkImage*,
                         const SkIRect& center,
                         const SkRect&  dst,
                         const SkPaint*) override;
    void onDrawAtlas(const SkImage*,
                     const SkRSXform[],
                     const SkRect[],
                     const SkColor[],
                     int,
                     SkBlendMode,
                     const SkRect*,
                     const SkPaint*) override;
    void onClipRect(const SkRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipRRect(const SkRRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipPath(const SkPath&, SkClipOp, ClipEdgeStyle) override;
    void onClipRegion(const SkRegion& region, SkClipOp) override;
    void onDrawShadowRec(const SkPath&, const SkDrawShadowRec&) override;

    void onDrawDrawable(SkDrawable*, const SkMatrix*) override;
    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override;

    void onDrawEdgeAAQuad(const SkRect&,
                          const SkPoint[4],
                          QuadAAFlags,
                          SkColor,
                          SkBlendMode) override;
    void onDrawEdgeAAImageSet(const ImageSetEntry[],
                              int count,
                              const SkPoint[],
                              const SkMatrix[],
                              const SkPaint*,
                              SrcRectConstraint) override;

private:
    SkTDArray<DrawCommand*> fCommandVector;
    SkMatrix                fMatrix;
    SkIRect                 fClip;

    bool    fOverdrawViz;
    SkColor fClipVizColor;
    bool    fDrawGpuOpBounds;

    /**
        Adds the command to the class' vector of commands.
        @param command  The draw command for execution
     */
    void addDrawCommand(DrawCommand* command);

    GrAuditTrail* getAuditTrail(SkCanvas*);

    void drawAndCollectOps(int n, SkCanvas*);
    void cleanupAuditTrail(SkCanvas*);

    typedef SkCanvasVirtualEnforcer<SkCanvas> INHERITED;
};

#endif
