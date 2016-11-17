/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFlattenable.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "Test.h"

class IntFlattenable : public SkFlattenable {
public:
    IntFlattenable(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
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

    Factory getFactory() const override { return nullptr; }

    uint32_t a() const { return fA; }
    uint32_t b() const { return fB; }
    uint32_t c() const { return fC; }
    uint32_t d() const { return fD; }

    const char* getTypeName() const override { return "IntFlattenable"; }

private:
    uint32_t fA;
    uint32_t fB;
    uint32_t fC;
    uint32_t fD;
};

static sk_sp<SkFlattenable> custom_create_proc(SkReadBuffer& buffer) {
    uint32_t a = buffer.readUInt();
    uint32_t b = buffer.readUInt();
    uint32_t c = buffer.readUInt();
    uint32_t d = buffer.readUInt();
    return sk_sp<SkFlattenable>(new IntFlattenable(a + 1, b + 1, c + 1, d + 1));
}

DEF_TEST(UnflattenWithCustomFactory, r) {
    // Create and flatten the test flattenable
    SkBinaryWriteBuffer writeBuffer;
    sk_sp<SkFlattenable> flattenable1(new IntFlattenable(1, 2, 3, 4));
    writeBuffer.writeFlattenable(flattenable1.get());
    sk_sp<SkFlattenable> flattenable2(new IntFlattenable(2, 3, 4, 5));
    writeBuffer.writeFlattenable(flattenable2.get());
    sk_sp<SkFlattenable> flattenable3(new IntFlattenable(3, 4, 5, 6));
    writeBuffer.writeFlattenable(flattenable3.get());

    // Copy the contents of the write buffer into a read buffer
    sk_sp<SkData> data = SkData::MakeUninitialized(writeBuffer.bytesWritten());
    writeBuffer.writeToMemory(data->writable_data());
    SkReadBuffer readBuffer(data->data(), data->size());

    // Register a custom factory with the read buffer
    readBuffer.setCustomFactory(SkString("IntFlattenable"), &custom_create_proc);

    // Unflatten and verify the flattenables
    sk_sp<IntFlattenable> out1((IntFlattenable*) readBuffer.readFlattenable(
            SkFlattenable::kSkUnused_Type));
    REPORTER_ASSERT(r, out1);
    REPORTER_ASSERT(r, 2 == out1->a());
    REPORTER_ASSERT(r, 3 == out1->b());
    REPORTER_ASSERT(r, 4 == out1->c());
    REPORTER_ASSERT(r, 5 == out1->d());

    sk_sp<IntFlattenable> out2((IntFlattenable*) readBuffer.readFlattenable(
            SkFlattenable::kSkUnused_Type));
    REPORTER_ASSERT(r, out2);
    REPORTER_ASSERT(r, 3 == out2->a());
    REPORTER_ASSERT(r, 4 == out2->b());
    REPORTER_ASSERT(r, 5 == out2->c());
    REPORTER_ASSERT(r, 6 == out2->d());

    sk_sp<IntFlattenable> out3((IntFlattenable*) readBuffer.readFlattenable(
            SkFlattenable::kSkUnused_Type));
    REPORTER_ASSERT(r, out3);
    REPORTER_ASSERT(r, 4 == out3->a());
    REPORTER_ASSERT(r, 5 == out3->b());
    REPORTER_ASSERT(r, 6 == out3->c());
    REPORTER_ASSERT(r, 7 == out3->d());
}
