/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SKDEBUGCANVAS_H_
#define SKDEBUGCANVAS_H_

#include "SkCanvas.h"
#include "SkDrawCommand.h"
#include "SkPath.h"
#include "SkPathOps.h"
#include "SkPicture.h"
#include "SkString.h"
#include "SkTArray.h"
#include "SkVertices.h"
#include "UrlDataManager.h"

class GrAuditTrail;
class SkNWayCanvas;

class SkDebugCanvas : public SkCanvas {
public:
    SkDebugCanvas(int width, int height);

    ~SkDebugCanvas() override;

    void toggleFilter(bool toggle) { fFilter = toggle; }

    void setMegaVizMode(bool megaVizMode) { fMegaVizMode = megaVizMode; }

    bool getMegaVizMode() const { return fMegaVizMode; }

    /**
     * Enable or disable overdraw visualization
     */
    void setOverdrawViz(bool overdrawViz);

    bool getOverdrawViz() const { return fOverdrawViz; }

    /**
     * Set the color of the clip visualization. An alpha of zero renders the clip invisible.
     */
    void setClipVizColor(SkColor clipVizColor) { this->fClipVizColor = clipVizColor; }

    SkColor getClipVizColor() const { return fClipVizColor; }

    void setDrawGpuOpBounds(bool drawGpuOpBounds) { fDrawGpuOpBounds = drawGpuOpBounds; }

    bool getDrawGpuOpBounds() const { return fDrawGpuOpBounds; }

    bool getAllowSimplifyClip() const { return fAllowSimplifyClip; }

    void setPicture(SkPicture *picture) { fPicture = picture; }

    /**
     * Enable or disable texure filtering override
     */
    void overrideTexFiltering(bool overrideTexFiltering, SkFilterQuality);

    /**
        Executes all draw calls to the canvas.
        @param canvas  The canvas being drawn to
     */
    void draw(SkCanvas *canvas);

    /**
        Executes the draw calls up to the specified index.
        @param canvas  The canvas being drawn to
        @param index  The index of the final command being executed
        @param m an optional Mth gpu op to highlight, or -1
     */
    void drawTo(SkCanvas *canvas, int index, int m = -1);

    /**
        Returns the most recently calculated transformation matrix
     */
    const SkMatrix &getCurrentMatrix() {
        return fMatrix;
    }

    /**
        Returns the most recently calculated clip
     */
    const SkIRect &getCurrentClip() {
        return fClip;
    }

    /**
        Returns the index of the last draw command to write to the pixel at (x,y)
     */
    int getCommandAtPoint(int x, int y, int index);

    /**
        Removes the command at the specified index
        @param index  The index of the command to delete
     */
    void deleteDrawCommandAt(int index);

    /**
        Returns the draw command at the given index.
        @param index  The index of the command
     */
    SkDrawCommand *getDrawCommandAt(int index);

    /**
        Sets the draw command for a given index.
        @param index  The index to overwrite
        @param command The new command
     */
    void setDrawCommandAt(int index, SkDrawCommand *command);

    /**
        Returns information about the command at the given index.
        @param index  The index of the command
     */
    const SkTDArray<SkString *> *getCommandInfo(int index) const;

    /**
        Returns the visibility of the command at the given index.
        @param index  The index of the command
     */
    bool getDrawCommandVisibilityAt(int index);

    /**
        Returns the vector of draw commands
     */
    SK_ATTR_DEPRECATED("please use getDrawCommandAt and getSize instead")
    const SkTDArray<SkDrawCommand *> &getDrawCommands() const;

    /**
        Returns the vector of draw commands. Do not use this entry
        point - it is going away!
     */
    SkTDArray<SkDrawCommand *> &getDrawCommands();

    /**
        Returns length of draw command vector.
     */
    int getSize() const {
        return fCommandVector.count();
    }

    /**
        Toggles the visibility / execution of the draw command at index i with
        the value of toggle.
     */
    void toggleCommand(int index, bool toggle);

    void setUserMatrix(SkMatrix matrix) {
        fUserMatrix = matrix;
    }

    SkString clipStackData() const { return fClipStackData; }

    /**
        Returns a JSON object representing up to the Nth draw, where N is less than
        SkDebugCanvas::getSize(). The encoder may use the UrlDataManager to store binary data such
        as images, referring to them via URLs embedded in the JSON.
     */
    Json::Value toJSON(UrlDataManager &urlDataManager, int n, SkCanvas *);

    Json::Value toJSONOpList(int n, SkCanvas*);

    ////////////////////////////////////////////////////////////////////////////////
    // Inherited from SkCanvas
    ////////////////////////////////////////////////////////////////////////////////

