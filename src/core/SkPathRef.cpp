/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBuffer.h"
#include "SkOnce.h"
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
    fPathRef->fGenerationID = 0;
    fPathRef->fIsOval = false;
    SkDEBUGCODE(sk_atomic_inc(&fPathRef->fEditorsAttached);)
}

SkPoint* SkPathRef::Editor::growForConic(SkScalar w) {
    SkDEBUGCODE(fPathRef->validate();)
    SkPoint* pts = fPathRef->growForVerb(SkPath::kConic_Verb);
    *fPathRef->fConicWeights.append() = w;
    return pts;
}

//////////////////////////////////////////////////////////////////////////////
void SkPathRef::CreateEmptyImpl(SkPathRef** empty) {
    *empty = SkNEW(SkPathRef);
    (*empty)->computeBounds();  // Preemptively avoid a race to clear fBoundsIsDirty.
}

SkPathRef* SkPathRef::CreateEmpty() {
    static SkPathRef* gEmptyPathRef;
    SK_DECLARE_STATIC_ONCE(once);
    SkOnce(&once, SkPathRef::CreateEmptyImpl, &gEmptyPathRef);
    return SkRef(gEmptyPathRef);
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

    // It's an oval only if it stays a rect.
    (*dst)->fIsOval = src.fIsOval && matrix.rectStaysRect();

    SkDEBUGCODE((*dst)->validate();)
}

SkPathRef* SkPathRef::CreateFromBuffer(SkRBuffer* buffer
#ifndef DELETE_THIS_CODE_WHEN_SKPS_ARE_REBUILT_AT_V16_AND_ALL_OTHER_INSTANCES_TOO
                                   , bool newFormat, int32_t oldPacked
#endif
    ) {
    SkPathRef* ref = SkNEW(SkPathRef);
    bool isOval;

    int32_t packed;
    if (!buffer->readS32(&packed)) {
        SkDELETE(ref);
        return NULL;
    }

    ref->fIsFinite = (packed >> kIsFinite_SerializationShift) & 1;

#ifndef DELETE_THIS_CODE_WHEN_SKPS_ARE_REBUILT_AT_V16_AND_ALL_OTHER_INSTANCES_TOO
    if (newFormat) {
#endif
        isOval  = (packed >> kIsOval_SerializationShift) & 1;
#ifndef DELETE_THIS_CODE_WHEN_SKPS_ARE_REBUILT_AT_V16_AND_ALL_OTHER_INSTANCES_TOO
    } else {
        isOval  = (oldPacked >> SkPath::kOldIsOval_SerializationShift) & 1;
    }
#endif

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
    ref->fIsOval = isOval;
    return ref;
}

void SkPathRef::Rewind(SkAutoTUnref<SkPathRef>* pathRef) {
    if ((*pathRef)->unique()) {
        SkDEBUGCODE((*pathRef)->validate();)
        (*pathRef)->fBoundsIsDirty = true;  // this also invalidates fIsFinite
        (*pathRef)->fVerbCnt = 0;
        (*pathRef)->fPointCnt = 0;
        (*pathRef)->fFreeSpace = (*pathRef)->currSize();
        (*pathRef)->fGenerationID = 0;
        (*pathRef)->fConicWeights.rewind();
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
    if (0 != memcmp(this->verbsMemBegin(),
                    ref.verbsMemBegin(),
                    ref.fVerbCnt * sizeof(uint8_t))) {
        SkASSERT(!genIDMatch);
        return false;
    }
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
    // We've done the work to determine that these are equal. If either has a zero genID, copy
    // the other's. If both are 0 then genID() will compute the next ID.
    if (0 == fGenerationID) {
        fGenerationID = ref.genID();
    } else if (0 == ref.fGenerationID) {
        ref.fGenerationID = this->genID();
    }
    return true;
}

void SkPathRef::writeToBuffer(SkWBuffer* buffer) {
    SkDEBUGCODE(this->validate();)
    SkDEBUGCODE(size_t beforePos = buffer->pos();)

    // Call getBounds() to ensure (as a side-effect) that fBounds
    // and fIsFinite are computed.
    const SkRect& bounds = this->getBounds();

    int32_t packed = ((fIsFinite & 1) << kIsFinite_SerializationShift) |
                     ((fIsOval & 1) << kIsOval_SerializationShift);
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

uint32_t SkPathRef::writeSize() {
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
    // We could call genID() here to force a real ID (instead of 0). However, if we're making
    // a copy then presumably we intend to make a modification immediately afterwards.
    fGenerationID = ref.fGenerationID;
    fBoundsIsDirty = ref.fBoundsIsDirty;
    if (!fBoundsIsDirty) {
        fBounds = ref.fBounds;
        fIsFinite = ref.fIsFinite;
    }
    fIsOval = ref.fIsOval;
    SkDEBUGCODE(this->validate();)
}

SkPoint* SkPathRef::growForVerb(int /* SkPath::Verb*/ verb) {
    SkDEBUGCODE(this->validate();)
    int pCnt;
    bool dirtyAfterEdit = true;
    switch (verb) {
        case SkPath::kMove_Verb:
            pCnt = 1;
            dirtyAfterEdit = false;
            break;
        case SkPath::kLine_Verb:
            pCnt = 1;
            break;
        case SkPath::kQuad_Verb:
            // fall through
        case SkPath::kConic_Verb:
            pCnt = 2;
            break;
        case SkPath::kCubic_Verb:
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
            SkASSERT(!fPoints[i].isFinite() || (
                     fBounds.fLeft - fPoints[i].fX   < SK_ScalarNearlyZero &&
                     fPoints[i].fX - fBounds.fRight  < SK_ScalarNearlyZero &&
                     fBounds.fTop  - fPoints[i].fY   < SK_ScalarNearlyZero &&
                     fPoints[i].fY - fBounds.fBottom < SK_ScalarNearlyZero));
            if (!fPoints[i].isFinite()) {
                isFinite = false;
            }
        }
        SkASSERT(SkToBool(fIsFinite) == isFinite);
    }
}
#endif
