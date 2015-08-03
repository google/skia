/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBuffer.h"
#include "SkLazyPtr.h"
#include "SkPath.h"
#include "SkPathRef.h"

//////////////////////////////////////////////////////////////////////////////
SkPathRef::Editor::Editor(SkAutoTUnref<SkPathRef>* pathRef,
                          int incReserveVerbs,
                          int incReservePoints)
{
    if ((*pathRef)->unique()) {
        (*pathRef)->incReserve(incReserveVerbs, incReservePoints);
    } else {
        SkPathRef* copy = SkNEW(SkPathRef);
        copy->copy(**pathRef, incReserveVerbs, incReservePoints);
        pathRef->reset(copy);
    }
    fPathRef = *pathRef;
    fPathRef->callGenIDChangeListeners();
    fPathRef->fGenerationID = 0;
    SkDEBUGCODE(sk_atomic_inc(&fPathRef->fEditorsAttached);)
}

//////////////////////////////////////////////////////////////////////////////

SkPathRef::~SkPathRef() {
    this->callGenIDChangeListeners();
    SkDEBUGCODE(this->validate();)
    sk_free(fPoints);

    SkDEBUGCODE(fPoints = NULL;)
    SkDEBUGCODE(fVerbs = NULL;)
    SkDEBUGCODE(fVerbCnt = 0x9999999;)
    SkDEBUGCODE(fPointCnt = 0xAAAAAAA;)
    SkDEBUGCODE(fPointCnt = 0xBBBBBBB;)
    SkDEBUGCODE(fGenerationID = 0xEEEEEEEE;)
    SkDEBUGCODE(fEditorsAttached = 0x7777777;)
}

// As a template argument, this must have external linkage.
SkPathRef* sk_create_empty_pathref() {
    SkPathRef* empty = SkNEW(SkPathRef);
    empty->computeBounds();   // Avoids races later to be the first to do this.
    return empty;
}

SK_DECLARE_STATIC_LAZY_PTR(SkPathRef, empty, sk_create_empty_pathref);

SkPathRef* SkPathRef::CreateEmpty() {
    return SkRef(empty.get());
}

void SkPathRef::CreateTransformedCopy(SkAutoTUnref<SkPathRef>* dst,
                                      const SkPathRef& src,
                                      const SkMatrix& matrix) {
    SkDEBUGCODE(src.validate();)
    if (matrix.isIdentity()) {
        if (*dst != &src) {
            src.ref();
            dst->reset(const_cast<SkPathRef*>(&src));
            SkDEBUGCODE((*dst)->validate();)
        }
        return;
    }

    if (!(*dst)->unique()) {
        dst->reset(SkNEW(SkPathRef));
    }

    if (*dst != &src) {
        (*dst)->resetToSize(src.fVerbCnt, src.fPointCnt, src.fConicWeights.count());
        memcpy((*dst)->verbsMemWritable(), src.verbsMemBegin(), src.fVerbCnt * sizeof(uint8_t));
        (*dst)->fConicWeights = src.fConicWeights;
    }

    SkASSERT((*dst)->countPoints() == src.countPoints());
    SkASSERT((*dst)->countVerbs() == src.countVerbs());
    SkASSERT((*dst)->fConicWeights.count() == src.fConicWeights.count());

    // Need to check this here in case (&src == dst)
    bool canXformBounds = !src.fBoundsIsDirty && matrix.rectStaysRect() && src.countPoints() > 1;

    matrix.mapPoints((*dst)->fPoints, src.points(), src.fPointCnt);

    /*
        *  Here we optimize the bounds computation, by noting if the bounds are
        *  already known, and if so, we just transform those as well and mark
        *  them as "known", rather than force the transformed path to have to
        *  recompute them.
        *
        *  Special gotchas if the path is effectively empty (<= 1 point) or
        *  if it is non-finite. In those cases bounds need to stay empty,
        *  regardless of the matrix.
        */
    if (canXformBounds) {
        (*dst)->fBoundsIsDirty = false;
        if (src.fIsFinite) {
            matrix.mapRect(&(*dst)->fBounds, src.fBounds);
            if (!((*dst)->fIsFinite = (*dst)->fBounds.isFinite())) {
                (*dst)->fBounds.setEmpty();
            }
        } else {
            (*dst)->fIsFinite = false;
            (*dst)->fBounds.setEmpty();
        }
    } else {
        (*dst)->fBoundsIsDirty = true;
    }

    (*dst)->fSegmentMask = src.fSegmentMask;

    // It's an oval only if it stays a rect.
    (*dst)->fIsOval = src.fIsOval && matrix.rectStaysRect();

    SkDEBUGCODE((*dst)->validate();)
}

