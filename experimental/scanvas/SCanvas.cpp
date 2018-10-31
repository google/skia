// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "SCanvas.h"

#include "Target.h"

#include "SkClipStack.h"
#include "SkGlyphRun.h"
#include "SkPathPriv.h"
#include "SkRSXform.h"
#include "SkUtils.h"
#include "SkPatchUtils.h"
#include "SkLatticeIter.h"
#include "SkSurface.h"

#include <vector>

static bool is_clip_rect(const SkClipStack& clipStack, SkISize size) {
    if (clipStack.isWideOpen()) {
        return true;
    }
    if (clipStack.isEmpty(SkIRect::MakeSize(size))) {
        return false;
    }
    SkClipStack::BoundsType boundType;
    bool isIntersectionOfRects;
    SkRect bounds;
    clipStack.getBounds(&bounds, &boundType, &isIntersectionOfRects);
    return isIntersectionOfRects && SkClipStack::kNormal_BoundsType == boundType;
}

static SkPath wide_open_path() {
    SkPath path;
    path.toggleInverseFillType();
    path.setIsVolatile(false);
    return path;
}

static SkPath to_path(SkRect r) {
    SkPath path;
    path.addRect(r);
    path.setIsVolatile(false);
    return path;
}

static SkPath to_path(const SkRRect& rr) {
    SkPath path;
    path.addRRect(rr);
    path.setIsVolatile(false);
    return path;
}

static SkPath drrect_to_path(const SkRRect& outer, const SkRRect& inner) {
    SkPath path;
    path.addRRect(outer);
    path.addRRect(inner);
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.setIsVolatile(false);
    return path;
}

static SkPath oval_to_path(SkRect oval) {
    SkPath path;
    path.addOval(oval);
    path.setIsVolatile(false);
    return path;
}

static SkPath arc_to_path(const SkRect& oval, SkScalar startAngle,
                          SkScalar sweepAngle, bool useCenter, bool isFillNoPathEffect) {
    SkPath path;
    SkPathPriv::CreateDrawArcPath(&path, oval, startAngle, sweepAngle, useCenter,
                                  isFillNoPathEffect);
    path.setIsVolatile(false);
    return path;
}

static SkPath to_path(const SkRegion& region) {
    SkPath path;
    region.getBoundaryPath(&path);
    path.setIsVolatile(false);
    return path;
}

static void draw_atlas(SkCanvas* canvas, const SkImage* atlas, const SkRSXform xform[],
                       const SkRect tex[], const SkColor colors[], int quadCount,
                       SkBlendMode mode, const SkPaint& paint) {
    const int triCount = quadCount << 1;
    const int vertexCount = triCount * 3;
    uint32_t flags = SkVertices::kHasTexCoords_BuilderFlag;
    if (colors) {
        flags |= SkVertices::kHasColors_BuilderFlag;
    }
    SkVertices::Builder builder(SkVertices::kTriangles_VertexMode, vertexCount, 0, flags);

    SkPoint* vPos = builder.positions();
    SkPoint* vTex = builder.texCoords();
    SkColor* vCol = builder.colors();
    for (int i = 0; i < quadCount; ++i) {
        SkPoint tmp[4];
        xform[i].toQuad(tex[i].width(), tex[i].height(), tmp);
        static constexpr unsigned kQuadToTris[6] = {0, 1, 2, 0, 2, 3};
        for (unsigned idx : kQuadToTris) { *vPos++ = tmp[idx]; }
        tex[i].toQuad(tmp);
        for (unsigned idx : kQuadToTris) { *vTex++ = tmp[idx]; }
        if (colors) {
            sk_memset32(vCol, colors[i], 6);
            vCol += 6;
        }
    }
    SkPaint p(paint);
    p.setShader(atlas->makeShader());
    auto verts = builder.detach();
    canvas->drawVertices(verts.get(), nullptr, 0, mode, p);
}

namespace {

class SkAutoDrawLooper {
public:
    SkAutoDrawLooper(SkCanvas* canvas, const SkPaint* paint)
        : fCanvas(canvas), fPaint(MakeCopy(paint)), fAlloc(0) {}
    struct Iter {
        void operator++() {
            if (!fLooperContext || !fLooperContext->next(fCanvas, fPaint->writable())) {
                fCanvas = nullptr;
            }
        }
        const SkPaint& operator*() const { return **fPaint; }
        bool operator!=(const Iter& rhs) const { return fCanvas != rhs.fCanvas; }
        SkCanvas* fCanvas;
        SkDrawLooper::Context* fLooperContext;
        SkTCopyOnFirstWrite<SkPaint>* fPaint;
    };
    Iter begin() {
        SkDrawLooper* looper = fPaint->getDrawLooper();
        return Iter{fCanvas, looper ? looper->makeContext(fCanvas, &fAlloc) : nullptr, &fPaint};
    }
    Iter end() { return Iter{nullptr, nullptr, nullptr}; }

