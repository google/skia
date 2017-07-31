
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathRef_DEFINED
#define SkPathRef_DEFINED

#include "../private/SkAtomics.h"
#include "../private/SkTDArray.h"
#include "SkMatrix.h"
#include "SkPoint.h"
#include "SkRRect.h"
#include "SkRect.h"
#include "SkRefCnt.h"
#include "SkTemplates.h"

class SkRBuffer;
class SkWBuffer;

/**
 * Holds the path verbs and points. It is versioned by a generation ID. None of its public methods
 * modify the contents. To modify or append to the verbs/points wrap the SkPathRef in an
 * SkPathRef::Editor object. Installing the editor resets the generation ID. It also performs
 * copy-on-write if the SkPathRef is shared by multiple SkPaths. The caller passes the Editor's
 * constructor a pointer to a sk_sp<SkPathRef>, which may be updated to point to a new SkPathRef
 * after the editor's constructor returns.
 *
 * The points and verbs are stored in a single allocation. The points are at the begining of the
 * allocation while the verbs are stored at end of the allocation, in reverse order. Thus the points
 * and verbs both grow into the middle of the allocation until the meet. To access verb i in the
 * verb array use ref.verbs()[~i] (because verbs() returns a pointer just beyond the first
 * logical verb or the last verb in memory).
 */

class SK_API SkPathRef final : public SkNVRefCnt<SkPathRef> {
public:
    class Editor {
    public:
        Editor(sk_sp<SkPathRef>* pathRef,
               int incReserveVerbs = 0,
               int incReservePoints = 0);

        ~Editor() { SkDEBUGCODE(sk_atomic_dec(&fPathRef->fEditorsAttached);) }

        /**
         * Returns the array of points.
         */
        SkPoint* points() { return fPathRef->getPoints(); }
        const SkPoint* points() const { return fPathRef->points(); }

        /**
         * Gets the ith point. Shortcut for this->points() + i
         */
        SkPoint* atPoint(int i) {
            SkASSERT((unsigned) i < (unsigned) fPathRef->fPointCnt);
            return this->points() + i;
        }
        const SkPoint* atPoint(int i) const {
            SkASSERT((unsigned) i < (unsigned) fPathRef->fPointCnt);
            return this->points() + i;
        }

        /**
         * Adds the verb and allocates space for the number of points indicated by the verb. The
         * return value is a pointer to where the points for the verb should be written.
         * 'weight' is only used if 'verb' is kConic_Verb
         */
        SkPoint* growForVerb(int /*SkPath::Verb*/ verb, SkScalar weight = 0) {
            SkDEBUGCODE(fPathRef->validate();)
            return fPathRef->growForVerb(verb, weight);
        }

        /**
         * Allocates space for multiple instances of a particular verb and the
         * requisite points & weights.
         * The return pointer points at the first new point (indexed normally [<i>]).
         * If 'verb' is kConic_Verb, 'weights' will return a pointer to the
         * space for the conic weights (indexed normally).
         */
        SkPoint* growForRepeatedVerb(int /*SkPath::Verb*/ verb,
                                     int numVbs,
                                     SkScalar** weights = NULL) {
            return fPathRef->growForRepeatedVerb(verb, numVbs, weights);
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

        void setIsOval(bool isOval, bool isCCW, unsigned start) {
            fPathRef->setIsOval(isOval, isCCW, start);
        }

        void setIsRRect(bool isRRect, bool isCCW, unsigned start) {
            fPathRef->setIsRRect(isRRect, isCCW, start);
        }

        void setBounds(const SkRect& rect) { fPathRef->setBounds(rect); }

    private:
        SkPathRef* fPathRef;
    };

    class SK_API Iter {
    public:
        Iter();
        Iter(const SkPathRef&);

        void setPathRef(const SkPathRef&);

        /** Return the next verb in this iteration of the path. When all
            segments have been visited, return kDone_Verb.

            @param  pts The points representing the current verb and/or segment
                        This must not be NULL.
            @return The verb for the current segment
        */
        uint8_t next(SkPoint pts[4]);
        uint8_t peek() const;

        SkScalar conicWeight() const { return *fConicWeights; }

    private:
        const SkPoint*  fPts;
        const uint8_t*  fVerbs;
        const uint8_t*  fVerbStop;
        const SkScalar* fConicWeights;
    };

public:
    /**
     * Gets a path ref with no verbs or points.
     */
    static SkPathRef* CreateEmpty();

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

    /**
     *  Returns a mask, where each bit corresponding to a SegmentMask is
     *  set if the path contains 1 or more segments of that type.
     *  Returns 0 for an empty path (no segments).
     */
    uint32_t getSegmentMasks() const { return fSegmentMask; }

