/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKDRAWCOMMAND_H_
#define SKDRAWCOMMAND_H_

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkRegion.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypes.h"
#include "include/core/SkVertices.h"
#include "include/private/base/SkTDArray.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkDrawShadowInfo.h"

#include <cstddef>
#include <cstdint>

class DebugLayerManager;
class SkBitmap;
class SkFlattenable;
class SkJSONWriter;
class SkWStream;
class UrlDataManager;
enum class SkBlendMode;
enum class SkClipOp;
struct SkPoint3;
struct SkRSXform;

class DrawCommand {
public:
    enum OpType {
        kBeginDrawPicture_OpType,
        kClear_OpType,
        kClipPath_OpType,
        kClipRegion_OpType,
        kClipRect_OpType,
        kClipRRect_OpType,
        kClipShader_OpType,
        kResetClip_OpType,
        kConcat_OpType,
        kConcat44_OpType,
        kDrawAnnotation_OpType,
        kDrawBitmap_OpType,
        kDrawBitmapRect_OpType,
        kDrawDRRect_OpType,
        kDrawImage_OpType,
        kDrawImageLattice_OpType,
        kDrawImageRect_OpType,
        kDrawImageRectLayer_OpType, // unique to DebugCanvas
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
        kSetM44_OpType,

        kLast_OpType = kSetM44_OpType
    };

    static const int kOpTypeCount = kLast_OpType + 1;

    static void WritePNG(const SkBitmap& bitmap, SkWStream& out);

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
    static void MakeJsonMatrix44(SkJSONWriter&, const SkM44&);
    static void MakeJsonPath(SkJSONWriter&, const SkPath& path);
    static void MakeJsonRegion(SkJSONWriter&, const SkRegion& region);
    static void MakeJsonSampling(SkJSONWriter&, const SkSamplingOptions& sampling);
    static void MakeJsonPaint(SkJSONWriter&, const SkPaint& paint, UrlDataManager& urlDataManager);
    static void MakeJsonLattice(SkJSONWriter&, const SkCanvas::Lattice& lattice);

    static void flatten(const SkFlattenable* flattenable,
                        SkJSONWriter&        writer,
                        UrlDataManager&      urlDataManager);
    static bool flatten(const SkImage& image, SkJSONWriter& writer, UrlDataManager& urlDataManager);
    static bool flatten(const SkBitmap& bitmap,
                        SkJSONWriter&   writer,
                        UrlDataManager& urlDataManager);
    OpType getOpType() const { return fOpType; }

private:
    OpType fOpType;
    bool   fVisible;
};

class RestoreCommand : public DrawCommand {
public:
    RestoreCommand();
    void execute(SkCanvas* canvas) const override;

private:
    using INHERITED = DrawCommand;
};

class ClearCommand : public DrawCommand {
public:
    ClearCommand(SkColor color);
    void execute(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkColor fColor;

    using INHERITED = DrawCommand;
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

    using INHERITED = DrawCommand;
};

class ClipRegionCommand : public DrawCommand {
public:
    ClipRegionCommand(const SkRegion& region, SkClipOp op);
    void execute(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkRegion fRegion;
    SkClipOp fOp;

    using INHERITED = DrawCommand;
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

    using INHERITED = DrawCommand;
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

    using INHERITED = DrawCommand;
};

class ClipShaderCommand : public DrawCommand {
public:
    ClipShaderCommand(sk_sp<SkShader>, SkClipOp);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    sk_sp<SkShader> fShader;
    SkClipOp fOp;