SkPathRef* SkPathRef::CreateFromBuffer(SkRBuffer* buffer) {
    SkPathRef* ref = SkNEW(SkPathRef);
    bool isOval;
    uint8_t segmentMask;

    int32_t packed;
    if (!buffer->readS32(&packed)) {
        SkDELETE(ref);
        return NULL;
    }

    ref->fIsFinite = (packed >> kIsFinite_SerializationShift) & 1;
    segmentMask = (packed >> kSegmentMask_SerializationShift) & 0xF;
    isOval  = (packed >> kIsOval_SerializationShift) & 1;

    int32_t verbCount, pointCount, conicCount;
    if (!buffer->readU32(&(ref->fGenerationID)) ||
        !buffer->readS32(&verbCount) ||
        !buffer->readS32(&pointCount) ||
        !buffer->readS32(&conicCount)) {
        SkDELETE(ref);
        return NULL;
    }

    ref->resetToSize(verbCount, pointCount, conicCount);
    SkASSERT(verbCount == ref->countVerbs());
    SkASSERT(pointCount == ref->countPoints());
    SkASSERT(conicCount == ref->fConicWeights.count());

    if (!buffer->read(ref->verbsMemWritable(), verbCount * sizeof(uint8_t)) ||
        !buffer->read(ref->fPoints, pointCount * sizeof(SkPoint)) ||
        !buffer->read(ref->fConicWeights.begin(), conicCount * sizeof(SkScalar)) ||
        !buffer->read(&ref->fBounds, sizeof(SkRect))) {
        SkDELETE(ref);
        return NULL;
    }
    ref->fBoundsIsDirty = false;

    // resetToSize clears fSegmentMask and fIsOval
    ref->fSegmentMask = segmentMask;
    ref->fIsOval = isOval;
    return ref;
}

void SkPathRef::Rewind(SkAutoTUnref<SkPathRef>* pathRef) {
    if ((*pathRef)->unique()) {
        SkDEBUGCODE((*pathRef)->validate();)
        (*pathRef)->callGenIDChangeListeners();
        (*pathRef)->fBoundsIsDirty = true;  // this also invalidates fIsFinite
        (*pathRef)->fVerbCnt = 0;
        (*pathRef)->fPointCnt = 0;
        (*pathRef)->fFreeSpace = (*pathRef)->currSize();
        (*pathRef)->fGenerationID = 0;
        (*pathRef)->fConicWeights.rewind();
        (*pathRef)->fSegmentMask = 0;
        (*pathRef)->fIsOval = false;
        SkDEBUGCODE((*pathRef)->validate();)
    } else {
        int oldVCnt = (*pathRef)->countVerbs();
        int oldPCnt = (*pathRef)->countPoints();
        pathRef->reset(SkNEW(SkPathRef));
        (*pathRef)->resetToSize(0, 0, 0, oldVCnt, oldPCnt);
    }
}

bool SkPathRef::operator== (const SkPathRef& ref) const {
    SkDEBUGCODE(this->validate();)
    SkDEBUGCODE(ref.validate();)

    // We explicitly check fSegmentMask as a quick-reject. We could skip it,
    // since it is only a cache of info in the fVerbs, but its a fast way to
    // notice a difference
    if (fSegmentMask != ref.fSegmentMask) {
        return false;
    }

    bool genIDMatch = fGenerationID && fGenerationID == ref.fGenerationID;
#ifdef SK_RELEASE
    if (genIDMatch) {
        return true;
    }
#endif
    if (fPointCnt != ref.fPointCnt ||
        fVerbCnt != ref.fVerbCnt) {
        SkASSERT(!genIDMatch);
        return false;
    }
    if (0 == ref.fVerbCnt) {
        SkASSERT(0 == ref.fPointCnt);
        return true;
    }
    SkASSERT(this->verbsMemBegin() && ref.verbsMemBegin());
    if (0 != memcmp(this->verbsMemBegin(),
                    ref.verbsMemBegin(),
                    ref.fVerbCnt * sizeof(uint8_t))) {
        SkASSERT(!genIDMatch);
        return false;
    }
    SkASSERT(this->points() && ref.points());
    if (0 != memcmp(this->points(),
                    ref.points(),
                    ref.fPointCnt * sizeof(SkPoint))) {
        SkASSERT(!genIDMatch);
        return false;
    }
    if (fConicWeights != ref.fConicWeights) {
        SkASSERT(!genIDMatch);
        return false;
    }
    return true;
}

