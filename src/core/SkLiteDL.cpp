/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkData.h"
#include "SkImageFilter.h"
#include "SkLiteDL.h"
#include "SkPicture.h"
#include "SkMutex.h"
#include "SkRSXform.h"
#include "SkSpinlock.h"
#include "SkTextBlob.h"

// A stand-in for an optional SkRect which was not set, e.g. bounds for a saveLayer().
static const SkRect kUnset = {SK_ScalarInfinity, 0,0,0};
static const SkRect* maybe_unset(const SkRect& r) {
    return r.left() == SK_ScalarInfinity ? nullptr : &r;
}

// copy_v(dst, src,n, src,n, ...) copies an arbitrary number of typed srcs into dst.
static void copy_v(void* dst) {}

template <typename S, typename... Rest>
static void copy_v(void* dst, const S* src, int n, Rest&&... rest) {
    SkASSERTF(((uintptr_t)dst & (alignof(S)-1)) == 0,
              "Expected %p to be aligned for at least %zu bytes.", dst, alignof(S));
    sk_careful_memcpy(dst, src, n*sizeof(S));
    copy_v(SkTAddOffset<void>(dst, n*sizeof(S)), std::forward<Rest>(rest)...);
}

// Helper for getting back at arrays which have been copy_v'd together after an Op.
template <typename D, typename T>
static D* pod(T* op, size_t offset = 0) {
    return SkTAddOffset<D>(op+1, offset);
}

// Convert images and image-based shaders to textures.
static void optimize_for(GrContext* ctx, SkPaint* paint, sk_sp<const SkImage>* image = nullptr) {
    SkMatrix matrix;
    SkShader::TileMode xy[2];
    if (auto shader = paint->getShader())
    if (auto image  = shader->isAImage(&matrix, xy)) {  // TODO: compose shaders, etc.
        paint->setShader(image->makeTextureImage(ctx)->makeShader(xy[0], xy[1], &matrix));
    }

    if (image) {
        *image = (*image)->makeTextureImage(ctx);
    }
}

// Pre-cache lazy non-threadsafe fields on SkPath and/or SkMatrix.
static void make_threadsafe(SkPath* path, SkMatrix* matrix) {
    if (path)   { path->updateBoundsCache(); }
    if (matrix) { (void)matrix->getType(); }
}

namespace {
    struct Op {
        virtual ~Op() {}
        virtual void draw(SkCanvas*) = 0;
        virtual void optimizeFor(GrContext*) {}
        virtual void makeThreadsafe() {}

        size_t skip;
    };

