/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/CtsEnforcement.h"

CtsEnforcement::RunMode CtsEnforcement::eval(int apiLevel) const {
    if (apiLevel >= fStrictVersion) {
        return RunMode::kRunStrict;
    } else if (apiLevel >= fWorkaroundsVersion) {
        return RunMode::kRunWithWorkarounds;
    }
    return RunMode::kSkip;
}
