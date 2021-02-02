/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DEFINES
#define SKSL_DEFINES

#include <cstdint>

#include "include/core/SkTypes.h"

#if defined(SK_BUILD_FOR_IOS) && \
        (!defined(__IPHONE_9_0) || __IPHONE_OS_VERSION_MIN_REQUIRED < __IPHONE_9_0)
#define SKSL_USE_THREAD_LOCAL 0
#else
#define SKSL_USE_THREAD_LOCAL 1
#endif

using SKSL_INT = int64_t;
using SKSL_FLOAT = float;

#endif
