/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Log_DEFINED
#define skgpu_graphite_Log_DEFINED

#include "include/private/base/SkLog.h"

#define SKGPU_LOG(priority, fmt, ...) \
    SKIA_LOG((SkLogPriority)priority, "[graphite] " fmt, ##__VA_ARGS__)

#define SKGPU_LOG_F(fmt, ...) SKIA_LOG(SkLogPriority::kFatal,   "[graphite] " fmt, ##__VA_ARGS__)
#define SKGPU_LOG_E(fmt, ...) SKIA_LOG(SkLogPriority::kError,   "[graphite] " fmt, ##__VA_ARGS__)
#define SKGPU_LOG_W(fmt, ...) SKIA_LOG(SkLogPriority::kWarning, "[graphite] " fmt, ##__VA_ARGS__)
#define SKGPU_LOG_D(fmt, ...) SKIA_LOG(SkLogPriority::kDebug,   "[graphite] " fmt, ##__VA_ARGS__)

#endif // skgpu_graphite_Log_DEFINED
