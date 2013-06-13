
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKDRAWCOMMAND_H_
#define SKDRAWCOMMAND_H_

#include "SkPictureFlat.h"
#include "SkCanvas.h"
#include "SkString.h"

class SK_API SkDrawCommand {
public:
    /* TODO(chudy): Remove subclasses. */
    SkDrawCommand(DrawType drawType);
    SkDrawCommand();

    virtual ~SkDrawCommand();

    virtual SkString toString();

    virtual const char* toCString() {
        return GetCommandString(fDrawType);
    }

    bool isVisible() const {
        return fVisible;
    }

    void setVisible(bool toggle) {
        fVisible = toggle;
    }

    SkTDArray<SkString*>* Info() {return &fInfo; };
    virtual void execute(SkCanvas* canvas)=0;
    /** Does nothing by default, but used by save() and restore()-type
        subclassse to track unresolved save() calls. */
    virtual void trackSaveState(int* state) { };
    DrawType getType() { return fDrawType; };

    virtual bool render(SkCanvas* canvas) const { return false; }

    static const char* GetCommandString(DrawType type);

protected:
    DrawType fDrawType;
    SkTDArray<SkString*> fInfo;

private:
    bool fVisible;
};

class Restore : public SkDrawCommand {
public:
    Restore();
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
    virtual void trackSaveState(int* state) SK_OVERRIDE;

private:
    typedef SkDrawCommand INHERITED;
};

class Clear : public SkDrawCommand {
public:
    Clear(SkColor color);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    SkColor fColor;

    typedef SkDrawCommand INHERITED;
};

class ClipPath : public SkDrawCommand {
public:
    ClipPath(const SkPath& path, SkRegion::Op op, bool doAA);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
    virtual bool render(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkPath       fPath;
    SkRegion::Op fOp;
    bool         fDoAA;

    typedef SkDrawCommand INHERITED;
};

class ClipRegion : public SkDrawCommand {
public:
    ClipRegion(const SkRegion& region, SkRegion::Op op);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    SkRegion     fRegion;
    SkRegion::Op fOp;

    typedef SkDrawCommand INHERITED;
};

class ClipRect : public SkDrawCommand {
public:
    ClipRect(const SkRect& rect, SkRegion::Op op, bool doAA);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;

    const SkRect& rect() const { return fRect; }
    SkRegion::Op op() const { return fOp; }
    bool doAA() const { return fDoAA; }

private:
    SkRect       fRect;
    SkRegion::Op fOp;
    bool         fDoAA;

    typedef SkDrawCommand INHERITED;
};

class ClipRRect : public SkDrawCommand {
public:
    ClipRRect(const SkRRect& rrect, SkRegion::Op op, bool doAA);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
    virtual bool render(SkCanvas* canvas) const SK_OVERRIDE;

    const SkRRect& rrect() const { return fRRect; }
    SkRegion::Op op() const { return fOp; }
    bool doAA() const { return fDoAA; }

private:
    SkRRect      fRRect;
    SkRegion::Op fOp;
    bool         fDoAA;

    typedef SkDrawCommand INHERITED;
};

class Concat : public SkDrawCommand {
public:
    Concat(const SkMatrix& matrix);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    SkMatrix fMatrix;

