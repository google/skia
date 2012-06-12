
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "SkReader32.h"
#include "SkWriter32.h"
#include "Test.h"

static void test_ptr(skiatest::Reporter* reporter) {
    SkSWriter32<32> writer(32);
    
    void* p0 = reporter;
    void* p1 = &writer;

    // try writing ptrs where at least one of them may be at a non-multiple of
    // 8 boundary, to confirm this works on 64bit machines.

    writer.writePtr(p0);
    writer.write8(0x33);
    writer.writePtr(p1);
    writer.write8(0x66);

    size_t size = writer.size();
    REPORTER_ASSERT(reporter, 2 * sizeof(void*) + 2 * sizeof(int32_t));

    char buffer[32];
    SkASSERT(sizeof(buffer) >= size);
    writer.flatten(buffer);

    SkReader32 reader(buffer, size);
    REPORTER_ASSERT(reporter, reader.readPtr() == p0);
    REPORTER_ASSERT(reporter, reader.readInt() == 0x33);
    REPORTER_ASSERT(reporter, reader.readPtr() == p1);
    REPORTER_ASSERT(reporter, reader.readInt() == 0x66);
}

static void test1(skiatest::Reporter* reporter, SkWriter32* writer) {
    const uint32_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    for (size_t i = 0; i < SK_ARRAY_COUNT(data); ++i) {
        REPORTER_ASSERT(reporter, i*4 == writer->size());
        writer->write32(data[i]);
        uint32_t* addr = writer->peek32(i * 4);
        REPORTER_ASSERT(reporter, data[i] == *addr);
    }

    char buffer[sizeof(data)];
    REPORTER_ASSERT(reporter, sizeof(buffer) == writer->size());
    writer->flatten(buffer);
    REPORTER_ASSERT(reporter, !memcmp(data, buffer, sizeof(buffer)));
}

static void test2(skiatest::Reporter* reporter, SkWriter32* writer) {
    static const char gStr[] = "abcdefghimjklmnopqrstuvwxyz";
    size_t i;

    size_t len = 0;
    for (i = 0; i <= 26; ++i) {
        len += SkWriter32::WriteStringSize(gStr, i);
        writer->writeString(gStr, i);
    }
    REPORTER_ASSERT(reporter, writer->size() == len);

    SkAutoMalloc storage(len);
    writer->flatten(storage.get());

    SkReader32 reader;
    reader.setMemory(storage.get(), len);
    for (i = 0; i <= 26; ++i) {
        REPORTER_ASSERT(reporter, !reader.eof());
        const char* str = reader.readString(&len);
        REPORTER_ASSERT(reporter, i == len);
        REPORTER_ASSERT(reporter, strlen(str) == len);
        REPORTER_ASSERT(reporter, !memcmp(str, gStr, len));
    }
    REPORTER_ASSERT(reporter, reader.eof());
}

static void Tests(skiatest::Reporter* reporter) {
    // dynamic allocator
    {
        SkWriter32 writer(256 * 4);
        REPORTER_ASSERT(reporter, NULL == writer.getSingleBlock());
        test1(reporter, &writer);
        
        writer.reset();
        test2(reporter, &writer);
    }
    
    // single-block
    {
        SkWriter32 writer(0);
        uint32_t storage[256];
        REPORTER_ASSERT(reporter, NULL == writer.getSingleBlock());
        writer.reset(storage, sizeof(storage));
        REPORTER_ASSERT(reporter, (void*)storage == writer.getSingleBlock());
        test1(reporter, &writer);

        writer.reset(storage, sizeof(storage));
        test2(reporter, &writer);
    }
    
    // small storage
    {
        SkSWriter32<8 * sizeof(intptr_t)> writer(100);
        test1(reporter, &writer);
        writer.reset(); // should just rewind our storage
        test2(reporter, &writer);
    }
    
    // large storage
    {
        SkSWriter32<1024 * sizeof(intptr_t)> writer(100);
        test1(reporter, &writer);
        writer.reset(); // should just rewind our storage
        test2(reporter, &writer);
    }
    
    test_ptr(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Writer32", Writer32Class, Tests)

