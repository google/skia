/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkTArray.h"
#include "include/private/SkTDArray.h"
#include "include/private/SkTemplates.h"
#include "include/utils/SkRandom.h"
#include "src/gpu/GrMemoryPool.h"
#include "tests/Test.h"

// A is the top of an inheritance tree of classes that overload op new and
// and delete to use a GrMemoryPool. The objects have values of different types
// that can be set and checked.
class A {
public:
    A() {}
    virtual void setValues(int v) {
        fChar = static_cast<char>(v & 0xFF);
    }
    virtual bool checkValues(int v) {
        return fChar == static_cast<char>(v & 0xFF);
    }
    virtual ~A() {}

    void* operator new(size_t size) {
        if (!gPool.get()) {
            return ::operator new(size);
        } else {
            return gPool->allocate(size);
        }
    }

    void operator delete(void* p) {
        if (!gPool.get()) {
            ::operator delete(p);
        } else {
            return gPool->release(p);
        }
    }

    static A* Create(SkRandom* r);

    static void SetAllocator(size_t preallocSize, size_t minAllocSize) {
        GrMemoryPool* pool = new GrMemoryPool(preallocSize, minAllocSize);
        gPool.reset(pool);
    }

    static void ResetAllocator() {
        gPool.reset(nullptr);
    }

private:
    static std::unique_ptr<GrMemoryPool> gPool;
    char fChar;
};

std::unique_ptr<GrMemoryPool> A::gPool;

class B : public A {
public:
    B() {}
    virtual void setValues(int v) {
        fDouble = static_cast<double>(v);
        this->INHERITED::setValues(v);
    }
    virtual bool checkValues(int v) {
        return fDouble == static_cast<double>(v) &&
               this->INHERITED::checkValues(v);
    }
    virtual ~B() {}

private:
    double fDouble;

    typedef A INHERITED;
};

class C : public A {
public:
    C() {}
    virtual void setValues(int v) {
        fInt64 = static_cast<int64_t>(v);
        this->INHERITED::setValues(v);
    }
    virtual bool checkValues(int v) {
        return fInt64 == static_cast<int64_t>(v) &&
               this->INHERITED::checkValues(v);
    }
    virtual ~C() {}

private:
    int64_t fInt64;

    typedef A INHERITED;
};

// D derives from C and owns a dynamically created B
class D : public C {
public:
    D() {
        fB = new B();
    }
    virtual void setValues(int v) {
        fVoidStar = reinterpret_cast<void*>(static_cast<intptr_t>(v));
        this->INHERITED::setValues(v);
        fB->setValues(v);
    }
    virtual bool checkValues(int v) {
        return fVoidStar == reinterpret_cast<void*>(static_cast<intptr_t>(v)) &&
               fB->checkValues(v) &&
               this->INHERITED::checkValues(v);
    }
    virtual ~D() {
        delete fB;
    }
private:
    void*   fVoidStar;
    B*      fB;

    typedef C INHERITED;
};

class E : public A {
public:
    E() {}
    virtual void setValues(int v) {
        for (size_t i = 0; i < SK_ARRAY_COUNT(fIntArray); ++i) {
            fIntArray[i] = v;
        }
        this->INHERITED::setValues(v);
    }
    virtual bool checkValues(int v) {
        bool ok = true;
        for (size_t i = 0; ok && i < SK_ARRAY_COUNT(fIntArray); ++i) {
            if (fIntArray[i] != v) {
                ok = false;
            }
        }
        return ok && this->INHERITED::checkValues(v);
    }
    virtual ~E() {}
private:
    int   fIntArray[20];

    typedef A INHERITED;
};

A* A::Create(SkRandom* r) {
    switch (r->nextRangeU(0, 4)) {
        case 0:
            return new A;
        case 1:
            return new B;
        case 2:
            return new C;
        case 3:
            return new D;
        case 4:
            return new E;
        default:
            // suppress warning
            return nullptr;
    }
}

struct Rec {
    A* fInstance;
    int fValue;
};

