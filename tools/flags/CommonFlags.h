/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SK_COMMON_FLAGS_H
#define SK_COMMON_FLAGS_H

#include "../private/SkTArray.h"
#include "CommandLineFlags.h"
#include "SkString.h"

DECLARE_bool(cpu);
DECLARE_bool(dryRun);
DECLARE_bool(gpu);
DECLARE_string(images);
DECLARE_bool(simpleCodec);
DECLARE_string(match);
DECLARE_bool(quiet);
DECLARE_string(skps);
DECLARE_string(lotties);
DECLARE_string(svgs);
DECLARE_bool(nativeFonts);
DECLARE_int(threads);
DECLARE_string(resourcePath);
DECLARE_bool(verbose);
DECLARE_bool(veryVerbose);
DECLARE_string(writePath);
DECLARE_bool(analyticAA);
DECLARE_bool(forceAnalyticAA);
DECLARE_string(key);
DECLARE_string(properties);

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
 *     --noGS
 *     --pr
 *     --disableDriverCorrectnessWorkarounds
 *     --reduceOpListSplitting
 */
void SetCtxOptionsFromCommonFlags(struct GrContextOptions*);

#endif
