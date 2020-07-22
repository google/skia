/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/utils/SkRandom.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkArenaAllocList.h"
#include "src/gpu/GrSTArenaList.h"
#include "src/gpu/GrTAllocator.h"

#include <type_traits>
#include <vector>

// Test list/array-like collection implementations through a variety of access patterns.
// Define static functions that standardize push/pop/get/iterate/concat/push_n/reset for
//   std::vector, SkTArray, GrTBlockLinkedList
namespace {

template<typename T, int S>
struct StdVectorAdapter {
    std::vector<T> list;

    StdVectorAdapter(int) : list(S) {}

    void push(const T& t) {
        list.push_back(t);
    }

    void push_n(int n, const T& t) {
        list.reserve(n);
        for (int i = 0; i < n; ++i) {
            list.push_back(t);
        }
    }

    void concat(StdVectorAdapter<T, S>&& other) {
        list.insert(list.end(), other.list.begin(), other.list.end());
    }

    void pop() {
        list.pop_back();
    }

    void reset() {
        list.clear();
    }

    const T& get(int i) const {
        return list[i];
    }

    int iterate(int (*fn)(const T&, int)) const {
        int i = 0;
        int total = 0;
        for (const T& t : list) {
            total += fn(t, i);
            i++;
        }
        return total;
    }
};

template<typename T, int S>
struct SkTArrayAdapter {
    SkSTArray<S, T> list;


    SkTArrayAdapter(int) : list(S) {}

    void push(const T& t) {
        list.push_back(t);
    }

    void push_n(int n, const T& t) {
        list.push_back_n(n, t);
    }

    void concat(SkTArrayAdapter<T, S>&& other) {
        list.push_back_n(other.list.count(), other.list.begin());
    }

    void pop() {
        list.pop_back();
    }

    void reset() {
        list.reset();
    }

    const T& get(int i) const {
        return list[i];
    }

    int iterate(int (*fn)(const T&, int)) const {
        int total = 0;
        for (int i = 0; i < list.count(); ++i) {
            total += fn(list[i], i);
        }
        return total;
    }
};

template<typename T, int S>
struct GrTBlockLinkedListAdapter {
    GrTAllocator<T, S> list;

    GrTBlockLinkedListAdapter(int heapItemIncrement) : list(heapItemIncrement) {}

    void push(const T& t) {
        list.push_back(t);
    }

    void push_n(int n, const T& t) {
        list.reserve(n);
        for (int i = 0; i < n; ++i) {
            list.push_back(t);
        }
    }

    void concat(GrTBlockLinkedListAdapter<T, S>&& other) {
        list.concat(std::move(other.list));
    }

    void pop() {
        list.pop_back();
    }

    void reset() {
        list.reset();
    }

    const T& get(int i) const {
        return list.item(i);
    }

    int iterate(int (*fn)(const T&, int)) const {
        int i = 0;
        int total = 0;
        for (const T& t : list.items()) {
            total += fn(t, i);
            ++i;
        }
        return total;
    }
};

enum class BenchMode {
    kAllocateOnly,
    kPushPop,
    kConcat,
    kRandomAccess
};

static constexpr int kS = 64;

template<typename T, template<class, int> typename V>
class StackBench : public Benchmark {
public:
    StackBench(const char* typeName, BenchMode benchMode, int pushCount)
            : fMode(benchMode)
            , fPushCount(pushCount) {
        const char* modeName;
        switch(benchMode) {
            case BenchMode::kAllocateOnly: modeName = "allocate"; break;
            case BenchMode::kPushPop:      modeName = "pushpop"; break;
            case BenchMode::kConcat:       modeName = "concat"; break;
            case BenchMode::kRandomAccess: modeName = "ra"; break;
        }
        fName.appendf("stackbench_%s_%s_%d", modeName, typeName, pushCount);
    }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

protected:
    static int ItemToInt(const T& t, int i) {
        return i + t.toInt();
    }

    const char* onGetName() override {
        return fName.c_str();
    }