    struct Save    final : Op { void draw(SkCanvas* c) override { c->   save(); } };
    struct Restore final : Op { void draw(SkCanvas* c) override { c->restore(); } };
    struct SaveLayer final : Op {
        SaveLayer(const SkRect* bounds, const SkPaint* paint,
                  const SkImageFilter* backdrop, SkCanvas::SaveLayerFlags flags) {
            if (bounds) { this->bounds = *bounds; }
            if (paint)  { this->paint  = *paint;  }
            this->backdrop = sk_ref_sp(backdrop);
            this->flags = flags;
        }
        SkRect                     bounds = kUnset;
        SkPaint                    paint;
        sk_sp<const SkImageFilter> backdrop;
        SkCanvas::SaveLayerFlags   flags;
        void draw(SkCanvas* c) override {
            c->saveLayer({ maybe_unset(bounds), &paint, backdrop.get(), flags });
        }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint); }
    };

    struct Concat final : Op {
        Concat(const SkMatrix& matrix) : matrix(matrix) {}
        SkMatrix matrix;
        void draw(SkCanvas* c) override { c->concat(matrix); }
        void makeThreadsafe() override { make_threadsafe(nullptr, &matrix); }
    };
    struct SetMatrix final : Op {
        SetMatrix(const SkMatrix& matrix) : matrix(matrix) {}
        SkMatrix matrix;
        void draw(SkCanvas* c) override { c->setMatrix(matrix); }
        void makeThreadsafe() override { make_threadsafe(nullptr, &matrix); }
    };
    struct TranslateZ final : Op {
        TranslateZ(SkScalar dz) : dz(dz) {}
        SkScalar dz;
        void draw(SkCanvas* c) override {
        #ifdef SK_EXPERIMENTAL_SHADOWING
            c->translateZ(dz);
        #endif
        }
    };

    struct ClipPath final : Op {
        ClipPath(const SkPath& path, SkRegion::Op op, bool aa) : path(path), op(op), aa(aa) {}
        SkPath       path;
        SkRegion::Op op;
        bool         aa;
        void draw(SkCanvas* c) override { c->clipPath(path, op, aa); }
        void makeThreadsafe() override { make_threadsafe(&path, nullptr); }
    };
    struct ClipRect final : Op {
        ClipRect(const SkRect& rect, SkRegion::Op op, bool aa) : rect(rect), op(op), aa(aa) {}
        SkRect       rect;
        SkRegion::Op op;
        bool         aa;
        void draw(SkCanvas* c) override { c->clipRect(rect, op, aa); }
    };
    struct ClipRRect final : Op {
        ClipRRect(const SkRRect& rrect, SkRegion::Op op, bool aa) : rrect(rrect), op(op), aa(aa) {}
        SkRRect      rrect;
        SkRegion::Op op;
        bool         aa;
        void draw(SkCanvas* c) override { c->clipRRect(rrect, op, aa); }
    };
    struct ClipRegion final : Op {
        ClipRegion(const SkRegion& region, SkRegion::Op op) : region(region), op(op) {}
        SkRegion     region;
        SkRegion::Op op;
        void draw(SkCanvas* c) override { c->clipRegion(region, op); }
    };

    struct DrawPaint final : Op {
        DrawPaint(const SkPaint& paint) : paint(paint) {}
        SkPaint paint;
        void draw(SkCanvas* c) override { c->drawPaint(paint); }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint); }
    };
    struct DrawPath final : Op {
        DrawPath(const SkPath& path, const SkPaint& paint) : path(path), paint(paint) {}
        SkPath  path;
        SkPaint paint;
        void draw(SkCanvas* c) override { c->drawPath(path, paint); }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint); }
        void makeThreadsafe() override { make_threadsafe(&path, nullptr); }
    };
    struct DrawRect final : Op {
        DrawRect(const SkRect& rect, const SkPaint& paint) : rect(rect), paint(paint) {}
        SkRect  rect;
        SkPaint paint;
        void draw(SkCanvas* c) override { c->drawRect(rect, paint); }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint); }
    };
    struct DrawOval final : Op {
        DrawOval(const SkRect& oval, const SkPaint& paint) : oval(oval), paint(paint) {}
        SkRect  oval;
        SkPaint paint;
        void draw(SkCanvas* c) override { c->drawOval(oval, paint); }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint); }
    };
    struct DrawRRect final : Op {
        DrawRRect(const SkRRect& rrect, const SkPaint& paint) : rrect(rrect), paint(paint) {}
        SkRRect rrect;
        SkPaint paint;
        void draw(SkCanvas* c) override { c->drawRRect(rrect, paint); }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint); }
    };
    struct DrawDRRect final : Op {
        DrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint)
            : outer(outer), inner(inner), paint(paint) {}
        SkRRect outer, inner;
        SkPaint paint;
        void draw(SkCanvas* c) override { c->drawDRRect(outer, inner, paint); }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint); }
    };

    struct DrawAnnotation final : Op {
        DrawAnnotation(const SkRect& rect, SkData* value) : rect(rect), value(sk_ref_sp(value)) {}
        SkRect        rect;
        sk_sp<SkData> value;
        void draw(SkCanvas* c) override { c->drawAnnotation(rect, pod<char>(this), value.get()); }
    };
    struct DrawDrawable final : Op {
        DrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) : drawable(sk_ref_sp(drawable)) {
            if (matrix) { this->matrix = *matrix; }
        }
        sk_sp<SkDrawable>      drawable;
        sk_sp<const SkPicture> snapped;
        SkMatrix               matrix = SkMatrix::I();
        void draw(SkCanvas* c) override {
            snapped ? c->drawPicture(snapped.get(), &matrix, nullptr)
                    : c->drawDrawable(drawable.get(), &matrix);
        }
        void makeThreadsafe() override {
            snapped.reset(drawable->newPictureSnapshot());
            make_threadsafe(nullptr, &matrix);
        }
    };
    struct DrawPicture final : Op {
        DrawPicture(const SkPicture* picture, const SkMatrix* matrix, const SkPaint* paint)
            : picture(sk_ref_sp(picture)) {
            if (matrix) { this->matrix = *matrix; }
            if (paint)  { this->paint  = *paint; has_paint = true; }
        }
        sk_sp<const SkPicture> picture;
        SkMatrix               matrix = SkMatrix::I();
        SkPaint                paint;
        bool                   has_paint = false;  // TODO: why is a default paint not the same?
        void draw(SkCanvas* c) override {
            c->drawPicture(picture.get(), &matrix, has_paint ? &paint : nullptr);
        }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint); }
        void makeThreadsafe() override { make_threadsafe(nullptr, &matrix); }
    };
    struct DrawShadowedPicture final : Op {
        DrawShadowedPicture(const SkPicture* picture, const SkMatrix* matrix, const SkPaint* paint)
            : picture(sk_ref_sp(picture)) {
            if (matrix) { this->matrix = *matrix; }
            if (paint)  { this->paint  = *paint;  }
        }
        sk_sp<const SkPicture> picture;
        SkMatrix               matrix = SkMatrix::I();
        SkPaint                paint;
        void draw(SkCanvas* c) override {
        #ifdef SK_EXPERIMENTAL_SHADOWING
            c->drawShadowedPicture(picture.get(), &matrix, &paint);
        #endif
        }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint); }
        void makeThreadsafe() override { make_threadsafe(nullptr, &matrix); }
    };

    struct DrawImage final : Op {
        DrawImage(sk_sp<const SkImage>&& image, SkScalar x, SkScalar y, const SkPaint* paint)
            : image(std::move(image)), x(x), y(y) {
            if (paint) { this->paint = *paint; }
        }
        sk_sp<const SkImage> image;
        SkScalar x,y;
        SkPaint paint;
        void draw(SkCanvas* c) override { c->drawImage(image.get(), x,y, &paint); }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint, &image); }
    };
    struct DrawImageNine final : Op {
        DrawImageNine(sk_sp<const SkImage>&& image,
                      const SkIRect& center, const SkRect& dst, const SkPaint* paint)
            : image(std::move(image)), center(center), dst(dst) {
            if (paint) { this->paint = *paint; }
        }
        sk_sp<const SkImage> image;
        SkIRect center;
        SkRect  dst;
        SkPaint paint;
        void draw(SkCanvas* c) override { c->drawImageNine(image.get(), center, dst, &paint); }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint, &image); }
    };
    struct DrawImageRect final : Op {
        DrawImageRect(sk_sp<const SkImage>&& image, const SkRect* src, const SkRect& dst,
                      const SkPaint* paint, SkCanvas::SrcRectConstraint constraint)
            : image(std::move(image)), dst(dst), constraint(constraint) {
            this->src = src ? *src : SkRect::MakeIWH(image->width(), image->height());
            if (paint) { this->paint = *paint; }
        }
        sk_sp<const SkImage> image;
        SkRect src, dst;
        SkPaint paint;
        SkCanvas::SrcRectConstraint constraint;
        void draw(SkCanvas* c) override {
            c->drawImageRect(image.get(), src, dst, &paint, constraint);
        }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint, &image); }
    };
    struct DrawImageLattice final : Op {
        DrawImageLattice(sk_sp<const SkImage>&& image, int xs, int ys,
                         const SkRect& dst, const SkPaint* paint)
            : image(std::move(image)), xs(xs), ys(ys), dst(dst) {
            if (paint) { this->paint = *paint; }
        }
        sk_sp<const SkImage> image;
        int                  xs, ys;
        SkRect               dst;
        SkPaint              paint;
        void draw(SkCanvas* c) override {
            auto xdivs = pod<int>(this, 0),
                 ydivs = pod<int>(this, xs*sizeof(int));
            c->drawImageLattice(image.get(), {xdivs, xs, ydivs, ys}, dst, &paint);
        }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint, &image); }
    };

    struct DrawText final : Op {
        DrawText(size_t bytes, SkScalar x, SkScalar y, const SkPaint& paint)
            : bytes(bytes), x(x), y(y), paint(paint) {}
        size_t bytes;
        SkScalar x,y;
        SkPaint paint;
        void draw(SkCanvas* c) override { c->drawText(pod<void>(this), bytes, x,y, paint); }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint); }
    };
    struct DrawPosText final : Op {
        DrawPosText(size_t bytes, const SkPaint& paint, int n)
            : bytes(bytes), paint(paint), n(n) {}
        size_t bytes;
        SkPaint paint;
        int n;
        void draw(SkCanvas* c) override {
            auto points = pod<SkPoint>(this);
            auto text   = pod<void>(this, n*sizeof(SkPoint));
            c->drawPosText(text, bytes, points, paint);
        }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint); }
    };
    struct DrawPosTextH final : Op {
        DrawPosTextH(size_t bytes, SkScalar y, const SkPaint& paint, int n)
            : bytes(bytes), y(y), paint(paint), n(n) {}
        size_t   bytes;
        SkScalar y;
        SkPaint  paint;
        int n;
        void draw(SkCanvas* c) override {
            auto xs   = pod<SkScalar>(this);
            auto text = pod<void>(this, n*sizeof(SkScalar));
            c->drawPosTextH(text, bytes, xs, y, paint);
        }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint); }
    };
    struct DrawTextOnPath final : Op {
        DrawTextOnPath(size_t bytes, const SkPath& path,
                       const SkMatrix* matrix, const SkPaint& paint)
            : bytes(bytes), path(path), paint(paint) {
            if (matrix) { this->matrix = *matrix; }
        }
        size_t   bytes;
        SkPath   path;
        SkMatrix matrix = SkMatrix::I();
        SkPaint  paint;
        void draw(SkCanvas* c) override {
            c->drawTextOnPath(pod<void>(this), bytes, path, &matrix, paint);
        }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint); }
        void makeThreadsafe() override { make_threadsafe(&path, &matrix); }
    };
    struct DrawTextRSXform final : Op {
        DrawTextRSXform(size_t bytes, const SkRect* cull, const SkPaint& paint)
            : bytes(bytes), paint(paint) {
            if (cull) { this->cull = *cull; }
        }
        size_t  bytes;
        SkRect  cull = kUnset;
        SkPaint paint;
        void draw(SkCanvas* c) override {
            c->drawTextRSXform(pod<void>(this), bytes, pod<SkRSXform>(this, bytes),
                               maybe_unset(cull), paint);
        }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint); }
    };
    struct DrawTextBlob final : Op {
        DrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y, const SkPaint& paint)
            : blob(sk_ref_sp(blob)), x(x), y(y), paint(paint) {}
        sk_sp<const SkTextBlob> blob;
        SkScalar x,y;
        SkPaint paint;
        void draw(SkCanvas* c) override { c->drawTextBlob(blob.get(), x,y, paint); }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint); }
    };

    struct DrawPatch final : Op {
        DrawPatch(const SkPoint cubics[12], const SkColor colors[4], const SkPoint texs[4],
                  SkXfermode* xfermode, const SkPaint& paint)
            : xfermode(sk_ref_sp(xfermode)), paint(paint) {
            copy_v(this->cubics, cubics, 12);
            if (colors) { copy_v(this->colors, colors, 4); has_colors = true; }
            if (texs  ) { copy_v(this->texs  , texs  , 4); has_texs   = true; }
        }
        SkPoint           cubics[12];
        SkColor           colors[4];
        SkPoint           texs[4];
        sk_sp<SkXfermode> xfermode;
        SkPaint           paint;
        bool              has_colors = false;
        bool              has_texs   = false;
        void draw(SkCanvas* c) override {
            c->drawPatch(cubics, has_colors ? colors : nullptr, has_texs ? texs : nullptr,
                         xfermode.get(), paint);
        }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint); }
    };
    struct DrawPoints final : Op {
        DrawPoints(SkCanvas::PointMode mode, size_t count, const SkPaint& paint)
            : mode(mode), count(count), paint(paint) {}
        SkCanvas::PointMode mode;
        size_t              count;
        SkPaint             paint;
        void draw(SkCanvas* c) override { c->drawPoints(mode, count, pod<SkPoint>(this), paint); }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint); }
    };
    struct DrawVertices final : Op {
        DrawVertices(SkCanvas::VertexMode mode, int count, SkXfermode* xfermode, int nindices,
                     const SkPaint& paint, bool has_texs, bool has_colors, bool has_indices)
            : mode(mode), count(count), xfermode(sk_ref_sp(xfermode)), nindices(nindices)
            , paint(paint), has_texs(has_texs), has_colors(has_colors), has_indices(has_indices) {}
        SkCanvas::VertexMode mode;
        int                  count;
        sk_sp<SkXfermode>    xfermode;
        int                  nindices;
        SkPaint              paint;
        bool                 has_texs;
        bool                 has_colors;
        bool                 has_indices;
        void draw(SkCanvas* c) override {
            SkPoint* vertices = pod<SkPoint>(this, 0);
            size_t offset = count*sizeof(SkPoint);

            SkPoint* texs = nullptr;
            if (has_texs) {
                texs = pod<SkPoint>(this, offset);
                offset += count*sizeof(SkPoint);
            }

            SkColor* colors = nullptr;
            if (has_colors) {
                colors = pod<SkColor>(this, offset);
                offset += count*sizeof(SkColor);
            }

            uint16_t* indices = nullptr;
            if (has_indices) {
                indices = pod<uint16_t>(this, offset);
            }
            c->drawVertices(mode, count, vertices, texs, colors, xfermode.get(),
                            indices, nindices, paint);
        }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint); }
    };
    struct DrawAtlas final : Op {
        DrawAtlas(const SkImage* atlas, int count, SkXfermode::Mode xfermode,
                  const SkRect* cull, const SkPaint* paint, bool has_colors)
            : atlas(sk_ref_sp(atlas)), count(count), xfermode(xfermode), has_colors(has_colors) {
            if (cull)  { this->cull  = *cull; }
            if (paint) { this->paint = *paint; }
        }
        sk_sp<const SkImage> atlas;
        int                  count;
        SkXfermode::Mode     xfermode;
        SkRect               cull = kUnset;
        SkPaint              paint;
        bool                 has_colors;
        void draw(SkCanvas* c) override {
            auto xforms = pod<SkRSXform>(this, 0);
            auto   texs = pod<SkRect>(this, count*sizeof(SkRSXform));
            auto colors = has_colors
                ? pod<SkColor>(this, count*(sizeof(SkRSXform) + sizeof(SkRect)))
                : nullptr;
            c->drawAtlas(atlas.get(), xforms, texs, colors, count, xfermode,
                         maybe_unset(cull), &paint);
        }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint, &atlas); }
    };
}

