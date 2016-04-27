/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkDrawable.h"
#include "SkOnce.h"
#include "SkPictureRecorder.h"
#include "SkReadBuffer.h"
#include "SkRect.h"
#include "SkStream.h"
#include "SkWriteBuffer.h"
#include "Test.h"

class IntDrawable : public SkDrawable {
public:
    IntDrawable(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
        : fA(a)
        , fB(b)
        , fC(c)
        , fD(d)
    {}

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeUInt(fA);
        buffer.writeUInt(fB);
        buffer.writeUInt(fC);
        buffer.writeUInt(fD);
    }

    static sk_sp<SkFlattenable> CreateProc(SkReadBuffer& buffer) {
        uint32_t a = buffer.readUInt();
        uint32_t b = buffer.readUInt();
        uint32_t c = buffer.readUInt();
        uint32_t d = buffer.readUInt();
        return sk_sp<IntDrawable>(new IntDrawable(a, b, c, d));
    }

    Factory getFactory() const override { return CreateProc; }

    uint32_t a() const { return fA; }
    uint32_t b() const { return fB; }
    uint32_t c() const { return fC; }
    uint32_t d() const { return fD; }

    const char* getTypeName() const override { return "IntDrawable"; }

protected:
    SkRect onGetBounds() override { return SkRect::MakeEmpty(); }
    void onDraw(SkCanvas*) override {}

private:
    uint32_t fA;
    uint32_t fB;
    uint32_t fC;
    uint32_t fD;
};

class PaintDrawable : public SkDrawable {
public:
    PaintDrawable(const SkPaint& paint)
        : fPaint(paint)
    {}

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writePaint(fPaint);
    }

    static sk_sp<SkFlattenable> CreateProc(SkReadBuffer& buffer) {
        SkPaint paint;
        buffer.readPaint(&paint);
        return sk_sp<PaintDrawable>(new PaintDrawable(paint));
    }

    Factory getFactory() const override { return CreateProc; }

    const SkPaint& paint() const { return fPaint; }

    const char* getTypeName() const override { return "PaintDrawable"; }

protected:
    SkRect onGetBounds() override { return SkRect::MakeEmpty(); }
    void onDraw(SkCanvas*) override {}

private:
    SkPaint fPaint;
};

class CompoundDrawable : public SkDrawable {
public:
    CompoundDrawable(uint32_t a, uint32_t b, uint32_t c, uint32_t d, const SkPaint& paint)
        : fIntDrawable(new IntDrawable(a, b, c, d))
        , fPaintDrawable(new PaintDrawable(paint))
    {}

    CompoundDrawable(IntDrawable* intDrawable, PaintDrawable* paintDrawable)
        : fIntDrawable(SkRef(intDrawable))
        , fPaintDrawable(SkRef(paintDrawable))
    {}

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeFlattenable(fIntDrawable);
        buffer.writeFlattenable(fPaintDrawable);
    }

    static sk_sp<SkFlattenable> CreateProc(SkReadBuffer& buffer) {
        SkAutoTUnref<SkFlattenable> intDrawable(
                buffer.readFlattenable(SkFlattenable::kSkDrawable_Type));
        SkASSERT(intDrawable);
        SkASSERT(!strcmp("IntDrawable", intDrawable->getTypeName()));

        SkAutoTUnref<SkFlattenable> paintDrawable(
                buffer.readFlattenable(SkFlattenable::kSkDrawable_Type));
        SkASSERT(paintDrawable);
        SkASSERT(!strcmp("PaintDrawable", paintDrawable->getTypeName()));

        return sk_sp<CompoundDrawable>(new CompoundDrawable((IntDrawable*) intDrawable.get(),
                                                            (PaintDrawable*) paintDrawable.get()));
    }

    Factory getFactory() const override { return CreateProc; }

    IntDrawable* intDrawable() const { return fIntDrawable; }
    PaintDrawable* paintDrawable() const { return fPaintDrawable; }

    const char* getTypeName() const override { return "CompoundDrawable"; }

protected:
    SkRect onGetBounds() override { return SkRect::MakeEmpty(); }
    void onDraw(SkCanvas*) override {}

private:
    SkAutoTUnref<IntDrawable>   fIntDrawable;
    SkAutoTUnref<PaintDrawable> fPaintDrawable;
};

class RootDrawable : public SkDrawable {
public:
    RootDrawable(uint32_t a, uint32_t b, uint32_t c, uint32_t d, const SkPaint& paint,
                   uint32_t e, uint32_t f, uint32_t g, uint32_t h, SkDrawable* drawable)
        : fCompoundDrawable(new CompoundDrawable(a, b, c, d, paint))
        , fIntDrawable(new IntDrawable(e, f, g, h))
        , fDrawable(SkRef(drawable))
    {}

