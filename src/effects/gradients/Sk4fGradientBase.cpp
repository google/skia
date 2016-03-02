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

class IntervalBuilder {
public:
    IntervalBuilder(const SkColor* colors, const SkScalar* pos, int count, bool reverse)
        : fColors(colors)
        , fPos(pos)
        , fCount(count)
        , fFirstPos(reverse ? SK_Scalar1 : 0)
        , fBegin(reverse ? count - 1 : 0)
        , fAdvance(reverse ? -1 : 1) {
        SkASSERT(colors);
        SkASSERT(count > 1);
    }

    template<typename F>
    void build(F func) const {
        if (!fPos) {
            this->buildImplicitPos(func);
            return;
        }

        const int end = fBegin + fAdvance * (fCount - 1);
        const SkScalar lastPos = 1 - fFirstPos;
        int prev = fBegin;
        SkScalar prevPos = fFirstPos;

        do {
            const int curr = prev + fAdvance;
            SkASSERT(curr >= 0 && curr < fCount);

            // TODO: this sanitization should be done in SkGradientShaderBase
            const SkScalar currPos = (fAdvance > 0)
                ? SkTPin(fPos[curr], prevPos, lastPos)
                : SkTPin(fPos[curr], lastPos, prevPos);

            if (currPos != prevPos) {
                SkASSERT((currPos - prevPos > 0) == (fAdvance > 0));
                func(fColors[prev], fColors[curr], prevPos, currPos);
            }

            prev = curr;
            prevPos = currPos;
        } while (prev != end);
    }

private:
    template<typename F>
    void buildImplicitPos(F func) const {
        // When clients don't provide explicit color stop positions (fPos == nullptr),
        // the color stops are distributed evenly across the unit interval
        // (implicit positioning).
        const SkScalar dt = fAdvance * SK_Scalar1 / (fCount - 1);
        const int end = fBegin + fAdvance * (fCount - 2);
        int prev = fBegin;
        SkScalar prevPos = fFirstPos;

        while (prev != end) {
            const int curr = prev + fAdvance;
            SkASSERT(curr >= 0 && curr < fCount);

            const SkScalar currPos = prevPos + dt;
            func(fColors[prev], fColors[curr], prevPos, currPos);
            prev = curr;
            prevPos = currPos;
        }

        // emit the last interval with a pinned end position, to avoid precision issues
        func(fColors[prev], fColors[prev + fAdvance], prevPos, 1 - fFirstPos);
    }

    const SkColor*  fColors;
    const SkScalar* fPos;
    const int       fCount;
    const SkScalar  fFirstPos;
    const int       fBegin;
    const int       fAdvance;
};

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

    const bool dx_is_pos = fDstToPos.getScaleX() >= 0;
    const int first_index = dx_is_pos ? 0 : shader.fColorCount - 1;
    const int last_index = shader.fColorCount - 1 - first_index;
    const SkScalar first_pos = dx_is_pos ? 0 : SK_Scalar1;
    const SkScalar last_pos = 1 - first_pos;

    if (shader.fTileMode == SkShader::kClamp_TileMode) {
        // synthetic edge interval: -/+inf .. P0
        const SkPMColor clamp_color = pack_color(shader.fOrigColors[first_index],
                                                 fColorsArePremul);
        const SkScalar clamp_pos = dx_is_pos ? SK_ScalarMin : SK_ScalarMax;
        fIntervals.emplace_back(clamp_color, clamp_pos,
                                clamp_color, first_pos,
                                componentScale);
    } else if (shader.fTileMode == SkShader::kMirror_TileMode && !dx_is_pos) {
        // synthetic mirror intervals injected before main intervals: (2 .. 1]
        addMirrorIntervals(shader, componentScale, dx_is_pos);
    }

    const IntervalBuilder builder(shader.fOrigColors,
                                  shader.fOrigPos,
                                  shader.fColorCount,
                                  !dx_is_pos);
    builder.build([this, &componentScale] (SkColor c0, SkColor c1, SkScalar p0, SkScalar p1) {
        SkASSERT(fIntervals.empty() || fIntervals.back().fP1 == p0);

        fIntervals.emplace_back(pack_color(c0, fColorsArePremul),
                                p0,
                                pack_color(c1, fColorsArePremul),
                                p1,
                                componentScale);
    });

    if (shader.fTileMode == SkShader::kClamp_TileMode) {
        // synthetic edge interval: Pn .. +/-inf
        const SkPMColor clamp_color =
            pack_color(shader.fOrigColors[last_index], fColorsArePremul);
        const SkScalar clamp_pos = dx_is_pos ? SK_ScalarMax : SK_ScalarMin;
        fIntervals.emplace_back(clamp_color, last_pos,
                                clamp_color, clamp_pos,
                                componentScale);
    } else if (shader.fTileMode == SkShader::kMirror_TileMode && dx_is_pos) {
        // synthetic mirror intervals injected after main intervals: [1 .. 2)
        addMirrorIntervals(shader, componentScale, dx_is_pos);
    }

    SkASSERT(fIntervals.count() > 0);
    fCachedInterval = fIntervals.begin();
}

void SkGradientShaderBase::
GradientShaderBase4fContext::addMirrorIntervals(const SkGradientShaderBase& shader,
                                                const Sk4f& componentScale, bool dx_is_pos) {
    // Iterates in reverse order (vs main interval builder) and adds intervals reflected in 2.
    const IntervalBuilder builder(shader.fOrigColors,
                                  shader.fOrigPos,
                                  shader.fColorCount,
                                  dx_is_pos);
    builder.build([this, &componentScale] (SkColor c0, SkColor c1, SkScalar p0, SkScalar p1) {
        SkASSERT(fIntervals.empty() || fIntervals.back().fP1 == 2 - p0);

        fIntervals.emplace_back(pack_color(c0, fColorsArePremul),
                                2 - p0,
                                pack_color(c1, fColorsArePremul),
                                2 - p1,
                                componentScale);
    });
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
