/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKDRAWCOMMAND_H_
#define SKDRAWCOMMAND_H_

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkRegion.h"
#include "include/core/SkString.h"
#include "include/core/SkVertices.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkDrawShadowInfo.h"
#include "src/core/SkTLazy.h"
#include "src/utils/SkJSONWriter.h"
#include "tools/UrlDataManager.h"

class DrawCommand {
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
        kDrawRect_OpType,
        kDrawRRect_OpType,
        kDrawRegion_OpType,
        kDrawShadow_OpType,
        kDrawTextBlob_OpType,
        kDrawVertices_OpType,
        kDrawAtlas_OpType,
        kDrawDrawable_OpType,
        kDrawEdgeAAQuad_OpType,
        kDrawEdgeAAImageSet_OpType,
        kEndDrawPicture_OpType,
        kRestore_OpType,
        kSave_OpType,
        kSaveLayer_OpType,
        kSetMatrix_OpType,

        kLast_OpType = kSetMatrix_OpType
    };

    static const int kOpTypeCount = kLast_OpType + 1;

    static void WritePNG(SkBitmap bitmap, SkWStream& out);

    DrawCommand(OpType opType);

    virtual ~DrawCommand() {}

    bool isVisible() const { return fVisible; }

    void setVisible(bool toggle) { fVisible = toggle; }

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

    static void flatten(const SkFlattenable* flattenable,
                        SkJSONWriter&        writer,
                        UrlDataManager&      urlDataManager);
    static bool flatten(const SkImage& image, SkJSONWriter& writer, UrlDataManager& urlDataManager);
    static bool flatten(const SkBitmap& bitmap,
                        SkJSONWriter&   writer,
                        UrlDataManager& urlDataManager);

private:
    OpType fOpType;
    bool   fVisible;
};

class RestoreCommand : public DrawCommand {
public:
    RestoreCommand();
    void execute(SkCanvas* canvas) const override;

private:
    typedef DrawCommand INHERITED;
};

class ClearCommand : public DrawCommand {
public:
    ClearCommand(SkColor color);
    void execute(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkColor fColor;

    typedef DrawCommand INHERITED;
};

class ClipPathCommand : public DrawCommand {
public:
    ClipPathCommand(const SkPath& path, SkClipOp op, bool doAA);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkPath   fPath;
    SkClipOp fOp;
    bool     fDoAA;

    typedef DrawCommand INHERITED;
};

class ClipRegionCommand : public DrawCommand {
public:
    ClipRegionCommand(const SkRegion& region, SkClipOp op);
    void execute(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkRegion fRegion;
    SkClipOp fOp;

    typedef DrawCommand INHERITED;
};

class ClipRectCommand : public DrawCommand {
public:
    ClipRectCommand(const SkRect& rect, SkClipOp op, bool doAA);
    void execute(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkRect   fRect;
    SkClipOp fOp;
    bool     fDoAA;

    typedef DrawCommand INHERITED;
};

class ClipRRectCommand : public DrawCommand {
public:
    ClipRRectCommand(const SkRRect& rrect, SkClipOp op, bool doAA);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkRRect  fRRect;
    SkClipOp fOp;
    bool     fDoAA;

