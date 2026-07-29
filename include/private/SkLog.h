/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLog_DEFINED
#define SkLog_DEFINED

#include <cstdarg>

#include "include/private/SkAPI.h"
#include "include/private/SkAttributes.h"
#include "include/private/SkLoadUserConfig.h" // IWYU pragma: keep
#include "include/private/SkLogPriority.h"

#if !defined(SkLog)
// Implemented per platform.
void SkLogVAList(SkLogPriority priority, const char format[], va_list args) SK_PRINTF_LIKE(2, 0);
void SK_SPI SkLog(SkLogPriority priority, const char format[], ...) SK_PRINTF_LIKE(2, 3);
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
            SkLog(priority, "[skia] " fmt "\n", ##__VA_ARGS__);                \
        }                                                                      \
    } while (0)

#define SKIA_LOG_E(fmt, ...) SKIA_LOG(SkLogPriority::kError, "** ERROR ** " fmt, ##__VA_ARGS__)
#define SKIA_LOG_W(fmt, ...) SKIA_LOG(SkLogPriority::kWarning, "WARNING - " fmt, ##__VA_ARGS__)
#define SKIA_LOG_I(fmt, ...) SKIA_LOG(SkLogPriority::kInfo, fmt, ##__VA_ARGS__)
#define SKIA_LOG_D(fmt, ...) SKIA_LOG(SkLogPriority::kDebug, fmt, ##__VA_ARGS__)

#endif // SkLog_DEFINED
