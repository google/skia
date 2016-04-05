/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkOpCoincidence.h"
#include "SkOpSegment.h"
#include "SkPathOpsTSect.h"

bool SkOpCoincidence::extend(SkOpPtT* coinPtTStart, SkOpPtT* coinPtTEnd, SkOpPtT* oppPtTStart,
        SkOpPtT* oppPtTEnd) {
    // if there is an existing pair that overlaps the addition, extend it
    SkCoincidentSpans* coinRec = fHead;
    if (coinRec) {
        do {
            if (coinRec->fCoinPtTStart->segment() != coinPtTStart->segment()) {
                continue;
            }
            if (coinRec->fOppPtTStart->segment() != oppPtTStart->segment()) {
                continue;
            }
            if (coinRec->fCoinPtTStart->fT > coinPtTEnd->fT) {
                continue;
            }
            if (coinRec->fCoinPtTEnd->fT < coinPtTStart->fT) {
                continue;
            }
            if (coinRec->fCoinPtTStart->fT > coinPtTStart->fT) {
                coinRec->fCoinPtTStart = coinPtTStart;
                coinRec->fOppPtTStart = oppPtTStart;
            }
            if (coinRec->fCoinPtTEnd->fT < coinPtTEnd->fT) {
                coinRec->fCoinPtTEnd = coinPtTEnd;
                coinRec->fOppPtTEnd = oppPtTEnd;
            }
            return true;
        } while ((coinRec = coinRec->fNext));
    }
    return false;
}

void SkOpCoincidence::add(SkOpPtT* coinPtTStart, SkOpPtT* coinPtTEnd, SkOpPtT* oppPtTStart,
        SkOpPtT* oppPtTEnd, SkChunkAlloc* allocator) {
    SkASSERT(coinPtTStart->fT < coinPtTEnd->fT);
    bool flipped = oppPtTStart->fT > oppPtTEnd->fT;
    SkCoincidentSpans* coinRec = SkOpTAllocator<SkCoincidentSpans>::Allocate(allocator);
    coinRec->fNext = this->fHead;
    coinRec->fCoinPtTStart = coinPtTStart;
    coinRec->fCoinPtTEnd = coinPtTEnd;
    coinRec->fOppPtTStart = oppPtTStart;
    coinRec->fOppPtTEnd = oppPtTEnd;
    coinRec->fFlipped = flipped;
    SkDEBUGCODE(coinRec->fID = fDebugState->nextCoinID());

    this->fHead = coinRec;
}

static void t_range(const SkOpPtT* overS, const SkOpPtT* overE, double tStart, double tEnd,
        const SkOpPtT* coinPtTStart, const SkOpPtT* coinPtTEnd, double* coinTs, double* coinTe) {
    double denom = overE->fT - overS->fT;
    double start = 0 < denom ? tStart : tEnd;
    double end = 0 < denom ? tEnd : tStart;
    double sRatio = (start - overS->fT) / denom;
    double eRatio = (end - overS->fT) / denom;
    *coinTs = coinPtTStart->fT + (coinPtTEnd->fT - coinPtTStart->fT) * sRatio;
    *coinTe = coinPtTStart->fT + (coinPtTEnd->fT - coinPtTStart->fT) * eRatio;
}

