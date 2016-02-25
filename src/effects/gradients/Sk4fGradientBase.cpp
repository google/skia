/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sk4fGradientBase.h"

namespace {

const float kInv255Float = 1.0f / 255;

SkPMColor pack_color(SkColor c, bool premul) {
    return premul
        ? SkPreMultiplyColor(c)
        : SkPackARGB32NoCheck(SkColorGetA(c), SkColorGetR(c), SkColorGetG(c), SkColorGetB(c));
}

// true when x is in [k1,k2)
bool in_range(SkScalar x, SkScalar k1, SkScalar k2) {
    SkASSERT(k1 != k2);
    return (k1 < k2)
        ? (x >= k1 && x < k2)
        : (x >= k2 && x < k1);
}

} // anonymous namespace

SkGradientShaderBase::GradientShaderBase4fContext::
Interval::Interval(SkPMColor c0, SkScalar p0,
                   SkPMColor c1, SkScalar p1,
                   const Sk4f& componentScale)
    : fP0(p0)
    , fP1(p1)
    , fZeroRamp(c0 == c1) {
    SkASSERT(p0 != p1);

    const Sk4f c4f0 = SkNx_cast<float>(Sk4b::Load(&c0)) * componentScale;
    const Sk4f c4f1 = SkNx_cast<float>(Sk4b::Load(&c1)) * componentScale;
    const Sk4f dc4f = (c4f1 - c4f0) / (p1 - p0);

    c4f0.store(&fC0.fVec);
    dc4f.store(&fDc.fVec);
}

SkGradientShaderBase::GradientShaderBase4fContext::
Interval::Interval(const Sk4f& c0, const Sk4f& dc,
                   SkScalar p0, SkScalar p1)
    : fP0(p0)
    , fP1(p1)
    , fZeroRamp((dc == 0).allTrue()) {
    c0.store(fC0.fVec);
    dc.store(fDc.fVec);
}

bool SkGradientShaderBase::GradientShaderBase4fContext::
Interval::contains(SkScalar fx) const {
    return in_range(fx, fP0, fP1);
}

SkGradientShaderBase::
GradientShaderBase4fContext::GradientShaderBase4fContext(const SkGradientShaderBase& shader,
                                                         const ContextRec& rec)
    : INHERITED(shader, rec)
    , fFlags(this->INHERITED::getFlags())
#ifdef SK_SUPPORT_LEGACY_GRADIENT_DITHERING
    , fDither(true)
#else
    , fDither(rec.fPaint->isDither())
