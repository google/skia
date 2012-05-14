/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#include "Test.h"

#include "SkRefCnt.h"
#include "SkThreadUtils.h"

///////////////////////////////////////////////////////////////////////////////

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

    thing1.setProcessorAffinity(0);
    thing2.setProcessorAffinity(23);

    SkASSERT(thing1.start());
    SkASSERT(thing2.start());

    thing1.join();
    thing2.join();

    REPORTER_ASSERT(reporter, ref->getRefCnt() == 1);
    ref->unref();
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("ref_cnt", RefCntTestClass, test_refCnt)
