/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkDataTable.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkTaskGroup.h"
#include "src/utils/SkOSPath.h"
#include "tests/Test.h"

#include <cstdio>
#include <cstring>
#include <memory>

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

static void test_datatable_is_empty(skiatest::Reporter* reporter, SkDataTable* table) {
    REPORTER_ASSERT(reporter, table->isEmpty());
    REPORTER_ASSERT(reporter, 0 == table->count());
}

static void test_emptytable(skiatest::Reporter* reporter) {
    sk_sp<SkDataTable> table0(SkDataTable::MakeEmpty());
    sk_sp<SkDataTable> table1(SkDataTable::MakeCopyArrays(nullptr, nullptr, 0));
    sk_sp<SkDataTable> table2(SkDataTable::MakeCopyArray(nullptr, 0, 0));
    sk_sp<SkDataTable> table3(SkDataTable::MakeArrayProc(nullptr, 0, 0, nullptr, nullptr));

    test_datatable_is_empty(reporter, table0.get());
    test_datatable_is_empty(reporter, table1.get());
    test_datatable_is_empty(reporter, table2.get());
    test_datatable_is_empty(reporter, table3.get());

    test_is_equal(reporter, table0.get(), table1.get());
    test_is_equal(reporter, table0.get(), table2.get());
    test_is_equal(reporter, table0.get(), table3.get());
}

static void test_simpletable(skiatest::Reporter* reporter) {
    const int idata[] = { 1, 4, 9, 16, 25, 63 };
    int icount = SK_ARRAY_COUNT(idata);
    sk_sp<SkDataTable> itable(SkDataTable::MakeCopyArray(idata, sizeof(idata[0]), icount));
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

    sk_sp<SkDataTable> table(SkDataTable::MakeCopyArrays((const void*const*)str, sizes, count));

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

static void test_globaltable(skiatest::Reporter* reporter) {
    static const int gData[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    int count = SK_ARRAY_COUNT(gData);

    sk_sp<SkDataTable> table(
        SkDataTable::MakeArrayProc(gData, sizeof(gData[0]), count, nullptr, nullptr));

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
    test_globaltable(reporter);
}

static void* gGlobal;

static void delete_int_proc(const void* ptr, void* context) {
    int* data = (int*)ptr;
    SkASSERT(context == gGlobal);
    delete[] data;
}

static void assert_len(skiatest::Reporter* reporter, const sk_sp<SkData>& ref, size_t len) {
    REPORTER_ASSERT(reporter, ref->size() == len);
}

static void assert_data(skiatest::Reporter* reporter, const sk_sp<SkData>& ref,
                        const void* data, size_t len) {
    REPORTER_ASSERT(reporter, ref->size() == len);
    REPORTER_ASSERT(reporter, !memcmp(ref->data(), data, len));
}

static void test_cstring(skiatest::Reporter* reporter) {
    const char str[] = "Hello world";
    size_t     len = strlen(str);

    sk_sp<SkData> r0(SkData::MakeWithCopy(str, len + 1));
    sk_sp<SkData> r1(SkData::MakeWithCString(str));

    REPORTER_ASSERT(reporter, r0->equals(r1.get()));

    sk_sp<SkData> r2(SkData::MakeWithCString(nullptr));
    REPORTER_ASSERT(reporter, 1 == r2->size());
    REPORTER_ASSERT(reporter, 0 == *r2->bytes());
}

static void test_files(skiatest::Reporter* reporter) {
    SkString tmpDir = skiatest::GetTmpDir();
    if (tmpDir.isEmpty()) {
        return;
    }

    SkString path = SkOSPath::Join(tmpDir.c_str(), "data_test");

    const char s[] = "abcdefghijklmnopqrstuvwxyz";
    {
        SkFILEWStream writer(path.c_str());
        if (!writer.isValid()) {
            ERRORF(reporter, "Failed to create tmp file %s\n", path.c_str());
            return;
        }
        writer.write(s, 26);
    }

    FILE* file = sk_fopen(path.c_str(), kRead_SkFILE_Flag);
    sk_sp<SkData> r1(SkData::MakeFromFILE(file));
    REPORTER_ASSERT(reporter, r1.get() != nullptr);
    REPORTER_ASSERT(reporter, r1->size() == 26);
    REPORTER_ASSERT(reporter, strncmp(static_cast<const char*>(r1->data()), s, 26) == 0);

    int fd = sk_fileno(file);
    sk_sp<SkData> r2(SkData::MakeFromFD(fd));
    REPORTER_ASSERT(reporter, r2.get() != nullptr);
    REPORTER_ASSERT(reporter, r2->size() == 26);
    REPORTER_ASSERT(reporter, strncmp(static_cast<const char*>(r2->data()), s, 26) == 0);
}

DEF_TEST(Data, reporter) {
    const char* str = "We the people, in order to form a more perfect union.";
    const int N = 10;

    sk_sp<SkData> r0(SkData::MakeEmpty());
    sk_sp<SkData> r1(SkData::MakeWithCopy(str, strlen(str)));
    sk_sp<SkData> r2(SkData::MakeWithProc(new int[N], N*sizeof(int), delete_int_proc, gGlobal));
    sk_sp<SkData> r3(SkData::MakeSubset(r1.get(), 7, 6));

    assert_len(reporter, r0, 0);
    assert_len(reporter, r1, strlen(str));
    assert_len(reporter, r2, N * sizeof(int));
    assert_len(reporter, r3, 6);

    assert_data(reporter, r1, str, strlen(str));
    assert_data(reporter, r3, "people", 6);

    sk_sp<SkData> tmp(SkData::MakeSubset(r1.get(), strlen(str), 10));
    assert_len(reporter, tmp, 0);
    tmp = SkData::MakeSubset(r1.get(), 0, 0);
    assert_len(reporter, tmp, 0);

    test_cstring(reporter);
    test_files(reporter);
}

DEF_TEST(Data_empty, reporter) {
    sk_sp<SkData> array[] = {
        SkData::MakeEmpty(),
        SkData::MakeUninitialized(0),
        SkData::MakeFromMalloc(sk_malloc_throw(0), 0),
        SkData::MakeWithCopy("", 0),
        SkData::MakeWithProc(nullptr, 0, [](const void*, void*){}, nullptr),
        SkData::MakeWithoutCopy(nullptr, 0),
    };
    constexpr int N = SK_ARRAY_COUNT(array);

    for (int i = 0; i < N; ++i) {
        REPORTER_ASSERT(reporter, array[i]->size() == 0);
        for (int j = 0; j < N; ++j) {
            REPORTER_ASSERT(reporter, array[i]->equals(array[j].get()));
        }
    }
}
