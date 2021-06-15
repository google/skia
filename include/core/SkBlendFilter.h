/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlendFilter_DEFINED
#define SkBlendFilter_DEFINED

#include "include/core/SkFlattenable.h"

/**
 * A BlendFilter represents a custom blend function in the Skia pipeline. When a blend filter
 * present in a paint, the SkBlendMode is ignored. A blend filter combines a source color (the
 * result of our paint) and destination color (from the canvas) into a final color.
 */
class SK_API SkBlendFilter : public SkFlattenable {
private:
    SkBlendFilter() = default;
    friend class SkBlendFilterBase;

    using INHERITED = SkFlattenable;
};

#endif
