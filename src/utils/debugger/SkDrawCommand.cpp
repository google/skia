
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDrawCommand.h"
#include "SkObjectParser.h"

// TODO(chudy): Refactor into non subclass model.

SkDrawCommand::SkDrawCommand(DrawType type)
    : fDrawType(type)
    , fVisible(true) {
}

SkDrawCommand::SkDrawCommand() {
    fVisible = true;
}

SkDrawCommand::~SkDrawCommand() {
    fInfo.deleteAll();
}

const char* SkDrawCommand::GetCommandString(DrawType type) {
    switch (type) {
        case UNUSED: SkDEBUGFAIL("DrawType UNUSED\n"); break;
        case DRAW_CLEAR: return "Clear";
        case CLIP_PATH: return "Clip Path";
        case CLIP_REGION: return "Clip Region";
        case CLIP_RECT: return "Clip Rect";
        case CLIP_RRECT: return "Clip RRect";
        case CONCAT: return "Concat";
        case DRAW_BITMAP: return "Draw Bitmap";
        case DRAW_BITMAP_MATRIX: return "Draw Bitmap Matrix";
        case DRAW_BITMAP_NINE: return "Draw Bitmap Nine";
        case DRAW_BITMAP_RECT_TO_RECT: return "Draw Bitmap Rect";
        case DRAW_DATA: return "Draw Data";
        case DRAW_OVAL: return "Draw Oval";
        case DRAW_PAINT: return "Draw Paint";
        case DRAW_PATH: return "Draw Path";
        case DRAW_PICTURE: return "Draw Picture";
        case DRAW_POINTS: return "Draw Points";
        case DRAW_POS_TEXT: return "Draw Pos Text";
        case DRAW_POS_TEXT_H: return "Draw Pos Text H";
        case DRAW_RECT: return "Draw Rect";
        case DRAW_RRECT: return "Draw RRect";
        case DRAW_SPRITE: return "Draw Sprite";
        case DRAW_TEXT: return "Draw Text";
        case DRAW_TEXT_ON_PATH: return "Draw Text On Path";
        case DRAW_VERTICES: return "Draw Vertices";
        case RESTORE: return "Restore";
        case ROTATE: return "Rotate";
        case SAVE: return "Save";
        case SAVE_LAYER: return "Save Layer";
        case SCALE: return "Scale";
        case SET_MATRIX: return "Set Matrix";
        case SKEW: return "Skew";
        case TRANSLATE: return "Translate";
        case NOOP: return "NoOp";
        case BEGIN_COMMENT_GROUP: return "BeginCommentGroup";
        case COMMENT: return "Comment";
        case END_COMMENT_GROUP: return "EndCommentGroup";
        default:
            SkDebugf("DrawType error 0x%08x\n", type);
            SkASSERT(0);
            break;
    }
    SkDEBUGFAIL("DrawType UNUSED\n");
    return NULL;
}

SkString SkDrawCommand::toString() {
    return SkString(GetCommandString(fDrawType));
}

Clear::Clear(SkColor color) {
    fColor = color;
    fDrawType = DRAW_CLEAR;
    fInfo.push(SkObjectParser::CustomTextToString("No Parameters"));
}

void Clear::execute(SkCanvas* canvas) {
    canvas->clear(fColor);
}

