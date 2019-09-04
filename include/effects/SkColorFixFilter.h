/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorFixFilter_DEFINED
#define SkColorFixFilter_DEFINED

#include "include/core/SkColorFilter.h"
#include "include/core/SkPaint.h"

/**
 *  Configuration struct for SkColorFixFilter.
 */
struct SkColorFixConfig {
    SkColorFixConfig() {}
    SkColorFixConfig(float R, float G) : fR(R), fG(G) {}
    bool isValid() const { return true; }
    float fR, fG;
};

class SK_API SkColorFixFilter {
public:
    // Returns the filter, or nullptr if the config is invalid.
    static sk_sp<SkColorFilter> Make(const SkColorFixConfig& config);

    static void RegisterFlattenables();
};

#endif
