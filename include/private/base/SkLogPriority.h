/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLogPriority_DEFINED
#define SkLogPriority_DEFINED

/**
 * Note: this file may be included in clients' SkUserConfig.h files, so including any other headers
 * in this file should be avoided.
 */

/**
 * SKIA_LOWEST_ACTIVE_LOG_PRIORITY can be defined to one of these values (in
 * SkUserConfig.h) to control Skia's logging behavior.
 *
 * For example:
 * ```
 * #define SKIA_LOWEST_ACTIVE_LOG_PRIORITY SkLogPriority::kWarning
 * ```
 * Would cause Skia to log warnings, non-fatal errors, and fatal errors.
 * However, debug logs would be omitted.
 */
enum class SkLogPriority : int {
    kFatal = 0,
    kError = 1,
    kWarning = 2,
    kInfo = 3,
    kDebug = 4,
};

#endif // SkLogPriority_DEFINED
