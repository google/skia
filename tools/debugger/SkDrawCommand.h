/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKDRAWCOMMAND_H_
#define SKDRAWCOMMAND_H_

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkFlattenable.h"
#include "SkTLazy.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkRRect.h"
#include "SkRSXform.h"
#include "SkString.h"
#include "SkTDArray.h"
#include "SkVertices.h"
#include "SkJSONCPP.h"
#include "UrlDataManager.h"

class SkDrawCommand {
public:
    enum OpType {
        kBeginDrawPicture_OpType,
        kClipPath_OpType,
        kClipRegion_OpType,
        kClipRect_OpType,
        kClipRRect_OpType,
        kConcat_OpType,
        kDrawAnnotation_OpType,
        kDrawBitmap_OpType,
        kDrawBitmapNine_OpType,
        kDrawBitmapRect_OpType,
        kDrawClear_OpType,
        kDrawDRRect_OpType,
        kDrawImage_OpType,
        kDrawImageLattice_OpType,
        kDrawImageRect_OpType,
        kDrawOval_OpType,
        kDrawPaint_OpType,
        kDrawPatch_OpType,
        kDrawPath_OpType,
        kDrawPoints_OpType,
        kDrawPosText_OpType,
        kDrawPosTextH_OpType,
        kDrawRect_OpType,
        kDrawRRect_OpType,
        kDrawText_OpType,
        kDrawTextBlob_OpType,
        kDrawTextOnPath_OpType,
        kDrawTextRSXform_OpType,
        kDrawVertices_OpType,
        kEndDrawPicture_OpType,
        kRestore_OpType,
        kSave_OpType,
        kSaveLayer_OpType,
        kSetMatrix_OpType,

        kLast_OpType = kSetMatrix_OpType
    };

    static const int kOpTypeCount = kLast_OpType + 1;

    static void WritePNG(const uint8_t* rgba, unsigned width, unsigned height,
                         SkWStream& out, bool isOpaque);

    SkDrawCommand(OpType opType);

    virtual ~SkDrawCommand();

    virtual SkString toString() const;

    virtual const char* toCString() const {
        return GetCommandString(fOpType);
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

    OpType getType() const { return fOpType; }

    virtual bool render(SkCanvas* canvas) const { return false; }

    virtual Json::Value toJSON(UrlDataManager& urlDataManager) const;

    /* Converts a JSON representation of a command into a newly-allocated SkDrawCommand object. It
     * is the caller's responsibility to delete this object. This method may return null if an error
     * occurs.
     */
    static SkDrawCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

    static const char* GetCommandString(OpType type);

    // Helper methods for converting things to JSON
    static Json::Value MakeJsonColor(const SkColor color);
    static Json::Value MakeJsonColor4f(const SkColor4f& color);
    static Json::Value MakeJsonPoint(const SkPoint& point);
    static Json::Value MakeJsonPoint(SkScalar x, SkScalar y);
    static Json::Value MakeJsonRect(const SkRect& rect);
    static Json::Value MakeJsonIRect(const SkIRect&);
    static Json::Value MakeJsonMatrix(const SkMatrix&);
    static Json::Value MakeJsonScalar(SkScalar);
    static Json::Value MakeJsonPath(const SkPath& path);
    static Json::Value MakeJsonRegion(const SkRegion& region);
    static Json::Value MakeJsonPaint(const SkPaint& paint, UrlDataManager& urlDataManager);
    static Json::Value MakeJsonLattice(const SkCanvas::Lattice& lattice);

    static void flatten(const SkFlattenable* flattenable, Json::Value* target,
                        UrlDataManager& urlDataManager);
    static bool flatten(const SkImage& image, Json::Value* target,
                        UrlDataManager& urlDataManager);
    static bool flatten(const SkBitmap& bitmap, Json::Value* target,
                        UrlDataManager& urlDataManager);

protected:
    SkTDArray<SkString*> fInfo;

private:
    OpType fOpType;
    bool   fVisible;
};

class SkRestoreCommand : public SkDrawCommand {
public:
    SkRestoreCommand();
    void execute(SkCanvas* canvas) const override;
    Action action() const override { return kPopLayer_Action; }
    static SkRestoreCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

private:
    typedef SkDrawCommand INHERITED;
};

class SkClearCommand : public SkDrawCommand {
public:
    SkClearCommand(SkColor color);
    void execute(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkClearCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

private:
    SkColor fColor;

    typedef SkDrawCommand INHERITED;
};

class SkClipPathCommand : public SkDrawCommand {
public:
    SkClipPathCommand(const SkPath& path, SkClipOp op, bool doAA);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkClipPathCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

private:
    SkPath   fPath;
    SkClipOp fOp;
    bool     fDoAA;

