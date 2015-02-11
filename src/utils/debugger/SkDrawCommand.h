
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
    SkDrawCommand(DrawType drawType);

    virtual ~SkDrawCommand();

    virtual SkString toString() const;

    void setOffset(size_t offset) { fOffset = offset; }
    size_t offset() const { return fOffset; }

    virtual const char* toCString() const {
        return GetCommandString(fDrawType);
    }

    bool isVisible() const {
        return fVisible;
    }

    void setVisible(bool toggle) {
        fVisible = toggle;
    }

    const SkTDArray<SkString*>* Info() const { return &fInfo; }
    virtual void execute(SkCanvas*) const = 0;
    virtual void vizExecute(SkCanvas*) const {}

    virtual void setUserMatrix(const SkMatrix&) {}

    // The next "active" system is only used by save, saveLayer, and restore.
    // It is used to determine which saveLayers are currently active (at a
    // given point in the rendering).
    //      saves just return a kPushLayer action but don't track active state
    //      restores just return a kPopLayer action
    //      saveLayers return kPushLayer but also track the active state
    enum Action {
        kNone_Action,
        kPopLayer_Action,
        kPushLayer_Action,
    };
    virtual Action action() const { return kNone_Action; }
    virtual void setActive(bool active) {}
    virtual bool active() const { return false; }

    DrawType getType() const { return fDrawType; }

    virtual bool render(SkCanvas* canvas) const { return false; }

    static const char* GetCommandString(DrawType type);

protected:
    SkTDArray<SkString*> fInfo;

private:
    DrawType fDrawType;
    size_t fOffset;
    bool   fVisible;
};

// TODO: switch Chromium over to not using these
typedef SkDrawCommand::OpType DrawType;

#define SAVE SkDrawCommand::kSave_OpType
#define SCALE SkDrawCommand::kSetMatrix_OpType
#define TRANSLATE SkDrawCommand::kSetMatrix_OpType
#define DRAW_RECT SkDrawCommand::kDrawRect_OpType
#define RESTORE SkDrawCommand::kRestore_OpType
#define CLIP_RECT SkDrawCommand::kClipRect_OpType

class SkRestoreCommand : public SkDrawCommand {
public:
    SkRestoreCommand();
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
    Action action() const SK_OVERRIDE { return kPopLayer_Action; }

private:
    typedef SkDrawCommand INHERITED;
};

class SkClearCommand : public SkDrawCommand {
public:
    SkClearCommand(SkColor color);
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkColor fColor;

    typedef SkDrawCommand INHERITED;
};

class SkClipPathCommand : public SkDrawCommand {
public:
    SkClipPathCommand(const SkPath& path, SkRegion::Op op, bool doAA);
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
    bool render(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkPath       fPath;
    SkRegion::Op fOp;
    bool         fDoAA;

    typedef SkDrawCommand INHERITED;
};

class SkClipRegionCommand : public SkDrawCommand {
public:
    SkClipRegionCommand(const SkRegion& region, SkRegion::Op op);
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkRegion     fRegion;
    SkRegion::Op fOp;

    typedef SkDrawCommand INHERITED;
};

class SkClipRectCommand : public SkDrawCommand {
public:
    SkClipRectCommand(const SkRect& rect, SkRegion::Op op, bool doAA);
    void execute(SkCanvas* canvas) const SK_OVERRIDE;

    const SkRect& rect() const { return fRect; }
    SkRegion::Op op() const { return fOp; }
    bool doAA() const { return fDoAA; }

private:
    SkRect       fRect;
    SkRegion::Op fOp;
    bool         fDoAA;

    typedef SkDrawCommand INHERITED;
};

class SkClipRRectCommand : public SkDrawCommand {
public:
    SkClipRRectCommand(const SkRRect& rrect, SkRegion::Op op, bool doAA);
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
    bool render(SkCanvas* canvas) const SK_OVERRIDE;

