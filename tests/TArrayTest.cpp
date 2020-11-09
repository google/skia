/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/private/SkTArray.h"
#include "include/utils/SkRandom.h"
#include "tests/Test.h"

// Tests the SkTArray<T> class template.

template <bool MEM_MOVE>
static void TestTSet_basic(skiatest::Reporter* reporter) {
    SkTArray<int, MEM_MOVE> a;

    // Starts empty.
    REPORTER_ASSERT(reporter, a.empty());
    REPORTER_ASSERT(reporter, a.count() == 0);

    // { }, add a default constructed element
    a.push_back() = 0;
    REPORTER_ASSERT(reporter, !a.empty());
    REPORTER_ASSERT(reporter, a.count() == 1);

    // { 0 }, removeShuffle the only element.
    a.removeShuffle(0);
    REPORTER_ASSERT(reporter, a.empty());
    REPORTER_ASSERT(reporter, a.count() == 0);

    // { }, add a default, add a 1, remove first
    a.push_back() = 0;
    REPORTER_ASSERT(reporter, a.push_back() = 1);
    a.removeShuffle(0);
    REPORTER_ASSERT(reporter, !a.empty());
    REPORTER_ASSERT(reporter, a.count() == 1);
    REPORTER_ASSERT(reporter, a[0] == 1);

    // { 1 }, replace with new array
    int b[5] = { 0, 1, 2, 3, 4 };
    a.reset(b, SK_ARRAY_COUNT(b));
    REPORTER_ASSERT(reporter, a.count() == SK_ARRAY_COUNT(b));
    REPORTER_ASSERT(reporter, a[2] == 2);
    REPORTER_ASSERT(reporter, a[4] == 4);

    // { 0, 1, 2, 3, 4 }, removeShuffle the last
    a.removeShuffle(4);
    REPORTER_ASSERT(reporter, a.count() == SK_ARRAY_COUNT(b) - 1);
    REPORTER_ASSERT(reporter, a[3] == 3);

    // { 0, 1, 2, 3 }, remove a middle, note shuffle
    a.removeShuffle(1);
    REPORTER_ASSERT(reporter, a.count() == SK_ARRAY_COUNT(b) - 2);
    REPORTER_ASSERT(reporter, a[0] == 0);
    REPORTER_ASSERT(reporter, a[1] == 3);
    REPORTER_ASSERT(reporter, a[2] == 2);

    // { 0, 3, 2 }
}

template <typename T> static void test_construction(skiatest::Reporter* reporter) {
    // No arguments: Creates an empty array with no initial storage.
    T arrayNoArgs;
    REPORTER_ASSERT(reporter, arrayNoArgs.empty());

    // Single integer: Creates an empty array that will preallocate space for reserveCount elements.
    T arrayReserve(15);
    REPORTER_ASSERT(reporter, arrayReserve.empty());
    REPORTER_ASSERT(reporter, arrayReserve.capacity() == 15);

    // Another array, const&: Copies one array to another.
    T arrayInitial;
    arrayInitial.push_back(1);
    arrayInitial.push_back(2);
    arrayInitial.push_back(3);

    T arrayCopy(arrayInitial);
    REPORTER_ASSERT(reporter, arrayInitial.size() == 3);
    REPORTER_ASSERT(reporter, arrayInitial[0] == 1);
    REPORTER_ASSERT(reporter, arrayInitial[1] == 2);
    REPORTER_ASSERT(reporter, arrayInitial[2] == 3);
    REPORTER_ASSERT(reporter, arrayCopy.size() == 3);
    REPORTER_ASSERT(reporter, arrayCopy[0] == 1);
    REPORTER_ASSERT(reporter, arrayCopy[1] == 2);
    REPORTER_ASSERT(reporter, arrayCopy[2] == 3);

    // Another array, &&: Moves one array to another.
    T arrayMove(std::move(arrayInitial));
    REPORTER_ASSERT(reporter, arrayInitial.empty()); // NOLINT(bugprone-use-after-move)
    REPORTER_ASSERT(reporter, arrayMove.size() == 3);
    REPORTER_ASSERT(reporter, arrayMove[0] == 1);
    REPORTER_ASSERT(reporter, arrayMove[1] == 2);
    REPORTER_ASSERT(reporter, arrayMove[2] == 3);

    // Pointer and count: Copies contents of a standard C array.
    typename T::value_type data[3] = { 7, 8, 9 };
    T arrayPtrCount(data, 3);
    REPORTER_ASSERT(reporter, arrayPtrCount.size() == 3);
    REPORTER_ASSERT(reporter, arrayPtrCount[0] == 7);
    REPORTER_ASSERT(reporter, arrayPtrCount[1] == 8);
    REPORTER_ASSERT(reporter, arrayPtrCount[2] == 9);

    // Initializer list.
    T arrayInitializer{8, 6, 7, 5, 3, 0, 9};
    REPORTER_ASSERT(reporter, arrayInitializer.size() == 7);
    REPORTER_ASSERT(reporter, arrayInitializer[0] == 8);
    REPORTER_ASSERT(reporter, arrayInitializer[1] == 6);
    REPORTER_ASSERT(reporter, arrayInitializer[2] == 7);
    REPORTER_ASSERT(reporter, arrayInitializer[3] == 5);
    REPORTER_ASSERT(reporter, arrayInitializer[4] == 3);
    REPORTER_ASSERT(reporter, arrayInitializer[5] == 0);
    REPORTER_ASSERT(reporter, arrayInitializer[6] == 9);
}

