/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Fuzz.h"
#include "FuzzCommon.h"
#include "SkPath.h"
#include "SkPathOps.h"

const int kLastOp = SkPathOp::kReverseDifference_SkPathOp;

DEF_FUZZ(Pathop, fuzz) {
    SkOpBuilder builder;

    uint8_t stragglerOp;
    fuzz->next(&stragglerOp);
    SkPath path;

    BuildPath(fuzz, &path, SkPath::Verb::kDone_Verb);
    builder.add(path, static_cast<SkPathOp>(stragglerOp % (kLastOp + 1)));

    SkPath result;
    builder.resolve(&result);
}
