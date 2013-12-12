/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkRTConf.h"

DEF_TEST(UnitTest, reporter) {
#ifdef SK_SUPPORT_UNITTEST
    SkRTConfRegistry::UnitTest();
#endif
}
