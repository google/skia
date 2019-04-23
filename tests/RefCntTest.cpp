/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/SkWeakRefCnt.h"
#include "tests/Test.h"

#include <thread>

static void bounce_ref(void* data) {
    SkRefCnt* ref = static_cast<SkRefCnt*>(data);
    for (int i = 0; i < 100000; ++i) {
        ref->ref();
        ref->unref();
    }
}

static void test_refCnt(skiatest::Reporter* reporter) {
    SkRefCnt* ref = new SkRefCnt();

    std::thread thing1(bounce_ref, ref);
    std::thread thing2(bounce_ref, ref);

    thing1.join();
    thing2.join();

    REPORTER_ASSERT(reporter, ref->unique());
    ref->unref();
}

static void bounce_weak_ref(void* data) {
    SkWeakRefCnt* ref = static_cast<SkWeakRefCnt*>(data);
    for (int i = 0; i < 100000; ++i) {
        if (ref->try_ref()) {
            ref->unref();
        }
    }
}

static void bounce_weak_weak_ref(void* data) {
    SkWeakRefCnt* ref = static_cast<SkWeakRefCnt*>(data);
    for (int i = 0; i < 100000; ++i) {
        ref->weak_ref();
        ref->weak_unref();
    }
}

static void test_weakRefCnt(skiatest::Reporter* reporter) {
    SkWeakRefCnt* ref = new SkWeakRefCnt();

    std::thread thing1(bounce_ref, ref);
    std::thread thing2(bounce_ref, ref);
    std::thread thing3(bounce_weak_ref, ref);
    std::thread thing4(bounce_weak_weak_ref, ref);

    thing1.join();
    thing2.join();
    thing3.join();
    thing4.join();

    REPORTER_ASSERT(reporter, ref->unique());
    SkDEBUGCODE(REPORTER_ASSERT(reporter, ref->getWeakCnt() == 1));
    ref->unref();
}

