/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkLiteDL.h"
#include "SkMutex.h"
#include "SkSpinlock.h"

namespace {
    struct Op {
        virtual ~Op() {}
        virtual void draw(SkCanvas*) = 0;

        size_t skip;
    };

    struct Save    final : Op { void draw(SkCanvas* c) override { c->   save(); } };
    struct Restore final : Op { void draw(SkCanvas* c) override { c->restore(); } };

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

    struct ClipRect final : Op {
        ClipRect(const SkRect& rect, SkRegion::Op op, bool aa) : rect(rect), op(op), aa(aa) {}
        SkRect       rect;
        SkRegion::Op op;
        bool         aa;
        void draw(SkCanvas* c) override { c->clipRect(rect, op, aa); }
    };

    struct DrawRect final : Op {
        DrawRect(const SkRect& rect, const SkPaint& paint) : rect(rect), paint(paint) {}
        SkRect  rect;
        SkPaint paint;
        void draw(SkCanvas* c) override { c->drawRect(rect, paint); }
    };

    struct DrawPath final : Op {
        DrawPath(const SkPath& path, const SkPaint& paint) : path(path), paint(paint) {}
        SkPath  path;
        SkPaint paint;
        void draw(SkCanvas* c) override { c->drawPath(path, paint); }
    };

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
}

void SkLiteDL::   save() { push   <Save>(&fBytes, 0); }
void SkLiteDL::restore() { push<Restore>(&fBytes, 0); }

void SkLiteDL::   concat(const SkMatrix& matrix) { push   <Concat>(&fBytes, 0, matrix); }
void SkLiteDL::setMatrix(const SkMatrix& matrix) { push<SetMatrix>(&fBytes, 0, matrix); }

void SkLiteDL::clipRect(const SkRect& rect, SkRegion::Op op, bool aa) {
    push<ClipRect>(&fBytes, 0, rect, op, aa);
}

void SkLiteDL::drawRect(const SkRect& rect, const SkPaint& paint) {
    push<DrawRect>(&fBytes, 0, rect, paint);
}
void SkLiteDL::drawPath(const SkPath& path, const SkPaint& paint) {
    push<DrawPath>(&fBytes, 0, path, paint);
}

void SkLiteDL::onDraw(SkCanvas* canvas) {
    map(&fBytes, [canvas](Op* op) { op->draw(canvas); });
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
