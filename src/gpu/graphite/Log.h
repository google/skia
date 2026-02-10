/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Log_DEFINED
#define skgpu_graphite_Log_DEFINED

#include "include/gpu/graphite/LogPriority.h"

// Ensure SkUserConfig.h is loaded, as clients may define SKGPU_GRAPHITE_LOWEST_ACTIVE_LOG_PRIORITY
#include "include/core/SkTypes.h" // IWYU pragma: keep

#if !defined(SKGPU_GRAPHITE_LOWEST_ACTIVE_LOG_PRIORITY)
#ifdef SK_DEBUG
    #define SKGPU_GRAPHITE_LOWEST_ACTIVE_LOG_PRIORITY skgpu::graphite::LogPriority::kWarning
#else
    #define SKGPU_GRAPHITE_LOWEST_ACTIVE_LOG_PRIORITY skgpu::graphite::LogPriority::kError
#endif
#endif

#define SKGPU_LOG(priority, fmt, ...)                                           \
    do {                                                                        \
        if (priority <= SKGPU_GRAPHITE_LOWEST_ACTIVE_LOG_PRIORITY) {            \
            if (priority == skgpu::graphite::LogPriority::kFatal) {             \
                SK_ABORT("[graphite] " fmt, ##__VA_ARGS__);                     \
            }                                                                   \
            else {                                                              \
                SkDebugf("[graphite] " fmt "\n", ##__VA_ARGS__);                \
            }                                                                   \
        }                                                                       \
    } while (0)

#define SKGPU_LOG_F(fmt, ...) SKGPU_LOG(skgpu::graphite::LogPriority::kFatal, "** ERROR ** " fmt, \
                                        ##__VA_ARGS__)
#define SKGPU_LOG_E(fmt, ...) SKGPU_LOG(skgpu::graphite::LogPriority::kError, "** ERROR ** " fmt, \
                                        ##__VA_ARGS__)
#define SKGPU_LOG_W(fmt, ...) SKGPU_LOG(skgpu::graphite::LogPriority::kWarning, "WARNING - " fmt, \
                                        ##__VA_ARGS__)
#define SKGPU_LOG_D(fmt, ...) SKGPU_LOG(skgpu::graphite::LogPriority::kDebug, fmt, \
                                        ##__VA_ARGS__)

#endif // skgpu_graphite_Log_DEFINED