namespace {

void xlate_and_scale_to_bounds(SkCanvas* canvas, const SkRect& bounds) {
    const SkISize& size = canvas->getDeviceSize();

    static const SkScalar kInsetFrac = 0.9f; // Leave a border around object

    canvas->translate(size.fWidth/2.0f, size.fHeight/2.0f);
    if (bounds.width() > bounds.height()) {
        canvas->scale(SkDoubleToScalar((kInsetFrac*size.fWidth)/bounds.width()),
                      SkDoubleToScalar((kInsetFrac*size.fHeight)/bounds.width()));
    } else {
        canvas->scale(SkDoubleToScalar((kInsetFrac*size.fWidth)/bounds.height()),
                      SkDoubleToScalar((kInsetFrac*size.fHeight)/bounds.height()));
    }
    canvas->translate(-bounds.centerX(), -bounds.centerY());
}


void render_path(SkCanvas* canvas, const SkPath& path) {
    canvas->clear(0xFFFFFFFF);
    canvas->save();

    const SkRect& bounds = path.getBounds();

    xlate_and_scale_to_bounds(canvas, bounds);

    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setStyle(SkPaint::kStroke_Style);

    canvas->drawPath(path, p);
    canvas->restore();
}

void render_bitmap(SkCanvas* canvas, const SkBitmap& input, const SkRect* srcRect = NULL) {
    const SkISize& size = canvas->getDeviceSize();

    SkScalar xScale = SkIntToScalar(size.fWidth-2) / input.width();
    SkScalar yScale = SkIntToScalar(size.fHeight-2) / input.height();

    if (input.width() > input.height()) {
        yScale *= input.height() / (float) input.width();
    } else {
        xScale *= input.width() / (float) input.height();
    }

    SkRect dst = SkRect::MakeXYWH(SK_Scalar1, SK_Scalar1,
                                  xScale * input.width(),
                                  yScale * input.height());

    canvas->clear(0xFFFFFFFF);
    canvas->drawBitmapRect(input, NULL, dst);

    if (NULL != srcRect) {
        SkRect r = SkRect::MakeLTRB(srcRect->fLeft * xScale + SK_Scalar1,
                                    srcRect->fTop * yScale + SK_Scalar1,
                                    srcRect->fRight * xScale + SK_Scalar1,
                                    srcRect->fBottom * yScale + SK_Scalar1);
        SkPaint p;
        p.setColor(SK_ColorRED);
        p.setStyle(SkPaint::kStroke_Style);

        canvas->drawRect(r, p);
    }
}

void render_rrect(SkCanvas* canvas, const SkRRect& rrect) {
    canvas->clear(0xFFFFFFFF);
    canvas->save();

    const SkRect& bounds = rrect.getBounds();

    xlate_and_scale_to_bounds(canvas, bounds);

    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setStyle(SkPaint::kStroke_Style);

    canvas->drawRRect(rrect, p);
    canvas->restore();
}

};


ClipPath::ClipPath(const SkPath& path, SkRegion::Op op, bool doAA) {
    fPath = path;
    fOp = op;
    fDoAA = doAA;
    fDrawType = CLIP_PATH;

    fInfo.push(SkObjectParser::PathToString(path));
    fInfo.push(SkObjectParser::RegionOpToString(op));
    fInfo.push(SkObjectParser::BoolToString(doAA));
}

void ClipPath::execute(SkCanvas* canvas) {
    canvas->clipPath(fPath, fOp, fDoAA);
}

bool ClipPath::render(SkCanvas* canvas) const {
    render_path(canvas, fPath);
    return true;
}

ClipRegion::ClipRegion(const SkRegion& region, SkRegion::Op op) {
    fRegion = region;
    fOp = op;
    fDrawType = CLIP_REGION;

    fInfo.push(SkObjectParser::RegionToString(region));
    fInfo.push(SkObjectParser::RegionOpToString(op));
}

void ClipRegion::execute(SkCanvas* canvas) {
    canvas->clipRegion(fRegion, fOp);
}

ClipRect::ClipRect(const SkRect& rect, SkRegion::Op op, bool doAA) {
    fRect = rect;
    fOp = op;
    fDoAA = doAA;
    fDrawType = CLIP_RECT;

    fInfo.push(SkObjectParser::RectToString(rect));
    fInfo.push(SkObjectParser::RegionOpToString(op));
    fInfo.push(SkObjectParser::BoolToString(doAA));
}

void ClipRect::execute(SkCanvas* canvas) {
    canvas->clipRect(fRect, fOp, fDoAA);
}

