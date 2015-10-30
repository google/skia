/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkOpCoincidence.h"
#include "SkOpContour.h"
#include "SkOpSegment.h"
#include "SkPathWriter.h"

/*
After computing raw intersections, post process all segments to:
- find small collections of points that can be collapsed to a single point
- find missing intersections to resolve differences caused by different algorithms

Consider segments containing tiny or small intervals. Consider coincident segments
because coincidence finds intersections through distance measurement that non-coincident
intersection tests cannot.
 */

#define F (false)      // discard the edge
#define T (true)       // keep the edge

static const bool gUnaryActiveEdge[2][2] = {
//  from=0  from=1
//  to=0,1  to=0,1
    {F, T}, {T, F},
};

static const bool gActiveEdge[kXOR_SkPathOp + 1][2][2][2][2] = {
//                 miFrom=0                              miFrom=1
//         miTo=0             miTo=1             miTo=0             miTo=1
//     suFrom=0    1      suFrom=0    1      suFrom=0    1      suFrom=0    1
//   suTo=0,1 suTo=0,1  suTo=0,1 suTo=0,1  suTo=0,1 suTo=0,1  suTo=0,1 suTo=0,1
    {{{{F, F}, {F, F}}, {{T, F}, {T, F}}}, {{{T, T}, {F, F}}, {{F, T}, {T, F}}}},  // mi - su
    {{{{F, F}, {F, F}}, {{F, T}, {F, T}}}, {{{F, F}, {T, T}}, {{F, T}, {T, F}}}},  // mi & su
    {{{{F, T}, {T, F}}, {{T, T}, {F, F}}}, {{{T, F}, {T, F}}, {{F, F}, {F, F}}}},  // mi | su
    {{{{F, T}, {T, F}}, {{T, F}, {F, T}}}, {{{T, F}, {F, T}}, {{F, T}, {T, F}}}},  // mi ^ su
};

#undef F
#undef T

SkOpAngle* SkOpSegment::activeAngle(SkOpSpanBase* start, SkOpSpanBase** startPtr,
        SkOpSpanBase** endPtr, bool* done) {
    if (SkOpAngle* result = activeAngleInner(start, startPtr, endPtr, done)) {
        return result;
    }
    if (SkOpAngle* result = activeAngleOther(start, startPtr, endPtr, done)) {
        return result;
    }
    return nullptr;
}

SkOpAngle* SkOpSegment::activeAngleInner(SkOpSpanBase* start, SkOpSpanBase** startPtr,
        SkOpSpanBase** endPtr, bool* done) {
    SkOpSpan* upSpan = start->upCastable();
    if (upSpan) {
        if (upSpan->windValue() || upSpan->oppValue()) {
            SkOpSpanBase* next = upSpan->next();
            if (!*endPtr) {
                *startPtr = start;
                *endPtr = next;
            }
            if (!upSpan->done()) {
                if (upSpan->windSum() != SK_MinS32) {
                    return spanToAngle(start, next);
                }
                *done = false;
            }
        } else {
            SkASSERT(upSpan->done());
        }
    }
    SkOpSpan* downSpan = start->prev();
    // edge leading into junction
    if (downSpan) {
        if (downSpan->windValue() || downSpan->oppValue()) {
            if (!*endPtr) {
                *startPtr = start;
                *endPtr = downSpan;
            }
            if (!downSpan->done()) {
                if (downSpan->windSum() != SK_MinS32) {
                    return spanToAngle(start, downSpan);
                }
                *done = false;
            }
        } else {
            SkASSERT(downSpan->done());
        }
    }
    return nullptr;
}

SkOpAngle* SkOpSegment::activeAngleOther(SkOpSpanBase* start, SkOpSpanBase** startPtr,
        SkOpSpanBase** endPtr, bool* done) {
    SkOpPtT* oPtT = start->ptT()->next();
    SkOpSegment* other = oPtT->segment();
    SkOpSpanBase* oSpan = oPtT->span();
    return other->activeAngleInner(oSpan, startPtr, endPtr, done);
}

bool SkOpSegment::activeOp(SkOpSpanBase* start, SkOpSpanBase* end, int xorMiMask, int xorSuMask,
        SkPathOp op) {
    int sumMiWinding = this->updateWinding(end, start);
    int sumSuWinding = this->updateOppWinding(end, start);
#if DEBUG_LIMIT_WIND_SUM
    SkASSERT(abs(sumMiWinding) <= DEBUG_LIMIT_WIND_SUM);
    SkASSERT(abs(sumSuWinding) <= DEBUG_LIMIT_WIND_SUM);
#endif
    if (this->operand()) {
        SkTSwap<int>(sumMiWinding, sumSuWinding);
    }
    return this->activeOp(xorMiMask, xorSuMask, start, end, op, &sumMiWinding, &sumSuWinding);
}

bool SkOpSegment::activeOp(int xorMiMask, int xorSuMask, SkOpSpanBase* start, SkOpSpanBase* end,
        SkPathOp op, int* sumMiWinding, int* sumSuWinding) {
    int maxWinding, sumWinding, oppMaxWinding, oppSumWinding;
    this->setUpWindings(start, end, sumMiWinding, sumSuWinding,
            &maxWinding, &sumWinding, &oppMaxWinding, &oppSumWinding);
    bool miFrom;
    bool miTo;
    bool suFrom;
    bool suTo;
    if (operand()) {
        miFrom = (oppMaxWinding & xorMiMask) != 0;
        miTo = (oppSumWinding & xorMiMask) != 0;
        suFrom = (maxWinding & xorSuMask) != 0;
        suTo = (sumWinding & xorSuMask) != 0;
    } else {
        miFrom = (maxWinding & xorMiMask) != 0;
        miTo = (sumWinding & xorMiMask) != 0;
        suFrom = (oppMaxWinding & xorSuMask) != 0;
        suTo = (oppSumWinding & xorSuMask) != 0;
    }
    bool result = gActiveEdge[op][miFrom][miTo][suFrom][suTo];
#if DEBUG_ACTIVE_OP
    SkDebugf("%s id=%d t=%1.9g tEnd=%1.9g op=%s miFrom=%d miTo=%d suFrom=%d suTo=%d result=%d\n",
            __FUNCTION__, debugID(), start->t(), end->t(),
            SkPathOpsDebug::kPathOpStr[op], miFrom, miTo, suFrom, suTo, result);
#endif
    return result;
}

bool SkOpSegment::activeWinding(SkOpSpanBase* start, SkOpSpanBase* end) {
    int sumWinding = updateWinding(end, start);
    return activeWinding(start, end, &sumWinding);
}

bool SkOpSegment::activeWinding(SkOpSpanBase* start, SkOpSpanBase* end, int* sumWinding) {
    int maxWinding;
    setUpWinding(start, end, &maxWinding, sumWinding);
    bool from = maxWinding != 0;
    bool to = *sumWinding  != 0;
    bool result = gUnaryActiveEdge[from][to];
    return result;
}

void SkOpSegment::addAlignIntersection(SkOpPtT& endPtT, SkPoint& oldPt,
        SkOpContourHead* contourList, SkChunkAlloc* allocator) {
    const SkPoint& newPt = endPtT.fPt;
    if (newPt == oldPt) {
        return;
    }
    SkPoint line[2] = { newPt, oldPt };
    SkPathOpsBounds lineBounds;
    lineBounds.setBounds(line, 2);
    SkDLine aLine;
    aLine.set(line);
    SkOpContour* current = contourList;
    do {
        if (!SkPathOpsBounds::Intersects(current->bounds(), lineBounds)) {
            continue;
        }
        SkOpSegment* segment = current->first();
        do {
            if (!SkPathOpsBounds::Intersects(segment->bounds(), lineBounds)) {
                continue;
            }
            if (newPt == segment->fPts[0]) {
                continue;
            }
            if (newPt == segment->fPts[SkPathOpsVerbToPoints(segment->fVerb)]) {
                continue;
            }
            if (oldPt == segment->fPts[0]) {
                continue;
            }
            if (oldPt == segment->fPts[SkPathOpsVerbToPoints(segment->fVerb)]) {
                continue;
            }
            if (endPtT.contains(segment)) {
                continue;
            }
            SkIntersections i;
            switch (segment->fVerb) {
                case SkPath::kLine_Verb: {
                    SkDLine bLine;
                    bLine.set(segment->fPts);
                    i.intersect(bLine, aLine);
                    } break;
                case SkPath::kQuad_Verb: {
                    SkDQuad bQuad;
                    bQuad.set(segment->fPts);
                    i.intersect(bQuad, aLine);
                    } break;
                case SkPath::kConic_Verb: {
                    SkDConic bConic;
                    bConic.set(segment->fPts, segment->fWeight);
                    i.intersect(bConic, aLine);
                    } break;
                case SkPath::kCubic_Verb: {
                    SkDCubic bCubic;
                    bCubic.set(segment->fPts);
                    i.intersect(bCubic, aLine);
                    } break;
                default:
                    SkASSERT(0);
            }
            if (i.used()) {
                SkASSERT(i.used() == 1);
                SkASSERT(!zero_or_one(i[0][0]));
                SkOpSpanBase* checkSpan = fHead.next();
                while (!checkSpan->final()) {
                    if (checkSpan->contains(segment)) {
                        goto nextSegment;
                    }
                    checkSpan = checkSpan->upCast()->next();
                }
                SkOpPtT* ptT = segment->addT(i[0][0], SkOpSegment::kAllowAlias, allocator);
                ptT->fPt = newPt;
                endPtT.addOpp(ptT);
            }
    nextSegment:
            ;
        } while ((segment = segment->next()));
    } while ((current = current->next()));
}

