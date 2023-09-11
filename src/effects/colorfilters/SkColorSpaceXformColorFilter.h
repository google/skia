/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkColorSpaceXformColorFilter_DEFINED
#define SkColorSpaceXformColorFilter_DEFINED

#include "include/core/SkColorSpace.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkRefCnt.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"

class SkReadBuffer;
class SkWriteBuffer;
struct SkStageRec;

class SkColorSpaceXformColorFilter final : public SkColorFilterBase {
public:
    SkColorSpaceXformColorFilter(sk_sp<SkColorSpace> src, sk_sp<SkColorSpace> dst);

    bool appendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;

    SkColorFilterBase::Type type() const override {
        return SkColorFilterBase::Type::kColorSpaceXform;
    }

    sk_sp<SkColorSpace> src() const { return fSrc; }
    sk_sp<SkColorSpace> dst() const { return fDst; }

protected:
    void flatten(SkWriteBuffer& buffer) const override;

private:
    friend void ::SkRegisterSkColorSpaceXformColorFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkColorSpaceXformColorFilter)
    static sk_sp<SkFlattenable> LegacyGammaOnlyCreateProc(SkReadBuffer& buffer);

    const sk_sp<SkColorSpace> fSrc;
    const sk_sp<SkColorSpace> fDst;
    SkColorSpaceXformSteps fSteps;

    friend class SkColorFilter;
    using INHERITED = SkColorFilterBase;
};

#endif
