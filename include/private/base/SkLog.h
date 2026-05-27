/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLog_DEFINED
#define SkLog_DEFINED

#include <cstdarg>

#include "include/private/base/SkAPI.h"
#include "include/private/base/SkAttributes.h"
#include "include/private/base/SkLoadUserConfig.h" // IWYU pragma: keep
#include "include/private/base/SkLogPriority.h"

#if !defined(SkLog)
// To be implemented per platform.
void SkLogVAList(SkLogPriority priority, const char format[], va_list args) SK_PRINTF_LIKE(2, 0);
void SK_SPI SkLog(SkLogPriority priority, const char format[], ...) SK_PRINTF_LIKE(2, 3);
#endif

/**
 * TODO (b/469441457): SKGPU_GRAPHITE_LOWEST_ACTIVE_LOG_PRIORITY Was the config used to set the log
 * priority, so we check for that define as well. Eventually, we should move clients using this
 * define to the new one.
 */
#if defined(SKGPU_GRAPHITE_LOWEST_ACTIVE_LOG_PRIORITY)
    static constexpr SkLogPriority MapGraphitePriority(skgpu::graphite::LogPriority priority) {
        switch (priority) {
            case skgpu::graphite::LogPriority::kError:   return SkLogPriority::kError;
            case skgpu::graphite::LogPriority::kWarning: return SkLogPriority::kWarning;
            case skgpu::graphite::LogPriority::kInfo:    return SkLogPriority::kInfo;
            case skgpu::graphite::LogPriority::kDebug:   return SkLogPriority::kDebug;
            default: return SkLogPriority::kDebug;
        }
    }
    #define SKIA_LOWEST_ACTIVE_LOG_PRIORITY MapGraphitePriority(SKGPU_GRAPHITE_LOWEST_ACTIVE_LOG_PRIORITY)
#endif

#if !defined(SKIA_LOWEST_ACTIVE_LOG_PRIORITY)
#ifdef SK_DEBUG
    #define SKIA_LOWEST_ACTIVE_LOG_PRIORITY SkLogPriority::kDebug
#else
    #define SKIA_LOWEST_ACTIVE_LOG_PRIORITY SkLogPriority::kInfo
#endif
#endif

#define SKIA_LOG(priority, fmt, ...)                                           \
    do {                                                                       \
        if constexpr (priority <= SKIA_LOWEST_ACTIVE_LOG_PRIORITY) {           \
            if constexpr (priority == SkLogPriority::kFatal) {                 \
                SK_ABORT("[skia] " fmt, ##__VA_ARGS__);                        \
            }                                                                  \
            else {                                                             \
                SkLog(priority, "[skia] " fmt "\n", ##__VA_ARGS__);            \
            }                                                                  \
        }                                                                      \
    } while (0)

#define SKIA_LOG_F(fmt, ...) SKIA_LOG(SkLogPriority::kFatal, "** ERROR ** " fmt, ##__VA_ARGS__)
#define SKIA_LOG_E(fmt, ...) SKIA_LOG(SkLogPriority::kError, "** ERROR ** " fmt, ##__VA_ARGS__)
#define SKIA_LOG_W(fmt, ...) SKIA_LOG(SkLogPriority::kWarning, "WARNING - " fmt, ##__VA_ARGS__)
#define SKIA_LOG_I(fmt, ...) SKIA_LOG(SkLogPriority::kInfo, fmt, ##__VA_ARGS__)
#define SKIA_LOG_D(fmt, ...) SKIA_LOG(SkLogPriority::kDebug, fmt, ##__VA_ARGS__)

#endif // SkLog_DEFINED
