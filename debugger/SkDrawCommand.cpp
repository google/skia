
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDrawCommand.h"
#include "SkObjectParser.h"

// TODO(chudy): Refactor into non subclass model.

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
        case CONCAT: return "Concat";
        case DRAW_BITMAP: return "Draw Bitmap";
        case DRAW_BITMAP_MATRIX: return "Draw Bitmap Matrix";
        case DRAW_BITMAP_NINE: return "Draw Bitmap Nine";
        case DRAW_BITMAP_RECT: return "Draw Bitmap Rect";
        case DRAW_DATA: return "Draw Data";
        case DRAW_PAINT: return "Draw Paint";
        case DRAW_PATH: return "Draw Path";
        case DRAW_PICTURE: return "Draw Picture";
        case DRAW_POINTS: return "Draw Points";
        case DRAW_POS_TEXT: return "Draw Pos Text";
        case DRAW_POS_TEXT_H: return "Draw Pos Text H";
        case DRAW_RECT: return "Draw Rect";
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
    this->fColor = color;
    this->fDrawType = DRAW_CLEAR;
    this->fInfo.push(SkObjectParser::CustomTextToString("No Parameters"));
}

void Clear::execute(SkCanvas* canvas) {
    canvas->clear(this->fColor);
}

ClipPath::ClipPath(const SkPath& path, SkRegion::Op op, bool doAA) {
    this->fPath = &path;
    this->fOp = op;
    this->fDoAA = doAA;
    this->fDrawType = CLIP_PATH;

    this->fInfo.push(SkObjectParser::PathToString(path));
    this->fInfo.push(SkObjectParser::RegionOpToString(op));
    this->fInfo.push(SkObjectParser::BoolToString(doAA));
}

void ClipPath::execute(SkCanvas* canvas) {
    canvas->clipPath(*this->fPath, this->fOp, this->fDoAA);
}

ClipRegion::ClipRegion(const SkRegion& region, SkRegion::Op op) {
    this->fRegion = &region;
    this->fOp = op;
    this->fDrawType = CLIP_REGION;

    this->fInfo.push(SkObjectParser::RegionToString(region));
    this->fInfo.push(SkObjectParser::RegionOpToString(op));
}

void ClipRegion::execute(SkCanvas* canvas) {
    canvas->clipRegion(*this->fRegion, this->fOp);
}

ClipRect::ClipRect(const SkRect& rect, SkRegion::Op op, bool doAA) {
    this->fRect = &rect;
    this->fOp = op;
    this->fDoAA = doAA;
    this->fDrawType = CLIP_RECT;

    this->fInfo.push(SkObjectParser::RectToString(rect));
    this->fInfo.push(SkObjectParser::RegionOpToString(op));
    this->fInfo.push(SkObjectParser::BoolToString(doAA));
}

void ClipRect::execute(SkCanvas* canvas) {
    canvas->clipRect(*this->fRect, this->fOp, this->fDoAA);
}

Concat::Concat(const SkMatrix& matrix) {
    this->fMatrix = &matrix;
    this->fDrawType = CONCAT;

    this->fInfo.push(SkObjectParser::MatrixToString(matrix));
}

void Concat::execute(SkCanvas* canvas) {
    canvas->concat(*this->fMatrix);
}

DrawBitmap::DrawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
        const SkPaint* paint) {
    this->fBitmap = &bitmap;
    this->fLeft = left;
    this->fTop = top;
    this->fPaint = paint;
    this->fDrawType = DRAW_BITMAP;

    this->fInfo.push(SkObjectParser::BitmapToString(bitmap));
    this->fInfo.push(SkObjectParser::ScalarToString(left, "SkScalar left: "));
    this->fInfo.push(SkObjectParser::ScalarToString(top, "SkScalar top: "));
}

void DrawBitmap::execute(SkCanvas* canvas) {
    canvas->drawBitmap(*this->fBitmap, this->fLeft, this->fTop, this->fPaint);
}

DrawBitmapMatrix::DrawBitmapMatrix(const SkBitmap& bitmap,
        const SkMatrix& matrix, const SkPaint* paint) {
    this->fBitmap = &bitmap;
    this->fMatrix = &matrix;
    this->fPaint = paint;
    this->fDrawType = DRAW_BITMAP_MATRIX;

    this->fInfo.push(SkObjectParser::BitmapToString(bitmap));
    this->fInfo.push(SkObjectParser::MatrixToString(matrix));
    if (paint) this->fInfo.push(SkObjectParser::PaintToString(*paint));
}