bool SkOpSegment::addCurveTo(const SkOpSpanBase* start, const SkOpSpanBase* end,
        SkPathWriter* path) const {
    if (start->starter(end)->alreadyAdded()) {
        return false;
    }
    SkOpCurve edge;
    const SkPoint* ePtr;
    SkScalar eWeight;
    if ((start == &fHead && end == &fTail) || (start == &fTail && end == &fHead)) {
        ePtr = fPts;
        eWeight = fWeight;
    } else {
    // OPTIMIZE? if not active, skip remainder and return xyAtT(end)
        subDivide(start, end, &edge);
        ePtr = edge.fPts;
        eWeight = edge.fWeight;
    }
    bool reverse = ePtr == fPts && start != &fHead;
    if (reverse) {
        path->deferredMoveLine(ePtr[SkPathOpsVerbToPoints(fVerb)]);
        switch (fVerb) {
            case SkPath::kLine_Verb:
                path->deferredLine(ePtr[0]);
                break;
            case SkPath::kQuad_Verb:
                path->quadTo(ePtr[1], ePtr[0]);
                break;
            case SkPath::kConic_Verb:
                path->conicTo(ePtr[1], ePtr[0], eWeight);
                break;
            case SkPath::kCubic_Verb:
                path->cubicTo(ePtr[2], ePtr[1], ePtr[0]);
                break;
            default:
                SkASSERT(0);
        }
    } else {
        path->deferredMoveLine(ePtr[0]);
        switch (fVerb) {
            case SkPath::kLine_Verb:
                path->deferredLine(ePtr[1]);
                break;
            case SkPath::kQuad_Verb:
                path->quadTo(ePtr[1], ePtr[2]);
                break;
            case SkPath::kConic_Verb:
                path->conicTo(ePtr[1], ePtr[2], eWeight);
                break;
            case SkPath::kCubic_Verb:
                path->cubicTo(ePtr[1], ePtr[2], ePtr[3]);
                break;
            default:
                SkASSERT(0);
        }
    }
    return true;
}

SkOpPtT* SkOpSegment::addMissing(double t, SkOpSegment* opp, SkChunkAlloc* allocator) {
    SkOpSpanBase* existing = nullptr;
    SkOpSpanBase* test = &fHead;
    double testT;
    do {
        if ((testT = test->ptT()->fT) >= t) {
            if (testT == t) {
                existing = test;
            }
            break;
        }
    } while ((test = test->upCast()->next()));
    SkOpPtT* result;
    if (existing && existing->contains(opp)) {
        result = existing->ptT();
    } else {
        result = this->addT(t, SkOpSegment::kNoAlias, allocator);
    }
    SkASSERT(result);
    return result;
}

SkOpPtT* SkOpSegment::addT(double t, AllowAlias allowAlias, SkChunkAlloc* allocator) {
    debugValidate();
    SkPoint pt = this->ptAtT(t);
    SkOpSpanBase* span = &fHead;
    do {
        SkOpPtT* result = span->ptT();
        SkOpPtT* loop;
        bool duplicatePt;
        if (t == result->fT) {
            goto bumpSpan;
        }
        if (this->match(result, this, t, pt)) {
            // see if any existing alias matches segment, pt, and t
           loop = result->next();
            duplicatePt = false;
            while (loop != result) {
                bool ptMatch = loop->fPt == pt;
                if (loop->segment() == this && loop->fT == t && ptMatch) {
                    goto bumpSpan;
                }
                duplicatePt |= ptMatch;
                loop = loop->next();
            }
            if (kNoAlias == allowAlias) {
    bumpSpan:
                span->bumpSpanAdds();
                return result;
            }
            SkOpPtT* alias = SkOpTAllocator<SkOpPtT>::Allocate(allocator);
            alias->init(result->span(), t, pt, duplicatePt);
            result->insert(alias);
            result->span()->unaligned();
            this->debugValidate();
#if DEBUG_ADD_T
            SkDebugf("%s alias t=%1.9g segID=%d spanID=%d\n",  __FUNCTION__, t,
                    alias->segment()->debugID(), alias->span()->debugID());
#endif
            span->bumpSpanAdds();
            return alias;
        }
        if (t < result->fT) {
            SkOpSpan* prev = result->span()->prev();
            SkOpSpan* span = insert(prev, allocator);
            span->init(this, prev, t, pt);
            this->debugValidate();
#if DEBUG_ADD_T
            SkDebugf("%s insert t=%1.9g segID=%d spanID=%d\n", __FUNCTION__, t,
                    span->segment()->debugID(), span->debugID());
#endif
            span->bumpSpanAdds();
            return span->ptT();
        }
        SkASSERT(span != &fTail);
    } while ((span = span->upCast()->next()));
    SkASSERT(0);
    return nullptr;
}

// choose a solitary t and pt value; remove aliases; align the opposite ends
void SkOpSegment::align() {
    debugValidate();
    SkOpSpanBase* span = &fHead;
    if (!span->aligned()) {
        span->alignEnd(0, fPts[0]);
    }
    while ((span = span->upCast()->next())) {
        if (span == &fTail) {
            break;
        }
        span->align();
    }
    if (!span->aligned()) {
        span->alignEnd(1, fPts[SkPathOpsVerbToPoints(fVerb)]);
    }
    if (this->collapsed()) {
        SkOpSpan* span = &fHead;
        do {
            span->setWindValue(0);
            span->setOppValue(0);
            this->markDone(span);
        } while ((span = span->next()->upCastable()));
    }
    debugValidate();
}

void SkOpSegment::calcAngles(SkChunkAlloc* allocator) {
    bool activePrior = !fHead.isCanceled();
    if (activePrior && !fHead.simple()) {
        addStartSpan(allocator);
    }
    SkOpSpan* prior = &fHead;
    SkOpSpanBase* spanBase = fHead.next();
    while (spanBase != &fTail) {
        if (activePrior) {
            SkOpAngle* priorAngle = SkOpTAllocator<SkOpAngle>::Allocate(allocator);
            priorAngle->set(spanBase, prior);
            spanBase->setFromAngle(priorAngle);
        }
        SkOpSpan* span = spanBase->upCast();
        bool active = !span->isCanceled();
        SkOpSpanBase* next = span->next();
        if (active) {
            SkOpAngle* angle = SkOpTAllocator<SkOpAngle>::Allocate(allocator);
            angle->set(span, next);
            span->setToAngle(angle);
        }
        activePrior = active;
        prior = span;
        spanBase = next;
    }
    if (activePrior && !fTail.simple()) {
        addEndSpan(allocator);
    }
}

bool SkOpSegment::collapsed() const {
    return fVerb == SkPath::kLine_Verb && fHead.pt() == fTail.pt();
}

void SkOpSegment::ComputeOneSum(const SkOpAngle* baseAngle, SkOpAngle* nextAngle,
        SkOpAngle::IncludeType includeType) {
    SkOpSegment* baseSegment = baseAngle->segment();
    int sumMiWinding = baseSegment->updateWindingReverse(baseAngle);
    int sumSuWinding;
    bool binary = includeType >= SkOpAngle::kBinarySingle;
    if (binary) {
        sumSuWinding = baseSegment->updateOppWindingReverse(baseAngle);
        if (baseSegment->operand()) {
            SkTSwap<int>(sumMiWinding, sumSuWinding);
        }
    }
    SkOpSegment* nextSegment = nextAngle->segment();
    int maxWinding, sumWinding;
    SkOpSpanBase* last;
    if (binary) {
        int oppMaxWinding, oppSumWinding;
        nextSegment->setUpWindings(nextAngle->start(), nextAngle->end(), &sumMiWinding,
                &sumSuWinding, &maxWinding, &sumWinding, &oppMaxWinding, &oppSumWinding);
        last = nextSegment->markAngle(maxWinding, sumWinding, oppMaxWinding, oppSumWinding,
                nextAngle);
    } else {
        nextSegment->setUpWindings(nextAngle->start(), nextAngle->end(), &sumMiWinding,
                &maxWinding, &sumWinding);
        last = nextSegment->markAngle(maxWinding, sumWinding, nextAngle);
    }
    nextAngle->setLastMarked(last);
}