template <typename T, typename... Args>
static void* push(SkTDArray<uint8_t>* bytes, size_t pod, Args&&... args) {
    size_t skip = SkAlignPtr(sizeof(T) + pod);
    auto op = (T*)bytes->append(skip);
    new (op) T{ std::forward<Args>(args)... };
    op->skip = skip;
    return op+1;
}

template <typename Fn>
static void map(SkTDArray<uint8_t>* bytes, Fn&& fn) {
    auto end = bytes->end();
    for (uint8_t* ptr = bytes->begin(); ptr < end; ) {
        auto op = (Op*)ptr;
        fn(op);
        ptr += op->skip;
    }
}

void SkLiteDL::   save() { push   <Save>(&fBytes, 0); }
void SkLiteDL::restore() { push<Restore>(&fBytes, 0); }
void SkLiteDL::saveLayer(const SkRect* bounds, const SkPaint* paint,
                         const SkImageFilter* backdrop, SkCanvas::SaveLayerFlags flags) {
    push<SaveLayer>(&fBytes, 0, bounds, paint, backdrop, flags);
}

void SkLiteDL::   concat(const SkMatrix& matrix) { push   <Concat>(&fBytes, 0, matrix); }
void SkLiteDL::setMatrix(const SkMatrix& matrix) { push<SetMatrix>(&fBytes, 0, matrix); }
void SkLiteDL::translateZ(SkScalar dz) { push<TranslateZ>(&fBytes, 0, dz); }

