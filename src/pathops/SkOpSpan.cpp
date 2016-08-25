/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkOpCoincidence.h"
#include "SkOpContour.h"
#include "SkOpSegment.h"
#include "SkPathWriter.h"

bool SkOpPtT::alias() const {
    return this->span()->ptT() != this;
}

bool SkOpPtT::collapsed(const SkOpPtT* check) const {
    if (fPt != check->fPt) {
        return false;
    }
    SkASSERT(this != check);
    const SkOpSegment* segment = this->segment();
    SkASSERT(segment == check->segment());
    return segment->collapsed();
}

bool SkOpPtT::contains(const SkOpPtT* check) const {
    SkASSERT(this != check);
    const SkOpPtT* ptT = this;
    const SkOpPtT* stopPtT = ptT;
    while ((ptT = ptT->next()) != stopPtT) {
        if (ptT == check) {
            return true;
        }
    }
    return false;
}

bool SkOpPtT::contains(const SkOpSegment* segment, const SkPoint& pt) const {
    SkASSERT(this->segment() != segment);
    const SkOpPtT* ptT = this;
    const SkOpPtT* stopPtT = ptT;
    while ((ptT = ptT->next()) != stopPtT) {
        if (ptT->fPt == pt && ptT->segment() == segment) {
            return true;
        }
    }
    return false;
}

bool SkOpPtT::contains(const SkOpSegment* segment, double t) const {
    const SkOpPtT* ptT = this;
    const SkOpPtT* stopPtT = ptT;
    while ((ptT = ptT->next()) != stopPtT) {
        if (ptT->fT == t && ptT->segment() == segment) {
            return true;
        }
    }
    return false;
}

const SkOpPtT* SkOpPtT::contains(const SkOpSegment* check) const {
    SkASSERT(this->segment() != check);
    const SkOpPtT* ptT = this;
    const SkOpPtT* stopPtT = ptT;
    while ((ptT = ptT->next()) != stopPtT) {
        if (ptT->segment() == check && !ptT->deleted()) {
            return ptT;
        }
    }
    return nullptr;
}

SkOpContour* SkOpPtT::contour() const {
    return segment()->contour();
}

const SkOpPtT* SkOpPtT::find(const SkOpSegment* segment) const {
    const SkOpPtT* ptT = this;
    const SkOpPtT* stopPtT = ptT;
    do {
        if (ptT->segment() == segment && !ptT->deleted()) {
            return ptT;
        }
        ptT = ptT->fNext;
    } while (stopPtT != ptT);
//    SkASSERT(0);
    return nullptr;
}

SkOpGlobalState* SkOpPtT::globalState() const {
    return contour()->globalState();
}

void SkOpPtT::init(SkOpSpanBase* span, double t, const SkPoint& pt, bool duplicate) {
    fT = t;
    fPt = pt;
    fSpan = span;
    fNext = this;
    fDuplicatePt = duplicate;
    fDeleted = false;
    fCoincident = false;
    SkDEBUGCODE(fID = span->globalState()->nextPtTID());
}

bool SkOpPtT::onEnd() const {
    const SkOpSpanBase* span = this->span();
    if (span->ptT() != this) {
        return false;
    }
    const SkOpSegment* segment = this->segment();
    return span == segment->head() || span == segment->tail();
}

bool SkOpPtT::ptAlreadySeen(const SkOpPtT* check) const {
    while (this != check) {
        if (this->fPt == check->fPt) {
            return true;
        }
        check = check->fNext;
    }
    return false;
}

SkOpPtT* SkOpPtT::prev() {
    SkOpPtT* result = this;
    SkOpPtT* next = this;
    while ((next = next->fNext) != this) {
        result = next;
    }
    SkASSERT(result->fNext == this);
    return result;
}

SkOpPtT* SkOpPtT::remove(const SkOpPtT* kept) {
    SkOpPtT* prev = this;
    do {
        SkOpPtT* next = prev->fNext;
        if (next == this) {
            prev->removeNext(kept);
            SkASSERT(prev->fNext != prev);
            fDeleted = true;
            return prev;
        }
        prev = next;
    } while (prev != this);
    SkASSERT(0);
    return nullptr;
}

void SkOpPtT::removeNext(const SkOpPtT* kept) {
    SkASSERT(this->fNext);
    SkOpPtT* next = this->fNext;
    SkASSERT(this != next->fNext);
    this->fNext = next->fNext;
    SkOpSpanBase* span = next->span();
    SkOpCoincidence* coincidence = span->globalState()->coincidence();
    if (coincidence) {
        coincidence->fixUp(next, kept);
    }
    next->setDeleted();
    if (span->ptT() == next) {
        span->upCast()->release(kept);
    }
}

const SkOpSegment* SkOpPtT::segment() const {
    return span()->segment();
}

SkOpSegment* SkOpPtT::segment() {
    return span()->segment();
}