ClipRRect::ClipRRect(const SkRRect& rrect, SkRegion::Op op, bool doAA) {
    fRRect = rrect;
    fOp = op;
    fDoAA = doAA;
    fDrawType = CLIP_RRECT;

    fInfo.push(SkObjectParser::RRectToString(rrect));
    fInfo.push(SkObjectParser::RegionOpToString(op));
    fInfo.push(SkObjectParser::BoolToString(doAA));
}

void ClipRRect::execute(SkCanvas* canvas) {
    canvas->clipRRect(fRRect, fOp, fDoAA);
}

bool ClipRRect::render(SkCanvas* canvas) const {
    render_rrect(canvas, fRRect);
    return true;
}

Concat::Concat(const SkMatrix& matrix) {
    fMatrix = matrix;
    fDrawType = CONCAT;

    fInfo.push(SkObjectParser::MatrixToString(matrix));
}

void Concat::execute(SkCanvas* canvas) {
    canvas->concat(fMatrix);
}

DrawBitmap::DrawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                       const SkPaint* paint) {
    fBitmap = bitmap;
    fLeft = left;
    fTop = top;
    if (NULL != paint) {
        fPaint = *paint;
        fPaintPtr = &fPaint;
    } else {
        fPaintPtr = NULL;
    }
    fDrawType = DRAW_BITMAP;

    fInfo.push(SkObjectParser::BitmapToString(bitmap));
    fInfo.push(SkObjectParser::ScalarToString(left, "SkScalar left: "));
    fInfo.push(SkObjectParser::ScalarToString(top, "SkScalar top: "));
    if (NULL != paint) {
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }
}

void DrawBitmap::execute(SkCanvas* canvas) {
    canvas->drawBitmap(fBitmap, fLeft, fTop, fPaintPtr);
}

bool DrawBitmap::render(SkCanvas* canvas) const {
    render_bitmap(canvas, fBitmap);
    return true;
}

DrawBitmapMatrix::DrawBitmapMatrix(const SkBitmap& bitmap,
                                   const SkMatrix& matrix,
                                   const SkPaint* paint) {
    fBitmap = bitmap;
    fMatrix = matrix;
    if (NULL != paint) {
        fPaint = *paint;
        fPaintPtr = &fPaint;
    } else {
        fPaintPtr = NULL;
    }
    fDrawType = DRAW_BITMAP_MATRIX;

    fInfo.push(SkObjectParser::BitmapToString(bitmap));
    fInfo.push(SkObjectParser::MatrixToString(matrix));
    if (NULL != paint) {
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }
}

void DrawBitmapMatrix::execute(SkCanvas* canvas) {
    canvas->drawBitmapMatrix(fBitmap, fMatrix, fPaintPtr);
}

bool DrawBitmapMatrix::render(SkCanvas* canvas) const {
    render_bitmap(canvas, fBitmap);
    return true;
}

DrawBitmapNine::DrawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                               const SkRect& dst, const SkPaint* paint) {
    fBitmap = bitmap;
    fCenter = center;
    fDst = dst;
    if (NULL != paint) {
        fPaint = *paint;
        fPaintPtr = &fPaint;
    } else {
        fPaintPtr = NULL;
    }
    fDrawType = DRAW_BITMAP_NINE;

    fInfo.push(SkObjectParser::BitmapToString(bitmap));
    fInfo.push(SkObjectParser::IRectToString(center));
    fInfo.push(SkObjectParser::RectToString(dst, "Dst: "));
    if (NULL != paint) {
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }
}

void DrawBitmapNine::execute(SkCanvas* canvas) {
    canvas->drawBitmapNine(fBitmap, fCenter, fDst, fPaintPtr);
}

bool DrawBitmapNine::render(SkCanvas* canvas) const {
    render_bitmap(canvas, fBitmap);
    return true;
}

