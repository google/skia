/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJSONCanvas_DEFINED
#define SkJSONCanvas_DEFINED

#include "SkCanvas.h"
#include "SkStream.h"

#define SKJSONCANVAS_VERSION                     "version"
#define SKJSONCANVAS_COMMANDS                    "commands"
#define SKJSONCANVAS_COMMAND                     "command"

#define SKJSONCANVAS_COMMAND_MATRIX              "Matrix"
#define SKJSONCANVAS_COMMAND_PAINT               "Paint"
#define SKJSONCANVAS_COMMAND_RECT                "Rect"
#define SKJSONCANVAS_COMMAND_OVAL                "Oval"
#define SKJSONCANVAS_COMMAND_RRECT               "RRect"
#define SKJSONCANVAS_COMMAND_DRRECT              "DRRect"
#define SKJSONCANVAS_COMMAND_POINTS              "Points"
#define SKJSONCANVAS_COMMAND_VERTICES            "Vertices"
#define SKJSONCANVAS_COMMAND_ATLAS               "Atlas"
#define SKJSONCANVAS_COMMAND_PATH                "Path"
#define SKJSONCANVAS_COMMAND_IMAGE               "Image"
#define SKJSONCANVAS_COMMAND_IMAGERECT           "ImageRect"
#define SKJSONCANVAS_COMMAND_IMAGENINE           "ImageNine"
#define SKJSONCANVAS_COMMAND_BITMAP              "Bitmap"
#define SKJSONCANVAS_COMMAND_BITMAPRECT          "BitmapRect"
#define SKJSONCANVAS_COMMAND_BITMAPNINE          "BitmapNine"
#define SKJSONCANVAS_COMMAND_TEXT                "Text"
#define SKJSONCANVAS_COMMAND_POSTEXT             "PosText"
#define SKJSONCANVAS_COMMAND_POSTEXTH            "PosTextH"
#define SKJSONCANVAS_COMMAND_TEXTONPATH          "TextOnPath"
#define SKJSONCANVAS_COMMAND_TEXTBLOB            "TextBlob"
#define SKJSONCANVAS_COMMAND_PATCH               "Patch"
#define SKJSONCANVAS_COMMAND_DRAWABLE            "Drawable"
#define SKJSONCANVAS_COMMAND_CLIPRECT            "ClipRect"
#define SKJSONCANVAS_COMMAND_CLIPRRECT           "ClipRRect"
#define SKJSONCANVAS_COMMAND_CLIPPATH            "ClipPath"
#define SKJSONCANVAS_COMMAND_CLIPREGION          "ClipRegion"
#define SKJSONCANVAS_COMMAND_SAVE                "Save"
#define SKJSONCANVAS_COMMAND_RESTORE             "Restore"

#define SKJSONCANVAS_ATTRIBUTE_MATRIX            "matrix"
#define SKJSONCANVAS_ATTRIBUTE_COORDS            "coords"
#define SKJSONCANVAS_ATTRIBUTE_PAINT             "paint"
#define SKJSONCANVAS_ATTRIBUTE_OUTER             "outer"
#define SKJSONCANVAS_ATTRIBUTE_INNER             "inner"
#define SKJSONCANVAS_ATTRIBUTE_MODE              "mode"
#define SKJSONCANVAS_ATTRIBUTE_POINTS            "points"
#define SKJSONCANVAS_ATTRIBUTE_PATH              "path"
#define SKJSONCANVAS_ATTRIBUTE_TEXT              "text"
#define SKJSONCANVAS_ATTRIBUTE_COLOR             "color"
#define SKJSONCANVAS_ATTRIBUTE_STYLE             "style"
#define SKJSONCANVAS_ATTRIBUTE_STROKEWIDTH       "strokeWidth"
#define SKJSONCANVAS_ATTRIBUTE_ANTIALIAS         "antiAlias"
#define SKJSONCANVAS_ATTRIBUTE_REGIONOP          "op"
#define SKJSONCANVAS_ATTRIBUTE_EDGESTYLE         "edgeStyle"
#define SKJSONCANVAS_ATTRIBUTE_DEVICEREGION      "deviceRegion"

#define SKJSONCANVAS_VERB_MOVE                   "move"
#define SKJSONCANVAS_VERB_LINE                   "line"
#define SKJSONCANVAS_VERB_QUAD                   "quad"
#define SKJSONCANVAS_VERB_CUBIC                  "cubic"
#define SKJSONCANVAS_VERB_CONIC                  "conic"
#define SKJSONCANVAS_VERB_CLOSE                  "close"

#define SKJSONCANVAS_STYLE_FILL                  "fill"
#define SKJSONCANVAS_STYLE_STROKE                "stroke"
#define SKJSONCANVAS_STYLE_STROKEANDFILL         "strokeAndFill"

#define SKJSONCANVAS_EDGESTYLE_HARD              "hard"
#define SKJSONCANVAS_EDGESTYLE_SOFT              "soft"