#endif
{
    // The main job here is to build an interval list.  Intervals are a different
    // representation of the color stops data, optimized for efficient scan line
    // access during shading.
    //
    //   [{P0,C0} , {P1,C1}) [{P1,C2} , {P2,c3}) ... [{Pn,C2n} , {Pn+1,C2n+1})
    //
    // The list is sorted in increasing dst order, i.e. X(Pk) < X(Pk+1).  This
    // allows us to always traverse left->right when iterating over a scan line.
    // It also means that the interval order matches the color stops when dx >= 0,
    // and is the inverse (pos, colors, order are flipped) when dx < 0.
    //
    // Note: the current representation duplicates pos data; we could refactor to
    //       avoid this if interval storage size becomes a concern.
    //
    // Aside from reordering, we also perform two more pre-processing steps at
    // this stage:
    //
    //   1) scale the color components depending on paint alpha and the requested
    //      interpolation space (note: the interval color storage is SkPM4f, but
    //      that doesn't necessarily mean the colors are premultiplied; that
    //      property is tracked in fColorsArePremul)
    //
    //   2) inject synthetic intervals to support tiling.
    //
    //      * for kRepeat, no extra intervals are needed - the iterator just
    //        wraps around at the end:
    //
    //          ->[P0,P1)->..[Pn-1,Pn)->
    //
    //      * for kClamp, we add two "infinite" intervals before/after:
    //
    //          [-/+inf , P0)->[P0 , P1)->..[Pn-1 , Pn)->[Pn , +/-inf)
    //
    //        (the iterator should never run off the end in this mode)
    //
    //      * for kMirror, we extend the range to [0..2] and add a flipped
    //        interval series - then the iterator operates just as in the
    //        kRepeat case:
    //
    //          ->[P0,P1)->..[Pn-1,Pn)->[2 - Pn,2 - Pn-1)->..[2 - P1,2 - P0)->
    //
    // TODO: investigate collapsing intervals << 1px.

    const SkMatrix& inverse = this->getTotalInverse();
    fDstToPos.setConcat(shader.fPtsToUnit, inverse);
    fDstToPosProc = fDstToPos.getMapXYProc();
    fDstToPosClass = static_cast<uint8_t>(INHERITED::ComputeMatrixClass(fDstToPos));

    if (shader.fColorsAreOpaque && this->getPaintAlpha() == SK_AlphaOPAQUE) {
        fFlags |= kOpaqueAlpha_Flag;
    }

    fColorsArePremul =
        (shader.fGradFlags & SkGradientShader::kInterpolateColorsInPremul_Flag)
        || shader.fColorsAreOpaque;

    const float paintAlpha = rec.fPaint->getAlpha() * kInv255Float;
    const Sk4f componentScale = fColorsArePremul
        ? Sk4f(paintAlpha * kInv255Float)
        : Sk4f(kInv255Float, kInv255Float, kInv255Float, paintAlpha * kInv255Float);

    SkASSERT(shader.fColorCount > 1);
    SkASSERT(shader.fOrigColors);

    int direction = 1;
    int first_index = 0;
    int last_index = shader.fColorCount - 1;
    SkScalar first_pos = 0;
    SkScalar last_pos = 1;
    const bool dx_is_pos = fDstToPos.getScaleX() >= 0;
    if (!dx_is_pos) {
        direction = -direction;
        SkTSwap(first_index, last_index);
        SkTSwap(first_pos, last_pos);
    }

    if (shader.fTileMode == SkShader::kClamp_TileMode) {
        // synthetic edge interval: -/+inf .. P0)
        const SkPMColor clamp_color = pack_color(shader.fOrigColors[first_index],
                                                 fColorsArePremul);
        const SkScalar clamp_pos = dx_is_pos ? SK_ScalarMin : SK_ScalarMax;
        fIntervals.emplace_back(clamp_color, clamp_pos,
                                clamp_color, first_pos,
                                componentScale);
    }

    int prev = first_index;
    int curr = prev + direction;
    SkScalar prev_pos = first_pos;
    if (shader.fOrigPos) {
        // explicit positions
        do {
            // TODO: this sanitization should be done in SkGradientShaderBase
            const SkScalar curr_pos = (dx_is_pos)
                ? SkTPin(shader.fOrigPos[curr], prev_pos, last_pos)
                : SkTPin(shader.fOrigPos[curr], last_pos, prev_pos);
            if (curr_pos != prev_pos) {
                fIntervals.emplace_back(
                    pack_color(shader.fOrigColors[prev], fColorsArePremul),
                    prev_pos,
                    pack_color(shader.fOrigColors[curr], fColorsArePremul),
                    curr_pos,
                    componentScale);
            }
            prev = curr;
            prev_pos = curr_pos;
            curr += direction;
        } while (prev != last_index);
    } else {
        // implicit positions
        const SkScalar dt = direction * SK_Scalar1 / (shader.fColorCount - 1);
        do {
            const SkScalar curr_pos = prev_pos + dt;
            fIntervals.emplace_back(
                pack_color(shader.fOrigColors[prev], fColorsArePremul),
                prev_pos,
                pack_color(shader.fOrigColors[curr], fColorsArePremul),
                curr_pos,
                componentScale);

            prev = curr;
            prev_pos = curr_pos;
            curr += direction;
        } while (prev != last_index);
        // pin the last pos to maintain accurate [0,1] pos coverage.
        fIntervals.back().fP1 = last_pos;
    }

    if (shader.fTileMode == SkShader::kClamp_TileMode) {
        // synthetic edge interval: Pn .. +/-inf
        const SkPMColor clamp_color =
            pack_color(shader.fOrigColors[last_index], fColorsArePremul);
        const SkScalar clamp_pos = dx_is_pos ? SK_ScalarMax : SK_ScalarMin;
        fIntervals.emplace_back(clamp_color, last_pos,
                                clamp_color, clamp_pos,
                                componentScale);
    } else if (shader.fTileMode == SkShader::kMirror_TileMode) {
        const int count = fIntervals.count();
        // synthetic flipped intervals in [1 .. 2)
        for (int i = count - 1; i >= 0; --i) {
            const Interval& interval = fIntervals[i];
            const SkScalar p0 = interval.fP0;
            const SkScalar p1 = interval.fP1;
            Sk4f dc = Sk4f::Load(interval.fDc.fVec);
            Sk4f c = Sk4f::Load(interval.fC0.fVec) + dc * Sk4f(p1 - p0);
            fIntervals.emplace_back(c, dc * Sk4f(-1), 2 - p1, 2 - p0);
        }

        if (!dx_is_pos) {
            // When dx is negative, our initial invervals are in (1..0] order.
            // The loop above appends their flipped counterparts, pivoted in 2: (1..0](2..1]
            // To achieve the expected monotonic interval order, we need to
            // swap the two halves: (2..1](1..0]
            // TODO: we can probably avoid this late swap with some additional logic during
            //       the initial interval buildup.
            SkASSERT(fIntervals.count() == count * 2)
            for (int i = 0; i < count; ++i) {
                SkTSwap(fIntervals[i], fIntervals[count + i]);
            }
        }
    }

    SkASSERT(fIntervals.count() > 0);
    fCachedInterval = fIntervals.begin();
}

const SkGradientShaderBase::GradientShaderBase4fContext::Interval*
SkGradientShaderBase::
GradientShaderBase4fContext::findInterval(SkScalar fx) const {
    SkASSERT(in_range(fx, fIntervals.front().fP0, fIntervals.back().fP1));

    if (1) {
        // Linear search, using the last scanline interval as a starting point.
        SkASSERT(fCachedInterval >= fIntervals.begin());
        SkASSERT(fCachedInterval < fIntervals.end());
        const int search_dir = fDstToPos.getScaleX() >= 0 ? 1 : -1;
        while (!in_range(fx, fCachedInterval->fP0, fCachedInterval->fP1)) {
            fCachedInterval += search_dir;
            if (fCachedInterval >= fIntervals.end()) {
                fCachedInterval = fIntervals.begin();
            } else if (fCachedInterval < fIntervals.begin()) {
                fCachedInterval = fIntervals.end() - 1;
            }
        }
        return fCachedInterval;
    } else {
        // Binary search.  Seems less effective than linear + caching.
        const Interval* i0 = fIntervals.begin();
        const Interval* i1 = fIntervals.end() - 1;

        while (i0 != i1) {
            SkASSERT(i0 < i1);
            SkASSERT(in_range(fx, i0->fP0, i1->fP1));

            const Interval* i = i0 + ((i1 - i0) >> 1);

            if (in_range(fx, i0->fP0, i->fP1)) {
                i1 = i;
            } else {
                SkASSERT(in_range(fx, i->fP1, i1->fP1));
                i0 = i + 1;
            }
        }

        SkASSERT(in_range(fx, i0->fP0, i0->fP1));
        return i0;
    }
}