    const SkRRect& rrect() const { return fRRect; }
    SkRegion::Op op() const { return fOp; }
    bool doAA() const { return fDoAA; }

private:
    SkRRect      fRRect;
    SkRegion::Op fOp;
    bool         fDoAA;

    typedef SkDrawCommand INHERITED;
};

class SkConcatCommand : public SkDrawCommand {
public:
    SkConcatCommand(const SkMatrix& matrix);
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkMatrix fMatrix;

    typedef SkDrawCommand INHERITED;
};

class SkDrawBitmapCommand : public SkDrawCommand {
public:
    SkDrawBitmapCommand(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                        const SkPaint* paint);
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
    bool render(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkBitmap fBitmap;
    SkScalar fLeft;
    SkScalar fTop;
    SkPaint  fPaint;
    SkPaint* fPaintPtr;

    typedef SkDrawCommand INHERITED;
};

class SkDrawBitmapNineCommand : public SkDrawCommand {
public:
    SkDrawBitmapNineCommand(const SkBitmap& bitmap, const SkIRect& center,
                            const SkRect& dst, const SkPaint* paint);
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
    bool render(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkBitmap fBitmap;
    SkIRect  fCenter;
    SkRect   fDst;
    SkPaint  fPaint;
    SkPaint* fPaintPtr;

    typedef SkDrawCommand INHERITED;
};

class SkDrawBitmapRectCommand : public SkDrawCommand {
public:
    SkDrawBitmapRectCommand(const SkBitmap& bitmap, const SkRect* src,
                            const SkRect& dst, const SkPaint* paint,
                            SkCanvas::DrawBitmapRectFlags flags);
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
    bool render(SkCanvas* canvas) const SK_OVERRIDE;

    const SkBitmap& bitmap() const { return fBitmap; }

    // The non-const 'paint' method allows modification of this object's
    // SkPaint. For this reason the ctor and setPaint method make a local copy.
    // The 'fPaintPtr' member acts a signal that the local SkPaint is valid
    // (since only an SkPaint* is passed into the ctor).
    const SkPaint* paint() const { return fPaintPtr; }
    SkPaint* paint() { return fPaintPtr; }

    void setPaint(const SkPaint& paint) { fPaint = paint; fPaintPtr = &fPaint; }

    const SkRect* srcRect() const { return fSrc.isEmpty() ? NULL : &fSrc; }
    void setSrcRect(const SkRect& src) { fSrc = src; }

    const SkRect& dstRect() const { return fDst; }
    void setDstRect(const SkRect& dst) { fDst = dst; }

    SkCanvas::DrawBitmapRectFlags flags() const { return fFlags; }
    void setFlags(SkCanvas::DrawBitmapRectFlags flags) { fFlags = flags; }

private:
    SkBitmap                      fBitmap;
    SkRect                        fSrc;
    SkRect                        fDst;
    SkPaint                       fPaint;
    SkPaint*                      fPaintPtr;
    SkCanvas::DrawBitmapRectFlags fFlags;

    typedef SkDrawCommand INHERITED;
};

class SkBeginCommentGroupCommand : public SkDrawCommand {
public:
    SkBeginCommentGroupCommand(const char* description);
    void execute(SkCanvas* canvas) const SK_OVERRIDE {
        canvas->beginCommentGroup(fDescription.c_str());
    };
private:
    SkString fDescription;

    typedef SkDrawCommand INHERITED;
};

class SkCommentCommand : public SkDrawCommand {
public:
    SkCommentCommand(const char* kywd, const char* value);
    void execute(SkCanvas* canvas) const SK_OVERRIDE {
        canvas->addComment(fKywd.c_str(), fValue.c_str());
    };
private:
    SkString fKywd;
    SkString fValue;