    typedef DrawCommand INHERITED;
};

class ConcatCommand : public DrawCommand {
public:
    ConcatCommand(const SkMatrix& matrix);
    void execute(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkMatrix fMatrix;

    typedef DrawCommand INHERITED;
};

class DrawAnnotationCommand : public DrawCommand {
public:
    DrawAnnotationCommand(const SkRect&, const char key[], sk_sp<SkData> value);
    void execute(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkRect        fRect;
    SkString      fKey;
    sk_sp<SkData> fValue;

    typedef DrawCommand INHERITED;
};

class DrawBitmapCommand : public DrawCommand {
public:
    DrawBitmapCommand(const SkBitmap& bitmap, SkScalar left, SkScalar top, const SkPaint* paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkBitmap         fBitmap;
    SkScalar         fLeft;
    SkScalar         fTop;
    SkTLazy<SkPaint> fPaint;

    typedef DrawCommand INHERITED;
};

class DrawBitmapLatticeCommand : public DrawCommand {
public:
    DrawBitmapLatticeCommand(const SkBitmap&          bitmap,
                             const SkCanvas::Lattice& lattice,
                             const SkRect&            dst,
                             const SkPaint*           paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkBitmap          fBitmap;
    SkCanvas::Lattice fLattice;
    SkRect            fDst;
    SkTLazy<SkPaint>  fPaint;

    typedef DrawCommand INHERITED;
};

class DrawBitmapNineCommand : public DrawCommand {
public:
    DrawBitmapNineCommand(const SkBitmap& bitmap,
                          const SkIRect&  center,
                          const SkRect&   dst,
                          const SkPaint*  paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkBitmap         fBitmap;
    SkIRect          fCenter;
    SkRect           fDst;
    SkTLazy<SkPaint> fPaint;

    typedef DrawCommand INHERITED;
};

class DrawBitmapRectCommand : public DrawCommand {
public:
    DrawBitmapRectCommand(const SkBitmap& bitmap,
                          const SkRect*   src,
                          const SkRect&   dst,
                          const SkPaint*  paint,
                          SkCanvas::SrcRectConstraint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkBitmap                    fBitmap;
    SkTLazy<SkRect>             fSrc;
    SkRect                      fDst;
    SkTLazy<SkPaint>            fPaint;
    SkCanvas::SrcRectConstraint fConstraint;

    typedef DrawCommand INHERITED;
};

class DrawImageCommand : public DrawCommand {
public:
    DrawImageCommand(const SkImage* image, SkScalar left, SkScalar top, const SkPaint* paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    sk_sp<const SkImage> fImage;
    SkScalar             fLeft;
    SkScalar             fTop;
    SkTLazy<SkPaint>     fPaint;

    typedef DrawCommand INHERITED;
};

class DrawImageLatticeCommand : public DrawCommand {
public:
    DrawImageLatticeCommand(const SkImage*           image,
                            const SkCanvas::Lattice& lattice,
                            const SkRect&            dst,
                            const SkPaint*           paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    sk_sp<const SkImage> fImage;
    SkCanvas::Lattice    fLattice;
    SkRect               fDst;
    SkTLazy<SkPaint>     fPaint;

    typedef DrawCommand INHERITED;
};

class DrawImageNineCommand : public DrawCommand {
public:
    DrawImageNineCommand(const SkImage* image,
                         const SkIRect& center,
                         const SkRect&  dst,
                         const SkPaint* paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    sk_sp<const SkImage> fImage;
    SkIRect              fCenter;
    SkRect               fDst;
    SkTLazy<SkPaint>     fPaint;

    typedef DrawCommand INHERITED;
};

class DrawImageRectCommand : public DrawCommand {
public:
    DrawImageRectCommand(const SkImage*              image,
                         const SkRect*               src,
                         const SkRect&               dst,
                         const SkPaint*              paint,
                         SkCanvas::SrcRectConstraint constraint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    sk_sp<const SkImage>        fImage;
    SkTLazy<SkRect>             fSrc;
    SkRect                      fDst;
    SkTLazy<SkPaint>            fPaint;
    SkCanvas::SrcRectConstraint fConstraint;

    typedef DrawCommand INHERITED;
};

class DrawOvalCommand : public DrawCommand {
public:
    DrawOvalCommand(const SkRect& oval, const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkRect  fOval;
    SkPaint fPaint;

    typedef DrawCommand INHERITED;
};

class DrawArcCommand : public DrawCommand {
public:
    DrawArcCommand(const SkRect&  oval,
                   SkScalar       startAngle,
                   SkScalar       sweepAngle,
                   bool           useCenter,
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

    typedef DrawCommand INHERITED;
};

class DrawPaintCommand : public DrawCommand {
public:
    DrawPaintCommand(const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkPaint fPaint;

    typedef DrawCommand INHERITED;
};

class DrawBehindCommand : public DrawCommand {
public:
    DrawBehindCommand(const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkPaint fPaint;

    typedef DrawCommand INHERITED;
};

class DrawPathCommand : public DrawCommand {
public:
    DrawPathCommand(const SkPath& path, const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkPath  fPath;
    SkPaint fPaint;

    typedef DrawCommand INHERITED;
};

class BeginDrawPictureCommand : public DrawCommand {
public:
    BeginDrawPictureCommand(const SkPicture* picture, const SkMatrix* matrix, const SkPaint* paint);

    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;

private:
    sk_sp<const SkPicture> fPicture;
    SkTLazy<SkMatrix>      fMatrix;
    SkTLazy<SkPaint>       fPaint;

    typedef DrawCommand INHERITED;
};

class EndDrawPictureCommand : public DrawCommand {
public:
    EndDrawPictureCommand(bool restore);

    void execute(SkCanvas* canvas) const override;

private:
    bool fRestore;

    typedef DrawCommand INHERITED;
};

class DrawPointsCommand : public DrawCommand {
public:
    DrawPointsCommand(SkCanvas::PointMode mode,
                      size_t              count,
                      const SkPoint       pts[],
                      const SkPaint&      paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkCanvas::PointMode fMode;
    SkTDArray<SkPoint>  fPts;
    SkPaint             fPaint;

    typedef DrawCommand INHERITED;
};

class DrawRegionCommand : public DrawCommand {
public:
    DrawRegionCommand(const SkRegion& region, const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkRegion fRegion;
    SkPaint  fPaint;

    typedef DrawCommand INHERITED;
};

class DrawTextBlobCommand : public DrawCommand {
public:
    DrawTextBlobCommand(sk_sp<SkTextBlob> blob, SkScalar x, SkScalar y, const SkPaint& paint);

    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    sk_sp<SkTextBlob> fBlob;
    SkScalar          fXPos;
    SkScalar          fYPos;
    SkPaint           fPaint;

