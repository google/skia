/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTrimImpl_DEFINED
#define SkTrimImpl_DEFINED

#include "include/effects/SkTrimPathEffect.h"
#include "src/core/SkPathEffectBase.h"

class SkTrimPE : public SkPathEffectBase {
public:
    SkTrimPE(SkScalar startT, SkScalar stopT, SkTrimPathEffect::Mode);

protected:
    void flatten(SkWriteBuffer&) const override;
    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*,
                      const SkMatrix&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkTrimPE)

    bool computeFastBounds(SkRect* bounds) const override {
        // Trimming a path returns a subset of the input path so just return true and leave bounds
        // unmodified
        return true;
    }

    const SkScalar               fStartT,
                                 fStopT;
    const SkTrimPathEffect::Mode fMode;

    using INHERITED = SkPathEffectBase;
};

#endif