void SkOpSegment::ComputeOneSumReverse(SkOpAngle* baseAngle, SkOpAngle* nextAngle,
        SkOpAngle::IncludeType includeType) {
    SkOpSegment* baseSegment = baseAngle->segment();
    int sumMiWinding = baseSegment->updateWinding(baseAngle);
    int sumSuWinding;
    bool binary = includeType >= SkOpAngle::kBinarySingle;
    if (binary) {
        sumSuWinding = baseSegment->updateOppWinding(baseAngle);
        if (baseSegment->operand()) {
            SkTSwap<int>(sumMiWinding, sumSuWinding);
        }
    }
    SkOpSegment* nextSegment = nextAngle->segment();
    int maxWinding, sumWinding;
    SkOpSpanBase* last;
    if (binary) {
        int oppMaxWinding, oppSumWinding;
        nextSegment->setUpWindings(nextAngle->end(), nextAngle->start(), &sumMiWinding,
                &sumSuWinding, &maxWinding, &sumWinding, &oppMaxWinding, &oppSumWinding);
        last = nextSegment->markAngle(maxWinding, sumWinding, oppMaxWinding, oppSumWinding,
                nextAngle);
    } else {
        nextSegment->setUpWindings(nextAngle->end(), nextAngle->start(), &sumMiWinding,
                &maxWinding, &sumWinding);
        last = nextSegment->markAngle(maxWinding, sumWinding, nextAngle);
    }
    nextAngle->setLastMarked(last);
}

// at this point, the span is already ordered, or unorderable
int SkOpSegment::computeSum(SkOpSpanBase* start, SkOpSpanBase* end,
        SkOpAngle::IncludeType includeType) {
    SkASSERT(includeType != SkOpAngle::kUnaryXor);
    SkOpAngle* firstAngle = this->spanToAngle(end, start);
    if (nullptr == firstAngle || nullptr == firstAngle->next()) {
        return SK_NaN32;
    }
    // if all angles have a computed winding,
    //  or if no adjacent angles are orderable,
    //  or if adjacent orderable angles have no computed winding,
    //  there's nothing to do
    // if two orderable angles are adjacent, and both are next to orderable angles,
    //  and one has winding computed, transfer to the other
    SkOpAngle* baseAngle = nullptr;
    bool tryReverse = false;
    // look for counterclockwise transfers
    SkOpAngle* angle = firstAngle->previous();
    SkOpAngle* next = angle->next();
    firstAngle = next;
    do {
        SkOpAngle* prior = angle;
        angle = next;
        next = angle->next();
        SkASSERT(prior->next() == angle);
        SkASSERT(angle->next() == next);
        if (prior->unorderable() || angle->unorderable() || next->unorderable()) {
            baseAngle = nullptr;
            continue;
        }
        int testWinding = angle->starter()->windSum();
        if (SK_MinS32 != testWinding) {
            baseAngle = angle;
            tryReverse = true;
            continue;
        }
        if (baseAngle) {
            ComputeOneSum(baseAngle, angle, includeType);
            baseAngle = SK_MinS32 != angle->starter()->windSum() ? angle : nullptr;
        }
    } while (next != firstAngle);
    if (baseAngle && SK_MinS32 == firstAngle->starter()->windSum()) {
        firstAngle = baseAngle;
        tryReverse = true;
    }
    if (tryReverse) {
        baseAngle = nullptr;
        SkOpAngle* prior = firstAngle;
        do {
            angle = prior;
            prior = angle->previous();
            SkASSERT(prior->next() == angle);
            next = angle->next();
            if (prior->unorderable() || angle->unorderable() || next->unorderable()) {
                baseAngle = nullptr;
                continue;
            }
            int testWinding = angle->starter()->windSum();
            if (SK_MinS32 != testWinding) {
                baseAngle = angle;
                continue;
            }
            if (baseAngle) {
                ComputeOneSumReverse(baseAngle, angle, includeType);
                baseAngle = SK_MinS32 != angle->starter()->windSum() ? angle : nullptr;
            }
        } while (prior != firstAngle);
    }
    return start->starter(end)->windSum();
}

void SkOpSegment::detach(const SkOpSpan* span) {
    if (span->done()) {
        --fDoneCount;
    }
    --fCount;
    SkASSERT(fCount >= fDoneCount);
}

double SkOpSegment::distSq(double t, const SkOpAngle* oppAngle) const {
    SkDPoint testPt = this->dPtAtT(t);
    SkDLine testPerp = {{ testPt, testPt }};
    SkDVector slope = this->dSlopeAtT(t);
    testPerp[1].fX += slope.fY;
    testPerp[1].fY -= slope.fX;
    SkIntersections i;
    const SkOpSegment* oppSegment = oppAngle->segment();
    (*CurveIntersectRay[oppSegment->verb()])(oppSegment->pts(), oppSegment->weight(), testPerp, &i);
    double closestDistSq = SK_ScalarInfinity;
    for (int index = 0; index < i.used(); ++index) {
        if (!between(oppAngle->start()->t(), i[0][index], oppAngle->end()->t())) {
            continue;
        }
        double testDistSq = testPt.distanceSquared(i.pt(index));
        if (closestDistSq > testDistSq) {
            closestDistSq = testDistSq;
        }
    }
    return closestDistSq;
}

void SkOpSegment::findCollapsed() {
    if (fHead.contains(&fTail)) {
        markAllDone();
        // move start and end to the same point
        fHead.alignEnd(0, fHead.pt());
        fTail.setAligned();
    }
}

/*
 The M and S variable name parts stand for the operators.
   Mi stands for Minuend (see wiki subtraction, analogous to difference)
   Su stands for Subtrahend
 The Opp variable name part designates that the value is for the Opposite operator.
 Opposite values result from combining coincident spans.
 */
SkOpSegment* SkOpSegment::findNextOp(SkTDArray<SkOpSpanBase*>* chase, SkOpSpanBase** nextStart,
        SkOpSpanBase** nextEnd, bool* unsortable, SkPathOp op, int xorMiMask, int xorSuMask) {
    SkOpSpanBase* start = *nextStart;
    SkOpSpanBase* end = *nextEnd;
    SkASSERT(start != end);
    int step = start->step(end);
    SkOpSegment* other = this->isSimple(nextStart, &step);  // advances nextStart
    if (other) {
    // mark the smaller of startIndex, endIndex done, and all adjacent
    // spans with the same T value (but not 'other' spans)
#if DEBUG_WINDING
        SkDebugf("%s simple\n", __FUNCTION__);
#endif
        SkOpSpan* startSpan = start->starter(end);
        if (startSpan->done()) {
            return nullptr;
        }
        markDone(startSpan);
        *nextEnd = step > 0 ? (*nextStart)->upCast()->next() : (*nextStart)->prev();
        return other;
    }
    SkOpSpanBase* endNear = step > 0 ? (*nextStart)->upCast()->next() : (*nextStart)->prev();
    SkASSERT(endNear == end);  // is this ever not end?
    SkASSERT(endNear);
    SkASSERT(start != endNear);
    SkASSERT((start->t() < endNear->t()) ^ (step < 0));
    // more than one viable candidate -- measure angles to find best
    int calcWinding = computeSum(start, endNear, SkOpAngle::kBinaryOpp);
    bool sortable = calcWinding != SK_NaN32;
    if (!sortable) {
        *unsortable = true;
        markDone(start->starter(end));
        return nullptr;
    }
    SkOpAngle* angle = this->spanToAngle(end, start);
    if (angle->unorderable()) {
        *unsortable = true;
        markDone(start->starter(end));
        return nullptr;
    }
#if DEBUG_SORT
    SkDebugf("%s\n", __FUNCTION__);
    angle->debugLoop();
#endif
    int sumMiWinding = updateWinding(end, start);
    if (sumMiWinding == SK_MinS32) {
        *unsortable = true;
        markDone(start->starter(end));
        return nullptr;
    }
    int sumSuWinding = updateOppWinding(end, start);
    if (operand()) {
        SkTSwap<int>(sumMiWinding, sumSuWinding);
    }
    SkOpAngle* nextAngle = angle->next();
    const SkOpAngle* foundAngle = nullptr;
    bool foundDone = false;
    // iterate through the angle, and compute everyone's winding
    SkOpSegment* nextSegment;
    int activeCount = 0;
    do {
        nextSegment = nextAngle->segment();
        bool activeAngle = nextSegment->activeOp(xorMiMask, xorSuMask, nextAngle->start(),
                nextAngle->end(), op, &sumMiWinding, &sumSuWinding);
        if (activeAngle) {
            ++activeCount;
            if (!foundAngle || (foundDone && activeCount & 1)) {
                foundAngle = nextAngle;
                foundDone = nextSegment->done(nextAngle);
            }
        }
        if (nextSegment->done()) {
            continue;
        }
        if (!activeAngle) {
            (void) nextSegment->markAndChaseDone(nextAngle->start(), nextAngle->end());
        }
        SkOpSpanBase* last = nextAngle->lastMarked();
        if (last) {
            SkASSERT(!SkPathOpsDebug::ChaseContains(*chase, last));
            *chase->append() = last;
#if DEBUG_WINDING
            SkDebugf("%s chase.append segment=%d span=%d", __FUNCTION__,
                    last->segment()->debugID(), last->debugID());
            if (!last->final()) {
                SkDebugf(" windSum=%d", last->upCast()->windSum());
            }
            SkDebugf("\n");
#endif
        }
    } while ((nextAngle = nextAngle->next()) != angle);
    start->segment()->markDone(start->starter(end));
    if (!foundAngle) {
        return nullptr;
    }
    *nextStart = foundAngle->start();
    *nextEnd = foundAngle->end();
    nextSegment = foundAngle->segment();
#if DEBUG_WINDING
    SkDebugf("%s from:[%d] to:[%d] start=%d end=%d\n",
            __FUNCTION__, debugID(), nextSegment->debugID(), *nextStart, *nextEnd);
 #endif
    return nextSegment;
}

