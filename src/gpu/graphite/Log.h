/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Log_DEFINED
#define skgpu_graphite_Log_DEFINED

namespace skgpu::graphite {
enum class Priority : int {
    kFatal = 0,
    kError = 1,
    kWarning = 2,
    kDebug = 3,
};
};  // namespace skgpu::graphite

#if !defined(SKGPU_LOWEST_ACTIVE_PRIORITY)
#ifdef SK_DEBUG
    #define SKGPU_LOWEST_ACTIVE_PRIORITY skgpu::graphite::Priority::kWarning
#else
    #define SKGPU_LOWEST_ACTIVE_PRIORITY skgpu::graphite::Priority::kError
#endif
#endif
#define SKGPU_LOG(priority, fmt, ...) \
    do { \
        if (priority <= SKGPU_LOWEST_ACTIVE_PRIORITY) { \
            SkDebugf("[graphite] " fmt "\n", ##__VA_ARGS__); \
            if (priority == skgpu::graphite::Priority::kFatal) { \
                SK_ABORT("Fatal log call"); \
            } \
        } \
    } while (0)
#define SKGPU_LOG_F(fmt, ...) SKGPU_LOG(skgpu::graphite::Priority::kFatal, "** ERROR ** " fmt, \
                                        ##__VA_ARGS__)
#define SKGPU_LOG_E(fmt, ...) SKGPU_LOG(skgpu::graphite::Priority::kError, "** ERROR ** " fmt, \
                                        ##__VA_ARGS__)
#define SKGPU_LOG_W(fmt, ...) SKGPU_LOG(skgpu::graphite::Priority::kWarning, "WARNING - " fmt, \
                                        ##__VA_ARGS__)
#define SKGPU_LOG_D(fmt, ...) SKGPU_LOG(skgpu::graphite::Priority::kDebug, fmt, \
                                        ##__VA_ARGS__)

#endif // skgpu_graphite_Log_DEFINED
