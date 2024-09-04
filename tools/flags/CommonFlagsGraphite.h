/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#pragma once

#include "tools/flags/CommandLineFlags.h"

namespace skiatest::graphite {
    struct TestOptions;
}

namespace CommonFlags {
/**
 *  Helper to set TestOptions from common GPU flags, including:
 *     --neverYieldToWebGPU
 *     --useTintIR
 *     --useWGPUTextureView
 */
void SetTestOptions(skiatest::graphite::TestOptions*);

}  // namespace CommonFlags
