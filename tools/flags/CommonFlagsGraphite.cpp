/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/flags/CommonFlagsGraphite.h"
#include "tools/graphite/TestOptions.h"

namespace CommonFlags {

static DEFINE_bool(neverYieldToWebGPU, false, "Run Graphite with never-yield context option.");
static DEFINE_bool(useWGPUTextureView, false, "Run Graphite w/ a wrapped WGPU texture view as "
                                              "the destination");

void SetTestOptions(skiatest::graphite::TestOptions* testOptions) {
    testOptions->fNeverYieldToWebGPU = FLAGS_neverYieldToWebGPU;
    testOptions->fUseWGPUTextureView = FLAGS_useWGPUTextureView;
}

}  // namespace CommonFlags