    typedef SkDrawCommand INHERITED;
};

class SkEndCommentGroupCommand : public SkDrawCommand {
public:
    SkEndCommentGroupCommand();
    void execute(SkCanvas* canvas) const SK_OVERRIDE {
        canvas->endCommentGroup();
    };
private:
    typedef SkDrawCommand INHERITED;
};

class SkDrawOvalCommand : public SkDrawCommand {
public:
    SkDrawOvalCommand(const SkRect& oval, const SkPaint& paint);
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
    bool render(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkRect  fOval;
    SkPaint fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawPaintCommand : public SkDrawCommand {
public:
    SkDrawPaintCommand(const SkPaint& paint);
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
    bool render(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkPaint fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawPathCommand : public SkDrawCommand {
public:
    SkDrawPathCommand(const SkPath& path, const SkPaint& paint);
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
    bool render(SkCanvas* canvas) const SK_OVERRIDE;

private:
    SkPath   fPath;
    SkPaint  fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawPictureCommand : public SkDrawCommand {
public:
    SkDrawPictureCommand(const SkPicture* picture, const SkMatrix* matrix, const SkPaint* paint);
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
    bool render(SkCanvas* canvas) const SK_OVERRIDE;

private:
    SkAutoTUnref<const SkPicture> fPicture;
    SkMatrix                      fMatrix;
    SkMatrix*                     fMatrixPtr;
    SkPaint                       fPaint;
    SkPaint*                      fPaintPtr;

    typedef SkDrawCommand INHERITED;
};

class SkDrawPointsCommand : public SkDrawCommand {
public:
    SkDrawPointsCommand(SkCanvas::PointMode mode, size_t count, const SkPoint pts[],
                        const SkPaint& paint);
    virtual ~SkDrawPointsCommand() { delete [] fPts; }
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
    bool render(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkCanvas::PointMode fMode;
    size_t              fCount;
    SkPoint*            fPts;
    SkPaint             fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawTextCommand : public SkDrawCommand {
public:
    SkDrawTextCommand(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                      const SkPaint& paint);
    virtual ~SkDrawTextCommand() { delete [] fText; }
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
private:
    char*    fText;
    size_t   fByteLength;
    SkScalar fX;
    SkScalar fY;
    SkPaint  fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawPosTextCommand : public SkDrawCommand {
public:
    SkDrawPosTextCommand(const void* text, size_t byteLength, const SkPoint pos[],
                         const SkPaint& paint);
    virtual ~SkDrawPosTextCommand() { delete [] fPos; delete [] fText; }
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
private:
    char*    fText;
    size_t   fByteLength;
    SkPoint* fPos;
    SkPaint  fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawTextOnPathCommand : public SkDrawCommand {
public:
    SkDrawTextOnPathCommand(const void* text, size_t byteLength, const SkPath& path,
                            const SkMatrix* matrix, const SkPaint& paint);
    virtual ~SkDrawTextOnPathCommand() { delete [] fText; }
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
private:
    char*    fText;
    size_t   fByteLength;
    SkPath   fPath;
    SkMatrix fMatrix;
    SkPaint  fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawPosTextHCommand : public SkDrawCommand {
public:
    SkDrawPosTextHCommand(const void* text, size_t byteLength, const SkScalar xpos[],
                          SkScalar constY, const SkPaint& paint);
    virtual ~SkDrawPosTextHCommand() { delete [] fXpos; delete [] fText; }
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkScalar* fXpos;
    char*     fText;
    size_t    fByteLength;
    SkScalar  fConstY;
    SkPaint   fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawTextBlobCommand : public SkDrawCommand {
public:
    SkDrawTextBlobCommand(const SkTextBlob* blob, SkScalar x, SkScalar y, const SkPaint& paint);

    void execute(SkCanvas* canvas) const SK_OVERRIDE;
    bool render(SkCanvas* canvas) const SK_OVERRIDE;

private:
    SkAutoTUnref<const SkTextBlob> fBlob;
    SkScalar                       fXPos;
    SkScalar                       fYPos;
    SkPaint                        fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawRectCommand : public SkDrawCommand {
public:
    SkDrawRectCommand(const SkRect& rect, const SkPaint& paint);
    void execute(SkCanvas* canvas) const SK_OVERRIDE;

    const SkRect& rect() const   { return fRect; }
    const SkPaint& paint() const { return fPaint; }
private:
    SkRect  fRect;
    SkPaint fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawRRectCommand : public SkDrawCommand {
public:
    SkDrawRRectCommand(const SkRRect& rrect, const SkPaint& paint);
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
    bool render(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkRRect fRRect;
    SkPaint fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawDRRectCommand : public SkDrawCommand {
public:
    SkDrawDRRectCommand(const SkRRect& outer, const SkRRect& inner,
                        const SkPaint& paint);
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
    bool render(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkRRect fOuter;
    SkRRect fInner;
    SkPaint fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawSpriteCommand : public SkDrawCommand {
public:
    SkDrawSpriteCommand(const SkBitmap& bitmap, int left, int top, const SkPaint* paint);
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
    bool render(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkBitmap fBitmap;
    int      fLeft;
    int      fTop;
    SkPaint  fPaint;
    SkPaint* fPaintPtr;

    typedef SkDrawCommand INHERITED;
};

class SkDrawVerticesCommand : public SkDrawCommand {
public:
    SkDrawVerticesCommand(SkCanvas::VertexMode vmode, int vertexCount,
                          const SkPoint vertices[], const SkPoint texs[],
                          const SkColor colors[], SkXfermode* xfermode,
                          const uint16_t indices[], int indexCount,
                          const SkPaint& paint);
    virtual ~SkDrawVerticesCommand();
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
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

class SkRotateCommand : public SkDrawCommand {
public:
    SkRotateCommand(SkScalar degrees);
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkScalar fDegrees;

    typedef SkDrawCommand INHERITED;
};

class SkSaveCommand : public SkDrawCommand {
public:
    SkSaveCommand();
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
    Action action() const SK_OVERRIDE { return kPushLayer_Action; }
private:
    typedef SkDrawCommand INHERITED;
};

class SkSaveLayerCommand : public SkDrawCommand {
public:
    SkSaveLayerCommand(const SkRect* bounds, const SkPaint* paint,
                       SkCanvas::SaveFlags flags);
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
    void vizExecute(SkCanvas* canvas) const SK_OVERRIDE;
    Action action() const SK_OVERRIDE{ return kPushLayer_Action; }
    void setActive(bool active) SK_OVERRIDE { fActive = active; }
    bool active() const SK_OVERRIDE { return fActive; }

    const SkPaint* paint() const { return fPaintPtr; }

private:
    SkRect              fBounds;
    SkPaint             fPaint;
    SkPaint*            fPaintPtr;
    SkCanvas::SaveFlags fFlags;

    bool                fActive;

    typedef SkDrawCommand INHERITED;
};

class SkScaleCommand : public SkDrawCommand {
public:
    SkScaleCommand(SkScalar sx, SkScalar sy);
    void execute(SkCanvas* canvas) const SK_OVERRIDE;

    SkScalar x() const { return fSx; }
    SkScalar y() const { return fSy; }

private:
    SkScalar fSx;
    SkScalar fSy;

    typedef SkDrawCommand INHERITED;
};

class SkSetMatrixCommand : public SkDrawCommand {
public:
    SkSetMatrixCommand(const SkMatrix& matrix);
    void setUserMatrix(const SkMatrix&) SK_OVERRIDE;
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkMatrix fUserMatrix;
    SkMatrix fMatrix;

    typedef SkDrawCommand INHERITED;
};

class SkSkewCommand : public SkDrawCommand {
public:
    SkSkewCommand(SkScalar sx, SkScalar sy);
    void execute(SkCanvas* canvas) const SK_OVERRIDE;
private:
    SkScalar fSx;
    SkScalar fSy;

    typedef SkDrawCommand INHERITED;
};

class SkTranslateCommand : public SkDrawCommand {
public:
    SkTranslateCommand(SkScalar dx, SkScalar dy);
    void execute(SkCanvas* canvas) const SK_OVERRIDE;

    SkScalar x() const { return fDx; }
    SkScalar y() const { return fDy; }

private:
    SkScalar fDx;
    SkScalar fDy;

    typedef SkDrawCommand INHERITED;
};

#endif
