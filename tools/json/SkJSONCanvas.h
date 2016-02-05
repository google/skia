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
#include "SkJSONCPP.h"

#define SKJSONCANVAS_VERSION                     "version"
#define SKJSONCANVAS_COMMANDS                    "commands"
#define SKJSONCANVAS_COMMAND                     "command"

#define SKJSONCANVAS_COMMAND_TRANSLATE           "Translate"
#define SKJSONCANVAS_COMMAND_SCALE               "Scale"
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
#define SKJSONCANVAS_COMMAND_SAVELAYER           "SaveLayer"

#define SKJSONCANVAS_ATTRIBUTE_MATRIX            "matrix"
#define SKJSONCANVAS_ATTRIBUTE_COORDS            "coords"
#define SKJSONCANVAS_ATTRIBUTE_BOUNDS            "bounds"
#define SKJSONCANVAS_ATTRIBUTE_PAINT             "paint"
#define SKJSONCANVAS_ATTRIBUTE_OUTER             "outer"
#define SKJSONCANVAS_ATTRIBUTE_INNER             "inner"
#define SKJSONCANVAS_ATTRIBUTE_MODE              "mode"
#define SKJSONCANVAS_ATTRIBUTE_POINTS            "points"
#define SKJSONCANVAS_ATTRIBUTE_PATH              "path"
#define SKJSONCANVAS_ATTRIBUTE_TEXT              "text"
#define SKJSONCANVAS_ATTRIBUTE_COLOR             "color"
#define SKJSONCANVAS_ATTRIBUTE_ALPHA             "alpha"
#define SKJSONCANVAS_ATTRIBUTE_STYLE             "style"
#define SKJSONCANVAS_ATTRIBUTE_STROKEWIDTH       "strokeWidth"
#define SKJSONCANVAS_ATTRIBUTE_STROKEMITER       "strokeMiter"
#define SKJSONCANVAS_ATTRIBUTE_CAP               "cap"
#define SKJSONCANVAS_ATTRIBUTE_ANTIALIAS         "antiAlias"
#define SKJSONCANVAS_ATTRIBUTE_REGION            "region"
#define SKJSONCANVAS_ATTRIBUTE_REGIONOP          "op"
#define SKJSONCANVAS_ATTRIBUTE_EDGESTYLE         "edgeStyle"
#define SKJSONCANVAS_ATTRIBUTE_DEVICEREGION      "deviceRegion"
#define SKJSONCANVAS_ATTRIBUTE_BLUR              "blur"
#define SKJSONCANVAS_ATTRIBUTE_SIGMA             "sigma"
#define SKJSONCANVAS_ATTRIBUTE_QUALITY           "quality"
#define SKJSONCANVAS_ATTRIBUTE_TEXTALIGN         "textAlign"
#define SKJSONCANVAS_ATTRIBUTE_TEXTSIZE          "textSize"
#define SKJSONCANVAS_ATTRIBUTE_TEXTSCALEX        "textScaleX"
#define SKJSONCANVAS_ATTRIBUTE_TEXTSKEWX         "textSkewX"
#define SKJSONCANVAS_ATTRIBUTE_DASHING           "dashing"
#define SKJSONCANVAS_ATTRIBUTE_INTERVALS         "intervals"
#define SKJSONCANVAS_ATTRIBUTE_PHASE             "phase"
#define SKJSONCANVAS_ATTRIBUTE_FILLTYPE          "fillType"
#define SKJSONCANVAS_ATTRIBUTE_VERBS             "verbs"
#define SKJSONCANVAS_ATTRIBUTE_NAME              "name"
#define SKJSONCANVAS_ATTRIBUTE_BYTES             "bytes"
#define SKJSONCANVAS_ATTRIBUTE_SHADER            "shader"
#define SKJSONCANVAS_ATTRIBUTE_PATHEFFECT        "pathEffect"
#define SKJSONCANVAS_ATTRIBUTE_MASKFILTER        "maskFilter"
#define SKJSONCANVAS_ATTRIBUTE_XFERMODE          "xfermode"
#define SKJSONCANVAS_ATTRIBUTE_BACKDROP          "backdrop"
#define SKJSONCANVAS_ATTRIBUTE_COLORFILTER       "colorfilter"
#define SKJSONCANVAS_ATTRIBUTE_IMAGEFILTER       "imagefilter"
#define SKJSONCANVAS_ATTRIBUTE_IMAGE             "image"
#define SKJSONCANVAS_ATTRIBUTE_BITMAP            "bitmap"
#define SKJSONCANVAS_ATTRIBUTE_SRC               "src"
#define SKJSONCANVAS_ATTRIBUTE_DST               "dst"
#define SKJSONCANVAS_ATTRIBUTE_STRICT            "strict"
#define SKJSONCANVAS_ATTRIBUTE_DESCRIPTION       "description"
#define SKJSONCANVAS_ATTRIBUTE_X                 "x"
#define SKJSONCANVAS_ATTRIBUTE_Y                 "y"
#define SKJSONCANVAS_ATTRIBUTE_RUNS              "runs"
#define SKJSONCANVAS_ATTRIBUTE_POSITIONS         "positions"
#define SKJSONCANVAS_ATTRIBUTE_GLYPHS            "glyphs"
#define SKJSONCANVAS_ATTRIBUTE_FONT              "font"
#define SKJSONCANVAS_ATTRIBUTE_TYPEFACE          "typeface"

