/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeIterator_DEFINED
#define GrStrokeIterator_DEFINED

#include "include/core/SkPaint.h"
#include "include/core/SkStrokeRec.h"
#include "src/core/SkPathPriv.h"

class GrStrokeIterator {
public:
    GrStrokeIterator(const SkPath& path, const SkStrokeRec& stroke)
            : fCapType(stroke.getCap()), fStrokeRadius(stroke.getWidth() * .5) {
        SkPathPriv::Iterate it(path);
        fIter = it.begin();
        fEnd = it.end();
    }

    enum class Verb {
        kLine = (int)SkPathVerb::kLine,
        kQuad = (int)SkPathVerb::kQuad,
        kCubic = (int)SkPathVerb::kCubic,

        kMoveWithinContour = (int)SkPathVerb::kMove,
        kContourFinished = (int)SkPathVerb::kClose,
        kCusp
    };

    Verb prevVerb() const { return this->atVerb(0); }
    const SkPoint* prevPts() const { return this->atPts(0); }

    Verb nextVerb() const { return this->atVerb(1); }
    const SkPoint* nextPts() const { return this->atPts(1); }

    Verb firstVerb() const { SkASSERT(fQueueCount > 0); return fFirstVerb; }
    const SkPoint* firstPts() const { SkASSERT(fQueueCount > 0); return fFirstPts; }

    bool advance() {
        if (fQueueCount) {
            SkASSERT(fQueueCount >= 2);
            this->popFront();
            if (fQueueCount >= 2) {
                return true;
            }
            SkASSERT(fQueueCount == 1);
            if (this->atVerb(0) == Verb::kContourFinished) {
                // Don't let "kContourFinished" be prevVerb at the start of the next contour.
                fQueueCount = 0;
            }
        }
        for (; fIter != fEnd; ++fIter) {
            SkASSERT(fQueueCount == 0 || fQueueCount == 1);
            auto [verb, pts, w] = *fIter;
            switch (verb) {
                case SkPathVerb::kMove:
                    if (!this->enqueueCapsAndFirstStroke()) {
                        continue;
                    }
                    break;
                case SkPathVerb::kCubic:
                    if (pts[3] == pts[2]) {
                        [[fallthrough]];  // i.e., "if (p3 == p2 && p2 == p1 && p1 == p0)"
                case SkPathVerb::kQuad:
                    if (pts[2] == pts[1]) {
                        [[fallthrough]];  // i.e., "if (p2 == p1 && p1 == p0)"
                case SkPathVerb::kLine:
                    if (pts[1] == pts[0]) {
                        fLastDegenerateStrokePt = pts;
                        continue;
                    }}}
                    this->enqueue((Verb)verb, pts);
                    if (fQueueCount == 1) {
                        fFirstVerb = (Verb)verb;
                        fFirstPts = pts;
                        continue;
                    }
                    break;
                case SkPathVerb::kClose:
                    if (!fQueueCount) {
                        fLastDegenerateStrokePt = pts;
                        continue;
                    }
                    if (pts[0] != fFirstPts[0]) {
                        // Draw a line back to the contour's starting point.
                        fClosePts = {pts[0], fFirstPts[0]};
                        this->enqueue(Verb::kLine, fClosePts.data());
                    }
                    // Repeat the first verb, this time as the "next" stroke instead of the "prev".
                    this->enqueue(fFirstVerb, fFirstPts);
                    this->enqueue(Verb::kContourFinished, nullptr);
                    fLastDegenerateStrokePt = nullptr;
                    break;
                case SkPathVerb::kConic:
                    SkUNREACHABLE;
            }
            SkASSERT(fQueueCount >= 2);
            ++fIter;
            return true;
        }
        return this->enqueueCapsAndFirstStroke();
    }

private:
    constexpr static int kQueueBufferCount = 8;
    Verb atVerb(int i) const {
        SkASSERT(0 <= i && i < fQueueCount);
        return fVerbs[(fQueueIdx + i) & (kQueueBufferCount - 1)];
    }
    Verb backVerb() const {
        return this->atVerb(fQueueCount - 1);
    }
    const SkPoint* atPts(int i) const {
        SkASSERT(0 <= i && i < fQueueCount);
        return fPts[(fQueueIdx + i) & (kQueueBufferCount - 1)];
    }
    const SkPoint* backPts() const {
        return this->atPts(fQueueCount - 1);
    }
    void enqueue(Verb verb, const SkPoint* pts) {
        SkASSERT(fQueueCount < kQueueBufferCount);
        int i = (fQueueIdx + fQueueCount) & (kQueueBufferCount - 1);
        fVerbs[i] = verb;
        fPts[i] = pts;
        ++fQueueCount;
    }
    void popFront() {
        SkASSERT(fQueueCount > 0);
        ++fQueueIdx;
        --fQueueCount;
    }

