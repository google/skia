
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
#include "SkPicture.h"
#include "SkTArray.h"
#include "SkString.h"

class SkTexOverrideFilter;

class SK_API SkDebugCanvas : public SkCanvas {
public:
    SkDebugCanvas(int width, int height);
    virtual ~SkDebugCanvas();

    void toggleFilter(bool toggle) { fFilter = toggle; }

    void setMegaVizMode(bool megaVizMode) { fMegaVizMode = megaVizMode; }

    /**
     * Enable or disable overdraw visualization
     */
    void setOverdrawViz(bool overdrawViz) { fOverdrawViz = overdrawViz; }

    /**
     * Enable or disable texure filtering override
     */
    void overrideTexFiltering(bool overrideTexFiltering, SkPaint::FilterLevel level);

    /**
        Executes all draw calls to the canvas.
        @param canvas  The canvas being drawn to
     */
    void draw(SkCanvas* canvas);

    /**
        Executes the draw calls up to the specified index.
        @param canvas  The canvas being drawn to
        @param index  The index of the final command being executed
     */
    void drawTo(SkCanvas* canvas, int index);

    /**
        Returns the most recently calculated transformation matrix
     */
    const SkMatrix& getCurrentMatrix() {
        return fMatrix;
    }

    /**
        Returns the most recently calculated clip
     */
    const SkIRect& getCurrentClip() {
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
    SkDrawCommand* getDrawCommandAt(int index);

    /**
        Sets the draw command for a given index.
        @param index  The index to overwrite
        @param command The new command
     */
    void setDrawCommandAt(int index, SkDrawCommand* command);

    /**
        Returns information about the command at the given index.
        @param index  The index of the command
     */
    SkTDArray<SkString*>* getCommandInfo(int index);

    /**
        Returns the visibility of the command at the given index.
        @param index  The index of the command
     */
    bool getDrawCommandVisibilityAt(int index);

    /**
        Returns the vector of draw commands
     */
    SK_ATTR_DEPRECATED("please use getDrawCommandAt and getSize instead")
    const SkTDArray<SkDrawCommand*>& getDrawCommands() const;

    /**
        Returns the vector of draw commands. Do not use this entry
        point - it is going away!
     */
    SkTDArray<SkDrawCommand*>& getDrawCommands();

    /**
     * Returns the string vector of draw commands
     */
    SkTArray<SkString>* getDrawCommandsAsStrings() const;

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

    void setBounds(int width, int height) {
        fWidth = width;
        fHeight = height;
    }

    void setUserMatrix(SkMatrix matrix) {
        fUserMatrix = matrix;
    }

////////////////////////////////////////////////////////////////////////////////
// Inherited from SkCanvas
////////////////////////////////////////////////////////////////////////////////

    virtual void clear(SkColor) SK_OVERRIDE;

    virtual void drawBitmap(const SkBitmap&, SkScalar left, SkScalar top,
                            const SkPaint*) SK_OVERRIDE;

    virtual void drawBitmapRectToRect(const SkBitmap&, const SkRect* src,
                                      const SkRect& dst, const SkPaint* paint,
                                      DrawBitmapRectFlags flags) SK_OVERRIDE;

    virtual void drawBitmapMatrix(const SkBitmap&, const SkMatrix&,
                                  const SkPaint*) SK_OVERRIDE;

    virtual void drawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                const SkRect& dst, const SkPaint*) SK_OVERRIDE;

    virtual void drawData(const void*, size_t) SK_OVERRIDE;

    virtual void beginCommentGroup(const char* description) SK_OVERRIDE;

    virtual void addComment(const char* kywd, const char* value) SK_OVERRIDE;

    virtual void endCommentGroup() SK_OVERRIDE;

    virtual void drawOval(const SkRect& oval, const SkPaint&) SK_OVERRIDE;

    virtual void drawPaint(const SkPaint& paint) SK_OVERRIDE;

    virtual void drawPath(const SkPath& path, const SkPaint&) SK_OVERRIDE;

    virtual void drawPicture(SkPicture& picture) SK_OVERRIDE;

    virtual void drawPoints(PointMode, size_t count, const SkPoint pts[],
                            const SkPaint&) SK_OVERRIDE;

    virtual void drawPosText(const void* text, size_t byteLength,
                             const SkPoint pos[], const SkPaint&) SK_OVERRIDE;

