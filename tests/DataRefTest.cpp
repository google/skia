/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkData.h"
#include "SkDataTable.h"
#include "SkOrderedReadBuffer.h"
#include "SkOrderedWriteBuffer.h"
#include "SkOSFile.h"
#include "SkStream.h"

static void test_is_equal(skiatest::Reporter* reporter,
                          const SkDataTable* a, const SkDataTable* b) {
    REPORTER_ASSERT(reporter, a->count() == b->count());
    for (int i = 0; i < a->count(); ++i) {
        size_t sizea, sizeb;
        const void* mema = a->at(i, &sizea);
        const void* memb = b->at(i, &sizeb);
        REPORTER_ASSERT(reporter, sizea == sizeb);
        REPORTER_ASSERT(reporter, !memcmp(mema, memb, sizea));
    }
}

static void test_datatable_is_empty(skiatest::Reporter* reporter,
                                    SkDataTable* table) {
    REPORTER_ASSERT(reporter, table->isEmpty());
    REPORTER_ASSERT(reporter, 0 == table->count());
}

static void test_emptytable(skiatest::Reporter* reporter) {
    SkAutoTUnref<SkDataTable> table0(SkDataTable::NewEmpty());
    SkAutoTUnref<SkDataTable> table1(SkDataTable::NewCopyArrays(NULL, NULL, 0));
    SkAutoTUnref<SkDataTable> table2(SkDataTable::NewCopyArray(NULL, 0, 0));
    SkAutoTUnref<SkDataTable> table3(SkDataTable::NewArrayProc(NULL, 0, 0,
                                                               NULL, NULL));

    test_datatable_is_empty(reporter, table0);
    test_datatable_is_empty(reporter, table1);
    test_datatable_is_empty(reporter, table2);
    test_datatable_is_empty(reporter, table3);

    test_is_equal(reporter, table0, table1);
    test_is_equal(reporter, table0, table2);
    test_is_equal(reporter, table0, table3);
}

static void test_simpletable(skiatest::Reporter* reporter) {
    const int idata[] = { 1, 4, 9, 16, 25, 63 };
    int icount = SK_ARRAY_COUNT(idata);
    SkAutoTUnref<SkDataTable> itable(SkDataTable::NewCopyArray(idata,
                                                               sizeof(idata[0]),
                                                               icount));
    REPORTER_ASSERT(reporter, itable->count() == icount);
    for (int i = 0; i < icount; ++i) {
        size_t size;
        REPORTER_ASSERT(reporter, sizeof(int) == itable->atSize(i));
        REPORTER_ASSERT(reporter, *itable->atT<int>(i, &size) == idata[i]);
        REPORTER_ASSERT(reporter, sizeof(int) == size);
    }
}

static void test_vartable(skiatest::Reporter* reporter) {
    const char* str[] = {
        "", "a", "be", "see", "deigh", "ef", "ggggggggggggggggggggggggggg"
    };
    int count = SK_ARRAY_COUNT(str);
    size_t sizes[SK_ARRAY_COUNT(str)];
    for (int i = 0; i < count; ++i) {
        sizes[i] = strlen(str[i]) + 1;
    }

    SkAutoTUnref<SkDataTable> table(SkDataTable::NewCopyArrays(
                                        (const void*const*)str, sizes, count));

    REPORTER_ASSERT(reporter, table->count() == count);
    for (int i = 0; i < count; ++i) {
        size_t size;
        REPORTER_ASSERT(reporter, table->atSize(i) == sizes[i]);
        REPORTER_ASSERT(reporter, !strcmp(table->atT<const char>(i, &size),
                                          str[i]));
        REPORTER_ASSERT(reporter, size == sizes[i]);

        const char* s = table->atStr(i);
        REPORTER_ASSERT(reporter, strlen(s) == strlen(str[i]));
    }
}

static void test_tablebuilder(skiatest::Reporter* reporter) {
    const char* str[] = {
        "", "a", "be", "see", "deigh", "ef", "ggggggggggggggggggggggggggg"
    };
    int count = SK_ARRAY_COUNT(str);

    SkDataTableBuilder builder(16);

    for (int i = 0; i < count; ++i) {
        builder.append(str[i], strlen(str[i]) + 1);
    }
    SkAutoTUnref<SkDataTable> table(builder.detachDataTable());

    REPORTER_ASSERT(reporter, table->count() == count);
    for (int i = 0; i < count; ++i) {
        size_t size;
        REPORTER_ASSERT(reporter, table->atSize(i) == strlen(str[i]) + 1);
        REPORTER_ASSERT(reporter, !strcmp(table->atT<const char>(i, &size),
                                          str[i]));
        REPORTER_ASSERT(reporter, size == strlen(str[i]) + 1);

        const char* s = table->atStr(i);
        REPORTER_ASSERT(reporter, strlen(s) == strlen(str[i]));
    }
}