DEF_TEST(RefCnt, reporter) {
    test_refCnt(reporter);
    test_weakRefCnt(reporter);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static int gRefCounter;
static int gUnrefCounter;
static int gNewCounter;
static int gDeleteCounter;

#define check(reporter, ref, unref, make, kill)        \
    REPORTER_ASSERT(reporter, gRefCounter == ref);     \
    REPORTER_ASSERT(reporter, gUnrefCounter == unref); \
    REPORTER_ASSERT(reporter, gNewCounter == make);    \
    REPORTER_ASSERT(reporter, gDeleteCounter == kill)

class Effect {
public:
    Effect() : fRefCnt(1) {
        gNewCounter += 1;
    }
    virtual ~Effect() {}

    int fRefCnt;

    void ref() {
        gRefCounter += 1;
        fRefCnt += 1;
    }
    void unref() {
        gUnrefCounter += 1;

        SkASSERT(fRefCnt > 0);
        if (0 == --fRefCnt) {
            gDeleteCounter += 1;
            delete this;
        }
    }

    int* method() const { return new int; }
};

static sk_sp<Effect> Create() {
    return sk_make_sp<Effect>();
}

class Paint {
public:
    sk_sp<Effect> fEffect;

    const sk_sp<Effect>& get() const { return fEffect; }

    void set(sk_sp<Effect> value) {
        fEffect = std::move(value);
    }
};

struct EffectImpl : public Effect {
    ~EffectImpl() override {}

    static sk_sp<EffectImpl> Create() {
        return sk_sp<EffectImpl>(new EffectImpl);
    }
    int fValue;
};
static sk_sp<Effect> make_effect() {
    auto foo = EffectImpl::Create();
    foo->fValue = 42;
    return std::move(foo);
}

static void reset_counters() {
    gRefCounter = 0;
    gUnrefCounter = 0;
    gNewCounter = 0;
    gDeleteCounter = 0;
}
DEF_TEST(sk_sp, reporter) {
    reset_counters();

    Paint paint;
    REPORTER_ASSERT(reporter, paint.fEffect.get() == nullptr);
    REPORTER_ASSERT(reporter, !paint.get());
    check(reporter, 0, 0, 0, 0);

    paint.set(Create());
    check(reporter, 0, 0, 1, 0);
    REPORTER_ASSERT(reporter, paint.fEffect.get()->fRefCnt == 1);

    if (paint.get()) {
        REPORTER_ASSERT(reporter, true);
    } else {
        REPORTER_ASSERT(reporter, false);
    }
    if (!paint.get()) {
        REPORTER_ASSERT(reporter, false);
    } else {
        REPORTER_ASSERT(reporter, true);
    }

    paint.set(nullptr);
    check(reporter, 0, 1, 1, 1);

    if (paint.get()) {
        REPORTER_ASSERT(reporter, false);
    } else {
        REPORTER_ASSERT(reporter, true);
    }
    if (!paint.get()) {
        REPORTER_ASSERT(reporter, true);
    } else {
        REPORTER_ASSERT(reporter, false);
    }

    auto e = Create();
    REPORTER_ASSERT(reporter, sizeof(e) == sizeof(void*));

    check(reporter, 0, 1, 2, 1);
    paint.set(e);
    check(reporter, 1, 1, 2, 1);
    REPORTER_ASSERT(reporter, paint.fEffect.get()->fRefCnt == 2);

    Paint paint2;
    paint2.set(paint.get());
    check(reporter, 2, 1, 2, 1);
    REPORTER_ASSERT(reporter, paint.fEffect.get()->fRefCnt == 3);

    // Test sk_sp::operator->
    delete paint.get()->method();
    check(reporter, 2, 1, 2, 1);

    // Test sk_sp::operator*
    delete (*paint.get()).method();
    check(reporter, 2, 1, 2, 1);

    paint.set(nullptr);
    e = nullptr;
    paint2.set(nullptr);
    check(reporter, 2, 4, 2, 2);

    reset_counters();
    {
        // Test convertible sk_sp assignment.
        check(reporter, 0, 0, 0, 0);
        sk_sp<Effect> foo(nullptr);
        REPORTER_ASSERT(reporter, !foo);
        foo = make_effect();
        REPORTER_ASSERT(reporter, foo);
        check(reporter, 0, 0, 1, 0);
    }
    check(reporter, 0, 1, 1, 1);

    // Test passing convertible rvalue into funtion.
    reset_counters();
    paint.set(EffectImpl::Create());
    check(reporter, 0, 0, 1, 0);
    paint.set(nullptr);
    check(reporter, 0, 1, 1, 1);

    reset_counters();
    auto baz = EffectImpl::Create();
    check(reporter, 0, 0, 1, 0);
    paint.set(std::move(baz));
    check(reporter, 0, 0, 1, 0);
    REPORTER_ASSERT(reporter, !baz);  // NOLINT(bugprone-use-after-move)
    paint.set(nullptr);
    check(reporter, 0, 1, 1, 1);

    reset_counters();
    {
        // test comparison operator with convertible type.
        sk_sp<EffectImpl> bar1 = EffectImpl::Create();
        sk_sp<Effect> bar2(bar1);  // convertible copy constructor
        check(reporter, 1, 0, 1, 0);
        REPORTER_ASSERT(reporter, bar1);
        REPORTER_ASSERT(reporter, bar2);
        REPORTER_ASSERT(reporter, bar1 == bar2);
        REPORTER_ASSERT(reporter, bar2 == bar1);
        REPORTER_ASSERT(reporter, !(bar1 != bar2));
        REPORTER_ASSERT(reporter, !(bar2 != bar1));
        sk_sp<Effect> bar3(nullptr);
        bar3 = bar1;  // convertible copy assignment
        check(reporter, 2, 0, 1, 0);

    }
    check(reporter, 2, 3, 1, 1);

    // test passing convertible copy into funtion.
    reset_counters();
    baz = EffectImpl::Create();
    check(reporter, 0, 0, 1, 0);
    paint.set(baz);
    check(reporter, 1, 0, 1, 0);
    baz = nullptr;
    check(reporter, 1, 1, 1, 0);
    paint.set(nullptr);
    check(reporter, 1, 2, 1, 1);

    {
        sk_sp<SkRefCnt> empty;
        sk_sp<SkRefCnt> notEmpty = sk_make_sp<SkRefCnt>();
        REPORTER_ASSERT(reporter, empty == sk_sp<SkRefCnt>());

        REPORTER_ASSERT(reporter, notEmpty != empty);
        REPORTER_ASSERT(reporter, empty != notEmpty);

        REPORTER_ASSERT(reporter, nullptr == empty);
        REPORTER_ASSERT(reporter, empty == nullptr);
        REPORTER_ASSERT(reporter, empty == empty);

        REPORTER_ASSERT(reporter, nullptr <= empty);
        REPORTER_ASSERT(reporter, empty <= nullptr);
        REPORTER_ASSERT(reporter, empty <= empty);

        REPORTER_ASSERT(reporter, nullptr >= empty);
        REPORTER_ASSERT(reporter, empty >= nullptr);
        REPORTER_ASSERT(reporter, empty >= empty);
    }

    {
        sk_sp<SkRefCnt> a = sk_make_sp<SkRefCnt>();
        sk_sp<SkRefCnt> b = sk_make_sp<SkRefCnt>();
        REPORTER_ASSERT(reporter, a != b);
        REPORTER_ASSERT(reporter, (a < b) != (b < a));
        REPORTER_ASSERT(reporter, (b > a) != (a > b));
        REPORTER_ASSERT(reporter, (a <= b) != (b <= a));
        REPORTER_ASSERT(reporter, (b >= a) != (a >= b));

        REPORTER_ASSERT(reporter, a == a);
        REPORTER_ASSERT(reporter, a <= a);
        REPORTER_ASSERT(reporter, a >= a);
    }

    // http://wg21.cmeerw.net/lwg/issue998
    {
        class foo : public SkRefCnt {
        public:
            foo() : bar(this) {}
            void reset() { bar.reset(); }
        private:
            sk_sp<foo> bar;
        };
        // The following should properly delete the object and not cause undefined behavior.
        // This is an ugly example, but the same issue can arise in more subtle ways.
        (new foo)->reset();
    }

    // https://crrev.com/0d4ef2583a6f19c3e61be04d36eb1a60b133832c
    {
        struct StructB;
        struct StructA : public SkRefCnt {
            sk_sp<StructB> b;
        };

        struct StructB : public SkRefCnt {
            sk_sp<StructA> a;
            ~StructB() override {} // Some clang versions don't emit this implicitly.
        };

        // Create a reference cycle.
        StructA* a = new StructA;
        a->b.reset(new StructB);
        a->b->a.reset(a);

        // Break the cycle by calling reset(). This will cause |a| (and hence, |a.b|)
        // to be deleted before the call to reset() returns. This tests that the
        // implementation of sk_sp::reset() doesn't access |this| after it
        // deletes the underlying pointer. This behaviour is consistent with the
        // definition of unique_ptr::reset in C++11.
        a->b.reset();
    }
}

namespace {
struct FooAbstract : public SkRefCnt {
    virtual void f() = 0;
};
struct FooConcrete : public FooAbstract {
    void f() override {}
};
}
static sk_sp<FooAbstract> make_foo() {
    // can not cast FooConcrete to FooAbstract.
    // can cast FooConcrete* to FooAbstract*.
    return sk_make_sp<FooConcrete>();
}
DEF_TEST(sk_make_sp, r) {
    auto x = make_foo();
}

// Test that reset() "adopts" ownership from the caller, even if we are given the same ptr twice
//
DEF_TEST(sk_sp_reset, r) {
    SkRefCnt* rc = new SkRefCnt;
    REPORTER_ASSERT(r, rc->unique());

    sk_sp<SkRefCnt> sp;
    sp.reset(rc);
    // We have transfered our ownership over to sp
    REPORTER_ASSERT(r, rc->unique());

    rc->ref();  // now "rc" is also an owner
    REPORTER_ASSERT(r, !rc->unique());

    sp.reset(rc);   // this should transfer our ownership over to sp
    REPORTER_ASSERT(r, rc->unique());
}

DEF_TEST(sk_sp_ref, r) {
    SkRefCnt* rc = new SkRefCnt;
    REPORTER_ASSERT(r, rc->unique());

    {
        sk_sp<SkRefCnt> sp = sk_ref_sp(rc);
        REPORTER_ASSERT(r, !rc->unique());
    }

    REPORTER_ASSERT(r, rc->unique());
    rc->unref();
}