void DrawBitmapMatrix::execute(SkCanvas* canvas) {
    canvas->drawBitmapMatrix(*this->fBitmap, *this->fMatrix, this->fPaint);
}

DrawBitmapNine::DrawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
        const SkRect& dst, const SkPaint* paint) {
    this->fBitmap = &bitmap;
    this->fCenter = &center;
    this->fDst = &dst;
    this->fPaint = paint;
    this->fDrawType = DRAW_BITMAP_NINE;

    this->fInfo.push(SkObjectParser::BitmapToString(bitmap));
    this->fInfo.push(SkObjectParser::IRectToString(center));
    this->fInfo.push(SkObjectParser::RectToString(dst));
    if (paint) this->fInfo.push(SkObjectParser::PaintToString(*paint));
}

void DrawBitmapNine::execute(SkCanvas* canvas) {
    canvas->drawBitmapNine(*this->fBitmap, *this->fCenter, *this->fDst, this->fPaint);
}

DrawBitmapRect::DrawBitmapRect(const SkBitmap& bitmap, const SkRect* src,
        const SkRect& dst, const SkPaint* paint) {
    this->fBitmap = &bitmap;
    this->fSrc = src;
    this->fDst = &dst;
    this->fPaint = paint;
    this->fDrawType = DRAW_BITMAP_RECT;

    this->fInfo.push(SkObjectParser::BitmapToString(bitmap));
    if (src) this->fInfo.push(SkObjectParser::RectToString(*src));
    this->fInfo.push(SkObjectParser::RectToString(dst));
    if (paint) this->fInfo.push(SkObjectParser::PaintToString(*paint));
}

void DrawBitmapRect::execute(SkCanvas* canvas) {
    canvas->drawBitmapRect(*this->fBitmap, this->fSrc, *this->fDst, this->fPaint);
}

DrawData::DrawData(const void* data, size_t length) {
    this->fData = data;
    this->fLength = length;
    this->fDrawType = DRAW_DATA;
    // TODO(chudy): See if we can't display data and length.
}

void DrawData::execute(SkCanvas* canvas) {
    canvas->drawData(this->fData, this->fLength);
}

DrawPaint::DrawPaint(const SkPaint& paint) {
    this->fPaint = &paint;
    this->fDrawType = DRAW_PAINT;

    this->fInfo.push(SkObjectParser::PaintToString(paint));
}

void DrawPaint::execute(SkCanvas* canvas) {
    canvas->drawPaint(*this->fPaint);
}

DrawPath::DrawPath(const SkPath& path, const SkPaint& paint) {
    this->fPath = &path;
    this->fPaint = &paint;
    this->fDrawType = DRAW_PATH;

    this->fInfo.push(SkObjectParser::PathToString(path));
    this->fInfo.push(SkObjectParser::PaintToString(paint));
}

void DrawPath::execute(SkCanvas* canvas) {
    canvas->drawPath(*this->fPath, *this->fPaint);
}

DrawPicture::DrawPicture(SkPicture& picture) {
    this->fPicture = &picture;
    this->fDrawType = DRAW_PICTURE;
    this->fInfo.push(SkObjectParser::CustomTextToString("To be implemented."));
}

void DrawPicture::execute(SkCanvas* canvas) {
    canvas->drawPicture(*this->fPicture);
}

DrawPoints::DrawPoints(SkCanvas::PointMode mode, size_t count,
        const SkPoint pts[], const SkPaint& paint) {
    this->fMode = mode;
    this->fCount = count;
    this->fPts = pts;
    this->fPaint = &paint;
    this->fDrawType = DRAW_POINTS;

    this->fInfo.push(SkObjectParser::PointsToString(pts, count));
    this->fInfo.push(SkObjectParser::ScalarToString(SkIntToScalar(count),
                                                    "Points: "));
    this->fInfo.push(SkObjectParser::PointModeToString(mode));
}

void DrawPoints::execute(SkCanvas* canvas) {
    canvas->drawPoints(this->fMode, this->fCount, this->fPts, *this->fPaint);
}