template <typename T, typename U>
static void test_skstarray_compatibility(skiatest::Reporter* reporter) {
    // We expect SkTArrays of the same type to be copyable and movable, even when:
    // - one side is an SkTArray, and the other side is an SkSTArray
    // - both sides are SkSTArray, but each side has a different internal capacity
    T tArray;
    tArray.push_back(1);
    tArray.push_back(2);
    tArray.push_back(3);
    T tArray2 = tArray;

    // Copy construction from other-type array.
    U arrayCopy(tArray);
    REPORTER_ASSERT(reporter, tArray.size() == 3);
    REPORTER_ASSERT(reporter, tArray[0] == 1);
    REPORTER_ASSERT(reporter, tArray[1] == 2);
    REPORTER_ASSERT(reporter, tArray[2] == 3);
    REPORTER_ASSERT(reporter, arrayCopy.size() == 3);
    REPORTER_ASSERT(reporter, arrayCopy[0] == 1);
    REPORTER_ASSERT(reporter, arrayCopy[1] == 2);
    REPORTER_ASSERT(reporter, arrayCopy[2] == 3);

    // Assignment from other-type array.
    U arrayAssignment;
    arrayAssignment = tArray;
    REPORTER_ASSERT(reporter, tArray.size() == 3);
    REPORTER_ASSERT(reporter, tArray[0] == 1);
    REPORTER_ASSERT(reporter, tArray[1] == 2);
    REPORTER_ASSERT(reporter, tArray[2] == 3);
    REPORTER_ASSERT(reporter, arrayAssignment.size() == 3);
    REPORTER_ASSERT(reporter, arrayAssignment[0] == 1);
    REPORTER_ASSERT(reporter, arrayAssignment[1] == 2);
    REPORTER_ASSERT(reporter, arrayAssignment[2] == 3);

    // Move construction from other-type array.
    U arrayMove(std::move(tArray));
    REPORTER_ASSERT(reporter, tArray.empty()); // NOLINT(bugprone-use-after-move)
    REPORTER_ASSERT(reporter, arrayMove.size() == 3);
    REPORTER_ASSERT(reporter, arrayMove[0] == 1);
    REPORTER_ASSERT(reporter, arrayMove[1] == 2);
    REPORTER_ASSERT(reporter, arrayMove[2] == 3);

    // Move assignment from other-type array.
    U arrayMoveAssign;
    arrayMoveAssign = std::move(tArray2);
    REPORTER_ASSERT(reporter, tArray2.empty()); // NOLINT(bugprone-use-after-move)
    REPORTER_ASSERT(reporter, arrayMoveAssign.size() == 3);
    REPORTER_ASSERT(reporter, arrayMoveAssign[0] == 1);
    REPORTER_ASSERT(reporter, arrayMoveAssign[1] == 2);
    REPORTER_ASSERT(reporter, arrayMoveAssign[2] == 3);
}

