/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DebugUtils_DEFINED
#define DebugUtils_DEFINED

#include "include/private/base/SkDebug.h"

#if defined(SK_DEBUG) && defined(SK_DUMP_TASKS)
    #define SK_DUMP_TASKS_CODE(...)  __VA_ARGS__
#else
    #define SK_DUMP_TASKS_CODE(...)
#endif

#endif // DebugUtils_DEFINED