DrawPosText::DrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
        const SkPaint& paint) {
    this->fText = text;
    this->fByteLength = byteLength;
    this->fPos = pos;
    this->fPaint = &paint;
    this->fDrawType = DRAW_POS_TEXT;

    this->fInfo.push(SkObjectParser::TextToString(text, byteLength));
    // TODO(chudy): Test that this works.
    this->fInfo.push(SkObjectParser::PointsToString(pos, 1));
    this->fInfo.push(SkObjectParser::PaintToString(paint));
}

void DrawPosText::execute(SkCanvas* canvas) {
    canvas->drawPosText(this->fText, this->fByteLength, this->fPos, *this->fPaint);
}


DrawPosTextH::DrawPosTextH(const void* text, size_t byteLength,
        const SkScalar xpos[], SkScalar constY, const SkPaint& paint) {
    this->fText = text;
    this->fByteLength = byteLength;
    this->fXpos = xpos;
    this->fConstY = constY;
    this->fPaint = &paint;
    this->fDrawType = DRAW_POS_TEXT_H;

    this->fInfo.push(SkObjectParser::TextToString(text, byteLength));
    this->fInfo.push(SkObjectParser::ScalarToString(xpos[0], "XPOS: "));
    this->fInfo.push(SkObjectParser::ScalarToString(constY, "SkScalar constY: "));
    this->fInfo.push(SkObjectParser::PaintToString(paint));
}

void DrawPosTextH::execute(SkCanvas* canvas) {
    canvas->drawPosTextH(this->fText, this->fByteLength, this->fXpos, this->fConstY,
            *this->fPaint);
}

DrawRectC::DrawRectC(const SkRect& rect, const SkPaint& paint) {
    this->fRect = &rect;
    this->fPaint = &paint;
    this->fDrawType = DRAW_RECT;

    this->fInfo.push(SkObjectParser::RectToString(rect));
    this->fInfo.push(SkObjectParser::PaintToString(paint));
}

void DrawRectC::execute(SkCanvas* canvas) {
    canvas->drawRect(*this->fRect, *this->fPaint);
}

DrawSprite::DrawSprite(const SkBitmap& bitmap, int left, int top,
        const SkPaint* paint) {
    this->fBitmap = &bitmap;
    this->fLeft = left;
    this->fTop = top;
    this->fPaint = paint;
    this->fDrawType = DRAW_SPRITE;

    this->fInfo.push(SkObjectParser::BitmapToString(bitmap));
    this->fInfo.push(SkObjectParser::IntToString(left, "Left: "));
    this->fInfo.push(SkObjectParser::IntToString(top, "Top: "));
}

void DrawSprite::execute(SkCanvas* canvas) {
    canvas->drawSprite(*this->fBitmap, this->fLeft, this->fTop, this->fPaint);
}

DrawTextC::DrawTextC(const void* text, size_t byteLength, SkScalar x, SkScalar y,
        const SkPaint& paint) {
    this->fText = text;
    this->fByteLength = byteLength;
    this->fX = x;
    this->fY = y;
    this->fPaint = &paint;
    this->fDrawType = DRAW_TEXT;

    this->fInfo.push(SkObjectParser::TextToString(text, byteLength));
    this->fInfo.push(SkObjectParser::ScalarToString(x, "SkScalar x: "));
    this->fInfo.push(SkObjectParser::ScalarToString(y, "SkScalar y: "));
    this->fInfo.push(SkObjectParser::PaintToString(paint));
}

void DrawTextC::execute(SkCanvas* canvas) {
    canvas->drawText(this->fText, this->fByteLength, this->fX, this->fY, *this->fPaint);
}

DrawTextOnPath::DrawTextOnPath(const void* text, size_t byteLength,
        const SkPath& path, const SkMatrix* matrix, const SkPaint& paint) {
    this->fText = text;
    this->fByteLength = byteLength;
    this->fPath = &path;
    this->fMatrix = matrix;
    this->fPaint = &paint;
    this->fDrawType = DRAW_TEXT_ON_PATH;

    this->fInfo.push(SkObjectParser::TextToString(text, byteLength));
    this->fInfo.push(SkObjectParser::PathToString(path));
    if (matrix) this->fInfo.push(SkObjectParser::MatrixToString(*matrix));
    this->fInfo.push(SkObjectParser::PaintToString(paint));
}

void DrawTextOnPath::execute(SkCanvas* canvas) {
    canvas->drawTextOnPath(this->fText, this->fByteLength, *this->fPath,
            this->fMatrix, *this->fPaint);
}

