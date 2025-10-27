/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkASAN.h"  // IWYU pragma: keep
#include "include/private/base/SkTArray.h"
#include "src/base/SkFixedArray.h"
#include "src/base/SkRandom.h"
#include "tests/Test.h"

#include <array>
#include <cstdint>
#include <initializer_list>
#include <utility>

using namespace skia_private;

// This class is used to test TArray's behavior with classes containing a vtable.

namespace {

class TestClass {
public:
    TestClass() = default;
    TestClass(const TestClass&) = default;
    TestClass& operator=(const TestClass&) = default;
    TestClass(int v) : value(v) {}
    virtual ~TestClass() {}

    bool operator==(const TestClass& c) const { return value == c.value; }
    bool operator!=(const TestClass& c) const { return value != c.value; }

    int value = 0;
};

}  // namespace

// Tests the TArray<T> class template.

template <typename ArrayType>
static void TestTSet_basic(skiatest::Reporter* reporter) {
    using T = typename ArrayType::value_type;
    ArrayType a;

    // Starts empty.
    REPORTER_ASSERT(reporter, a.empty());
    REPORTER_ASSERT(reporter, a.size() == 0);

    // { }, add a default constructed element
    a.push_back() = T{0};
    REPORTER_ASSERT(reporter, !a.empty());
    REPORTER_ASSERT(reporter, a.size() == 1);

    // { 0 }, removeShuffle the only element.
    a.removeShuffle(0);
    REPORTER_ASSERT(reporter, a.empty());
    REPORTER_ASSERT(reporter, a.size() == 0);

    // { }, add a default, add a 1, remove first
    a.push_back() = T{0};
    a.push_back() = T{1};
    a.removeShuffle(0);
    REPORTER_ASSERT(reporter, !a.empty());
    REPORTER_ASSERT(reporter, a.size() == 1);
    REPORTER_ASSERT(reporter, a[0] == T{1});

    // { 1 }, replace with new array
    T b[5] = {T{0}, T{1}, T{2}, T{3}, T{4}};
    a.reset(b);
    REPORTER_ASSERT(reporter, a.size() == std::size(b));
    REPORTER_ASSERT(reporter, a[2] == T{2});
    REPORTER_ASSERT(reporter, a[4] == T{4});

    // { 0, 1, 2, 3, 4 }, removeShuffle the last
    a.removeShuffle(4);
    REPORTER_ASSERT(reporter, a.size() == std::size(b) - 1);
    REPORTER_ASSERT(reporter, a[3] == T{3});

    // { 0, 1, 2, 3 }, remove a middle, note shuffle
    a.removeShuffle(1);
    REPORTER_ASSERT(reporter, a.size() == std::size(b) - 2);
    REPORTER_ASSERT(reporter, a[0] == T{0});
    REPORTER_ASSERT(reporter, a[1] == T{3});
    REPORTER_ASSERT(reporter, a[2] == T{2});

    // { 0, 3, 2 }... remove the last element.
    a.pop_back();
    REPORTER_ASSERT(reporter, a.size() == std::size(b) - 3);

    // { 0, 3, 5, 5, 5 }... add three elements.
    a.push_back_n(3, 5);
    REPORTER_ASSERT(reporter, a.size() == std::size(b));
    REPORTER_ASSERT(reporter, a[2] == T{5});
    REPORTER_ASSERT(reporter, a[3] == T{5});
    REPORTER_ASSERT(reporter, a[4] == T{5});

    // { 0 }... pop four elements.
    a.pop_back_n(4);
    REPORTER_ASSERT(reporter, a.size() == std::size(b) - 4);
    REPORTER_ASSERT(reporter, a[0] == 0);
}