static void test_globaltable(skiatest::Reporter* reporter) {
    static const int gData[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    int count = SK_ARRAY_COUNT(gData);

    SkAutoTUnref<SkDataTable> table(SkDataTable::NewArrayProc(gData,
                                          sizeof(gData[0]), count, NULL, NULL));

    REPORTER_ASSERT(reporter, table->count() == count);
    for (int i = 0; i < count; ++i) {
        size_t size;
        REPORTER_ASSERT(reporter, table->atSize(i) == sizeof(int));
        REPORTER_ASSERT(reporter, *table->atT<const char>(i, &size) == i);
        REPORTER_ASSERT(reporter, sizeof(int) == size);
    }
}

DEF_TEST(DataTable, reporter) {
    test_emptytable(reporter);
    test_simpletable(reporter);
    test_vartable(reporter);
    test_tablebuilder(reporter);
    test_globaltable(reporter);
}

static void* gGlobal;

static void delete_int_proc(const void* ptr, size_t len, void* context) {
    int* data = (int*)ptr;
    SkASSERT(context == gGlobal);
    delete[] data;
}

static void assert_len(skiatest::Reporter* reporter, SkData* ref, size_t len) {
    REPORTER_ASSERT(reporter, ref->size() == len);
}

static void assert_data(skiatest::Reporter* reporter, SkData* ref,
                        const void* data, size_t len) {
    REPORTER_ASSERT(reporter, ref->size() == len);
    REPORTER_ASSERT(reporter, !memcmp(ref->data(), data, len));
}

static void test_cstring(skiatest::Reporter* reporter) {
    const char str[] = "Hello world";
    size_t     len = strlen(str);

    SkAutoTUnref<SkData> r0(SkData::NewWithCopy(str, len + 1));
    SkAutoTUnref<SkData> r1(SkData::NewWithCString(str));

    REPORTER_ASSERT(reporter, r0->equals(r1));

    SkAutoTUnref<SkData> r2(SkData::NewWithCString(NULL));
    REPORTER_ASSERT(reporter, 1 == r2->size());
    REPORTER_ASSERT(reporter, 0 == *r2->bytes());
}

static void test_files(skiatest::Reporter* reporter) {
    SkString tmpDir = skiatest::Test::GetTmpDir();
    if (tmpDir.isEmpty()) {
        return;
    }

    SkString path = SkOSPath::SkPathJoin(tmpDir.c_str(), "data_test");

    const char s[] = "abcdefghijklmnopqrstuvwxyz";
    {
        SkFILEWStream writer(path.c_str());
        if (!writer.isValid()) {
            SkString msg;
            msg.printf("Failed to create tmp file %s\n", path.c_str());
            reporter->reportFailed(msg);
            return;
        }
        writer.write(s, 26);
    }

    SkFILE* file = sk_fopen(path.c_str(), kRead_SkFILE_Flag);
    SkAutoTUnref<SkData> r1(SkData::NewFromFILE(file));
    REPORTER_ASSERT(reporter, r1.get() != NULL);
    REPORTER_ASSERT(reporter, r1->size() == 26);
    REPORTER_ASSERT(reporter, strncmp(static_cast<const char*>(r1->data()), s, 26) == 0);

    int fd = sk_fileno(file);
    SkAutoTUnref<SkData> r2(SkData::NewFromFD(fd));
    REPORTER_ASSERT(reporter, r2.get() != NULL);
    REPORTER_ASSERT(reporter, r2->size() == 26);
    REPORTER_ASSERT(reporter, strncmp(static_cast<const char*>(r2->data()), s, 26) == 0);
}

DEF_TEST(Data, reporter) {
    const char* str = "We the people, in order to form a more perfect union.";
    const int N = 10;

    SkAutoTUnref<SkData> r0(SkData::NewEmpty());
    SkAutoTUnref<SkData> r1(SkData::NewWithCopy(str, strlen(str)));
    SkAutoTUnref<SkData> r2(SkData::NewWithProc(new int[N], N*sizeof(int),
                                           delete_int_proc, gGlobal));
    SkAutoTUnref<SkData> r3(SkData::NewSubset(r1, 7, 6));

    assert_len(reporter, r0, 0);
    assert_len(reporter, r1, strlen(str));
    assert_len(reporter, r2, N * sizeof(int));
    assert_len(reporter, r3, 6);

    assert_data(reporter, r1, str, strlen(str));
    assert_data(reporter, r3, "people", 6);

    SkData* tmp = SkData::NewSubset(r1, strlen(str), 10);
    assert_len(reporter, tmp, 0);
    tmp->unref();
    tmp = SkData::NewSubset(r1, 0, 0);
    assert_len(reporter, tmp, 0);
    tmp->unref();

    test_cstring(reporter);
    test_files(reporter);
}