bool SkOpCoincidence::addExpanded(SkChunkAlloc* allocator
        PATH_OPS_DEBUG_VALIDATE_PARAMS(SkOpGlobalState* globalState)) {
#if DEBUG_VALIDATE
    globalState->setPhase(SkOpGlobalState::kIntersecting);
#endif
    // for each coincident pair, match the spans
    // if the spans don't match, add the mssing pt to the segment and loop it in the opposite span
    SkCoincidentSpans* coin = this->fHead;
    SkASSERT(coin);
    do {
        SkOpPtT* startPtT = coin->fCoinPtTStart;
        SkOpPtT* oStartPtT = coin->fOppPtTStart;
        SkASSERT(startPtT->contains(oStartPtT));
        SkASSERT(coin->fCoinPtTEnd->contains(coin->fOppPtTEnd));
        SkOpSpanBase* start = startPtT->span();
        SkOpSpanBase* oStart = oStartPtT->span();
        const SkOpSpanBase* end = coin->fCoinPtTEnd->span();
        const SkOpSpanBase* oEnd = coin->fOppPtTEnd->span();
        if (oEnd->deleted()) {
            return false;
        }
        SkOpSpanBase* test = start->upCast()->next();
        SkOpSpanBase* oTest = coin->fFlipped ? oStart->prev() : oStart->upCast()->next();
        while (test != end || oTest != oEnd) {
            if (!test->ptT()->contains(oTest->ptT())) {
                // use t ranges to guess which one is missing
                double startRange = coin->fCoinPtTEnd->fT - startPtT->fT;
                if (!startRange) {
                    return false;
                }
                double startPart = (test->t() - startPtT->fT) / startRange;
                double oStartRange = coin->fOppPtTEnd->fT - oStartPtT->fT;
                if (!oStartRange) {
                    return false;
                }
                double oStartPart = (oTest->t() - oStartPtT->fT) / oStartRange;
                if (startPart == oStartPart) {
                    return false;
                }
                SkOpPtT* newPt;
                if (startPart < oStartPart) {
                    double newT = oStartPtT->fT + oStartRange * startPart;
                    newPt = oStart->segment()->addT(newT, SkOpSegment::kAllowAlias, allocator);
                    if (!newPt) {
                        return false;
                    }
                    newPt->fPt = test->pt();
                    test->ptT()->addOpp(newPt);
                } else {
                    double newT = startPtT->fT + startRange * oStartPart;
                    newPt = start->segment()->addT(newT, SkOpSegment::kAllowAlias, allocator);
                    if (!newPt) {
                        return false;
                    }
                    newPt->fPt = oTest->pt();
                    oTest->ptT()->addOpp(newPt);
                }
                // start over
                test = start;
                oTest = oStart;
            }
            if (test != end) {
                test = test->upCast()->next();
            }
            if (oTest != oEnd) {
                oTest = coin->fFlipped ? oTest->prev() : oTest->upCast()->next();
            }
        }
    } while ((coin = coin->fNext));
#if DEBUG_VALIDATE
    globalState->setPhase(SkOpGlobalState::kWalking);
#endif
    return true;
}

bool SkOpCoincidence::addIfMissing(const SkCoincidentSpans* outer, SkOpPtT* over1s,
            SkOpPtT* over1e, SkChunkAlloc* allocator) {
    SkCoincidentSpans* check = this->fTop;
    do {
        if (check->fCoinPtTStart->span() == over1s->span()
                && check->fOppPtTStart->span() == outer->fOppPtTStart->span()) {
            SkASSERT(check->fCoinPtTEnd->span() == over1e->span()
                    || !fDebugState->debugRunFail());
            SkASSERT(check->fOppPtTEnd->span() == outer->fOppPtTEnd->span()
                    || !fDebugState->debugRunFail());
            return false;
        }
        if (check->fCoinPtTStart->span() == outer->fCoinPtTStart->span()
                && check->fOppPtTStart->span() == over1s->span()) {
            SkASSERT(check->fCoinPtTEnd->span() == outer->fCoinPtTEnd->span()
                    || !fDebugState->debugRunFail());
            SkASSERT(check->fOppPtTEnd->span() == over1e->span()
                    || !fDebugState->debugRunFail());
            return false;
        }
    } while ((check = check->fNext));
    this->add(outer->fCoinPtTStart, outer->fCoinPtTEnd, over1s, over1e, allocator);
#if 0
    // FIXME: up to four flavors could be added -- do we need only one?
#endif
    return true;
}

