/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "src/core/SkPathEffectBase.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

///////////////////////////////////////////////////////////////////////////////

bool SkPathEffect::filterPath(SkPath* dst, const SkPath& src, SkStrokeRec* rec,
                              const SkRect* bounds) const {
    return this->filterPath(dst, src, rec, bounds, SkMatrix::I());
}

bool SkPathEffect::filterPath(SkPath* dst, const SkPath& src, SkStrokeRec* rec,
                              const SkRect* bounds, const SkMatrix& ctm) const {
    SkPath tmp, *tmpDst = dst;
    if (dst == &src) {
        tmpDst = &tmp;
    }
    if (as_PEB(this)->onFilterPath(tmpDst, src, rec, bounds, ctm)) {
        if (dst == &src) {
            *dst = tmp;
        }
        return true;
    }
    return false;
}

bool SkPathEffectBase::asPoints(PointData* results, const SkPath& src,
                    const SkStrokeRec& rec, const SkMatrix& mx, const SkRect* rect) const {
    return this->onAsPoints(results, src, rec, mx, rect);
}

SkPathEffect::DashType SkPathEffect::asADash(DashInfo* info) const {
    return as_PEB(this)->onAsADash(info);
}

bool SkPathEffect::needsCTM() const {
    return as_PEB(this)->onNeedsCTM();
}

///////////////////////////////////////////////////////////////////////////////

/** \class SkPairPathEffect

 Common baseclass for Compose and Sum. This subclass manages two pathEffects,
 including flattening them. It does nothing in filterPath, and is only useful
 for managing the lifetimes of its two arguments.
 */
class SkPairPathEffect : public SkPathEffectBase {
protected:
    SkPairPathEffect(sk_sp<SkPathEffect> pe0, sk_sp<SkPathEffect> pe1)
        : fPE0(std::move(pe0)), fPE1(std::move(pe1))
    {
        SkASSERT(fPE0.get());
        SkASSERT(fPE1.get());
    }

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeFlattenable(fPE0.get());
        buffer.writeFlattenable(fPE1.get());
    }

    // these are visible to our subclasses
    sk_sp<SkPathEffect> fPE0;
    sk_sp<SkPathEffect> fPE1;

private:
    using INHERITED = SkPathEffectBase;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkComposePathEffect : public SkPairPathEffect {
public:
    /** Construct a pathEffect whose effect is to apply first the inner pathEffect
     and the the outer pathEffect (e.g. outer(inner(path)))
     The reference counts for outer and inner are both incremented in the constructor,
     and decremented in the destructor.
     */
    static sk_sp<SkPathEffect> Make(sk_sp<SkPathEffect> outer, sk_sp<SkPathEffect> inner) {
        if (!outer) {
            return inner;
        }
        if (!inner) {
            return outer;
        }
        return sk_sp<SkPathEffect>(new SkComposePathEffect(outer, inner));
    }

    SkComposePathEffect(sk_sp<SkPathEffect> outer, sk_sp<SkPathEffect> inner)
            : INHERITED(std::move(outer), std::move(inner)) {}

    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec* rec,
                       const SkRect* cullRect, const SkMatrix& ctm) const override {
        SkPath          tmp;
        const SkPath*   ptr = &src;

        if (fPE1->filterPath(&tmp, src, rec, cullRect, ctm)) {
            ptr = &tmp;
        }
        return fPE0->filterPath(dst, *ptr, rec, cullRect, ctm);
    }

    SK_FLATTENABLE_HOOKS(SkComposePathEffect)

    bool computeFastBounds(SkRect* bounds) const override {
        // inner (fPE1) is computed first, automatically updating bounds before computing outer.
        return as_PEB(fPE1)->computeFastBounds(bounds) &&
               as_PEB(fPE0)->computeFastBounds(bounds);
    }

private:
    // illegal
    SkComposePathEffect(const SkComposePathEffect&);
    SkComposePathEffect& operator=(const SkComposePathEffect&);
    friend class SkPathEffect;

    using INHERITED = SkPairPathEffect;
};

sk_sp<SkFlattenable> SkComposePathEffect::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkPathEffect> pe0(buffer.readPathEffect());
    sk_sp<SkPathEffect> pe1(buffer.readPathEffect());
    return SkComposePathEffect::Make(std::move(pe0), std::move(pe1));
}

///////////////////////////////////////////////////////////////////////////////

/** \class SkSumPathEffect

 This subclass of SkPathEffect applies two pathEffects, one after the other.
 Its filterPath() returns true if either of the effects succeeded.
 */
class SkSumPathEffect : public SkPairPathEffect {
public:
    /** Construct a pathEffect whose effect is to apply two effects, in sequence.
     (e.g. first(path) + second(path))
     The reference counts for first and second are both incremented in the constructor,
     and decremented in the destructor.
     */
    static sk_sp<SkPathEffect> Make(sk_sp<SkPathEffect> first, sk_sp<SkPathEffect> second) {
        if (!first) {
            return second;
        }
        if (!second) {
            return first;
        }
        return sk_sp<SkPathEffect>(new SkSumPathEffect(first, second));
    }

    SkSumPathEffect(sk_sp<SkPathEffect> first, sk_sp<SkPathEffect> second)
            : INHERITED(std::move(first), std::move(second)) {}

    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec* rec,
                      const SkRect* cullRect, const SkMatrix& ctm) const override {
        // always call both, even if the first one succeeds
        bool filteredFirst = fPE0->filterPath(dst, src, rec, cullRect, ctm);
        bool filteredSecond = fPE1->filterPath(dst, src, rec, cullRect, ctm);
        return filteredFirst || filteredSecond;
    }

    SK_FLATTENABLE_HOOKS(SkSumPathEffect)

    bool computeFastBounds(SkRect* bounds) const override {
        // Unlike Compose(), PE0 modifies the path first for Sum
        return as_PEB(fPE0)->computeFastBounds(bounds) &&
               as_PEB(fPE1)->computeFastBounds(bounds);
    }

private:
    // illegal
    SkSumPathEffect(const SkSumPathEffect&);
    SkSumPathEffect& operator=(const SkSumPathEffect&);
    friend class SkPathEffect;

    using INHERITED = SkPairPathEffect;
};

sk_sp<SkFlattenable> SkSumPathEffect::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkPathEffect> pe0(buffer.readPathEffect());
    sk_sp<SkPathEffect> pe1(buffer.readPathEffect());
    return SkSumPathEffect::Make(pe0, pe1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkPathEffect> SkPathEffect::MakeSum(sk_sp<SkPathEffect> first, sk_sp<SkPathEffect> second) {
    return SkSumPathEffect::Make(std::move(first), std::move(second));
}

sk_sp<SkPathEffect> SkPathEffect::MakeCompose(sk_sp<SkPathEffect> outer,
                                              sk_sp<SkPathEffect> inner) {
    return SkComposePathEffect::Make(std::move(outer), std::move(inner));
}

void SkPathEffectBase::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkComposePathEffect);
    SK_REGISTER_FLATTENABLE(SkSumPathEffect);
}

sk_sp<SkPathEffect> SkPathEffect::Deserialize(const void* data, size_t size,
                                              const SkDeserialProcs* procs) {
    return sk_sp<SkPathEffect>(static_cast<SkPathEffect*>(
                               SkFlattenable::Deserialize(
                               kSkPathEffect_Type, data, size, procs).release()));
}