void SkLiteDL::clipPath(const SkPath& path, SkRegion::Op op, bool aa) {
    push<ClipPath>(&fBytes, 0, path, op, aa);
}
void SkLiteDL::clipRect(const SkRect& rect, SkRegion::Op op, bool aa) {
    push<ClipRect>(&fBytes, 0, rect, op, aa);
}
void SkLiteDL::clipRRect(const SkRRect& rrect, SkRegion::Op op, bool aa) {
    push<ClipRRect>(&fBytes, 0, rrect, op, aa);
}
void SkLiteDL::clipRegion(const SkRegion& region, SkRegion::Op op) {
    push<ClipRegion>(&fBytes, 0, region, op);
}

void SkLiteDL::drawPaint(const SkPaint& paint) {
    push<DrawPaint>(&fBytes, 0, paint);
}
void SkLiteDL::drawPath(const SkPath& path, const SkPaint& paint) {
    push<DrawPath>(&fBytes, 0, path, paint);
}
void SkLiteDL::drawRect(const SkRect& rect, const SkPaint& paint) {
    push<DrawRect>(&fBytes, 0, rect, paint);
}
void SkLiteDL::drawOval(const SkRect& oval, const SkPaint& paint) {
    push<DrawOval>(&fBytes, 0, oval, paint);
}
void SkLiteDL::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    push<DrawRRect>(&fBytes, 0, rrect, paint);
}
void SkLiteDL::drawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    push<DrawDRRect>(&fBytes, 0, outer, inner, paint);
}

