/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/Sk1DPathEffect.h"

#include "include/core/SkFlattenable.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPathMeasure.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStrokeRec.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFloatingPoint.h"
#include "src/core/SkPathEffectBase.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

struct SkRect;

// Since we are stepping by a float, the do/while loop might go on forever (or nearly so).
// Put in a governor to limit crash values from looping too long (and allocating too much ram).
#define MAX_REASONABLE_ITERATIONS   100000

class Sk1DPathEffect : public SkPathEffectBase {
public:
protected:
    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*,
                      const SkMatrix&) const override {
        SkPathMeasure   meas(src, false);
        do {
            int governor = MAX_REASONABLE_ITERATIONS;
            SkScalar    length = meas.getLength();
            SkScalar    distance = this->begin(length);
            while (distance < length && --governor >= 0) {
                SkScalar delta = this->next(dst, distance, meas);
                if (delta <= 0) {
                    break;
                }
                distance += delta;
            }
            if (governor < 0) {
                return false;
            }
        } while (meas.nextContour());
        return true;
    }

    /** Called at the start of each contour, returns the initial offset
        into that contour.
    */
    virtual SkScalar begin(SkScalar contourLength) const = 0;
    /** Called with the current distance along the path, with the current matrix
        for the point/tangent at the specified distance.
        Return the distance to travel for the next call. If return <= 0, then that
        contour is done.
    */
    virtual SkScalar next(SkPath* dst, SkScalar dist, SkPathMeasure&) const = 0;

private:
    // For simplicity, assume fast bounds cannot be computed
    bool computeFastBounds(SkRect*) const override { return false; }
};

///////////////////////////////////////////////////////////////////////////////

class SkPath1DPathEffectImpl : public Sk1DPathEffect {
public:
    SkPath1DPathEffectImpl(const SkPath& path, SkScalar advance, SkScalar phase,
                           SkPath1DPathEffect::Style style) : fPath(path) {
        SkASSERT(advance > 0 && !path.isEmpty());

        // Make the path thread-safe.
        fPath.updateBoundsCache();
        (void)fPath.getGenerationID();

        // cleanup their phase parameter, inverting it so that it becomes an
        // offset along the path (to match the interpretation in PostScript)
        if (phase < 0) {
            phase = -phase;
            if (phase > advance) {
                phase = SkScalarMod(phase, advance);
            }
        } else {
            if (phase > advance) {
                phase = SkScalarMod(phase, advance);
            }
            phase = advance - phase;
        }
        // now catch the edge case where phase == advance (within epsilon)
        if (phase >= advance) {
            phase = 0;
        }
        SkASSERT(phase >= 0);

        fAdvance = advance;
        fInitialOffset = phase;
        fStyle = style;
    }

    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec* rec,
                      const SkRect* cullRect, const SkMatrix& ctm) const override {
        rec->setFillStyle();
        return this->INHERITED::onFilterPath(dst, src, rec, cullRect, ctm);
    }

    SkScalar begin(SkScalar contourLength) const override {
        return fInitialOffset;
    }

    SkScalar next(SkPath*, SkScalar, SkPathMeasure&) const override;

    static sk_sp<SkFlattenable> CreateProc(SkReadBuffer& buffer) {
        SkScalar advance = buffer.readScalar();
        SkPath path;
        buffer.readPath(&path);
        SkScalar phase = buffer.readScalar();
        SkPath1DPathEffect::Style style = buffer.read32LE(SkPath1DPathEffect::kLastEnum_Style);
        return buffer.isValid() ? SkPath1DPathEffect::Make(path, advance, phase, style) : nullptr;
    }

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeScalar(fAdvance);
        buffer.writePath(fPath);
        buffer.writeScalar(fInitialOffset);
        buffer.writeUInt(fStyle);
    }

    Factory getFactory() const override { return CreateProc; }
    const char* getTypeName() const override { return "SkPath1DPathEffect"; }

private:
    SkPath                      fPath;          // copied from constructor
    SkScalar                    fAdvance;       // copied from constructor
    SkScalar                    fInitialOffset; // computed from phase
    SkPath1DPathEffect::Style   fStyle;         // copied from constructor

    using INHERITED = Sk1DPathEffect;
};

