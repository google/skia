
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDrawCommand.h"
#include "SkObjectParser.h"
#include "SkPicture.h"
#include "SkTextBlob.h"
#include "SkTextBlobRunIterator.h"

// TODO(chudy): Refactor into non subclass model.

SkDrawCommand::SkDrawCommand(OpType type)
    : fOpType(type)
    , fVisible(true) {
}

SkDrawCommand::~SkDrawCommand() {
    fInfo.deleteAll();
}

const char* SkDrawCommand::GetCommandString(OpType type) {
    switch (type) {
        case kBeginDrawPicture_OpType: return "BeginDrawPicture";
        case kClipPath_OpType: return "ClipPath";
        case kClipRegion_OpType: return "ClipRegion";
        case kClipRect_OpType: return "ClipRect";
        case kClipRRect_OpType: return "ClipRRect";
        case kConcat_OpType: return "Concat";
        case kDrawBitmap_OpType: return "DrawBitmap";
        case kDrawBitmapNine_OpType: return "DrawBitmapNine";
        case kDrawBitmapRect_OpType: return "DrawBitmapRect";
        case kDrawClear_OpType: return "DrawClear";
        case kDrawDRRect_OpType: return "DrawDRRect";
        case kDrawImage_OpType: return "DrawImage";
        case kDrawImageRect_OpType: return "DrawImageRect";
        case kDrawOval_OpType: return "DrawOval";
        case kDrawPaint_OpType: return "DrawPaint";
        case kDrawPatch_OpType: return "DrawPatch";
        case kDrawPath_OpType: return "DrawPath";
        case kDrawPoints_OpType: return "DrawPoints";
        case kDrawPosText_OpType: return "DrawPosText";
        case kDrawPosTextH_OpType: return "DrawPosTextH";
        case kDrawRect_OpType: return "DrawRect";
        case kDrawRRect_OpType: return "DrawRRect";
        case kDrawText_OpType: return "DrawText";
        case kDrawTextBlob_OpType: return "DrawTextBlob";
        case kDrawTextOnPath_OpType: return "DrawTextOnPath";
        case kDrawVertices_OpType: return "DrawVertices";
        case kEndDrawPicture_OpType: return "EndDrawPicture";
        case kRestore_OpType: return "Restore";
        case kSave_OpType: return "Save";
        case kSaveLayer_OpType: return "SaveLayer";
        case kSetMatrix_OpType: return "SetMatrix";
        default:
            SkDebugf("OpType error 0x%08x\n", type);
            SkASSERT(0);
            break;
    }
    SkDEBUGFAIL("DrawType UNUSED\n");
    return nullptr;
}

SkString SkDrawCommand::toString() const {
    return SkString(GetCommandString(fOpType));
}

SkClearCommand::SkClearCommand(SkColor color) : INHERITED(kDrawClear_OpType) {
    fColor = color;
    fInfo.push(SkObjectParser::CustomTextToString("No Parameters"));
}

void SkClearCommand::execute(SkCanvas* canvas) const {
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

    const SkRect& bounds = path.getBounds();
    if (bounds.isEmpty()) {
        return;
    }

    SkAutoCanvasRestore acr(canvas, true);
    xlate_and_scale_to_bounds(canvas, bounds);

    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setStyle(SkPaint::kStroke_Style);

    canvas->drawPath(path, p);
}

