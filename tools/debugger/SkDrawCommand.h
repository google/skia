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
#include "SkDrawShadowInfo.h"
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
        kClear_OpType,
        kClipPath_OpType,
        kClipRegion_OpType,
        kClipRect_OpType,
        kClipRRect_OpType,
        kConcat_OpType,
        kDrawAnnotation_OpType,
        kDrawBitmap_OpType,
        kDrawBitmapLattice_OpType,
        kDrawBitmapNine_OpType,
        kDrawBitmapRect_OpType,
        kDrawDRRect_OpType,
        kDrawImage_OpType,
        kDrawImageLattice_OpType,
        kDrawImageNine_OpType,
        kDrawImageRect_OpType,
        kDrawOval_OpType,
        kDrawArc_OpType,
        kDrawPaint_OpType,
        kDrawPatch_OpType,
        kDrawPath_OpType,
        kDrawPoints_OpType,
        kDrawPosText_OpType,
        kDrawPosTextH_OpType,
        kDrawRect_OpType,
        kDrawRRect_OpType,
        kDrawRegion_OpType,
        kDrawShadow_OpType,
        kDrawText_OpType,
        kDrawTextBlob_OpType,
        kDrawTextOnPath_OpType,
        kDrawTextRSXform_OpType,
        kDrawVertices_OpType,
        kDrawAtlas_OpType,
        kDrawDrawable_OpType,
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

    virtual ~SkDrawCommand() {}

    bool isVisible() const {
        return fVisible;
    }

    void setVisible(bool toggle) {
        fVisible = toggle;
    }

    virtual void execute(SkCanvas*) const = 0;

    virtual bool render(SkCanvas* canvas) const { return false; }

    virtual Json::Value toJSON(UrlDataManager& urlDataManager) const;

    static const char* GetCommandString(OpType type);

    // Helper methods for converting things to JSON
    static Json::Value MakeJsonColor(const SkColor color);
    static Json::Value MakeJsonColor4f(const SkColor4f& color);
    static Json::Value MakeJsonPoint(const SkPoint& point);
    static Json::Value MakeJsonPoint(SkScalar x, SkScalar y);
    static Json::Value MakeJsonPoint3(const SkPoint3& point);
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

private:
    OpType fOpType;
    bool   fVisible;
};

class SkRestoreCommand : public SkDrawCommand {
public:
    SkRestoreCommand();
    void execute(SkCanvas* canvas) const override;

private:
    typedef SkDrawCommand INHERITED;
};

