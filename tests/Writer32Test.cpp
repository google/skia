/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "SkReader32.h"
#include "SkWriter32.h"
#include "Test.h"

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
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Writer32", Writer32Class, Tests)

