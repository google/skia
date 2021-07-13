/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrStyle.h"
#include "src/utils/SkDashPathPriv.h"

int GrStyle::KeySize(const GrStyle &style, Apply apply, uint32_t flags) {
    static_assert(sizeof(uint32_t) == sizeof(SkScalar));
    int size = 0;
    if (style.isDashed()) {
        // One scalar for scale, one for dash phase, and one for each dash value.
        size += 2 + style.dashIntervalCnt();
    } else if (style.pathEffect()) {
        // No key for a generic path effect.
        return -1;
    }

    if (Apply::kPathEffectOnly == apply) {
        return size;
    }

    if (style.strokeRec().needToApply()) {
        // One for res scale, one for style/cap/join, one for miter limit, and one for width.
        size += 4;
    }
    return size;
}

void GrStyle::WriteKey(uint32_t *key, const GrStyle &style, Apply apply, SkScalar scale,
                       uint32_t flags) {
    SkASSERT(key);
    SkASSERT(KeySize(style, apply) >= 0);
    static_assert(sizeof(uint32_t) == sizeof(SkScalar));

    int i = 0;
    // The scale can influence both the path effect and stroking. We want to preserve the
    // property that the following two are equal:
    // 1. WriteKey with apply == kPathEffectAndStrokeRec
    // 2. WriteKey with apply == kPathEffectOnly followed by WriteKey of a GrStyle made
    //    from SkStrokeRec output by the the path effect (and no additional path effect).
    // Since the scale can affect both parts of 2 we write it into the key twice.
    if (style.isDashed()) {
        static_assert(sizeof(style.dashPhase()) == sizeof(uint32_t));
        SkScalar phase = style.dashPhase();
        memcpy(&key[i++], &scale, sizeof(SkScalar));
        memcpy(&key[i++], &phase, sizeof(SkScalar));

        int32_t count = style.dashIntervalCnt();
        // Dash count should always be even.
        SkASSERT(0 == (count & 0x1));
        const SkScalar *intervals = style.dashIntervals();
        int intervalByteCnt = count * sizeof(SkScalar);
        memcpy(&key[i], intervals, intervalByteCnt);
        i += count;
    } else {
        SkASSERT(!style.pathEffect());
    }

    if (Apply::kPathEffectAndStrokeRec == apply && style.strokeRec().needToApply()) {
        memcpy(&key[i++], &scale, sizeof(SkScalar));
        enum {
            kStyleBits = 2,
            kJoinBits = 2,
            kCapBits = 32 - kStyleBits - kJoinBits,

            kJoinShift = kStyleBits,
            kCapShift = kJoinShift + kJoinBits,
        };
        static_assert(SkStrokeRec::kStyleCount <= (1 << kStyleBits));
        static_assert(SkPaint::kJoinCount <= (1 << kJoinBits));
        static_assert(SkPaint::kCapCount <= (1 << kCapBits));
        // The cap type only matters for unclosed shapes. However, a path effect could unclose
        // the shape before it is stroked.
        SkPaint::Cap cap = SkPaint::kDefault_Cap;
        if (!(flags & kClosed_KeyFlag) || style.pathEffect()) {
            cap = style.strokeRec().getCap();
        }
        SkScalar miter = -1.f;
        SkPaint::Join join = SkPaint::kDefault_Join;

        // Dashing will not insert joins but other path effects may.
        if (!(flags & kNoJoins_KeyFlag) || style.hasNonDashPathEffect()) {
            join = style.strokeRec().getJoin();
            // Miter limit only affects miter joins
            if (SkPaint::kMiter_Join == join) {
                miter = style.strokeRec().getMiter();
            }
        }

        key[i++] = style.strokeRec().getStyle() |
                   join << kJoinShift |
                   cap << kCapShift;

        memcpy(&key[i++], &miter, sizeof(miter));

        SkScalar width = style.strokeRec().getWidth();
        memcpy(&key[i++], &width, sizeof(width));
    }
    SkASSERT(KeySize(style, apply) == i);
}