bool SkOpCoincidence::addIfMissing(const SkOpPtT* over1s, const SkOpPtT* over1e,
                      const SkOpPtT* over2s, const SkOpPtT* over2e, double tStart, double tEnd,
        SkOpPtT* coinPtTStart, const SkOpPtT* coinPtTEnd,
        SkOpPtT* oppPtTStart, const SkOpPtT* oppPtTEnd, SkChunkAlloc* allocator) {
    double coinTs, coinTe, oppTs, oppTe;
    t_range(over1s, over1e, tStart, tEnd, coinPtTStart, coinPtTEnd, &coinTs, &coinTe);
    t_range(over2s, over2e, tStart, tEnd, oppPtTStart, oppPtTEnd, &oppTs, &oppTe);
    SkOpSegment* coinSeg = coinPtTStart->segment();
    SkOpSegment* oppSeg = oppPtTStart->segment();
    SkASSERT(coinSeg != oppSeg);
    SkCoincidentSpans* check = this->fTop;
    do {
        const SkOpSegment* checkCoinSeg = check->fCoinPtTStart->segment();
        if (checkCoinSeg != coinSeg && checkCoinSeg != oppSeg) {
            continue;
        }
        const SkOpSegment* checkOppSeg = check->fOppPtTStart->segment();
        if (checkOppSeg != coinSeg && checkOppSeg != oppSeg) {
            continue;
        }
        int cTs = coinTs;
        int cTe = coinTe;
        int oTs = oppTs;
        int oTe = oppTe;
        if (checkCoinSeg != coinSeg) {
            SkASSERT(checkOppSeg != oppSeg);
            SkTSwap(cTs, oTs);
            SkTSwap(cTe, oTe);
        }
        int tweenCount = (int) between(check->fCoinPtTStart->fT, cTs, check->fCoinPtTEnd->fT)
                       + (int) between(check->fCoinPtTStart->fT, cTe, check->fCoinPtTEnd->fT)
                       + (int) between(check->fOppPtTStart->fT, oTs, check->fOppPtTEnd->fT)
                       + (int) between(check->fOppPtTStart->fT, oTe, check->fOppPtTEnd->fT);
//        SkASSERT(tweenCount == 0 || tweenCount == 4);
        if (tweenCount) {
            return false;
        }
    } while ((check = check->fNext));
    if ((over1s->fT < over1e->fT) != (over2s->fT < over2e->fT)) {
        SkTSwap(oppTs, oppTe);
    }
    if (coinTs > coinTe) {
        SkTSwap(coinTs, coinTe);
        SkTSwap(oppTs, oppTe);
    }
    SkOpPtT* cs = coinSeg->addMissing(coinTs, oppSeg, allocator);
    SkOpPtT* ce = coinSeg->addMissing(coinTe, oppSeg, allocator);
    SkASSERT(cs != ce);
    SkOpPtT* os = oppSeg->addMissing(oppTs, coinSeg, allocator);
    SkOpPtT* oe = oppSeg->addMissing(oppTe, coinSeg, allocator);
//    SkASSERT(os != oe);
    cs->addOpp(os);
    ce->addOpp(oe);
    this->add(cs, ce, os, oe, allocator);
    return true;
}