#define SKJSONCANVAS_POINTMODE_POINTS            "points"
#define SKJSONCANVAS_POINTMODE_LINES             "lines"
#define SKJSONCANVAS_POINTMODE_POLYGON           "polygon"

#define SKJSONCANVAS_REGIONOP_DIFFERENCE         "difference"
#define SKJSONCANVAS_REGIONOP_INTERSECT          "intersect"
#define SKJSONCANVAS_REGIONOP_UNION              "union"
#define SKJSONCANVAS_REGIONOP_XOR                "xor"
#define SKJSONCANVAS_REGIONOP_REVERSE_DIFFERENCE "reverseDifference"
#define SKJSONCANVAS_REGIONOP_REPLACE            "replace"

/* 
 * Implementation of SkCanvas which writes JSON when drawn to. The JSON describes all of the draw
 * commands issued to the canvas, and can later be turned back into draw commands using 
 * SkJSONRenderer. Be sure to call finish() when you are done drawing.
 */
class SkJSONCanvas : public SkCanvas {
public:
    /* Create a canvas which writes to the specified output stream. */
    SkJSONCanvas(int width, int height, SkWStream& out);

    /* Complete the JSON document. */
    void finish();

    // overridden SkCanvas API

    void onDrawPaint(const SkPaint&) override;

    void onDrawRect(const SkRect&, const SkPaint&) override;

    void onDrawOval(const SkRect&, const SkPaint&) override;

    void onDrawRRect(const SkRRect&, const SkPaint&) override;

    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override;

    void onDrawPoints(SkCanvas::PointMode, size_t count, const SkPoint pts[], 
                      const SkPaint&) override;

    void onDrawVertices(SkCanvas::VertexMode, int vertexCount, const SkPoint vertices[],
                        const SkPoint texs[], const SkColor colors[], SkXfermode*,
                        const uint16_t indices[], int indexCount, const SkPaint&) override;

    void onDrawAtlas(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[],
                     int count, SkXfermode::Mode, const SkRect* cull, const SkPaint*) override;

    void onDrawPath(const SkPath&, const SkPaint&) override;

    void onDrawImage(const SkImage*, SkScalar dx, SkScalar dy, const SkPaint*) override;

    void onDrawImageRect(const SkImage*, const SkRect*, const SkRect&, const SkPaint*,
                         SrcRectConstraint) override;

    void onDrawImageNine(const SkImage*, const SkIRect& center, const SkRect& dst,
                         const SkPaint*) override;

    void onDrawBitmap(const SkBitmap&, SkScalar dx, SkScalar dy, const SkPaint*) override;

    void onDrawBitmapRect(const SkBitmap&, const SkRect*, const SkRect&, const SkPaint*,
                          SkCanvas::SrcRectConstraint) override;

    void onDrawBitmapNine(const SkBitmap&, const SkIRect& center, const SkRect& dst,
                          const SkPaint*) override;

    void onDrawText(const void* text, size_t byteLength, SkScalar x,
                    SkScalar y, const SkPaint& paint) override;

    void onDrawPosText(const void* text, size_t byteLength,
                       const SkPoint pos[], const SkPaint& paint) override;

    void onDrawPosTextH(const void* text, size_t byteLength,
                        const SkScalar xpos[], SkScalar constY,
                        const SkPaint& paint) override;

    void onDrawTextOnPath(const void* text, size_t byteLength,
                          const SkPath& path, const SkMatrix* matrix,
                          const SkPaint& paint) override;

    void onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                        const SkPaint& paint) override;

    void onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                     const SkPoint texCoords[4], SkXfermode* xmode, const SkPaint& paint) override;

    void onDrawDrawable(SkDrawable*, const SkMatrix*) override;    

    void onClipRect(const SkRect& rect, SkRegion::Op op, ClipEdgeStyle edgeStyle) override;

    void onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle) override;

    void onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle) override;

    void onClipRegion(const SkRegion& deviceRgn, SkRegion::Op op) override;

    void willSave() override;

    void willRestore() override;

private:
    void writef(const char* fmt, ...);

    void open(const char* name);

    void close();

    void writeString(const char* name, const char* text);

    void writeString(const char* name, const void* text, size_t length);

    void writePoint(const char* name, const SkPoint& point);

    void writeRect(const char* name, const SkRect& rect);

    void writeRRect(const char* name, const SkRRect& rrect);

    void writePath(const char* name, const SkPath& path);

    void writeRegion(const char* name, const SkRegion& region);
  
    void writePaint(const SkPaint& paint);
  
    void writeRegionOp(const char* name, SkRegion::Op op);
  
    void writeEdgeStyle(const char* name, SkCanvas::ClipEdgeStyle edgeStyle);
  
    void writePointMode(const char* name, SkCanvas::PointMode mode);
  
    void writeMatrix(const char* name, const SkMatrix& matrix);
  
    void updateMatrix();

    SkWStream& fOut;
    SkMatrix   fLastMatrix;
    bool       fFirstCommand;

    typedef SkCanvas INHERITED;
};

#endif