class SkClearCommand : public SkDrawCommand {
public:
    SkClearCommand(SkColor color);
    void execute(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;

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

private:
    SkMatrix fMatrix;

    typedef SkDrawCommand INHERITED;
};

class SkDrawAnnotationCommand : public SkDrawCommand {
public:
    SkDrawAnnotationCommand(const SkRect&, const char key[], sk_sp<SkData> value);
    void execute(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;

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

private:
    SkBitmap         fBitmap;
    SkScalar         fLeft;
    SkScalar         fTop;
    SkTLazy<SkPaint> fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawBitmapLatticeCommand : public SkDrawCommand {
public:
    SkDrawBitmapLatticeCommand(const SkBitmap& bitmap, const SkCanvas::Lattice& lattice,
                               const SkRect& dst, const SkPaint* paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;

private:
    SkBitmap          fBitmap;
    SkCanvas::Lattice fLattice;
    SkRect            fDst;
    SkTLazy<SkPaint>  fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawBitmapNineCommand : public SkDrawCommand {
public:
    SkDrawBitmapNineCommand(const SkBitmap& bitmap, const SkIRect& center,
                            const SkRect& dst, const SkPaint* paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;

private:
    SkBitmap         fBitmap;
    SkIRect          fCenter;
    SkRect           fDst;
    SkTLazy<SkPaint> fPaint;

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

private:
    SkBitmap                      fBitmap;
    SkTLazy<SkRect>               fSrc;
    SkRect                        fDst;
    SkTLazy<SkPaint>              fPaint;
    SkCanvas::SrcRectConstraint   fConstraint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawImageCommand : public SkDrawCommand {
public:
    SkDrawImageCommand(const SkImage* image, SkScalar left, SkScalar top, const SkPaint* paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;

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

class SkDrawImageNineCommand : public SkDrawCommand {
public:
    SkDrawImageNineCommand(const SkImage* image, const SkIRect& center,
                           const SkRect& dst, const SkPaint* paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;

private:
    sk_sp<const SkImage> fImage;
    SkIRect              fCenter;
    SkRect               fDst;
    SkTLazy<SkPaint>     fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawImageRectCommand : public SkDrawCommand {
public:
    SkDrawImageRectCommand(const SkImage* image, const SkRect* src, const SkRect& dst,
                           const SkPaint* paint, SkCanvas::SrcRectConstraint constraint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;

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
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;

private:
    SkCanvas::PointMode fMode;
    SkTDArray<SkPoint>  fPts;
    SkPaint             fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawRegionCommand : public SkDrawCommand {
public:
    SkDrawRegionCommand(const SkRegion& region, const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;

private:
    SkRegion fRegion;
    SkPaint  fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawTextCommand : public SkDrawCommand {
public:
    SkDrawTextCommand(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                      const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;

private:
    sk_sp<SkData> fText;
    SkScalar      fX;
    SkScalar      fY;
    SkPaint       fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawPosTextCommand : public SkDrawCommand {
public:
    SkDrawPosTextCommand(const void* text, size_t byteLength, const SkPoint pos[],
                         const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;

private:
    sk_sp<SkData>      fText;
    SkTDArray<SkPoint> fPos;
    SkPaint            fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawTextOnPathCommand : public SkDrawCommand {
public:
    SkDrawTextOnPathCommand(const void* text, size_t byteLength, const SkPath& path,
                            const SkMatrix* matrix, const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;

private:
    sk_sp<SkData>     fText;
    SkPath            fPath;
    SkTLazy<SkMatrix> fMatrix;
    SkPaint           fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawTextRSXformCommand : public SkDrawCommand {
public:
    SkDrawTextRSXformCommand(const void* text, size_t byteLength, const SkRSXform[],
                             const SkRect*, const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;

private:
    sk_sp<SkData>        fText;
    SkTDArray<SkRSXform> fXform;
    SkTLazy<SkRect>      fCull;
    SkPaint              fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawPosTextHCommand : public SkDrawCommand {
public:
    SkDrawPosTextHCommand(const void* text, size_t byteLength, const SkScalar xpos[],
                          SkScalar constY, const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;

private:
    sk_sp<SkData>       fText;
    SkTDArray<SkScalar> fXpos;
    SkScalar            fConstY;
    SkPaint             fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawTextBlobCommand : public SkDrawCommand {
public:
    SkDrawTextBlobCommand(sk_sp<SkTextBlob> blob, SkScalar x, SkScalar y, const SkPaint& paint);

    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;

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

class SkDrawAtlasCommand : public SkDrawCommand {
public:
    SkDrawAtlasCommand(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[], int,
                       SkBlendMode, const SkRect*, const SkPaint*);

    void execute(SkCanvas* canvas) const override;

private:
    sk_sp<const SkImage> fImage;
    SkTDArray<SkRSXform> fXform;
    SkTDArray<SkRect>    fTex;
    SkTDArray<SkColor>   fColors;
    SkBlendMode          fBlendMode;
    SkTLazy<SkRect>      fCull;
    SkTLazy<SkPaint>     fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkSaveCommand : public SkDrawCommand {
public:
    SkSaveCommand();
    void execute(SkCanvas* canvas) const override;

private:
    typedef SkDrawCommand INHERITED;
};

class SkSaveLayerCommand : public SkDrawCommand {
public:
    SkSaveLayerCommand(const SkCanvas::SaveLayerRec&);
    void execute(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;

private:
    SkTLazy<SkRect>            fBounds;
    SkTLazy<SkPaint>           fPaint;
    sk_sp<const SkImageFilter> fBackdrop;
    uint32_t                   fSaveLayerFlags;

    typedef SkDrawCommand INHERITED;
};

class SkSetMatrixCommand : public SkDrawCommand {
public:
    SkSetMatrixCommand(const SkMatrix& matrix);
    void execute(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;

private:
    SkMatrix fMatrix;

    typedef SkDrawCommand INHERITED;
};

class SkDrawShadowCommand : public SkDrawCommand {
public:
    SkDrawShadowCommand(const SkPath& path, const SkDrawShadowRec& rec);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    Json::Value toJSON(UrlDataManager& urlDataManager) const override;

private:
    SkPath           fPath;
    SkDrawShadowRec  fShadowRec;

    typedef SkDrawCommand INHERITED;
};

class SkDrawDrawableCommand : public SkDrawCommand {
public:
    SkDrawDrawableCommand(SkDrawable*, const SkMatrix*);
    void execute(SkCanvas* canvas) const override;

private:
    sk_sp<SkDrawable> fDrawable;
    SkTLazy<SkMatrix> fMatrix;

    typedef SkDrawCommand INHERITED;
};
#endif
