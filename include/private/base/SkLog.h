/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLog_DEFINED
#define SkLog_DEFINED

#include "include/private/base/SkLogPriority.h"

// Ensure SkUserConfig.h is loaded, as clients may define SKIA_LOWEST_ACTIVE_LOG_PRIORITY
#include "include/core/SkTypes.h" // IWYU pragma: keep

/**
 * TODO (b/469441457): SKGPU_GRAPHITE_LOWEST_ACTIVE_LOG_PRIORITY Was the config used to set the log
 * priority, so we check for that define as well. Eventually, we should move clients using this
 * define to the new one.
 */
 #if defined (SKGPU_GRAPHITE_LOWEST_ACTIVE_LOG_PRIORITY)
   #define SKIA_LOWEST_ACTIVE_LOG_PRIORITY SKGPU_GRAPHITE_LOWEST_ACTIVE_LOG_PRIORITY
#endif

#if !defined(SKIA_LOWEST_ACTIVE_LOG_PRIORITY)
#ifdef SK_DEBUG
    #define SKIA_LOWEST_ACTIVE_LOG_PRIORITY SkLogPriority::kWarning
#else
    #define SKIA_LOWEST_ACTIVE_LOG_PRIORITY SkLogPriority::kError
#endif
#endif

#define SKIA_LOG(priority, fmt, ...)                                           \
    do {                                                                       \
        if (priority <= SKIA_LOWEST_ACTIVE_LOG_PRIORITY) {                     \
            if (priority == SkLogPriority::kFatal) {                           \
                SK_ABORT("[skia] " fmt, ##__VA_ARGS__);                        \
            }                                                                  \
            else {                                                             \
                /* TODO: SkLog(priority, "[skia] " fmt "\n", ##__VA_ARGS__); */\
            }                                                                  \
        }                                                                      \
    } while (0)

#define SKIA_LOG_F(fmt, ...) SKIA_LOG(SkLogPriority::kFatal, "** ERROR ** " fmt, ##__VA_ARGS__)
#define SKIA_LOG_E(fmt, ...) SKIA_LOG(SkLogPriority::kError, "** ERROR ** " fmt, ##__VA_ARGS__)
#define SKIA_LOG_W(fmt, ...) SKIA_LOG(SkLogPriority::kWarning, "WARNING - " fmt, ##__VA_ARGS__)
#define SKIA_LOG_D(fmt, ...) SKIA_LOG(SkLogPriority::kDebug, fmt, ##__VA_ARGS__)

#endif // SkLog_DEFINED
