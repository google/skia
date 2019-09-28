/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/core/SkPathMeasure.h"
#include "include/core/SkStrokeRec.h"
#include "include/effects/Sk1DPathEffect.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

// Since we are stepping by a float, the do/while loop might go on forever (or nearly so).
// Put in a governor to limit crash values from looping too long (and allocating too much ram).
#define MAX_REASONABLE_ITERATIONS   100000

bool Sk1DPathEffect::onFilterPath(SkPath* dst, const SkPath& src,
                                  SkStrokeRec*, const SkRect*) const {
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
    } while (meas.nextContour());
    return true;
}

///////////////////////////////////////////////////////////////////////////////

SkPath1DPathEffect::SkPath1DPathEffect(const SkPath& path, SkScalar advance, SkScalar phase,
                                       Style style) : fPath(path) {
    SkASSERT(advance > 0 && !path.isEmpty());
    SkASSERT((unsigned)style <= kMorph_Style);

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

    if ((unsigned)style > kMorph_Style) {
        SkDEBUGF("SkPath1DPathEffect style enum out of range %d\n", style);
    }
    fStyle = style;
}

bool SkPath1DPathEffect::onFilterPath(SkPath* dst, const SkPath& src,
                                      SkStrokeRec* rec, const SkRect* cullRect) const {
    rec->setFillStyle();
    return this->INHERITED::onFilterPath(dst, src, rec, cullRect);
}

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
                // fall through to quad
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

SkScalar SkPath1DPathEffect::begin(SkScalar contourLength) const {
    return fInitialOffset;
}

sk_sp<SkFlattenable> SkPath1DPathEffect::CreateProc(SkReadBuffer& buffer) {
    SkScalar advance = buffer.readScalar();
    SkPath path;
    buffer.readPath(&path);
    SkScalar phase = buffer.readScalar();
    Style style = buffer.read32LE(kLastEnum_Style);
    return buffer.isValid() ? SkPath1DPathEffect::Make(path, advance, phase, style) : nullptr;
}

void SkPath1DPathEffect::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fAdvance);
    buffer.writePath(fPath);
    buffer.writeScalar(fInitialOffset);
    buffer.writeUInt(fStyle);
}

SkScalar SkPath1DPathEffect::next(SkPath* dst, SkScalar distance,
                                  SkPathMeasure& meas) const {
#if defined(IS_FUZZING_WITH_LIBFUZZER)
    if (dst->countPoints() > 100000) {
        return fAdvance;
    }
#endif
    switch (fStyle) {
        case kTranslate_Style: {
            SkPoint pos;
            if (meas.getPosTan(distance, &pos, nullptr)) {
                dst->addPath(fPath, pos.fX, pos.fY);
            }
        } break;
        case kRotate_Style: {
            SkMatrix matrix;
            if (meas.getMatrix(distance, &matrix)) {
                dst->addPath(fPath, matrix);
            }
        } break;
        case kMorph_Style:
            morphpath(dst, fPath, meas, distance);
            break;
        default:
            SkDEBUGFAIL("unknown Style enum");
            break;
    }
    return fAdvance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkPathEffect> SkPath1DPathEffect::Make(const SkPath& path, SkScalar advance, SkScalar phase,
                                             Style style) {
    if (advance <= 0 || !SkScalarIsFinite(advance) || !SkScalarIsFinite(phase) || path.isEmpty()) {
        return nullptr;
    }
    return sk_sp<SkPathEffect>(new SkPath1DPathEffect(path, advance, phase, style));
}