SkOpSegment* SkOpSegment::findNextWinding(SkTDArray<SkOpSpanBase*>* chase,
        SkOpSpanBase** nextStart, SkOpSpanBase** nextEnd, bool* unsortable) {
    SkOpSpanBase* start = *nextStart;
    SkOpSpanBase* end = *nextEnd;
    SkASSERT(start != end);
    int step = start->step(end);
    SkOpSegment* other = this->isSimple(nextStart, &step);  // advances nextStart
    if (other) {
    // mark the smaller of startIndex, endIndex done, and all adjacent
    // spans with the same T value (but not 'other' spans)
#if DEBUG_WINDING
        SkDebugf("%s simple\n", __FUNCTION__);
#endif
        SkOpSpan* startSpan = start->starter(end);
        if (startSpan->done()) {
            return nullptr;
        }
        markDone(startSpan);
        *nextEnd = step > 0 ? (*nextStart)->upCast()->next() : (*nextStart)->prev();
        return other;
    }
    SkOpSpanBase* endNear = step > 0 ? (*nextStart)->upCast()->next() : (*nextStart)->prev();
    SkASSERT(endNear == end);  // is this ever not end?
    SkASSERT(endNear);
    SkASSERT(start != endNear);
    SkASSERT((start->t() < endNear->t()) ^ (step < 0));
    // more than one viable candidate -- measure angles to find best
    int calcWinding = computeSum(start, endNear, SkOpAngle::kUnaryWinding);
    bool sortable = calcWinding != SK_NaN32;
    if (!sortable) {
        *unsortable = true;
        markDone(start->starter(end));
        return nullptr;
    }
    SkOpAngle* angle = this->spanToAngle(end, start);
    if (angle->unorderable()) {
        *unsortable = true;
        markDone(start->starter(end));
        return nullptr;
    }
#if DEBUG_SORT
    SkDebugf("%s\n", __FUNCTION__);
    angle->debugLoop();
#endif
    int sumWinding = updateWinding(end, start);
    SkOpAngle* nextAngle = angle->next();
    const SkOpAngle* foundAngle = nullptr;
    bool foundDone = false;
    // iterate through the angle, and compute everyone's winding
    SkOpSegment* nextSegment;
    int activeCount = 0;
    do {
        nextSegment = nextAngle->segment();
        bool activeAngle = nextSegment->activeWinding(nextAngle->start(), nextAngle->end(),
                &sumWinding);
        if (activeAngle) {
            ++activeCount;
            if (!foundAngle || (foundDone && activeCount & 1)) {
                foundAngle = nextAngle;
                foundDone = nextSegment->done(nextAngle);
            }
        }
        if (nextSegment->done()) {
            continue;
        }
        if (!activeAngle) {
            (void) nextSegment->markAndChaseDone(nextAngle->start(), nextAngle->end());
        }
        SkOpSpanBase* last = nextAngle->lastMarked();
        if (last) {
            SkASSERT(!SkPathOpsDebug::ChaseContains(*chase, last));
            *chase->append() = last;
#if DEBUG_WINDING
            SkDebugf("%s chase.append segment=%d span=%d", __FUNCTION__,
                    last->segment()->debugID(), last->debugID());
            if (!last->final()) {
                SkDebugf(" windSum=%d", last->upCast()->windSum());
            }
            SkDebugf("\n");
#endif
        }
    } while ((nextAngle = nextAngle->next()) != angle);
    start->segment()->markDone(start->starter(end));
    if (!foundAngle) {
        return nullptr;
    }
    *nextStart = foundAngle->start();
    *nextEnd = foundAngle->end();
    nextSegment = foundAngle->segment();
#if DEBUG_WINDING
    SkDebugf("%s from:[%d] to:[%d] start=%d end=%d\n",
            __FUNCTION__, debugID(), nextSegment->debugID(), *nextStart, *nextEnd);
 #endif
    return nextSegment;
}

SkOpSegment* SkOpSegment::findNextXor(SkOpSpanBase** nextStart, SkOpSpanBase** nextEnd,
        bool* unsortable) {
    SkOpSpanBase* start = *nextStart;
    SkOpSpanBase* end = *nextEnd;
    SkASSERT(start != end);
    int step = start->step(end);
    SkOpSegment* other = this->isSimple(nextStart, &step);  // advances nextStart
    if (other) {
    // mark the smaller of startIndex, endIndex done, and all adjacent
    // spans with the same T value (but not 'other' spans)
#if DEBUG_WINDING
        SkDebugf("%s simple\n", __FUNCTION__);
#endif
        SkOpSpan* startSpan = start->starter(end);
        if (startSpan->done()) {
            return nullptr;
        }
        markDone(startSpan);
        *nextEnd = step > 0 ? (*nextStart)->upCast()->next() : (*nextStart)->prev();
        return other;
    }
    SkDEBUGCODE(SkOpSpanBase* endNear = step > 0 ? (*nextStart)->upCast()->next() \
            : (*nextStart)->prev());
    SkASSERT(endNear == end);  // is this ever not end?
    SkASSERT(endNear);
    SkASSERT(start != endNear);
    SkASSERT((start->t() < endNear->t()) ^ (step < 0));
    SkOpAngle* angle = this->spanToAngle(end, start);
    if (!angle || angle->unorderable()) {
        *unsortable = true;
        markDone(start->starter(end));
        return nullptr;
    }
#if DEBUG_SORT
    SkDebugf("%s\n", __FUNCTION__);
    angle->debugLoop();
#endif
    SkOpAngle* nextAngle = angle->next();
    const SkOpAngle* foundAngle = nullptr;
    bool foundDone = false;
    // iterate through the angle, and compute everyone's winding
    SkOpSegment* nextSegment;
    int activeCount = 0;
    do {
        nextSegment = nextAngle->segment();
        ++activeCount;
        if (!foundAngle || (foundDone && activeCount & 1)) {
            foundAngle = nextAngle;
            if (!(foundDone = nextSegment->done(nextAngle))) {
                break;
            }
        }
        nextAngle = nextAngle->next();
    } while (nextAngle != angle);
    start->segment()->markDone(start->starter(end));
    if (!foundAngle) {
        return nullptr;
    }
    *nextStart = foundAngle->start();
    *nextEnd = foundAngle->end();
    nextSegment = foundAngle->segment();
#if DEBUG_WINDING
    SkDebugf("%s from:[%d] to:[%d] start=%d end=%d\n",
            __FUNCTION__, debugID(), nextSegment->debugID(), *nextStart, *nextEnd);
 #endif
    return nextSegment;
}

SkOpGlobalState* SkOpSegment::globalState() const {
    return contour()->globalState(); 
}

