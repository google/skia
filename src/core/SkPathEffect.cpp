/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPathEffect.h"
#include "SkPath.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

///////////////////////////////////////////////////////////////////////////////

void SkPathEffect::computeFastBounds(SkRect* dst, const SkRect& src) const {
    *dst = src;
}

bool SkPathEffect::asPoints(PointData* results, const SkPath& src,
                    const SkStrokeRec&, const SkMatrix&, const SkRect*) const {
    return false;
}

SkPathEffect::DashType SkPathEffect::asADash(DashInfo* info) const {
    return kNone_DashType;
}

///////////////////////////////////////////////////////////////////////////////

/** \class SkPairPathEffect

 Common baseclass for Compose and Sum. This subclass manages two pathEffects,
 including flattening them. It does nothing in filterPath, and is only useful
 for managing the lifetimes of its two arguments.
 */
class SK_API SkPairPathEffect : public SkPathEffect {
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

    SK_TO_STRING_OVERRIDE()

private:
    typedef SkPathEffect INHERITED;
};

#ifndef SK_IGNORE_TO_STRING
void SkPairPathEffect::toString(SkString* str) const {
    str->appendf("first: ");
    if (fPE0) {
        fPE0->toString(str);
    }
    str->appendf(" second: ");
    if (fPE1) {
        fPE1->toString(str);
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

/** \class SkComposePathEffect

 This subclass of SkPathEffect composes its two arguments, to create
 a compound pathEffect.
 */
class SK_API SkComposePathEffect : public SkPairPathEffect {
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

    bool filterPath(SkPath* dst, const SkPath& src, SkStrokeRec* rec,
                    const SkRect* cullRect) const override {
        SkPath          tmp;
        const SkPath*   ptr = &src;

        if (fPE1->filterPath(&tmp, src, rec, cullRect)) {
            ptr = &tmp;
        }
        return fPE0->filterPath(dst, *ptr, rec, cullRect);
    }
    

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkComposePathEffect)

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    bool exposedInAndroidJavaAPI() const override { return true; }
#endif

protected:
    SkComposePathEffect(sk_sp<SkPathEffect> outer, sk_sp<SkPathEffect> inner)
        : INHERITED(outer, inner) {}

private:
    // illegal
    SkComposePathEffect(const SkComposePathEffect&);
    SkComposePathEffect& operator=(const SkComposePathEffect&);
    friend class SkPathEffect;

    typedef SkPairPathEffect INHERITED;
};

sk_sp<SkFlattenable> SkComposePathEffect::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkPathEffect> pe0(buffer.readPathEffect());
    sk_sp<SkPathEffect> pe1(buffer.readPathEffect());
    return SkComposePathEffect::Make(std::move(pe0), std::move(pe1));
}

#ifndef SK_IGNORE_TO_STRING
void SkComposePathEffect::toString(SkString* str) const {
    str->appendf("SkComposePathEffect: (");
    this->INHERITED::toString(str);
    str->appendf(")");
}
#endif

///////////////////////////////////////////////////////////////////////////////

/** \class SkSumPathEffect

 This subclass of SkPathEffect applies two pathEffects, one after the other.
 Its filterPath() returns true if either of the effects succeeded.
 */
class SK_API SkSumPathEffect : public SkPairPathEffect {
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

    bool filterPath(SkPath* dst, const SkPath& src, SkStrokeRec* rec,
                    const SkRect* cullRect) const override {
        // use bit-or so that we always call both, even if the first one succeeds
        return fPE0->filterPath(dst, src, rec, cullRect) |
               fPE1->filterPath(dst, src, rec, cullRect);
    }
    

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkSumPathEffect)

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    bool exposedInAndroidJavaAPI() const override { return true; }
#endif

protected:
    SkSumPathEffect(sk_sp<SkPathEffect> first, sk_sp<SkPathEffect> second)
    : INHERITED(first, second) {}

private:
    // illegal
    SkSumPathEffect(const SkSumPathEffect&);
    SkSumPathEffect& operator=(const SkSumPathEffect&);
    friend class SkPathEffect;

    typedef SkPairPathEffect INHERITED;
};

sk_sp<SkFlattenable> SkSumPathEffect::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkPathEffect> pe0(buffer.readPathEffect());
    sk_sp<SkPathEffect> pe1(buffer.readPathEffect());
    return SkSumPathEffect::Make(pe0, pe1);
}

#ifndef SK_IGNORE_TO_STRING
void SkSumPathEffect::toString(SkString* str) const {
    str->appendf("SkSumPathEffect: (");
    this->INHERITED::toString(str);
    str->appendf(")");
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkPathEffect> SkPathEffect::MakeSum(sk_sp<SkPathEffect> first, sk_sp<SkPathEffect> second) {
    return SkSumPathEffect::Make(std::move(first), std::move(second));
}

sk_sp<SkPathEffect> SkPathEffect::MakeCompose(sk_sp<SkPathEffect> outer,
                                              sk_sp<SkPathEffect> inner) {
    return SkComposePathEffect::Make(std::move(outer), std::move(inner));
}

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkComposePathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkSumPathEffect)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