    static const int kVizImageHeight = 256;
    static const int kVizImageWidth = 256;

    bool isClipEmpty() const override { return false; }

    bool isClipRect() const override { return true; }

    SkRect onGetLocalClipBounds() const override {
        return SkRect::MakeIWH(this->imageInfo().width(), this->imageInfo().height());
    }

    SkIRect onGetDeviceClipBounds() const override {
        return SkIRect::MakeWH(this->imageInfo().width(), this->imageInfo().height());
    }

protected:
    void willSave() override;

    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec &) override;

    void willRestore() override;

    void didConcat(const SkMatrix &) override;

    void didSetMatrix(const SkMatrix &) override;

    void onDrawAnnotation(const SkRect&, const char[], SkData*) override;
    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override;
    void onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                    const SkPaint&) override;
    void onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                       const SkPaint&) override;
    void onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                        SkScalar constY, const SkPaint&) override;
    void onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                          const SkMatrix* matrix, const SkPaint&) override;
    void onDrawTextRSXform(const void* text, size_t byteLength, const SkRSXform[], const SkRect*,
                           const SkPaint&) override;
    void onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                        const SkPaint& paint) override;

    void onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                     const SkPoint texCoords[4], SkBlendMode, const SkPaint& paint) override;
    void onDrawPaint(const SkPaint&) override;

    void onDrawRect(const SkRect&, const SkPaint&) override;
    void onDrawOval(const SkRect&, const SkPaint&) override;
    void onDrawArc(const SkRect&, SkScalar, SkScalar, bool, const SkPaint&) override;
    void onDrawRRect(const SkRRect&, const SkPaint&) override;
    void onDrawPoints(PointMode, size_t count, const SkPoint pts[], const SkPaint&) override;
    void onDrawVerticesObject(const SkVertices*, SkBlendMode, const SkPaint&) override;
    void onDrawPath(const SkPath&, const SkPaint&) override;
    void onDrawBitmap(const SkBitmap&, SkScalar left, SkScalar top, const SkPaint*) override;
    void onDrawBitmapRect(const SkBitmap&, const SkRect* src, const SkRect& dst, const SkPaint*,
                          SrcRectConstraint) override;
    void onDrawImage(const SkImage*, SkScalar left, SkScalar top, const SkPaint*) override;
    void onDrawImageLattice(const SkImage* image, const Lattice& lattice,
                            const SkRect& dst, const SkPaint* paint) override;
    void onDrawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                         const SkPaint*, SrcRectConstraint) override;
    void onDrawBitmapNine(const SkBitmap&, const SkIRect& center, const SkRect& dst,
                          const SkPaint*) override;
    void onClipRect(const SkRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipRRect(const SkRRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipPath(const SkPath&, SkClipOp, ClipEdgeStyle) override;
    void onClipRegion(const SkRegion& region, SkClipOp) override;

    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override;

    void markActiveCommands(int index);

private:
    SkTDArray<SkDrawCommand*> fCommandVector;
    SkPicture* fPicture;
    bool fFilter;
    bool fMegaVizMode;
    SkMatrix fUserMatrix;
    SkMatrix fMatrix;
    SkIRect fClip;

    SkString fClipStackData;
    bool fCalledAddStackData;
    SkPath fSaveDevPath;

    bool fOverdrawViz;
    bool fOverrideFilterQuality;
    SkFilterQuality fFilterQuality;
    SkColor fClipVizColor;
    bool fDrawGpuOpBounds;

    /**
        The active saveLayer commands at a given point in the renderering.
        Only used when "mega" visualization is enabled.
    */
    SkTDArray<SkDrawCommand*> fActiveLayers;

    /**
        Adds the command to the classes vector of commands.
        @param command  The draw command for execution
     */
    void addDrawCommand(SkDrawCommand* command);

    /**
        Applies any panning and zooming the user has specified before
        drawing anything else into the canvas.
     */
    void applyUserTransform(SkCanvas* canvas);

    void resetClipStackData() { fClipStackData.reset(); fCalledAddStackData = false; }

    void addClipStackData(const SkPath& devPath, const SkPath& operand, SkClipOp elementOp);
    void addPathData(const SkPath& path, const char* pathName);
    bool lastClipStackData(const SkPath& devPath);
    void outputConicPoints(const SkPoint* pts, SkScalar weight);
    void outputPoints(const SkPoint* pts, int count);
    void outputPointsCommon(const SkPoint* pts, int count);
    void outputScalar(SkScalar num);

    GrAuditTrail* getAuditTrail(SkCanvas*);

    void drawAndCollectOps(int n, SkCanvas*);
    void cleanupAuditTrail(SkCanvas*);

    typedef SkCanvas INHERITED;
};

#endif