void SkOpSegment::init(SkPoint pts[], SkScalar weight, SkOpContour* contour, SkPath::Verb verb) {
    fContour = contour;
    fNext = nullptr;
    fOriginal[0] = pts[0];
    fOriginal[1] = pts[SkPathOpsVerbToPoints(verb)];
    fPts = pts;
    fWeight = weight;
    fVerb = verb;
    fCount = 0;
    fDoneCount = 0;
    fVisited = false;
    SkOpSpan* zeroSpan = &fHead;
    zeroSpan->init(this, nullptr, 0, fPts[0]);
    SkOpSpanBase* oneSpan = &fTail;
    zeroSpan->setNext(oneSpan);
    oneSpan->initBase(this, zeroSpan, 1, fPts[SkPathOpsVerbToPoints(fVerb)]);
    SkDEBUGCODE(fID = globalState()->nextSegmentID());
}

bool SkOpSegment::isClose(double t, const SkOpSegment* opp) const {
    SkDPoint cPt = this->dPtAtT(t);
    SkDVector dxdy = (*CurveDSlopeAtT[this->verb()])(this->pts(), this->weight(), t);
    SkDLine perp = {{ cPt, {cPt.fX + dxdy.fY, cPt.fY - dxdy.fX} }};
    SkIntersections i;
    (*CurveIntersectRay[opp->verb()])(opp->pts(), opp->weight(), perp, &i);
    int used = i.used();
    for (int index = 0; index < used; ++index) {
        if (cPt.roughlyEqual(i.pt(index))) {
            return true;
        }
    }
    return false;
}

bool SkOpSegment::isXor() const {
    return fContour->isXor();
}

void SkOpSegment::markAllDone() {
    SkOpSpan* span = this->head();
    do {
        this->markDone(span);
    } while ((span = span->next()->upCastable()));
}

SkOpSpanBase* SkOpSegment::markAndChaseDone(SkOpSpanBase* start, SkOpSpanBase* end) {
    int step = start->step(end);
    SkOpSpan* minSpan = start->starter(end);
    markDone(minSpan);
    SkOpSpanBase* last = nullptr;
    SkOpSegment* other = this;
    while ((other = other->nextChase(&start, &step, &minSpan, &last))) {
        if (other->done()) {
            SkASSERT(!last);
            break;
        }
        other->markDone(minSpan);
    }
    return last;
}

bool SkOpSegment::markAndChaseWinding(SkOpSpanBase* start, SkOpSpanBase* end, int winding,
        SkOpSpanBase** lastPtr) {
    SkOpSpan* spanStart = start->starter(end);
    int step = start->step(end);
    bool success = markWinding(spanStart, winding);
    SkOpSpanBase* last = nullptr;
    SkOpSegment* other = this;
    while ((other = other->nextChase(&start, &step, &spanStart, &last))) {
        if (spanStart->windSum() != SK_MinS32) {
            SkASSERT(spanStart->windSum() == winding);
            SkASSERT(!last);
            break;
        }
        (void) other->markWinding(spanStart, winding);
    }
    if (lastPtr) {
        *lastPtr = last;
    }
    return success;
}

bool SkOpSegment::markAndChaseWinding(SkOpSpanBase* start, SkOpSpanBase* end,
        int winding, int oppWinding, SkOpSpanBase** lastPtr) {
    SkOpSpan* spanStart = start->starter(end);
    int step = start->step(end);
    bool success = markWinding(spanStart, winding, oppWinding);
    SkOpSpanBase* last = nullptr;
    SkOpSegment* other = this;
    while ((other = other->nextChase(&start, &step, &spanStart, &last))) {
        if (spanStart->windSum() != SK_MinS32) {
            if (this->operand() == other->operand()) {
                if (spanStart->windSum() != winding || spanStart->oppSum() != oppWinding) {
                    this->globalState()->setWindingFailed();
                    return false;
                }
            } else {
                SkASSERT(spanStart->windSum() == oppWinding);
                SkASSERT(spanStart->oppSum() == winding);
            }
            SkASSERT(!last);
            break;
        }
        if (this->operand() == other->operand()) {
            (void) other->markWinding(spanStart, winding, oppWinding);
        } else {
            (void) other->markWinding(spanStart, oppWinding, winding);
        }
    }
    if (lastPtr) {
        *lastPtr = last;
    }
    return success;
}

SkOpSpanBase* SkOpSegment::markAngle(int maxWinding, int sumWinding, const SkOpAngle* angle) {
    SkASSERT(angle->segment() == this);
    if (UseInnerWinding(maxWinding, sumWinding)) {
        maxWinding = sumWinding;
    }
    SkOpSpanBase* last;
    (void) markAndChaseWinding(angle->start(), angle->end(), maxWinding, &last);
#if DEBUG_WINDING
    if (last) {
        SkDebugf("%s last seg=%d span=%d", __FUNCTION__,
                last->segment()->debugID(), last->debugID());
        if (!last->final()) {
            SkDebugf(" windSum=");
            SkPathOpsDebug::WindingPrintf(last->upCast()->windSum());
        }
        SkDebugf("\n");
    }
#endif
    return last;
}

SkOpSpanBase* SkOpSegment::markAngle(int maxWinding, int sumWinding, int oppMaxWinding,
                                   int oppSumWinding, const SkOpAngle* angle) {
    SkASSERT(angle->segment() == this);
    if (UseInnerWinding(maxWinding, sumWinding)) {
        maxWinding = sumWinding;
    }
    if (oppMaxWinding != oppSumWinding && UseInnerWinding(oppMaxWinding, oppSumWinding)) {
        oppMaxWinding = oppSumWinding;
    }
    SkOpSpanBase* last = nullptr;
    // caller doesn't require that this marks anything
    (void) markAndChaseWinding(angle->start(), angle->end(), maxWinding, oppMaxWinding, &last);
#if DEBUG_WINDING
    if (last) {
        SkDebugf("%s last segment=%d span=%d", __FUNCTION__,
                last->segment()->debugID(), last->debugID());
        if (!last->final()) {
            SkDebugf(" windSum=");
            SkPathOpsDebug::WindingPrintf(last->upCast()->windSum());
        }
        SkDebugf(" \n");
    }
#endif
    return last;
}

void SkOpSegment::markDone(SkOpSpan* span) {
    SkASSERT(this == span->segment());
    if (span->done()) {
        return;
    }
#if DEBUG_MARK_DONE
    debugShowNewWinding(__FUNCTION__, span, span->windSum(), span->oppSum());
#endif
    span->setDone(true);
    ++fDoneCount;
    debugValidate();
}

bool SkOpSegment::markWinding(SkOpSpan* span, int winding) {
    SkASSERT(this == span->segment());
    SkASSERT(winding);
    if (span->done()) {
        return false;
    }
#if DEBUG_MARK_DONE
    debugShowNewWinding(__FUNCTION__, span, winding);
#endif
    span->setWindSum(winding);
    debugValidate();
    return true;
}

bool SkOpSegment::markWinding(SkOpSpan* span, int winding, int oppWinding) {
    SkASSERT(this == span->segment());
    SkASSERT(winding || oppWinding);
    if (span->done()) {
        return false;
    }
#if DEBUG_MARK_DONE
    debugShowNewWinding(__FUNCTION__, span, winding, oppWinding);
#endif
    span->setWindSum(winding);
    span->setOppSum(oppWinding);
    debugValidate();
    return true;
}

bool SkOpSegment::match(const SkOpPtT* base, const SkOpSegment* testParent, double testT,
        const SkPoint& testPt) const {
    const SkOpSegment* baseParent = base->segment();
    if (this == baseParent && this == testParent && precisely_equal(base->fT, testT)) {
        return true;
    }
    if (!SkDPoint::ApproximatelyEqual(testPt, base->fPt)) {
        return false;
    }
    return !this->ptsDisjoint(base->fT, base->fPt, testT, testPt);
}

static SkOpSegment* set_last(SkOpSpanBase** last, SkOpSpanBase* endSpan) {
    if (last) {
        *last = endSpan;
    }
    return nullptr;
}

