/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRTConf.h"
#include "Test.h"

DEF_TEST(UnitTest, reporter) {
#ifdef SK_SUPPORT_UNITTEST
    SkRTConfRegistry::UnitTest();
#endif
}
