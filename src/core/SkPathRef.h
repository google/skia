
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathRef_DEFINED
#define SkPathRef_DEFINED

#include "SkRefCnt.h"
#include <stddef.h> // ptrdiff_t

// When we're ready to break the picture format. Changes:
// * Write genID.
// * SkPathRef read/write counts (which will change the field order)
// * SkPathRef reads/writes verbs backwards.
#define NEW_PICTURE_FORMAT 0

/**
 * Holds the path verbs and points. It is versioned by a generation ID. None of its public methods
 * modify the contents. To modify or append to the verbs/points wrap the SkPathRef in an
 * SkPathRef::Editor object. Installing the editor resets the generation ID. It also performs
 * copy-on-write if the SkPathRef is shared by multipls SkPaths. The caller passes the Editor's
 * constructor a SkAutoTUnref, which may be updated to point to a new SkPathRef after the editor's
 * constructor returns.
 *
 * The points and verbs are stored in a single allocation. The points are at the begining of the
 * allocation while the verbs are stored at end of the allocation, in reverse order. Thus the points
 * and verbs both grow into the middle of the allocation until the meet. To access verb i in the
 * verb array use ref.verbs()[~i] (because verbs() returns a pointer just beyond the first
 * logical verb or the last verb in memory).
 */
class SkPathRef : public ::SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkPathRef);

    class Editor {
    public:
        Editor(SkAutoTUnref<SkPathRef>* pathRef,
               int incReserveVerbs = 0,
               int incReservePoints = 0) {
            if (pathRef->get()->getRefCnt() > 1) {
                SkPathRef* copy = SkNEW(SkPathRef);
                copy->copy(*pathRef->get(), incReserveVerbs, incReservePoints);
                pathRef->reset(copy);
            } else {
                (*pathRef)->incReserve(incReserveVerbs, incReservePoints);
            }
            fPathRef = pathRef->get();
            fPathRef->fGenerationID = 0;
            SkDEBUGCODE(sk_atomic_inc(&fPathRef->fEditorsAttached);)
        }

        ~Editor() { SkDEBUGCODE(sk_atomic_dec(&fPathRef->fEditorsAttached);) }

        /**
         * Returns the array of points.
         */
        SkPoint* points() { return fPathRef->fPoints; }

        /**
         * Gets the ith point. Shortcut for this->points() + i
         */
        SkPoint* atPoint(int i) {
            SkASSERT((unsigned) i < (unsigned) fPathRef->fPointCnt);
            return this->points() + i;
        };

        /**
         * Adds the verb and allocates space for the number of points indicated by the verb. The
         * return value is a pointer to where the points for the verb should be written.
         */
        SkPoint* growForVerb(SkPath::Verb verb) {
            fPathRef->validate();
            return fPathRef->growForVerb(verb);
        }

        /**
         * Allocates space for additional verbs and points and returns pointers to the new verbs and
         * points. verbs will point one beyond the first new verb (index it using [~<i>]). pts points
         * at the first new point (indexed normally [<i>]).
         */
        void grow(int newVerbs, int newPts, uint8_t** verbs, SkPoint** pts) {
            SkASSERT(NULL != verbs);
            SkASSERT(NULL != pts);
            fPathRef->validate();
            int oldVerbCnt = fPathRef->fVerbCnt;
            int oldPointCnt = fPathRef->fPointCnt;
            SkASSERT(verbs && pts);
            fPathRef->grow(newVerbs, newPts);
            *verbs = fPathRef->fVerbs - oldVerbCnt;
            *pts = fPathRef->fPoints + oldPointCnt;
            fPathRef->validate();
        }

        /**
         * Resets the path ref to a new verb and point count. The new verbs and points are
         * uninitialized.
         */
        void resetToSize(int newVerbCnt, int newPointCnt) {
            fPathRef->resetToSize(newVerbCnt, newPointCnt);
        }
        /**
         * Gets the path ref that is wrapped in the Editor.
         */
        SkPathRef* pathRef() { return fPathRef; }

    private:
        SkPathRef* fPathRef;
    };

public:

    /**
     * Gets a path ref with no verbs or points.
     */
    static SkPathRef* CreateEmpty() {
        static SkAutoTUnref<SkPathRef> gEmptyPathRef(SkNEW(SkPathRef));
        gEmptyPathRef.get()->ref();
        return gEmptyPathRef.get();
    }

    /**
     * Transforms a path ref by a matrix, allocating a new one only if necessary.
     */
    static void CreateTransformedCopy(SkAutoTUnref<SkPathRef>* dst,
                                      const SkPathRef& src,
                                      const SkMatrix& matrix) {
        src.validate();
        if (matrix.isIdentity()) {
            if (dst->get() != &src) {
                dst->reset(const_cast<SkPathRef*>(&src));
                (*dst)->validate();
                src.ref();
            }
            return;
        }
        int32_t rcnt = dst->get()->getRefCnt();
        if (&src == dst->get() && 1 == rcnt) {
            matrix.mapPoints((*dst)->fPoints, (*dst)->fPointCnt);
            return;
        } else if (rcnt > 1) {
            dst->reset(SkNEW(SkPathRef));
        }
        (*dst)->resetToSize(src.fVerbCnt, src.fPointCnt);
        memcpy((*dst)->verbsMemWritable(), src.verbsMemBegin(), src.fVerbCnt * sizeof(uint8_t));
        matrix.mapPoints((*dst)->fPoints, src.points(), src.fPointCnt);
        (*dst)->validate();
    }