    typedef SkDrawCommand INHERITED;
};

class DrawBitmap : public SkDrawCommand {
public:
    DrawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
               const SkPaint* paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
    virtual bool render(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkBitmap fBitmap;
    SkScalar fLeft;
    SkScalar fTop;
    SkPaint  fPaint;
    SkPaint* fPaintPtr;

    typedef SkDrawCommand INHERITED;
};

class DrawBitmapMatrix : public SkDrawCommand {
public:
    DrawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& matrix,
                     const SkPaint* paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
    virtual bool render(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkBitmap fBitmap;
    SkMatrix fMatrix;
    SkPaint  fPaint;
    SkPaint* fPaintPtr;

    typedef SkDrawCommand INHERITED;
};

class DrawBitmapNine : public SkDrawCommand {
public:
    DrawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                   const SkRect& dst, const SkPaint* paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
    virtual bool render(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkBitmap fBitmap;
    SkIRect  fCenter;
    SkRect   fDst;
    SkPaint  fPaint;
    SkPaint* fPaintPtr;

    typedef SkDrawCommand INHERITED;
};

class DrawBitmapRect : public SkDrawCommand {
public:
    DrawBitmapRect(const SkBitmap& bitmap, const SkRect* src,
                   const SkRect& dst, const SkPaint* paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
    virtual bool render(SkCanvas* canvas) const SK_OVERRIDE;

    const SkBitmap& bitmap() const { return fBitmap; }

    // The non-const 'paint' method allows modification of this object's
    // SkPaint. For this reason the ctor and setPaint method make a local copy.
    // The 'fPaintPtr' member acts a signal that the local SkPaint is valid
    // (since only an SkPaint* is passed into the ctor).
    const SkPaint* paint() const { return fPaintPtr; }
    SkPaint* paint() { return fPaintPtr; }

    void setPaint(const SkPaint& paint) { fPaint = paint; fPaintPtr = &fPaint; }

    const SkRect* srcRect() const { return fSrc.isEmpty() ? NULL : &fSrc; }
    const SkRect& dstRect() const { return fDst; }

    void setSrcRect(const SkRect& src) { fSrc = src; }
    void setDstRect(const SkRect& dst) { fDst = dst; }

private:
    SkBitmap fBitmap;
    SkRect   fSrc;
    SkRect   fDst;
    SkPaint  fPaint;
    SkPaint* fPaintPtr;

    typedef SkDrawCommand INHERITED;
};

class DrawData : public SkDrawCommand {
public:
    DrawData(const void* data, size_t length);
    virtual ~DrawData() { delete [] fData; }
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    char*  fData;
    size_t fLength;

    typedef SkDrawCommand INHERITED;
};

class BeginCommentGroup : public SkDrawCommand {
public:
    BeginCommentGroup(const char* description);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE {
        canvas->beginCommentGroup(fDescription.c_str());
    };
private:
    SkString fDescription;

    typedef SkDrawCommand INHERITED;
};

class Comment : public SkDrawCommand {
public:
    Comment(const char* kywd, const char* value);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE {
        canvas->addComment(fKywd.c_str(), fValue.c_str());
    };
private:
    SkString fKywd;
    SkString fValue;

