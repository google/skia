/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkComposeColorFilter_DEFINED
#define SkComposeColorFilter_DEFINED

#include "include/core/SkColorFilter.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkRefCnt.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"

class SkReadBuffer;
class SkWriteBuffer;
struct SkStageRec;

class SkComposeColorFilter final : public SkColorFilterBase {
public:
    bool onIsAlphaUnchanged() const override;

    bool appendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;

    SkColorFilterBase::Type type() const override { return SkColorFilterBase::Type::kCompose; }

    sk_sp<SkColorFilterBase> outer() const { return fOuter; }
    sk_sp<SkColorFilterBase> inner() const { return fInner; }

protected:
    void flatten(SkWriteBuffer& buffer) const override;

private:
    friend void ::SkRegisterComposeColorFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkComposeColorFilter)

    SkComposeColorFilter(sk_sp<SkColorFilter> outer, sk_sp<SkColorFilter> inner);

    sk_sp<SkColorFilterBase> fOuter;
    sk_sp<SkColorFilterBase> fInner;

    friend class SkColorFilter;

    using INHERITED = SkColorFilter;
};

#endif