void SkLiteDL::drawAnnotation(const SkRect& rect, const char* key, SkData* value) {
    size_t bytes = strlen(key)+1;
    void* pod = push<DrawAnnotation>(&fBytes, bytes, rect, value);
    copy_v(pod, key,bytes);
}
void SkLiteDL::drawDrawable(SkDrawable* drawable, const SkMatrix* matrix) {
    push<DrawDrawable>(&fBytes, 0, drawable, matrix);
}
void SkLiteDL::drawPicture(const SkPicture* picture,
                           const SkMatrix* matrix, const SkPaint* paint) {
    push<DrawPicture>(&fBytes, 0, picture, matrix, paint);
}
void SkLiteDL::drawShadowedPicture(const SkPicture* picture,
                                   const SkMatrix* matrix, const SkPaint* paint) {
    push<DrawShadowedPicture>(&fBytes, 0, picture, matrix, paint);
}

void SkLiteDL::drawBitmap(const SkBitmap& bm, SkScalar x, SkScalar y, const SkPaint* paint) {
    push<DrawImage>(&fBytes, 0, SkImage::MakeFromBitmap(bm), x,y, paint);
}
void SkLiteDL::drawBitmapNine(const SkBitmap& bm, const SkIRect& center,
                              const SkRect& dst, const SkPaint* paint) {
    push<DrawImageNine>(&fBytes, 0, SkImage::MakeFromBitmap(bm), center, dst, paint);
}
void SkLiteDL::drawBitmapRect(const SkBitmap& bm, const SkRect* src, const SkRect& dst,
                              const SkPaint* paint, SkCanvas::SrcRectConstraint constraint) {
    push<DrawImageRect>(&fBytes, 0, SkImage::MakeFromBitmap(bm), src, dst, paint, constraint);
}

