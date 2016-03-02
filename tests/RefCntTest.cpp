/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRefCnt.h"
#include "SkThreadUtils.h"
#include "SkTypes.h"
#include "SkWeakRefCnt.h"
#include "Test.h"

static void bounce_ref(void* data) {
    SkRefCnt* ref = static_cast<SkRefCnt*>(data);
    for (int i = 0; i < 100000; ++i) {
        ref->ref();
        ref->unref();
    }
}

static void test_refCnt(skiatest::Reporter* reporter) {
    SkRefCnt* ref = new SkRefCnt();

    SkThread thing1(bounce_ref, ref);
    SkThread thing2(bounce_ref, ref);

    SkASSERT(thing1.start());
    SkASSERT(thing2.start());

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

    SkThread thing1(bounce_ref, ref);
    SkThread thing2(bounce_ref, ref);
    SkThread thing3(bounce_weak_ref, ref);
    SkThread thing4(bounce_weak_weak_ref, ref);

    SkASSERT(thing1.start());
    SkASSERT(thing2.start());
    SkASSERT(thing3.start());
    SkASSERT(thing4.start());

    thing1.join();
    thing2.join();
    thing3.join();
    thing4.join();

    REPORTER_ASSERT(reporter, ref->unique());
    REPORTER_ASSERT(reporter, ref->getWeakCnt() == 1);
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

#define check(reporter, ref, unref, make, kill)             \
    REPORTER_ASSERT(reporter, gRefCounter == ref);          \
    REPORTER_ASSERT(reporter, gUnrefCounter == unref);      \
    REPORTER_ASSERT(reporter, gNewCounter == make);         \
    REPORTER_ASSERT(reporter, gDeleteCounter == kill);


class Effect {
public:
    Effect() : fRefCnt(1) {
        gNewCounter += 1;
    }

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

DEF_TEST(sk_sp, reporter) {
    gRefCounter = 0;
    gUnrefCounter = 0;
    gNewCounter = 0;
    gDeleteCounter = 0;

    Paint paint;
    REPORTER_ASSERT(reporter, paint.fEffect.get() == nullptr);
    REPORTER_ASSERT(reporter, !paint.get());
    check(reporter, 0, 0, 0, 0);

    paint.set(Create());
    check(reporter, 0, 0, 1, 0);
    REPORTER_ASSERT(reporter, paint.fEffect.get()->fRefCnt == 1);

    paint.set(nullptr);
    check(reporter, 0, 1, 1, 1);

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

    delete paint.get()->method();
    check(reporter, 2, 1, 2, 1);
}

