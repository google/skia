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
#include <array>

// This class iterates over the stroke geometry defined by a path and stroke. It automatically
// converts closes and square caps to lines, and round caps to circles so the user doesn't have to
// worry about it. At each location it provides a verb and "prevVerb" so there is context about the
// preceding join. Usage:
//
//     GrStrokeIterator iter(path, stroke);
//     while (iter.next()) {  // Call next() first.
//         iter.verb();
//         iter.pts();
//         iter.prevVerb();
//         iter.prevPts();
//     }
//
class GrStrokeIterator {
public:
    GrStrokeIterator(const SkPath& path, const SkStrokeRec& stroke)
            : fCapType(stroke.getCap()), fStrokeRadius(stroke.getWidth() * .5) {
        SkPathPriv::Iterate it(path);
        fIter = it.begin();
        fEnd = it.end();
    }

    enum class Verb {
        // Verbs that describe stroke geometry.
        kLine = (int)SkPathVerb::kLine,
        kQuad = (int)SkPathVerb::kQuad,
        kCubic = (int)SkPathVerb::kCubic,
        kCircle,  // A stroke-width circle drawn as a 180-degree point stroke.

        // Helper verbs that notify callers to update their own iteration state.
        kMoveWithinContour,
        kContourFinished
    };
    constexpr static bool IsVerbGeometric(Verb verb) { return verb < Verb::kMoveWithinContour; }

    // Must be called first. Loads the next pair of "prev" and "current" stroke. Returns false if
    // iteration is complete.
    bool next() {
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
                    if (!this->finishOpenContour()) {
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
                        // Defer the first verb until the end when we know what it's joined to.
                        fFirstVerbInContour = (Verb)verb;
                        fFirstPtsInContour = pts;
                        continue;
                    }
                    break;
                case SkPathVerb::kClose:
                    if (!fQueueCount) {
                        fLastDegenerateStrokePt = pts;
                        continue;
                    }
                    if (pts[0] != fFirstPtsInContour[0]) {
                        // Draw a line back to the contour's starting point.
                        fClosePts = {pts[0], fFirstPtsInContour[0]};
                        this->enqueue(Verb::kLine, fClosePts.data());
                    }
                    // Repeat the first verb, this time as the "current" stroke instead of the prev.
                    this->enqueue(fFirstVerbInContour, fFirstPtsInContour);
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
        return this->finishOpenContour();
    }

    Verb prevVerb() const { return this->atVerb(0); }
    const SkPoint* prevPts() const { return this->atPts(0); }

    Verb verb() const { return this->atVerb(1); }
    const SkPoint* pts() const { return this->atPts(1); }

    Verb firstVerbInContour() const { SkASSERT(fQueueCount > 0); return fFirstVerbInContour; }
    const SkPoint* firstPtsInContour() const {
        SkASSERT(fQueueCount > 0);
        return fFirstPtsInContour;
    }

private:
    constexpr static int kQueueBufferCount = 8;
    Verb atVerb(int i) const {
        SkASSERT(0 <= i && i < fQueueCount);
        return fVerbs[(fQueueFrontIdx + i) & (kQueueBufferCount - 1)];
    }
    Verb backVerb() const {
        return this->atVerb(fQueueCount - 1);
    }
    const SkPoint* atPts(int i) const {
        SkASSERT(0 <= i && i < fQueueCount);
        return fPts[(fQueueFrontIdx + i) & (kQueueBufferCount - 1)];
    }
    const SkPoint* backPts() const {
        return this->atPts(fQueueCount - 1);
    }
    void enqueue(Verb verb, const SkPoint* pts) {
        SkASSERT(fQueueCount < kQueueBufferCount);
        int i = (fQueueFrontIdx + fQueueCount) & (kQueueBufferCount - 1);
        fVerbs[i] = verb;
        fPts[i] = pts;
        ++fQueueCount;
    }
    void popFront() {
        SkASSERT(fQueueCount > 0);
        ++fQueueFrontIdx;
        --fQueueCount;
    }