    bool enqueueCapsAndFirstStroke() {
        if (fQueueCount) {
            switch (fCapType) {
                SkASSERT(this->backVerb() == Verb::kLine || this->backVerb() == Verb::kQuad ||
                         this->backVerb() == Verb::kCubic);
                case SkPaint::kButt_Cap:
                    // No caps, but inject a "move" so the first stroke doesn't get joined with the
                    // end of the contour when it's processed.
                    this->enqueue(Verb::kMoveWithinContour, fFirstPts);
                    break;
                case SkPaint::kRound_Cap: {
                    // The "kCusp" verb serves as our barrier to prevent the first stroke from
                    // getting joined with the end of the contour. We just need to make sure that
                    // the first point of the contour goes last.
                    int backIdx = SkPathPriv::PtsInIter((unsigned)this->backVerb()) - 1;
                    this->enqueue(Verb::kCusp, this->backPts() + backIdx);
                    this->enqueue(Verb::kCusp, fFirstPts);
                    break;
                }
                case SkPaint::kSquare_Cap:
                    this->fillSquareCapPoints();  // Fills in fEndingCapPts and fBeginningCapPts.
                    // Append the ending cap onto the current contour.
                    this->enqueue(Verb::kLine, fEndingCapPts.data());
                    // Move to the beginning cap and append it right before (and joined to) the
                    // first stroke (that we will add below).
                    this->enqueue(Verb::kMoveWithinContour, fBeginningCapPts.data());
                    this->enqueue(Verb::kLine, fBeginningCapPts.data());
                    break;
            }
        } else if (fLastDegenerateStrokePt) {
            // fQueueCount=0 means this subpath is zero length. Generates caps on its location.
            //
            //   "Any zero length subpath ...  shall be stroked if the 'stroke-linecap' property has
            //   a value of round or square producing respectively a circle or a square."
            //
            //   (https://www.w3.org/TR/SVG11/painting.html#StrokeProperties)
            //
            switch (fCapType) {
                case SkPaint::kButt_Cap:
                    // Zero-length contour with butt caps. There are no caps and no first stroke to
                    // generate.
                    return false;
                case SkPaint::kRound_Cap:
                    this->enqueue(Verb::kCusp, fLastDegenerateStrokePt);
                    // Setting the "first" stroke as the cusp causes it to be added again below,
                    // this time as the "next" stroke.
                    fFirstVerb = Verb::kCusp;
                    fFirstPts = fLastDegenerateStrokePt;
                    break;
                case SkPaint::kSquare_Cap:
                    fEndingCapPts = {*fLastDegenerateStrokePt - SkPoint{fStrokeRadius, 0},
                                     *fLastDegenerateStrokePt + SkPoint{fStrokeRadius, 0}};
                    // Add the square first as the "prev" join.
                    this->enqueue(Verb::kLine, fEndingCapPts.data());
                    this->enqueue(Verb::kMoveWithinContour, fEndingCapPts.data());
                    // Setting the "first" stroke as the square causes it to be added again below,
                    // this time as the "next" stroke.
                    fFirstVerb = Verb::kLine;
                    fFirstPts = fEndingCapPts.data();
                    break;
            }
        } else {
            // This contour had no lines, beziers, or "close" verbs. There are no caps and no first
            // stroke to generate.
            return false;
        }

        // Repeat the first verb, this time as the "next" stroke instead of the "prev".
        this->enqueue(fFirstVerb, fFirstPts);
        this->enqueue(Verb::kContourFinished, nullptr);
        fLastDegenerateStrokePt = nullptr;
        return true;
    }

    static int endpoint_index(Verb verb) {
        switch (verb) {
            case Verb::kLine: return 1;
            case Verb::kQuad: return 2;
            case Verb::kCubic: return 3;
            default: SkUNREACHABLE;
        }
    }

    void fillSquareCapPoints() {
        // Find the endpoints of the cap at the end of the contour.
        SkVector lastTangent;
        const SkPoint* lastPts = this->backPts();
        Verb lastVerb = this->backVerb();
        switch (lastVerb) {
            case Verb::kCubic:
                lastTangent = lastPts[3] - lastPts[2];
                if (!lastTangent.isZero()) {
                    break;
                }
                [[fallthrough]];
            case Verb::kQuad:
                lastTangent = lastPts[2] - lastPts[1];
                if (!lastTangent.isZero()) {
                    break;
                }
                [[fallthrough]];
            case Verb::kLine:
                lastTangent = lastPts[1] - lastPts[0];
                SkASSERT(!lastTangent.isZero());
                break;
            default:
                SkUNREACHABLE;
        }
        lastTangent.normalize();
        SkPoint lastPoint = lastPts[SkPathPriv::PtsInIter((unsigned)lastVerb) - 1];
        fEndingCapPts = {lastPoint, lastPoint + lastTangent * fStrokeRadius};

        // Find the endpoints of the cap at the beginning of the contour.
        SkVector firstTangent = fFirstPts[1] - fFirstPts[0];
        if (firstTangent.isZero()) {
            SkASSERT(fFirstVerb == Verb::kQuad || fFirstVerb == Verb::kCubic);
            firstTangent = fFirstPts[2] - fFirstPts[0];
            if (firstTangent.isZero()) {
                SkASSERT(fFirstVerb == Verb::kCubic);
                firstTangent = fFirstPts[3] - fFirstPts[0];
                SkASSERT(!firstTangent.isZero());
            }
        }
        firstTangent.normalize();
        fBeginningCapPts = {fFirstPts[0] - firstTangent * fStrokeRadius, fFirstPts[0]};
    }

    const SkPaint::Cap fCapType;
    const float fStrokeRadius;
    SkPathPriv::RangeIter fIter;
    SkPathPriv::RangeIter fEnd;

    Verb fVerbs[kQueueBufferCount];
    const SkPoint* fPts[kQueueBufferCount];
    int fQueueIdx = 0;
    int fQueueCount = 0;
    Verb fFirstVerb;
    const SkPoint* fFirstPts;
    const SkPoint* fLastDegenerateStrokePt = nullptr;

    std::array<SkPoint, 2> fClosePts;
    std::array<SkPoint, 2> fEndingCapPts;
    std::array<SkPoint, 2> fBeginningCapPts;
};

#endif
