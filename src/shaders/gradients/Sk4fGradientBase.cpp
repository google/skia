/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPaint.h"
#include "src/shaders/gradients/Sk4fGradientBase.h"
#include <functional>

namespace {

Sk4f pack_color(const SkColor4f& c4f, bool premul, const Sk4f& component_scale) {
    Sk4f pm4f = premul
        ? Sk4f::Load(c4f.premul().vec())
        : Sk4f::Load(c4f.vec());

    if (premul) {
        // If the stops are premul, we clamp them to gamut now.
        // If the stops are unpremul, the colors will eventually go through Sk4f_toL32(),
        // which ends up clamping to gamut then.
        pm4f = Sk4f::Max(0, Sk4f::Min(pm4f, pm4f[3]));
    }

    return pm4f * component_scale;
}

class IntervalIterator {
public:
    IntervalIterator(const SkGradientShaderBase& shader, bool reverse)
        : fShader(shader)
        , fFirstPos(reverse ? SK_Scalar1 : 0)
        , fBegin(reverse ? shader.fColorCount - 1 : 0)
        , fAdvance(reverse ? -1 : 1) {
        SkASSERT(shader.fColorCount > 0);
    }

    void iterate(const SkColor4f* colors,
                 std::function<void(const SkColor4f&, const SkColor4f&,
                                    SkScalar, SkScalar)> func) const {
        if (!fShader.fOrigPos) {
            this->iterateImplicitPos(colors, func);
            return;
        }

        const int end = fBegin + fAdvance * (fShader.fColorCount - 1);
        int prev = fBegin;
        SkScalar prevPos = fFirstPos;

        do {
            const int curr = prev + fAdvance;
            SkASSERT(curr >= 0 && curr < fShader.fColorCount);

            const SkScalar currPos = fShader.fOrigPos[curr];
            if (currPos != prevPos) {
                SkASSERT((currPos - prevPos > 0) == (fAdvance > 0));
                func(colors[prev], colors[curr], prevPos, currPos);
            }

            prev = curr;
            prevPos = currPos;
        } while (prev != end);
    }

private:
    void iterateImplicitPos(const SkColor4f* colors,
                            std::function<void(const SkColor4f&, const SkColor4f&,
                                               SkScalar, SkScalar)> func) const {
        // When clients don't provide explicit color stop positions (fPos == nullptr),
        // the color stops are distributed evenly across the unit interval
        // (implicit positioning).
        const SkScalar dt = fAdvance * SK_Scalar1 / (fShader.fColorCount - 1);
        const int end = fBegin + fAdvance * (fShader.fColorCount - 2);
        int prev = fBegin;
        SkScalar prevPos = fFirstPos;

        while (prev != end) {
            const int curr = prev + fAdvance;
            SkASSERT(curr >= 0 && curr < fShader.fColorCount);

            const SkScalar currPos = prevPos + dt;
            func(colors[prev], colors[curr], prevPos, currPos);
            prev = curr;
            prevPos = currPos;
        }

        // emit the last interval with a pinned end position, to avoid precision issues
        func(colors[prev], colors[prev + fAdvance], prevPos, 1 - fFirstPos);
    }

    const SkGradientShaderBase& fShader;
    const SkScalar              fFirstPos;
    const int                   fBegin;
    const int                   fAdvance;
};

void addMirrorIntervals(const SkGradientShaderBase& shader,
                        const SkColor4f* colors,
                        const Sk4f& componentScale,
                        bool premulColors, bool reverse,
                        Sk4fGradientIntervalBuffer::BufferType* buffer) {
    const IntervalIterator iter(shader, reverse);
    iter.iterate(colors, [&] (const SkColor4f& c0, const SkColor4f& c1, SkScalar t0, SkScalar t1) {
        SkASSERT(buffer->empty() || buffer->back().fT1 == 2 - t0);

        const auto mirror_t0 = 2 - t0;
        const auto mirror_t1 = 2 - t1;
        // mirror_p1 & mirror_p1 may collapse for very small values - recheck to avoid
        // triggering Interval asserts.
        if (mirror_t0 != mirror_t1) {
            buffer->emplace_back(pack_color(c0, premulColors, componentScale), mirror_t0,
                                 pack_color(c1, premulColors, componentScale), mirror_t1);
        }
    });
}

} // anonymous namespace

Sk4fGradientInterval::Sk4fGradientInterval(const Sk4f& c0, SkScalar t0,
                                           const Sk4f& c1, SkScalar t1)
    : fT0(t0)
    , fT1(t1) {
    SkASSERT(t0 != t1);
    // Either p0 or p1 can be (-)inf for synthetic clamp edge intervals.
    SkASSERT(SkScalarIsFinite(t0) || SkScalarIsFinite(t1));

    const auto dt = t1 - t0;

    // Clamp edge intervals are always zero-ramp.
    SkASSERT(SkScalarIsFinite(dt) || (c0 == c1).allTrue());
    SkASSERT(SkScalarIsFinite(t0) || (c0 == c1).allTrue());
    const Sk4f   dc = SkScalarIsFinite(dt) ? (c1 - c0) / dt : 0;
    const Sk4f bias = c0 - (SkScalarIsFinite(t0) ? t0 * dc : 0);

    bias.store(fCb.vec());
    dc.store(fCg.vec());
}