void SkLiteDL::drawImage(const SkImage* image, SkScalar x, SkScalar y, const SkPaint* paint) {
    push<DrawImage>(&fBytes, 0, sk_ref_sp(image), x,y, paint);
}
void SkLiteDL::drawImageNine(const SkImage* image, const SkIRect& center,
                             const SkRect& dst, const SkPaint* paint) {
    push<DrawImageNine>(&fBytes, 0, sk_ref_sp(image), center, dst, paint);
}
void SkLiteDL::drawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                             const SkPaint* paint, SkCanvas::SrcRectConstraint constraint) {
    push<DrawImageRect>(&fBytes, 0, sk_ref_sp(image), src, dst, paint, constraint);
}
void SkLiteDL::drawImageLattice(const SkImage* image, const SkCanvas::Lattice& lattice,
                                const SkRect& dst, const SkPaint* paint) {
    int xs = lattice.fXCount, ys = lattice.fYCount;
    size_t bytes = (xs + ys) * sizeof(int);
    void* pod = push<DrawImageLattice>(&fBytes, bytes, sk_ref_sp(image), xs, ys, dst, paint);
    copy_v(pod, lattice.fXDivs, xs,
                lattice.fYDivs, ys);
}

void SkLiteDL::drawText(const void* text, size_t bytes,
                        SkScalar x, SkScalar y, const SkPaint& paint) {
    void* pod = push<DrawText>(&fBytes, bytes, bytes, x, y, paint);
    copy_v(pod, (const char*)text,bytes);
}
void SkLiteDL::drawPosText(const void* text, size_t bytes,
                           const SkPoint pos[], const SkPaint& paint) {
    int n = paint.countText(text, bytes);
    void* pod = push<DrawPosText>(&fBytes, n*sizeof(SkPoint)+bytes, bytes, paint, n);
    copy_v(pod, pos,n, (const char*)text,bytes);
}
void SkLiteDL::drawPosTextH(const void* text, size_t bytes,
                           const SkScalar xs[], SkScalar y, const SkPaint& paint) {
    int n = paint.countText(text, bytes);
    void* pod = push<DrawPosTextH>(&fBytes, n*sizeof(SkScalar)+bytes, bytes, y, paint, n);
    copy_v(pod, xs,n, (const char*)text,bytes);
}
void SkLiteDL::drawTextOnPath(const void* text, size_t bytes,
                              const SkPath& path, const SkMatrix* matrix, const SkPaint& paint) {
    void* pod = push<DrawTextOnPath>(&fBytes, bytes, bytes, path, matrix, paint);
    copy_v(pod, (const char*)text,bytes);
}
void SkLiteDL::drawTextRSXform(const void* text, size_t bytes,
                               const SkRSXform xforms[], const SkRect* cull, const SkPaint& paint) {
    int n = paint.countText(text, bytes);
    void* pod = push<DrawTextRSXform>(&fBytes, bytes+n*sizeof(SkRSXform), bytes, cull, paint);
    copy_v(pod, (const char*)text,bytes, xforms,n);
}
void SkLiteDL::drawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y, const SkPaint& paint) {
    push<DrawTextBlob>(&fBytes, 0, blob, x,y, paint);
}

