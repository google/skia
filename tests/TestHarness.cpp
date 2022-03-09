/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/TestHarness.h"

bool CurrentTestHarnessIsSkQP() {
    return CurrentTestHarness() == TestHarness::kSkQP;
}