void render_bitmap(SkCanvas* canvas, const SkBitmap& input, const SkRect* srcRect = nullptr) {
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

    static const int kNumBlocks = 8;

    canvas->clear(0xFFFFFFFF);
    SkISize block = {
        canvas->imageInfo().width()/kNumBlocks,
        canvas->imageInfo().height()/kNumBlocks
    };
    for (int y = 0; y < kNumBlocks; ++y) {
        for (int x = 0; x < kNumBlocks; ++x) {
            SkPaint paint;
            paint.setColor((x+y)%2 ? SK_ColorLTGRAY : SK_ColorDKGRAY);
            SkRect r = SkRect::MakeXYWH(SkIntToScalar(x*block.width()),
                                        SkIntToScalar(y*block.height()),
                                        SkIntToScalar(block.width()),
                                        SkIntToScalar(block.height()));
            canvas->drawRect(r, paint);
        }
    }

    canvas->drawBitmapRect(input, dst, nullptr);

    if (srcRect) {
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

void render_drrect(SkCanvas* canvas, const SkRRect& outer, const SkRRect& inner) {
    canvas->clear(0xFFFFFFFF);
    canvas->save();

    const SkRect& bounds = outer.getBounds();

    xlate_and_scale_to_bounds(canvas, bounds);

    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setStyle(SkPaint::kStroke_Style);

    canvas->drawDRRect(outer, inner, p);
    canvas->restore();
}

};


SkClipPathCommand::SkClipPathCommand(const SkPath& path, SkRegion::Op op, bool doAA)
    : INHERITED(kClipPath_OpType) {
    fPath = path;
    fOp = op;
    fDoAA = doAA;

    fInfo.push(SkObjectParser::PathToString(path));
    fInfo.push(SkObjectParser::RegionOpToString(op));
    fInfo.push(SkObjectParser::BoolToString(doAA));
}

void SkClipPathCommand::execute(SkCanvas* canvas) const {
    canvas->clipPath(fPath, fOp, fDoAA);
}

bool SkClipPathCommand::render(SkCanvas* canvas) const {
    render_path(canvas, fPath);
    return true;
}

SkClipRegionCommand::SkClipRegionCommand(const SkRegion& region, SkRegion::Op op)
    : INHERITED(kClipRegion_OpType) {
    fRegion = region;
    fOp = op;

    fInfo.push(SkObjectParser::RegionToString(region));
    fInfo.push(SkObjectParser::RegionOpToString(op));
}

void SkClipRegionCommand::execute(SkCanvas* canvas) const {
    canvas->clipRegion(fRegion, fOp);
}

SkClipRectCommand::SkClipRectCommand(const SkRect& rect, SkRegion::Op op, bool doAA)
    : INHERITED(kClipRect_OpType) {
    fRect = rect;
    fOp = op;
    fDoAA = doAA;

    fInfo.push(SkObjectParser::RectToString(rect));
    fInfo.push(SkObjectParser::RegionOpToString(op));
    fInfo.push(SkObjectParser::BoolToString(doAA));
}

void SkClipRectCommand::execute(SkCanvas* canvas) const {
    canvas->clipRect(fRect, fOp, fDoAA);
}

SkClipRRectCommand::SkClipRRectCommand(const SkRRect& rrect, SkRegion::Op op, bool doAA)
    : INHERITED(kClipRRect_OpType) {
    fRRect = rrect;
    fOp = op;
    fDoAA = doAA;

    fInfo.push(SkObjectParser::RRectToString(rrect));
    fInfo.push(SkObjectParser::RegionOpToString(op));
    fInfo.push(SkObjectParser::BoolToString(doAA));
}

void SkClipRRectCommand::execute(SkCanvas* canvas) const {
    canvas->clipRRect(fRRect, fOp, fDoAA);
}

bool SkClipRRectCommand::render(SkCanvas* canvas) const {
    render_rrect(canvas, fRRect);
    return true;
}

SkConcatCommand::SkConcatCommand(const SkMatrix& matrix)
    : INHERITED(kConcat_OpType) {
    fMatrix = matrix;

    fInfo.push(SkObjectParser::MatrixToString(matrix));
}

void SkConcatCommand::execute(SkCanvas* canvas) const {
    canvas->concat(fMatrix);
}

SkDrawBitmapCommand::SkDrawBitmapCommand(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                                         const SkPaint* paint)
    : INHERITED(kDrawBitmap_OpType) {
    fBitmap = bitmap;
    fLeft = left;
    fTop = top;
    if (paint) {
        fPaint = *paint;
        fPaintPtr = &fPaint;
    } else {
        fPaintPtr = nullptr;
    }

    fInfo.push(SkObjectParser::BitmapToString(bitmap));
    fInfo.push(SkObjectParser::ScalarToString(left, "SkScalar left: "));
    fInfo.push(SkObjectParser::ScalarToString(top, "SkScalar top: "));
    if (paint) {
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }
}

void SkDrawBitmapCommand::execute(SkCanvas* canvas) const {
    canvas->drawBitmap(fBitmap, fLeft, fTop, fPaintPtr);
}

bool SkDrawBitmapCommand::render(SkCanvas* canvas) const {
    render_bitmap(canvas, fBitmap);
    return true;
}

SkDrawBitmapNineCommand::SkDrawBitmapNineCommand(const SkBitmap& bitmap, const SkIRect& center,
                                                 const SkRect& dst, const SkPaint* paint)
    : INHERITED(kDrawBitmapNine_OpType) {
    fBitmap = bitmap;
    fCenter = center;
    fDst = dst;
    if (paint) {
        fPaint = *paint;
        fPaintPtr = &fPaint;
    } else {
        fPaintPtr = nullptr;
    }

    fInfo.push(SkObjectParser::BitmapToString(bitmap));
    fInfo.push(SkObjectParser::IRectToString(center));
    fInfo.push(SkObjectParser::RectToString(dst, "Dst: "));
    if (paint) {
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }
}

void SkDrawBitmapNineCommand::execute(SkCanvas* canvas) const {
    canvas->drawBitmapNine(fBitmap, fCenter, fDst, fPaintPtr);
}

bool SkDrawBitmapNineCommand::render(SkCanvas* canvas) const {
    SkRect tmp = SkRect::Make(fCenter);
    render_bitmap(canvas, fBitmap, &tmp);
    return true;
}

SkDrawBitmapRectCommand::SkDrawBitmapRectCommand(const SkBitmap& bitmap, const SkRect* src,
                                                 const SkRect& dst, const SkPaint* paint,
                                                 SkCanvas::SrcRectConstraint constraint)
    : INHERITED(kDrawBitmapRect_OpType) {
    fBitmap = bitmap;
    if (src) {
        fSrc = *src;
    } else {
        fSrc.setEmpty();
    }
    fDst = dst;

    if (paint) {
        fPaint = *paint;
        fPaintPtr = &fPaint;
    } else {
        fPaintPtr = nullptr;
    }
    fConstraint = constraint;

    fInfo.push(SkObjectParser::BitmapToString(bitmap));
    if (src) {
        fInfo.push(SkObjectParser::RectToString(*src, "Src: "));
    }
    fInfo.push(SkObjectParser::RectToString(dst, "Dst: "));
    if (paint) {
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }
    fInfo.push(SkObjectParser::IntToString(fConstraint, "Constraint: "));
}

void SkDrawBitmapRectCommand::execute(SkCanvas* canvas) const {
    canvas->legacy_drawBitmapRect(fBitmap, this->srcRect(), fDst, fPaintPtr, fConstraint);
}

bool SkDrawBitmapRectCommand::render(SkCanvas* canvas) const {
    render_bitmap(canvas, fBitmap, this->srcRect());
    return true;
}

SkDrawImageCommand::SkDrawImageCommand(const SkImage* image, SkScalar left, SkScalar top,
                                       const SkPaint* paint)
    : INHERITED(kDrawImage_OpType)
    , fImage(SkRef(image))
    , fLeft(left)
    , fTop(top) {

    fInfo.push(SkObjectParser::ImageToString(image));
    fInfo.push(SkObjectParser::ScalarToString(left, "Left: "));
    fInfo.push(SkObjectParser::ScalarToString(top, "Top: "));

    if (paint) {
        fPaint.set(*paint);
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }
}

void SkDrawImageCommand::execute(SkCanvas* canvas) const {
    canvas->drawImage(fImage, fLeft, fTop, fPaint.getMaybeNull());
}

bool SkDrawImageCommand::render(SkCanvas* canvas) const {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->clear(0xFFFFFFFF);

    xlate_and_scale_to_bounds(canvas, SkRect::MakeXYWH(fLeft, fTop,
                                                       SkIntToScalar(fImage->width()),
                                                       SkIntToScalar(fImage->height())));
    this->execute(canvas);
    return true;
}

SkDrawImageRectCommand::SkDrawImageRectCommand(const SkImage* image, const SkRect* src,
                                               const SkRect& dst, const SkPaint* paint,
                                               SkCanvas::SrcRectConstraint constraint)
    : INHERITED(kDrawImageRect_OpType)
    , fImage(SkRef(image))
    , fDst(dst)
    , fConstraint(constraint) {

    if (src) {
        fSrc.set(*src);
    }

    if (paint) {
        fPaint.set(*paint);
    }

    fInfo.push(SkObjectParser::ImageToString(image));
    if (src) {
        fInfo.push(SkObjectParser::RectToString(*src, "Src: "));
    }
    fInfo.push(SkObjectParser::RectToString(dst, "Dst: "));
    if (paint) {
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }
    fInfo.push(SkObjectParser::IntToString(fConstraint, "Constraint: "));
}

void SkDrawImageRectCommand::execute(SkCanvas* canvas) const {
    canvas->legacy_drawImageRect(fImage, fSrc.getMaybeNull(), fDst, fPaint.getMaybeNull(), fConstraint);
}

bool SkDrawImageRectCommand::render(SkCanvas* canvas) const {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->clear(0xFFFFFFFF);

    xlate_and_scale_to_bounds(canvas, fDst);

    this->execute(canvas);
    return true;
}

SkDrawOvalCommand::SkDrawOvalCommand(const SkRect& oval, const SkPaint& paint)
    : INHERITED(kDrawOval_OpType) {
    fOval = oval;
    fPaint = paint;

    fInfo.push(SkObjectParser::RectToString(oval));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawOvalCommand::execute(SkCanvas* canvas) const {
    canvas->drawOval(fOval, fPaint);
}

bool SkDrawOvalCommand::render(SkCanvas* canvas) const {
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

SkDrawPaintCommand::SkDrawPaintCommand(const SkPaint& paint)
    : INHERITED(kDrawPaint_OpType) {
    fPaint = paint;

    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawPaintCommand::execute(SkCanvas* canvas) const {
    canvas->drawPaint(fPaint);
}

bool SkDrawPaintCommand::render(SkCanvas* canvas) const {
    canvas->clear(0xFFFFFFFF);
    canvas->drawPaint(fPaint);
    return true;
}

SkDrawPathCommand::SkDrawPathCommand(const SkPath& path, const SkPaint& paint)
    : INHERITED(kDrawPath_OpType) {
    fPath = path;
    fPaint = paint;

    fInfo.push(SkObjectParser::PathToString(path));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawPathCommand::execute(SkCanvas* canvas) const {
    canvas->drawPath(fPath, fPaint);
}

bool SkDrawPathCommand::render(SkCanvas* canvas) const {
    render_path(canvas, fPath);
    return true;
}

SkBeginDrawPictureCommand::SkBeginDrawPictureCommand(const SkPicture* picture,
                                                     const SkMatrix* matrix,
                                                     const SkPaint* paint)
    : INHERITED(kBeginDrawPicture_OpType)
    , fPicture(SkRef(picture)) {

    SkString* str = new SkString;
    str->appendf("SkPicture: L: %f T: %f R: %f B: %f",
                 picture->cullRect().fLeft, picture->cullRect().fTop,
                 picture->cullRect().fRight, picture->cullRect().fBottom);
    fInfo.push(str);

    if (matrix) {
        fMatrix.set(*matrix);
        fInfo.push(SkObjectParser::MatrixToString(*matrix));
    }

    if (paint) {
        fPaint.set(*paint);
        fInfo.push(SkObjectParser::PaintToString(*paint));
    }

}

void SkBeginDrawPictureCommand::execute(SkCanvas* canvas) const {
    if (fPaint.isValid()) {
        SkRect bounds = fPicture->cullRect();
        if (fMatrix.isValid()) {
            fMatrix.get()->mapRect(&bounds);
        }
        canvas->saveLayer(&bounds, fPaint.get());
    }

    if (fMatrix.isValid()) {
        if (!fPaint.isValid()) {
            canvas->save();
        }
        canvas->concat(*fMatrix.get());
    }
}

bool SkBeginDrawPictureCommand::render(SkCanvas* canvas) const {
    canvas->clear(0xFFFFFFFF);
    canvas->save();

    xlate_and_scale_to_bounds(canvas, fPicture->cullRect());

    canvas->drawPicture(fPicture.get());

    canvas->restore();

    return true;
}

SkEndDrawPictureCommand::SkEndDrawPictureCommand(bool restore)
    : INHERITED(kEndDrawPicture_OpType) , fRestore(restore) { }

void SkEndDrawPictureCommand::execute(SkCanvas* canvas) const {
    if (fRestore) {
        canvas->restore();
    }
}

SkDrawPointsCommand::SkDrawPointsCommand(SkCanvas::PointMode mode, size_t count,
                                         const SkPoint pts[], const SkPaint& paint)
    : INHERITED(kDrawPoints_OpType) {
    fMode = mode;
    fCount = count;
    fPts = new SkPoint[count];
    memcpy(fPts, pts, count * sizeof(SkPoint));
    fPaint = paint;

    fInfo.push(SkObjectParser::PointsToString(pts, count));
    fInfo.push(SkObjectParser::ScalarToString(SkIntToScalar((unsigned int)count),
                                              "Points: "));
    fInfo.push(SkObjectParser::PointModeToString(mode));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawPointsCommand::execute(SkCanvas* canvas) const {
    canvas->drawPoints(fMode, fCount, fPts, fPaint);
}

bool SkDrawPointsCommand::render(SkCanvas* canvas) const {
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

SkDrawPosTextCommand::SkDrawPosTextCommand(const void* text, size_t byteLength,
                                           const SkPoint pos[], const SkPaint& paint)
    : INHERITED(kDrawPosText_OpType) {
    size_t numPts = paint.countText(text, byteLength);

    fText = new char[byteLength];
    memcpy(fText, text, byteLength);
    fByteLength = byteLength;

    fPos = new SkPoint[numPts];
    memcpy(fPos, pos, numPts * sizeof(SkPoint));

    fPaint = paint;

    fInfo.push(SkObjectParser::TextToString(text, byteLength, paint.getTextEncoding()));
    // TODO(chudy): Test that this works.
    fInfo.push(SkObjectParser::PointsToString(pos, 1));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawPosTextCommand::execute(SkCanvas* canvas) const {
    canvas->drawPosText(fText, fByteLength, fPos, fPaint);
}


SkDrawPosTextHCommand::SkDrawPosTextHCommand(const void* text, size_t byteLength,
                                             const SkScalar xpos[], SkScalar constY,
                                             const SkPaint& paint)
    : INHERITED(kDrawPosTextH_OpType) {
    size_t numPts = paint.countText(text, byteLength);

    fText = new char[byteLength];
    memcpy(fText, text, byteLength);
    fByteLength = byteLength;

    fXpos = new SkScalar[numPts];
    memcpy(fXpos, xpos, numPts * sizeof(SkScalar));

    fConstY = constY;
    fPaint = paint;

    fInfo.push(SkObjectParser::TextToString(text, byteLength, paint.getTextEncoding()));
    fInfo.push(SkObjectParser::ScalarToString(xpos[0], "XPOS: "));
    fInfo.push(SkObjectParser::ScalarToString(constY, "SkScalar constY: "));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawPosTextHCommand::execute(SkCanvas* canvas) const {
    canvas->drawPosTextH(fText, fByteLength, fXpos, fConstY, fPaint);
}

static const char* gPositioningLabels[] = {
    "kDefault_Positioning",
    "kHorizontal_Positioning",
    "kFull_Positioning",
};

SkDrawTextBlobCommand::SkDrawTextBlobCommand(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                             const SkPaint& paint)
    : INHERITED(kDrawTextBlob_OpType)
    , fBlob(SkRef(blob))
    , fXPos(x)
    , fYPos(y)
    , fPaint(paint) {

    SkAutoTDelete<SkString> runsStr(new SkString);
    fInfo.push(SkObjectParser::ScalarToString(x, "XPOS: "));
    fInfo.push(SkObjectParser::ScalarToString(y, "YPOS: "));
    fInfo.push(SkObjectParser::RectToString(fBlob->bounds(), "Bounds: "));
    fInfo.push(runsStr);
    fInfo.push(SkObjectParser::PaintToString(paint));

    unsigned runs = 0;
    SkPaint runPaint(paint);
    SkTextBlobRunIterator iter(blob);
    while (!iter.done()) {
        SkAutoTDelete<SkString> tmpStr(new SkString);
        tmpStr->printf("==== Run [%d] ====", runs++);
        fInfo.push(tmpStr.release());

        fInfo.push(SkObjectParser::IntToString(iter.glyphCount(), "GlyphCount: "));
        tmpStr.reset(new SkString("GlyphPositioning: "));
        tmpStr->append(gPositioningLabels[iter.positioning()]);
        fInfo.push(tmpStr.release());

        iter.applyFontToPaint(&runPaint);
        fInfo.push(SkObjectParser::PaintToString(runPaint));

        iter.next();
    }

    runsStr->printf("Runs: %d", runs);
    // runStr is owned by fInfo at this point.
    runsStr.release();
}

void SkDrawTextBlobCommand::execute(SkCanvas* canvas) const {
    canvas->drawTextBlob(fBlob, fXPos, fYPos, fPaint);
}

bool SkDrawTextBlobCommand::render(SkCanvas* canvas) const {
    canvas->clear(SK_ColorWHITE);
    canvas->save();

    SkRect bounds = fBlob->bounds().makeOffset(fXPos, fYPos);
    xlate_and_scale_to_bounds(canvas, bounds);

    canvas->drawTextBlob(fBlob.get(), fXPos, fYPos, fPaint);

    canvas->restore();

    return true;
}

SkDrawPatchCommand::SkDrawPatchCommand(const SkPoint cubics[12], const SkColor colors[4],
                                       const SkPoint texCoords[4], SkXfermode* xfermode,
                                       const SkPaint& paint)
    : INHERITED(kDrawPatch_OpType) {
    memcpy(fCubics, cubics, sizeof(fCubics));
    memcpy(fColors, colors, sizeof(fColors));
    memcpy(fTexCoords, texCoords, sizeof(fTexCoords));
    fXfermode.reset(xfermode);
    fPaint = paint;

    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawPatchCommand::execute(SkCanvas* canvas) const {
    canvas->drawPatch(fCubics, fColors, fTexCoords, fXfermode, fPaint);
}

SkDrawRectCommand::SkDrawRectCommand(const SkRect& rect, const SkPaint& paint)
    : INHERITED(kDrawRect_OpType) {
    fRect = rect;
    fPaint = paint;

    fInfo.push(SkObjectParser::RectToString(rect));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawRectCommand::execute(SkCanvas* canvas) const {
    canvas->drawRect(fRect, fPaint);
}

SkDrawRRectCommand::SkDrawRRectCommand(const SkRRect& rrect, const SkPaint& paint)
    : INHERITED(kDrawRRect_OpType) {
    fRRect = rrect;
    fPaint = paint;

    fInfo.push(SkObjectParser::RRectToString(rrect));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawRRectCommand::execute(SkCanvas* canvas) const {
    canvas->drawRRect(fRRect, fPaint);
}

bool SkDrawRRectCommand::render(SkCanvas* canvas) const {
    render_rrect(canvas, fRRect);
    return true;
}

SkDrawDRRectCommand::SkDrawDRRectCommand(const SkRRect& outer,
                                         const SkRRect& inner,
                                         const SkPaint& paint)
    : INHERITED(kDrawDRRect_OpType) {
    fOuter = outer;
    fInner = inner;
    fPaint = paint;

    fInfo.push(SkObjectParser::RRectToString(outer));
    fInfo.push(SkObjectParser::RRectToString(inner));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawDRRectCommand::execute(SkCanvas* canvas) const {
    canvas->drawDRRect(fOuter, fInner, fPaint);
}

bool SkDrawDRRectCommand::render(SkCanvas* canvas) const {
    render_drrect(canvas, fOuter, fInner);
    return true;
}

SkDrawTextCommand::SkDrawTextCommand(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                                     const SkPaint& paint)
    : INHERITED(kDrawText_OpType) {
    fText = new char[byteLength];
    memcpy(fText, text, byteLength);
    fByteLength = byteLength;
    fX = x;
    fY = y;
    fPaint = paint;

    fInfo.push(SkObjectParser::TextToString(text, byteLength, paint.getTextEncoding()));
    fInfo.push(SkObjectParser::ScalarToString(x, "SkScalar x: "));
    fInfo.push(SkObjectParser::ScalarToString(y, "SkScalar y: "));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawTextCommand::execute(SkCanvas* canvas) const {
    canvas->drawText(fText, fByteLength, fX, fY, fPaint);
}

SkDrawTextOnPathCommand::SkDrawTextOnPathCommand(const void* text, size_t byteLength,
                                                 const SkPath& path, const SkMatrix* matrix,
                                                 const SkPaint& paint)
    : INHERITED(kDrawTextOnPath_OpType) {
    fText = new char[byteLength];
    memcpy(fText, text, byteLength);
    fByteLength = byteLength;
    fPath = path;
    if (matrix) {
        fMatrix = *matrix;
    } else {
        fMatrix.setIdentity();
    }
    fPaint = paint;

    fInfo.push(SkObjectParser::TextToString(text, byteLength, paint.getTextEncoding()));
    fInfo.push(SkObjectParser::PathToString(path));
    if (matrix) {
        fInfo.push(SkObjectParser::MatrixToString(*matrix));
    }
    fInfo.push(SkObjectParser::PaintToString(paint));
}

void SkDrawTextOnPathCommand::execute(SkCanvas* canvas) const {
    canvas->drawTextOnPath(fText, fByteLength, fPath,
                           fMatrix.isIdentity() ? nullptr : &fMatrix,
                           fPaint);
}

SkDrawVerticesCommand::SkDrawVerticesCommand(SkCanvas::VertexMode vmode, int vertexCount,
                                             const SkPoint vertices[], const SkPoint texs[],
                                             const SkColor colors[], SkXfermode* xfermode,
                                             const uint16_t indices[], int indexCount,
                                             const SkPaint& paint)
    : INHERITED(kDrawVertices_OpType) {
    fVmode = vmode;

    fVertexCount = vertexCount;

    fVertices = new SkPoint[vertexCount];
    memcpy(fVertices, vertices, vertexCount * sizeof(SkPoint));

    if (texs) {
        fTexs = new SkPoint[vertexCount];
        memcpy(fTexs, texs, vertexCount * sizeof(SkPoint));
    } else {
        fTexs = nullptr;
    }

    if (colors) {
        fColors = new SkColor[vertexCount];
        memcpy(fColors, colors, vertexCount * sizeof(SkColor));
    } else {
        fColors = nullptr;
    }

    fXfermode = xfermode;
    if (fXfermode) {
        fXfermode->ref();
    }

    if (indexCount > 0) {
        fIndices = new uint16_t[indexCount];
        memcpy(fIndices, indices, indexCount * sizeof(uint16_t));
    } else {
        fIndices = nullptr;
    }

    fIndexCount = indexCount;
    fPaint = paint;

    // TODO(chudy)
    fInfo.push(SkObjectParser::CustomTextToString("To be implemented."));
    fInfo.push(SkObjectParser::PaintToString(paint));
}

SkDrawVerticesCommand::~SkDrawVerticesCommand() {
    delete [] fVertices;
    delete [] fTexs;
    delete [] fColors;
    SkSafeUnref(fXfermode);
    delete [] fIndices;
}

void SkDrawVerticesCommand::execute(SkCanvas* canvas) const {
    canvas->drawVertices(fVmode, fVertexCount, fVertices,
                         fTexs, fColors, fXfermode, fIndices,
                         fIndexCount, fPaint);
}

SkRestoreCommand::SkRestoreCommand()
    : INHERITED(kRestore_OpType) {
    fInfo.push(SkObjectParser::CustomTextToString("No Parameters"));
}

void SkRestoreCommand::execute(SkCanvas* canvas) const {
    canvas->restore();
}

SkSaveCommand::SkSaveCommand()
    : INHERITED(kSave_OpType) {
}

void SkSaveCommand::execute(SkCanvas* canvas) const {
    canvas->save();
}

SkSaveLayerCommand::SkSaveLayerCommand(const SkCanvas::SaveLayerRec& rec)
    : INHERITED(kSaveLayer_OpType) {
    if (rec.fBounds) {
        fBounds = *rec.fBounds;
    } else {
        fBounds.setEmpty();
    }

    if (rec.fPaint) {
        fPaint = *rec.fPaint;
        fPaintPtr = &fPaint;
    } else {
        fPaintPtr = nullptr;
    }
    fSaveLayerFlags = rec.fSaveLayerFlags;

    if (rec.fBounds) {
        fInfo.push(SkObjectParser::RectToString(*rec.fBounds, "Bounds: "));
    }
    if (rec.fPaint) {
        fInfo.push(SkObjectParser::PaintToString(*rec.fPaint));
    }
    fInfo.push(SkObjectParser::SaveLayerFlagsToString(fSaveLayerFlags));
}

void SkSaveLayerCommand::execute(SkCanvas* canvas) const {
    canvas->saveLayer(SkCanvas::SaveLayerRec(fBounds.isEmpty() ? nullptr : &fBounds,
                                             fPaintPtr,
                                             fSaveLayerFlags));
}

void SkSaveLayerCommand::vizExecute(SkCanvas* canvas) const {
    canvas->save();
}

SkSetMatrixCommand::SkSetMatrixCommand(const SkMatrix& matrix)
    : INHERITED(kSetMatrix_OpType) {
    fUserMatrix.reset();
    fMatrix = matrix;

    fInfo.push(SkObjectParser::MatrixToString(matrix));
}

void SkSetMatrixCommand::setUserMatrix(const SkMatrix& userMatrix) {
    fUserMatrix = userMatrix;
}

void SkSetMatrixCommand::execute(SkCanvas* canvas) const {
    SkMatrix temp = SkMatrix::Concat(fUserMatrix, fMatrix);
    canvas->setMatrix(temp);
}