/* detects overlaps of different coincident runs on same segment */
/* does not detect overlaps for pairs without any segments in common */
bool SkOpCoincidence::addMissing(SkChunkAlloc* allocator) {
    SkCoincidentSpans* outer = fHead;
    if (!outer) {
        return true;
    }
    bool added = false;
    fTop = outer;
    fHead = nullptr;
    do {
    // addifmissing can modify the list that this is walking
    // save head so that walker can iterate over old data unperturbed
    // addifmissing adds to head freely then add saved head in the end
        const SkOpSegment* outerCoin = outer->fCoinPtTStart->segment();
        SkASSERT(outerCoin == outer->fCoinPtTEnd->segment());
        const SkOpSegment* outerOpp = outer->fOppPtTStart->segment();
        SkASSERT(outerOpp == outer->fOppPtTEnd->segment());
        SkCoincidentSpans* inner = outer;
        while ((inner = inner->fNext)) {
            double overS, overE;
            const SkOpSegment* innerCoin = inner->fCoinPtTStart->segment();
            SkASSERT(innerCoin == inner->fCoinPtTEnd->segment());
            const SkOpSegment* innerOpp = inner->fOppPtTStart->segment();
            SkASSERT(innerOpp == inner->fOppPtTEnd->segment());
            if (outerCoin == innerCoin
                    && this->overlap(outer->fCoinPtTStart, outer->fCoinPtTEnd,
                    inner->fCoinPtTStart, inner->fCoinPtTEnd, &overS, &overE)) {
                added |= this->addIfMissing(outer->fCoinPtTStart, outer->fCoinPtTEnd,
                        inner->fCoinPtTStart, inner->fCoinPtTEnd, overS, overE,
                        outer->fOppPtTStart, outer->fOppPtTEnd,
                        inner->fOppPtTStart, inner->fOppPtTEnd, allocator);
            } else if (outerCoin == innerOpp
                    && this->overlap(outer->fCoinPtTStart, outer->fCoinPtTEnd,
                    inner->fOppPtTStart, inner->fOppPtTEnd, &overS, &overE)) {
                added |= this->addIfMissing(outer->fCoinPtTStart, outer->fCoinPtTEnd,
                        inner->fOppPtTStart, inner->fOppPtTEnd, overS, overE,
                        outer->fOppPtTStart, outer->fOppPtTEnd,
                        inner->fCoinPtTStart, inner->fCoinPtTEnd, allocator);
            } else if (outerOpp == innerCoin
                    && this->overlap(outer->fOppPtTStart, outer->fOppPtTEnd,
                    inner->fCoinPtTStart, inner->fCoinPtTEnd, &overS, &overE)) {
                added |= this->addIfMissing(outer->fOppPtTStart, outer->fOppPtTEnd,
                        inner->fCoinPtTStart, inner->fCoinPtTEnd, overS, overE,
                        outer->fCoinPtTStart, outer->fCoinPtTEnd,
                        inner->fOppPtTStart, inner->fOppPtTEnd, allocator);
            } else if (outerOpp == innerOpp
                    && this->overlap(outer->fOppPtTStart, outer->fOppPtTEnd,
                    inner->fOppPtTStart, inner->fOppPtTEnd, &overS, &overE)) {
                added |= this->addIfMissing(outer->fOppPtTStart, outer->fOppPtTEnd,
                        inner->fOppPtTStart, inner->fOppPtTEnd, overS, overE,
                        outer->fCoinPtTStart, outer->fCoinPtTEnd,
                        inner->fCoinPtTStart, inner->fCoinPtTEnd, allocator);
            } else if (outerCoin != innerCoin) {
                // check to see if outer span overlaps the inner span
                    // look for inner segment in pt-t list
                    // if present, and if t values are in coincident range
                    // add two pairs of new coincidence
                SkOpPtT* testS = outer->fCoinPtTStart->contains(innerCoin);
                SkOpPtT* testE = outer->fCoinPtTEnd->contains(innerCoin);
                if (testS && testS->fT >= inner->fCoinPtTStart->fT
                        && testE && testE->fT <= inner->fCoinPtTEnd->fT
                        && this->testForCoincidence(outer, testS, testE)) {
                    added |= this->addIfMissing(outer, testS, testE, allocator);
                } else {
                    testS = inner->fCoinPtTStart->contains(outerCoin);
                    testE = inner->fCoinPtTEnd->contains(outerCoin);
                    if (testS && testS->fT >= outer->fCoinPtTStart->fT
                            && testE && testE->fT <= outer->fCoinPtTEnd->fT
                            && this->testForCoincidence(inner, testS, testE)) {
                        added |= this->addIfMissing(inner, testS, testE, allocator);
                    }
                }
            }
#if 0 && DEBUG_COINCIDENCE
            SkString miss;
            miss.printf("addMissing inner=%d outer=%d", inner->debugID(), outer->debugID());
            DEBUG_COINCIDENCE_HEALTH(fDebugState->contourHead(), miss.c_str());
#endif
        }
    } while ((outer = outer->fNext));
    SkCoincidentSpans** headPtr = &fHead;
    while (*headPtr) {
        SkCoincidentSpans** headNext = &(*headPtr)->fNext;
        if (*headNext) {
            break;
        }
        headPtr = headNext;
    }
    *headPtr = fTop;
    return added;
}

void SkOpCoincidence::addOverlap(SkOpSegment* seg1, SkOpSegment* seg1o, SkOpSegment* seg2,
        SkOpSegment* seg2o, SkOpPtT* overS, SkOpPtT* overE, SkChunkAlloc* allocator) {
    SkOpPtT* s1 = overS->find(seg1);
    SkOpPtT* e1 = overE->find(seg1);
    if (!s1->starter(e1)->span()->upCast()->windValue()) {
        s1 = overS->find(seg1o);
        e1 = overE->find(seg1o);
        if (!s1->starter(e1)->span()->upCast()->windValue()) {
            return;
        }
    }
    SkOpPtT* s2 = overS->find(seg2);
    SkOpPtT* e2 = overE->find(seg2);
    if (!s2->starter(e2)->span()->upCast()->windValue()) {
        s2 = overS->find(seg2o);
        e2 = overE->find(seg2o);
        if (!s2->starter(e2)->span()->upCast()->windValue()) {
            return;
        }
    }
    if (s1->segment() == s2->segment()) {
        return;
    }
    if (s1->fT > e1->fT) {
        SkTSwap(s1, e1);
        SkTSwap(s2, e2);
    }
    this->add(s1, e1, s2, e2, allocator);
}

