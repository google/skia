/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkImageFilter.h"
#include "SkLiteDL.h"
#include "SkMutex.h"
#include "SkSpinlock.h"
#include "SkTextBlob.h"

// memcpy_v(dst, src0,bytes0, src1,bytes1, ...) copies an arbitrary number of srcs into dst.
static void memcpy_v(void* dst) {}

template <typename... Rest>
static void memcpy_v(void* dst, const void* src, size_t bytes, Rest&&... rest) {
    memcpy(dst, src, bytes);
    memcpy_v(SkTAddOffset<void>(dst, bytes), std::forward<Rest>(rest)...);
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

namespace {
    struct Op {
        virtual ~Op() {}
        virtual void draw(SkCanvas*) = 0;
        virtual void optimizeFor(GrContext*) {}

        size_t skip;
    };

    struct Save    final : Op { void draw(SkCanvas* c) override { c->   save(); } };
    struct Restore final : Op { void draw(SkCanvas* c) override { c->restore(); } };
    struct SaveLayer final : Op {
        SaveLayer(const SkRect* bounds, const SkPaint* paint,
                  const SkImageFilter* backdrop, uint32_t flags) {
            if (bounds) { this->bounds = *bounds; }
            if (paint)  { this->paint  = *paint;  }
            this->backdrop = sk_ref_sp(backdrop);
            this->flags = flags;
        }
        SkRect                     bounds = {SK_ScalarMin,SK_ScalarMin, SK_ScalarMax,SK_ScalarMax};
        SkPaint                    paint;
        sk_sp<const SkImageFilter> backdrop;
        uint32_t                   flags;
        void draw(SkCanvas* c) override {
            c->saveLayer({ &bounds, &paint, backdrop.get(), flags });
        }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint); }
    };

    struct Concat final : Op {
        Concat(const SkMatrix& matrix) : matrix(matrix) {}
        SkMatrix matrix;
        void draw(SkCanvas* c) override { c->concat(matrix); }
    };
    struct SetMatrix final : Op {
        SetMatrix(const SkMatrix& matrix) : matrix(matrix) {}
        SkMatrix matrix;
        void draw(SkCanvas* c) override { c->setMatrix(matrix); }
    };

    struct ClipPath final : Op {
        ClipPath(const SkPath& path, SkRegion::Op op, bool aa) : path(path), op(op), aa(aa) {}
        SkPath       path;
        SkRegion::Op op;
        bool         aa;
        void draw(SkCanvas* c) override { c->clipPath(path, op, aa); }
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

    struct DrawImage final : Op {
        DrawImage(sk_sp<const SkImage>&& image, SkScalar x, SkScalar y, const SkPaint* paint)
            : image(image), x(x), y(y) {
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
            : image(image), center(center), dst(dst) {
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
            : image(image), dst(dst), constraint(constraint) {
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

    struct DrawText final : Op {
        DrawText(size_t bytes, SkScalar x, SkScalar y, const SkPaint& paint)
            : bytes(bytes), x(x), y(y), paint(paint) {}
        size_t bytes;
        SkScalar x,y;
        SkPaint paint;
        void draw(SkCanvas* c) override { c->drawText(this+1, bytes, x,y, paint); }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint); }
    };
    struct DrawPosText final : Op {
        DrawPosText(size_t bytes, const SkPaint& paint)
            : bytes(bytes), paint(paint) {}
        size_t bytes;
        SkPaint paint;
        void draw(SkCanvas* c) override {
            auto pos = SkTAddOffset<SkPoint>(this+1, bytes);
            c->drawPosText(this+1, bytes, pos, paint);
        }
        void optimizeFor(GrContext* ctx) override { optimize_for(ctx, &paint); }
    };
    struct DrawPosTextH final : Op {
        DrawPosTextH(size_t bytes, SkScalar y, const SkPaint& paint)
            : bytes(bytes), y(y), paint(paint) {}
        size_t bytes;
        SkScalar y;
        SkPaint paint;
        void draw(SkCanvas* c) override {
            auto xs = SkTAddOffset<SkScalar>(this+1, bytes);
            c->drawPosTextH(this+1, bytes, xs, y, paint);
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
    for (uint8_t* ptr = bytes->begin(); ptr < bytes->end(); ) {
        auto op = (Op*)ptr;
        fn(op);
        ptr += op->skip;
    }
}

void SkLiteDL::   save() { push   <Save>(&fBytes, 0); }
void SkLiteDL::restore() { push<Restore>(&fBytes, 0); }
void SkLiteDL::saveLayer(const SkRect* bounds, const SkPaint* paint,
                         const SkImageFilter* backdrop, uint32_t flags) {
    push<SaveLayer>(&fBytes, 0, bounds, paint, backdrop, flags);
}

void SkLiteDL::   concat(const SkMatrix& matrix) { push   <Concat>(&fBytes, 0, matrix); }
void SkLiteDL::setMatrix(const SkMatrix& matrix) { push<SetMatrix>(&fBytes, 0, matrix); }

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

void SkLiteDL::drawText(const void* text, size_t bytes,
                        SkScalar x, SkScalar y, const SkPaint& paint) {
    void* pod = push<DrawText>(&fBytes, bytes, bytes, x, y, paint);
    memcpy_v(pod, text,bytes);
}
void SkLiteDL::drawPosText(const void* text, size_t bytes,
                           const SkPoint pos[], const SkPaint& paint) {
    int n = paint.countText(text, bytes);
    void* pod = push<DrawPosText>(&fBytes, bytes+n*sizeof(SkPoint), bytes, paint);
    memcpy_v(pod, text,bytes, pos,n*sizeof(SkPoint));
}
void SkLiteDL::drawPosTextH(const void* text, size_t bytes,
                           const SkScalar xs[], SkScalar y, const SkPaint& paint) {
    int n = paint.countText(text, bytes);
    void* pod = push<DrawPosTextH>(&fBytes, bytes + n*sizeof(SkScalar), bytes, y, paint);
    memcpy_v(pod, text,bytes, xs,n*sizeof(SkScalar));
}
void SkLiteDL::drawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y, const SkPaint& paint) {
    push<DrawTextBlob>(&fBytes, 0, blob, x,y, paint);
}


void SkLiteDL::onDraw(SkCanvas* canvas) {
    map(&fBytes, [canvas](Op* op) { op->draw(canvas); });
}

void SkLiteDL::optimizeFor(GrContext* ctx) {
    map(&fBytes, [ctx](Op* op) { op->optimizeFor(ctx); });
}

SkRect SkLiteDL::onGetBounds() {
    return fBounds;
}

SkLiteDL:: SkLiteDL() {}
SkLiteDL::~SkLiteDL() {}

static const int kFreeStackByteLimit  = 128*1024;
static const int kFreeStackCountLimit = 8;

static SkSpinlock gFreeStackLock;
static SkLiteDL*  gFreeStack      = nullptr;
static int        gFreeStackCount = 0;

sk_sp<SkLiteDL> SkLiteDL::New(SkRect bounds) {
    sk_sp<SkLiteDL> dl;
    {
        SkAutoMutexAcquire lock(gFreeStackLock);
        if (gFreeStack) {
            dl.reset(gFreeStack);  // Adopts the ref the stack's been holding.
            gFreeStack = gFreeStack->fNext;
            gFreeStackCount--;
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

    if (self->fBytes.reserved() < kFreeStackByteLimit) {
        self->fBytes.rewind();
        SkAutoMutexAcquire lock(gFreeStackLock);
        if (gFreeStackCount < kFreeStackCountLimit) {
            self->fNext = gFreeStack;
            gFreeStack = self;
            gFreeStackCount++;
            return;
        }
    }

    delete this;
}
