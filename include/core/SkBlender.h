/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlender_DEFINED
#define SkBlender_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkFlattenable.h"

/**
 * SkBlender represents a blend function in the Skia pipeline. This can represent either a
 * traditional built-in blend (SkBlendMode) or a user-defined blend function. A blender combines a
 * source color (the result of our paint) and destination color (from the canvas) into a final
 * color.
 */
class SK_API SkBlender : public SkFlattenable {
public:
    /** Returns true if this SkBlender represents any SkBlendMode. */
    bool isBlendMode() const {
        SkBlendMode mode;
        return this->asBlendMode(&mode);
    }

    /** Returns true if this SkBlender does NOT represent any SkBlendMode. */
    bool isCustomBlend() const {
        return !this->isBlendMode();
    }

    /** Returns true if this SkBlender matches the passed-in SkBlendMode. */
    bool isBlendMode(SkBlendMode expected) const {
        SkBlendMode mode;
        return this->asBlendMode(&mode) && (mode == expected);
    }

    /** Returns true if this SkBlender represents any coefficient-based SkBlendMode. */
    bool isCoefficient() const {
        return this->asCoefficient(/*src=*/nullptr, /*dst=*/nullptr);
    }

    /**
     * For a SkBlendMode-based Porter-Duff blend, retrieves its coefficients and returns true.
     * Returns false for other types of blends.
     */
    bool asCoefficient(SkBlendModeCoeff* src, SkBlendModeCoeff* dst) const {
        SkBlendMode mode;
        return this->asBlendMode(&mode) && SkBlendMode_AsCoeff(mode, src, dst);
    }

    /**
     * Returns true if this SkBlender represents any SkBlendMode, and returns the blender's
     * SkBlendMode in `mode`. Returns false for other types of blends.
     */
    virtual bool asBlendMode([[maybe_unused]] SkBlendMode* mode) const {
        return false;
    }

private:
    SkBlender() = default;
    friend class SkBlenderBase;

    using INHERITED = SkFlattenable;
};

/**
 * Factory functions for synthesizing an SkBlender.
 */
class SK_API SkBlenders {
public:
    /** Returns a SkBlender for the requested SkBlendMode. */
    static sk_sp<SkBlender> Mode(SkBlendMode mode);

private:
    SkBlenders() = delete;
};

#endif