bool SkOpCoincidence::contains(const SkOpPtT* coinPtTStart, const SkOpPtT* coinPtTEnd,
        const SkOpPtT* oppPtTStart, const SkOpPtT* oppPtTEnd, bool flipped) const {
    const SkCoincidentSpans* coin = fHead;
    if (!coin) {
        return false;
    }
    do {
        if (coin->fCoinPtTStart == coinPtTStart &&  coin->fCoinPtTEnd == coinPtTEnd
                && coin->fOppPtTStart == oppPtTStart && coin->fOppPtTEnd == oppPtTEnd
                && coin->fFlipped == flipped) {
            return true;
        }
    } while ((coin = coin->fNext));
    return false;
}

// walk span sets in parallel, moving winding from one to the other
bool SkOpCoincidence::apply() {
    SkCoincidentSpans* coin = fHead;
    if (!coin) {
        return true;
    }
    do {
        SkOpSpan* start = coin->fCoinPtTStart->span()->upCast();
        if (start->deleted()) {
            continue;
        }
        SkOpSpanBase* end = coin->fCoinPtTEnd->span();
        SkASSERT(start == start->starter(end));
        bool flipped = coin->fFlipped;
        SkOpSpan* oStart = (flipped ? coin->fOppPtTEnd : coin->fOppPtTStart)->span()->upCast();
        if (oStart->deleted()) {
            continue;
        }
        SkOpSpanBase* oEnd = (flipped ? coin->fOppPtTStart : coin->fOppPtTEnd)->span();
        SkASSERT(oStart == oStart->starter(oEnd));
        SkOpSegment* segment = start->segment();
        SkOpSegment* oSegment = oStart->segment();
        bool operandSwap = segment->operand() != oSegment->operand();
        if (flipped) {
            if (oEnd->deleted()) {
                continue;
            }
            do {
                SkOpSpanBase* oNext = oStart->next();
                if (oNext == oEnd) {
                    break;
                }
                oStart = oNext->upCast();
            } while (true);
        }
        do {
            int windValue = start->windValue();
            int oppValue = start->oppValue();
            int oWindValue = oStart->windValue();
            int oOppValue = oStart->oppValue();
            // winding values are added or subtracted depending on direction and wind type
            // same or opposite values are summed depending on the operand value
            int windDiff = operandSwap ? oOppValue : oWindValue;
            int oWindDiff = operandSwap ? oppValue : windValue;
            if (!flipped) {
                windDiff = -windDiff;
                oWindDiff = -oWindDiff;
            }
            if (windValue && (windValue > windDiff || (windValue == windDiff
                    && oWindValue <= oWindDiff))) {
                if (operandSwap) {
                    SkTSwap(oWindValue, oOppValue);
                }
                if (flipped) {
                    windValue -= oWindValue;
                    oppValue -= oOppValue;
                } else {
                    windValue += oWindValue;
                    oppValue += oOppValue;
                }
                if (segment->isXor()) {
                    windValue &= 1;
                }
                if (segment->oppXor()) {
                    oppValue &= 1;
                }
                oWindValue = oOppValue = 0;
            } else {
                if (operandSwap) {
                    SkTSwap(windValue, oppValue);
                }
                if (flipped) {
                    oWindValue -= windValue;
                    oOppValue -= oppValue;
                } else {
                    oWindValue += windValue;
                    oOppValue += oppValue;
                }
                if (oSegment->isXor()) {
                    oWindValue &= 1;
                }
                if (oSegment->oppXor()) {
                    oOppValue &= 1;
                }
                windValue = oppValue = 0;
            }
            start->setWindValue(windValue);
            start->setOppValue(oppValue);
            oStart->setWindValue(oWindValue);
            oStart->setOppValue(oOppValue);
            if (!windValue && !oppValue) {
                segment->markDone(start);
            }
            if (!oWindValue && !oOppValue) {
                oSegment->markDone(oStart);
            }
            SkOpSpanBase* next = start->next();
            SkOpSpanBase* oNext = flipped ? oStart->prev() : oStart->next();
            if (next == end) {
                break;
            }
            if (!next->upCastable()) {
                return false;
            }
            start = next->upCast();
            // if the opposite ran out too soon, just reuse the last span
            if (!oNext || !oNext->upCastable()) {
               oNext = oStart;
            }
            oStart = oNext->upCast();
        } while (true);
    } while ((coin = coin->fNext));
    return true;
}