DrawVertices::DrawVertices(SkCanvas::VertexMode vmode, int vertexCount,
        const SkPoint vertices[], const SkPoint texs[], const SkColor colors[],
        SkXfermode* xfermode, const uint16_t indices[], int indexCount,
        const SkPaint& paint) {
    this->fVmode = vmode;
    this->fVertexCount = vertexCount;
    this->fTexs = texs;
    this->fColors = colors;
    this->fXfermode = xfermode;
    this->fIndices = indices;
    this->fIndexCount = indexCount;
    this->fPaint = &paint;
    this->fDrawType = DRAW_VERTICES;
    // TODO(chudy)
    this->fInfo.push(SkObjectParser::CustomTextToString("To be implemented."));
}

void DrawVertices::execute(SkCanvas* canvas) {
    canvas->drawVertices(this->fVmode, this->fVertexCount, this->fVertices,
            this->fTexs, this->fColors, this->fXfermode, this->fIndices,
            this->fIndexCount, *this->fPaint);
}

Restore::Restore() {
    this->fDrawType = RESTORE;
    this->fInfo.push(SkObjectParser::CustomTextToString("No Parameters"));
}

void Restore::execute(SkCanvas* canvas) {
    canvas->restore();
}

Rotate::Rotate(SkScalar degrees) {
    this->fDegrees = degrees;
    this->fDrawType = ROTATE;

    this->fInfo.push(SkObjectParser::ScalarToString(degrees, "SkScalar degrees: "));
}

void Rotate::execute(SkCanvas* canvas) {
    canvas->rotate(this->fDegrees);
}

Save::Save(SkCanvas::SaveFlags flags) {
    this->fFlags = flags;
    this->fDrawType = SAVE;
    this->fInfo.push(SkObjectParser::SaveFlagsToString(flags));
}

void Save::execute(SkCanvas* canvas) {
    canvas->save(this->fFlags);
}

SaveLayer::SaveLayer(const SkRect* bounds, const SkPaint* paint,
        SkCanvas::SaveFlags flags) {
    this->fBounds = bounds;
    this->fPaint = paint;
    this->fFlags = flags;
    this->fDrawType = SAVE_LAYER;

    if (bounds) this->fInfo.push(SkObjectParser::RectToString(*bounds));
    if (paint) this->fInfo.push(SkObjectParser::PaintToString(*paint));
    this->fInfo.push(SkObjectParser::SaveFlagsToString(flags));
}

void SaveLayer::execute(SkCanvas* canvas) {
    canvas->saveLayer(this->fBounds, this->fPaint, this->fFlags);
}

Scale::Scale(SkScalar sx, SkScalar sy) {
    this->fSx = sx;
    this->fSy = sy;
    this->fDrawType = SCALE;

    this->fInfo.push(SkObjectParser::ScalarToString(sx, "SkScalar sx: "));
    this->fInfo.push(SkObjectParser::ScalarToString(sy, "SkScalar sy: "));
}

void Scale::execute(SkCanvas* canvas) {
    canvas->scale(this->fSx, this->fSy);
}

SetMatrix::SetMatrix(const SkMatrix& matrix) {
    this->fMatrix = &matrix;
    this->fDrawType = SET_MATRIX;

    this->fInfo.push(SkObjectParser::MatrixToString(matrix));
}

void SetMatrix::execute(SkCanvas* canvas) {
    canvas->setMatrix(*this->fMatrix);
}

Skew::Skew(SkScalar sx, SkScalar sy) {
    this->fSx = sx;
    this->fSy = sy;
    this->fDrawType = SKEW;

    this->fInfo.push(SkObjectParser::ScalarToString(sx, "SkScalar sx: "));
    this->fInfo.push(SkObjectParser::ScalarToString(sy, "SkScalar sy: "));
}

void Skew::execute(SkCanvas* canvas) {
    canvas->skew(this->fSx, this->fSy);
}

Translate::Translate(SkScalar dx, SkScalar dy) {
    this->fDx = dx;
    this->fDy = dy;
    this->fDrawType = TRANSLATE;

    this->fInfo.push(SkObjectParser::ScalarToString(dx, "SkScalar dx: "));
    this->fInfo.push(SkObjectParser::ScalarToString(dy, "SkScalar dy: "));
}

void Translate::execute(SkCanvas* canvas) {
    canvas->translate(this->fDx, this->fDy);
}