    template <typename T>
    static SkTCopyOnFirstWrite<T> MakeCopy(const T* ptr) {
        return ptr ? SkTCopyOnFirstWrite<T>(*ptr) : SkTCopyOnFirstWrite<T>(T{});
    }

private:
    SkCanvas* fCanvas;
    SkTCopyOnFirstWrite<SkPaint> fPaint;
    SkArenaAlloc fAlloc;
};

class SkClipStackImpl : public SkClipStack {
public:
    SkClipStackImpl() : SkClipStack(fStorage, sizeof(fStorage)) {}
private:
    SkClipStack::Element fStorage[16];
};


//struct T {
//    T(SkISize);
//    ~T();
//    SkISize size() const;
//    void flush();
//    void discard();
//    void saveLayer(const SkClipStack&, const SkMatrix&, const SkCanvas::SaveLayerRec&);
//    void restoreLayer();
//    void drawGlyphs(const SkClipStack&, const SkMatrix&,
//                    const SkGlyphRunList&);
//    void drawGlyphsRSXform(const SkClipStack&, const SkMatrix&,
//                           const SkGlyphRunList&, const SkRSXform*);
//    void drawPath(const SkClipStack&, const SkMatrix&,
//                  const SkPath& path, const SkPaint&);
//    void drawPoints(const SkClipStack&, const SkMatrix&,
//                    SkCanvas::PointMode, size_t, const SkPoint*, const SkPaint&);
//    void drawVertices(const SkClipStack&, const SkMatrix&,
//                      const SkVertices*, const SkVertices::Bone*,
//                      int, SkBlendMode, const SkPaint&);
//    void drawImageRect(const SkClipStack&, const SkMatrix&,
//                       const SkImage*, const SkRect*, const SkRect&,
//                       const SkPaint*, SkCanvas::SrcRectConstraint);
//    void drawAnnotation(const SkClipStack&, const SkMatrix&,
//                        const SkRect&, const char key[], SkData* value);
//    void drawShadowRec(const SkClipStack&, const SkMatrix&, const SkPath&, SkDrawShadowRec&);
//};

template <class T>
struct ClipStackCanvas final : public SkCanvas {
    SkClipStackImpl fClipStack;
    T fTarget;
    std::vector<int> fLayerSaves;

    ClipStackCanvas(SkISize s) : SkCanvas(s.width(), s.height()), fTarget(s) {}
    ~ClipStackCanvas() {}

    SkISize getBaseLayerSize() const override { return fTarget.size(); }
    GrContext* getGrContext() override { return nullptr; }
    bool isClipEmpty() const override {
        return fClipStack.isEmpty(SkIRect::MakeSize(fTarget.size()));
    }
    bool isClipRect() const override {
        return is_clip_rect(fClipStack, fTarget.size());
    }
    GrRenderTargetContext* internal_private_accessTopLayerRenderTargetContext() override {
        return nullptr;
    }
    sk_sp<SkSurface> onNewSurface(const SkImageInfo& info, const SkSurfaceProps& props) override {
        return nullptr;
    }
    bool onPeekPixels(SkPixmap* pixmap) override { return false; }
    bool onAccessTopLayerPixels(SkPixmap* pixmap) override { return false; }
    SkImageInfo onImageInfo() const override {
        return SkImageInfo::MakeUnknown(fTarget.size().width(), fTarget.size().height());
    }
    bool onGetProps(SkSurfaceProps* props) const override { return false; }
    void willSave() override { fClipStack.save(); }
    void willRestore() override { fClipStack.restore(); }

    void onDrawPaint(const SkPaint& paint) override { this->onDrawPath(wide_open_path(), paint); }
    void onDrawRect(const SkRect& rect, const SkPaint& paint) override {
        this->onDrawPath(to_path(rect), paint);
    }
    void onDrawRRect(const SkRRect& rrect, const SkPaint& paint) override {
        this->onDrawPath(to_path(rrect), paint);
    }
    void onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) override {
        this->onDrawPath(drrect_to_path(outer, inner), paint);
    }
    void onDrawOval(const SkRect& rect, const SkPaint& paint) override {
        this->onDrawPath(oval_to_path(rect), paint);
    }
    void onDrawArc(const SkRect& rect, SkScalar start, SkScalar sweep,
                   bool useCenter, const SkPaint& paint) override {
        bool fillNoPathEffect = SkPaint::kFill_Style == paint.getStyle() && !paint.getPathEffect();
        this->onDrawPath(arc_to_path(rect, start, sweep, useCenter, fillNoPathEffect), paint);
    }

