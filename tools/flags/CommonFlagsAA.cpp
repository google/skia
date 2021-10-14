// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "src/core/SkScan.h"
#include "tools/flags/CommonFlags.h"

namespace CommonFlags {

static DEFINE_bool(analyticAA, true, "If false, disable analytic anti-aliasing");
static DEFINE_bool(forceAnalyticAA, false,
            "Force analytic anti-aliasing even if the path is complicated: "
            "whether it's concave or convex, we consider a path complicated"
            "if its number of points is comparable to its resolution.");

void SetAnalyticAA() {
    gSkUseAnalyticAA   = FLAGS_analyticAA;
    gSkForceAnalyticAA = FLAGS_forceAnalyticAA;
}

}
