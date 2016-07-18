/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SK_COMMON_FLAGS_H
#define SK_COMMON_FLAGS_H

#include "../private/SkTArray.h"
#include "SkCommandLineFlags.h"
#include "SkString.h"

DECLARE_bool(cpu);
DECLARE_bool(dryRun);
DECLARE_bool(gpu);
DECLARE_string(images);
DECLARE_string(colorImages);
DECLARE_bool(simpleCodec);
DECLARE_string(match);
DECLARE_bool(quiet);
DECLARE_bool(resetGpuContext);
DECLARE_bool(preAbandonGpuContext);
DECLARE_bool(abandonGpuContext);
DECLARE_bool(releaseAndAbandonGpuContext);
DECLARE_string(skps);
DECLARE_int32(threads);
DECLARE_string(resourcePath);
DECLARE_bool(verbose);
DECLARE_bool(veryVerbose);
DECLARE_string(writePath);
DECLARE_bool(pre_log);

DECLARE_string(key);
DECLARE_string(properties);

/**
 *  Helper to assist in collecting image paths from |dir| specified through a command line flag.
 *
 *  Populates |output|, an array of strings with paths to images to test.
 *
 *  Returns true if each argument to the images flag is meaningful:
 *  - If the file/directory does not exist, return false.
 *  - If |dir| does not have any supported images (based on file type), return false.
 *  - If |dir| is a single file, assume the user is deliberately testing this image,
 *    regardless of file type.
 */
bool CollectImages(SkCommandLineFlags::StringArray dir, SkTArray<SkString>* output);

#endif
