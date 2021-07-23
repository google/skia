/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKDEBUGCANVAS_H_
#define SKDEBUGCANVAS_H_

#include "include/core/SkCanvas.h"
#include "include/core/SkCanvasVirtualEnforcer.h"
#include "include/core/SkPath.h"
#include "include/core/SkString.h"
#include "include/core/SkVertices.h"
#include "include/pathops/SkPathOps.h"
#include "include/private/SkTArray.h"
#include "tools/UrlDataManager.h"
#include "tools/debugger/DebugLayerManager.h"
#include "tools/debugger/DrawCommand.h"

#include <map>
#include <vector>

class GrAuditTrail;
class SkNWayCanvas;
class SkPicture;
class DebugLayerManager;

class DebugCanvas : public SkCanvasVirtualEnforcer<SkCanvas> {
public:
    DebugCanvas(int width, int height);

    DebugCanvas(SkIRect bounds);

    ~DebugCanvas() override;

    /**
     * Provide a DebugLayerManager for mskp files containing layer information
     * when set this DebugCanvas will attempt to parse layer info from annotations.
     * it will store layer pictures to the layer manager, and interpret some drawImageRects
     * as layer draws, deferring to the layer manager for images.
     * Provide a frame number that will be passed to all layer manager functions to identify this
     * DebugCanvas.
     *
     * Used only in wasm debugger animations.
     */
    void setLayerManagerAndFrame(DebugLayerManager* lm, int frame) {
        fLayerManager = lm;
        fFrame = frame;
    }

    /**
     * Enable or disable overdraw visualization
     */
    void setOverdrawViz(bool overdrawViz);

    bool getOverdrawViz() const { return fOverdrawViz; }

    /**
     * Set the color of the clip visualization. An alpha of zero renders the clip invisible.
     */
    void setClipVizColor(SkColor clipVizColor) { this->fClipVizColor = clipVizColor; }

    void setAndroidClipViz(bool enable) { this->fShowAndroidClip = enable; }

    void setOriginVisible(bool enable) { this->fShowOrigin = enable; }

    void setDrawGpuOpBounds(bool drawGpuOpBounds) { fDrawGpuOpBounds = drawGpuOpBounds; }

    bool getDrawGpuOpBounds() const { return fDrawGpuOpBounds; }

    /**
        Executes all draw calls to the canvas.
        @param canvas  The canvas being drawn to
     */
    void draw(SkCanvas* canvas);

    /**
        Executes the draw calls up to the specified index.
        Does not clear the canvas to transparent black first,
        if needed, caller should do that first.
        @param canvas  The canvas being drawn to
        @param index  The index of the final command being executed
        @param m an optional Mth gpu op to highlight, or -1
     */
    void drawTo(SkCanvas* canvas, int index, int m = -1);

    /**
        Returns the most recently calculated transformation matrix
     */
    const SkM44& getCurrentMatrix() { return fMatrix; }

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
    DrawCommand* getDrawCommandAt(int index) const;

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
        Returns a JSON object representing all commands in the picture.
        The encoder may use the UrlDataManager to store binary data such
        as images, referring to them via URLs embedded in the JSON.
     */
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager, SkCanvas*);

    void toJSONOpsTask(SkJSONWriter& writer, SkCanvas*);

    void detachCommands(SkTDArray<DrawCommand*>* dst) { fCommandVector.swap(*dst); }

    /**
        Returns a map from image IDs to command indices where they are used.
     */
    std::map<int, std::vector<int>> getImageIdToCommandMap(UrlDataManager& udm) const;

protected:
    void              willSave() override;
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec&) override;
    bool              onDoSaveBehind(const SkRect*) override;
    void              willRestore() override;

    void didConcat44(const SkM44&) override;
    void didSetM44(const SkM44&) override;
    void didScale(SkScalar, SkScalar) override;
    void didTranslate(SkScalar, SkScalar) override;

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
    void onDrawVerticesObject(const SkVertices*, SkBlendMode, const SkPaint&) override;
    void onDrawPath(const SkPath&, const SkPaint&) override;
    void onDrawRegion(const SkRegion&, const SkPaint&) override;

    void onDrawImage2(const SkImage*, SkScalar, SkScalar, const SkSamplingOptions&,
                      const SkPaint*) override;
    void onDrawImageRect2(const SkImage*, const SkRect&, const SkRect&, const SkSamplingOptions&,
                          const SkPaint*, SrcRectConstraint) override;
    void onDrawImageLattice2(const SkImage*, const Lattice&, const SkRect&, SkFilterMode,
                             const SkPaint*) override;
    void onDrawAtlas2(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[], int,
                     SkBlendMode, const SkSamplingOptions&, const SkRect*, const SkPaint*) override;

    void onClipRect(const SkRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipRRect(const SkRRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipPath(const SkPath&, SkClipOp, ClipEdgeStyle) override;
    void onClipShader(sk_sp<SkShader>, SkClipOp) override;
    void onClipRegion(const SkRegion& region, SkClipOp) override;
    void onResetClip() override;

    void onDrawShadowRec(const SkPath&, const SkDrawShadowRec&) override;
    void onDrawDrawable(SkDrawable*, const SkMatrix*) override;
    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override;

    void onDrawEdgeAAQuad(const SkRect&,
                          const SkPoint[4],
                          QuadAAFlags,
                          const SkColor4f&,
                          SkBlendMode) override;
    void onDrawEdgeAAImageSet2(const ImageSetEntry[],
                               int count,
                               const SkPoint[],
                               const SkMatrix[],
                               const SkSamplingOptions&,
                               const SkPaint*,
                               SrcRectConstraint) override;

private:
    SkTDArray<DrawCommand*> fCommandVector;
    SkM44                   fMatrix;
    SkIRect                 fClip;

    bool    fOverdrawViz = false;
    SkColor fClipVizColor;
    bool    fDrawGpuOpBounds = false;
    bool    fShowAndroidClip = false;
    bool    fShowOrigin = false;

    // When not negative, indicates the render node id of the layer represented by the next
    // drawPicture call.
    int         fnextDrawPictureLayerId = -1;
    int         fnextDrawImageRectLayerId = -1;
    SkIRect     fnextDrawPictureDirtyRect;
    // may be null, in which case layer annotations are ignored.
    DebugLayerManager* fLayerManager = nullptr;
    // May be set when DebugCanvas is used in playing back an animation.
    // Only used for passing to fLayerManager to identify itself.
    int fFrame = -1;
    SkRect fAndroidClip = SkRect::MakeEmpty();

    /**
        Adds the command to the class' vector of commands.
        @param command  The draw command for execution
     */
    void addDrawCommand(DrawCommand* command);

#if SK_GPU_V1
    GrAuditTrail* getAuditTrail(SkCanvas*);
    void drawAndCollectOps(SkCanvas*);
    void cleanupAuditTrail(GrAuditTrail*);
#endif

    using INHERITED = SkCanvasVirtualEnforcer<SkCanvas>;
};

#endif
