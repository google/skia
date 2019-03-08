// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkPenPathEffectImpl_DEFINED
#define SkPenPathEffectImpl_DEFINED

#include "SkPathEffect.h"
#include "SkMatrix.h"

struct SkPenPathEffect : public SkPathEffect {
    SkMatrix fMat;
    SkPenPathEffect(const SkMatrix&);
    ~SkPenPathEffect() override;
    SkRect onComputeFastBounds(const SkRect& src) const override;
    bool onFilterPath(SkPath*, const SkPath&, SkStrokeRec*, const SkRect*) const override;
    void flatten(SkWriteBuffer&) const override;
    SK_FLATTENABLE_HOOKS(SkPenPathEffect)
};
#endif  // SkPenPathEffectImpl_DEFINED