template <typename T>
static void test_construction(skiatest::Reporter* reporter, bool hasMoveSemantics = true) {
    using ValueType = typename T::value_type;

    // No arguments: Creates an empty array with no initial storage.
    T arrayNoArgs;
    REPORTER_ASSERT(reporter, arrayNoArgs.empty());

    // Single integer: Creates an empty array that will preallocate space for reserveCount elements.
    T arrayReserve(15);
    REPORTER_ASSERT(reporter, arrayReserve.empty());
    // May get some extra elements for free because sk_allocate_* can round up.
    REPORTER_ASSERT(reporter, arrayReserve.capacity() >= 15 && arrayReserve.capacity() < 50);

    // Another array, const&: Copies one array to another.
    T arrayInitial;
    arrayInitial.push_back(ValueType{1});
    arrayInitial.push_back(ValueType{2});
    arrayInitial.push_back(ValueType{3});

    T arrayCopy(arrayInitial);
    REPORTER_ASSERT(reporter, arrayInitial.size() == 3);
    REPORTER_ASSERT(reporter, arrayInitial[0] == ValueType{1});
    REPORTER_ASSERT(reporter, arrayInitial[1] == ValueType{2});
    REPORTER_ASSERT(reporter, arrayInitial[2] == ValueType{3});
    REPORTER_ASSERT(reporter, arrayCopy.size() == 3);
    REPORTER_ASSERT(reporter, arrayCopy[0] == ValueType{1});
    REPORTER_ASSERT(reporter, arrayCopy[1] == ValueType{2});
    REPORTER_ASSERT(reporter, arrayCopy[2] == ValueType{3});

    // Another array, &&: Moves one array to another.
    T arrayMove(std::move(arrayInitial));
    if (hasMoveSemantics) {
        REPORTER_ASSERT(reporter, arrayInitial.empty()); // NOLINT(bugprone-use-after-move)
    }
    REPORTER_ASSERT(reporter, arrayMove.size() == 3);
    REPORTER_ASSERT(reporter, arrayMove[0] == ValueType{1});
    REPORTER_ASSERT(reporter, arrayMove[1] == ValueType{2});
    REPORTER_ASSERT(reporter, arrayMove[2] == ValueType{3});

    // Pointer and count: Copies contents of a standard C array.
    typename T::value_type data[3] = { 7, 8, 9 };
    T arrayPtrCount(data);
    REPORTER_ASSERT(reporter, arrayPtrCount.size() == 3);
    REPORTER_ASSERT(reporter, arrayPtrCount[0] == ValueType{7});
    REPORTER_ASSERT(reporter, arrayPtrCount[1] == ValueType{8});
    REPORTER_ASSERT(reporter, arrayPtrCount[2] == ValueType{9});

    // Initializer list.
    T arrayInitializer{8, 6, 7, 5, 3, 0, 9};
    REPORTER_ASSERT(reporter, arrayInitializer.size() == 7);
    REPORTER_ASSERT(reporter, arrayInitializer[0] == ValueType{8});
    REPORTER_ASSERT(reporter, arrayInitializer[1] == ValueType{6});
    REPORTER_ASSERT(reporter, arrayInitializer[2] == ValueType{7});
    REPORTER_ASSERT(reporter, arrayInitializer[3] == ValueType{5});
    REPORTER_ASSERT(reporter, arrayInitializer[4] == ValueType{3});
    REPORTER_ASSERT(reporter, arrayInitializer[5] == ValueType{0});
    REPORTER_ASSERT(reporter, arrayInitializer[6] == ValueType{9});
}

