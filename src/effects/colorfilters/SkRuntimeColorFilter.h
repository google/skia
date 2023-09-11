/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRuntimeColorFilter_DEFINED
#define SkRuntimeColorFilter_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkDebug.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"

#include <vector>

class SkReadBuffer;
class SkWriteBuffer;
struct SkStageRec;

class SkRuntimeColorFilter : public SkColorFilterBase {
public:
    SkRuntimeColorFilter(sk_sp<SkRuntimeEffect> effect,
                         sk_sp<const SkData> uniforms,
                         SkSpan<const SkRuntimeEffect::ChildPtr> children);

    bool appendStages(const SkStageRec& rec, bool) const override;

    bool onIsAlphaUnchanged() const override;

    void flatten(SkWriteBuffer& buffer) const override;

    SkRuntimeEffect* asRuntimeEffect() const override;

    SkColorFilterBase::Type type() const override { return SkColorFilterBase::Type::kRuntime; }

    SK_FLATTENABLE_HOOKS(SkRuntimeColorFilter)

    sk_sp<SkRuntimeEffect> effect() const { return fEffect; }
    sk_sp<const SkData> uniforms() const { return fUniforms; }
    SkSpan<const SkRuntimeEffect::ChildPtr> children() const { return fChildren; }

private:
    sk_sp<SkRuntimeEffect> fEffect;
    sk_sp<const SkData> fUniforms;
    std::vector<SkRuntimeEffect::ChildPtr> fChildren;
};

#endif