DEF_TEST(GrMemoryPool, reporter) {
    // prealloc and min alloc sizes for the pool
    static const size_t gSizes[][2] = {
        {0, 0},
        {10 * sizeof(A), 20 * sizeof(A)},
        {100 * sizeof(A), 100 * sizeof(A)},
        {500 * sizeof(A), 500 * sizeof(A)},
        {10000 * sizeof(A), 0},
        {1, 100 * sizeof(A)},
    };
    // different percentages of creation vs deletion
    static const float gCreateFraction[] = {1.f, .95f, 0.75f, .5f};
    // number of create/destroys per test
    static const int kNumIters = 20000;
    // check that all the values stored in A objects are correct after this
    // number of iterations
    static const int kCheckPeriod = 500;

    SkRandom r;
    for (size_t s = 0; s < SK_ARRAY_COUNT(gSizes); ++s) {
        A::SetAllocator(gSizes[s][0], gSizes[s][1]);
        for (size_t c = 0; c < SK_ARRAY_COUNT(gCreateFraction); ++c) {
            SkTDArray<Rec> instanceRecs;
            for (int i = 0; i < kNumIters; ++i) {
                float createOrDestroy = r.nextUScalar1();
                if (createOrDestroy < gCreateFraction[c] ||
                    0 == instanceRecs.count()) {
                    Rec* rec = instanceRecs.append();
                    rec->fInstance = A::Create(&r);
                    rec->fValue = static_cast<int>(r.nextU());
                    rec->fInstance->setValues(rec->fValue);
                } else {
                    int d = r.nextRangeU(0, instanceRecs.count() - 1);
                    Rec& rec = instanceRecs[d];
                    REPORTER_ASSERT(reporter, rec.fInstance->checkValues(rec.fValue));
                    delete rec.fInstance;
                    instanceRecs.removeShuffle(d);
                }
                if (0 == i % kCheckPeriod) {
                    for (int r = 0; r < instanceRecs.count(); ++r) {
                        Rec& rec = instanceRecs[r];
                        REPORTER_ASSERT(reporter, rec.fInstance->checkValues(rec.fValue));
                    }
                }
            }
            for (int i = 0; i < instanceRecs.count(); ++i) {
                Rec& rec = instanceRecs[i];
                REPORTER_ASSERT(reporter, rec.fInstance->checkValues(rec.fValue));
                delete rec.fInstance;
            }
        }
    }
}

// GrMemoryPool requires that it's empty at the point of destruction. This helps
// achieving that by releasing all added memory in the destructor.
class AutoPoolReleaser {
public:
    AutoPoolReleaser(GrMemoryPool& pool): fPool(pool) {
    }
    ~AutoPoolReleaser() {
        for (void* ptr: fAllocated) {
            fPool.release(ptr);
        }
    }
    void add(void* ptr) {
        fAllocated.push_back(ptr);
    }
private:
    GrMemoryPool& fPool;
    SkTArray<void*> fAllocated;
};

DEF_TEST(GrMemoryPoolAPI, reporter) {
    constexpr size_t kSmallestMinAllocSize = GrMemoryPool::kSmallestMinAllocSize;

    // Allocates memory until pool adds a new block (pool.size() changes).
    auto allocateMemory = [](GrMemoryPool& pool, AutoPoolReleaser& r) {
        size_t origPoolSize = pool.size();
        while (pool.size() == origPoolSize) {
            r.add(pool.allocate(31));
        }
    };

    // Effective prealloc space capacity is >= kSmallestMinAllocSize.
    {
        GrMemoryPool pool(0, 0);
        REPORTER_ASSERT(reporter, pool.preallocSize() == kSmallestMinAllocSize);
    }

    // Effective prealloc space capacity is >= minAllocSize.
    {
        constexpr size_t kMinAllocSize = kSmallestMinAllocSize * 2;
        GrMemoryPool pool(kSmallestMinAllocSize, kMinAllocSize);
        REPORTER_ASSERT(reporter, pool.preallocSize() == kMinAllocSize);
    }

    // Effective block size capacity >= kSmallestMinAllocSize.
    {
        GrMemoryPool pool(kSmallestMinAllocSize, kSmallestMinAllocSize / 2);
        AutoPoolReleaser r(pool);

        allocateMemory(pool, r);
        REPORTER_ASSERT(reporter, pool.size() == kSmallestMinAllocSize);
    }

    // Pool allocates exactly preallocSize on creation.
    {
        constexpr size_t kPreallocSize = kSmallestMinAllocSize * 5;
        GrMemoryPool pool(kPreallocSize, 0);
        REPORTER_ASSERT(reporter, pool.preallocSize() == kPreallocSize);
    }

    // Pool allocates exactly minAllocSize when it expands.
    {
        constexpr size_t kMinAllocSize = kSmallestMinAllocSize * 7;
        GrMemoryPool pool(0, kMinAllocSize);
        AutoPoolReleaser r(pool);

        allocateMemory(pool, r);
        REPORTER_ASSERT(reporter, pool.size() == kMinAllocSize);

        allocateMemory(pool, r);
        REPORTER_ASSERT(reporter, pool.size() == 2 * kMinAllocSize);
    }

    // When asked to allocate amount > minAllocSize, pool allocates larger block
    // to accommodate all internal structures.
    {
        constexpr size_t kMinAllocSize = kSmallestMinAllocSize * 2;
        GrMemoryPool pool(kSmallestMinAllocSize, kMinAllocSize);
        AutoPoolReleaser r(pool);

        REPORTER_ASSERT(reporter, pool.size() == 0);

        constexpr size_t hugeSize = 10 * kMinAllocSize;
        r.add(pool.allocate(hugeSize));
        REPORTER_ASSERT(reporter, pool.size() > hugeSize);

        // Block size allocated to accommodate huge request doesn't include any extra
        // space, so next allocation request allocates a new block.
        size_t hugeBlockSize = pool.size();
        r.add(pool.allocate(0));
        REPORTER_ASSERT(reporter, pool.size() == hugeBlockSize + kMinAllocSize);
    }
}
