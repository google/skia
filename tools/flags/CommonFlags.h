/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#pragma once

#include "include/core/SkString.h"
#include "include/private/SkTArray.h"
#include "tools/flags/CommandLineFlags.h"

/**
 *  Helper to assist in collecting image paths from |dir| specified through a command line
 * flag.
 *
 *  Populates |output|, an array of strings with paths to images to test.
 *
 *  Returns true if each argument to the images flag is meaningful:
 *  - If the file/directory does not exist, return false.
 *  - If |dir| does not have any supported images (based on file type), return false.
 *  - If |dir| is a single file, assume the user is deliberately testing this image,
 *    regardless of file type.
 */
bool CollectImages(CommandLineFlags::StringArray dir, SkTArray<SkString>* output);

/**
 *  Helper to set GrContextOptions from common GPU flags, including
 *     --gpuThreads
 *     --cachePathMasks
 *     --allPathsVolatile
 *     --(no)gs
 *     --(no)ts
 *     --maxTessellationSegments
 *     --pr
 *     --internalSamples
 *     --disableDriverCorrectnessWorkarounds
 *     --reduceOpsTaskSplitting
 *     --dontReduceOpsTaskSplitting
 */
void SetCtxOptionsFromCommonFlags(struct GrContextOptions*);

/**
 *  Enable, disable, or force analytic anti-aliasing using --analyticAA and --forceAnalyticAA.
 */
void SetAnalyticAAFromCommonFlags();