void SkPathRef::writeToBuffer(SkWBuffer* buffer) const {
    SkDEBUGCODE(this->validate();)
    SkDEBUGCODE(size_t beforePos = buffer->pos();)

    // Call getBounds() to ensure (as a side-effect) that fBounds
    // and fIsFinite are computed.
    const SkRect& bounds = this->getBounds();

    int32_t packed = ((fIsFinite & 1) << kIsFinite_SerializationShift) |
                     ((fIsOval & 1) << kIsOval_SerializationShift) |
                     (fSegmentMask << kSegmentMask_SerializationShift);
    buffer->write32(packed);

    // TODO: write gen ID here. Problem: We don't know if we're cross process or not from
    // SkWBuffer. Until this is fixed we write 0.
    buffer->write32(0);
    buffer->write32(fVerbCnt);
    buffer->write32(fPointCnt);
    buffer->write32(fConicWeights.count());
    buffer->write(verbsMemBegin(), fVerbCnt * sizeof(uint8_t));
    buffer->write(fPoints, fPointCnt * sizeof(SkPoint));
    buffer->write(fConicWeights.begin(), fConicWeights.bytes());
    buffer->write(&bounds, sizeof(bounds));

    SkASSERT(buffer->pos() - beforePos == (size_t) this->writeSize());
}

uint32_t SkPathRef::writeSize() const {
    return uint32_t(5 * sizeof(uint32_t) +
                    fVerbCnt * sizeof(uint8_t) +
                    fPointCnt * sizeof(SkPoint) +
                    fConicWeights.bytes() +
                    sizeof(SkRect));
}

void SkPathRef::copy(const SkPathRef& ref,
                     int additionalReserveVerbs,
                     int additionalReservePoints) {
    SkDEBUGCODE(this->validate();)
    this->resetToSize(ref.fVerbCnt, ref.fPointCnt, ref.fConicWeights.count(),
                        additionalReserveVerbs, additionalReservePoints);
    memcpy(this->verbsMemWritable(), ref.verbsMemBegin(), ref.fVerbCnt * sizeof(uint8_t));
    memcpy(this->fPoints, ref.fPoints, ref.fPointCnt * sizeof(SkPoint));
    fConicWeights = ref.fConicWeights;
    fBoundsIsDirty = ref.fBoundsIsDirty;
    if (!fBoundsIsDirty) {
        fBounds = ref.fBounds;
        fIsFinite = ref.fIsFinite;
    }
    fSegmentMask = ref.fSegmentMask;
    fIsOval = ref.fIsOval;
    SkDEBUGCODE(this->validate();)
}