#if NEW_PICTURE_FORMAT
    static SkPathRef* CreateFromBuffer(SkRBuffer* buffer) {
        SkPathRef* ref = SkNEW(SkPathRef);
        ref->fGenerationID = buffer->readU32();
        int32_t verbCount = buffer->readS32();
        int32_t pointCount = buffer->readS32();
        ref->resetToSize(verbCount, pointCount);

        SkASSERT(verbCount == ref->countVerbs());
        SkASSERT(pointCount == ref->countPoints());
        buffer->read(ref->verbsMemWritable(), verbCount * sizeof(uint8_t));
        buffer->read(ref->fPoints, pointCount * sizeof(SkPoint));
        return ref;
    }
#else
    static SkPathRef* CreateFromBuffer(int verbCount, int pointCount, SkRBuffer* buffer) {
        SkPathRef* ref = SkNEW(SkPathRef);

        ref->resetToSize(verbCount, pointCount);
        SkASSERT(verbCount == ref->countVerbs());
        SkASSERT(pointCount == ref->countPoints());
        buffer->read(ref->fPoints, pointCount * sizeof(SkPoint));
        for (int i = 0; i < verbCount; ++i) {
            ref->fVerbs[~i] = buffer->readU8();
        }
        return ref;
    }
#endif

    /**
     * Rollsback a path ref to zero verbs and points with the assumption that the path ref will be
     * repopulated with approximately the same number of verbs and points. A new path ref is created
     * only if necessary.
     */
    static void Rewind(SkAutoTUnref<SkPathRef>* pathRef) {
        if (1 == (*pathRef)->getRefCnt()) {
            (*pathRef)->validate();
            (*pathRef)->fVerbCnt = 0;
            (*pathRef)->fPointCnt = 0;
            (*pathRef)->fFreeSpace = (*pathRef)->currSize();
            (*pathRef)->fGenerationID = 0;
            (*pathRef)->validate();
        } else {
            int oldVCnt = (*pathRef)->countVerbs();
            int oldPCnt = (*pathRef)->countPoints();
            pathRef->reset(SkNEW(SkPathRef));
            (*pathRef)->resetToSize(0, 0, oldVCnt, oldPCnt);
        }
    }

    virtual ~SkPathRef() {
        this->validate();
        sk_free(fPoints);
    }

    int countPoints() const { this->validate(); return fPointCnt; }
    int countVerbs() const { this->validate(); return fVerbCnt; }

    /**
     * Returns a pointer one beyond the first logical verb (last verb in memory order).
     */
    const uint8_t* verbs() const { this->validate(); return fVerbs; }

    /**
     * Returns a const pointer to the first verb in memory (which is the last logical verb).
     */
    const uint8_t* verbsMemBegin() const { return this->verbs() - fVerbCnt; }

    /**
     * Returns a const pointer to the first point.
     */
    const SkPoint* points() const { this->validate(); return fPoints; }

    /**
     * Shortcut for this->points() + this->countPoints()
     */
    const SkPoint* pointsEnd() const { return this->points() + this->countPoints(); }

    /**
     * Convenience methods for getting to a verb or point by index.
     */
    uint8_t atVerb(int index) {
        SkASSERT((unsigned) index < (unsigned) fVerbCnt);
        return this->verbs()[~index];
    }
    const SkPoint& atPoint(int index) const {
        SkASSERT((unsigned) index < (unsigned) fPointCnt);
        return this->points()[index];
    }

    bool operator== (const SkPathRef& ref) const {
        this->validate();
        ref.validate();
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
        // We've done the work to determine that these are equal. If either has a zero genID, copy
        // the other's. If both are 0 then genID() will compute the next ID.
        if (0 == fGenerationID) {
            fGenerationID = ref.genID();
        } else if (0 == ref.fGenerationID) {
            ref.fGenerationID = this->genID();
        }
        return true;
    }

    /**
     * Writes the path points and verbs to a buffer.
     */