DrawBitmapRect::DrawBitmapRect(const SkBitmap& bitmap, const SkRect* src,
                               const SkRect& dst, const SkPaint* paint) {
    fBitmap = bitmap;
    if (NULL != src) {
        fSrc = *src;
    } else {
        fSrc.setEmpty();
    }
    fDst = dst;

    if (NULL != paint) {
        fPaint = *paint;
        fPaintPtr = &fPaint;
    } else {
        fPaintPtr = NULL;
    }
    fDrawType = DRAW_BITMAP_RECT_TO_RECT;

    fInfo.push(SkObjectParser::BitmapToString(bitmap));
    if (NULL != src) {
        fInfo.push(SkObjectParser::RectToString(*src, "Src: "));
    }
    fInfo.push(SkObjectParser::RectToString(dst, "Dst: "));
    if (NULL != paint) {
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }
}

void DrawBitmapRect::execute(SkCanvas* canvas) {
    canvas->drawBitmapRectToRect(fBitmap, this->srcRect(), fDst, fPaintPtr);
}

bool DrawBitmapRect::render(SkCanvas* canvas) const {
    render_bitmap(canvas, fBitmap, this->srcRect());
    return true;
}

DrawData::DrawData(const void* data, size_t length) {
    fData = new char[length];
    memcpy(fData, data, length);
    fLength = length;
    fDrawType = DRAW_DATA;

    // TODO: add display of actual data?
    SkString* str = new SkString;
    str->appendf("length: %d", (int) length);
    fInfo.push(str);
}

void DrawData::execute(SkCanvas* canvas) {
    canvas->drawData(fData, fLength);
}

BeginCommentGroup::BeginCommentGroup(const char* description)
    : INHERITED(BEGIN_COMMENT_GROUP)
    , fDescription(description) {
    SkString* temp = new SkString;
    temp->appendf("Description: %s", description);
    fInfo.push(temp);
}

Comment::Comment(const char* kywd, const char* value)
    : INHERITED(COMMENT)
    , fKywd(kywd)
    , fValue(value) {
    SkString* temp = new SkString;
    temp->appendf("%s: %s", kywd, value);
    fInfo.push(temp);
}

EndCommentGroup::EndCommentGroup() : INHERITED(END_COMMENT_GROUP) {
}

