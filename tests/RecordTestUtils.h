/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef RecordTestUtils_DEFINED
#define RecordTestUtils_DEFINED

#include "SkRecord.h"
#include "SkRecords.h"
#include "Test.h"

// If the command we're reading is a U, set ptr to it, otherwise set it to nullptr.
template <typename U>
struct ReadAs {
    ReadAs() : ptr(nullptr), type(SkRecords::Type(~0)) {}

    const U* ptr;
    SkRecords::Type type;

    void operator()(const U& r) { ptr = &r; type = U::kType; }

    template <typename T>
    void operator()(const T&) { type = U::kType; }
};

// Assert that the ith command in record is of type T, and return it.
template <typename T>
static const T* assert_type(skiatest::Reporter* r, const SkRecord& record, int index) {
    ReadAs<T> reader;
    record.visit(index, reader);
    REPORTER_ASSERT(r, T::kType == reader.type);
    REPORTER_ASSERT(r, SkToBool(reader.ptr));
    return reader.ptr;
}

template <typename DrawT> struct MatchType {
    template <typename T> int operator()(const T&) { return 0; }
    int operator()(const DrawT&) { return 1; }
};

template <typename DrawT> int count_instances_of_type(const SkRecord& record) {
    MatchType<DrawT> matcher;
    int counter = 0;
    for (int i = 0; i < record.count(); i++) {
        counter += record.visit(i, matcher);
    }
    return counter;
}

template <typename DrawT> int find_first_instances_of_type(const SkRecord& record) {
    MatchType<DrawT> matcher;
    for (int i = 0; i < record.count(); i++) {
        if (record.visit(i, matcher)) {
            return i;
        }
    }
    return -1;
}

#endif//RecordTestUtils_DEFINED
