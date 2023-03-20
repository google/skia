/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkTraceEventCommon.h"

#ifdef SK_ANDROID_FRAMEWORK_USE_PERFETTO
// This line cannot be placed in a header.
PERFETTO_TRACK_EVENT_STATIC_STORAGE();
#endif
