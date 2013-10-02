
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathRef_DEFINED
#define SkPathRef_DEFINED

#include "SkMatrix.h"
#include "SkPoint.h"
#include "SkRect.h"
#include "SkRefCnt.h"
#include "SkTDArray.h"
#include <stddef.h> // ptrdiff_t

class SkRBuffer;
class SkWBuffer;

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

class SK_API SkPathRef : public ::SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkPathRef);

    class Editor {
    public:
        Editor(SkAutoTUnref<SkPathRef>* pathRef,
               int incReserveVerbs = 0,
               int incReservePoints = 0);

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
        SkPoint* growForVerb(int /*SkPath::Verb*/ verb) {
            SkDEBUGCODE(fPathRef->validate();)
            return fPathRef->growForVerb(verb);
        }

        SkPoint* growForConic(SkScalar w);

        /**
         * Allocates space for additional verbs and points and returns pointers to the new verbs and
         * points. verbs will point one beyond the first new verb (index it using [~<i>]). pts points
         * at the first new point (indexed normally [<i>]).
         */
        void grow(int newVerbs, int newPts, uint8_t** verbs, SkPoint** pts) {
            SkASSERT(NULL != verbs);
            SkASSERT(NULL != pts);
            SkDEBUGCODE(fPathRef->validate();)
            int oldVerbCnt = fPathRef->fVerbCnt;
            int oldPointCnt = fPathRef->fPointCnt;
            SkASSERT(verbs && pts);
            fPathRef->grow(newVerbs, newPts);
            *verbs = fPathRef->fVerbs - oldVerbCnt;
            *pts = fPathRef->fPoints + oldPointCnt;
            SkDEBUGCODE(fPathRef->validate();)
        }

        /**
         * Resets the path ref to a new verb and point count. The new verbs and points are
         * uninitialized.
         */
        void resetToSize(int newVerbCnt, int newPointCnt, int newConicCount) {
            fPathRef->resetToSize(newVerbCnt, newPointCnt, newConicCount);
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
        static SkPathRef* gEmptyPathRef;
        if (!gEmptyPathRef) {
            gEmptyPathRef = SkNEW(SkPathRef); // leak!
            gEmptyPathRef->computeBounds();   // Premptively avoid a race to clear fBoundsIsDirty.
        }
        return SkRef(gEmptyPathRef);
    }

    /**
     *  Returns true if all of the points in this path are finite, meaning there
     *  are no infinities and no NaNs.
     */
    bool isFinite() const {
        if (fBoundsIsDirty) {
            this->computeBounds();
        }
        return SkToBool(fIsFinite);
    }

    bool hasComputedBounds() const {
        return !fBoundsIsDirty;
    }

    /** Returns the bounds of the path's points. If the path contains 0 or 1
        points, the bounds is set to (0,0,0,0), and isEmpty() will return true.
        Note: this bounds may be larger than the actual shape, since curves
        do not extend as far as their control points.
    */
    const SkRect& getBounds() const {
        if (fBoundsIsDirty) {
            this->computeBounds();
        }
        return fBounds;
    }

    void setBounds(const SkRect& rect) {
        SkASSERT(rect.fLeft <= rect.fRight && rect.fTop <= rect.fBottom);
        fBounds = rect;
        fBoundsIsDirty = false;
        fIsFinite = fBounds.isFinite();
    }

    /**
     * Transforms a path ref by a matrix, allocating a new one only if necessary.
     */
    static void CreateTransformedCopy(SkAutoTUnref<SkPathRef>* dst,
                                      const SkPathRef& src,
                                      const SkMatrix& matrix);

    static SkPathRef* CreateFromBuffer(SkRBuffer* buffer
#ifndef DELETE_THIS_CODE_WHEN_SKPS_ARE_REBUILT_AT_V14_AND_ALL_OTHER_INSTANCES_TOO
        , bool newFormat, int32_t oldPacked
#endif
        );

    /**
     * Rollsback a path ref to zero verbs and points with the assumption that the path ref will be
     * repopulated with approximately the same number of verbs and points. A new path ref is created
     * only if necessary.
     */
    static void Rewind(SkAutoTUnref<SkPathRef>* pathRef);

    virtual ~SkPathRef() {
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

    int countPoints() const { SkDEBUGCODE(this->validate();) return fPointCnt; }
    int countVerbs() const { SkDEBUGCODE(this->validate();) return fVerbCnt; }

    /**
     * Returns a pointer one beyond the first logical verb (last verb in memory order).
     */
    const uint8_t* verbs() const { SkDEBUGCODE(this->validate();) return fVerbs; }

    /**
     * Returns a const pointer to the first verb in memory (which is the last logical verb).
     */
    const uint8_t* verbsMemBegin() const { return this->verbs() - fVerbCnt; }

    /**
     * Returns a const pointer to the first point.
     */
    const SkPoint* points() const { SkDEBUGCODE(this->validate();) return fPoints; }

    /**
     * Shortcut for this->points() + this->countPoints()
     */
    const SkPoint* pointsEnd() const { return this->points() + this->countPoints(); }

    const SkScalar* conicWeights() const { SkDEBUGCODE(this->validate();) return fConicWeights.begin(); }
    const SkScalar* conicWeightsEnd() const { SkDEBUGCODE(this->validate();) return fConicWeights.end(); }

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

    bool operator== (const SkPathRef& ref) const;

    /**
     * Writes the path points and verbs to a buffer.
     */
    void writeToBuffer(SkWBuffer* buffer);

    /**
     * Gets the number of bytes that would be written in writeBuffer()
     */
    uint32_t writeSize();

private:
    enum SerializationOffsets {
        kIsFinite_SerializationShift = 25,  // requires 1 bit
    };

    SkPathRef() {
        fBoundsIsDirty = true;    // this also invalidates fIsFinite
        fPointCnt = 0;
        fVerbCnt = 0;
        fVerbs = NULL;
        fPoints = NULL;
        fFreeSpace = 0;
        fGenerationID = kEmptyGenID;
        SkDEBUGCODE(fEditorsAttached = 0;)
        SkDEBUGCODE(this->validate();)
    }

    void copy(const SkPathRef& ref, int additionalReserveVerbs, int additionalReservePoints);

    // Return true if the computed bounds are finite.
    static bool ComputePtBounds(SkRect* bounds, const SkPathRef& ref) {
        int count = ref.countPoints();
        if (count <= 1) {  // we ignore just 1 point (moveto)
            bounds->setEmpty();
            return count ? ref.points()->isFinite() : true;
        } else {
            return bounds->setBoundsCheck(ref.points(), count);
        }
    }

    // called, if dirty, by getBounds()
    void computeBounds() const {
        SkDEBUGCODE(this->validate();)
        SkASSERT(fBoundsIsDirty);

        fIsFinite = ComputePtBounds(&fBounds, *this);
        fBoundsIsDirty = false;
    }

    /** Makes additional room but does not change the counts or change the genID */
    void incReserve(int additionalVerbs, int additionalPoints) {
        SkDEBUGCODE(this->validate();)
        size_t space = additionalVerbs * sizeof(uint8_t) + additionalPoints * sizeof (SkPoint);
        this->makeSpace(space);
        SkDEBUGCODE(this->validate();)
    }

    /** Resets the path ref with verbCount verbs and pointCount points, all uninitialized. Also
     *  allocates space for reserveVerb additional verbs and reservePoints additional points.*/
    void resetToSize(int verbCount, int pointCount, int conicCount,
                     int reserveVerbs = 0, int reservePoints = 0) {
        SkDEBUGCODE(this->validate();)
        fBoundsIsDirty = true;      // this also invalidates fIsFinite
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
        fConicWeights.setCount(conicCount);
        SkDEBUGCODE(this->validate();)
    }

    /**
     * Increases the verb count by newVerbs and the point count be newPoints. New verbs and points
     * are uninitialized.
     */
    void grow(int newVerbs, int newPoints) {
        SkDEBUGCODE(this->validate();)
        size_t space = newVerbs * sizeof(uint8_t) + newPoints * sizeof (SkPoint);
        this->makeSpace(space);
        fVerbCnt += newVerbs;
        fPointCnt += newPoints;
        fFreeSpace -= space;
        fBoundsIsDirty = true;  // this also invalidates fIsFinite
        SkDEBUGCODE(this->validate();)
    }

    /**
     * Increases the verb count 1, records the new verb, and creates room for the requisite number
     * of additional points. A pointer to the first point is returned. Any new points are
     * uninitialized.
     */
    SkPoint* growForVerb(int /*SkPath::Verb*/ verb);

    /**
     * Ensures that the free space available in the path ref is >= size. The verb and point counts
     * are not changed.
     */
    void makeSpace(size_t size) {
        SkDEBUGCODE(this->validate();)
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
        SkDEBUGCODE(this->validate();)
    }

    /**
     * Private, non-const-ptr version of the public function verbsMemBegin().
     */
    uint8_t* verbsMemWritable() {
        SkDEBUGCODE(this->validate();)
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
    int32_t genID() const;

    SkDEBUGCODE(void validate() const;)

    enum {
        kMinSize = 256,
    };

    mutable SkRect      fBounds;
    mutable uint8_t     fBoundsIsDirty;
    mutable SkBool8     fIsFinite;    // only meaningful if bounds are valid

    SkPoint*            fPoints; // points to begining of the allocation
    uint8_t*            fVerbs; // points just past the end of the allocation (verbs grow backwards)
    int                 fVerbCnt;
    int                 fPointCnt;
    size_t              fFreeSpace; // redundant but saves computation
    SkTDArray<SkScalar> fConicWeights;

    enum {
        kEmptyGenID = 1, // GenID reserved for path ref with zero points and zero verbs.
    };
    mutable int32_t     fGenerationID;
    SkDEBUGCODE(int32_t fEditorsAttached;) // assert that only one editor in use at any time.

    typedef SkRefCnt INHERITED;
};

#endif