    typedef SkDrawCommand INHERITED;
};

class SkClipRegionCommand : public SkDrawCommand {
public:
    SkClipRegionCommand(const SkRegion& region, SkClipOp op);
    void execute(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkClipRegionCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

private:
    SkRegion fRegion;
    SkClipOp fOp;

    typedef SkDrawCommand INHERITED;
};

class SkClipRectCommand : public SkDrawCommand {
public:
    SkClipRectCommand(const SkRect& rect, SkClipOp op, bool doAA);
    void execute(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkClipRectCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

    const SkRect& rect() const { return fRect; }
    SkClipOp op() const { return fOp; }
    bool doAA() const { return fDoAA; }

private:
    SkRect   fRect;
    SkClipOp fOp;
    bool     fDoAA;

    typedef SkDrawCommand INHERITED;
};

class SkClipRRectCommand : public SkDrawCommand {
public:
    SkClipRRectCommand(const SkRRect& rrect, SkClipOp op, bool doAA);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkClipRRectCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

    const SkRRect& rrect() const { return fRRect; }
    SkClipOp op() const { return fOp; }
    bool doAA() const { return fDoAA; }

private:
    SkRRect  fRRect;
    SkClipOp fOp;
    bool     fDoAA;

    typedef SkDrawCommand INHERITED;
};

class SkConcatCommand : public SkDrawCommand {
public:
    SkConcatCommand(const SkMatrix& matrix);
    void execute(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkConcatCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

private:
    SkMatrix fMatrix;

    typedef SkDrawCommand INHERITED;
};

class SkDrawAnnotationCommand : public SkDrawCommand {
public:
    SkDrawAnnotationCommand(const SkRect&, const char key[], sk_sp<SkData> value);
    void execute(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkDrawAnnotationCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

private:
    SkRect          fRect;
    SkString        fKey;
    sk_sp<SkData>   fValue;

    typedef SkDrawCommand INHERITED;
};

class SkDrawBitmapCommand : public SkDrawCommand {
public:
    SkDrawBitmapCommand(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                        const SkPaint* paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkDrawBitmapCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

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
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkDrawBitmapNineCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

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
                            SkCanvas::SrcRectConstraint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkDrawBitmapRectCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

    const SkBitmap& bitmap() const { return fBitmap; }

    // The non-const 'paint' method allows modification of this object's
    // SkPaint. For this reason the ctor and setPaint method make a local copy.
    // The 'fPaintPtr' member acts a signal that the local SkPaint is valid
    // (since only an SkPaint* is passed into the ctor).
    const SkPaint* paint() const { return fPaintPtr; }
    SkPaint* paint() { return fPaintPtr; }

    void setPaint(const SkPaint& paint) { fPaint = paint; fPaintPtr = &fPaint; }

    const SkRect* srcRect() const { return fSrc.isEmpty() ? nullptr : &fSrc; }
    void setSrcRect(const SkRect& src) { fSrc = src; }

    const SkRect& dstRect() const { return fDst; }
    void setDstRect(const SkRect& dst) { fDst = dst; }

    SkCanvas::SrcRectConstraint constraint() const { return fConstraint; }
    void setConstraint(SkCanvas::SrcRectConstraint constraint) { fConstraint = constraint; }

private:
    SkBitmap                      fBitmap;
    SkRect                        fSrc;
    SkRect                        fDst;
    SkPaint                       fPaint;
    SkPaint*                      fPaintPtr;
    SkCanvas::SrcRectConstraint   fConstraint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawImageCommand : public SkDrawCommand {
public:
    SkDrawImageCommand(const SkImage* image, SkScalar left, SkScalar top, const SkPaint* paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkDrawImageCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

private:
    sk_sp<const SkImage> fImage;
    SkScalar             fLeft;
    SkScalar             fTop;
    SkTLazy<SkPaint>     fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawImageLatticeCommand : public SkDrawCommand {
public:
    SkDrawImageLatticeCommand(const SkImage* image, const SkCanvas::Lattice& lattice,
                              const SkRect& dst, const SkPaint* paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;

private:
    sk_sp<const SkImage>        fImage;
    SkCanvas::Lattice           fLattice;
    SkRect                      fDst;
    SkTLazy<SkPaint>            fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawImageRectCommand : public SkDrawCommand {
public:
    SkDrawImageRectCommand(const SkImage* image, const SkRect* src, const SkRect& dst,
                           const SkPaint* paint, SkCanvas::SrcRectConstraint constraint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkDrawImageRectCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

private:
    sk_sp<const SkImage>        fImage;
    SkTLazy<SkRect>             fSrc;
    SkRect                      fDst;
    SkTLazy<SkPaint>            fPaint;
    SkCanvas::SrcRectConstraint fConstraint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawOvalCommand : public SkDrawCommand {
public:
    SkDrawOvalCommand(const SkRect& oval, const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkDrawOvalCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

private:
    SkRect  fOval;
    SkPaint fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawArcCommand : public SkDrawCommand {
public:
    SkDrawArcCommand(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle, bool useCenter,
                     const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkDrawArcCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

private:
    SkRect   fOval;
    SkScalar fStartAngle;
    SkScalar fSweepAngle;
    bool     fUseCenter;
    SkPaint  fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawPaintCommand : public SkDrawCommand {
public:
    SkDrawPaintCommand(const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkDrawPaintCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

private:
    SkPaint fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawPathCommand : public SkDrawCommand {
public:
    SkDrawPathCommand(const SkPath& path, const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkDrawPathCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

private:
    SkPath   fPath;
    SkPaint  fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkBeginDrawPictureCommand : public SkDrawCommand {
public:
    SkBeginDrawPictureCommand(const SkPicture* picture,
                              const SkMatrix* matrix,
                              const SkPaint* paint);

    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;

private:
    sk_sp<const SkPicture> fPicture;
    SkTLazy<SkMatrix>      fMatrix;
    SkTLazy<SkPaint>       fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkEndDrawPictureCommand : public SkDrawCommand {
public:
    SkEndDrawPictureCommand(bool restore);

    void execute(SkCanvas* canvas) const override;

private:
    bool fRestore;

    typedef SkDrawCommand INHERITED;
};

class SkDrawPointsCommand : public SkDrawCommand {
public:
    SkDrawPointsCommand(SkCanvas::PointMode mode, size_t count, const SkPoint pts[],
                        const SkPaint& paint);
    ~SkDrawPointsCommand() override { delete [] fPts; }
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkDrawPointsCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

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
    ~SkDrawTextCommand() override { delete [] fText; }
    void execute(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkDrawTextCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

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
    ~SkDrawPosTextCommand() override { delete [] fPos; delete [] fText; }
    void execute(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkDrawPosTextCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

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
    ~SkDrawTextOnPathCommand() override { delete [] fText; }
    void execute(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkDrawTextOnPathCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

private:
    char*    fText;
    size_t   fByteLength;
    SkPath   fPath;
    SkMatrix fMatrix;
    SkPaint  fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawTextRSXformCommand : public SkDrawCommand {
public:
    SkDrawTextRSXformCommand(const void* text, size_t byteLength, const SkRSXform[],
                             const SkRect*, const SkPaint& paint);
    ~SkDrawTextRSXformCommand() override { delete[] fText; delete[] fXform; }
    void execute(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkDrawTextRSXformCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

private:
    char*       fText;
    size_t      fByteLength;
    SkRSXform*  fXform;
    SkRect*     fCull;
    SkRect      fCullStorage;
    SkPaint     fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawPosTextHCommand : public SkDrawCommand {
public:
    SkDrawPosTextHCommand(const void* text, size_t byteLength, const SkScalar xpos[],
                          SkScalar constY, const SkPaint& paint);
    ~SkDrawPosTextHCommand() override { delete [] fXpos; delete [] fText; }
    void execute(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkDrawPosTextHCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

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
    SkDrawTextBlobCommand(sk_sp<SkTextBlob> blob, SkScalar x, SkScalar y, const SkPaint& paint);

    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkDrawTextBlobCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

private:
    sk_sp<SkTextBlob> fBlob;
    SkScalar          fXPos;
    SkScalar          fYPos;
    SkPaint           fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawPatchCommand : public SkDrawCommand {
public:
    SkDrawPatchCommand(const SkPoint cubics[12], const SkColor colors[4],
                       const SkPoint texCoords[4], SkBlendMode bmode,
                       const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkDrawPatchCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

private:
    SkPoint fCubics[12];
    SkColor* fColorsPtr;
    SkColor  fColors[4];
    SkPoint* fTexCoordsPtr;
    SkPoint  fTexCoords[4];
    SkBlendMode fBlendMode;
    SkPaint fPaint;

    typedef SkDrawCommand INHERITED;
};


class SkDrawRectCommand : public SkDrawCommand {
public:
    SkDrawRectCommand(const SkRect& rect, const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkDrawRectCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

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
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkDrawRRectCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

private:
    SkRRect fRRect;
    SkPaint fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawDRRectCommand : public SkDrawCommand {
public:
    SkDrawDRRectCommand(const SkRRect& outer, const SkRRect& inner,
                        const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkDrawDRRectCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

private:
    SkRRect fOuter;
    SkRRect fInner;
    SkPaint fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawVerticesCommand : public SkDrawCommand {
public:
    SkDrawVerticesCommand(sk_sp<SkVertices>, SkBlendMode, const SkPaint&);

    void execute(SkCanvas* canvas) const override;

private:
    sk_sp<SkVertices>   fVertices;
    SkBlendMode         fBlendMode;
    SkPaint             fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkSaveCommand : public SkDrawCommand {
public:
    SkSaveCommand();
    void execute(SkCanvas* canvas) const override;
    Action action() const override { return kPushLayer_Action; }
    static SkSaveCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

private:
    typedef SkDrawCommand INHERITED;
};

class SkSaveLayerCommand : public SkDrawCommand {
public:
    SkSaveLayerCommand(const SkCanvas::SaveLayerRec&);
    ~SkSaveLayerCommand() override;
    void execute(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkSaveLayerCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);
    void vizExecute(SkCanvas* canvas) const override;
    Action action() const override{ return kPushLayer_Action; }
    void setActive(bool active) override { fActive = active; }
    bool active() const override { return fActive; }

    const SkPaint* paint() const { return fPaintPtr; }

private:
    SkRect               fBounds;
    SkPaint              fPaint;
    SkPaint*             fPaintPtr;
    const SkImageFilter* fBackdrop;
    uint32_t       fSaveLayerFlags;

    bool        fActive;

    typedef SkDrawCommand INHERITED;
};

class SkSetMatrixCommand : public SkDrawCommand {
public:
    SkSetMatrixCommand(const SkMatrix& matrix);
    void setUserMatrix(const SkMatrix&) override;
    void execute(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;
    static SkSetMatrixCommand* fromJSON(Json::Value& command, UrlDataManager& urlDataManager);

private:
    SkMatrix fUserMatrix;
    SkMatrix fMatrix;

    typedef SkDrawCommand INHERITED;
};
#endif