template <typename T> static void test_swap(skiatest::Reporter* reporter,
                                            SkTArray<T>* (&arrays)[4],
                                            int (&sizes)[7])
{
    for (auto a : arrays) {
    for (auto b : arrays) {
        if (a == b) {
            continue;
        }

        for (auto sizeA : sizes) {
        for (auto sizeB : sizes) {
            a->reset();
            b->reset();

            int curr = 0;
            for (int i = 0; i < sizeA; i++) { a->push_back(curr++); }
            for (int i = 0; i < sizeB; i++) { b->push_back(curr++); }

            a->swap(*b);
            REPORTER_ASSERT(reporter, b->count() == sizeA);
            REPORTER_ASSERT(reporter, a->count() == sizeB);

            curr = 0;
            for (auto&& x : *b) { REPORTER_ASSERT(reporter, x == curr++); }
            for (auto&& x : *a) { REPORTER_ASSERT(reporter, x == curr++); }

            a->swap(*a);
            curr = sizeA;
            for (auto&& x : *a) { REPORTER_ASSERT(reporter, x == curr++); }
        }}
    }}
}

static void test_swap(skiatest::Reporter* reporter) {
    int sizes[] = {0, 1, 5, 10, 15, 20, 25};

    SkTArray<int> arr;
    SkSTArray< 5, int> arr5;
    SkSTArray<10, int> arr10;
    SkSTArray<20, int> arr20;
    SkTArray<int>* arrays[] = { &arr, &arr5, &arr10, &arr20 };
    test_swap(reporter, arrays, sizes);

    struct MoveOnlyInt {
        MoveOnlyInt(int i) : fInt(i) {}
        MoveOnlyInt(MoveOnlyInt&& that) : fInt(that.fInt) {}
        bool operator==(int i) { return fInt == i; }
        int fInt;
    };

    SkTArray<MoveOnlyInt> moi;
    SkSTArray< 5, MoveOnlyInt> moi5;
    SkSTArray<10, MoveOnlyInt> moi10;
    SkSTArray<20, MoveOnlyInt> moi20;
    SkTArray<MoveOnlyInt>* arraysMoi[] = { &moi, &moi5, &moi10, &moi20 };
    test_swap(reporter, arraysMoi, sizes);
}

void test_unnecessary_alloc(skiatest::Reporter* reporter) {
    {
        SkTArray<int> a;
        REPORTER_ASSERT(reporter, a.capacity() == 0);
    }
    {
        SkSTArray<10, int> a;
        REPORTER_ASSERT(reporter, a.capacity() == 10);
    }
    {
        SkTArray<int> a(1);
        REPORTER_ASSERT(reporter, a.capacity() >= 1);
    }
    {
        SkTArray<int> a, b;
        b = a;
        REPORTER_ASSERT(reporter, b.capacity() == 0);
    }
    {
        SkSTArray<10, int> a;
        SkTArray<int> b;
        b = a;
        REPORTER_ASSERT(reporter, b.capacity() == 0);
    }
    {
        SkTArray<int> a;
        SkTArray<int> b(a);  // NOLINT(performance-unnecessary-copy-initialization)
        REPORTER_ASSERT(reporter, b.capacity() == 0);
    }
    {
        SkSTArray<10, int> a;
        SkTArray<int> b(a);  // NOLINT(performance-unnecessary-copy-initialization)
        REPORTER_ASSERT(reporter, b.capacity() == 0);
    }
    {
        SkTArray<int> a;
        SkTArray<int> b(std::move(a));
        REPORTER_ASSERT(reporter, b.capacity() == 0);
    }
    {
        SkSTArray<10, int> a;
        SkTArray<int> b(std::move(a));
        REPORTER_ASSERT(reporter, b.capacity() == 0);
    }
    {
        SkTArray<int> a;
        SkTArray<int> b;
        b = std::move(a);
        REPORTER_ASSERT(reporter, b.capacity() == 0);
    }
    {
        SkSTArray<10, int> a;
        SkTArray<int> b;
        b = std::move(a);
        REPORTER_ASSERT(reporter, b.capacity() == 0);
    }
}