SkOpSegment* SkOpSegment::nextChase(SkOpSpanBase** startPtr, int* stepPtr, SkOpSpan** minPtr,
        SkOpSpanBase** last) const {
    SkOpSpanBase* origStart = *startPtr;
    int step = *stepPtr;
    SkOpSpanBase* endSpan = step > 0 ? origStart->upCast()->next() : origStart->prev();
    SkASSERT(endSpan);
    SkOpAngle* angle = step > 0 ? endSpan->fromAngle() : endSpan->upCast()->toAngle();
    SkOpSpanBase* foundSpan;
    SkOpSpanBase* otherEnd;
    SkOpSegment* other;
    if (angle == nullptr) {
        if (endSpan->t() != 0 && endSpan->t() != 1) {
            return nullptr;
        }
        SkOpPtT* otherPtT = endSpan->ptT()->next();
        other = otherPtT->segment();
        foundSpan = otherPtT->span();
        otherEnd = step > 0 ? foundSpan->upCast()->next() : foundSpan->prev();
    } else {
        int loopCount = angle->loopCount();
        if (loopCount > 2) {
            return set_last(last, endSpan);
        }
        const SkOpAngle* next = angle->next();
        if (nullptr == next) {
            return nullptr;
        }
#if DEBUG_WINDING
        if (angle->debugSign() != next->debugSign() && !angle->segment()->contour()->isXor()
                && !next->segment()->contour()->isXor()) {
            SkDebugf("%s mismatched signs\n", __FUNCTION__);
        }
#endif
        other = next->segment();
        foundSpan = endSpan = next->start();
        otherEnd = next->end();
    }
    int foundStep = foundSpan->step(otherEnd);
    if (*stepPtr != foundStep) {
        return set_last(last, endSpan);
    }
    SkASSERT(*startPtr);
    if (!otherEnd) {
        return nullptr;
    }
//    SkASSERT(otherEnd >= 0);
    SkOpSpan* origMin = step < 0 ? origStart->prev() : origStart->upCast();
    SkOpSpan* foundMin = foundSpan->starter(otherEnd);
    if (foundMin->windValue() != origMin->windValue()
            || foundMin->oppValue() != origMin->oppValue()) {
          return set_last(last, endSpan);
    }
    *startPtr = foundSpan;
    *stepPtr = foundStep;
    if (minPtr) {
        *minPtr = foundMin;
    }
    return other;
}

static void clear_visited(SkOpSpanBase* span) {
    // reset visited flag back to false
    do {
        SkOpPtT* ptT = span->ptT(), * stopPtT = ptT;
        while ((ptT = ptT->next()) != stopPtT) {
            SkOpSegment* opp = ptT->segment();
            opp->resetVisited();
        }
    } while (!span->final() && (span = span->upCast()->next()));
}

// look for pairs of undetected coincident curves
// assumes that segments going in have visited flag clear
// curve/curve intersection should now do a pretty good job of finding coincident runs so 
// this may be only be necessary for line/curve pairs -- so skip unless this is a line and the
// the opp is not a line
bool SkOpSegment::missingCoincidence(SkOpCoincidence* coincidences, SkChunkAlloc* allocator) {
    if (this->verb() != SkPath::kLine_Verb) {
        return false;
    }
    if (this->done()) {
        return false;
    }
    SkOpSpan* prior = nullptr;
    SkOpSpanBase* spanBase = &fHead;
    do {
        SkOpPtT* ptT = spanBase->ptT(), * spanStopPtT = ptT;
        SkASSERT(ptT->span() == spanBase);
        while ((ptT = ptT->next()) != spanStopPtT) {
            if (ptT->deleted()) {
                continue;
            }
            SkOpSegment* opp = ptT->span()->segment();
//            if (opp->verb() == SkPath::kLine_Verb) {
//                continue;
//            }
            if (opp->done()) {
                continue;
            }
            // when opp is encounted the 1st time, continue; on 2nd encounter, look for coincidence
            if (!opp->visited()) {
                continue;
            }
            if (spanBase == &fHead) {
                continue;
            }
            SkOpSpan* span = spanBase->upCastable();
            // FIXME?: this assumes that if the opposite segment is coincident then no more
            // coincidence needs to be detected. This may not be true.
            if (span && span->containsCoincidence(opp)) { 
                continue;
            }
            if (spanBase->segment() == opp) {
                continue;
            }
            if (spanBase->containsCoinEnd(opp)) {
                continue;
            } 
            SkOpPtT* priorPtT = nullptr, * priorStopPtT;
            // find prior span containing opp segment
            SkOpSegment* priorOpp = nullptr;
            SkOpSpan* priorTest = spanBase->prev();
            while (!priorOpp && priorTest) {
                priorStopPtT = priorPtT = priorTest->ptT();
                while ((priorPtT = priorPtT->next()) != priorStopPtT) {
                    if (priorPtT->deleted()) {
                        continue;
                    }
                    SkOpSegment* segment = priorPtT->span()->segment();
                    if (segment == opp) {
                        prior = priorTest;
                        priorOpp = opp;
                        break;
                    }
                }
                priorTest = priorTest->prev();
            }
            if (!priorOpp) {
                continue;
            }
            if (priorPtT == ptT) {
                continue;
            }
            SkOpPtT* oppStart = prior->ptT();
            SkOpPtT* oppEnd = spanBase->ptT();
            bool swapped = priorPtT->fT > ptT->fT;
            if (swapped) {
                SkTSwap(priorPtT, ptT);
                SkTSwap(oppStart, oppEnd);
            }
            bool flipped = oppStart->fT > oppEnd->fT;
            bool coincident = false;
            if (coincidences->contains(priorPtT, ptT, oppStart, oppEnd, flipped)) {
                goto swapBack;
            }
            if (opp->verb() == SkPath::kLine_Verb) {
                coincident = (SkDPoint::ApproximatelyEqual(priorPtT->fPt, oppStart->fPt) ||
                        SkDPoint::ApproximatelyEqual(priorPtT->fPt, oppEnd->fPt)) &&
                        (SkDPoint::ApproximatelyEqual(ptT->fPt, oppStart->fPt) ||
                        SkDPoint::ApproximatelyEqual(ptT->fPt, oppEnd->fPt));
            }
            if (!coincident) {
                coincident = testForCoincidence(priorPtT, ptT, prior, spanBase, opp, 5000);
            }
            if (coincident) {
            // mark coincidence
                if (!coincidences->extend(priorPtT, ptT, oppStart, oppEnd)
                        && !coincidences->extend(oppStart, oppEnd, priorPtT, ptT)) {
                    coincidences->add(priorPtT, ptT, oppStart, oppEnd, allocator);
                }
                clear_visited(&fHead);
                return true;
            }
    swapBack:
            if (swapped) {
                SkTSwap(priorPtT, ptT);
            }
        }
    } while ((spanBase = spanBase->final() ? nullptr : spanBase->upCast()->next()));
    clear_visited(&fHead);
    return false;
}

// if a span has more than one intersection, merge the other segments' span as needed
void SkOpSegment::moveMultiples() {
    debugValidate();
    SkOpSpanBase* test = &fHead;
    do {
        int addCount = test->spanAddsCount();
        SkASSERT(addCount >= 1);
        if (addCount == 1) {
            continue;
        }
        SkOpPtT* startPtT = test->ptT();
        SkOpPtT* testPtT = startPtT;
        do {  // iterate through all spans associated with start
            SkOpSpanBase* oppSpan = testPtT->span();
            if (oppSpan->spanAddsCount() == addCount) {
                continue;
            }
            if (oppSpan->deleted()) {
                continue;
            }
            SkOpSegment* oppSegment = oppSpan->segment();
            if (oppSegment == this) {
                continue;
            }
            // find range of spans to consider merging
            SkOpSpanBase* oppPrev = oppSpan;
            SkOpSpanBase* oppFirst = oppSpan;
            while ((oppPrev = oppPrev->prev())) {
                if (!roughly_equal(oppPrev->t(), oppSpan->t())) {
                    break;
                }
                if (oppPrev->spanAddsCount() == addCount) {
                    continue;
                }
                if (oppPrev->deleted()) {
                    continue;
                }
                oppFirst = oppPrev;
            }
            SkOpSpanBase* oppNext = oppSpan;
            SkOpSpanBase* oppLast = oppSpan;
            while ((oppNext = oppNext->final() ? nullptr : oppNext->upCast()->next())) {
                if (!roughly_equal(oppNext->t(), oppSpan->t())) {
                    break;
                }
                if (oppNext->spanAddsCount() == addCount) {
                    continue;
                }
                if (oppNext->deleted()) {
                    continue;
                }
                oppLast = oppNext;
            }
            if (oppFirst == oppLast) {
                continue;
            }
            SkOpSpanBase* oppTest = oppFirst;
            do {
                if (oppTest == oppSpan) {
                    continue;
                }
                // check to see if the candidate meets specific criteria:
                // it contains spans of segments in test's loop but not including 'this'
                SkOpPtT* oppStartPtT = oppTest->ptT();
                SkOpPtT* oppPtT = oppStartPtT;
                while ((oppPtT = oppPtT->next()) != oppStartPtT) {
                    SkOpSegment* oppPtTSegment = oppPtT->segment();
                    if (oppPtTSegment == this) {
                        goto tryNextSpan;
                    }
                    SkOpPtT* matchPtT = startPtT;
                    do {
                        if (matchPtT->segment() == oppPtTSegment) {
                            goto foundMatch;
                        }
                    } while ((matchPtT = matchPtT->next()) != startPtT);
                    goto tryNextSpan;
            foundMatch:  // merge oppTest and oppSpan
                    oppSegment->debugValidate();
                    if (oppTest == &oppSegment->fTail || oppTest == &oppSegment->fHead) {
                        SkASSERT(oppSpan != &oppSegment->fHead); // don't expect collapse
                        SkASSERT(oppSpan != &oppSegment->fTail);
                        oppTest->merge(oppSpan->upCast());
                    } else {
                        oppSpan->merge(oppTest->upCast());
                    }
                    oppSegment->debugValidate();
                    goto checkNextSpan;
                }
        tryNextSpan: 
                ;
            } while (oppTest != oppLast && (oppTest = oppTest->upCast()->next()));
        } while ((testPtT = testPtT->next()) != startPtT);
checkNextSpan: 
        ;
    } while ((test = test->final() ? nullptr : test->upCast()->next()));
    debugValidate();
}