void SkOpPtT::setDeleted() {
    SkASSERT(this->span()->debugDeleted() || this->span()->ptT() != this);
    SkOPASSERT(!fDeleted);
    fDeleted = true;
}

// please keep this in sync with debugAddOppAndMerge
// If the added points envelop adjacent spans, merge them in.
void SkOpSpanBase::addOppAndMerge(SkOpSpanBase* opp) {
    SkOpPtT* oppPrev = this->ptT()->oppPrev(opp->ptT());
    if (oppPrev) {
        this->ptT()->addOpp(opp->ptT(), oppPrev);
        this->checkForCollapsedCoincidence();
    }
    // compute bounds of points in span
    SkPathOpsBounds bounds;
    bounds.set(SK_ScalarMax, SK_ScalarMax, SK_ScalarMin, SK_ScalarMin);
    const SkOpPtT* head = this->ptT();
    const SkOpPtT* nextPt = head;
    do {
        bounds.add(nextPt->fPt);
    } while ((nextPt = nextPt->next()) != head);
    if (!bounds.width() && !bounds.height()) {
        return;
    }
    this->mergeContained(bounds);
    opp->mergeContained(bounds);
}

// Please keep this in sync with debugMergeContained()
void SkOpSpanBase::mergeContained(const SkPathOpsBounds& bounds) {
    // while adjacent spans' points are contained by the bounds, merge them
    SkOpSpanBase* prev = this;
    SkOpSegment* seg = this->segment();
    while ((prev = prev->prev()) && bounds.contains(prev->pt()) && !seg->ptsDisjoint(prev, this)) {
        if (prev->prev()) {
            this->merge(prev->upCast());
            prev = this;
        } else if (this->final()) {
            seg->clearAll();
            return;
        } else {
            prev->merge(this->upCast());
        }
    }
    SkOpSpanBase* current = this;
    SkOpSpanBase* next = this;
    while (next->upCastable() && (next = next->upCast()->next())
            && bounds.contains(next->pt()) && !seg->ptsDisjoint(this, next)) {
        if (!current->prev() && next->final()) {
            seg->clearAll();
            return;
        }
        if (current->prev()) {
            next->merge(current->upCast());
            current = next;
        } else {
            current->merge(next->upCast());
            // extra line in debug version
        }
    }
#if DEBUG_COINCIDENCE
    this->globalState()->coincidence()->debugValidate();
#endif
}

bool SkOpSpanBase::contains(const SkOpSpanBase* span) const {
    const SkOpPtT* start = &fPtT;
    const SkOpPtT* check = &span->fPtT;
    SkOPASSERT(start != check);
    const SkOpPtT* walk = start;
    while ((walk = walk->next()) != start) {
        if (walk == check) {
            return true;
        }
    }
    return false;
}

const SkOpPtT* SkOpSpanBase::contains(const SkOpSegment* segment) const {
    const SkOpPtT* start = &fPtT;
    const SkOpPtT* walk = start;
    while ((walk = walk->next()) != start) {
        if (walk->deleted()) {
            continue;
        }
        if (walk->segment() == segment && walk->span()->ptT() == walk) {
            return walk;
        }
    }
    return nullptr;
}

bool SkOpSpanBase::containsCoinEnd(const SkOpSegment* segment) const {
    SkASSERT(this->segment() != segment);
    const SkOpSpanBase* next = this;
    while ((next = next->fCoinEnd) != this) {
        if (next->segment() == segment) {
            return true;
        }
    }
    return false;
}

SkOpContour* SkOpSpanBase::contour() const {
    return segment()->contour();
}

SkOpGlobalState* SkOpSpanBase::globalState() const {
    return contour()->globalState();
}

void SkOpSpanBase::initBase(SkOpSegment* segment, SkOpSpan* prev, double t, const SkPoint& pt) {
    fSegment = segment;
    fPtT.init(this, t, pt, false);
    fCoinEnd = this;
    fFromAngle = nullptr;
    fPrev = prev;
    fSpanAdds = 0;
    fAligned = true;
    fChased = false;
    SkDEBUGCODE(fCount = 1);
    SkDEBUGCODE(fID = globalState()->nextSpanID());
    SkDEBUGCODE(fDeleted = false);
}

// this pair of spans share a common t value or point; merge them and eliminate duplicates
// this does not compute the best t or pt value; this merely moves all data into a single list
void SkOpSpanBase::merge(SkOpSpan* span) {
    SkOpPtT* spanPtT = span->ptT();
    SkASSERT(this->t() != spanPtT->fT);
    SkASSERT(!zero_or_one(spanPtT->fT));
    span->release(this->ptT());
    if (this->contains(span)) {
        return;  // merge is already in the ptT loop
    }
    SkOpPtT* remainder = spanPtT->next();
    this->ptT()->insert(spanPtT);
    while (remainder != spanPtT) {
        SkOpPtT* next = remainder->next();
        SkOpPtT* compare = spanPtT->next();
        while (compare != spanPtT) {
            SkOpPtT* nextC = compare->next();
            if (nextC->span() == remainder->span() && nextC->fT == remainder->fT) {
                goto tryNextRemainder;
            }
            compare = nextC;
        }
        spanPtT->insert(remainder);
tryNextRemainder:
        remainder = next;
    }
    fSpanAdds += span->fSpanAdds;
    this->checkForCollapsedCoincidence();
}

