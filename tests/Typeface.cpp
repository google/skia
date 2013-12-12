/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkTypeface.h"

DEF_TEST(Typeface, reporter) {

    SkAutoTUnref<SkTypeface> t1(SkTypeface::CreateFromName(NULL, SkTypeface::kNormal));
    SkAutoTUnref<SkTypeface> t2(SkTypeface::RefDefault(SkTypeface::kNormal));

    REPORTER_ASSERT(reporter, SkTypeface::Equal(t1.get(), t2.get()));
    REPORTER_ASSERT(reporter, SkTypeface::Equal(0, t1.get()));
    REPORTER_ASSERT(reporter, SkTypeface::Equal(0, t2.get()));
    REPORTER_ASSERT(reporter, SkTypeface::Equal(t1.get(), 0));
    REPORTER_ASSERT(reporter, SkTypeface::Equal(t2.get(), 0));

#ifdef SK_BUILD_FOR_ANDROID
    SkAutoTUnref<SkTypeface> t3(SkTypeface::CreateFromName("non-existent-font", SkTypeface::kNormal));
    REPORTER_ASSERT(reporter, NULL == t3.get());
#endif
}
