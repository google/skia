/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkRTConf.h"
#include "Test.h"
#include "TestClassDef.h"

SK_CONF_DECLARE(int, c_RTConfTestVariable,
                "test.utils.rtconf.testVariable", 1,
                "This is only a test. Do not be alarmed.");
// TODO(skia-team): more comprehensive unit tests of the SkRTConf
// system.
DEF_TEST(RuntimeConfig, reporter) {
    REPORTER_ASSERT(reporter, 1 == c_RTConfTestVariable);

    SK_CONF_SET("test.utils.rtconf.testVariable", 2);
#ifdef SK_DEVELOPER
    REPORTER_ASSERT(reporter, 2 == c_RTConfTestVariable);
#else  // not SK_DEVELOPER
    // Can not change RTConf variables in Release.
    REPORTER_ASSERT(reporter, 1 == c_RTConfTestVariable);
#endif  // SK_DEVELOPER

    // This should not give a warning.
    SK_CONF_TRY_SET("test.utils.rtconf.nonexistentVariable", 7);
}