SkPoint* SkPathRef::growForRepeatedVerb(int /*SkPath::Verb*/ verb,
                                        int numVbs,
                                        SkScalar** weights) {
    // This value is just made-up for now. When count is 4, calling memset was much
    // slower than just writing the loop. This seems odd, and hopefully in the
    // future this will appear to have been a fluke...
    static const unsigned int kMIN_COUNT_FOR_MEMSET_TO_BE_FAST = 16;

    SkDEBUGCODE(this->validate();)
    int pCnt;
    bool dirtyAfterEdit = true;
    switch (verb) {
        case SkPath::kMove_Verb:
            pCnt = numVbs;
            dirtyAfterEdit = false;
            break;
        case SkPath::kLine_Verb:
            fSegmentMask |= SkPath::kLine_SegmentMask;
            pCnt = numVbs;
            break;
        case SkPath::kQuad_Verb:
            fSegmentMask |= SkPath::kQuad_SegmentMask;
            pCnt = 2 * numVbs;
            break;
        case SkPath::kConic_Verb:
            fSegmentMask |= SkPath::kConic_SegmentMask;
            pCnt = 2 * numVbs;
            break;
        case SkPath::kCubic_Verb:
            fSegmentMask |= SkPath::kCubic_SegmentMask;
            pCnt = 3 * numVbs;
            break;
        case SkPath::kClose_Verb:
            SkDEBUGFAIL("growForRepeatedVerb called for kClose_Verb");
            pCnt = 0;
            dirtyAfterEdit = false;
            break;
        case SkPath::kDone_Verb:
            SkDEBUGFAIL("growForRepeatedVerb called for kDone");
            // fall through
        default:
            SkDEBUGFAIL("default should not be reached");
            pCnt = 0;
            dirtyAfterEdit = false;
    }

    size_t space = numVbs * sizeof(uint8_t) + pCnt * sizeof (SkPoint);
    this->makeSpace(space);

    SkPoint* ret = fPoints + fPointCnt;
    uint8_t* vb = fVerbs - fVerbCnt;

    // cast to unsigned, so if kMIN_COUNT_FOR_MEMSET_TO_BE_FAST is defined to
    // be 0, the compiler will remove the test/branch entirely.
    if ((unsigned)numVbs >= kMIN_COUNT_FOR_MEMSET_TO_BE_FAST) {
        memset(vb - numVbs, verb, numVbs);
    } else {
        for (int i = 0; i < numVbs; ++i) {
            vb[~i] = verb;
        }
    }

    fVerbCnt += numVbs;
    fPointCnt += pCnt;
    fFreeSpace -= space;
    fBoundsIsDirty = true;  // this also invalidates fIsFinite
    if (dirtyAfterEdit) {
        fIsOval = false;
    }

    if (SkPath::kConic_Verb == verb) {
        SkASSERT(weights);
        *weights = fConicWeights.append(numVbs);
    }

    SkDEBUGCODE(this->validate();)
    return ret;
}

SkPoint* SkPathRef::growForVerb(int /* SkPath::Verb*/ verb, SkScalar weight) {
    SkDEBUGCODE(this->validate();)
    int pCnt;
    bool dirtyAfterEdit = true;
    switch (verb) {
        case SkPath::kMove_Verb:
            pCnt = 1;
            dirtyAfterEdit = false;
            break;
        case SkPath::kLine_Verb:
            fSegmentMask |= SkPath::kLine_SegmentMask;
            pCnt = 1;
            break;
        case SkPath::kQuad_Verb:
            fSegmentMask |= SkPath::kQuad_SegmentMask;
            pCnt = 2;
            break;
        case SkPath::kConic_Verb:
            fSegmentMask |= SkPath::kConic_SegmentMask;
            pCnt = 2;
            break;
        case SkPath::kCubic_Verb:
            fSegmentMask |= SkPath::kCubic_SegmentMask;
            pCnt = 3;
            break;
        case SkPath::kClose_Verb:
            pCnt = 0;
            dirtyAfterEdit = false;
            break;
        case SkPath::kDone_Verb:
            SkDEBUGFAIL("growForVerb called for kDone");
            // fall through
        default:
            SkDEBUGFAIL("default is not reached");
            dirtyAfterEdit = false;
            pCnt = 0;
    }
    size_t space = sizeof(uint8_t) + pCnt * sizeof (SkPoint);
    this->makeSpace(space);
    this->fVerbs[~fVerbCnt] = verb;
    SkPoint* ret = fPoints + fPointCnt;
    fVerbCnt += 1;
    fPointCnt += pCnt;
    fFreeSpace -= space;
    fBoundsIsDirty = true;  // this also invalidates fIsFinite
    if (dirtyAfterEdit) {
        fIsOval = false;
    }

    if (SkPath::kConic_Verb == verb) {
        *fConicWeights.append() = weight;
    }

    SkDEBUGCODE(this->validate();)
    return ret;
}