    void onDraw(int loops, SkCanvas*) override {
        int total = 0;
        for (int l = 0; l < loops; ++l) {
            V<T,kS> list(kS);
            // push n items (done for all bench modes)
            for (int i = 0; i < fPushCount; ++i) {
                list.push(T(i));
            }

            switch(fMode) {
                case BenchMode::kAllocateOnly:
                    break; // nothing else is needed
                case BenchMode::kPushPop:
                    for (int i = 0; i < 2 * fPushCount; ++i) {
                        if (i % 2 == 0) {
                            list.push(T(i));
                        } else {
                            list.pop();
                        }
                    }
                    break;
                case BenchMode::kConcat: {
                    V<T,kS> list2(kS);
                    list2.push_n(fPushCount, T(-1));

                    // and concat
                    list.concat(std::move(list2));
                    break; }
                case BenchMode::kRandomAccess: {
                    SkRandom r;
                    for (int i = 0; i < fPushCount; ++i) {
                        total += ItemToInt(list.get((int) r.nextULessThan(fPushCount)), i);
                    }
                    break; }
            }

            // then iterate over the remaining collection (for all bench modes)
            total += list.iterate(&ItemToInt);

            list.reset();
        }
    }

    BenchMode fMode;
    int       fPushCount;
    SkString  fName;
};

struct SmallTrivialT {
    SmallTrivialT() = default;
    explicit SmallTrivialT(int i) : id(i) {}

    int id;

    int toInt() const { return id; }
};
static_assert(std::is_trivially_destructible<SmallTrivialT>::value &&
              std::is_trivially_copyable<SmallTrivialT>::value);

struct SmallComplexT {
    SmallComplexT() {
        id = 0;
    }

    explicit SmallComplexT(int i) {
        id = i + 1;
    }

    SmallComplexT(const SmallComplexT& t) {
        id = t.id + 1;
    }

    ~SmallComplexT() {
        id = 0;
    }

    int id;

    int toInt() const { return id; }
};
static_assert(!std::is_trivially_destructible<SmallComplexT>::value &&
              !std::is_trivially_copyable<SmallComplexT>::value);


struct LargeTrivialT {
    LargeTrivialT() = default;
    explicit LargeTrivialT(int i) : id(i) {}

    int id;
    int extra[10];

    int toInt() const { return id; }
};
static_assert(std::is_trivially_destructible<LargeTrivialT>::value &&
              std::is_trivially_copyable<LargeTrivialT>::value);

struct LargeComplexT {
    LargeComplexT() {
        id = 0;
        for (int i = 0; i < 10; ++i) {
            extra[i] = i;
        }
    }

    explicit LargeComplexT(int i) : id(i) {
        for (int j = 0; j < 10; ++j) {
            extra[j] = j + i;
        }
    }

    LargeComplexT(const LargeComplexT& t) {
        id = t.id;
        for (int j = 0; j < 10; ++j) {
            extra[j] = t.extra[j];
        }
    }

    ~LargeComplexT() {
        if ((uintptr_t) &extra != (uintptr_t) &id + sizeof(int)) {
            SkASSERT(false); // should never happen, but makes it non trivially destructible
            extra[0] = 1;
        }
    }

    int id;
    int extra[10];

    int toInt() const { return id; }
};
static_assert(!std::is_trivially_destructible<LargeComplexT>::value &&
              !std::is_trivially_copyable<LargeComplexT>::value);
}