void SkOpCoincidence::release(SkCoincidentSpans* remove) {
    SkCoincidentSpans* coin = fHead;
    SkCoincidentSpans* prev = nullptr;
    SkCoincidentSpans* next;
    do {
        next = coin->fNext;
        if (coin == remove) {
            if (prev) {
                prev->fNext = next;
            } else {
                fHead = next;
            }
            break;
        }
        prev = coin;
    } while ((coin = next));
    SkASSERT(coin);
}

bool SkOpCoincidence::expand() {
    SkCoincidentSpans* coin = fHead;
    if (!coin) {
        return false;
    }
    bool expanded = false;
    do {
        SkOpSpan* start = coin->fCoinPtTStart->span()->upCast();
        SkOpSpanBase* end = coin->fCoinPtTEnd->span();
        SkOpSegment* segment = coin->fCoinPtTStart->segment();
        SkOpSegment* oppSegment = coin->fOppPtTStart->segment();
        SkOpSpan* prev = start->prev();
        SkOpPtT* oppPtT;
        if (prev && (oppPtT = prev->contains(oppSegment))) {
            double midT = (prev->t() + start->t()) / 2;
            if (segment->isClose(midT, oppSegment)) {
                coin->fCoinPtTStart = prev->ptT();
                coin->fOppPtTStart = oppPtT;
                expanded = true;
            }
        }
        SkOpSpanBase* next = end->final() ? nullptr : end->upCast()->next();
        if (next && (oppPtT = next->contains(oppSegment))) {
            double midT = (end->t() + next->t()) / 2;
            if (segment->isClose(midT, oppSegment)) {
                coin->fCoinPtTEnd = next->ptT();
                coin->fOppPtTEnd = oppPtT;
                expanded = true;
            }
        }
    } while ((coin = coin->fNext));
    return expanded;
}

void SkOpCoincidence::findOverlaps(SkOpCoincidence* overlaps, SkChunkAlloc* allocator) const {
    overlaps->fHead = overlaps->fTop = nullptr;
    SkDEBUGCODE_(overlaps->debugSetGlobalState(fDebugState));
    SkCoincidentSpans* outer = fHead;
    while (outer) {
        SkOpSegment* outerCoin = outer->fCoinPtTStart->segment();
        SkOpSegment* outerOpp = outer->fOppPtTStart->segment();
        SkCoincidentSpans* inner = outer;
        while ((inner = inner->fNext)) {
            SkOpSegment* innerCoin = inner->fCoinPtTStart->segment();
            if (outerCoin == innerCoin) {
                continue;  // both winners are the same segment, so there's no additional overlap
            }
            SkOpSegment* innerOpp = inner->fOppPtTStart->segment();
            SkOpPtT* overlapS, * overlapE;
            if ((outerOpp == innerCoin && SkOpPtT::Overlaps(outer->fOppPtTStart, outer->fOppPtTEnd,
                    inner->fCoinPtTStart, inner->fCoinPtTEnd, &overlapS, &overlapE))
                    || (outerCoin == innerOpp && SkOpPtT::Overlaps(outer->fCoinPtTStart,
                    outer->fCoinPtTEnd, inner->fOppPtTStart, inner->fOppPtTEnd,
                    &overlapS, &overlapE))
                    || (outerOpp == innerOpp && SkOpPtT::Overlaps(outer->fOppPtTStart,
                    outer->fOppPtTEnd, inner->fOppPtTStart, inner->fOppPtTEnd,
                    &overlapS, &overlapE))) {
                overlaps->addOverlap(outerCoin, outerOpp, innerCoin, innerOpp,
                        overlapS, overlapE, allocator);
            }
        }
        outer = outer->fNext;
    }
}