DrawOval::DrawOval(const SkRect& oval, const SkPaint& paint) {
    fOval = oval;
    fPaint = paint;
    fDrawType = DRAW_OVAL;

    fInfo.push(SkObjectParser::RectToString(oval));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void DrawOval::execute(SkCanvas* canvas) {
    canvas->drawOval(fOval, fPaint);
}

bool DrawOval::render(SkCanvas* canvas) const {
    canvas->clear(0xFFFFFFFF);
    canvas->save();

    xlate_and_scale_to_bounds(canvas, fOval);

    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setStyle(SkPaint::kStroke_Style);

    canvas->drawOval(fOval, p);
    canvas->restore();

    return true;
}

DrawPaint::DrawPaint(const SkPaint& paint) {
    fPaint = paint;
    fDrawType = DRAW_PAINT;

    fInfo.push(SkObjectParser::PaintToString(paint));
}

void DrawPaint::execute(SkCanvas* canvas) {
    canvas->drawPaint(fPaint);
}

bool DrawPaint::render(SkCanvas* canvas) const {
    canvas->clear(0xFFFFFFFF);
    canvas->drawPaint(fPaint);
    return true;
}

DrawPath::DrawPath(const SkPath& path, const SkPaint& paint) {
    fPath = path;
    fPaint = paint;
    fDrawType = DRAW_PATH;

    fInfo.push(SkObjectParser::PathToString(path));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void DrawPath::execute(SkCanvas* canvas) {
    canvas->drawPath(fPath, fPaint);
}

bool DrawPath::render(SkCanvas* canvas) const {
    render_path(canvas, fPath);
    return true;
}

DrawPicture::DrawPicture(SkPicture& picture) :
    fPicture(picture) {
    fDrawType = DRAW_PICTURE;
    fInfo.push(SkObjectParser::CustomTextToString("To be implemented."));
}

void DrawPicture::execute(SkCanvas* canvas) {
    canvas->drawPicture(fPicture);
}

DrawPoints::DrawPoints(SkCanvas::PointMode mode, size_t count,
                       const SkPoint pts[], const SkPaint& paint) {
    fMode = mode;
    fCount = count;
    fPts = new SkPoint[count];
    memcpy(fPts, pts, count * sizeof(SkPoint));
    fPaint = paint;
    fDrawType = DRAW_POINTS;

    fInfo.push(SkObjectParser::PointsToString(pts, count));
    fInfo.push(SkObjectParser::ScalarToString(SkIntToScalar((unsigned int)count),
                                              "Points: "));
    fInfo.push(SkObjectParser::PointModeToString(mode));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void DrawPoints::execute(SkCanvas* canvas) {
    canvas->drawPoints(fMode, fCount, fPts, fPaint);
}

bool DrawPoints::render(SkCanvas* canvas) const {
    canvas->clear(0xFFFFFFFF);
    canvas->save();

    SkRect bounds;

    bounds.setEmpty();
    for (unsigned int i = 0; i < fCount; ++i) {
        bounds.growToInclude(fPts[i].fX, fPts[i].fY);
    }

    xlate_and_scale_to_bounds(canvas, bounds);

    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setStyle(SkPaint::kStroke_Style);

    canvas->drawPoints(fMode, fCount, fPts, p);
    canvas->restore();

    return true;
}

DrawPosText::DrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                         const SkPaint& paint) {
    size_t numPts = paint.countText(text, byteLength);

    fText = new char[byteLength];
    memcpy(fText, text, byteLength);
    fByteLength = byteLength;

    fPos = new SkPoint[numPts];
    memcpy(fPos, pos, numPts * sizeof(SkPoint));

    fPaint = paint;
    fDrawType = DRAW_POS_TEXT;

    fInfo.push(SkObjectParser::TextToString(text, byteLength, paint.getTextEncoding()));
    // TODO(chudy): Test that this works.
    fInfo.push(SkObjectParser::PointsToString(pos, 1));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void DrawPosText::execute(SkCanvas* canvas) {
    canvas->drawPosText(fText, fByteLength, fPos, fPaint);
}


DrawPosTextH::DrawPosTextH(const void* text, size_t byteLength,
                           const SkScalar xpos[], SkScalar constY,
                           const SkPaint& paint) {
    size_t numPts = paint.countText(text, byteLength);

    fText = new char[byteLength];
    memcpy(fText, text, byteLength);
    fByteLength = byteLength;

    fXpos = new SkScalar[numPts];
    memcpy(fXpos, xpos, numPts * sizeof(SkScalar));

    fConstY = constY;
    fPaint = paint;
    fDrawType = DRAW_POS_TEXT_H;

    fInfo.push(SkObjectParser::TextToString(text, byteLength, paint.getTextEncoding()));
    fInfo.push(SkObjectParser::ScalarToString(xpos[0], "XPOS: "));
    fInfo.push(SkObjectParser::ScalarToString(constY, "SkScalar constY: "));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void DrawPosTextH::execute(SkCanvas* canvas) {
    canvas->drawPosTextH(fText, fByteLength, fXpos, fConstY, fPaint);
}

DrawRectC::DrawRectC(const SkRect& rect, const SkPaint& paint) {
    fRect = rect;
    fPaint = paint;
    fDrawType = DRAW_RECT;

    fInfo.push(SkObjectParser::RectToString(rect));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void DrawRectC::execute(SkCanvas* canvas) {
    canvas->drawRect(fRect, fPaint);
}

DrawRRect::DrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    fRRect = rrect;
    fPaint = paint;
    fDrawType = DRAW_RRECT;

    fInfo.push(SkObjectParser::RRectToString(rrect));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void DrawRRect::execute(SkCanvas* canvas) {
    canvas->drawRRect(fRRect, fPaint);
}

bool DrawRRect::render(SkCanvas* canvas) const {
    render_rrect(canvas, fRRect);
    return true;
}

DrawSprite::DrawSprite(const SkBitmap& bitmap, int left, int top,
                       const SkPaint* paint) {
    fBitmap = bitmap;
    fLeft = left;
    fTop = top;
    if (NULL != paint) {
        fPaint = *paint;
        fPaintPtr = &fPaint;
    } else {
        fPaintPtr = NULL;
    }
    fDrawType = DRAW_SPRITE;

    fInfo.push(SkObjectParser::BitmapToString(bitmap));
    fInfo.push(SkObjectParser::IntToString(left, "Left: "));
    fInfo.push(SkObjectParser::IntToString(top, "Top: "));
    if (NULL != paint) {
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }
}

void DrawSprite::execute(SkCanvas* canvas) {
    canvas->drawSprite(fBitmap, fLeft, fTop, fPaintPtr);
}

bool DrawSprite::render(SkCanvas* canvas) const {
    render_bitmap(canvas, fBitmap);
    return true;
}

DrawTextC::DrawTextC(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                     const SkPaint& paint) {
    fText = new char[byteLength];
    memcpy(fText, text, byteLength);
    fByteLength = byteLength;
    fX = x;
    fY = y;
    fPaint = paint;
    fDrawType = DRAW_TEXT;

    fInfo.push(SkObjectParser::TextToString(text, byteLength, paint.getTextEncoding()));
    fInfo.push(SkObjectParser::ScalarToString(x, "SkScalar x: "));
    fInfo.push(SkObjectParser::ScalarToString(y, "SkScalar y: "));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void DrawTextC::execute(SkCanvas* canvas) {
    canvas->drawText(fText, fByteLength, fX, fY, fPaint);
}

DrawTextOnPath::DrawTextOnPath(const void* text, size_t byteLength,
                               const SkPath& path, const SkMatrix* matrix,
                               const SkPaint& paint) {
    fText = new char[byteLength];
    memcpy(fText, text, byteLength);
    fByteLength = byteLength;
    fPath = path;
    if (NULL != matrix) {
        fMatrix = *matrix;
    } else {
        fMatrix.setIdentity();
    }
    fPaint = paint;
    fDrawType = DRAW_TEXT_ON_PATH;

    fInfo.push(SkObjectParser::TextToString(text, byteLength, paint.getTextEncoding()));
    fInfo.push(SkObjectParser::PathToString(path));
    if (NULL != matrix) {
        fInfo.push(SkObjectParser::MatrixToString(*matrix));
    }
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void DrawTextOnPath::execute(SkCanvas* canvas) {
    canvas->drawTextOnPath(fText, fByteLength, fPath,
                           fMatrix.isIdentity() ? NULL : &fMatrix,
                           fPaint);
}

DrawVertices::DrawVertices(SkCanvas::VertexMode vmode, int vertexCount,
                           const SkPoint vertices[], const SkPoint texs[],
                           const SkColor colors[], SkXfermode* xfermode,
                           const uint16_t indices[], int indexCount,
                           const SkPaint& paint) {
    fVmode = vmode;

    fVertexCount = vertexCount;

    fVertices = new SkPoint[vertexCount];
    memcpy(fVertices, vertices, vertexCount * sizeof(SkPoint));

    if (NULL != texs) {
        fTexs = new SkPoint[vertexCount];
        memcpy(fTexs, texs, vertexCount * sizeof(SkPoint));
    } else {
        fTexs = NULL;
    }

    if (NULL != colors) {
        fColors = new SkColor[vertexCount];
        memcpy(fColors, colors, vertexCount * sizeof(SkColor));
    } else {
        fColors = NULL;
    }

    fXfermode = xfermode;
    if (NULL != fXfermode) {
        fXfermode->ref();
    }

    if (indexCount > 0) {
        fIndices = new uint16_t[indexCount];
        memcpy(fIndices, indices, indexCount * sizeof(uint16_t));
    } else {
        fIndices = NULL;
    }

    fIndexCount = indexCount;
    fPaint = paint;
    fDrawType = DRAW_VERTICES;

    // TODO(chudy)
    fInfo.push(SkObjectParser::CustomTextToString("To be implemented."));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

DrawVertices::~DrawVertices() {
    delete [] fVertices;
    delete [] fTexs;
    delete [] fColors;
    SkSafeUnref(fXfermode);
    delete [] fIndices;
}

void DrawVertices::execute(SkCanvas* canvas) {
    canvas->drawVertices(fVmode, fVertexCount, fVertices,
                         fTexs, fColors, fXfermode, fIndices,
                         fIndexCount, fPaint);
}

Restore::Restore() {
    fDrawType = RESTORE;
    fInfo.push(SkObjectParser::CustomTextToString("No Parameters"));
}

void Restore::execute(SkCanvas* canvas) {
    canvas->restore();
}

void Restore::trackSaveState(int* state) {
    (*state)--;
}

Rotate::Rotate(SkScalar degrees) {
    fDegrees = degrees;
    fDrawType = ROTATE;

    fInfo.push(SkObjectParser::ScalarToString(degrees, "SkScalar degrees: "));
}

void Rotate::execute(SkCanvas* canvas) {
    canvas->rotate(fDegrees);
}

Save::Save(SkCanvas::SaveFlags flags) {
    fFlags = flags;
    fDrawType = SAVE;
    fInfo.push(SkObjectParser::SaveFlagsToString(flags));
}

void Save::execute(SkCanvas* canvas) {
    canvas->save(fFlags);
}

void Save::trackSaveState(int* state) {
    (*state)++;
}

SaveLayer::SaveLayer(const SkRect* bounds, const SkPaint* paint,
                     SkCanvas::SaveFlags flags) {
    if (NULL != bounds) {
        fBounds = *bounds;
    } else {
        fBounds.setEmpty();
    }

    if (NULL != paint) {
        fPaint = *paint;
        fPaintPtr = &fPaint;
    } else {
        fPaintPtr = NULL;
    }
    fFlags = flags;
    fDrawType = SAVE_LAYER;

    if (NULL != bounds) {
        fInfo.push(SkObjectParser::RectToString(*bounds, "Bounds: "));
    }
    if (NULL != paint) {
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }
    fInfo.push(SkObjectParser::SaveFlagsToString(flags));
}

void SaveLayer::execute(SkCanvas* canvas) {
    canvas->saveLayer(fBounds.isEmpty() ? NULL : &fBounds,
                      fPaintPtr,
                      fFlags);
}

void SaveLayer::trackSaveState(int* state) {
    (*state)++;
}

Scale::Scale(SkScalar sx, SkScalar sy) {
    fSx = sx;
    fSy = sy;
    fDrawType = SCALE;

    fInfo.push(SkObjectParser::ScalarToString(sx, "SkScalar sx: "));
    fInfo.push(SkObjectParser::ScalarToString(sy, "SkScalar sy: "));
}

void Scale::execute(SkCanvas* canvas) {
    canvas->scale(fSx, fSy);
}

SetMatrix::SetMatrix(const SkMatrix& matrix) {
    fMatrix = matrix;
    fDrawType = SET_MATRIX;

    fInfo.push(SkObjectParser::MatrixToString(matrix));
}

void SetMatrix::execute(SkCanvas* canvas) {
    canvas->setMatrix(fMatrix);
}

Skew::Skew(SkScalar sx, SkScalar sy) {
    fSx = sx;
    fSy = sy;
    fDrawType = SKEW;

    fInfo.push(SkObjectParser::ScalarToString(sx, "SkScalar sx: "));
    fInfo.push(SkObjectParser::ScalarToString(sy, "SkScalar sy: "));
}

void Skew::execute(SkCanvas* canvas) {
    canvas->skew(fSx, fSy);
}

Translate::Translate(SkScalar dx, SkScalar dy) {
    fDx = dx;
    fDy = dy;
    fDrawType = TRANSLATE;

    fInfo.push(SkObjectParser::ScalarToString(dx, "SkScalar dx: "));
    fInfo.push(SkObjectParser::ScalarToString(dy, "SkScalar dy: "));
}

void Translate::execute(SkCanvas* canvas) {
    canvas->translate(fDx, fDy);
}