    virtual void drawPosTextH(const void* text, size_t byteLength,
                              const SkScalar xpos[], SkScalar constY,
                              const SkPaint&) SK_OVERRIDE;

    virtual void drawRect(const SkRect& rect, const SkPaint&) SK_OVERRIDE;

    virtual void drawRRect(const SkRRect& rrect, const SkPaint& paint) SK_OVERRIDE;

    virtual void drawSprite(const SkBitmap&, int left, int top,
                            const SkPaint*) SK_OVERRIDE;

    virtual void drawText(const void* text, size_t byteLength, SkScalar x,
                          SkScalar y, const SkPaint&) SK_OVERRIDE;

    virtual void drawTextOnPath(const void* text, size_t byteLength,
                                const SkPath& path, const SkMatrix* matrix,
                                const SkPaint&) SK_OVERRIDE;

    virtual void drawVertices(VertexMode, int vertexCount,
                              const SkPoint vertices[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode*,
                              const uint16_t indices[], int indexCount,
                              const SkPaint&) SK_OVERRIDE;

    static const int kVizImageHeight = 256;
    static const int kVizImageWidth = 256;

    virtual bool isClipEmpty() const SK_OVERRIDE { return false; }
    virtual bool isClipRect() const SK_OVERRIDE { return true; }
#ifdef SK_SUPPORT_LEGACY_GETCLIPTYPE
    virtual ClipType getClipType() const SK_OVERRIDE {
        return kRect_ClipType;
    }
#endif
    virtual bool getClipBounds(SkRect* bounds) const SK_OVERRIDE {
        if (NULL != bounds) {
            bounds->setXYWH(0, 0,
                            SkIntToScalar(this->imageInfo().fWidth),
                            SkIntToScalar(this->imageInfo().fHeight));
        }
        return true;
    }
    virtual bool getClipDeviceBounds(SkIRect* bounds) const SK_OVERRIDE {
        if (NULL != bounds) {
            bounds->setLargest();
        }
        return true;
    }

protected:
    virtual void willSave(SaveFlags) SK_OVERRIDE;
    virtual SaveLayerStrategy willSaveLayer(const SkRect*, const SkPaint*, SaveFlags) SK_OVERRIDE;
    virtual void willRestore() SK_OVERRIDE;

    virtual void didTranslate(SkScalar, SkScalar) SK_OVERRIDE;
    virtual void didScale(SkScalar, SkScalar) SK_OVERRIDE;
    virtual void didRotate(SkScalar) SK_OVERRIDE;
    virtual void didSkew(SkScalar, SkScalar) SK_OVERRIDE;
    virtual void didConcat(const SkMatrix&) SK_OVERRIDE;
    virtual void didSetMatrix(const SkMatrix&) SK_OVERRIDE;

    virtual void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) SK_OVERRIDE;
    virtual void onPushCull(const SkRect& cullRect) SK_OVERRIDE;
    virtual void onPopCull() SK_OVERRIDE;

    virtual void onClipRect(const SkRect&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    virtual void onClipRRect(const SkRRect&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    virtual void onClipPath(const SkPath&, SkRegion::Op, ClipEdgeStyle) SK_OVERRIDE;
    virtual void onClipRegion(const SkRegion& region, SkRegion::Op) SK_OVERRIDE;

    void markActiveCommands(int index);

private:
    SkTDArray<SkDrawCommand*> fCommandVector;
    int fWidth;
    int fHeight;
    bool fFilter;
    bool fMegaVizMode;
    int fIndex;
    SkMatrix fUserMatrix;
    SkMatrix fMatrix;
    SkIRect fClip;

    bool fOverdrawViz;
    SkDrawFilter* fOverdrawFilter;

    bool fOverrideTexFiltering;
    SkTexOverrideFilter* fTexOverrideFilter;

    /**
        Number of unmatched save() calls at any point during a draw.
        If there are any saveLayer() calls outstanding, we need to resolve
        all of them, which in practice means resolving all save() calls,
        to avoid corruption of our canvas.
    */
    int fOutstandingSaveCount;

    /**
        The active saveLayer commands at a given point in the renderering.
        Only used when "mega" visualization is enabled.
    */
    SkTDArray<SkDrawCommand*> fActiveLayers;

    /**
        The active cull commands at a given point in the rendering.
        Only used when "mega" visualization is enabled.
    */
    SkTDArray<SkDrawCommand*> fActiveCulls;

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

    typedef SkCanvas INHERITED;
};

#endif