#if NEW_PICTURE_FORMAT
    void writeToBuffer(SkWBuffer* buffer) {
        this->validate();
        SkDEBUGCODE(size_t beforePos = buffer->pos();)

        // TODO: write gen ID here. Problem: We don't know if we're cross process or not from
        // SkWBuffer. Until this is fixed we write 0.
        buffer->write32(0);
        buffer->write32(this->fVerbCnt);
        buffer->write32(this->fPointCnt);
        buffer->write(this->verbsMemBegin(), fVerbCnt * sizeof(uint8_t));
        buffer->write(fPoints, fPointCnt * sizeof(SkPoint));

        SkASSERT(buffer->pos() - beforePos == (size_t) this->writeSize());
    }

    /**
     * Gets the number of bytes that would be written in writeBuffer()
     */
    uint32_t writeSize() {
        return 3 * sizeof(uint32_t) + fVerbCnt * sizeof(uint8_t) + fPointCnt * sizeof(SkPoint);
    }
#else
    void writeToBuffer(SkWBuffer* buffer) {
        this->validate();
        buffer->write(fPoints, fPointCnt * sizeof(SkPoint));
        for (int i = 0; i < fVerbCnt; ++i) {
            buffer->write8(fVerbs[~i]);
        }
    }
#endif

private:
    SkPathRef() {
        fPointCnt = 0;
        fVerbCnt = 0;
        fVerbs = NULL;
        fPoints = NULL;
        fFreeSpace = 0;
        fGenerationID = kEmptyGenID;
        SkDEBUGCODE(fEditorsAttached = 0;)
        this->validate();
    }

    void copy(const SkPathRef& ref, int additionalReserveVerbs, int additionalReservePoints) {
        this->validate();
        this->resetToSize(ref.fVerbCnt, ref.fPointCnt,
                          additionalReserveVerbs, additionalReservePoints);
        memcpy(this->verbsMemWritable(), ref.verbsMemBegin(), ref.fVerbCnt * sizeof(uint8_t));
        memcpy(this->fPoints, ref.fPoints, ref.fPointCnt * sizeof(SkPoint));
        // We could call genID() here to force a real ID (instead of 0). However, if we're making
        // a copy then presumably we intend to make a modification immediately afterwards.
        fGenerationID = ref.fGenerationID;
        this->validate();
    }

    /** Makes additional room but does not change the counts or change the genID */
    void incReserve(int additionalVerbs, int additionalPoints) {
        this->validate();
        size_t space = additionalVerbs * sizeof(uint8_t) + additionalPoints * sizeof (SkPoint);
        this->makeSpace(space);
        this->validate();
    }

    /** Resets the path ref with verbCount verbs and pointCount points, all unitialized. Also
     *  allocates space for reserveVerb additional verbs and reservePoints additional points.*/
    void resetToSize(int verbCount, int pointCount, int reserveVerbs = 0, int reservePoints = 0) {
        this->validate();
        fGenerationID = 0;

        size_t newSize = sizeof(uint8_t) * verbCount + sizeof(SkPoint) * pointCount;
        size_t newReserve = sizeof(uint8_t) * reserveVerbs + sizeof(SkPoint) * reservePoints;
        size_t minSize = newSize + newReserve;

        ptrdiff_t sizeDelta = this->currSize() - minSize;

        if (sizeDelta < 0 || static_cast<size_t>(sizeDelta) >= 3 * minSize) {
            sk_free(fPoints);
            fPoints = NULL;
            fVerbs = NULL;
            fFreeSpace = 0;
            fVerbCnt = 0;
            fPointCnt = 0;
            this->makeSpace(minSize);
            fVerbCnt = verbCount;
            fPointCnt = pointCount;
            fFreeSpace -= newSize;
        } else {
            fPointCnt = pointCount;
            fVerbCnt = verbCount;
            fFreeSpace = this->currSize() - minSize;
        }
        this->validate();
    }

    /**
     * Increases the verb count by newVerbs and the point count be newPoints. New verbs and points
     * are uninitialized.
     */
    void grow(int newVerbs, int newPoints) {
        this->validate();
        size_t space = newVerbs * sizeof(uint8_t) + newPoints * sizeof (SkPoint);
        this->makeSpace(space);
        fVerbCnt += newVerbs;
        fPointCnt += newPoints;
        fFreeSpace -= space;
        this->validate();
    }

    /**
     * Increases the verb count 1, records the new verb, and creates room for the requisite number
     * of additional points. A pointer to the first point is returned. Any new points are
     * uninitialized.
     */
    SkPoint* growForVerb(SkPath::Verb verb) {
        this->validate();
        int pCnt;
        switch (verb) {
            case SkPath::kMove_Verb:
                pCnt = 1;
                break;
            case SkPath::kLine_Verb:
                pCnt = 1;
                break;
            case SkPath::kQuad_Verb:
                pCnt = 2;
                break;
            case SkPath::kCubic_Verb:
                pCnt = 3;
                break;
            default:
                pCnt = 0;
        }
        size_t space = sizeof(uint8_t) + pCnt * sizeof (SkPoint);
        this->makeSpace(space);
        this->fVerbs[~fVerbCnt] = verb;
        SkPoint* ret = fPoints + fPointCnt;
        fVerbCnt += 1;
        fPointCnt += pCnt;
        fFreeSpace -= space;
        this->validate();
        return ret;
    }

    /**
     * Ensures that the free space available in the path ref is >= size. The verb and point counts
     * are not changed.
     */
    void makeSpace(size_t size) {
        this->validate();
        ptrdiff_t growSize = size - fFreeSpace;
        if (growSize <= 0) {
            return;
        }
        size_t oldSize = this->currSize();
        // round to next multiple of 8 bytes
        growSize = (growSize + 7) & ~static_cast<size_t>(7);
        // we always at least double the allocation
        if (static_cast<size_t>(growSize) < oldSize) {
            growSize = oldSize;
        }
        if (growSize < kMinSize) {
            growSize = kMinSize;
        }
        size_t newSize = oldSize + growSize;
        // Note that realloc could memcpy more than we need. It seems to be a win anyway. TODO:
        // encapsulate this.
        fPoints = reinterpret_cast<SkPoint*>(sk_realloc_throw(fPoints, newSize));
        size_t oldVerbSize = fVerbCnt * sizeof(uint8_t);
        void* newVerbsDst = reinterpret_cast<void*>(
                                reinterpret_cast<intptr_t>(fPoints) + newSize - oldVerbSize);
        void* oldVerbsSrc = reinterpret_cast<void*>(
                                reinterpret_cast<intptr_t>(fPoints) + oldSize - oldVerbSize);
        memmove(newVerbsDst, oldVerbsSrc, oldVerbSize);
        fVerbs = reinterpret_cast<uint8_t*>(reinterpret_cast<intptr_t>(fPoints) + newSize);
        fFreeSpace += growSize;
        this->validate();
    }

    /**
     * Private, non-const-ptr version of the public function verbsMemBegin().
     */
    uint8_t* verbsMemWritable() {
        this->validate();
        return fVerbs - fVerbCnt;
    }

    /**
     * Gets the total amount of space allocated for verbs, points, and reserve.
     */
    size_t currSize() const {
        return reinterpret_cast<intptr_t>(fVerbs) - reinterpret_cast<intptr_t>(fPoints);
    }

    /**
     * Gets an ID that uniquely identifies the contents of the path ref. If two path refs have the
     * same ID then they have the same verbs and points. However, two path refs may have the same
     * contents but different genIDs. Zero is reserved and means an ID has not yet been determined
     * for the path ref.
     */
    int32_t genID() const {
        SkDEBUGCODE(SkASSERT(!fEditorsAttached));
        if (!fGenerationID) {
            if (0 == fPointCnt && 0 == fVerbCnt) {
                fGenerationID = kEmptyGenID;
            } else {
                static int32_t  gPathRefGenerationID;
                // do a loop in case our global wraps around, as we never want to return a 0 or the
                // empty ID
                do {
                    fGenerationID = sk_atomic_inc(&gPathRefGenerationID) + 1;
                } while (fGenerationID <= kEmptyGenID);
            }
        }
        return fGenerationID;
    }

    void validate() const {
        SkASSERT(static_cast<ptrdiff_t>(fFreeSpace) >= 0);
        SkASSERT(reinterpret_cast<intptr_t>(fVerbs) - reinterpret_cast<intptr_t>(fPoints) >= 0);
        SkASSERT((NULL == fPoints) == (NULL == fVerbs));
        SkASSERT(!(NULL == fPoints && 0 != fFreeSpace));
        SkASSERT(!(NULL == fPoints && 0 != fFreeSpace));
        SkASSERT(!(NULL == fPoints && fPointCnt));
        SkASSERT(!(NULL == fVerbs && fVerbCnt));
        SkASSERT(this->currSize() ==
                 fFreeSpace + sizeof(SkPoint) * fPointCnt + sizeof(uint8_t) * fVerbCnt);
    }

    enum {
        kMinSize = 256,
    };

    SkPoint*            fPoints; // points to begining of the allocation
    uint8_t*            fVerbs; // points just past the end of the allocation (verbs grow backwards)
    int                 fVerbCnt;
    int                 fPointCnt;
    size_t              fFreeSpace; // redundant but saves computation
    enum {
        kEmptyGenID = 1, // GenID reserved for path ref with zero points and zero verbs.
    };
    mutable int32_t     fGenerationID;
    SkDEBUGCODE(int32_t fEditorsAttached;) // assert that only one editor in use at any time.

    typedef SkRefCnt INHERITED;
};

SK_DEFINE_INST_COUNT(SkPathRef);

#endif