void SkOpCoincidence::fixAligned() {
    SkCoincidentSpans* coin = fHead;
    if (!coin) {
        return;
    }
    do {
        if (coin->fCoinPtTStart->deleted()) {
            coin->fCoinPtTStart = coin->fCoinPtTStart->doppelganger();
        }
        if (coin->fCoinPtTEnd->deleted()) {
            coin->fCoinPtTEnd = coin->fCoinPtTEnd->doppelganger();
        }
        if (coin->fOppPtTStart->deleted()) {
            coin->fOppPtTStart = coin->fOppPtTStart->doppelganger();
        }
        if (coin->fOppPtTEnd->deleted()) {
            coin->fOppPtTEnd = coin->fOppPtTEnd->doppelganger();
        }
    } while ((coin = coin->fNext));
    coin = fHead;
    SkCoincidentSpans** priorPtr = &fHead;
    do {
        if (coin->fCoinPtTStart->collapsed(coin->fCoinPtTEnd)
                || coin->fOppPtTStart->collapsed(coin->fOppPtTEnd)) {
            *priorPtr = coin->fNext;
            continue;
        }
        priorPtr = &coin->fNext;
    } while ((coin = coin->fNext));
}

void SkOpCoincidence::fixUp(SkOpPtT* deleted, SkOpPtT* kept) {
    SkCoincidentSpans* coin = fHead;
    if (!coin) {
        return;
    }
    do {
        if (coin->fCoinPtTStart == deleted) {
            if (coin->fCoinPtTEnd->span() == kept->span()) {
                this->release(coin);
                continue;
            }
            coin->fCoinPtTStart = kept;
        }
        if (coin->fCoinPtTEnd == deleted) {
            if (coin->fCoinPtTStart->span() == kept->span()) {
                this->release(coin);
                continue;
            }
            coin->fCoinPtTEnd = kept;
        }
        if (coin->fOppPtTStart == deleted) {
            if (coin->fOppPtTEnd->span() == kept->span()) {
                this->release(coin);
                continue;
            }
            coin->fOppPtTStart = kept;
        }
        if (coin->fOppPtTEnd == deleted) {
            if (coin->fOppPtTStart->span() == kept->span()) {
                this->release(coin);
                continue;
            }
            coin->fOppPtTEnd = kept;
        }
    } while ((coin = coin->fNext));
}

/* this sets up the coincidence links in the segments when the coincidence crosses multiple spans */
bool SkOpCoincidence::mark() {
    SkCoincidentSpans* coin = fHead;
    if (!coin) {
        return true;
    }
    do {
        SkOpSpanBase* end = coin->fCoinPtTEnd->span();
        if (end->deleted()) {
          return false;
        }
        SkOpSpanBase* oldEnd = end;
        SkOpSpan* start = coin->fCoinPtTStart->span()->starter(&end);
        SkOpSpanBase* oEnd = coin->fOppPtTEnd->span();
        if (oEnd->deleted()) {
          return false;
        }
        SkOpSpanBase* oOldEnd = oEnd;
        SkOpSpanBase* oStart = coin->fOppPtTStart->span()->starter(&oEnd);
        bool flipped = (end == oldEnd) != (oEnd == oOldEnd);
        if (flipped) {
            SkTSwap(oStart, oEnd);
        }
        SkOpSpanBase* next = start;
        SkOpSpanBase* oNext = oStart;
        do {
            next = next->upCast()->next();
            oNext = flipped ? oNext->prev() : oNext->upCast()->next();
            if (next == end || oNext == oEnd) {
                break;
            }
            if (!next->containsCoinEnd(oNext)) {
                next->insertCoinEnd(oNext);
            }
            SkOpSpan* nextSpan = next->upCast();
            SkOpSpan* oNextSpan = oNext->upCast();
            if (!nextSpan->containsCoincidence(oNextSpan)) {
                nextSpan->insertCoincidence(oNextSpan);
            }
        } while (true);
    } while ((coin = coin->fNext));
    return true;
}

bool SkOpCoincidence::overlap(const SkOpPtT* coin1s, const SkOpPtT* coin1e, 
        const SkOpPtT* coin2s, const SkOpPtT* coin2e, double* overS, double* overE) const {
    SkASSERT(coin1s->segment() == coin2s->segment());
    *overS = SkTMax(SkTMin(coin1s->fT, coin1e->fT), SkTMin(coin2s->fT, coin2e->fT));
    *overE = SkTMin(SkTMax(coin1s->fT, coin1e->fT), SkTMax(coin2s->fT, coin2e->fT));
    return *overS < *overE;
}

bool SkOpCoincidence::testForCoincidence(const SkCoincidentSpans* outer, const SkOpPtT* testS,
        const SkOpPtT* testE) const {
    return testS->segment()->testForCoincidence(testS, testE, testS->span(),
            testE->span(), outer->fCoinPtTStart->segment(), 120000);  // FIXME: replace with tuned
}