static void test_self_assignment(skiatest::Reporter* reporter) {
    SkTArray<int> a;
    a.push_back(1);
    REPORTER_ASSERT(reporter, !a.empty());
    REPORTER_ASSERT(reporter, a.count() == 1);
    REPORTER_ASSERT(reporter, a[0] == 1);

    a = static_cast<decltype(a)&>(a);
    REPORTER_ASSERT(reporter, !a.empty());
    REPORTER_ASSERT(reporter, a.count() == 1);
    REPORTER_ASSERT(reporter, a[0] == 1);
}

template <typename Array> static void test_array_reserve(skiatest::Reporter* reporter,
                                                         Array* array, int reserveCount) {
    SkRandom random;
    REPORTER_ASSERT(reporter, array->capacity() >= reserveCount);
    array->push_back();
    REPORTER_ASSERT(reporter, array->capacity() >= reserveCount);
    array->pop_back();
    REPORTER_ASSERT(reporter, array->capacity() >= reserveCount);
    while (array->count() < reserveCount) {
        // Two steps forward, one step back
        if (random.nextULessThan(3) < 2) {
            array->push_back();
        } else if (array->count() > 0) {
            array->pop_back();
        }
        REPORTER_ASSERT(reporter, array->capacity() >= reserveCount);
    }
}

template<typename Array> static void test_reserve(skiatest::Reporter* reporter) {
    // Test that our allocated space stays >= to the reserve count until the array is filled to
    // the reserve count
    for (int reserveCount : {1, 2, 10, 100}) {
        // Test setting reserve in constructor.
        Array array1(reserveCount);
        test_array_reserve(reporter, &array1, reserveCount);

        // Test setting reserve after constructor.
        Array array2;
        array2.reserve_back(reserveCount);
        test_array_reserve(reporter, &array2, reserveCount);

        // Test increasing reserve after constructor.
        Array array3(reserveCount/2);
        array3.reserve_back(reserveCount);
        test_array_reserve(reporter, &array3, reserveCount);

        // Test setting reserve on non-empty array.
        Array array4;
        array4.push_back_n(reserveCount);
        array4.reserve_back(reserveCount);
        array4.pop_back_n(reserveCount);
        test_array_reserve(reporter, &array4, 2 * reserveCount);
    }
}

DEF_TEST(TArray, reporter) {
    TestTSet_basic<true>(reporter);
    TestTSet_basic<false>(reporter);
    test_swap(reporter);

    test_unnecessary_alloc(reporter);

    test_self_assignment(reporter);

    test_reserve<SkTArray<int>>(reporter);
    test_reserve<SkSTArray<1, int>>(reporter);
    test_reserve<SkSTArray<2, int>>(reporter);
    test_reserve<SkSTArray<16, int>>(reporter);

    test_construction<SkTArray<int>>(reporter);
    test_construction<SkTArray<double>>(reporter);
    test_construction<SkSTArray<1, int>>(reporter);
    test_construction<SkSTArray<5, char>>(reporter);
    test_construction<SkSTArray<10, float>>(reporter);

    test_skstarray_compatibility<SkSTArray<1, int>, SkTArray<int>>(reporter);
    test_skstarray_compatibility<SkSTArray<5, char>, SkTArray<char>>(reporter);
    test_skstarray_compatibility<SkSTArray<10, float>, SkTArray<float>>(reporter);
    test_skstarray_compatibility<SkTArray<int>, SkSTArray<1, int>>(reporter);
    test_skstarray_compatibility<SkTArray<char>, SkSTArray<5, char>>(reporter);
    test_skstarray_compatibility<SkTArray<float>, SkSTArray<10, float>>(reporter);
    test_skstarray_compatibility<SkSTArray<10, uint8_t>, SkSTArray<1, uint8_t>>(reporter);
    test_skstarray_compatibility<SkSTArray<1, long>, SkSTArray<10, long>>(reporter);
    test_skstarray_compatibility<SkSTArray<3, double>, SkSTArray<4, double>>(reporter);
    test_skstarray_compatibility<SkSTArray<2, short>, SkSTArray<1, short>>(reporter);
}