    // Finishes the current contour without closing it. Enqueues any necessary caps as well as the
    // contour's first stroke that we deferred at the beginning.
    // Returns false and makes no changes if the current contour was already finished.
    bool finishOpenContour() {
        if (fQueueCount) {
            SkASSERT(this->backVerb() == Verb::kLine || this->backVerb() == Verb::kQuad ||
                     this->backVerb() == Verb::kCubic);
            switch (fCapType) {
                case SkPaint::kButt_Cap:
                    // There are no caps, but inject a "move" so the first stroke doesn't get joined
                    // with the end of the contour when it's processed.
                    this->enqueue(Verb::kMoveWithinContour, fFirstPtsInContour);
                    break;
                case SkPaint::kRound_Cap: {
                    // The "kCircle" verb serves as our barrier to prevent the first stroke from
                    // getting joined with the end of the contour. We just need to make sure that
                    // the first point of the contour goes last.
                    int backIdx = SkPathPriv::PtsInIter((unsigned)this->backVerb()) - 1;
                    this->enqueue(Verb::kCircle, this->backPts() + backIdx);
                    this->enqueue(Verb::kCircle, fFirstPtsInContour);
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
                    this->enqueue(Verb::kCircle, fLastDegenerateStrokePt);
                    // Setting the "first" stroke as the circle causes it to be added again below,
                    // this time as the "current" stroke.
                    fFirstVerbInContour = Verb::kCircle;
                    fFirstPtsInContour = fLastDegenerateStrokePt;
                    break;
                case SkPaint::kSquare_Cap:
                    fEndingCapPts = {*fLastDegenerateStrokePt - SkPoint{fStrokeRadius, 0},
                                     *fLastDegenerateStrokePt + SkPoint{fStrokeRadius, 0}};
                    // Add the square first as the "prev" join.
                    this->enqueue(Verb::kLine, fEndingCapPts.data());
                    this->enqueue(Verb::kMoveWithinContour, fEndingCapPts.data());
                    // Setting the "first" stroke as the square causes it to be added again below,
                    // this time as the "current" stroke.
                    fFirstVerbInContour = Verb::kLine;
                    fFirstPtsInContour = fEndingCapPts.data();
                    break;
            }
        } else {
            // This contour had no lines, beziers, or "close" verbs. There are no caps and no first
            // stroke to generate.
            return false;
        }

        // Repeat the first verb, this time as the "current" stroke instead of the prev.
        this->enqueue(fFirstVerbInContour, fFirstPtsInContour);
        this->enqueue(Verb::kContourFinished, nullptr);
        fLastDegenerateStrokePt = nullptr;
        return true;
    }

    // We implement square caps as two extra "kLine" verbs. This method finds the endpoints for
    // those lines.
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
        SkVector firstTangent = fFirstPtsInContour[1] - fFirstPtsInContour[0];
        if (firstTangent.isZero()) {
            SkASSERT(fFirstVerbInContour == Verb::kQuad || fFirstVerbInContour == Verb::kCubic);
            firstTangent = fFirstPtsInContour[2] - fFirstPtsInContour[0];
            if (firstTangent.isZero()) {
                SkASSERT(fFirstVerbInContour == Verb::kCubic);
                firstTangent = fFirstPtsInContour[3] - fFirstPtsInContour[0];
                SkASSERT(!firstTangent.isZero());
            }
        }
        firstTangent.normalize();
        fBeginningCapPts = {fFirstPtsInContour[0] - firstTangent * fStrokeRadius,
                            fFirstPtsInContour[0]};
    }

    // Info and iterators from the original path.
    const SkPaint::Cap fCapType;
    const float fStrokeRadius;
    SkPathPriv::RangeIter fIter;
    SkPathPriv::RangeIter fEnd;

    // Info for the current contour we are iterating.
    Verb fFirstVerbInContour;
    const SkPoint* fFirstPtsInContour;
    const SkPoint* fLastDegenerateStrokePt = nullptr;

    // The queue is implemented as a roll-over array with a floating front index.
    Verb fVerbs[kQueueBufferCount];
    const SkPoint* fPts[kQueueBufferCount];
    int fQueueFrontIdx = 0;
    int fQueueCount = 0;

    // Storage space for geometry that gets defined implicitly by the path, but does not have
    // actual points in memory to reference.
    std::array<SkPoint, 2> fClosePts;
    std::array<SkPoint, 2> fEndingCapPts;
    std::array<SkPoint, 2> fBeginningCapPts;
};

#endif