    RootDrawable(CompoundDrawable* compoundDrawable, IntDrawable* intDrawable,
            SkDrawable* drawable)
        : fCompoundDrawable(SkRef(compoundDrawable))
        , fIntDrawable(SkRef(intDrawable))
        , fDrawable(SkRef(drawable))
    {}

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeFlattenable(fCompoundDrawable);
        buffer.writeFlattenable(fIntDrawable);
        buffer.writeFlattenable(fDrawable);
    }

    static sk_sp<SkFlattenable> CreateProc(SkReadBuffer& buffer) {
        SkAutoTUnref<SkFlattenable> compoundDrawable(
                buffer.readFlattenable(SkFlattenable::kSkDrawable_Type));
        SkASSERT(compoundDrawable);
        SkASSERT(!strcmp("CompoundDrawable", compoundDrawable->getTypeName()));

        SkAutoTUnref<SkFlattenable> intDrawable(
                buffer.readFlattenable(SkFlattenable::kSkDrawable_Type));
        SkASSERT(intDrawable);
        SkASSERT(!strcmp("IntDrawable", intDrawable->getTypeName()));

        SkAutoTUnref<SkFlattenable> drawable(
                buffer.readFlattenable(SkFlattenable::kSkDrawable_Type));
        SkASSERT(drawable);

        return sk_sp<RootDrawable>(new RootDrawable((CompoundDrawable*) compoundDrawable.get(),
                                                    (IntDrawable*) intDrawable.get(),
                                                    (SkDrawable*) drawable.get()));
    }

    Factory getFactory() const override { return CreateProc; }

    CompoundDrawable* compoundDrawable() const { return fCompoundDrawable; }
    IntDrawable* intDrawable() const { return fIntDrawable; }
    SkDrawable* drawable() const { return fDrawable; }

    const char* getTypeName() const override { return "RootDrawable"; }

protected:
    SkRect onGetBounds() override { return SkRect::MakeEmpty(); }
    void onDraw(SkCanvas*) override {}

private:
    SkAutoTUnref<CompoundDrawable> fCompoundDrawable;
    SkAutoTUnref<IntDrawable>      fIntDrawable;
    SkAutoTUnref<SkDrawable>       fDrawable;
};

static void register_test_drawables(SkReadBuffer& buffer) {
    buffer.setCustomFactory(SkString("IntDrawable"), IntDrawable::CreateProc);
    buffer.setCustomFactory(SkString("PaintDrawable"), PaintDrawable::CreateProc);
    buffer.setCustomFactory(SkString("CompoundDrawable"), CompoundDrawable::CreateProc);
    buffer.setCustomFactory(SkString("RootDrawable"), RootDrawable::CreateProc);
}

DEF_TEST(FlattenDrawable, r) {
    // Create and serialize the test drawable
    SkAutoTUnref<SkDrawable> drawable(new IntDrawable(1, 2, 3, 4));
    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    SkAutoTUnref<RootDrawable> root(new RootDrawable(5, 6, 7, 8, paint, 9, 10, 11, 12, drawable));
    SkWriteBuffer writeBuffer;
    writeBuffer.writeFlattenable(root);

    // Copy the contents of the write buffer into a read buffer
    sk_sp<SkData> data = SkData::MakeUninitialized(writeBuffer.bytesWritten());
    writeBuffer.writeToMemory(data->writable_data());
    SkReadBuffer readBuffer(data->data(), data->size());
    register_test_drawables(readBuffer);

    // Deserialize and verify the drawable
    SkAutoTUnref<SkDrawable> out((SkDrawable*)
            readBuffer.readFlattenable(SkFlattenable::kSkDrawable_Type));
    REPORTER_ASSERT(r, out);
    REPORTER_ASSERT(r, !strcmp("RootDrawable", out->getTypeName()));

    RootDrawable* rootOut = (RootDrawable*) out.get();
    REPORTER_ASSERT(r, 5 == rootOut->compoundDrawable()->intDrawable()->a());
    REPORTER_ASSERT(r, 6 == rootOut->compoundDrawable()->intDrawable()->b());
    REPORTER_ASSERT(r, 7 == rootOut->compoundDrawable()->intDrawable()->c());
    REPORTER_ASSERT(r, 8 == rootOut->compoundDrawable()->intDrawable()->d());
    REPORTER_ASSERT(r, SK_ColorBLUE ==
            rootOut->compoundDrawable()->paintDrawable()->paint().getColor());
    REPORTER_ASSERT(r, 9 == rootOut->intDrawable()->a());
    REPORTER_ASSERT(r, 10 == rootOut->intDrawable()->b());
    REPORTER_ASSERT(r, 11 == rootOut->intDrawable()->c());
    REPORTER_ASSERT(r, 12 == rootOut->intDrawable()->d());

    // Note that we can still recognize the generic drawable as an IntDrawable
    SkDrawable* generic = rootOut->drawable();
    REPORTER_ASSERT(r, !strcmp("IntDrawable", generic->getTypeName()));
    IntDrawable* integer = (IntDrawable*) generic;
    REPORTER_ASSERT(r, 1 == integer->a());
    REPORTER_ASSERT(r, 2 == integer->b());
    REPORTER_ASSERT(r, 3 == integer->c());
    REPORTER_ASSERT(r, 4 == integer->d());
}
