/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlender_DEFINED
#define SkBlender_DEFINED

#include "include/core/SkFlattenable.h"

class SkRuntimeEffect;

/**
 * SkBlender represents a custom blend function in the Skia pipeline. When an SkBlender is
 * present in a paint, the SkBlendMode is ignored. A blender combines a source color (the
 * result of our paint) and destination color (from the canvas) into a final color.
 */
class SK_API SkBlender : public SkFlattenable {
public:
    static SkFlattenable::Type GetFlattenableType() { return kSkBlender_Type; }
    Type getFlattenableType() const override { return GetFlattenableType(); }

    virtual SkRuntimeEffect* asRuntimeEffect() const { return nullptr; }

private:
    SkBlender() = default;
    friend class SkBlenderBase;

    using INHERITED = SkFlattenable;
};

#endif
