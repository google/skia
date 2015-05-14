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
    SkCoincidentSpans* check = this->fHead;
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
            return true;
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
    if (cs == ce) {
        return false;
    }
    SkOpPtT* os = oppSeg->addMissing(oppTs, coinSeg, allocator);
    SkOpPtT* oe = oppSeg->addMissing(oppTe, coinSeg, allocator);
    SkASSERT(os != oe);
    cs->addOpp(os);
    ce->addOpp(oe);
    this->add(cs, ce, os, oe, allocator);
    return true;
}

bool SkOpCoincidence::addMissing(SkChunkAlloc* allocator) {
    SkCoincidentSpans* outer = this->fHead;
    if (!outer) {
        return true;
    }
    do {
        SkCoincidentSpans* inner = outer;
        while ((inner = inner->fNext)) {
            double overS, overE;
            if (this->overlap(outer->fCoinPtTStart, outer->fCoinPtTEnd,
                    inner->fCoinPtTStart, inner->fCoinPtTEnd, &overS, &overE)) {
                if (!this->addIfMissing(outer->fCoinPtTStart, outer->fCoinPtTEnd,
                        inner->fCoinPtTStart, inner->fCoinPtTEnd, overS, overE,
                        outer->fOppPtTStart, outer->fOppPtTEnd,
                        inner->fOppPtTStart, inner->fOppPtTEnd, allocator)) {
                    return false;
                }
            } else if (this->overlap(outer->fCoinPtTStart, outer->fCoinPtTEnd,
                    inner->fOppPtTStart, inner->fOppPtTEnd, &overS, &overE)) {
                if (!this->addIfMissing(outer->fCoinPtTStart, outer->fCoinPtTEnd,
                        inner->fOppPtTStart, inner->fOppPtTEnd, overS, overE,
                        outer->fOppPtTStart, outer->fOppPtTEnd,
                        inner->fCoinPtTStart, inner->fCoinPtTEnd, allocator)) {
                    return false;
                }
            } else if (this->overlap(outer->fOppPtTStart, outer->fOppPtTEnd,
                    inner->fCoinPtTStart, inner->fCoinPtTEnd, &overS, &overE)) {
                if (!this->addIfMissing(outer->fOppPtTStart, outer->fOppPtTEnd,
                        inner->fCoinPtTStart, inner->fCoinPtTEnd, overS, overE,
                        outer->fCoinPtTStart, outer->fCoinPtTEnd,
                        inner->fOppPtTStart, inner->fOppPtTEnd, allocator)) {
                    return false;
                }
            } else if (this->overlap(outer->fOppPtTStart, outer->fOppPtTEnd,
                    inner->fOppPtTStart, inner->fOppPtTEnd, &overS, &overE)) {
                if (!this->addIfMissing(outer->fOppPtTStart, outer->fOppPtTEnd,
                        inner->fOppPtTStart, inner->fOppPtTEnd, overS, overE,
                        outer->fCoinPtTStart, outer->fCoinPtTEnd,
                        inner->fCoinPtTStart, inner->fCoinPtTEnd, allocator)) {
                    return false;
                }
            }
        }

    } while ((outer = outer->fNext));
    return true;
}

bool SkOpCoincidence::contains(SkOpPtT* coinPtTStart, SkOpPtT* coinPtTEnd, SkOpPtT* oppPtTStart,
        SkOpPtT* oppPtTEnd, bool flipped) {
    SkCoincidentSpans* coin = fHead;
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

void SkOpCoincidence::detach(SkCoincidentSpans* remove) {
    SkCoincidentSpans* coin = fHead;
    SkCoincidentSpans* prev = NULL;
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

void SkOpCoincidence::expand() {
    SkCoincidentSpans* coin = fHead;
    if (!coin) {
        return;
    }
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
            }
        }
        SkOpSpanBase* next = end->final() ? NULL : end->upCast()->next();
        if (next && (oppPtT = next->contains(oppSegment))) {
            double midT = (end->t() + next->t()) / 2;
            if (segment->isClose(midT, oppSegment)) {
                coin->fCoinPtTEnd = next->ptT();
                coin->fOppPtTEnd = oppPtT;
            }
        }
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
                return this->detach(coin);
            }
            coin->fCoinPtTStart = kept;
        }
        if (coin->fCoinPtTEnd == deleted) {
            if (coin->fCoinPtTStart->span() == kept->span()) {
                return this->detach(coin);
            }
            coin->fCoinPtTEnd = kept;
        }
        if (coin->fOppPtTStart == deleted) {
            if (coin->fOppPtTEnd->span() == kept->span()) {
                return this->detach(coin);
            }
            coin->fOppPtTStart = kept;
        }
        if (coin->fOppPtTEnd == deleted) {
            if (coin->fOppPtTStart->span() == kept->span()) {
                return this->detach(coin);
            }
            coin->fOppPtTEnd = kept;
        }
    } while ((coin = coin->fNext));
}

void SkOpCoincidence::mark() {
    SkCoincidentSpans* coin = fHead;
    if (!coin) {
        return;
    }
    do {
        SkOpSpanBase* end = coin->fCoinPtTEnd->span();
        SkOpSpanBase* oldEnd = end;
        SkOpSpan* start = coin->fCoinPtTStart->span()->starter(&end);
        SkOpSpanBase* oEnd = coin->fOppPtTEnd->span();
        SkOpSpanBase* oOldEnd = oEnd;
        SkOpSpanBase* oStart = coin->fOppPtTStart->span()->starter(&oEnd);
        bool flipped = (end == oldEnd) != (oEnd == oOldEnd);
        if (flipped) {
            SkTSwap(oStart, oEnd);
        }
        SkOpSpanBase* next = start;
        SkOpSpanBase* oNext = oStart;
        // check to see if coincident span could be bigger

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
}

bool SkOpCoincidence::overlap(const SkOpPtT* coin1s, const SkOpPtT* coin1e, 
        const SkOpPtT* coin2s, const SkOpPtT* coin2e, double* overS, double* overE) const {
    if (coin1s->segment() != coin2s->segment()) {
        return false;
    }
    *overS = SkTMax(SkTMin(coin1s->fT, coin1e->fT), SkTMin(coin2s->fT, coin2e->fT));
    *overE = SkTMin(SkTMax(coin1s->fT, coin1e->fT), SkTMax(coin2s->fT, coin2e->fT));
    return *overS < *overE;
}