    /** Returns true if the path is an oval.
     *
     * @param rect      returns the bounding rect of this oval. It's a circle
     *                  if the height and width are the same.
     * @param isCCW     is the oval CCW (or CW if false).
     * @param start     indicates where the contour starts on the oval (see
     *                  SkPath::addOval for intepretation of the index).
     *
     * @return true if this path is an oval.
     *              Tracking whether a path is an oval is considered an
     *              optimization for performance and so some paths that are in
     *              fact ovals can report false.
     */
    bool isOval(SkRect* rect, bool* isCCW, unsigned* start) const {
        if (fIsOval) {
            if (rect) {
                *rect = this->getBounds();
            }
            if (isCCW) {
                *isCCW = SkToBool(fRRectOrOvalIsCCW);
            }
            if (start) {
                *start = fRRectOrOvalStartIdx;
            }
        }

        return SkToBool(fIsOval);
    }

    bool isRRect(SkRRect* rrect, bool* isCCW, unsigned* start) const {
        if (fIsRRect) {
            if (rrect) {
                *rrect = this->getRRect();
            }
            if (isCCW) {
                *isCCW = SkToBool(fRRectOrOvalIsCCW);
            }
            if (start) {
                *start = fRRectOrOvalStartIdx;
            }
        }
        return SkToBool(fIsRRect);
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

    SkRRect getRRect() const;

    /**
     * Transforms a path ref by a matrix, allocating a new one only if necessary.
     */
    static void CreateTransformedCopy(sk_sp<SkPathRef>* dst,
                                      const SkPathRef& src,
                                      const SkMatrix& matrix);

    static SkPathRef* CreateFromBuffer(SkRBuffer* buffer);

    /**
     * Rollsback a path ref to zero verbs and points with the assumption that the path ref will be
     * repopulated with approximately the same number of verbs and points. A new path ref is created
     * only if necessary.
     */
    static void Rewind(sk_sp<SkPathRef>* pathRef);

    ~SkPathRef();
    int countPoints() const { SkDEBUGCODE(this->validate();) return fPointCnt; }
    int countVerbs() const { SkDEBUGCODE(this->validate();) return fVerbCnt; }
    int countWeights() const { SkDEBUGCODE(this->validate();) return fConicWeights.count(); }

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
    uint8_t atVerb(int index) const {
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
    void writeToBuffer(SkWBuffer* buffer) const;

    /**
     * Gets the number of bytes that would be written in writeBuffer()
     */
    uint32_t writeSize() const;

    void interpolate(const SkPathRef& ending, SkScalar weight, SkPathRef* out) const;

    /**
     * Gets an ID that uniquely identifies the contents of the path ref. If two path refs have the
     * same ID then they have the same verbs and points. However, two path refs may have the same
     * contents but different genIDs.
     */
    uint32_t genID() const;

    struct GenIDChangeListener {
        virtual ~GenIDChangeListener() {}
        virtual void onChange() = 0;
    };

    void addGenIDChangeListener(GenIDChangeListener* listener);

    SkDEBUGCODE(void validate() const;)

private:
    enum SerializationOffsets {
        kRRectOrOvalStartIdx_SerializationShift = 28,  // requires 3 bits
        kRRectOrOvalIsCCW_SerializationShift = 27,     // requires 1 bit
        kIsRRect_SerializationShift = 26,              // requires 1 bit
        kIsFinite_SerializationShift = 25,             // requires 1 bit
        kIsOval_SerializationShift = 24,               // requires 1 bit
        kSegmentMask_SerializationShift = 0            // requires 4 bits
    };

    SkPathRef() {
        fBoundsIsDirty = true;    // this also invalidates fIsFinite
        fPointCnt = 0;
        fVerbCnt = 0;
        fVerbs = NULL;
        fPoints = NULL;
        fFreeSpace = 0;
        fGenerationID = kEmptyGenID;
        fSegmentMask = 0;
        fIsOval = false;
        fIsRRect = false;
        // The next two values don't matter unless fIsOval or fIsRRect are true.
        fRRectOrOvalIsCCW = false;
        fRRectOrOvalStartIdx = 0xAC;
        SkDEBUGCODE(fEditorsAttached = 0;)
        SkDEBUGCODE(this->validate();)
    }

    void copy(const SkPathRef& ref, int additionalReserveVerbs, int additionalReservePoints);

    // Return true if the computed bounds are finite.
    static bool ComputePtBounds(SkRect* bounds, const SkPathRef& ref) {
        return bounds->setBoundsCheck(ref.points(), ref.countPoints());
    }

    // called, if dirty, by getBounds()
    void computeBounds() const {
        SkDEBUGCODE(this->validate();)
        // TODO(mtklein): remove fBoundsIsDirty and fIsFinite,
        // using an inverted rect instead of fBoundsIsDirty and always recalculating fIsFinite.
        SkASSERT(fBoundsIsDirty);

        fIsFinite = ComputePtBounds(&fBounds, *this);
        fBoundsIsDirty = false;
    }

    void setBounds(const SkRect& rect) {
        SkASSERT(rect.fLeft <= rect.fRight && rect.fTop <= rect.fBottom);
        fBounds = rect;
        fBoundsIsDirty = false;
        fIsFinite = fBounds.isFinite();
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

        fSegmentMask = 0;
        fIsOval = false;
        fIsRRect = false;

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
     * Increases the verb count by numVbs and point count by the required amount.
     * The new points are uninitialized. All the new verbs are set to the specified
     * verb. If 'verb' is kConic_Verb, 'weights' will return a pointer to the
     * uninitialized conic weights.
     */
    SkPoint* growForRepeatedVerb(int /*SkPath::Verb*/ verb, int numVbs, SkScalar** weights);

    /**
     * Increases the verb count 1, records the new verb, and creates room for the requisite number
     * of additional points. A pointer to the first point is returned. Any new points are
     * uninitialized.
     */
    SkPoint* growForVerb(int /*SkPath::Verb*/ verb, SkScalar weight);

    /**
     * Ensures that the free space available in the path ref is >= size. The verb and point counts
     * are not changed.
     */
    void makeSpace(size_t size) {
        SkDEBUGCODE(this->validate();)
        if (size <= fFreeSpace) {
            return;
        }
        size_t growSize = size - fFreeSpace;
        size_t oldSize = this->currSize();
        // round to next multiple of 8 bytes
        growSize = (growSize + 7) & ~static_cast<size_t>(7);
        // we always at least double the allocation
        if (growSize < oldSize) {
            growSize = oldSize;
        }
        if (growSize < kMinSize) {
            growSize = kMinSize;
        }
        constexpr size_t maxSize = std::numeric_limits<size_t>::max();
        size_t newSize;
        if (growSize <= maxSize - oldSize) {
            newSize = oldSize + growSize;
        } else {
            SK_ABORT("Path too big.");
        }
        // Note that realloc could memcpy more than we need. It seems to be a win anyway. TODO:
        // encapsulate this.
        fPoints = reinterpret_cast<SkPoint*>(sk_realloc_throw(fPoints, newSize));
        size_t oldVerbSize = fVerbCnt * sizeof(uint8_t);
        void* newVerbsDst = SkTAddOffset<void>(fPoints, newSize - oldVerbSize);
        void* oldVerbsSrc = SkTAddOffset<void>(fPoints, oldSize - oldVerbSize);
        memmove(newVerbsDst, oldVerbsSrc, oldVerbSize);
        fVerbs = SkTAddOffset<uint8_t>(fPoints, newSize);
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
     * Called the first time someone calls CreateEmpty to actually create the singleton.
     */
    friend SkPathRef* sk_create_empty_pathref();

    void setIsOval(bool isOval, bool isCCW, unsigned start) {
        fIsOval = isOval;
        fRRectOrOvalIsCCW = isCCW;
        fRRectOrOvalStartIdx = start;
    }

    void setIsRRect(bool isRRect, bool isCCW, unsigned start) {
        fIsRRect = isRRect;
        fRRectOrOvalIsCCW = isCCW;
        fRRectOrOvalStartIdx = start;
    }

    // called only by the editor. Note that this is not a const function.
    SkPoint* getPoints() {
        SkDEBUGCODE(this->validate();)
        fIsOval = false;
        fIsRRect = false;
        return fPoints;
    }

    const SkPoint* getPoints() const {
        SkDEBUGCODE(this->validate();)
        return fPoints;
    }

    void callGenIDChangeListeners();

    enum {
        kMinSize = 256,
    };

    mutable SkRect   fBounds;

    SkPoint*            fPoints; // points to begining of the allocation
    uint8_t*            fVerbs; // points just past the end of the allocation (verbs grow backwards)
    int                 fVerbCnt;
    int                 fPointCnt;
    size_t              fFreeSpace; // redundant but saves computation
    SkTDArray<SkScalar> fConicWeights;

    enum {
        kEmptyGenID = 1, // GenID reserved for path ref with zero points and zero verbs.
    };
    mutable uint32_t    fGenerationID;
    SkDEBUGCODE(int32_t fEditorsAttached;) // assert that only one editor in use at any time.

    SkTDArray<GenIDChangeListener*> fGenIDChangeListeners;  // pointers are owned

    mutable uint8_t  fBoundsIsDirty;
    mutable SkBool8  fIsFinite;    // only meaningful if bounds are valid

    SkBool8  fIsOval;
    SkBool8  fIsRRect;
    // Both the circle and rrect special cases have a notion of direction and starting point
    // The next two variables store that information for either.
    SkBool8  fRRectOrOvalIsCCW;
    uint8_t  fRRectOrOvalStartIdx;
    uint8_t  fSegmentMask;

    friend class PathRefTest_Private;
    friend class ForceIsRRect_Private; // unit test isRRect
};

#endif
