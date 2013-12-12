/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkReader32.h"

static void assert_eof(skiatest::Reporter* reporter, const SkReader32& reader) {
    REPORTER_ASSERT(reporter, reader.eof());
    REPORTER_ASSERT(reporter, reader.size() == reader.offset());
    REPORTER_ASSERT(reporter, (const char*)reader.peek() ==
                    (const char*)reader.base() + reader.size());
}

static void assert_start(skiatest::Reporter* reporter, const SkReader32& reader) {
    REPORTER_ASSERT(reporter, 0 == reader.offset());
    REPORTER_ASSERT(reporter, reader.size() == reader.available());
    REPORTER_ASSERT(reporter, reader.isAvailable(reader.size()));
    REPORTER_ASSERT(reporter, !reader.isAvailable(reader.size() + 1));
    REPORTER_ASSERT(reporter, reader.peek() == reader.base());
}

static void assert_empty(skiatest::Reporter* reporter, const SkReader32& reader) {
    REPORTER_ASSERT(reporter, 0 == reader.size());
    REPORTER_ASSERT(reporter, 0 == reader.offset());
    REPORTER_ASSERT(reporter, 0 == reader.available());
    REPORTER_ASSERT(reporter, !reader.isAvailable(1));
    assert_eof(reporter, reader);
    assert_start(reporter, reader);
}

DEF_TEST(Reader32, reporter) {
    SkReader32 reader;
    assert_empty(reporter, reader);
    REPORTER_ASSERT(reporter, NULL == reader.base());
    REPORTER_ASSERT(reporter, NULL == reader.peek());

    size_t i;

    const int32_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    const SkScalar data2[] = { 0, SK_Scalar1, -SK_Scalar1, SK_Scalar1/2 };
    const size_t bufsize = sizeof(data) > sizeof(data2) ?
      sizeof(data) : sizeof(data2);
    char buffer[bufsize];

    reader.setMemory(data, sizeof(data));
    for (i = 0; i < SK_ARRAY_COUNT(data); ++i) {
        REPORTER_ASSERT(reporter, sizeof(data) == reader.size());
        REPORTER_ASSERT(reporter, i*4 == reader.offset());
        REPORTER_ASSERT(reporter, (const void*)data == reader.base());
        REPORTER_ASSERT(reporter, (const void*)&data[i] == reader.peek());
        REPORTER_ASSERT(reporter, data[i] == reader.readInt());
    }
    assert_eof(reporter, reader);
    reader.rewind();
    assert_start(reporter, reader);
    reader.read(buffer, sizeof(data));
    REPORTER_ASSERT(reporter, !memcmp(data, buffer, sizeof(data)));

    reader.setMemory(data2, sizeof(data2));
    for (i = 0; i < SK_ARRAY_COUNT(data2); ++i) {
        REPORTER_ASSERT(reporter, sizeof(data2) == reader.size());
        REPORTER_ASSERT(reporter, i*4 == reader.offset());
        REPORTER_ASSERT(reporter, (const void*)data2 == reader.base());
        REPORTER_ASSERT(reporter, (const void*)&data2[i] == reader.peek());
        REPORTER_ASSERT(reporter, data2[i] == reader.readScalar());
    }
    assert_eof(reporter, reader);
    reader.rewind();
    assert_start(reporter, reader);
    reader.read(buffer, sizeof(data2));
    REPORTER_ASSERT(reporter, !memcmp(data2, buffer, sizeof(data2)));

    reader.setMemory(NULL, 0);
    assert_empty(reporter, reader);
    REPORTER_ASSERT(reporter, NULL == reader.base());
    REPORTER_ASSERT(reporter, NULL == reader.peek());
}
