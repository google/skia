/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_LogPriority_DEFINED
#define skgpu_graphite_LogPriority_DEFINED

/**
 * Note: this file may be included in clients' SkUserConfig.h files, so including any other headers
 * in this file should be avoided.
 */

namespace skgpu::graphite {
/**
 * SKGPU_GRAPHITE_LOWEST_ACTIVE_LOG_PRIORITY can be defined to one of these values (in
 * SkUserConfig.h) to control Graphite's logging behavior.
 *
 * For example:
 * ```
 * #define SKGPU_GRAPHITE_LOWEST_ACTIVE_LOG_PRIORITY skgpu::graphite::LogPriority::kWarning
 * ```
 * Would cause Graphite to log warnings, non-fatal errors, and fatal errors.
 * However, debug logs would be omitted.
 */
enum class LogPriority : int {
    kFatal = 0,
    kError = 1,
    kWarning = 2,
    kDebug = 3,
};
};  // namespace skgpu::graphite

#endif // skgpu_graphite_LogPriority_DEFINED