#define SKJSONCANVAS_VERB_MOVE                   "move"
#define SKJSONCANVAS_VERB_LINE                   "line"
#define SKJSONCANVAS_VERB_QUAD                   "quad"
#define SKJSONCANVAS_VERB_CUBIC                  "cubic"
#define SKJSONCANVAS_VERB_CONIC                  "conic"
#define SKJSONCANVAS_VERB_CLOSE                  "close"

#define SKJSONCANVAS_STYLE_FILL                  "fill"
#define SKJSONCANVAS_STYLE_STROKE                "stroke"
#define SKJSONCANVAS_STYLE_STROKEANDFILL         "strokeAndFill"

#define SKJSONCANVAS_POINTMODE_POINTS            "points"
#define SKJSONCANVAS_POINTMODE_LINES             "lines"
#define SKJSONCANVAS_POINTMODE_POLYGON           "polygon"

#define SKJSONCANVAS_REGIONOP_DIFFERENCE         "difference"
#define SKJSONCANVAS_REGIONOP_INTERSECT          "intersect"
#define SKJSONCANVAS_REGIONOP_UNION              "union"
#define SKJSONCANVAS_REGIONOP_XOR                "xor"
#define SKJSONCANVAS_REGIONOP_REVERSE_DIFFERENCE "reverseDifference"
#define SKJSONCANVAS_REGIONOP_REPLACE            "replace"

#define SKJSONCANVAS_BLURSTYLE_NORMAL            "normal"
#define SKJSONCANVAS_BLURSTYLE_SOLID             "solid"
#define SKJSONCANVAS_BLURSTYLE_OUTER             "outer"
#define SKJSONCANVAS_BLURSTYLE_INNER             "inner"

#define SKJSONCANVAS_BLURQUALITY_LOW             "low"
#define SKJSONCANVAS_BLURQUALITY_HIGH            "high"

#define SKJSONCANVAS_ALIGN_LEFT                  "left"
#define SKJSONCANVAS_ALIGN_CENTER                "center"
#define SKJSONCANVAS_ALIGN_RIGHT                 "right"

#define SKJSONCANVAS_FILLTYPE_WINDING            "winding"
#define SKJSONCANVAS_FILLTYPE_EVENODD            "evenOdd"
#define SKJSONCANVAS_FILLTYPE_INVERSEWINDING     "inverseWinding"
#define SKJSONCANVAS_FILLTYPE_INVERSEEVENODD     "inverseEvenOdd"

#define SKJSONCANVAS_CAP_BUTT                    "butt"
#define SKJSONCANVAS_CAP_ROUND                   "round"
#define SKJSONCANVAS_CAP_SQUARE                  "square"

#define SKJSONCANVAS_COLORTYPE_ARGB4444          "ARGB4444"
#define SKJSONCANVAS_COLORTYPE_RGBA8888          "RGBA8888"
#define SKJSONCANVAS_COLORTYPE_BGRA8888          "BGRA8888"
#define SKJSONCANVAS_COLORTYPE_565               "565"
#define SKJSONCANVAS_COLORTYPE_GRAY8             "Gray8"
#define SKJSONCANVAS_COLORTYPE_INDEX8            "Index8"
#define SKJSONCANVAS_COLORTYPE_ALPHA8            "Alpha8"

#define SKJSONCANVAS_ALPHATYPE_OPAQUE            "opaque"
#define SKJSONCANVAS_ALPHATYPE_PREMUL            "premul"
#define SKJSONCANVAS_ALPHATYPE_UNPREMUL          "unpremul"

/* 
 * Implementation of SkCanvas which writes JSON when drawn to. The JSON describes all of the draw
 * commands issued to the canvas, and can later be turned back into draw commands using 
 * SkJSONRenderer. Be sure to call finish() when you are done drawing.
 */
class SkJSONCanvas : public SkCanvas {
public:
    /* Create a canvas which writes to the specified output stream. */
    SkJSONCanvas(int width, int height, SkWStream& out, bool sendBinaries = false);

    /* Complete the JSON document. */
    void finish();

    static Json::Value MakeMatrix(const SkMatrix& matrix);

    static Json::Value MakeIRect(const SkIRect& irect);

    // overridden SkCanvas API

    void didConcat(const SkMatrix&) override;

    void didSetMatrix(const SkMatrix&) override;

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

    SkCanvas::SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& rec) override;

private:
    // Helpers to turn values into JSON, these could probably be static
    Json::Value makePoint(const SkPoint& point);

    Json::Value makePoint(SkScalar x, SkScalar y);

    Json::Value makeRect(const SkRect& rect);

    Json::Value makeRRect(const SkRRect& rrect);

    Json::Value makePath(const SkPath& path);

    Json::Value makeRegion(const SkRegion& region);
  
    Json::Value makePaint(const SkPaint& paint);
  
    Json::Value makeRegionOp(SkRegion::Op op);
  
    Json::Value makeEdgeStyle(SkCanvas::ClipEdgeStyle edgeStyle);
  
    Json::Value makePointMode(SkCanvas::PointMode mode);

    void updateMatrix();

    SkWStream&  fOut;
    Json::Value fRoot;
    Json::Value fCommands;
    bool        fSendBinaries;

    typedef SkCanvas INHERITED;
};

#endif