// Move nearby t values and pts so they all hang off the same span. Alignment happens later.
void SkOpSegment::moveNearby() {
    debugValidate();
    SkOpSpanBase* spanS = &fHead;
    do {
        SkOpSpanBase* test = spanS->upCast()->next();
        SkOpSpanBase* next;
        if (spanS->contains(test)) {
            if (!test->final()) {
                test->upCast()->detach(spanS->ptT());
                continue;
            } else if (spanS != &fHead) {
                spanS->upCast()->detach(test->ptT());
                spanS = test;
                continue;
            }
        }
        do {  // iterate through all spans associated with start
            SkOpPtT* startBase = spanS->ptT();
            next = test->final() ? nullptr : test->upCast()->next();
            do {
                SkOpPtT* testBase = test->ptT();
                do {
                    if (startBase == testBase) {
                        goto checkNextSpan;
                    }
                    if (testBase->duplicate()) {
                        continue;
                    }
                    if (this->match(startBase, testBase->segment(), testBase->fT, testBase->fPt)) {
                        if (test == &this->fTail) {
                            if (spanS == &fHead) {
                                debugValidate();
                                return;  // if this span has collapsed, remove it from parent
                            }
                            this->fTail.merge(spanS->upCast());
                            debugValidate();
                            return;
                        }
                        spanS->merge(test->upCast());
                        goto checkNextSpan;
                    }
                } while ((testBase = testBase->next()) != test->ptT());
            } while ((startBase = startBase->next()) != spanS->ptT());
    checkNextSpan:
            ;
        } while ((test = next));
        spanS = spanS->upCast()->next();
    } while (!spanS->final());
    debugValidate();
}

bool SkOpSegment::operand() const {
    return fContour->operand();
}

bool SkOpSegment::oppXor() const {
    return fContour->oppXor();
}

bool SkOpSegment::ptsDisjoint(double t1, const SkPoint& pt1, double t2, const SkPoint& pt2) const {
    if (fVerb == SkPath::kLine_Verb) {
        return false;
    }
    // quads (and cubics) can loop back to nearly a line so that an opposite curve
    // hits in two places with very different t values.
    // OPTIMIZATION: curves could be preflighted so that, for example, something like
    // 'controls contained by ends' could avoid this check for common curves
    // 'ends are extremes in x or y' is cheaper to compute and real-world common
    // on the other hand, the below check is relatively inexpensive
    double midT = (t1 + t2) / 2;
    SkPoint midPt = this->ptAtT(midT);
    double seDistSq = SkTMax(pt1.distanceToSqd(pt2) * 2, FLT_EPSILON * 2);
    return midPt.distanceToSqd(pt1) > seDistSq || midPt.distanceToSqd(pt2) > seDistSq;
}

void SkOpSegment::setUpWindings(SkOpSpanBase* start, SkOpSpanBase* end, int* sumMiWinding,
        int* maxWinding, int* sumWinding) {
    int deltaSum = SpanSign(start, end);
    *maxWinding = *sumMiWinding;
    *sumWinding = *sumMiWinding -= deltaSum;
    SkASSERT(!DEBUG_LIMIT_WIND_SUM || SkTAbs(*sumWinding) <= DEBUG_LIMIT_WIND_SUM);
}

void SkOpSegment::setUpWindings(SkOpSpanBase* start, SkOpSpanBase* end, int* sumMiWinding,
        int* sumSuWinding, int* maxWinding, int* sumWinding, int* oppMaxWinding,
        int* oppSumWinding) {
    int deltaSum = SpanSign(start, end);
    int oppDeltaSum = OppSign(start, end);
    if (operand()) {
        *maxWinding = *sumSuWinding;
        *sumWinding = *sumSuWinding -= deltaSum;
        *oppMaxWinding = *sumMiWinding;
        *oppSumWinding = *sumMiWinding -= oppDeltaSum;
    } else {
        *maxWinding = *sumMiWinding;
        *sumWinding = *sumMiWinding -= deltaSum;
        *oppMaxWinding = *sumSuWinding;
        *oppSumWinding = *sumSuWinding -= oppDeltaSum;
    }
    SkASSERT(!DEBUG_LIMIT_WIND_SUM || SkTAbs(*sumWinding) <= DEBUG_LIMIT_WIND_SUM);
    SkASSERT(!DEBUG_LIMIT_WIND_SUM || SkTAbs(*oppSumWinding) <= DEBUG_LIMIT_WIND_SUM);
}

void SkOpSegment::sortAngles() {
    SkOpSpanBase* span = &this->fHead;
    do {
        SkOpAngle* fromAngle = span->fromAngle();
        SkOpAngle* toAngle = span->final() ? nullptr : span->upCast()->toAngle();
        if (!fromAngle && !toAngle) {
            continue;
        }
#if DEBUG_ANGLE
        bool wroteAfterHeader = false;
#endif
        SkOpAngle* baseAngle = fromAngle;
        if (fromAngle && toAngle) {
#if DEBUG_ANGLE
            SkDebugf("%s [%d] tStart=%1.9g [%d]\n", __FUNCTION__, debugID(), span->t(),
                    span->debugID());
            wroteAfterHeader = true;
#endif
            fromAngle->insert(toAngle);
        } else if (!fromAngle) {
            baseAngle = toAngle;
        }
        SkOpPtT* ptT = span->ptT(), * stopPtT = ptT;
        do {
            SkOpSpanBase* oSpan = ptT->span();
            if (oSpan == span) {
                continue;
            }
            SkOpAngle* oAngle = oSpan->fromAngle();
            if (oAngle) {
#if DEBUG_ANGLE
                if (!wroteAfterHeader) {
                    SkDebugf("%s [%d] tStart=%1.9g [%d]\n", __FUNCTION__, debugID(),
                            span->t(), span->debugID());
                    wroteAfterHeader = true;
                }
#endif
                if (!oAngle->loopContains(baseAngle)) {
                    baseAngle->insert(oAngle);
                }
            }
            if (!oSpan->final()) {
                oAngle = oSpan->upCast()->toAngle();
                if (oAngle) {
#if DEBUG_ANGLE
                    if (!wroteAfterHeader) {
                        SkDebugf("%s [%d] tStart=%1.9g [%d]\n", __FUNCTION__, debugID(),
                                span->t(), span->debugID());
                        wroteAfterHeader = true;
                    }
#endif
                    if (!oAngle->loopContains(baseAngle)) {
                        baseAngle->insert(oAngle);
                    }
                }
            }
        } while ((ptT = ptT->next()) != stopPtT);
        if (baseAngle->loopCount() == 1) {
            span->setFromAngle(nullptr);
            if (toAngle) {
                span->upCast()->setToAngle(nullptr);
            }
            baseAngle = nullptr;
        }
#if DEBUG_SORT
        SkASSERT(!baseAngle || baseAngle->loopCount() > 1);
#endif
    } while (!span->final() && (span = span->upCast()->next()));
}