template <typename T, typename U>
static void test_starray_compatibility(skiatest::Reporter* reporter) {
    // We expect TArrays of the same type to be copyable and movable, even when:
    // - one side is an TArray, and the other side is an STArray
    // - both sides are STArray, but each side has a different internal capacity
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

// Move-only type used for testing swap and move_back() of TArray&&'s.
namespace {
struct MoveOnlyInt {
    MoveOnlyInt(int i) : fInt(i) {}
    MoveOnlyInt(MoveOnlyInt&& that) : fInt(that.fInt) {}
    bool operator==(int i) const { return fInt == i; }
    int fInt;
};
} // anonymous

template <typename T> static void test_swap(skiatest::Reporter* reporter,
                                            SkSpan<TArray<T>*> arrays,
                                            SkSpan<const int> sizes) {
    for (auto a : arrays) {
    for (auto b : arrays) {
        if (a == b) {
            continue;
        }

        for (auto sizeA : sizes) {
        for (auto sizeB : sizes) {
            a->clear();
            b->clear();

            int curr = 0;
            for (int i = 0; i < sizeA; i++) { a->push_back(curr++); }
            for (int i = 0; i < sizeB; i++) { b->push_back(curr++); }

            a->swap(*b);
            REPORTER_ASSERT(reporter, b->size() == sizeA);
            REPORTER_ASSERT(reporter, a->size() == sizeB);

            curr = 0;
            for (auto&& x : *b) { REPORTER_ASSERT(reporter, x == curr++); }
            for (auto&& x : *a) { REPORTER_ASSERT(reporter, x == curr++); }

            a->swap(*a);
            curr = sizeA;
            for (auto&& x : *a) { REPORTER_ASSERT(reporter, x == curr++); }
        }}
    }}
}

DEF_TEST(TArray_Swap, reporter) {
    static constexpr int kSizes[] = {0, 1, 5, 10, 15, 20, 25};

    TArray<int> arr;
    STArray< 5, int> arr5;
    STArray<10, int> arr10;
    STArray<20, int> arr20;
    TArray<int>* arrays[] = { &arr, &arr5, &arr10, &arr20 };
    test_swap<int>(reporter, arrays, kSizes);

    TArray<MoveOnlyInt> moi;
    STArray< 5, MoveOnlyInt> moi5;
    STArray<10, MoveOnlyInt> moi10;
    STArray<20, MoveOnlyInt> moi20;
    TArray<MoveOnlyInt>* arraysMoi[] = { &moi, &moi5, &moi10, &moi20 };
    test_swap<MoveOnlyInt>(reporter, arraysMoi, kSizes);
}

template <typename T> static void test_array_move(skiatest::Reporter* reporter,
                                                  SkSpan<TArray<T>*> arrays,
                                                  SkSpan<const int> sizes) {
    // self test is a no-op
    for (auto a : arrays) {
        for (auto sizeA : sizes) {
            a->clear();
            for (int i = 0; i < sizeA; i++) { a->push_back(i); }
            a->move_back(*a);
            REPORTER_ASSERT(reporter, a->size() == sizeA);
            for (int i = 0; i < sizeA; i++) {
                REPORTER_ASSERT(reporter, (*a)[i] == i);
            }
        }
    }

    for (auto a : arrays) {
    for (auto b : arrays) {
        if (a == b) {
            continue;
        }

        for (auto sizeA : sizes) {
        for (auto sizeB : sizes) {
            a->clear();
            b->clear();

            int curr = 0;
            for (int i = 0; i < sizeA; i++) { a->push_back(curr++); }
            for (int i = 0; i < sizeB; i++) { b->push_back(curr++); }

            a->move_back(*b);
            REPORTER_ASSERT(reporter, b->size() == 0);
            REPORTER_ASSERT(reporter, a->size() == sizeA + sizeB);

            curr = 0;
            for (auto&& x : *a) { REPORTER_ASSERT(reporter, x == curr++); }
        }}
    }}
}

DEF_TEST(TArray_Move, reporter) {
    static constexpr int kSizes[] = {0, 1, 5, 10, 15, 20, 25};

    TArray<int> arr;
    STArray< 5, int> arr5;
    STArray<10, int> arr10;
    STArray<20, int> arr20;
    TArray<int>* arrays[] = { &arr, &arr5, &arr10, &arr20 };
    test_array_move<int>(reporter, arrays, kSizes);

    TArray<MoveOnlyInt> moi;
    STArray< 5, MoveOnlyInt> moi5;
    STArray<10, MoveOnlyInt> moi10;
    STArray<20, MoveOnlyInt> moi20;
    TArray<MoveOnlyInt>* arraysMoi[] = { &moi, &moi5, &moi10, &moi20 };
    test_array_move<MoveOnlyInt>(reporter, arraysMoi, kSizes);
}

DEF_TEST(TArray_NoUnnecessaryAllocs, reporter) {
    {
        TArray<int> a;
        REPORTER_ASSERT(reporter, a.capacity() == 0);
    }
    {
        STArray<10, int> a;
        REPORTER_ASSERT(reporter, a.capacity() == 10);
    }
    {
        TArray<int> a(1);
        REPORTER_ASSERT(reporter, a.capacity() >= 1);
    }
    {
        TArray<int> a, b;
        b = a;
        REPORTER_ASSERT(reporter, b.capacity() == 0);
    }
    {
        STArray<10, int> a;
        TArray<int> b;
        b = a;
        REPORTER_ASSERT(reporter, b.capacity() == 0);
    }
    {
        TArray<int> a;
        TArray<int> b(a);  // NOLINT(performance-unnecessary-copy-initialization)
        REPORTER_ASSERT(reporter, b.capacity() == 0);
    }
    {
        STArray<10, int> a;
        TArray<int> b(a);  // NOLINT(performance-unnecessary-copy-initialization)
        REPORTER_ASSERT(reporter, b.capacity() == 0);
    }
    {
        TArray<int> a;
        TArray<int> b(std::move(a));
        REPORTER_ASSERT(reporter, b.capacity() == 0);
    }
    {
        STArray<10, int> a;
        TArray<int> b(std::move(a));
        REPORTER_ASSERT(reporter, b.capacity() == 0);
    }
    {
        TArray<int> a;
        TArray<int> b;
        b = std::move(a);
        REPORTER_ASSERT(reporter, b.capacity() == 0);
    }
    {
        STArray<10, int> a;
        TArray<int> b;
        b = std::move(a);
        REPORTER_ASSERT(reporter, b.capacity() == 0);
    }
}

template <typename ArrayType>
static void test_self_assignment(skiatest::Reporter* reporter) {
    ArrayType a;
    a.push_back(1);
    REPORTER_ASSERT(reporter, !a.empty());
    REPORTER_ASSERT(reporter, a.size() == 1);
    REPORTER_ASSERT(reporter, a[0] == 1);

    a = static_cast<ArrayType&>(a);
    REPORTER_ASSERT(reporter, !a.empty());
    REPORTER_ASSERT(reporter, a.size() == 1);
    REPORTER_ASSERT(reporter, a[0] == 1);
}

DEF_TEST(TArray_SelfAssignment, reporter) {
    test_self_assignment<TArray<int>>(reporter);
    test_self_assignment<STArray<3, unsigned short>>(reporter);
}

DEF_TEST(FixedArray_SelfAssignment, reporter) {
    test_self_assignment<FixedArray<1, int>>(reporter);
    test_self_assignment<FixedArray<4, unsigned short>>(reporter);
}

template <typename ArrayType>
static void test_comparison(skiatest::Reporter* reporter) {
    using T = typename ArrayType::value_type;
    ArrayType a, b;

    // Empty arrays.
    REPORTER_ASSERT(reporter, a == b);
    REPORTER_ASSERT(reporter, !(a != b));

    // Arrays with identical contents.
    for (int x = 0; x < 10; ++x) {
        a.push_back(T(x));
        b.push_back(T(x));
        REPORTER_ASSERT(reporter, a == b);
        REPORTER_ASSERT(reporter, !(a != b));
    }

    // Arrays with differing sizes.
    for (int x = 0; x < 10; ++x) {
        a.pop_back();
        REPORTER_ASSERT(reporter, a != b);
        REPORTER_ASSERT(reporter, b != a);
        REPORTER_ASSERT(reporter, !(a == b));
        REPORTER_ASSERT(reporter, !(b == a));
    }

    // Arrays with differing contents.
    a = b;
    for (int x = 0; x < 10; ++x) {
        a[x] = T(x + 100);
        REPORTER_ASSERT(reporter, a != b);
        REPORTER_ASSERT(reporter, b != a);
        REPORTER_ASSERT(reporter, !(a == b));
        REPORTER_ASSERT(reporter, !(b == a));
        a[x] = T(x);
    }
}

DEF_TEST(TArray_Comparison, reporter) {
    test_comparison<TArray<int>>(reporter);
    test_comparison<TArray<double>>(reporter);
    test_comparison<TArray<TestClass>>(reporter);
    test_comparison<STArray<1, int>>(reporter);
    test_comparison<STArray<5, char>>(reporter);
    test_comparison<STArray<7, TestClass>>(reporter);
    test_comparison<STArray<10, float>>(reporter);
}

DEF_TEST(FixedArray_Comparison, reporter) {
    test_comparison<FixedArray<15, int>>(reporter);
    test_comparison<FixedArray<20, char>>(reporter);
    test_comparison<FixedArray<25, float>>(reporter);
}

template <typename Array> static void test_array_reserve(skiatest::Reporter* reporter,
                                                         Array* array, int reserveCount) {
    SkRandom random;
    REPORTER_ASSERT(reporter, array->capacity() >= reserveCount);
    array->push_back();
    REPORTER_ASSERT(reporter, array->capacity() >= reserveCount);
    array->pop_back();
    REPORTER_ASSERT(reporter, array->capacity() >= reserveCount);
    while (array->size() < reserveCount) {
        // Two steps forward, one step back
        if (random.nextULessThan(3) < 2) {
            array->push_back();
        } else if (array->size() > 0) {
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
        array2.reserve(reserveCount);
        test_array_reserve(reporter, &array2, reserveCount);

        // Test increasing reserve after constructor.
        Array array3(reserveCount/2);
        array3.reserve(reserveCount);
        test_array_reserve(reporter, &array3, reserveCount);

        // Test setting reserve on non-empty array.
        Array array4;
        array4.push_back_n(reserveCount);
        array4.reserve(2 * reserveCount);
        array4.pop_back_n(reserveCount);
        test_array_reserve(reporter, &array4, 2 * reserveCount);
    }
}

template <typename T>
static void test_inner_push(skiatest::Reporter* reporter) {
    T a;
    a.push_back(12345);
    for (int x=0; x<50; ++x) {
        a.push_back(a.front());
    }
    for (int x=0; x<50; ++x) {
        a.push_back(a.back());
    }

    REPORTER_ASSERT(reporter, a.size() == 101);
    REPORTER_ASSERT(reporter, std::count(a.begin(), a.end(), 12345) == a.size());
}

struct EmplaceStruct {
    EmplaceStruct(int v) : fValue(v) {}
    int fValue;
};

template <typename T>
static void test_inner_emplace(skiatest::Reporter* reporter) {
    T a;
    a.emplace_back(12345);
    for (int x=0; x<50; ++x) {
        a.emplace_back(a.front().fValue);
    }
    for (int x=0; x<50; ++x) {
        a.emplace_back(a.back().fValue);
    }

    REPORTER_ASSERT(reporter, a.size() == 101);
    REPORTER_ASSERT(reporter, std::all_of(a.begin(), a.end(), [](const EmplaceStruct& s) {
                        return s.fValue == 12345;
                    }));
}

DEF_TEST(TArray_Basic, reporter) {
    // ints are POD types and can work with either MEM_MOVE=true or false.
    TestTSet_basic<TArray<int, true>>(reporter);
    TestTSet_basic<TArray<int, false>>(reporter);

    // TestClass has a vtable and can only work with MEM_MOVE=false.
    TestTSet_basic<TArray<TestClass, false>>(reporter);
}

DEF_TEST(FixedArray_Basic, reporter) {
    TestTSet_basic<FixedArray<5, char>>(reporter);
    TestTSet_basic<FixedArray<7, int>>(reporter);
    TestTSet_basic<FixedArray<100, double>>(reporter);
}

DEF_TEST(TArray_Reserve, reporter) {
    test_reserve<TArray<int>>(reporter);
    test_reserve<STArray<1, int>>(reporter);
    test_reserve<STArray<2, int>>(reporter);
    test_reserve<STArray<16, int>>(reporter);

    test_reserve<TArray<TestClass>>(reporter);
    test_reserve<STArray<1, TestClass>>(reporter);
    test_reserve<STArray<2, TestClass>>(reporter);
    test_reserve<STArray<16, TestClass>>(reporter);
}

DEF_TEST(TArray_Construction, reporter) {
    test_construction<TArray<int>>(reporter);
    test_construction<TArray<double>>(reporter);
    test_construction<TArray<TestClass>>(reporter);
    test_construction<STArray<1, int>>(reporter);
    test_construction<STArray<5, char>>(reporter);
    test_construction<STArray<7, TestClass>>(reporter);
    test_construction<STArray<10, float>>(reporter);
}

DEF_TEST(FixedArray_Construction, reporter) {
    test_construction<FixedArray<15, int>>(reporter, /*hasMoveSemantics=*/false);
    test_construction<FixedArray<20, char>>(reporter, /*hasMoveSemantics=*/false);
    test_construction<FixedArray<25, float>>(reporter, /*hasMoveSemantics=*/false);
}

DEF_TEST(TArray_InnerPush, reporter) {
    test_inner_push<TArray<int>>(reporter);
    test_inner_push<STArray<1, int>>(reporter);
    test_inner_push<STArray<99, int>>(reporter);
    test_inner_push<STArray<200, int>>(reporter);
}

DEF_TEST(FixedArray_InnerPush, reporter) {
    test_inner_push<FixedArray<101, int>>(reporter);
    test_inner_push<FixedArray<150, short>>(reporter);
    test_inner_push<FixedArray<250, double>>(reporter);
}

DEF_TEST(TArray_InnerEmplace, reporter) {
    test_inner_emplace<TArray<EmplaceStruct>>(reporter);
    test_inner_emplace<STArray<1, EmplaceStruct>>(reporter);
    test_inner_emplace<STArray<99, EmplaceStruct>>(reporter);
    test_inner_emplace<STArray<200, EmplaceStruct>>(reporter);
}

DEF_TEST(TArray_STArrayCompatibility, reporter) {
    test_starray_compatibility<STArray<1, int>, TArray<int>>(reporter);
    test_starray_compatibility<STArray<5, char>, TArray<char>>(reporter);
    test_starray_compatibility<STArray<10, float>, TArray<float>>(reporter);
    test_starray_compatibility<TArray<int>, STArray<1, int>>(reporter);
    test_starray_compatibility<TArray<char>, STArray<5, char>>(reporter);
    test_starray_compatibility<TArray<float>, STArray<10, float>>(reporter);
    test_starray_compatibility<STArray<10, uint8_t>, STArray<1, uint8_t>>(reporter);
    test_starray_compatibility<STArray<1, long>, STArray<10, long>>(reporter);
    test_starray_compatibility<STArray<3, double>, STArray<4, double>>(reporter);
    test_starray_compatibility<STArray<2, short>, STArray<1, short>>(reporter);
}

DEF_TEST(TArray_BoundsCheck, reporter) {
#if 0  // The v[0] fails
    TArray<int> v;
    v[0];
#endif
}

#if defined(SK_SANITIZE_ADDRESS)

template <typename Array>
static void verify_poison(skiatest::Reporter* r, const Array& array) {
    int allocated = array.size() * sizeof(typename Array::value_type);
    int capacity = array.capacity() * sizeof(typename Array::value_type);
    const std::byte* data = reinterpret_cast<const std::byte*>(array.data());

    for (int index = 0; index < allocated; ++index) {
        REPORTER_ASSERT(r, !sk_asan_address_is_poisoned(data + index));
    }

    // ASAN user poisoning is conservative for ranges that are smaller than eight bytes long.
    // We guarantee this alignment via SkContainerAllocator (because kCapacityMultiple == 8).
    // https://github.com/google/sanitizers/wiki/AddressSanitizerAlgorithm#mapping
    REPORTER_ASSERT(r, capacity >= 8);
    for (int index = allocated; index < capacity; ++index) {
        REPORTER_ASSERT(r, sk_asan_address_is_poisoned(data + index));
    }
}

template <typename Array>
static void test_poison(skiatest::Reporter* reporter) {
    Array array;

    for (int index = 0; index < 20; ++index) {
        array.emplace_back();
        verify_poison(reporter, array);
    }

    for (int index = 0; index < 20; ++index) {
        array.pop_back();
        verify_poison(reporter, array);
    }

    for (int index = 0; index < 20; ++index) {
        array.reserve(array.capacity() + 3);
        verify_poison(reporter, array);
    }

    array.clear();
    verify_poison(reporter, array);
}

DEF_TEST(TArray_ASANPoisoning, reporter) {
    test_poison<TArray<int>>(reporter);
    test_poison<STArray<1, double>>(reporter);
    test_poison<STArray<2, char>>(reporter);
    test_poison<STArray<16, TestClass>>(reporter);
}

#endif
