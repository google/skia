/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStyle.h"

int GrStyle::KeySize(const GrStyle &style, Apply apply) {
    GR_STATIC_ASSERT(sizeof(uint32_t) == sizeof(SkScalar));
    int size = 0;
    if (style.isDashed()) {
        // One scalar for dash phase and one for each dash value.
        size += 1 + style.dashIntervalCnt();
    } else if (style.pathEffect()) {
        // No key for a generic path effect.
        return -1;
    }

    if (Apply::kPathEffectOnly == apply) {
        return size;
    }

    if (style.strokeRec().needToApply()) {
        // One for style/cap/join, 2 for miter and width.
        size += 3;
    }
    return size;
}

void GrStyle::WriteKey(uint32_t *key, const GrStyle &style, Apply apply) {
    SkASSERT(key);
    SkASSERT(KeySize(style, apply) >= 0);
    GR_STATIC_ASSERT(sizeof(uint32_t) == sizeof(SkScalar));

    int i = 0;
    if (style.isDashed()) {
        GR_STATIC_ASSERT(sizeof(style.dashPhase()) == sizeof(uint32_t));
        SkScalar phase = style.dashPhase();
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
        enum {
            kStyleBits = 2,
            kJoinBits = 2,
            kCapBits = 32 - kStyleBits - kJoinBits,

            kJoinShift = kStyleBits,
            kCapShift = kJoinShift + kJoinBits,
        };
        GR_STATIC_ASSERT(SkStrokeRec::kStyleCount <= (1 << kStyleBits));
        GR_STATIC_ASSERT(SkPaint::kJoinCount <= (1 << kJoinBits));
        GR_STATIC_ASSERT(SkPaint::kCapCount <= (1 << kCapBits));
        key[i++] = style.strokeRec().getStyle() |
                   style.strokeRec().getJoin() << kJoinShift |
                   style.strokeRec().getCap() << kCapShift;

        SkScalar scalar;
        // Miter limit only affects miter joins
        scalar = SkPaint::kMiter_Join == style.strokeRec().getJoin()
                 ? style.strokeRec().getMiter()
                 : -1.f;
        memcpy(&key[i++], &scalar, sizeof(scalar));

        scalar = style.strokeRec().getWidth();
        memcpy(&key[i++], &scalar, sizeof(scalar));
    }
    SkASSERT(KeySize(style, apply) == i);
}

void GrStyle::initPathEffect(SkPathEffect* pe) {
    SkASSERT(!fPathEffect)
    SkASSERT(SkPathEffect::kNone_DashType == fDashInfo.fType);
    SkASSERT(0 == fDashInfo.fIntervals.count());
    if (!pe) {
        return;
    }
    SkPathEffect::DashInfo info;
    if (SkPathEffect::kDash_DashType == pe->asADash(&info)) {
        if (fStrokeRec.getStyle() != SkStrokeRec::kFill_Style) {
            fDashInfo.fType = SkPathEffect::kDash_DashType;
            fDashInfo.fIntervals.reset(info.fCount);
            fDashInfo.fPhase = info.fPhase;
            info.fIntervals = fDashInfo.fIntervals.get();
            pe->asADash(&info);
            fPathEffect.reset(SkSafeRef(pe));
        }
    } else {
        fPathEffect.reset(SkSafeRef(pe));
    }
}

static inline bool apply_path_effect(SkPath* dst, SkStrokeRec* strokeRec,
                                     const sk_sp<SkPathEffect>& pe, const SkPath& src) {
    if (!pe) {
        return false;
    }
    if (!pe->filterPath(dst, src, strokeRec, nullptr)) {
        return false;
    }
    dst->setIsVolatile(true);
    return true;
}

bool GrStyle::applyPathEffectToPath(SkPath *dst, SkStrokeRec *remainingStroke,
                                    const SkPath &src) const {
    SkASSERT(dst);
    SkStrokeRec strokeRec = fStrokeRec;
    if (!apply_path_effect(dst, &strokeRec, fPathEffect, src)) {
        return false;
    }
    *remainingStroke = strokeRec;
    return true;
}

bool GrStyle::applyToPath(SkPath* dst, SkStrokeRec::InitStyle* style, const SkPath& src) const {
    SkASSERT(style);
    SkASSERT(dst);
    SkStrokeRec strokeRec = fStrokeRec;
    if (!apply_path_effect(dst, &strokeRec, fPathEffect, src)) {
        return false;
    }
    if (strokeRec.needToApply()) {
        if (!strokeRec.applyToPath(dst, *dst)) {
            return false;
        }
        *style = SkStrokeRec::kFill_InitStyle;
    } else {
        SkASSERT(SkStrokeRec::kFill_Style == strokeRec.getStyle() ||
                 SkStrokeRec::kHairline_Style == strokeRec.getStyle());
        *style = strokeRec.getStyle() == SkStrokeRec::kFill_Style
                 ? SkStrokeRec::kFill_InitStyle
                 : SkStrokeRec::kHairline_InitStyle;
    }
    return true;
}