uint32_t SkPathRef::genID() const {
    SkASSERT(!fEditorsAttached);
    static const uint32_t kMask = (static_cast<int64_t>(1) << SkPath::kPathRefGenIDBitCnt) - 1;
    if (!fGenerationID) {
        if (0 == fPointCnt && 0 == fVerbCnt) {
            fGenerationID = kEmptyGenID;
        } else {
            static int32_t  gPathRefGenerationID;
            // do a loop in case our global wraps around, as we never want to return a 0 or the
            // empty ID
            do {
                fGenerationID = (sk_atomic_inc(&gPathRefGenerationID) + 1) & kMask;
            } while (fGenerationID <= kEmptyGenID);
        }
    }
    return fGenerationID;
}

void SkPathRef::addGenIDChangeListener(GenIDChangeListener* listener) {
    if (NULL == listener || this == empty.get()) {
        SkDELETE(listener);
        return;
    }
    *fGenIDChangeListeners.append() = listener;
}

// we need to be called *before* the genID gets changed or zerod
void SkPathRef::callGenIDChangeListeners() {
    for (int i = 0; i < fGenIDChangeListeners.count(); i++) {
        fGenIDChangeListeners[i]->onChange();
    }

    // Listeners get at most one shot, so whether these triggered or not, blow them away.
    fGenIDChangeListeners.deleteAll();
}

#ifdef SK_DEBUG
void SkPathRef::validate() const {
    this->INHERITED::validate();
    SkASSERT(static_cast<ptrdiff_t>(fFreeSpace) >= 0);
    SkASSERT(reinterpret_cast<intptr_t>(fVerbs) - reinterpret_cast<intptr_t>(fPoints) >= 0);
    SkASSERT((NULL == fPoints) == (NULL == fVerbs));
    SkASSERT(!(NULL == fPoints && 0 != fFreeSpace));
    SkASSERT(!(NULL == fPoints && 0 != fFreeSpace));
    SkASSERT(!(NULL == fPoints && fPointCnt));
    SkASSERT(!(NULL == fVerbs && fVerbCnt));
    SkASSERT(this->currSize() ==
                fFreeSpace + sizeof(SkPoint) * fPointCnt + sizeof(uint8_t) * fVerbCnt);

    if (!fBoundsIsDirty && !fBounds.isEmpty()) {
        bool isFinite = true;
        for (int i = 0; i < fPointCnt; ++i) {
#ifdef SK_DEBUG
            if (fPoints[i].isFinite() &&
                (fPoints[i].fX < fBounds.fLeft || fPoints[i].fX > fBounds.fRight ||
                 fPoints[i].fY < fBounds.fTop || fPoints[i].fY > fBounds.fBottom)) {
                SkDebugf("bounds: %f %f %f %f\n",
                         fBounds.fLeft, fBounds.fTop, fBounds.fRight, fBounds.fBottom);
                for (int j = 0; j < fPointCnt; ++j) {
                    if (i == j) {
                        SkDebugf("*");
                    }
                    SkDebugf("%f %f\n", fPoints[j].fX, fPoints[j].fY);
                }
            }
#endif

            SkASSERT(!fPoints[i].isFinite() ||
		     (fPoints[i].fX >= fBounds.fLeft && fPoints[i].fX <= fBounds.fRight &&
		      fPoints[i].fY >= fBounds.fTop && fPoints[i].fY <= fBounds.fBottom));
            if (!fPoints[i].isFinite()) {
                isFinite = false;
            }
        }
        SkASSERT(SkToBool(fIsFinite) == isFinite);
    }

#ifdef SK_DEBUG_PATH
    uint32_t mask = 0;
    for (int i = 0; i < fVerbCnt; ++i) {
        switch (fVerbs[~i]) {
            case SkPath::kMove_Verb:
                break;
            case SkPath::kLine_Verb:
                mask |= SkPath::kLine_SegmentMask;
                break;
            case SkPath::kQuad_Verb:
                mask |= SkPath::kQuad_SegmentMask;
                break;
            case SkPath::kConic_Verb:
                mask |= SkPath::kConic_SegmentMask;
                break;
            case SkPath::kCubic_Verb:
                mask |= SkPath::kCubic_SegmentMask;
                break;
            case SkPath::kClose_Verb:
                break;
            case SkPath::kDone_Verb:
                SkDEBUGFAIL("Done verb shouldn't be recorded.");
                break;
            default:
                SkDEBUGFAIL("Unknown Verb");
                break;
        }
    }
    SkASSERT(mask == fSegmentMask);
#endif // SK_DEBUG_PATH
}
#endif