void SkLiteDL::drawPatch(const SkPoint points[12], const SkColor colors[4], const SkPoint texs[4],
                         SkXfermode* xfermode, const SkPaint& paint) {
    push<DrawPatch>(&fBytes, 0, points, colors, texs, xfermode, paint);
}
void SkLiteDL::drawPoints(SkCanvas::PointMode mode, size_t count, const SkPoint points[],
                          const SkPaint& paint) {
    void* pod = push<DrawPoints>(&fBytes, count*sizeof(SkPoint), mode, count, paint);
    copy_v(pod, points,count);
}
void SkLiteDL::drawVertices(SkCanvas::VertexMode mode, int count, const SkPoint vertices[],
                            const SkPoint texs[], const SkColor colors[], SkXfermode* xfermode,
                            const uint16_t indices[], int nindices, const SkPaint& paint) {
    size_t bytes = count * sizeof(SkPoint);
    if (texs  )  { bytes += count    * sizeof(SkPoint); }
    if (colors)  { bytes += count    * sizeof(SkColor); }
    if (indices) { bytes += nindices * sizeof(uint16_t); }
    void* pod = push<DrawVertices>(&fBytes, bytes, mode, count, xfermode, nindices, paint,
                                   texs != nullptr, colors != nullptr, indices != nullptr);
    copy_v(pod, vertices, count,
                    texs, texs    ? count    : 0,
                  colors, colors  ? count    : 0,
                 indices, indices ? nindices : 0);
}
void SkLiteDL::drawAtlas(const SkImage* atlas, const SkRSXform xforms[], const SkRect texs[],
                         const SkColor colors[], int count, SkXfermode::Mode xfermode,
                         const SkRect* cull, const SkPaint* paint) {
    size_t bytes = count*(sizeof(SkRSXform) + sizeof(SkRect));
    if (colors) {
        bytes += count*sizeof(SkColor);
    }
    void* pod = push<DrawAtlas>(&fBytes, bytes,
                                atlas, count, xfermode, cull, paint, colors != nullptr);
    copy_v(pod, xforms, count,
                  texs, count,
                colors, colors ? count : 0);
}


