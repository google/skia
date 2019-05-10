/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkPathRef.h"

#include "include/core/SkPath.h"
#include "include/private/SkNx.h"
#include "include/private/SkOnce.h"
#include "include/private/SkTo.h"
#include "src/core/SkBuffer.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkSafeMath.h"

// Conic weights must be 0 < weight <= finite
static bool validate_conic_weights(const SkScalar weights[], int count) {
    for (int i = 0; i < count; ++i) {
        if (weights[i] <= 0 || !SkScalarIsFinite(weights[i])) {
            return false;
        }
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////
SkPathRef::Editor::Editor(sk_sp<SkPathRef>* pathRef,
                          int incReserveVerbs,
                          int incReservePoints)
{
    SkASSERT(incReserveVerbs >= 0);
    SkASSERT(incReservePoints >= 0);

    if ((*pathRef)->unique()) {
        (*pathRef)->incReserve(incReserveVerbs, incReservePoints);
    } else {
        SkPathRef* copy = new SkPathRef;
        copy->copy(**pathRef, incReserveVerbs, incReservePoints);
        pathRef->reset(copy);
    }
    fPathRef = pathRef->get();
    fPathRef->callGenIDChangeListeners();
    fPathRef->fGenerationID = 0;
    fPathRef->fBoundsIsDirty = true;
    SkDEBUGCODE(fPathRef->fEditorsAttached++;)
}

// Sort of like makeSpace(0) but the the additional requirement that we actively shrink the
// allocations to just fit the current needs. makeSpace() will only grow, but never shrinks.
//
void SkPath::shrinkToFit() {
    const size_t kMinFreeSpaceForShrink = 8;    // just made up a small number

    if (fPathRef->fFreeSpace <= kMinFreeSpaceForShrink) {
        return;
    }

    if (fPathRef->unique()) {
        int pointCount = fPathRef->fPointCnt;
        int verbCount = fPathRef->fVerbCnt;

        size_t ptsSize = sizeof(SkPoint) * pointCount;
        size_t vrbSize = sizeof(uint8_t) * verbCount;
        size_t minSize = ptsSize + vrbSize;

        void* newAlloc = sk_malloc_canfail(minSize);
        if (!newAlloc) {
            return; // couldn't allocate the smaller buffer, but that's ok
        }

        sk_careful_memcpy(newAlloc, fPathRef->fPoints, ptsSize);
        sk_careful_memcpy((char*)newAlloc + minSize - vrbSize, fPathRef->verbsMemBegin(), vrbSize);

        sk_free(fPathRef->fPoints);
        fPathRef->fPoints = static_cast<SkPoint*>(newAlloc);
        fPathRef->fVerbs = (uint8_t*)newAlloc + minSize;
        fPathRef->fFreeSpace = 0;
        fPathRef->fConicWeights.shrinkToFit();
    } else {
        sk_sp<SkPathRef> pr(new SkPathRef);
        pr->copy(*fPathRef, 0, 0);
        fPathRef = std::move(pr);
    }

    SkDEBUGCODE(fPathRef->validate();)
}

//////////////////////////////////////////////////////////////////////////////

SkPathRef::~SkPathRef() {
    // Deliberately don't validate() this path ref, otherwise there's no way
    // to read one that's not valid and then free its memory without asserting.
    this->callGenIDChangeListeners();
    SkASSERT(fGenIDChangeListeners.empty());  // These are raw ptrs.
    sk_free(fPoints);

    SkDEBUGCODE(fPoints = nullptr;)
    SkDEBUGCODE(fVerbs = nullptr;)
    SkDEBUGCODE(fVerbCnt = 0x9999999;)
    SkDEBUGCODE(fPointCnt = 0xAAAAAAA;)
    SkDEBUGCODE(fPointCnt = 0xBBBBBBB;)
    SkDEBUGCODE(fGenerationID = 0xEEEEEEEE;)
    SkDEBUGCODE(fEditorsAttached.store(0x7777777);)
}

static SkPathRef* gEmpty = nullptr;

SkPathRef* SkPathRef::CreateEmpty() {
    static SkOnce once;
    once([]{
        gEmpty = new SkPathRef;
        gEmpty->computeBounds();   // Avoids races later to be the first to do this.
    });
    return SkRef(gEmpty);
}

static void transform_dir_and_start(const SkMatrix& matrix, bool isRRect, bool* isCCW,
                                    unsigned* start) {
    int inStart = *start;
    int rm = 0;
    if (isRRect) {
        // Degenerate rrect indices to oval indices and remember the remainder.
        // Ovals have one index per side whereas rrects have two.
        rm = inStart & 0b1;
        inStart /= 2;
    }
    // Is the antidiagonal non-zero (otherwise the diagonal is zero)
    int antiDiag;
    // Is the non-zero value in the top row (either kMScaleX or kMSkewX) negative
    int topNeg;
    // Are the two non-zero diagonal or antidiagonal values the same sign.
    int sameSign;
    if (matrix.get(SkMatrix::kMScaleX) != 0) {
        antiDiag = 0b00;
        if (matrix.get(SkMatrix::kMScaleX) > 0) {
            topNeg = 0b00;
            sameSign = matrix.get(SkMatrix::kMScaleY) > 0 ? 0b01 : 0b00;
        } else {
            topNeg = 0b10;
            sameSign = matrix.get(SkMatrix::kMScaleY) > 0 ? 0b00 : 0b01;
        }
    } else {
        antiDiag = 0b01;
        if (matrix.get(SkMatrix::kMSkewX) > 0) {
            topNeg = 0b00;
            sameSign = matrix.get(SkMatrix::kMSkewY) > 0 ? 0b01 : 0b00;
        } else {
            topNeg = 0b10;
            sameSign = matrix.get(SkMatrix::kMSkewY) > 0 ? 0b00 : 0b01;
        }
    }
    if (sameSign != antiDiag) {
        // This is a rotation (and maybe scale). The direction is unchanged.
        // Trust me on the start computation (or draw yourself some pictures)
        *start = (inStart + 4 - (topNeg | antiDiag)) % 4;
        SkASSERT(*start < 4);
        if (isRRect) {
            *start = 2 * *start + rm;
        }
    } else {
        // This is a mirror (and maybe scale). The direction is reversed.
        *isCCW = !*isCCW;
        // Trust me on the start computation (or draw yourself some pictures)
        *start = (6 + (topNeg | antiDiag) - inStart) % 4;
        SkASSERT(*start < 4);
        if (isRRect) {
            *start = 2 * *start + (rm ? 0 : 1);
        }
    }
}

void SkPathRef::CreateTransformedCopy(sk_sp<SkPathRef>* dst,
                                      const SkPathRef& src,
                                      const SkMatrix& matrix) {
    SkDEBUGCODE(src.validate();)
    if (matrix.isIdentity()) {
        if (dst->get() != &src) {
            src.ref();
            dst->reset(const_cast<SkPathRef*>(&src));
            SkDEBUGCODE((*dst)->validate();)
        }
        return;
    }

    if (!(*dst)->unique()) {
        dst->reset(new SkPathRef);
    }

    if (dst->get() != &src) {
        (*dst)->resetToSize(src.fVerbCnt, src.fPointCnt, src.fConicWeights.count());
        sk_careful_memcpy((*dst)->verbsMemWritable(), src.verbsMemBegin(),
                           src.fVerbCnt * sizeof(uint8_t));
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
    bool rectStaysRect = matrix.rectStaysRect();
    (*dst)->fIsOval = src.fIsOval && rectStaysRect;
    (*dst)->fIsRRect = src.fIsRRect && rectStaysRect;
    if ((*dst)->fIsOval || (*dst)->fIsRRect) {
        unsigned start = src.fRRectOrOvalStartIdx;
        bool isCCW = SkToBool(src.fRRectOrOvalIsCCW);
        transform_dir_and_start(matrix, (*dst)->fIsRRect, &isCCW, &start);
        (*dst)->fRRectOrOvalIsCCW = isCCW;
        (*dst)->fRRectOrOvalStartIdx = start;
    }

    if (dst->get() == &src) {
        (*dst)->callGenIDChangeListeners();
        (*dst)->fGenerationID = 0;
    }

    SkDEBUGCODE((*dst)->validate();)
}

static bool validate_verb_sequence(const uint8_t verbs[], int vCount) {
    // verbs are stored backwards, but we need to visit them in logical order to determine if
    // they form a valid sequence.

    bool needsMoveTo = true;
    bool invalidSequence = false;

    for (int i = vCount - 1; i >= 0; --i) {
        switch (verbs[i]) {
            case SkPath::kMove_Verb:
                needsMoveTo = false;
                break;
            case SkPath::kLine_Verb:
            case SkPath::kQuad_Verb:
            case SkPath::kConic_Verb:
            case SkPath::kCubic_Verb:
                invalidSequence |= needsMoveTo;
                break;
            case SkPath::kClose_Verb:
                needsMoveTo = true;
                break;
            default:
                return false;   // unknown verb
        }
    }
    return !invalidSequence;
}

// Given the verb array, deduce the required number of pts and conics,
// or if an invalid verb is encountered, return false.
static bool deduce_pts_conics(const uint8_t verbs[], int vCount, int* ptCountPtr,
                              int* conicCountPtr) {
    // When there is at least one verb, the first is required to be kMove_Verb.
    if (0 < vCount && verbs[vCount-1] != SkPath::kMove_Verb) {
        return false;
    }

    SkSafeMath safe;
    int ptCount = 0;
    int conicCount = 0;
    for (int i = 0; i < vCount; ++i) {
        switch (verbs[i]) {
            case SkPath::kMove_Verb:
            case SkPath::kLine_Verb:
                ptCount = safe.addInt(ptCount, 1);
                break;
            case SkPath::kConic_Verb:
                conicCount += 1;
                // fall-through
            case SkPath::kQuad_Verb:
                ptCount = safe.addInt(ptCount, 2);
                break;
            case SkPath::kCubic_Verb:
                ptCount = safe.addInt(ptCount, 3);
                break;
            case SkPath::kClose_Verb:
                break;
            default:
                return false;
        }
    }
    if (!safe) {
        return false;
    }
    *ptCountPtr = ptCount;
    *conicCountPtr = conicCount;
    return true;
}

SkPathRef* SkPathRef::CreateFromBuffer(SkRBuffer* buffer) {
    std::unique_ptr<SkPathRef> ref(new SkPathRef);

    int32_t packed;
    if (!buffer->readS32(&packed)) {
        return nullptr;
    }

    ref->fIsFinite = (packed >> kIsFinite_SerializationShift) & 1;

    int32_t verbCount, pointCount, conicCount;
    if (!buffer->readU32(&(ref->fGenerationID)) ||
        !buffer->readS32(&verbCount)            || (verbCount  < 0) ||
        !buffer->readS32(&pointCount)           || (pointCount < 0) ||
        !buffer->readS32(&conicCount)           || (conicCount < 0))
    {
        return nullptr;
    }

    uint64_t pointSize64 = sk_64_mul(pointCount, sizeof(SkPoint));
    uint64_t conicSize64 = sk_64_mul(conicCount, sizeof(SkScalar));
    if (!SkTFitsIn<size_t>(pointSize64) || !SkTFitsIn<size_t>(conicSize64)) {
        return nullptr;
    }

    size_t verbSize = verbCount * sizeof(uint8_t);
    size_t pointSize = SkToSizeT(pointSize64);
    size_t conicSize = SkToSizeT(conicSize64);

    {
        uint64_t requiredBufferSize = sizeof(SkRect);
        requiredBufferSize += verbSize;
        requiredBufferSize += pointSize;
        requiredBufferSize += conicSize;
        if (buffer->available() < requiredBufferSize) {
            return nullptr;
        }
    }

    ref->resetToSize(verbCount, pointCount, conicCount);
    SkASSERT(verbCount  == ref->countVerbs());
    SkASSERT(pointCount == ref->countPoints());
    SkASSERT(conicCount == ref->fConicWeights.count());

    if (!buffer->read(ref->verbsMemWritable(), verbSize) ||
        !buffer->read(ref->fPoints, pointSize) ||
        !buffer->read(ref->fConicWeights.begin(), conicSize) ||
        !buffer->read(&ref->fBounds, sizeof(SkRect))) {
        return nullptr;
    }

    // Check that the verbs are valid, and imply the correct number of pts and conics
    {
        int pCount, cCount;
        if (!validate_verb_sequence(ref->verbsMemBegin(), ref->countVerbs())) {
            return nullptr;
        }
        if (!deduce_pts_conics(ref->verbsMemBegin(), ref->countVerbs(), &pCount, &cCount) ||
            pCount != ref->countPoints() || cCount != ref->fConicWeights.count()) {
            return nullptr;
        }
        if (!validate_conic_weights(ref->fConicWeights.begin(), ref->fConicWeights.count())) {
            return nullptr;
        }
        // Check that the bounds match the serialized bounds.
        SkRect bounds;
        if (ComputePtBounds(&bounds, *ref) != SkToBool(ref->fIsFinite) || bounds != ref->fBounds) {
            return nullptr;
        }

        // call this after validate_verb_sequence, since it relies on valid verbs
        ref->fSegmentMask = ref->computeSegmentMask();
    }

    ref->fBoundsIsDirty = false;

    return ref.release();
}

void SkPathRef::Rewind(sk_sp<SkPathRef>* pathRef) {
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
        (*pathRef)->fIsRRect = false;
        SkDEBUGCODE((*pathRef)->validate();)
    } else {
        int oldVCnt = (*pathRef)->countVerbs();
        int oldPCnt = (*pathRef)->countPoints();
        pathRef->reset(new SkPathRef);
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

    // We store fSegmentMask for older readers, but current readers can't trust it, so they
    // don't read it.
    int32_t packed = ((fIsFinite & 1) << kIsFinite_SerializationShift) |
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
    sk_careful_memcpy(this->verbsMemWritable(), ref.verbsMemBegin(), ref.fVerbCnt*sizeof(uint8_t));
    sk_careful_memcpy(this->fPoints, ref.fPoints, ref.fPointCnt * sizeof(SkPoint));
    fConicWeights = ref.fConicWeights;
    fBoundsIsDirty = ref.fBoundsIsDirty;
    if (!fBoundsIsDirty) {
        fBounds = ref.fBounds;
        fIsFinite = ref.fIsFinite;
    }
    fSegmentMask = ref.fSegmentMask;
    fIsOval = ref.fIsOval;
    fIsRRect = ref.fIsRRect;
    fRRectOrOvalIsCCW = ref.fRRectOrOvalIsCCW;
    fRRectOrOvalStartIdx = ref.fRRectOrOvalStartIdx;
    SkDEBUGCODE(this->validate();)
}

unsigned SkPathRef::computeSegmentMask() const {
    const uint8_t* verbs = this->verbsMemBegin();
    unsigned mask = 0;
    for (int i = this->countVerbs() - 1; i >= 0; --i) {
        switch (verbs[i]) {
            case SkPath::kLine_Verb:  mask |= SkPath::kLine_SegmentMask; break;
            case SkPath::kQuad_Verb:  mask |= SkPath::kQuad_SegmentMask; break;
            case SkPath::kConic_Verb: mask |= SkPath::kConic_SegmentMask; break;
            case SkPath::kCubic_Verb: mask |= SkPath::kCubic_SegmentMask; break;
            default: break;
        }
    }
    return mask;
}

void SkPathRef::interpolate(const SkPathRef& ending, SkScalar weight, SkPathRef* out) const {
    const SkScalar* inValues = &ending.getPoints()->fX;
    SkScalar* outValues = &out->getPoints()->fX;
    int count = out->countPoints() * 2;
    for (int index = 0; index < count; ++index) {
        outValues[index] = outValues[index] * weight + inValues[index] * (1 - weight);
    }
    out->fBoundsIsDirty = true;
    out->fIsOval = false;
    out->fIsRRect = false;
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
    switch (verb) {
        case SkPath::kMove_Verb:
            pCnt = numVbs;
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
            break;
        case SkPath::kDone_Verb:
            SkDEBUGFAIL("growForRepeatedVerb called for kDone");
            // fall through
        default:
            SkDEBUGFAIL("default should not be reached");
            pCnt = 0;
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

    SkSafeMath safe;
    fVerbCnt = safe.addInt(fVerbCnt, numVbs);
    fPointCnt = safe.addInt(fPointCnt, pCnt);
    if (!safe) {
        SK_ABORT("cannot grow path");
    }
    fFreeSpace -= space;
    fBoundsIsDirty = true;  // this also invalidates fIsFinite
    fIsOval = false;
    fIsRRect = false;

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
    unsigned mask = 0;
    switch (verb) {
        case SkPath::kMove_Verb:
            pCnt = 1;
            break;
        case SkPath::kLine_Verb:
            mask = SkPath::kLine_SegmentMask;
            pCnt = 1;
            break;
        case SkPath::kQuad_Verb:
            mask = SkPath::kQuad_SegmentMask;
            pCnt = 2;
            break;
        case SkPath::kConic_Verb:
            mask = SkPath::kConic_SegmentMask;
            pCnt = 2;
            break;
        case SkPath::kCubic_Verb:
            mask = SkPath::kCubic_SegmentMask;
            pCnt = 3;
            break;
        case SkPath::kClose_Verb:
            pCnt = 0;
            break;
        case SkPath::kDone_Verb:
            SkDEBUGFAIL("growForVerb called for kDone");
            // fall through
        default:
            SkDEBUGFAIL("default is not reached");
            pCnt = 0;
    }
    SkSafeMath safe;
    int newPointCnt = safe.addInt(fPointCnt, pCnt);
    int newVerbCnt  = safe.addInt(fVerbCnt, 1);
    if (!safe) {
        SK_ABORT("cannot grow path");
    }
    size_t space = sizeof(uint8_t) + pCnt * sizeof (SkPoint);
    this->makeSpace(space);
    this->fVerbs[~fVerbCnt] = verb;
    SkPoint* ret = fPoints + fPointCnt;
    fVerbCnt = newVerbCnt;
    fPointCnt = newPointCnt;
    fSegmentMask |= mask;
    fFreeSpace -= space;
    fBoundsIsDirty = true;  // this also invalidates fIsFinite
    fIsOval = false;
    fIsRRect = false;

    if (SkPath::kConic_Verb == verb) {
        *fConicWeights.append() = weight;
    }

    SkDEBUGCODE(this->validate();)
    return ret;
}

uint32_t SkPathRef::genID() const {
    SkASSERT(fEditorsAttached.load() == 0);
    static const uint32_t kMask = (static_cast<int64_t>(1) << SkPathPriv::kPathRefGenIDBitCnt) - 1;

    if (fGenerationID == 0) {
        if (fPointCnt == 0 && fVerbCnt == 0) {
            fGenerationID = kEmptyGenID;
        } else {
            static std::atomic<uint32_t> nextID{kEmptyGenID + 1};
            do {
                fGenerationID = nextID.fetch_add(1, std::memory_order_relaxed) & kMask;
            } while (fGenerationID == 0 || fGenerationID == kEmptyGenID);
        }
    }
    return fGenerationID;
}

void SkPathRef::addGenIDChangeListener(sk_sp<GenIDChangeListener> listener) {
    if (nullptr == listener || this == gEmpty) {
        return;
    }

    SkAutoMutexExclusive lock(fGenIDChangeListenersMutex);

    // Clean out any stale listeners before we append the new one.
    for (int i = 0; i < fGenIDChangeListeners.count(); ++i) {
        if (fGenIDChangeListeners[i]->shouldUnregisterFromPath()) {
            fGenIDChangeListeners[i]->unref();
            fGenIDChangeListeners.removeShuffle(i--);  // No need to preserve the order after i.
        }
    }

    SkASSERT(!listener->shouldUnregisterFromPath());
    *fGenIDChangeListeners.append() = listener.release();
}

// we need to be called *before* the genID gets changed or zerod
void SkPathRef::callGenIDChangeListeners() {
    SkAutoMutexExclusive lock(fGenIDChangeListenersMutex);
    for (GenIDChangeListener* listener : fGenIDChangeListeners) {
        if (!listener->shouldUnregisterFromPath()) {
            listener->onChange();
        }
        // Listeners get at most one shot, so whether these triggered or not, blow them away.
        listener->unref();
    }

    fGenIDChangeListeners.reset();
}

SkRRect SkPathRef::getRRect() const {
    const SkRect& bounds = this->getBounds();
    SkVector radii[4] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
    Iter iter(*this);
    SkPoint pts[4];
    uint8_t verb = iter.next(pts);
    SkASSERT(SkPath::kMove_Verb == verb);
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        if (SkPath::kConic_Verb == verb) {
            SkVector v1_0 = pts[1] - pts[0];
            SkVector v2_1 = pts[2] - pts[1];
            SkVector dxdy;
            if (v1_0.fX) {
                SkASSERT(!v2_1.fX && !v1_0.fY);
                dxdy.set(SkScalarAbs(v1_0.fX), SkScalarAbs(v2_1.fY));
            } else if (!v1_0.fY) {
                SkASSERT(!v2_1.fX || !v2_1.fY);
                dxdy.set(SkScalarAbs(v2_1.fX), SkScalarAbs(v2_1.fY));
            } else {
                SkASSERT(!v2_1.fY);
                dxdy.set(SkScalarAbs(v2_1.fX), SkScalarAbs(v1_0.fY));
            }
            SkRRect::Corner corner =
                    pts[1].fX == bounds.fLeft ?
                        pts[1].fY == bounds.fTop ?
                            SkRRect::kUpperLeft_Corner : SkRRect::kLowerLeft_Corner :
                    pts[1].fY == bounds.fTop ?
                            SkRRect::kUpperRight_Corner : SkRRect::kLowerRight_Corner;
            SkASSERT(!radii[corner].fX && !radii[corner].fY);
            radii[corner] = dxdy;
        } else {
            SkASSERT((verb == SkPath::kLine_Verb
                    && (!(pts[1].fX - pts[0].fX) || !(pts[1].fY - pts[0].fY)))
                    || verb == SkPath::kClose_Verb);
        }
    }
    SkRRect rrect;
    rrect.setRectRadii(bounds, radii);
    return rrect;
}

///////////////////////////////////////////////////////////////////////////////

SkPathRef::Iter::Iter() {
#ifdef SK_DEBUG
    fPts = nullptr;
    fConicWeights = nullptr;
#endif
    // need to init enough to make next() harmlessly return kDone_Verb
    fVerbs = nullptr;
    fVerbStop = nullptr;
}

SkPathRef::Iter::Iter(const SkPathRef& path) {
    this->setPathRef(path);
}

void SkPathRef::Iter::setPathRef(const SkPathRef& path) {
    fPts = path.points();
    fVerbs = path.verbs();
    fVerbStop = path.verbsMemBegin();
    fConicWeights = path.conicWeights();
    if (fConicWeights) {
      fConicWeights -= 1;  // begin one behind
    }

    // Don't allow iteration through non-finite points.
    if (!path.isFinite()) {
        fVerbStop = fVerbs;
    }
}

uint8_t SkPathRef::Iter::next(SkPoint pts[4]) {
    SkASSERT(pts);

    SkDEBUGCODE(unsigned peekResult = this->peek();)

    if (fVerbs == fVerbStop) {
        SkASSERT(peekResult == SkPath::kDone_Verb);
        return (uint8_t) SkPath::kDone_Verb;
    }

    // fVerbs points one beyond next verb so decrement first.
    unsigned verb = *(--fVerbs);
    const SkPoint* srcPts = fPts;

    switch (verb) {
        case SkPath::kMove_Verb:
            pts[0] = srcPts[0];
            srcPts += 1;
            break;
        case SkPath::kLine_Verb:
            pts[0] = srcPts[-1];
            pts[1] = srcPts[0];
            srcPts += 1;
            break;
        case SkPath::kConic_Verb:
            fConicWeights += 1;
            // fall-through
        case SkPath::kQuad_Verb:
            pts[0] = srcPts[-1];
            pts[1] = srcPts[0];
            pts[2] = srcPts[1];
            srcPts += 2;
            break;
        case SkPath::kCubic_Verb:
            pts[0] = srcPts[-1];
            pts[1] = srcPts[0];
            pts[2] = srcPts[1];
            pts[3] = srcPts[2];
            srcPts += 3;
            break;
        case SkPath::kClose_Verb:
            break;
        case SkPath::kDone_Verb:
            SkASSERT(fVerbs == fVerbStop);
            break;
    }
    fPts = srcPts;
    SkASSERT(peekResult == verb);
    return (uint8_t) verb;
}

uint8_t SkPathRef::Iter::peek() const {
    const uint8_t* next = fVerbs;
    return next <= fVerbStop ? (uint8_t) SkPath::kDone_Verb : next[-1];
}


bool SkPathRef::isValid() const {
    if (static_cast<ptrdiff_t>(fFreeSpace) < 0) {
        return false;
    }
    if (reinterpret_cast<intptr_t>(fVerbs) - reinterpret_cast<intptr_t>(fPoints) < 0) {
        return false;
    }
    if ((nullptr == fPoints) != (nullptr == fVerbs)) {
        return false;
    }
    if (nullptr == fPoints && 0 != fFreeSpace) {
        return false;
    }
    if (nullptr == fPoints && fPointCnt) {
        return false;
    }
    if (nullptr == fVerbs && fVerbCnt) {
        return false;
    }
    if (this->currSize() !=
                fFreeSpace + sizeof(SkPoint) * fPointCnt + sizeof(uint8_t) * fVerbCnt) {
        return false;
    }

    if (fIsOval || fIsRRect) {
        // Currently we don't allow both of these to be set, even though ovals are ro
        if (fIsOval == fIsRRect) {
            return false;
        }
        if (fIsOval) {
            if (fRRectOrOvalStartIdx >= 4) {
                return false;
            }
        } else {
            if (fRRectOrOvalStartIdx >= 8) {
                return false;
            }
        }
    }

    if (!fBoundsIsDirty && !fBounds.isEmpty()) {
        bool isFinite = true;
        Sk2s leftTop = Sk2s(fBounds.fLeft, fBounds.fTop);
        Sk2s rightBot = Sk2s(fBounds.fRight, fBounds.fBottom);
        for (int i = 0; i < fPointCnt; ++i) {
            Sk2s point = Sk2s(fPoints[i].fX, fPoints[i].fY);
#ifdef SK_DEBUG
            if (fPoints[i].isFinite() &&
                ((point < leftTop).anyTrue() || (point > rightBot).anyTrue())) {
                SkDebugf("bad SkPathRef bounds: %g %g %g %g\n",
                         fBounds.fLeft, fBounds.fTop, fBounds.fRight, fBounds.fBottom);
                for (int j = 0; j < fPointCnt; ++j) {
                    if (i == j) {
                        SkDebugf("*** bounds do not contain: ");
                    }
                    SkDebugf("%g %g\n", fPoints[j].fX, fPoints[j].fY);
                }
                return false;
            }
#endif

            if (fPoints[i].isFinite() && (point < leftTop).anyTrue() && !(point > rightBot).anyTrue())
                return false;
            if (!fPoints[i].isFinite()) {
                isFinite = false;
            }
        }
        if (SkToBool(fIsFinite) != isFinite) {
            return false;
        }
    }
    return true;
}
