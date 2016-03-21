/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sk4fGradientBase.h"

#include <functional>

namespace {

SkPMColor pack_color(SkColor c, bool premul) {
    return premul
        ? SkPreMultiplyColor(c)
        : SkPackARGB32NoCheck(SkColorGetA(c), SkColorGetR(c), SkColorGetG(c), SkColorGetB(c));
}

template<SkShader::TileMode>
SkScalar tileProc(SkScalar t);

template<>
SkScalar tileProc<SkShader::kClamp_TileMode>(SkScalar t) {
    // synthetic clamp-mode edge intervals allow for a free-floating t:
    //   [-inf..0)[0..1)[1..+inf)
    return t;
}

template<>
SkScalar tileProc<SkShader::kRepeat_TileMode>(SkScalar t) {
    // t % 1  (intervals range: [0..1))
    return t - SkScalarFloorToScalar(t);
}

template<>
SkScalar tileProc<SkShader::kMirror_TileMode>(SkScalar t) {
    // t % 2  (synthetic mirror intervals expand the range to [0..2)
    return t - SkScalarFloorToScalar(t / 2) * 2;
}

class IntervalIterator {
public:
    IntervalIterator(const SkColor* colors, const SkScalar* pos, int count, bool reverse)
        : fColors(colors)
        , fPos(pos)
        , fCount(count)
        , fFirstPos(reverse ? SK_Scalar1 : 0)
        , fBegin(reverse ? count - 1 : 0)
        , fAdvance(reverse ? -1 : 1) {
        SkASSERT(colors);
        SkASSERT(count > 0);
    }