// please keep in sync with debugCheckForCollapsedCoincidence()
void SkOpSpanBase::checkForCollapsedCoincidence() {
    SkOpCoincidence* coins = this->globalState()->coincidence();
    if (coins->isEmpty()) {
        return;
    }
// the insert above may have put both ends of a coincident run in the same span
// for each coincident ptT in loop; see if its opposite in is also in the loop
// this implementation is the motivation for marking that a ptT is referenced by a coincident span
    SkOpPtT* head = this->ptT();
    SkOpPtT* test = head;
    do {
        if (!test->coincident()) {
            continue;
        }
        coins->markCollapsed(test);
    } while ((test = test->next()) != head);
}

int SkOpSpan::computeWindSum() {
    SkOpGlobalState* globals = this->globalState();
    SkOpContour* contourHead = globals->contourHead();
    int windTry = 0;
    while (!this->sortableTop(contourHead) && ++windTry < SkOpGlobalState::kMaxWindingTries) {
        ;
    }
    return this->windSum();
}

bool SkOpSpan::containsCoincidence(const SkOpSegment* segment) const {
    SkASSERT(this->segment() != segment);
    const SkOpSpan* next = fCoincident;
    do {
        if (next->segment() == segment) {
            return true;
        }
    } while ((next = next->fCoincident) != this);
    return false;
}

void SkOpSpan::init(SkOpSegment* segment, SkOpSpan* prev, double t, const SkPoint& pt) {
    SkASSERT(t != 1);
    initBase(segment, prev, t, pt);
    fCoincident = this;
    fToAngle = nullptr;
    fWindSum = fOppSum = SK_MinS32;
    fWindValue = 1;
    fOppValue = 0;
    fTopTTry = 0;
    fChased = fDone = false;
    segment->bumpCount();
    fAlreadyAdded = false;
}

// Please keep this in sync with debugInsertCoincidence()
bool SkOpSpan::insertCoincidence(const SkOpSegment* segment, bool flipped) {
    if (this->containsCoincidence(segment)) {
        return true;
    }
    SkOpPtT* next = &fPtT;
    while ((next = next->next()) != &fPtT) {
        if (next->segment() == segment) {
            SkOpSpan* span;
            if (flipped) {
                span = next->span()->prev();
                if (!span) {
                    return false;
                }
            } else {
                SkOpSpanBase* base = next->span();
                if (!base->upCastable()) {
                    return false;
                }
                span = base->upCast();
            }
            this->insertCoincidence(span);
            return true;
        }
    }
#if DEBUG_COINCIDENCE
    SkASSERT(0); // FIXME? if we get here, the span is missing its opposite segment...
#endif
    return true;
}

void SkOpSpan::release(const SkOpPtT* kept) {
    SkDEBUGCODE(fDeleted = true);
    SkASSERT(kept->span() != this);
    SkASSERT(!final());
    SkOpSpan* prev = this->prev();
    SkASSERT(prev);
    SkOpSpanBase* next = this->next();
    SkASSERT(next);
    prev->setNext(next);
    next->setPrev(prev);
    this->segment()->release(this);
    SkOpCoincidence* coincidence = this->globalState()->coincidence();
    if (coincidence) {
        coincidence->fixUp(this->ptT(), kept);
    }
    this->ptT()->setDeleted();
    SkOpPtT* stopPtT = this->ptT();
    SkOpPtT* testPtT = stopPtT;
    const SkOpSpanBase* keptSpan = kept->span();
    do {
        if (this == testPtT->span()) {
            testPtT->setSpan(keptSpan);
        }
    } while ((testPtT = testPtT->next()) != stopPtT);
}

void SkOpSpan::setOppSum(int oppSum) {
    SkASSERT(!final());
    if (fOppSum != SK_MinS32 && fOppSum != oppSum) {
        this->globalState()->setWindingFailed();
        return;
    }
    SkASSERT(!DEBUG_LIMIT_WIND_SUM || SkTAbs(oppSum) <= DEBUG_LIMIT_WIND_SUM);
    fOppSum = oppSum;
}

void SkOpSpan::setWindSum(int windSum) {
    SkASSERT(!final());
    if (fWindSum != SK_MinS32 && fWindSum != windSum) {
        this->globalState()->setWindingFailed();
        return;
    }
    SkASSERT(!DEBUG_LIMIT_WIND_SUM || SkTAbs(windSum) <= DEBUG_LIMIT_WIND_SUM);
    fWindSum = windSum;
}
