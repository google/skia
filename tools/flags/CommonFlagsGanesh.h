/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#pragma once

#include "tools/flags/CommandLineFlags.h"

struct GrContextOptions;

namespace CommonFlags {
/**
 *  Helper to set GrContextOptions from common GPU flags, including
 *     --gpuThreads
 *     --cachePathMasks
 *     --allPathsVolatile
 *     --(no)gs
 *     --(no)ts
 *     --pr
 *     --internalSamples
 *     --disableDriverCorrectnessWorkarounds
 *     --reduceOpsTaskSplitting
 *     --dontReduceOpsTaskSplitting
 *     --allowMSAAOnNewIntel
 */
void SetCtxOptions(GrContextOptions*);

}  // namespace CommonFlags
