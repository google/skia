/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkRecorder_DEFINED
#define SkRecorder_DEFINED

#include "include/private/base/SkAPI.h"

class SK_API SkRecorder {
public:
    virtual ~SkRecorder() = default;
    enum class Type {
        kRaster,
        kGanesh,
        kGraphite,
    };

    virtual Type type() const = 0;
};

#endif