    void onDrawRegion(const SkRegion& region, const SkPaint& paint) override {
        this->onDrawPath(to_path(region), paint);
    }
    void onDrawText(const void* text, size_t byteLength, SkScalar x,
                    SkScalar y, const SkPaint& paint) override {
        SkGlyphRunBuilder grb;
        grb.drawText(paint, text, byteLength, SkPoint{x, y});
        for (const SkPaint& p : SkAutoDrawLooper(this, &paint)) {
            (void)p;
            this->drawGlyphs(grb.useGlyphRunList());
        }
    }
    void onDrawPosText(const void* text, size_t byteLength,
                       const SkPoint pos[], const SkPaint& paint) override {
        SkGlyphRunBuilder grb;
        grb.drawPosText(paint, text, byteLength, pos);
        for (const SkPaint& p : SkAutoDrawLooper(this, &paint)) {
            (void)p;
            this->drawGlyphs(grb.useGlyphRunList());
        }
    }
    void onDrawPosTextH(const void* text, size_t byteLength,
                        const SkScalar xpos[], SkScalar constY,
                        const SkPaint& paint) override {
        SkGlyphRunBuilder grb;
        grb.drawPosTextH(paint, text, byteLength, xpos, constY);
        for (const SkPaint& p : SkAutoDrawLooper(this, &paint)) {
            (void)p;
            this->drawGlyphs(grb.useGlyphRunList());
        }
    }
    void onDrawTextRSXform(const void* text, size_t byteLength, const SkRSXform xform[],
                           const SkRect* cullRect, const SkPaint& paint) override {
        SkGlyphRunBuilder grb;
        grb.drawTextAtOrigin(paint, text, byteLength);
        for (const SkPaint& p : SkAutoDrawLooper(this, &paint)) {
            (void)p;
            this->drawGlyphsRSXform(grb.useGlyphRunList(), xform);
        }
    }
    void onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                        const SkPaint& paint) override {
        SkGlyphRunBuilder grb;
        grb.drawTextBlob(paint, *blob, SkPoint{x, y});
        for (const SkPaint& p : SkAutoDrawLooper(this, &paint)) {
            (void)p;
            this->drawGlyphs(grb.useGlyphRunList());
        }
    }
    void onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                     const SkPoint texCoords[4], SkBlendMode mode, const SkPaint& paint) override {

        SkISize lod = SkPatchUtils::GetLevelOfDetail(cubics, &this->getTotalMatrix());
        sk_sp<SkVertices> vertices = SkPatchUtils::MakeVertices(cubics, colors, texCoords,
                                                                lod.width(), lod.height(), nullptr);
        if (vertices) {
            this->onDrawVerticesObject(vertices.get(), nullptr, 0, mode, paint);
        }
    }
    void onDrawImage(const SkImage* image, SkScalar dx, SkScalar dy,
                     const SkPaint* paint) override {
        this->onDrawImageRect(image,
                nullptr, SkRect::Make(image->bounds().makeOffset(dx, dy)), paint,
                SkCanvas::kStrict_SrcRectConstraint);
    }
    void onDrawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                         const SkPaint* paint, SrcRectConstraint constraint) override {
        for (const SkPaint& p : SkAutoDrawLooper(this, paint)) {
            this->drawImageImpl(image, src, dst, &p, constraint);
        }
    }
    void onDrawImageNine(const SkImage* image, const SkIRect& center, const SkRect& dst,
                         const SkPaint* paint) override {
        for (const SkPaint& p : SkAutoDrawLooper(this, paint)) {
            SkLatticeIter iter(image->width(), image->height(), center, dst);
            SkRect srcR, dstR;
            while (iter.next(&srcR, &dstR)) {
                this->drawImageImpl(image, &srcR, dstR, &p, SkCanvas::kStrict_SrcRectConstraint);
            }
        }
    }
    void onDrawImageLattice(const SkImage* image, const Lattice& lattice, const SkRect& dst,
                            const SkPaint* paint) override {
        for (const SkPaint& p : SkAutoDrawLooper(this, paint)) {
            SkLatticeIter iter(lattice, dst);
            SkRect srcR, dstR;
            while (iter.next(&srcR, &dstR)) {
                this->drawImageImpl(image, &srcR, dstR,  &p, SkCanvas::kStrict_SrcRectConstraint);
            }
        }
    }

    void onDrawBitmap(const SkBitmap& bitmap, SkScalar dx, SkScalar dy,
                      const SkPaint* paint) override {
        this->onDrawImage(SkImage::MakeFromBitmap(bitmap).get(), dx, dy, paint);
    }
    void onDrawBitmapRect(const SkBitmap& bitmap, const SkRect* src, const SkRect& dst,
                          const SkPaint* paint, SrcRectConstraint constraint) override {
        this->onDrawImageRect(SkImage::MakeFromBitmap(bitmap).get(), src, dst, paint, constraint);
    }
    void onDrawBitmapNine(const SkBitmap& bitmap, const SkIRect& center, const SkRect& dst,
                          const SkPaint* paint) override {
        this->onDrawImageNine(SkImage::MakeFromBitmap(bitmap).get(), center, dst, paint);
    }
    void onDrawBitmapLattice(const SkBitmap& bitmap, const Lattice& lattice,
                             const SkRect& dst, const SkPaint* paint) override {
        this->onDrawImageLattice(SkImage::MakeFromBitmap(bitmap).get(), lattice, dst, paint);
    }

    void onDrawAtlas(const SkImage* atlas, const SkRSXform xform[], const SkRect rect[],
                     const SkColor colors[], int count, SkBlendMode mode,
                     const SkRect* cull, const SkPaint* paint) override {
        draw_atlas(this, atlas, xform, rect, colors, count, mode, paint ? *paint : SkPaint());
    }

    void onClipRect(const SkRect& rect, SkClipOp op, ClipEdgeStyle edgeStyle) override {
        const bool isAA = SkCanvas::kSoft_ClipEdgeStyle == edgeStyle;
        fClipStack.clipRect(rect, this->getTotalMatrix(), op, isAA);
    }
    void onClipRRect(const SkRRect& rrect, SkClipOp op, ClipEdgeStyle edgeStyle) override {
        const bool isAA = SkCanvas::kSoft_ClipEdgeStyle == edgeStyle;
        fClipStack.clipRRect(rrect, this->getTotalMatrix(), op, isAA);
    }
    void onClipPath(const SkPath& path, SkClipOp op, ClipEdgeStyle edgeStyle) override {
        const bool isAA = SkCanvas::kSoft_ClipEdgeStyle == edgeStyle;
        fClipStack.clipPath(path, this->getTotalMatrix(), op, isAA);
    }
    void onClipRegion(const SkRegion& rgn, SkClipOp op) override {
        fClipStack.clipDevRect(rgn.getBounds(), op);
    }
    ////////////////////////////////////////////////////////////////////////////

    void onFlush() override { fTarget.flush(); }

    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& rec) override {
        fLayerSaves.push_back(this->getSaveCount());
        fTarget.saveLayer(fClipStack, this->getTotalMatrix(), rec);
        return SkCanvas::kNoLayer_SaveLayerStrategy;
    }

    void didRestore() override {
        if (!fLayerSaves.empty() && this->getSaveCount() == fLayerSaves.back()) {
            fLayerSaves.pop_back();
            fTarget.restoreLayer();
        }
    }

    void drawGlyphs(const SkGlyphRunList& glyphRuns) {
        fTarget.drawGlyphs(fClipStack, this->getTotalMatrix(), glyphRuns);
    }

    void drawGlyphsRSXform(const SkGlyphRunList& glyphRuns, const SkRSXform* xform) {
        fTarget.drawGlyphsRSXform(fClipStack, this->getTotalMatrix(), glyphRuns, xform);
    }

    void onDrawPath(const SkPath& path, const SkPaint& paint) override {
        for (const SkPaint& p : SkAutoDrawLooper(this, &paint)) {
            fTarget.drawPath(fClipStack, this->getTotalMatrix(), path, p);
        }
    }
    void onDrawPoints(PointMode mode, size_t count, const SkPoint pts[],
                              const SkPaint& paint) override {
        for (const SkPaint& p : SkAutoDrawLooper(this, &paint)) {
            fTarget.drawPoints(fClipStack, this->getTotalMatrix(), mode, count, pts, p);
        }
    }

    void onDrawVerticesObject(const SkVertices* vertices, const SkVertices::Bone bones[],
                              int boneCount, SkBlendMode mode, const SkPaint& paint) override {
        for (const SkPaint& p : SkAutoDrawLooper(this, &paint)) {
            fTarget.drawVertices(
                fClipStack, this->getTotalMatrix(), vertices, bones, boneCount, mode, p);
        }
    }
    void drawImageImpl(const SkImage* image, const SkRect* src, const SkRect& dst,
                       const SkPaint* paint, SrcRectConstraint constraint) {
        fTarget.drawImageRect(
                fClipStack, this->getTotalMatrix(), image, src, dst, paint, constraint);
    }
    void onDrawAnnotation(const SkRect& rect, const char key[], SkData* value) override {
        fTarget.drawAnnotation(fClipStack, this->getTotalMatrix(), rect, key, value);
    }
    void onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) override {
            fTarget.drawShadowRec(fClipStack, this->getTotalMatrix(), path, rec);
    }
    void onDiscard() override { fTarget.discard(); }
};
}  // namespace

std::unique_ptr<SkCanvas> MakeSCanvas(SkISize s) {
    return std::unique_ptr<SkCanvas>(new ClipStackCanvas<Target>(s));
}

