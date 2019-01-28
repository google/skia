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
#include "SkJSONWriter.h"
#include "SkTLazy.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkRRect.h"
#include "SkRSXform.h"
#include "SkString.h"
#include "SkTDArray.h"
#include "SkVertices.h"
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
        kDrawImageSet_OpType,
        kDrawOval_OpType,
        kDrawArc_OpType,
        kDrawPaint_OpType,
        kDrawPatch_OpType,
        kDrawPath_OpType,
        kDrawPoints_OpType,
        kDrawRect_OpType,
        kDrawEdgeAARect_OpType,
        kDrawRRect_OpType,
        kDrawRegion_OpType,
        kDrawShadow_OpType,
        kDrawTextBlob_OpType,
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

    static void WritePNG(SkBitmap bitmap, SkWStream& out);

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

    virtual void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const;

    static const char* GetCommandString(OpType type);

    // Helper methods for converting things to JSON
    static void MakeJsonColor(SkJSONWriter&, const SkColor color);
    static void MakeJsonColor4f(SkJSONWriter&, const SkColor4f& color);
    static void MakeJsonPoint(SkJSONWriter&, const SkPoint& point);
    static void MakeJsonPoint(SkJSONWriter&, SkScalar x, SkScalar y);
    static void MakeJsonPoint3(SkJSONWriter&, const SkPoint3& point);
    static void MakeJsonRect(SkJSONWriter&, const SkRect& rect);
    static void MakeJsonIRect(SkJSONWriter&, const SkIRect&);
    static void MakeJsonMatrix(SkJSONWriter&, const SkMatrix&);
    static void MakeJsonPath(SkJSONWriter&, const SkPath& path);
    static void MakeJsonRegion(SkJSONWriter&, const SkRegion& region);
    static void MakeJsonPaint(SkJSONWriter&, const SkPaint& paint, UrlDataManager& urlDataManager);
    static void MakeJsonLattice(SkJSONWriter&, const SkCanvas::Lattice& lattice);

    static void flatten(const SkFlattenable* flattenable, SkJSONWriter& writer,
                        UrlDataManager& urlDataManager);
    static bool flatten(const SkImage& image, SkJSONWriter& writer,
                        UrlDataManager& urlDataManager);
    static bool flatten(const SkBitmap& bitmap, SkJSONWriter& writer,
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
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkColor fColor;

    typedef SkDrawCommand INHERITED;
};

class SkClipPathCommand : public SkDrawCommand {
public:
    SkClipPathCommand(const SkPath& path, SkClipOp op, bool doAA);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

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
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkRegion fRegion;
    SkClipOp fOp;

    typedef SkDrawCommand INHERITED;
};

class SkClipRectCommand : public SkDrawCommand {
public:
    SkClipRectCommand(const SkRect& rect, SkClipOp op, bool doAA);
    void execute(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

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
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

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
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkMatrix fMatrix;

    typedef SkDrawCommand INHERITED;
};

class SkDrawAnnotationCommand : public SkDrawCommand {
public:
    SkDrawAnnotationCommand(const SkRect&, const char key[], sk_sp<SkData> value);
    void execute(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

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
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

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
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

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
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

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
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

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
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

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
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

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
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

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
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    sk_sp<const SkImage>        fImage;
    SkTLazy<SkRect>             fSrc;
    SkRect                      fDst;
    SkTLazy<SkPaint>            fPaint;
    SkCanvas::SrcRectConstraint fConstraint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawImageSetCommand : public SkDrawCommand {
public:
    SkDrawImageSetCommand(const SkCanvas::ImageSetEntry[], int count, SkFilterQuality, SkBlendMode);
    void execute(SkCanvas* canvas) const override;

private:
    SkAutoTArray<SkCanvas::ImageSetEntry> fSet;
    int fCount;
    SkFilterQuality fFilterQuality;
    SkBlendMode fMode;

    typedef SkDrawCommand INHERITED;
};

class SkDrawOvalCommand : public SkDrawCommand {
public:
    SkDrawOvalCommand(const SkRect& oval, const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

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
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

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
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkPaint fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawPathCommand : public SkDrawCommand {
public:
    SkDrawPathCommand(const SkPath& path, const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

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
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

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
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkRegion fRegion;
    SkPaint  fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawTextBlobCommand : public SkDrawCommand {
public:
    SkDrawTextBlobCommand(sk_sp<SkTextBlob> blob, SkScalar x, SkScalar y, const SkPaint& paint);

    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

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
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

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
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkRect  fRect;
    SkPaint fPaint;

    typedef SkDrawCommand INHERITED;
};

class SkDrawEdgeAARectCommand : public SkDrawCommand {
public:
    SkDrawEdgeAARectCommand(const SkRect& rect, SkCanvas::QuadAAFlags aa, SkColor color,
                            SkBlendMode mode);
    void execute(SkCanvas* canvas) const override;

private:
    SkRect  fRect;
    SkCanvas::QuadAAFlags fAA;
    SkColor fColor;
    SkBlendMode fMode;

    typedef SkDrawCommand INHERITED;
};

class SkDrawRRectCommand : public SkDrawCommand {
public:
    SkDrawRRectCommand(const SkRRect& rrect, const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

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
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

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
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

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
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkMatrix fMatrix;

    typedef SkDrawCommand INHERITED;
};

class SkDrawShadowCommand : public SkDrawCommand {
public:
    SkDrawShadowCommand(const SkPath& path, const SkDrawShadowRec& rec);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

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
