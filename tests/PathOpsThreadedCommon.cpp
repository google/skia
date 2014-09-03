/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "PathOpsExtendedTest.h"
#include "PathOpsThreadedCommon.h"
#include "SkTaskGroup.h"

PathOpsThreadedTestRunner::~PathOpsThreadedTestRunner() {
    for (int index = 0; index < fRunnables.count(); index++) {
        SkDELETE(fRunnables[index]);
    }
}

void PathOpsThreadedTestRunner::render() {
    SkTaskGroup tg;
    for (int index = 0; index < fRunnables.count(); ++ index) {
        tg.add(fRunnables[index]);
    }
}