static bool morphpoints(SkPoint dst[], const SkPoint src[], int count,
                        SkPathMeasure& meas, SkScalar dist) {
    for (int i = 0; i < count; i++) {
        SkPoint pos;
        SkVector tangent;

        SkScalar sx = src[i].fX;
        SkScalar sy = src[i].fY;

        if (!meas.getPosTan(dist + sx, &pos, &tangent)) {
            return false;
        }

        SkMatrix    matrix;
        SkPoint     pt;

        pt.set(sx, sy);
        matrix.setSinCos(tangent.fY, tangent.fX, 0, 0);
        matrix.preTranslate(-sx, 0);
        matrix.postTranslate(pos.fX, pos.fY);
        matrix.mapPoints(&dst[i], &pt, 1);
    }
    return true;
}

/*  TODO

Need differentially more subdivisions when the follow-path is curvy. Not sure how to
determine that, but we need it. I guess a cheap answer is let the caller tell us,
but that seems like a cop-out. Another answer is to get Rob Johnson to figure it out.
*/
static void morphpath(SkPath* dst, const SkPath& src, SkPathMeasure& meas,
                      SkScalar dist) {
    SkPath::Iter    iter(src, false);
    SkPoint         srcP[4], dstP[3];
    SkPath::Verb    verb;

    while ((verb = iter.next(srcP)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                if (morphpoints(dstP, srcP, 1, meas, dist)) {
                    dst->moveTo(dstP[0]);
                }
                break;
            case SkPath::kLine_Verb:
                srcP[2] = srcP[1];
                srcP[1].set(SkScalarAve(srcP[0].fX, srcP[2].fX),
                            SkScalarAve(srcP[0].fY, srcP[2].fY));
                [[fallthrough]];
            case SkPath::kQuad_Verb:
                if (morphpoints(dstP, &srcP[1], 2, meas, dist)) {
                    dst->quadTo(dstP[0], dstP[1]);
                }
                break;
            case SkPath::kConic_Verb:
                if (morphpoints(dstP, &srcP[1], 2, meas, dist)) {
                    dst->conicTo(dstP[0], dstP[1], iter.conicWeight());
                }
                break;
            case SkPath::kCubic_Verb:
                if (morphpoints(dstP, &srcP[1], 3, meas, dist)) {
                    dst->cubicTo(dstP[0], dstP[1], dstP[2]);
                }
                break;
            case SkPath::kClose_Verb:
                dst->close();
                break;
            default:
                SkDEBUGFAIL("unknown verb");
                break;
        }
    }
}

SkScalar SkPath1DPathEffectImpl::next(SkPath* dst, SkScalar distance,
                                      SkPathMeasure& meas) const {
#if defined(SK_BUILD_FOR_FUZZER)
    if (dst->countPoints() > 100000) {
        return fAdvance;
    }
#endif
    switch (fStyle) {
        case SkPath1DPathEffect::kTranslate_Style: {
            SkPoint pos;
            if (meas.getPosTan(distance, &pos, nullptr)) {
                dst->addPath(fPath, pos.fX, pos.fY);
            }
        } break;
        case SkPath1DPathEffect::kRotate_Style: {
            SkMatrix matrix;
            if (meas.getMatrix(distance, &matrix)) {
                dst->addPath(fPath, matrix);
            }
        } break;
        case SkPath1DPathEffect::kMorph_Style:
            morphpath(dst, fPath, meas, distance);
            break;
    }
    return fAdvance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkPathEffect> SkPath1DPathEffect::Make(const SkPath& path, SkScalar advance, SkScalar phase,
                                             Style style) {
    if (advance <= 0 || !SkIsFinite(advance, phase) || path.isEmpty()) {
        return nullptr;
    }
    return sk_sp<SkPathEffect>(new SkPath1DPathEffectImpl(path, advance, phase, style));
}

void SkPath1DPathEffect::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkPath1DPathEffectImpl);
}
