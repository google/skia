/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SK_COMMON_FLAGS_H
#define SK_COMMON_FLAGS_H

#include "SkCommandLineFlags.h"

DECLARE_string(config);
DECLARE_bool(cpu);
DECLARE_bool(dryRun);
DECLARE_bool(gpu);
DECLARE_string(gpuAPI);
DECLARE_string(images);
DECLARE_string(match);
DECLARE_bool(quiet);
DECLARE_bool(resetGpuContext);
DECLARE_bool(preAbandonGpuContext);
DECLARE_bool(abandonGpuContext);
DECLARE_string(skps);
DECLARE_int32(threads);
DECLARE_string(resourcePath);
DECLARE_bool(verbose);
DECLARE_bool(veryVerbose);
DECLARE_string(writePath);

DECLARE_string(key);
DECLARE_string(properties);

#endif
