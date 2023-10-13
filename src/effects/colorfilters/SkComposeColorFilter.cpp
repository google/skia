/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/effects/colorfilters/SkComposeColorFilter.h"

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"

#include <utility>
struct SkStageRec;

SkComposeColorFilter::SkComposeColorFilter(sk_sp<SkColorFilter> outer, sk_sp<SkColorFilter> inner)
        : fOuter(as_CFB_sp(std::move(outer))), fInner(as_CFB_sp(std::move(inner))) {
    SkASSERT(fOuter && fInner);
}

bool SkComposeColorFilter::onIsAlphaUnchanged() const {
    // Can only claim alphaunchanged support if both our proxys do.
    return fOuter->isAlphaUnchanged() && fInner->isAlphaUnchanged();
}

bool SkComposeColorFilter::appendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
    bool innerIsOpaque = shaderIsOpaque;
    if (!fInner->isAlphaUnchanged()) {
        innerIsOpaque = false;
    }
    return fInner->appendStages(rec, shaderIsOpaque) && fOuter->appendStages(rec, innerIsOpaque);
}

void SkComposeColorFilter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fOuter.get());
    buffer.writeFlattenable(fInner.get());
}

sk_sp<SkFlattenable> SkComposeColorFilter::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkColorFilter> outer(buffer.readColorFilter());
    sk_sp<SkColorFilter> inner(buffer.readColorFilter());
    return outer ? outer->makeComposed(std::move(inner)) : inner;
}

sk_sp<SkColorFilter> SkColorFilter::makeComposed(sk_sp<SkColorFilter> inner) const {
    if (!inner) {
        return sk_ref_sp(this);
    }

    return sk_sp<SkColorFilter>(new SkComposeColorFilter(sk_ref_sp(this), std::move(inner)));
}

void SkRegisterComposeColorFilterFlattenable() { SK_REGISTER_FLATTENABLE(SkComposeColorFilter); }