    using INHERITED = DrawCommand;
};

class ResetClipCommand : public DrawCommand {
public:
    ResetClipCommand();
    void execute(SkCanvas* canvas) const override;

private:
    using INHERITED = DrawCommand;
};

class ConcatCommand : public DrawCommand {
public:
    ConcatCommand(const SkMatrix& matrix);
    void execute(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkMatrix fMatrix;

    using INHERITED = DrawCommand;
};

class Concat44Command : public DrawCommand {
public:
    Concat44Command(const SkM44& matrix);
    void execute(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkM44 fMatrix;

    using INHERITED = DrawCommand;
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

    using INHERITED = DrawCommand;
};

class DrawImageCommand : public DrawCommand {
public:
    DrawImageCommand(const SkImage* image, SkScalar left, SkScalar top,
                     const SkSamplingOptions&, const SkPaint* paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;
    uint64_t imageId(UrlDataManager& udb) const;

private:
    sk_sp<const SkImage> fImage;
    SkScalar             fLeft;
    SkScalar             fTop;
    SkSamplingOptions    fSampling;
    SkTLazy<SkPaint>     fPaint;

    using INHERITED = DrawCommand;
};

class DrawImageLatticeCommand : public DrawCommand {
public:
    DrawImageLatticeCommand(const SkImage*           image,
                            const SkCanvas::Lattice& lattice,
                            const SkRect&            dst,
                            SkFilterMode,
                            const SkPaint*           paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;
    uint64_t imageId(UrlDataManager& udb) const;

private:
    sk_sp<const SkImage> fImage;
    SkCanvas::Lattice    fLattice;
    SkRect               fDst;
    SkFilterMode         fFilter;
    SkTLazy<SkPaint>     fPaint;

    using INHERITED = DrawCommand;
};

class DrawImageRectCommand : public DrawCommand {
public:
    DrawImageRectCommand(const SkImage*              image,
                         const SkRect&               src,
                         const SkRect&               dst,
                         const SkSamplingOptions&    sampling,
                         const SkPaint*              paint,
                         SkCanvas::SrcRectConstraint constraint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;
    uint64_t imageId(UrlDataManager& udm) const;

private:
    sk_sp<const SkImage>        fImage;
    SkRect                      fSrc;
    SkRect                      fDst;
    SkSamplingOptions           fSampling;
    SkTLazy<SkPaint>            fPaint;
    SkCanvas::SrcRectConstraint fConstraint;

    using INHERITED = DrawCommand;
};

// Command for resolving the deferred SkImage representing an android layer
// Functions like DrawImageRect except it uses the saved UrlDataManager to resolve the image
// at the time execute() is called.
class DrawImageRectLayerCommand : public DrawCommand {
public:
    DrawImageRectLayerCommand(DebugLayerManager*          layerManager,
                              const int                   nodeId,
                              const int                   frame,
                              const SkRect&               src,
                              const SkRect&               dst,
                              const SkSamplingOptions&    sampling,
                              const SkPaint*              paint,
                              SkCanvas::SrcRectConstraint constraint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    DebugLayerManager*          fLayerManager;
    int                         fNodeId;
    int                         fFrame;
    SkRect                      fSrc;
    SkRect                      fDst;
    SkSamplingOptions           fSampling;
    SkTLazy<SkPaint>            fPaint;
    SkCanvas::SrcRectConstraint fConstraint;

    using INHERITED = DrawCommand;
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

    using INHERITED = DrawCommand;
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

    using INHERITED = DrawCommand;
};

class DrawPaintCommand : public DrawCommand {
public:
    DrawPaintCommand(const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkPaint fPaint;

    using INHERITED = DrawCommand;
};

class DrawBehindCommand : public DrawCommand {
public:
    DrawBehindCommand(const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    bool render(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkPaint fPaint;

    using INHERITED = DrawCommand;
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

    using INHERITED = DrawCommand;
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

    using INHERITED = DrawCommand;
};

class EndDrawPictureCommand : public DrawCommand {
public:
    EndDrawPictureCommand(bool restore);

    void execute(SkCanvas* canvas) const override;

private:
    bool fRestore;

    using INHERITED = DrawCommand;
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

    using INHERITED = DrawCommand;
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

    using INHERITED = DrawCommand;
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

    using INHERITED = DrawCommand;
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

    using INHERITED = DrawCommand;
};

class DrawRectCommand : public DrawCommand {
public:
    DrawRectCommand(const SkRect& rect, const SkPaint& paint);
    void execute(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkRect  fRect;
    SkPaint fPaint;

    using INHERITED = DrawCommand;
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

    using INHERITED = DrawCommand;
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

    using INHERITED = DrawCommand;
};

class DrawVerticesCommand : public DrawCommand {
public:
    DrawVerticesCommand(sk_sp<SkVertices>, SkBlendMode, const SkPaint&);

    void execute(SkCanvas* canvas) const override;

private:
    sk_sp<SkVertices> fVertices;
    SkBlendMode       fBlendMode;
    SkPaint           fPaint;

    using INHERITED = DrawCommand;
};

class DrawAtlasCommand : public DrawCommand {
public:
    DrawAtlasCommand(const SkImage*,
                     const SkRSXform[],
                     const SkRect[],
                     const SkColor[],
                     int,
                     SkBlendMode,
                     const SkSamplingOptions&,
                     const SkRect*,
                     const SkPaint*);

    void execute(SkCanvas* canvas) const override;

private:
    sk_sp<const SkImage> fImage;
    SkTDArray<SkRSXform> fXform;
    SkTDArray<SkRect>    fTex;
    SkTDArray<SkColor>   fColors;
    SkBlendMode          fBlendMode;
    SkSamplingOptions    fSampling;
    SkTLazy<SkRect>      fCull;
    SkTLazy<SkPaint>     fPaint;

    using INHERITED = DrawCommand;
};

class SaveCommand : public DrawCommand {
public:
    SaveCommand();
    void execute(SkCanvas* canvas) const override;

private:
    using INHERITED = DrawCommand;
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
    SkScalar                   fBackdropScale;

    using INHERITED = DrawCommand;
};

class SetMatrixCommand : public DrawCommand {
public:
    SetMatrixCommand(const SkMatrix& matrix);
    void execute(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkMatrix fMatrix;

    using INHERITED = DrawCommand;
};

class SetM44Command : public DrawCommand {
public:
    SetM44Command(const SkM44& matrix);
    void execute(SkCanvas* canvas) const override;
    void toJSON(SkJSONWriter& writer, UrlDataManager& urlDataManager) const override;

private:
    SkM44 fMatrix;

    using INHERITED = DrawCommand;
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

    using INHERITED = DrawCommand;
};

class DrawDrawableCommand : public DrawCommand {
public:
    DrawDrawableCommand(SkDrawable*, const SkMatrix*);
    void execute(SkCanvas* canvas) const override;

private:
    sk_sp<SkDrawable> fDrawable;
    SkTLazy<SkMatrix> fMatrix;

    using INHERITED = DrawCommand;
};

class DrawEdgeAAQuadCommand : public DrawCommand {
public:
    DrawEdgeAAQuadCommand(const SkRect&         rect,
                          const SkPoint         clip[4],
                          SkCanvas::QuadAAFlags aa,
                          const SkColor4f&      color,
                          SkBlendMode           mode);
    void execute(SkCanvas* canvas) const override;

private:
    SkRect                fRect;
    SkPoint               fClip[4];
    int                   fHasClip;
    SkCanvas::QuadAAFlags fAA;
    SkColor4f             fColor;
    SkBlendMode           fMode;

    using INHERITED = DrawCommand;
};

class DrawEdgeAAImageSetCommand : public DrawCommand {
public:
    DrawEdgeAAImageSetCommand(const SkCanvas::ImageSetEntry[],
                              int count,
                              const SkPoint[],
                              const SkMatrix[],
                              const SkSamplingOptions&,
                              const SkPaint*,
                              SkCanvas::SrcRectConstraint);
    void execute(SkCanvas* canvas) const override;

private:
    skia_private::AutoTArray<SkCanvas::ImageSetEntry> fSet;
    int                                               fCount;
    skia_private::AutoTArray<SkPoint>                 fDstClips;
    skia_private::AutoTArray<SkMatrix>                fPreViewMatrices;
    SkSamplingOptions                                 fSampling;
    SkTLazy<SkPaint>                                  fPaint;
    SkCanvas::SrcRectConstraint                       fConstraint;

    using INHERITED = DrawCommand;
};
#endif
