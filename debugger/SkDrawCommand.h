
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKDRAWCOMMAND_H_
#define SKDRAWCOMMAND_H_

#include <iostream>
#include "SkPictureFlat.h"
#include "SkCanvas.h"
#include <sstream>
#include <vector>

class SkDrawCommand {
public:
    /* TODO(chudy): Remove subclasses. */
    SkDrawCommand();

    virtual ~SkDrawCommand();

    virtual std::string toString();

    virtual const char* toCString() {
        return GetCommandString(fDrawType);
    }

    bool getVisibility() const { return fVisible; }
    void setVisibility(bool toggle) {fVisible = toggle; }

    std::vector<std::string>* Info() {return &fInfo; };
    virtual void execute(SkCanvas* canvas)=0;
    DrawType getType() { return fDrawType; };

protected:
    DrawType fDrawType;
    std::vector<std::string> fInfo;

private:
    bool fVisible;
    static const char* GetCommandString(DrawType type);
};

class Restore : public SkDrawCommand {
public:
    Restore();
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
};

class Clear : public SkDrawCommand {
public:
    Clear(SkColor color);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    SkColor fColor;
};

class ClipPath : public SkDrawCommand {
public:
    ClipPath(const SkPath& path, SkRegion::Op op, bool doAA);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    const SkPath* fPath;
    SkRegion::Op fOp;
    bool fDoAA;
};

class ClipRegion : public SkDrawCommand {
public:
    ClipRegion(const SkRegion& region, SkRegion::Op op);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    const SkRegion* fRegion;
    SkRegion::Op fOp;
};

class ClipRect : public SkDrawCommand {
public:
    ClipRect(const SkRect& rect, SkRegion::Op op, bool doAA);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    const SkRect* fRect;
    SkRegion::Op fOp;
    bool fDoAA;
};

class Concat : public SkDrawCommand {
public:
    Concat(const SkMatrix& matrix);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    const SkMatrix* fMatrix;
};

class DrawBitmap : public SkDrawCommand {
public:
    DrawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
            const SkPaint* paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    const SkPaint* fPaint;
    const SkBitmap* fBitmap;
    SkScalar fLeft;
    SkScalar fTop;
};

class DrawBitmapMatrix : public SkDrawCommand {
public:
    DrawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& matrix,
            const SkPaint* paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    const SkPaint* fPaint;
    const SkBitmap* fBitmap;
    const SkMatrix* fMatrix;
};

class DrawBitmapNine : public SkDrawCommand {
public:
    DrawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
            const SkRect& dst, const SkPaint* paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    const SkBitmap* fBitmap;
    const SkIRect* fCenter;
    const SkRect* fDst;
    const SkPaint* fPaint;
};

class DrawBitmapRect : public SkDrawCommand {
public:
    DrawBitmapRect(const SkBitmap& bitmap, const SkIRect* src,
            const SkRect& dst, const SkPaint* paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    const SkIRect* fSrc;
    const SkPaint* fPaint;
    const SkBitmap* fBitmap;
    const SkRect* fDst;
};

class DrawData : public SkDrawCommand {
public:
    DrawData(const void* data, size_t length);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    const void* fData;
    size_t fLength;
};

class DrawPaint : public SkDrawCommand {
public:
    DrawPaint(const SkPaint& paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    const SkPaint* fPaint;
};

class DrawPath : public SkDrawCommand {
public:
    DrawPath(const SkPath& path, const SkPaint& paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    const SkPath* fPath;
    const SkPaint* fPaint;
};

class DrawPicture : public SkDrawCommand {
public:
    DrawPicture(SkPicture& picture);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    SkPicture* fPicture;
};

class DrawPoints : public SkDrawCommand {
public:
    DrawPoints(SkCanvas::PointMode mode, size_t count, const SkPoint pts[],
            const SkPaint& paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    const SkPoint* fPts;
    SkCanvas::PointMode fMode;
    size_t fCount;
    const SkPaint* fPaint;
};

/* TODO(chudy): DrawText is a predefined macro and was breaking something
 * in the windows build of the debugger.
 */
class DrawTextC : public SkDrawCommand {
public:
    DrawTextC(const void* text, size_t byteLength, SkScalar x, SkScalar y,
            const SkPaint& paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    const void* fText;
    size_t fByteLength;
    SkScalar fX;
    SkScalar fY;
    const SkPaint* fPaint;
};

class DrawPosText : public SkDrawCommand {
public:
    DrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
            const SkPaint& paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    const SkPoint* fPos;
    const void* fText;
    size_t fByteLength;
    const SkPaint* fPaint;
};

class DrawTextOnPath : public SkDrawCommand {
public:
    DrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
            const SkMatrix* matrix, const SkPaint& paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    const SkMatrix* fMatrix;
    const void* fText;
    size_t fByteLength;
    const SkPath* fPath;
    const SkPaint* fPaint;
};

class DrawPosTextH : public SkDrawCommand {
public:
    DrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
            SkScalar constY, const SkPaint& paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    const SkScalar* fXpos;
    const void* fText;
    size_t fByteLength;
    SkScalar fConstY;
    const SkPaint* fPaint;
};

class DrawRectC : public SkDrawCommand {
public:
    DrawRectC(const SkRect& rect, const SkPaint& paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    const SkRect* fRect;
    const SkPaint* fPaint;
};

class DrawSprite : public SkDrawCommand {
public:
    DrawSprite(const SkBitmap& bitmap, int left, int top, const SkPaint* paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    const SkPaint* fPaint;
    int fLeft;
    int fTop;
    const SkBitmap* fBitmap;
};

class DrawVertices : public SkDrawCommand {
public:
    DrawVertices(SkCanvas::VertexMode vmode, int vertexCount,
            const SkPoint vertices[], const SkPoint texs[], const SkColor colors[],
            SkXfermode* xfermode, const uint16_t indices[], int indexCount,
            const SkPaint& paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    SkCanvas::VertexMode fVmode;
    int fVertexCount;
    int fIndexCount;
    const SkPoint* fVertices;
    const SkPoint* fTexs;
    const SkColor* fColors;
    const uint16_t* fIndices;
    SkXfermode* fXfermode;
    const SkPaint* fPaint;
};

class Rotate : public SkDrawCommand {
public:
    Rotate(SkScalar degrees);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    SkScalar fDegrees;
};

class Save : public SkDrawCommand {
public:
    Save(SkCanvas::SaveFlags flags);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    SkCanvas::SaveFlags fFlags;
};

class SaveLayer : public SkDrawCommand {
public:
    SaveLayer(const SkRect* bounds, const SkPaint* paint,
            SkCanvas::SaveFlags flags);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    const SkRect* fBounds;
    const SkPaint* fPaint;
    SkCanvas::SaveFlags fFlags;
};

class Scale : public SkDrawCommand {
public:
    Scale(SkScalar sx, SkScalar sy);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    SkScalar fSx;
    SkScalar fSy;
};

class SetMatrix : public SkDrawCommand {
public:
    SetMatrix(const SkMatrix& matrix);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    const SkMatrix* fMatrix;
};

class Skew : public SkDrawCommand {
public:
    Skew(SkScalar sx, SkScalar sy);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    SkScalar fSx;
    SkScalar fSy;
};

class Translate : public SkDrawCommand {
public:
    Translate(SkScalar dx, SkScalar dy);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    SkScalar fDx;
    SkScalar fDy;
};

#endif
