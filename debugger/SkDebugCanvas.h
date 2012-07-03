
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SKDEBUGCANVAS_H_
#define SKDEBUGCANVAS_H_

#include <iostream>
#include "SkCanvas.h"
#include "SkDrawCommand.h"
#include "SkPicture.h"
#include <vector>

class SkDebugCanvas : public SkCanvas {
public:
    bool fFilter;

    SkDebugCanvas();
    ~SkDebugCanvas();

    void toggleFilter(bool toggle);

    /**
        Executes all draw calls to the canvas.
        @param canvas  The canvas being drawn to
     */
    void draw(SkCanvas* canvas);

    /**
        Executes the draw calls in the specified range.
        @param canvas  The canvas being drawn to
        @param i  The beginning of the range
        @param j  The end of the range
        TODO(chudy): Implement
     */
    void drawRange(SkCanvas* canvas, int i, int j);

    /**
        Executes the draw calls up to the specified index.
        @param canvas  The canvas being drawn to
        @param index  The index of the final command being executed
     */
    void drawTo(SkCanvas* canvas, int index);

    /**
        Returns the draw command at the given index.
        @param index  The index of the command
     */
    SkDrawCommand* getDrawCommandAt(int index);

    /**
        Returns information about the command at the given index.
        @param index  The index of the command
     */
    std::vector<std::string>* getCommandInfoAt(int index);

    /**
        Returns the vector of draw commands
     */
    std::vector<SkDrawCommand*> getDrawCommands();

    /**
     * Returns the string vector of draw commands
     */
    std::vector<std::string>* getDrawCommandsAsStrings();

    /**
        Returns length of draw command vector.
     */
    int getSize() {
        return commandVector.size();
    }

    /**
        Toggles the execution of the draw command at index i.
     */
    void toggleCommand(int index);

    /**
        Toggles the visibility / execution of the draw command at index i with
        the value of toggle.
     */
    void toggleCommand(int index, bool toggle);

////////////////////////////////////////////////////////////////////////////////
// Inherited from SkCanvas
////////////////////////////////////////////////////////////////////////////////

    virtual void clear(SkColor) SK_OVERRIDE;

    virtual bool clipPath(const SkPath&, SkRegion::Op, bool) SK_OVERRIDE;

    virtual bool clipRect(const SkRect&, SkRegion::Op, bool) SK_OVERRIDE;

    virtual bool clipRegion(const SkRegion& region, SkRegion::Op op) SK_OVERRIDE;

    virtual bool concat(const SkMatrix& matrix) SK_OVERRIDE;

    virtual void drawBitmap(const SkBitmap&, SkScalar left, SkScalar top,
                            const SkPaint*) SK_OVERRIDE;

    virtual void drawBitmapRect(const SkBitmap&, const SkIRect* src,
                                const SkRect& dst, const SkPaint*) SK_OVERRIDE;

    virtual void drawBitmapMatrix(const SkBitmap&, const SkMatrix&,
                                  const SkPaint*) SK_OVERRIDE;

    virtual void drawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                const SkRect& dst, const SkPaint*) SK_OVERRIDE;

    virtual void drawData(const void*, size_t) SK_OVERRIDE;

    virtual void drawPaint(const SkPaint& paint) SK_OVERRIDE;

    virtual void drawPath(const SkPath& path, const SkPaint&) SK_OVERRIDE;

    virtual void drawPicture(SkPicture& picture) SK_OVERRIDE;

    virtual void drawPoints(PointMode, size_t count, const SkPoint pts[],
                            const SkPaint&) SK_OVERRIDE;

    virtual void drawPosText(const void* text, size_t byteLength,
                             const SkPoint pos[], const SkPaint&) SK_OVERRIDE;

    virtual void drawPosTextH(const void* text, size_t byteLength,
                      const SkScalar xpos[], SkScalar constY, const SkPaint&) SK_OVERRIDE;

    virtual void drawRect(const SkRect& rect, const SkPaint&) SK_OVERRIDE;

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

    virtual void restore() SK_OVERRIDE;

    virtual bool rotate(SkScalar degrees) SK_OVERRIDE;

    virtual int save(SaveFlags) SK_OVERRIDE;

    virtual int saveLayer(const SkRect* bounds, const SkPaint*, SaveFlags) SK_OVERRIDE;

    virtual bool scale(SkScalar sx, SkScalar sy) SK_OVERRIDE;

    virtual void setMatrix(const SkMatrix& matrix) SK_OVERRIDE;

    virtual bool skew(SkScalar sx, SkScalar sy) SK_OVERRIDE;

    virtual bool translate(SkScalar dx, SkScalar dy) SK_OVERRIDE;

private:
    typedef SkCanvas INHERITED;
    std::vector<SkDrawCommand*> commandVector;
    std::vector<SkDrawCommand*>::const_iterator it;

    SkBitmap fBm;

    /**
        Adds the command to the classes vector of commands.
        @param command  The draw command for execution
     */
    void addDrawCommand(SkDrawCommand* command);
};

#endif
