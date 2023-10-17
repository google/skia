/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#pragma once

#include "include/core/SkString.h"
#include "include/private/base/SkTArray.h"
#include "tools/flags/CommandLineFlags.h"

namespace skgpu::graphite {
struct ContextOptions;
};

namespace CommonFlags {
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
bool CollectImages(const CommandLineFlags::StringArray& dir,
                   skia_private::TArray<SkString>* output);
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
void SetCtxOptions(struct GrContextOptions*);

/**
 *  Enable, disable, or force analytic anti-aliasing using --analyticAA and --forceAnalyticAA.
 */
void SetAnalyticAA();

/**
 *  Turn on portable (--nonativeFonts) or GDI font rendering (--gdi).
 */
void SetDefaultFontMgr();

}  // namespace CommonFlags