void Sk4fGradientIntervalBuffer::init(const SkGradientShaderBase& shader, SkColorSpace* dstCS,
                                      SkTileMode tileMode, bool premulColors,
                                      SkScalar alpha, bool reverse) {
    // The main job here is to build a specialized interval list: a different
    // representation of the color stops data, optimized for efficient scan line
    // access during shading.
    //
    //   [{P0,C0} , {P1,C1}) [{P1,C2} , {P2,c3}) ... [{Pn,C2n} , {Pn+1,C2n+1})
    //
    // The list may be inverted when requested (such that e.g. points are sorted
    // in increasing x order when dx < 0).
    //
    // Note: the current representation duplicates pos data; we could refactor to
    //       avoid this if interval storage size becomes a concern.
    //
    // Aside from reordering, we also perform two more pre-processing steps at
    // this stage:
    //
    //   1) scale the color components depending on paint alpha and the requested
    //      interpolation space (note: the interval color storage is SkPMColor4f, but
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

    const auto count = shader.fColorCount;

    SkASSERT(count > 0);

    fIntervals.reset();

    const Sk4f componentScale = premulColors
        ? Sk4f(alpha)
        : Sk4f(1.0f, 1.0f, 1.0f, alpha);
    const int first_index = reverse ? count - 1 : 0;
    const int last_index = count - 1 - first_index;
    const SkScalar first_pos = reverse ? SK_Scalar1 : 0;
    const SkScalar last_pos = SK_Scalar1 - first_pos;

    // Transform all of the colors to destination color space
    SkColor4fXformer xformedColors(shader.fOrigColors4f, count, shader.fColorSpace.get(), dstCS);

    if (tileMode == SkTileMode::kClamp) {
        // synthetic edge interval: -/+inf .. P0
        const Sk4f clamp_color = pack_color(xformedColors.fColors[first_index],
                                            premulColors, componentScale);
        const SkScalar clamp_pos = reverse ? SK_ScalarInfinity : SK_ScalarNegativeInfinity;
        fIntervals.emplace_back(clamp_color, clamp_pos,
                                clamp_color, first_pos);
    } else if (tileMode == SkTileMode::kMirror && reverse) {
        // synthetic mirror intervals injected before main intervals: (2 .. 1]
        addMirrorIntervals(shader, xformedColors.fColors, componentScale, premulColors, false,
                           &fIntervals);
    }

    const IntervalIterator iter(shader, reverse);
    iter.iterate(xformedColors.fColors,
                 [&] (const SkColor4f& c0, const SkColor4f& c1, SkScalar t0, SkScalar t1) {
        SkASSERT(fIntervals.empty() || fIntervals.back().fT1 == t0);

        fIntervals.emplace_back(pack_color(c0, premulColors, componentScale), t0,
                                pack_color(c1, premulColors, componentScale), t1);
    });

    if (tileMode == SkTileMode::kClamp) {
        // synthetic edge interval: Pn .. +/-inf
        const Sk4f clamp_color = pack_color(xformedColors.fColors[last_index],
                                            premulColors, componentScale);
        const SkScalar clamp_pos = reverse ? SK_ScalarNegativeInfinity : SK_ScalarInfinity;
        fIntervals.emplace_back(clamp_color, last_pos,
                                clamp_color, clamp_pos);
    } else if (tileMode == SkTileMode::kMirror && !reverse) {
        // synthetic mirror intervals injected after main intervals: [1 .. 2)
        addMirrorIntervals(shader, xformedColors.fColors, componentScale, premulColors, true,
                           &fIntervals);
    }
}

const Sk4fGradientInterval* Sk4fGradientIntervalBuffer::find(SkScalar t) const {
    // Binary search.
    const auto* i0 = fIntervals.begin();
    const auto* i1 = fIntervals.end() - 1;

    while (i0 != i1) {
        SkASSERT(i0 < i1);
        SkASSERT(t >= i0->fT0 && t <= i1->fT1);

        const auto* i = i0 + ((i1 - i0) >> 1);

        if (t > i->fT1) {
            i0 = i + 1;
        } else {
            i1 = i;
        }
    }

    SkASSERT(i0->contains(t));
    return i0;
}

const Sk4fGradientInterval* Sk4fGradientIntervalBuffer::findNext(
    SkScalar t, const Sk4fGradientInterval* prev, bool increasing) const {

    SkASSERT(!prev->contains(t));
    SkASSERT(prev >= fIntervals.begin() && prev < fIntervals.end());
    SkASSERT(t >= fIntervals.front().fT0 && t <= fIntervals.back().fT1);

    const auto* i = prev;

    // Use the |increasing| signal to figure which direction we should search for
    // the next interval, then perform a linear search.
    if (increasing) {
        do {
            i += 1;
            if (i >= fIntervals.end()) {
                i = fIntervals.begin();
            }
        } while (!i->contains(t));
    } else {
        do {
            i -= 1;
            if (i < fIntervals.begin()) {
                i = fIntervals.end() - 1;
            }
        } while (!i->contains(t));
    }

    return i;
}

SkGradientShaderBase::
GradientShaderBase4fContext::GradientShaderBase4fContext(const SkGradientShaderBase& shader,
                                                         const ContextRec& rec)
    : INHERITED(shader, rec)
    , fFlags(this->INHERITED::getFlags())
    , fDither(rec.fPaint->isDither())
{
    const SkMatrix& inverse = this->getTotalInverse();
    fDstToPos.setConcat(shader.fPtsToUnit, inverse);
    SkASSERT(!fDstToPos.hasPerspective());
    fDstToPosProc = SkMatrixPriv::GetMapXYProc(fDstToPos);

    if (shader.fColorsAreOpaque && this->getPaintAlpha() == SK_AlphaOPAQUE) {
        fFlags |= kOpaqueAlpha_Flag;
    }

    fColorsArePremul =
        (shader.fGradFlags & SkGradientShader::kInterpolateColorsInPremul_Flag)
        || shader.fColorsAreOpaque;
}

bool SkGradientShaderBase::
GradientShaderBase4fContext::isValid() const {
    return fDstToPos.isFinite();
}