// return true if midpoints were computed
bool SkOpSegment::subDivide(const SkOpSpanBase* start, const SkOpSpanBase* end,
        SkOpCurve* edge) const {
    SkASSERT(start != end);
    const SkOpPtT& startPtT = *start->ptT();
    const SkOpPtT& endPtT = *end->ptT();
    SkDEBUGCODE(edge->fVerb = fVerb);
    edge->fPts[0] = startPtT.fPt;
    int points = SkPathOpsVerbToPoints(fVerb);
    edge->fPts[points] = endPtT.fPt;
    edge->fWeight = 1;
    if (fVerb == SkPath::kLine_Verb) {
        return false;
    }
    double startT = startPtT.fT;
    double endT = endPtT.fT;
    if ((startT == 0 || endT == 0) && (startT == 1 || endT == 1)) {
        // don't compute midpoints if we already have them
        if (fVerb == SkPath::kQuad_Verb) {
            edge->fPts[1] = fPts[1];
            return false;
        }
        if (fVerb == SkPath::kConic_Verb) {
            edge->fPts[1] = fPts[1];
            edge->fWeight = fWeight;
            return false;
        }
        SkASSERT(fVerb == SkPath::kCubic_Verb);
        if (start < end) {
            edge->fPts[1] = fPts[1];
            edge->fPts[2] = fPts[2];
            return false;
        }
        edge->fPts[1] = fPts[2];
        edge->fPts[2] = fPts[1];
        return false;
    }
    const SkDPoint sub[2] = {{ edge->fPts[0].fX, edge->fPts[0].fY},
            {edge->fPts[points].fX, edge->fPts[points].fY }};
    if (fVerb == SkPath::kQuad_Verb) {
        edge->fPts[1] = SkDQuad::SubDivide(fPts, sub[0], sub[1], startT, endT).asSkPoint();
    } else if (fVerb == SkPath::kConic_Verb) {
        edge->fPts[1] = SkDConic::SubDivide(fPts, fWeight, sub[0], sub[1],
                startT, endT, &edge->fWeight).asSkPoint();
    } else {
        SkASSERT(fVerb == SkPath::kCubic_Verb);
        SkDPoint ctrl[2];
        SkDCubic::SubDivide(fPts, sub[0], sub[1], startT, endT, ctrl);
        edge->fPts[1] = ctrl[0].asSkPoint();
        edge->fPts[2] = ctrl[1].asSkPoint();
    }
    return true;
}

bool SkOpSegment::subDivide(const SkOpSpanBase* start, const SkOpSpanBase* end,
        SkDCurve* edge) const {
    SkASSERT(start != end);
    const SkOpPtT& startPtT = *start->ptT();
    const SkOpPtT& endPtT = *end->ptT();
    SkDEBUGCODE(edge->fVerb = fVerb);
    edge->fCubic[0].set(startPtT.fPt);
    int points = SkPathOpsVerbToPoints(fVerb);
    edge->fCubic[points].set(endPtT.fPt);
    if (fVerb == SkPath::kLine_Verb) {
        return false;
    }
    double startT = startPtT.fT;
    double endT = endPtT.fT;
    if ((startT == 0 || endT == 0) && (startT == 1 || endT == 1)) {
        // don't compute midpoints if we already have them
        if (fVerb == SkPath::kQuad_Verb) {
            edge->fLine[1].set(fPts[1]);
            return false;
        }
        if (fVerb == SkPath::kConic_Verb) {
            edge->fConic[1].set(fPts[1]);
            edge->fConic.fWeight = fWeight;
            return false;
        }
        SkASSERT(fVerb == SkPath::kCubic_Verb);
        if (startT == 0) {
            edge->fCubic[1].set(fPts[1]);
            edge->fCubic[2].set(fPts[2]);
            return false;
        }
        edge->fCubic[1].set(fPts[2]);
        edge->fCubic[2].set(fPts[1]);
        return false;
    }
    if (fVerb == SkPath::kQuad_Verb) {
        edge->fQuad[1] = SkDQuad::SubDivide(fPts, edge->fQuad[0], edge->fQuad[2], startT, endT);
    } else if (fVerb == SkPath::kConic_Verb) {
        edge->fConic[1] = SkDConic::SubDivide(fPts, fWeight, edge->fQuad[0], edge->fQuad[2],
            startT, endT, &edge->fConic.fWeight);
    } else {
        SkASSERT(fVerb == SkPath::kCubic_Verb);
        SkDCubic::SubDivide(fPts, edge->fCubic[0], edge->fCubic[3], startT, endT, &edge->fCubic[1]);
    }
    return true;
}

bool SkOpSegment::testForCoincidence(const SkOpPtT* priorPtT, const SkOpPtT* ptT,
        const SkOpSpanBase* prior, const SkOpSpanBase* spanBase, const SkOpSegment* opp,
        SkScalar flatnessLimit) const {
    // average t, find mid pt
    double midT = (prior->t() + spanBase->t()) / 2;
    SkPoint midPt = this->ptAtT(midT);
    bool coincident = true;
    // if the mid pt is not near either end pt, project perpendicular through opp seg
    if (!SkDPoint::ApproximatelyEqual(priorPtT->fPt, midPt)
            && !SkDPoint::ApproximatelyEqual(ptT->fPt, midPt)) {
        coincident = false;
        SkIntersections i;
        SkVector dxdy = (*CurveSlopeAtT[fVerb])(this->pts(), this->weight(), midT);
        SkDLine ray = {{{midPt.fX, midPt.fY}, {midPt.fX + dxdy.fY, midPt.fY - dxdy.fX}}};
        (*CurveIntersectRay[opp->verb()])(opp->pts(), opp->weight(), ray, &i);
        // measure distance and see if it's small enough to denote coincidence
        for (int index = 0; index < i.used(); ++index) {
            SkDPoint oppPt = i.pt(index);
            if (oppPt.approximatelyEqual(midPt)) {
                SkVector oppDxdy = (*CurveSlopeAtT[opp->verb()])(opp->pts(),
                        opp->weight(), i[index][0]);
                oppDxdy.normalize();
                dxdy.normalize();
                SkScalar flatness = SkScalarAbs(dxdy.cross(oppDxdy) / FLT_EPSILON);
                coincident |= flatness < flatnessLimit;
            }
        }
    }
    return coincident;
}

void SkOpSegment::undoneSpan(SkOpSpanBase** start, SkOpSpanBase** end) {
    SkOpSpan* span = this->head();
    do {
        if (!span->done()) {
            break;
        }
    } while ((span = span->next()->upCastable()));
    SkASSERT(span);
    *start = span;
    *end = span->next();
}

int SkOpSegment::updateOppWinding(const SkOpSpanBase* start, const SkOpSpanBase* end) const {
    const SkOpSpan* lesser = start->starter(end);
    int oppWinding = lesser->oppSum();
    int oppSpanWinding = SkOpSegment::OppSign(start, end);
    if (oppSpanWinding && UseInnerWinding(oppWinding - oppSpanWinding, oppWinding)
            && oppWinding != SK_MaxS32) {
        oppWinding -= oppSpanWinding;
    }
    return oppWinding;
}

int SkOpSegment::updateOppWinding(const SkOpAngle* angle) const {
    const SkOpSpanBase* startSpan = angle->start();
    const SkOpSpanBase* endSpan = angle->end();
    return updateOppWinding(endSpan, startSpan);
}

int SkOpSegment::updateOppWindingReverse(const SkOpAngle* angle) const {
    const SkOpSpanBase* startSpan = angle->start();
    const SkOpSpanBase* endSpan = angle->end();
    return updateOppWinding(startSpan, endSpan);
}

int SkOpSegment::updateWinding(SkOpSpanBase* start, SkOpSpanBase* end) {
    SkOpSpan* lesser = start->starter(end);
    int winding = lesser->windSum();
    if (winding == SK_MinS32) {
        winding = lesser->computeWindSum();
    }
    if (winding == SK_MinS32) {
        return winding;
    }
    int spanWinding = SkOpSegment::SpanSign(start, end);
    if (winding && UseInnerWinding(winding - spanWinding, winding)
            && winding != SK_MaxS32) {
        winding -= spanWinding;
    }
    return winding;
}

int SkOpSegment::updateWinding(SkOpAngle* angle) {
    SkOpSpanBase* startSpan = angle->start();
    SkOpSpanBase* endSpan = angle->end();
    return updateWinding(endSpan, startSpan);
}

int SkOpSegment::updateWindingReverse(const SkOpAngle* angle) {
    SkOpSpanBase* startSpan = angle->start();
    SkOpSpanBase* endSpan = angle->end();
    return updateWinding(startSpan, endSpan);
}

// OPTIMIZATION: does the following also work, and is it any faster?
// return outerWinding * innerWinding > 0
//      || ((outerWinding + innerWinding < 0) ^ ((outerWinding - innerWinding) < 0)))
bool SkOpSegment::UseInnerWinding(int outerWinding, int innerWinding) {
    SkASSERT(outerWinding != SK_MaxS32);
    SkASSERT(innerWinding != SK_MaxS32);
    int absOut = SkTAbs(outerWinding);
    int absIn = SkTAbs(innerWinding);
    bool result = absOut == absIn ? outerWinding < 0 : absOut < absIn;
    return result;
}

int SkOpSegment::windSum(const SkOpAngle* angle) const {
    const SkOpSpan* minSpan = angle->start()->starter(angle->end());
    return minSpan->windSum();
}