// N = 100
DEF_BENCH( return (new StackBench<SmallTrivialT, StdVectorAdapter>(         "std::vector<smallT>",           BenchMode::kAllocateOnly, 100));)
DEF_BENCH( return (new StackBench<SmallTrivialT, StdVectorAdapter>(         "std::vector<smallT>",           BenchMode::kPushPop,      100));)
DEF_BENCH( return (new StackBench<SmallTrivialT, StdVectorAdapter>(         "std::vector<smallT>",           BenchMode::kConcat,       100));)
DEF_BENCH( return (new StackBench<SmallTrivialT, StdVectorAdapter>(         "std::vector<smallT>",           BenchMode::kRandomAccess, 100));)
DEF_BENCH( return (new StackBench<SmallTrivialT, SkTArrayAdapter>(          "SkTArray<smallT>",              BenchMode::kAllocateOnly, 100));)
DEF_BENCH( return (new StackBench<SmallTrivialT, SkTArrayAdapter>(          "SkTArray<smallT>",              BenchMode::kPushPop,      100));)
DEF_BENCH( return (new StackBench<SmallTrivialT, SkTArrayAdapter>(          "SkTArray<smallT>",              BenchMode::kConcat,       100));)
DEF_BENCH( return (new StackBench<SmallTrivialT, SkTArrayAdapter>(          "SkTArray<smallT>",              BenchMode::kRandomAccess, 100));)
DEF_BENCH( return (new StackBench<SmallTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallT>",    BenchMode::kAllocateOnly, 100));)
DEF_BENCH( return (new StackBench<SmallTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallT>",    BenchMode::kPushPop,      100));)
DEF_BENCH( return (new StackBench<SmallTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallT>",    BenchMode::kConcat,       100));)
DEF_BENCH( return (new StackBench<SmallTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallT>",    BenchMode::kRandomAccess, 100));)

DEF_BENCH( return (new StackBench<SmallComplexT, StdVectorAdapter>(         "std::vector<smallC>",           BenchMode::kAllocateOnly, 100));)
DEF_BENCH( return (new StackBench<SmallComplexT, StdVectorAdapter>(         "std::vector<smallC>",           BenchMode::kPushPop,      100));)
DEF_BENCH( return (new StackBench<SmallComplexT, StdVectorAdapter>(         "std::vector<smallC>",           BenchMode::kConcat,       100));)
DEF_BENCH( return (new StackBench<SmallComplexT, StdVectorAdapter>(         "std::vector<smallC>",           BenchMode::kRandomAccess, 100));)
DEF_BENCH( return (new StackBench<SmallComplexT, SkTArrayAdapter>(          "SkTArray<smallC>",              BenchMode::kAllocateOnly, 100));)
DEF_BENCH( return (new StackBench<SmallComplexT, SkTArrayAdapter>(          "SkTArray<smallC>",              BenchMode::kPushPop,      100));)
DEF_BENCH( return (new StackBench<SmallComplexT, SkTArrayAdapter>(          "SkTArray<smallC>",              BenchMode::kConcat,       100));)
DEF_BENCH( return (new StackBench<SmallComplexT, SkTArrayAdapter>(          "SkTArray<smallC>",              BenchMode::kRandomAccess, 100));)
DEF_BENCH( return (new StackBench<SmallComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallC>",    BenchMode::kAllocateOnly, 100));)
DEF_BENCH( return (new StackBench<SmallComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallC>",    BenchMode::kPushPop,      100));)
DEF_BENCH( return (new StackBench<SmallComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallC>",    BenchMode::kConcat,       100));)
DEF_BENCH( return (new StackBench<SmallComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallC>",    BenchMode::kRandomAccess, 100));)

DEF_BENCH( return (new StackBench<LargeTrivialT, StdVectorAdapter>(         "std::vector<bigT>",             BenchMode::kAllocateOnly, 100));)
DEF_BENCH( return (new StackBench<LargeTrivialT, StdVectorAdapter>(         "std::vector<bigT>",             BenchMode::kPushPop,      100));)
DEF_BENCH( return (new StackBench<LargeTrivialT, StdVectorAdapter>(         "std::vector<bigT>",             BenchMode::kConcat,       100));)
DEF_BENCH( return (new StackBench<LargeTrivialT, StdVectorAdapter>(         "std::vector<bigT>",             BenchMode::kRandomAccess, 100));)
DEF_BENCH( return (new StackBench<LargeTrivialT, SkTArrayAdapter>(          "SkTArray<bigT>",                BenchMode::kAllocateOnly, 100));)
DEF_BENCH( return (new StackBench<LargeTrivialT, SkTArrayAdapter>(          "SkTArray<bigT>",                BenchMode::kPushPop,      100));)
DEF_BENCH( return (new StackBench<LargeTrivialT, SkTArrayAdapter>(          "SkTArray<bigT>",                BenchMode::kConcat,       100));)
DEF_BENCH( return (new StackBench<LargeTrivialT, SkTArrayAdapter>(          "SkTArray<bigT>",                BenchMode::kRandomAccess, 100));)
DEF_BENCH( return (new StackBench<LargeTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigT>",      BenchMode::kAllocateOnly, 100));)
DEF_BENCH( return (new StackBench<LargeTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigT>",      BenchMode::kPushPop,      100));)
DEF_BENCH( return (new StackBench<LargeTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigT>",      BenchMode::kConcat,       100));)
DEF_BENCH( return (new StackBench<LargeTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigT>",      BenchMode::kRandomAccess, 100));)

DEF_BENCH( return (new StackBench<LargeComplexT, StdVectorAdapter>(         "std::vector<bigC>",             BenchMode::kAllocateOnly, 100));)
DEF_BENCH( return (new StackBench<LargeComplexT, StdVectorAdapter>(         "std::vector<bigC>",             BenchMode::kPushPop,      100));)
DEF_BENCH( return (new StackBench<LargeComplexT, StdVectorAdapter>(         "std::vector<bigC>",             BenchMode::kConcat,       100));)
DEF_BENCH( return (new StackBench<LargeComplexT, StdVectorAdapter>(         "std::vector<bigC>",             BenchMode::kRandomAccess, 100));)
DEF_BENCH( return (new StackBench<LargeComplexT, SkTArrayAdapter>(          "SkTArray<bigC>",                BenchMode::kAllocateOnly, 100));)
DEF_BENCH( return (new StackBench<LargeComplexT, SkTArrayAdapter>(          "SkTArray<bigC>",                BenchMode::kPushPop,      100));)
DEF_BENCH( return (new StackBench<LargeComplexT, SkTArrayAdapter>(          "SkTArray<bigC>",                BenchMode::kConcat,       100));)
DEF_BENCH( return (new StackBench<LargeComplexT, SkTArrayAdapter>(          "SkTArray<bigC>",                BenchMode::kRandomAccess, 100));)
DEF_BENCH( return (new StackBench<LargeComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigC>",      BenchMode::kAllocateOnly, 100));)
DEF_BENCH( return (new StackBench<LargeComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigC>",      BenchMode::kPushPop,      100));)
DEF_BENCH( return (new StackBench<LargeComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigC>",      BenchMode::kConcat,       100));)
DEF_BENCH( return (new StackBench<LargeComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigC>",      BenchMode::kRandomAccess, 100));)

// N = 1000
DEF_BENCH( return (new StackBench<SmallTrivialT, StdVectorAdapter>(         "std::vector<smallT>",           BenchMode::kAllocateOnly, 1000));)
DEF_BENCH( return (new StackBench<SmallTrivialT, StdVectorAdapter>(         "std::vector<smallT>",           BenchMode::kPushPop,      1000));)
DEF_BENCH( return (new StackBench<SmallTrivialT, StdVectorAdapter>(         "std::vector<smallT>",           BenchMode::kConcat,       1000));)
DEF_BENCH( return (new StackBench<SmallTrivialT, StdVectorAdapter>(         "std::vector<smallT>",           BenchMode::kRandomAccess, 1000));)
DEF_BENCH( return (new StackBench<SmallTrivialT, SkTArrayAdapter>(          "SkTArray<smallT>",              BenchMode::kAllocateOnly, 1000));)
DEF_BENCH( return (new StackBench<SmallTrivialT, SkTArrayAdapter>(          "SkTArray<smallT>",              BenchMode::kPushPop,      1000));)
DEF_BENCH( return (new StackBench<SmallTrivialT, SkTArrayAdapter>(          "SkTArray<smallT>",              BenchMode::kConcat,       1000));)
DEF_BENCH( return (new StackBench<SmallTrivialT, SkTArrayAdapter>(          "SkTArray<smallT>",              BenchMode::kRandomAccess, 1000));)
DEF_BENCH( return (new StackBench<SmallTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallT>",    BenchMode::kAllocateOnly, 1000));)
DEF_BENCH( return (new StackBench<SmallTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallT>",    BenchMode::kPushPop,      1000));)
DEF_BENCH( return (new StackBench<SmallTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallT>",    BenchMode::kConcat,       1000));)
DEF_BENCH( return (new StackBench<SmallTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallT>",    BenchMode::kRandomAccess, 1000));)

DEF_BENCH( return (new StackBench<SmallComplexT, StdVectorAdapter>(         "std::vector<smallC>",           BenchMode::kAllocateOnly, 1000));)
DEF_BENCH( return (new StackBench<SmallComplexT, StdVectorAdapter>(         "std::vector<smallC>",           BenchMode::kPushPop,      1000));)
DEF_BENCH( return (new StackBench<SmallComplexT, StdVectorAdapter>(         "std::vector<smallC>",           BenchMode::kConcat,       1000));)
DEF_BENCH( return (new StackBench<SmallComplexT, StdVectorAdapter>(         "std::vector<smallC>",           BenchMode::kRandomAccess, 1000));)
DEF_BENCH( return (new StackBench<SmallComplexT, SkTArrayAdapter>(          "SkTArray<smallC>",              BenchMode::kAllocateOnly, 1000));)
DEF_BENCH( return (new StackBench<SmallComplexT, SkTArrayAdapter>(          "SkTArray<smallC>",              BenchMode::kPushPop,      1000));)
DEF_BENCH( return (new StackBench<SmallComplexT, SkTArrayAdapter>(          "SkTArray<smallC>",              BenchMode::kConcat,       1000));)
DEF_BENCH( return (new StackBench<SmallComplexT, SkTArrayAdapter>(          "SkTArray<smallC>",              BenchMode::kRandomAccess, 1000));)
DEF_BENCH( return (new StackBench<SmallComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallC>",    BenchMode::kAllocateOnly, 1000));)
DEF_BENCH( return (new StackBench<SmallComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallC>",    BenchMode::kPushPop,      1000));)
DEF_BENCH( return (new StackBench<SmallComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallC>",    BenchMode::kConcat,       1000));)
DEF_BENCH( return (new StackBench<SmallComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallC>",    BenchMode::kRandomAccess, 1000));)

DEF_BENCH( return (new StackBench<LargeTrivialT, StdVectorAdapter>(         "std::vector<bigT>",             BenchMode::kAllocateOnly, 1000));)
DEF_BENCH( return (new StackBench<LargeTrivialT, StdVectorAdapter>(         "std::vector<bigT>",             BenchMode::kPushPop,      1000));)
DEF_BENCH( return (new StackBench<LargeTrivialT, StdVectorAdapter>(         "std::vector<bigT>",             BenchMode::kConcat,       1000));)
DEF_BENCH( return (new StackBench<LargeTrivialT, StdVectorAdapter>(         "std::vector<bigT>",             BenchMode::kRandomAccess, 1000));)
DEF_BENCH( return (new StackBench<LargeTrivialT, SkTArrayAdapter>(          "SkTArray<bigT>",                BenchMode::kAllocateOnly, 1000));)
DEF_BENCH( return (new StackBench<LargeTrivialT, SkTArrayAdapter>(          "SkTArray<bigT>",                BenchMode::kPushPop,      1000));)
DEF_BENCH( return (new StackBench<LargeTrivialT, SkTArrayAdapter>(          "SkTArray<bigT>",                BenchMode::kConcat,       1000));)
DEF_BENCH( return (new StackBench<LargeTrivialT, SkTArrayAdapter>(          "SkTArray<bigT>",                BenchMode::kRandomAccess, 1000));)
DEF_BENCH( return (new StackBench<LargeTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigT>",      BenchMode::kAllocateOnly, 1000));)
DEF_BENCH( return (new StackBench<LargeTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigT>",      BenchMode::kPushPop,      1000));)
DEF_BENCH( return (new StackBench<LargeTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigT>",      BenchMode::kConcat,       1000));)
DEF_BENCH( return (new StackBench<LargeTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigT>",      BenchMode::kRandomAccess, 1000));)

DEF_BENCH( return (new StackBench<LargeComplexT, StdVectorAdapter>(         "std::vector<bigC>",             BenchMode::kAllocateOnly, 1000));)
DEF_BENCH( return (new StackBench<LargeComplexT, StdVectorAdapter>(         "std::vector<bigC>",             BenchMode::kPushPop,      1000));)
DEF_BENCH( return (new StackBench<LargeComplexT, StdVectorAdapter>(         "std::vector<bigC>",             BenchMode::kConcat,       1000));)
DEF_BENCH( return (new StackBench<LargeComplexT, StdVectorAdapter>(         "std::vector<bigC>",             BenchMode::kRandomAccess, 1000));)
DEF_BENCH( return (new StackBench<LargeComplexT, SkTArrayAdapter>(          "SkTArray<bigC>",                BenchMode::kAllocateOnly, 1000));)
DEF_BENCH( return (new StackBench<LargeComplexT, SkTArrayAdapter>(          "SkTArray<bigC>",                BenchMode::kPushPop,      1000));)
DEF_BENCH( return (new StackBench<LargeComplexT, SkTArrayAdapter>(          "SkTArray<bigC>",                BenchMode::kConcat,       1000));)
DEF_BENCH( return (new StackBench<LargeComplexT, SkTArrayAdapter>(          "SkTArray<bigC>",                BenchMode::kRandomAccess, 1000));)
DEF_BENCH( return (new StackBench<LargeComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigC>",      BenchMode::kAllocateOnly, 1000));)
DEF_BENCH( return (new StackBench<LargeComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigC>",      BenchMode::kPushPop,      1000));)
DEF_BENCH( return (new StackBench<LargeComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigC>",      BenchMode::kConcat,       1000));)
DEF_BENCH( return (new StackBench<LargeComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigC>",      BenchMode::kRandomAccess, 1000));)

// N = 10000
DEF_BENCH( return (new StackBench<SmallTrivialT, StdVectorAdapter>(         "std::vector<smallT>",           BenchMode::kAllocateOnly, 10000));)
DEF_BENCH( return (new StackBench<SmallTrivialT, StdVectorAdapter>(         "std::vector<smallT>",           BenchMode::kPushPop,      10000));)
DEF_BENCH( return (new StackBench<SmallTrivialT, StdVectorAdapter>(         "std::vector<smallT>",           BenchMode::kConcat,       10000));)
DEF_BENCH( return (new StackBench<SmallTrivialT, StdVectorAdapter>(         "std::vector<smallT>",           BenchMode::kRandomAccess, 10000));)
DEF_BENCH( return (new StackBench<SmallTrivialT, SkTArrayAdapter>(          "SkTArray<smallT>",              BenchMode::kAllocateOnly, 10000));)
DEF_BENCH( return (new StackBench<SmallTrivialT, SkTArrayAdapter>(          "SkTArray<smallT>",              BenchMode::kPushPop,      10000));)
DEF_BENCH( return (new StackBench<SmallTrivialT, SkTArrayAdapter>(          "SkTArray<smallT>",              BenchMode::kConcat,       10000));)
DEF_BENCH( return (new StackBench<SmallTrivialT, SkTArrayAdapter>(          "SkTArray<smallT>",              BenchMode::kRandomAccess, 10000));)
DEF_BENCH( return (new StackBench<SmallTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallT>",    BenchMode::kAllocateOnly, 10000));)
DEF_BENCH( return (new StackBench<SmallTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallT>",    BenchMode::kPushPop,      10000));)
DEF_BENCH( return (new StackBench<SmallTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallT>",    BenchMode::kConcat,       10000));)
DEF_BENCH( return (new StackBench<SmallTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallT>",    BenchMode::kRandomAccess, 10000));)

DEF_BENCH( return (new StackBench<SmallComplexT, StdVectorAdapter>(         "std::vector<smallC>",           BenchMode::kAllocateOnly, 10000));)
DEF_BENCH( return (new StackBench<SmallComplexT, StdVectorAdapter>(         "std::vector<smallC>",           BenchMode::kPushPop,      10000));)
DEF_BENCH( return (new StackBench<SmallComplexT, StdVectorAdapter>(         "std::vector<smallC>",           BenchMode::kConcat,       10000));)
DEF_BENCH( return (new StackBench<SmallComplexT, StdVectorAdapter>(         "std::vector<smallC>",           BenchMode::kRandomAccess, 10000));)
DEF_BENCH( return (new StackBench<SmallComplexT, SkTArrayAdapter>(          "SkTArray<smallC>",              BenchMode::kAllocateOnly, 10000));)
DEF_BENCH( return (new StackBench<SmallComplexT, SkTArrayAdapter>(          "SkTArray<smallC>",              BenchMode::kPushPop,      10000));)
DEF_BENCH( return (new StackBench<SmallComplexT, SkTArrayAdapter>(          "SkTArray<smallC>",              BenchMode::kConcat,       10000));)
DEF_BENCH( return (new StackBench<SmallComplexT, SkTArrayAdapter>(          "SkTArray<smallC>",              BenchMode::kRandomAccess, 10000));)
DEF_BENCH( return (new StackBench<SmallComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallC>",    BenchMode::kAllocateOnly, 10000));)
DEF_BENCH( return (new StackBench<SmallComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallC>",    BenchMode::kPushPop,      10000));)
DEF_BENCH( return (new StackBench<SmallComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallC>",    BenchMode::kConcat,       10000));)
DEF_BENCH( return (new StackBench<SmallComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<smallC>",    BenchMode::kRandomAccess, 10000));)

DEF_BENCH( return (new StackBench<LargeTrivialT, StdVectorAdapter>(         "std::vector<bigT>",             BenchMode::kAllocateOnly, 10000));)
DEF_BENCH( return (new StackBench<LargeTrivialT, StdVectorAdapter>(         "std::vector<bigT>",             BenchMode::kPushPop,      10000));)
DEF_BENCH( return (new StackBench<LargeTrivialT, StdVectorAdapter>(         "std::vector<bigT>",             BenchMode::kConcat,       10000));)
DEF_BENCH( return (new StackBench<LargeTrivialT, StdVectorAdapter>(         "std::vector<bigT>",             BenchMode::kRandomAccess, 10000));)
DEF_BENCH( return (new StackBench<LargeTrivialT, SkTArrayAdapter>(          "SkTArray<bigT>",                BenchMode::kAllocateOnly, 10000));)
DEF_BENCH( return (new StackBench<LargeTrivialT, SkTArrayAdapter>(          "SkTArray<bigT>",                BenchMode::kPushPop,      10000));)
DEF_BENCH( return (new StackBench<LargeTrivialT, SkTArrayAdapter>(          "SkTArray<bigT>",                BenchMode::kConcat,       10000));)
DEF_BENCH( return (new StackBench<LargeTrivialT, SkTArrayAdapter>(          "SkTArray<bigT>",                BenchMode::kRandomAccess, 10000));)
DEF_BENCH( return (new StackBench<LargeTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigT>",      BenchMode::kAllocateOnly, 10000));)
DEF_BENCH( return (new StackBench<LargeTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigT>",      BenchMode::kPushPop,      10000));)
DEF_BENCH( return (new StackBench<LargeTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigT>",      BenchMode::kConcat,       10000));)
DEF_BENCH( return (new StackBench<LargeTrivialT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigT>",      BenchMode::kRandomAccess, 10000));)

DEF_BENCH( return (new StackBench<LargeComplexT, StdVectorAdapter>(         "std::vector<bigC>",             BenchMode::kAllocateOnly, 10000));)
DEF_BENCH( return (new StackBench<LargeComplexT, StdVectorAdapter>(         "std::vector<bigC>",             BenchMode::kPushPop,      10000));)
DEF_BENCH( return (new StackBench<LargeComplexT, StdVectorAdapter>(         "std::vector<bigC>",             BenchMode::kConcat,       10000));)
DEF_BENCH( return (new StackBench<LargeComplexT, StdVectorAdapter>(         "std::vector<bigC>",             BenchMode::kRandomAccess, 10000));)
DEF_BENCH( return (new StackBench<LargeComplexT, SkTArrayAdapter>(          "SkTArray<bigC>",                BenchMode::kAllocateOnly, 10000));)
DEF_BENCH( return (new StackBench<LargeComplexT, SkTArrayAdapter>(          "SkTArray<bigC>",                BenchMode::kPushPop,      10000));)
DEF_BENCH( return (new StackBench<LargeComplexT, SkTArrayAdapter>(          "SkTArray<bigC>",                BenchMode::kConcat,       10000));)
DEF_BENCH( return (new StackBench<LargeComplexT, SkTArrayAdapter>(          "SkTArray<bigC>",                BenchMode::kRandomAccess, 10000));)
DEF_BENCH( return (new StackBench<LargeComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigC>",      BenchMode::kAllocateOnly, 10000));)
DEF_BENCH( return (new StackBench<LargeComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigC>",      BenchMode::kPushPop,      10000));)
DEF_BENCH( return (new StackBench<LargeComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigC>",      BenchMode::kConcat,       10000));)
DEF_BENCH( return (new StackBench<LargeComplexT, GrTBlockLinkedListAdapter>("GrTBlockLinkedList<bigC>",      BenchMode::kRandomAccess, 10000));)