    void iterate(std::function<void(SkColor, SkColor, SkScalar, SkScalar)> func) const {
        if (!fPos) {
            this->iterateImplicitPos(func);
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
    void iterateImplicitPos(std::function<void(SkColor, SkColor, SkScalar, SkScalar)> func) const {
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

    const Sk4f c4f0 = SkPM4f::FromPMColor(c0).to4f() * componentScale;
    const Sk4f c4f1 = SkPM4f::FromPMColor(c1).to4f() * componentScale;
    const Sk4f dc4f = (c4f1 - c4f0) / (p1 - p0);

    c4f0.store(&fC0.fVec);
    dc4f.store(&fDc.fVec);
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
}

void SkGradientShaderBase::
GradientShaderBase4fContext::buildIntervals(const SkGradientShaderBase& shader,
                                            const ContextRec& rec, bool reverse) {
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

    SkASSERT(shader.fColorCount > 0);
    SkASSERT(shader.fOrigColors);

    const float paintAlpha = rec.fPaint->getAlpha() * (1.0f / 255);
    const Sk4f componentScale = fColorsArePremul
        ? Sk4f(paintAlpha)
        : Sk4f(1.0f, 1.0f, 1.0f, paintAlpha);
    const int first_index = reverse ? shader.fColorCount - 1 : 0;
    const int last_index = shader.fColorCount - 1 - first_index;
    const SkScalar first_pos = reverse ? SK_Scalar1 : 0;
    const SkScalar last_pos = SK_Scalar1 - first_pos;

    if (shader.fTileMode == SkShader::kClamp_TileMode) {
        // synthetic edge interval: -/+inf .. P0
        const SkPMColor clamp_color = pack_color(shader.fOrigColors[first_index],
                                                 fColorsArePremul);
        const SkScalar clamp_pos = reverse ? SK_ScalarMax : SK_ScalarMin;
        fIntervals.emplace_back(clamp_color, clamp_pos,
                                clamp_color, first_pos,
                                componentScale);
    } else if (shader.fTileMode == SkShader::kMirror_TileMode && reverse) {
        // synthetic mirror intervals injected before main intervals: (2 .. 1]
        addMirrorIntervals(shader, componentScale, false);
    }

    const IntervalIterator iter(shader.fOrigColors,
                                shader.fOrigPos,
                                shader.fColorCount,
                                reverse);
    iter.iterate([this, &componentScale] (SkColor c0, SkColor c1, SkScalar p0, SkScalar p1) {
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
        const SkScalar clamp_pos = reverse ? SK_ScalarMin : SK_ScalarMax;
        fIntervals.emplace_back(clamp_color, last_pos,
                                clamp_color, clamp_pos,
                                componentScale);
    } else if (shader.fTileMode == SkShader::kMirror_TileMode && !reverse) {
        // synthetic mirror intervals injected after main intervals: [1 .. 2)
        addMirrorIntervals(shader, componentScale, true);
    }
}

void SkGradientShaderBase::
GradientShaderBase4fContext::addMirrorIntervals(const SkGradientShaderBase& shader,
                                            const Sk4f& componentScale, bool reverse) {
    const IntervalIterator iter(shader.fOrigColors,
                                shader.fOrigPos,
                                shader.fColorCount,
                                reverse);
    iter.iterate([this, &componentScale] (SkColor c0, SkColor c1, SkScalar p0, SkScalar p1) {
        SkASSERT(fIntervals.empty() || fIntervals.back().fP1 == 2 - p0);

        fIntervals.emplace_back(pack_color(c0, fColorsArePremul),
                                2 - p0,
                                pack_color(c1, fColorsArePremul),
                                2 - p1,
                                componentScale);
    });
}

void SkGradientShaderBase::
GradientShaderBase4fContext::shadeSpan(int x, int y, SkPMColor dst[], int count) {
    if (fColorsArePremul) {
        this->shadePremulSpan<DstType::L32, ApplyPremul::False>(x, y, dst, count);
    } else {
        this->shadePremulSpan<DstType::L32, ApplyPremul::True>(x, y, dst, count);
    }
}

void SkGradientShaderBase::
GradientShaderBase4fContext::shadeSpan4f(int x, int y, SkPM4f dst[], int count) {
    if (fColorsArePremul) {
        this->shadePremulSpan<DstType::F32, ApplyPremul::False>(x, y, dst, count);
    } else {
        this->shadePremulSpan<DstType::F32, ApplyPremul::True>(x, y, dst, count);
    }
}

template<DstType dstType, ApplyPremul premul>
void SkGradientShaderBase::
GradientShaderBase4fContext::shadePremulSpan(int x, int y,
                                             typename DstTraits<dstType, premul>::Type dst[],
                                             int count) const {
    const SkGradientShaderBase& shader =
        static_cast<const SkGradientShaderBase&>(fShader);

    switch (shader.fTileMode) {
    case kClamp_TileMode:
        this->shadeSpanInternal<dstType,
                                premul,
                                kClamp_TileMode>(x, y, dst, count);
        break;
    case kRepeat_TileMode:
        this->shadeSpanInternal<dstType,
                                premul,
                                kRepeat_TileMode>(x, y, dst, count);
        break;
    case kMirror_TileMode:
        this->shadeSpanInternal<dstType,
                                premul,
                                kMirror_TileMode>(x, y, dst, count);
        break;
    }
}

template<DstType dstType, ApplyPremul premul, SkShader::TileMode tileMode>
void SkGradientShaderBase::
GradientShaderBase4fContext::shadeSpanInternal(int x, int y,
                                               typename DstTraits<dstType, premul>::Type dst[],
                                               int count) const {
    static const int kBufSize = 128;
    SkScalar ts[kBufSize];
    TSampler<dstType, tileMode> sampler(*this);

    SkASSERT(count > 0);
    do {
        const int n = SkTMin(kBufSize, count);
        this->mapTs(x, y, ts, n);
        for (int i = 0; i < n; ++i) {
            const Sk4f c = sampler.sample(ts[i]);
            DstTraits<dstType, premul>::store(c, dst++);
        }
        x += n;
        count -= n;
    } while (count > 0);
}

template<DstType dstType, SkShader::TileMode tileMode>
class SkGradientShaderBase::GradientShaderBase4fContext::TSampler {
public:
    TSampler(const GradientShaderBase4fContext& ctx)
        : fFirstInterval(ctx.fIntervals.begin())
        , fLastInterval(ctx.fIntervals.end() - 1)
        , fInterval(nullptr) {
        SkASSERT(fLastInterval >= fFirstInterval);
    }

    Sk4f sample(SkScalar t) {
        const SkScalar tiled_t = tileProc<tileMode>(t);

        if (!fInterval) {
            // Very first sample => locate the initial interval.
            // TODO: maybe do this in ctor to remove a branch?
            fInterval = this->findFirstInterval(tiled_t);
            this->loadIntervalData(fInterval);
        } else if (tiled_t < fInterval->fP0 || tiled_t >= fInterval->fP1) {
            fInterval = this->findNextInterval(t, tiled_t);
            this->loadIntervalData(fInterval);
        }

        fPrevT = t;
        return lerp(tiled_t);
    }

private:
    Sk4f lerp(SkScalar t) {
        SkASSERT(t >= fInterval->fP0 && t < fInterval->fP1);
        return fCc + fDc * (t - fInterval->fP0);
    }

    const Interval* findFirstInterval(SkScalar t) const {
        // Binary search.
        const Interval* i0 = fFirstInterval;
        const Interval* i1 = fLastInterval;

        while (i0 != i1) {
            SkASSERT(i0 < i1);
            SkASSERT(t >= i0->fP0 && t < i1->fP1);

            const Interval* i = i0 + ((i1 - i0) >> 1);

            if (t >= i->fP1) {
                i0 = i + 1;
            } else {
                i1 = i;
            }
        }

        SkASSERT(t >= i0->fP0 && t <= i0->fP1);
        return i0;
    }

    const Interval* findNextInterval(SkScalar t, SkScalar tiled_t) const {
        SkASSERT(tiled_t < fInterval->fP0 || tiled_t >= fInterval->fP1);
        SkASSERT(tiled_t >= fFirstInterval->fP0 && tiled_t < fLastInterval->fP1);

        const Interval* i = fInterval;

        // Use the t vs. prev_t signal to figure which direction we should search for
        // the next interval, then perform a linear search.
        if (t >= fPrevT) {
            do {
                i += 1;
                if (i > fLastInterval) {
                    i = fFirstInterval;
                }
            } while (tiled_t < i->fP0 || tiled_t >= i->fP1);
        } else {
            do {
                i -= 1;
                if (i < fFirstInterval) {
                    i = fLastInterval;
                }
            } while (tiled_t < i->fP0 || tiled_t >= i->fP1);
        }

        return i;
    }

    void loadIntervalData(const Interval* i) {
        fCc = DstTraits<dstType>::load(i->fC0);
        fDc = DstTraits<dstType>::load(i->fDc);
    }

    const Interval* fFirstInterval;
    const Interval* fLastInterval;
    const Interval* fInterval;
    SkScalar        fPrevT;
    Sk4f            fCc;
    Sk4f            fDc;
};