    typedef SkDrawCommand INHERITED;
};

class EndCommentGroup : public SkDrawCommand {
public:
    EndCommentGroup();
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE {
        canvas->endCommentGroup();
    };
private:
    typedef SkDrawCommand INHERITED;
};

class DrawOval : public SkDrawCommand {
public:
    DrawOval(const SkRect& oval, const SkPaint& paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
    virtual bool render(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkRect  fOval;
    SkPaint fPaint;

    typedef SkDrawCommand INHERITED;
};

class DrawPaint : public SkDrawCommand {
public:
    DrawPaint(const SkPaint& paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
    virtual bool render(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkPaint fPaint;

    typedef SkDrawCommand INHERITED;
};

class DrawPath : public SkDrawCommand {
public:
    DrawPath(const SkPath& path, const SkPaint& paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
    virtual bool render(SkCanvas* canvas) const SK_OVERRIDE;

private:
    SkPath   fPath;
    SkPaint  fPaint;

    typedef SkDrawCommand INHERITED;
};

class DrawPicture : public SkDrawCommand {
public:
    DrawPicture(SkPicture& picture);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    SkPicture fPicture;

    typedef SkDrawCommand INHERITED;
};

class DrawPoints : public SkDrawCommand {
public:
    DrawPoints(SkCanvas::PointMode mode, size_t count, const SkPoint pts[],
               const SkPaint& paint);
    virtual ~DrawPoints() { delete [] fPts; }
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
    virtual bool render(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkCanvas::PointMode fMode;
    size_t              fCount;
    SkPoint*            fPts;
    SkPaint             fPaint;

    typedef SkDrawCommand INHERITED;
};

/* TODO(chudy): DrawText is a predefined macro and was breaking something
 * in the windows build of the debugger.
 */
class DrawTextC : public SkDrawCommand {
public:
    DrawTextC(const void* text, size_t byteLength, SkScalar x, SkScalar y,
              const SkPaint& paint);
    virtual ~DrawTextC() { delete [] fText; }
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    char*    fText;
    size_t   fByteLength;
    SkScalar fX;
    SkScalar fY;
    SkPaint  fPaint;

    typedef SkDrawCommand INHERITED;
};

class DrawPosText : public SkDrawCommand {
public:
    DrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                const SkPaint& paint);
    virtual ~DrawPosText() { delete [] fPos; delete [] fText; }
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    char*    fText;
    size_t   fByteLength;
    SkPoint* fPos;
    SkPaint  fPaint;

    typedef SkDrawCommand INHERITED;
};

class DrawTextOnPath : public SkDrawCommand {
public:
    DrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                   const SkMatrix* matrix, const SkPaint& paint);
    virtual ~DrawTextOnPath() { delete [] fText; }
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    char*    fText;
    size_t   fByteLength;
    SkPath   fPath;
    SkMatrix fMatrix;
    SkPaint  fPaint;

    typedef SkDrawCommand INHERITED;
};

class DrawPosTextH : public SkDrawCommand {
public:
    DrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                 SkScalar constY, const SkPaint& paint);
    virtual ~DrawPosTextH() { delete [] fXpos; delete [] fText; }
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    SkScalar* fXpos;
    char*     fText;
    size_t    fByteLength;
    SkScalar  fConstY;
    SkPaint   fPaint;

    typedef SkDrawCommand INHERITED;
};

class DrawRectC : public SkDrawCommand {
public:
    DrawRectC(const SkRect& rect, const SkPaint& paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;

    const SkRect& rect() const   { return fRect; }
    const SkPaint& paint() const { return fPaint; }
private:
    SkRect  fRect;
    SkPaint fPaint;

    typedef SkDrawCommand INHERITED;
};

class DrawRRect : public SkDrawCommand {
public:
    DrawRRect(const SkRRect& rrect, const SkPaint& paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
    virtual bool render(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkRRect fRRect;
    SkPaint fPaint;

    typedef SkDrawCommand INHERITED;
};

class DrawSprite : public SkDrawCommand {
public:
    DrawSprite(const SkBitmap& bitmap, int left, int top, const SkPaint* paint);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
    virtual bool render(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkBitmap fBitmap;
    int      fLeft;
    int      fTop;
    SkPaint  fPaint;
    SkPaint* fPaintPtr;

    typedef SkDrawCommand INHERITED;
};

class DrawVertices : public SkDrawCommand {
public:
    DrawVertices(SkCanvas::VertexMode vmode, int vertexCount,
                const SkPoint vertices[], const SkPoint texs[],
                const SkColor colors[], SkXfermode* xfermode,
                const uint16_t indices[], int indexCount,
                const SkPaint& paint);
    virtual ~DrawVertices();
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    SkCanvas::VertexMode fVmode;
    int         fVertexCount;
    SkPoint*    fVertices;
    SkPoint*    fTexs;
    SkColor*    fColors;
    SkXfermode* fXfermode;
    uint16_t*   fIndices;
    int         fIndexCount;
    SkPaint     fPaint;

    typedef SkDrawCommand INHERITED;
};

class Rotate : public SkDrawCommand {
public:
    Rotate(SkScalar degrees);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    SkScalar fDegrees;

    typedef SkDrawCommand INHERITED;
};

class Save : public SkDrawCommand {
public:
    Save(SkCanvas::SaveFlags flags);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
    virtual void trackSaveState(int* state) SK_OVERRIDE;
private:
    SkCanvas::SaveFlags fFlags;

    typedef SkDrawCommand INHERITED;
};

class SaveLayer : public SkDrawCommand {
public:
    SaveLayer(const SkRect* bounds, const SkPaint* paint,
              SkCanvas::SaveFlags flags);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
    virtual void trackSaveState(int* state) SK_OVERRIDE;

    const SkPaint* paint() const { return fPaintPtr; }

private:
    SkRect              fBounds;
    SkPaint             fPaint;
    SkPaint*            fPaintPtr;
    SkCanvas::SaveFlags fFlags;

    typedef SkDrawCommand INHERITED;
};

class Scale : public SkDrawCommand {
public:
    Scale(SkScalar sx, SkScalar sy);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;

    SkScalar x() const { return fSx; }
    SkScalar y() const { return fSy; }

private:
    SkScalar fSx;
    SkScalar fSy;

    typedef SkDrawCommand INHERITED;
};

class SetMatrix : public SkDrawCommand {
public:
    SetMatrix(const SkMatrix& matrix);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    SkMatrix fMatrix;

    typedef SkDrawCommand INHERITED;
};

class Skew : public SkDrawCommand {
public:
    Skew(SkScalar sx, SkScalar sy);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;
private:
    SkScalar fSx;
    SkScalar fSy;

    typedef SkDrawCommand INHERITED;
};

class Translate : public SkDrawCommand {
public:
    Translate(SkScalar dx, SkScalar dy);
    virtual void execute(SkCanvas* canvas) SK_OVERRIDE;

    SkScalar x() const { return fDx; }
    SkScalar y() const { return fDy; }

private:
    SkScalar fDx;
    SkScalar fDy;

    typedef SkDrawCommand INHERITED;
};

#endif