void GrStyle::initPathEffect(sk_sp<SkPathEffect> pe) {
    SkASSERT(!fPathEffect);
    SkASSERT(SkPathEffect::kNone_DashType == fDashInfo.fType);
    SkASSERT(0 == fDashInfo.fIntervals.count());
    if (!pe) {
        return;
    }
    SkPathEffect::DashInfo info;
    if (SkPathEffect::kDash_DashType == pe->asADash(&info)) {
        SkStrokeRec::Style recStyle = fStrokeRec.getStyle();
        if (recStyle != SkStrokeRec::kFill_Style && recStyle != SkStrokeRec::kStrokeAndFill_Style) {
            fDashInfo.fType = SkPathEffect::kDash_DashType;
            fDashInfo.fIntervals.reset(info.fCount);
            fDashInfo.fPhase = info.fPhase;
            info.fIntervals = fDashInfo.fIntervals.get();
            pe->asADash(&info);
            fPathEffect = std::move(pe);
        }
    } else {
        fPathEffect = std::move(pe);
    }
}

bool GrStyle::applyPathEffect(SkPath* dst, SkStrokeRec* strokeRec, const SkPath& src) const {
    if (!fPathEffect) {
        return false;
    }

    // TODO: [skbug.com/11957] Plumb CTM callers and pass it to filterPath().
    SkASSERT(!fPathEffect->needsCTM());

    if (SkPathEffect::kDash_DashType == fDashInfo.fType) {
        // We apply the dash ourselves here rather than using the path effect. This is so that
        // we can control whether the dasher applies the strokeRec for special cases. Our keying
        // depends on the strokeRec being applied separately.
        SkASSERT(!fPathEffect->needsCTM());  // Make sure specified PE doesn't need CTM
        SkScalar phase = fDashInfo.fPhase;
        const SkScalar* intervals = fDashInfo.fIntervals.get();
        int intervalCnt = fDashInfo.fIntervals.count();
        SkScalar initialLength;
        int initialIndex;
        SkScalar intervalLength;
        SkDashPath::CalcDashParameters(phase, intervals, intervalCnt, &initialLength,
                                       &initialIndex, &intervalLength);
        if (!SkDashPath::InternalFilter(dst, src, strokeRec,
                                        nullptr, intervals, intervalCnt,
                                        initialLength, initialIndex, intervalLength,
                                        SkDashPath::StrokeRecApplication::kDisallow)) {
            return false;
        }
    } else if (!fPathEffect->filterPath(dst, src, strokeRec, nullptr)) {
        return false;
    }
    dst->setIsVolatile(true);
    return true;
}

bool GrStyle::applyPathEffectToPath(SkPath *dst, SkStrokeRec *remainingStroke,
                                    const SkPath &src, SkScalar resScale) const {
    SkASSERT(dst);
    SkStrokeRec strokeRec = fStrokeRec;
    strokeRec.setResScale(resScale);
    if (!this->applyPathEffect(dst, &strokeRec, src)) {
        return false;
    }
    *remainingStroke = strokeRec;
    return true;
}

bool GrStyle::applyToPath(SkPath* dst, SkStrokeRec::InitStyle* style, const SkPath& src,
                          SkScalar resScale) const {
    SkASSERT(style);
    SkASSERT(dst);
    SkStrokeRec strokeRec = fStrokeRec;
    strokeRec.setResScale(resScale);
    const SkPath* pathForStrokeRec = &src;
    if (this->applyPathEffect(dst, &strokeRec, src)) {
        pathForStrokeRec = dst;
    } else if (fPathEffect) {
        return false;
    }
    if (strokeRec.needToApply()) {
        if (!strokeRec.applyToPath(dst, *pathForStrokeRec)) {
            return false;
        }
        dst->setIsVolatile(true);
        *style = SkStrokeRec::kFill_InitStyle;
    } else if (!fPathEffect) {
        // Nothing to do for path effect or stroke, fail.
        return false;
    } else {
        SkASSERT(SkStrokeRec::kFill_Style == strokeRec.getStyle() ||
                 SkStrokeRec::kHairline_Style == strokeRec.getStyle());
        *style = strokeRec.getStyle() == SkStrokeRec::kFill_Style
                 ? SkStrokeRec::kFill_InitStyle
                 : SkStrokeRec::kHairline_InitStyle;
    }
    return true;
}