void SkLiteDL::onDraw(SkCanvas* canvas) {
    map(&fBytes, [canvas](Op* op) { op->draw(canvas); });
}

void SkLiteDL::optimizeFor(GrContext* ctx) {
    map(&fBytes, [ctx](Op* op) { op->optimizeFor(ctx); });
}

void SkLiteDL::makeThreadsafe() {
    map(&fBytes, [](Op* op) { op->makeThreadsafe(); });
}

SkRect SkLiteDL::onGetBounds() {
    return fBounds;
}

#if !defined(SK_LITEDL_USES)
    #define  SK_LITEDL_USES 16
#endif

SkLiteDL:: SkLiteDL() : fUsesRemaining(SK_LITEDL_USES) {}
SkLiteDL::~SkLiteDL() {}

// If you're tempted to make this lock free, please don't forget about ABA.
static SkSpinlock gFreeStackLock;
static SkLiteDL*  gFreeStack = nullptr;

sk_sp<SkLiteDL> SkLiteDL::New(SkRect bounds) {
    sk_sp<SkLiteDL> dl;
    {
        SkAutoMutexAcquire lock(gFreeStackLock);
        if (gFreeStack) {
            dl.reset(gFreeStack);  // Adopts the ref the stack's been holding.
            gFreeStack = gFreeStack->fNext;
        }
    }

    if (!dl) {
        dl.reset(new SkLiteDL);
    }

    dl->fBounds = bounds;
    return dl;
}

void SkLiteDL::internal_dispose() const {
    // Whether we delete this or leave it on the free stack,
    // we want its refcnt at 1.
    this->internal_dispose_restore_refcnt_to_1();

    auto self = const_cast<SkLiteDL*>(this);
    map(&self->fBytes, [](Op* op) { op->~Op(); });

    if (--self->fUsesRemaining > 0) {
        self->fBytes.rewind();

        SkAutoMutexAcquire lock(gFreeStackLock);
        self->fNext = gFreeStack;
        gFreeStack = self;
        return;
    }

    delete this;
}

void SkLiteDL::PurgeFreelist() {
    SkAutoMutexAcquire lock(gFreeStackLock);
    while (gFreeStack) {
        SkLiteDL* top = gFreeStack;
        gFreeStack = gFreeStack->fNext;
        delete top;   // Calling unref() here would just put it back on the list!
    }
}