    typedef DrawCommand INHERITED;
};

class DrawPatchCommand : public DrawCommand {
public:
    DrawPatchCommand(const SkPoint  cubics[12],
                     const SkColor  colors[4],
                     const SkPoint  texCoords[4],
                     SkBlendMode    bmode,
                     const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkPoint     fCubics[12];
    SkColor*    fColorsPtr;
    SkColor     fColors[4];
    SkPoint*    fTexCoordsPtr;
    SkPoint     fTexCoords[4];
    SkBlendMode fBlendMode;
    SkPaint     fPaint;

    typedef DrawCommand INHERITED;
};

class DrawRectCommand : public DrawCommand {
public:
    DrawRectCommand(const SkRect& rect, const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkRect  fRect;
    SkPaint fPaint;

    typedef DrawCommand INHERITED;
};

class DrawRRectCommand : public DrawCommand {
public:
    DrawRRectCommand(const SkRRect& rrect, const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkRRect fRRect;
    SkPaint fPaint;

    typedef DrawCommand INHERITED;
};

class DrawDRRectCommand : public DrawCommand {
public:
    DrawDRRectCommand(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkRRect fOuter;
    SkRRect fInner;
    SkPaint fPaint;

    typedef DrawCommand INHERITED;
};

class DrawVerticesCommand : public DrawCommand {
public:
    DrawVerticesCommand(sk_sp<SkVertices>, SkBlendMode, const SkPaint&);

    void execute(SkCanvas* canvas) const override;

private:
    sk_sp<SkVertices> fVertices;
    SkBlendMode       fBlendMode;
    SkPaint           fPaint;

    typedef DrawCommand INHERITED;
};

class DrawAtlasCommand : public DrawCommand {
public:
    DrawAtlasCommand(const SkImage*,
                     const SkRSXform[],
                     const SkRect[],
                     const SkColor[],
                     int,
                     SkBlendMode,
                     const SkRect*,
                     const SkPaint*);

    void execute(SkCanvas* canvas) const override;

private:
    sk_sp<const SkImage> fImage;
    SkTDArray<SkRSXform> fXform;
    SkTDArray<SkRect>    fTex;
    SkTDArray<SkColor>   fColors;
    SkBlendMode          fBlendMode;
    SkTLazy<SkRect>      fCull;
    SkTLazy<SkPaint>     fPaint;

    typedef DrawCommand INHERITED;
};

class SaveCommand : public DrawCommand {
public:
    SaveCommand();
    void execute(SkCanvas* canvas) const override;

private:
    typedef DrawCommand INHERITED;
};

class SaveLayerCommand : public DrawCommand {
public:
    SaveLayerCommand(const SkCanvas::SaveLayerRec&);
    void execute(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkTLazy<SkRect>            fBounds;
    SkTLazy<SkPaint>           fPaint;
    sk_sp<const SkImageFilter> fBackdrop;
    uint32_t                   fSaveLayerFlags;

    typedef DrawCommand INHERITED;
};

class SetMatrixCommand : public DrawCommand {
public:
    SetMatrixCommand(const SkMatrix& matrix);
    void execute(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkMatrix fMatrix;

    typedef DrawCommand INHERITED;
};

class DrawShadowCommand : public DrawCommand {
public:
    DrawShadowCommand(const SkPath& path, const SkDrawShadowRec& rec);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkPath          fPath;
    SkDrawShadowRec fShadowRec;

    typedef DrawCommand INHERITED;
};

class DrawDrawableCommand : public DrawCommand {
public:
    DrawDrawableCommand(SkDrawable*, const SkMatrix*);
    void execute(SkCanvas* canvas) const override;

private:
    sk_sp<SkDrawable> fDrawable;
    SkTLazy<SkMatrix> fMatrix;

    typedef DrawCommand INHERITED;
};

class DrawEdgeAAQuadCommand : public DrawCommand {
public:
    DrawEdgeAAQuadCommand(const SkRect&         rect,
                          const SkPoint         clip[4],
                          SkCanvas::QuadAAFlags aa,
                          SkColor               color,
                          SkBlendMode           mode);
    void execute(SkCanvas* canvas) const override;

private:
    SkRect                fRect;
    SkPoint               fClip[4];
    int                   fHasClip;
    SkCanvas::QuadAAFlags fAA;
    SkColor               fColor;
    SkBlendMode           fMode;

    typedef DrawCommand INHERITED;
};

class DrawEdgeAAImageSetCommand : public DrawCommand {
public:
    DrawEdgeAAImageSetCommand(const SkCanvas::ImageSetEntry[],
                              int count,
                              const SkPoint[],
                              const SkMatrix[],
                              const SkPaint*,
                              SkCanvas::SrcRectConstraint);
    void execute(SkCanvas* canvas) const override;

private:
    SkAutoTArray<SkCanvas::ImageSetEntry> fSet;
    int                                   fCount;
    SkAutoTArray<SkPoint>                 fDstClips;
    SkAutoTArray<SkMatrix>                fPreViewMatrices;
    SkTLazy<SkPaint>                      fPaint;
    SkCanvas::SrcRectConstraint           fConstraint;

    typedef DrawCommand INHERITED;
};
#endif
