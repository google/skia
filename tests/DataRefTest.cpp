
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkData.h"
#include "SkDataSet.h"
#include "SkStream.h"

template <typename T> class SkTUnref {
public:
    SkTUnref(T* ref) : fRef(ref) {}
    ~SkTUnref() { fRef->unref(); }
    
    operator T*() { return fRef; }
    operator const T*() { return fRef; }

private:
    T*  fRef;
};

static void unrefAll(const SkDataSet::Pair pairs[], int count) {
    for (int i = 0; i < count; ++i) {
        pairs[i].fValue->unref();
    }
}

// asserts that inner is a subset of outer
static void test_dataset_subset(skiatest::Reporter* reporter,
                                const SkDataSet& outer, const SkDataSet& inner) {
    SkDataSet::Iter iter(inner);
    for (; !iter.done(); iter.next()) {
        SkData* outerData = outer.find(iter.key());
        REPORTER_ASSERT(reporter, outerData);
        REPORTER_ASSERT(reporter, outerData->equals(iter.value()));
    }
}

static void test_datasets_equal(skiatest::Reporter* reporter,
                                const SkDataSet& ds0, const SkDataSet& ds1) {
    REPORTER_ASSERT(reporter, ds0.count() == ds1.count());
    
    test_dataset_subset(reporter, ds0, ds1);
    test_dataset_subset(reporter, ds1, ds0);
}

static void test_dataset(skiatest::Reporter* reporter, const SkDataSet& ds,
                         int count) {
    REPORTER_ASSERT(reporter, ds.count() == count);
    
    SkDataSet::Iter iter(ds);
    int index = 0;
    for (; !iter.done(); iter.next()) {
        const char* name = iter.key();
        SkData* data = iter.value();
        SkDebugf("[%d] %s:%s\n", index, name, (const char*)data->bytes());
        index += 1;
    }
    REPORTER_ASSERT(reporter, index == count);

    SkDynamicMemoryWStream ostream;
    ds.writeToStream(&ostream);
    SkMemoryStream istream;
    istream.setData(ostream.copyToData())->unref();
    SkDataSet copy(&istream);
    
    test_datasets_equal(reporter, ds, copy);
}

static void test_dataset(skiatest::Reporter* reporter) {
    SkDataSet set0(NULL, 0);
    SkDataSet set1("hello", SkTUnref<SkData>(SkData::NewWithCString("world")));
    
    const SkDataSet::Pair pairs[] = {
        { "one", SkData::NewWithCString("1") },
        { "two", SkData::NewWithCString("2") },
        { "three", SkData::NewWithCString("3") },
    };
    SkDataSet set3(pairs, 3);
    unrefAll(pairs, 3);

    test_dataset(reporter, set0, 0);
    test_dataset(reporter, set1, 1);
    test_dataset(reporter, set3, 3);
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

static void TestData(skiatest::Reporter* reporter) {
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
    test_dataset(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Data", DataTestClass, TestData)
