/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathRef_DEFINED
#define SkPathRef_DEFINED

#include "include/core/SkArc.h"
#include "include/core/SkPathTypes.h" // IWYU pragma: keep
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/SkIDChangeListener.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkSpan_impl.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <tuple>

class SkMatrix;

struct SkPathRectInfo {
    SkRect          fRect;
    SkPathDirection fDirection;
    unsigned        fStartIndex;
};

struct SkPathOvalInfo {
    SkRect          fBounds;
    SkPathDirection fDirection;
    unsigned        fStartIndex;
};

struct SkPathRRectInfo {
    SkRRect         fRRect;
    SkPathDirection fDirection;
    unsigned        fStartIndex;
};

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
    // See https://bugs.chromium.org/p/skia/issues/detail?id=13817 for how these sizes were
    // determined.
    using PointsArray = skia_private::STArray<4, SkPoint>;
    using VerbsArray = skia_private::STArray<4, SkPathVerb>;
    using ConicWeightsArray = skia_private::STArray<2, float>;

    enum class PathType : uint8_t {
        kGeneral,
        kOval,
        kRRect,
        kArc,
    };

    SkPathRef(SkSpan<const SkPoint> points, SkSpan<const SkPathVerb> verbs,
              SkSpan<const SkScalar> weights, unsigned segmentMask)
        : fPoints(points)
        , fVerbs(verbs)
        , fConicWeights(weights)
    {
        fBoundsIsDirty = true;    // this also invalidates fIsFinite
        fGenerationID = 0;        // recompute
        fSegmentMask = segmentMask;
        fType = PathType::kGeneral;
        // The next two values don't matter unless fType is kOval or kRRect
        fRRectOrOvalIsCCW = false;
        fRRectOrOvalStartIdx = 0xAC;
        fArcOval.setEmpty();
        fArcStartAngle = fArcSweepAngle = 0.0f;
        fArcType = SkArc::Type::kArc;
        SkDEBUGCODE(fEditorsAttached.store(0);)

        this->computeBounds();  // do this now, before we worry about multiple owners/threads
        SkDEBUGCODE(this->validate();)
    }

    class Editor {
    public:
        Editor(sk_sp<SkPathRef>* pathRef,
               int incReserveVerbs = 0,
               int incReservePoints = 0,
               int incReserveConics = 0);

        ~Editor() { SkDEBUGCODE(fPathRef->fEditorsAttached--;) }

        /**
         * Returns the array of points.
         */
        SkPoint* writablePoints() { return fPathRef->getWritablePoints(); }
        const SkPoint* points() const { return fPathRef->points(); }

        /**
         * Gets the ith point. Shortcut for this->points() + i
         */
        SkPoint* atPoint(int i) { return fPathRef->getWritablePoints() + i; }
        const SkPoint* atPoint(int i) const { return &fPathRef->fPoints[i]; }

        /**
         * Adds the verb and allocates space for the number of points indicated by the verb. The
         * return value is a pointer to where the points for the verb should be written.
         * 'weight' is only used if 'verb' is kConic_Verb
         */
        SkPoint* growForVerb(SkPathVerb verb, SkScalar weight = 0) {
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
        SkPoint* growForRepeatedVerb(SkPathVerb verb,
                                     int numVbs,
                                     SkScalar** weights = nullptr) {
            return fPathRef->growForRepeatedVerb(verb, numVbs, weights);
        }

        /**
         * Concatenates all verbs from 'path' onto the pathRef's verbs array. Increases the point
         * count by the number of points in 'path', and the conic weight count by the number of
         * conics in 'path'.
         *
         * Returns pointers to the uninitialized points and conic weights data.
         */
        std::tuple<SkPoint*, SkScalar*> growForVerbsInPath(const SkPathRef& path) {
            return fPathRef->growForVerbsInPath(path);
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

        void setIsOval(bool isCCW, unsigned start) {
            fPathRef->setIsOval(isCCW, start);
        }

        void setIsRRect(bool isCCW, unsigned start) {
            fPathRef->setIsRRect(isCCW, start);
        }

        void setIsArc(const SkArc& arc) {
            fPathRef->setIsArc(arc);
        }

        void setBounds(const SkRect& rect) { fPathRef->setBounds(rect); }

    private:
        SkPathRef* fPathRef;
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

    /** Returns Info struct if the path is an oval, else return {}.
     *  Tracking whether a path is an oval is considered an
     *  optimization for performance and so some paths that are in
     *  fact ovals can report {}.
     */
    std::optional<SkPathOvalInfo> isOval() const {
        if (fType == PathType::kOval) {
            return {{
                this->getBounds(),
                fRRectOrOvalIsCCW ? SkPathDirection::kCCW : SkPathDirection::kCW,
                fRRectOrOvalStartIdx,
            }};
        }
        return {};
    }

    std::optional<SkPathRRectInfo> isRRect() const;

    std::optional<SkArc> isArc() const {
        if (fType == PathType::kArc) {
            return SkArc::Make(fArcOval, fArcStartAngle, fArcSweepAngle, fArcType);
        }
        return {};
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

  //  static SkPathRef* CreateFromBuffer(SkRBuffer* buffer);

    /**
     * Rollsback a path ref to zero verbs and points with the assumption that the path ref will be
     * repopulated with approximately the same number of verbs and points. A new path ref is created
     * only if necessary.
     */
    static void Rewind(sk_sp<SkPathRef>* pathRef);

    ~SkPathRef();
    int countPoints() const { return fPoints.size(); }
    int countVerbs() const { return fVerbs.size(); }
    int countWeights() const { return fConicWeights.size(); }

    size_t approximateBytesUsed() const;

    /**
     * Returns a pointer one beyond the first logical verb (last verb in memory order).
     */
    const SkPathVerb* verbsBegin() const { return fVerbs.begin(); }

    /**
     * Returns a const pointer to the first verb in memory (which is the last logical verb).
     */
    const SkPathVerb* verbsEnd() const { return fVerbs.end(); }

    SkSpan<const SkPathVerb> verbs() const { return fVerbs; }

    /**
     * Returns a const pointer to the first point.
     */
    const SkPoint* points() const { return fPoints.begin(); }

    /**
     * Shortcut for this->points() + this->countPoints()
     */
    const SkPoint* pointsEnd() const { return this->points() + this->countPoints(); }

    SkSpan<const SkPoint> pointSpan() const { return fPoints; }

    const SkScalar* conicWeights() const { return fConicWeights.begin(); }
    const SkScalar* conicWeightsEnd() const { return fConicWeights.end(); }

    /**
     * Convenience methods for getting to a verb or point by index.
     */
    SkPathVerb atVerb(int index) const { return fVerbs[index]; }
    SkPoint atPoint(int index) const { return fPoints[index]; }

    bool operator== (const SkPathRef& ref) const;

    void interpolate(const SkPathRef& ending, SkScalar weight, SkPathRef* out) const;

    /**
     * Gets an ID that uniquely identifies the contents of the path ref. If two path refs have the
     * same ID then they have the same verbs and points. However, two path refs may have the same
     * contents but different genIDs.
     * skbug.com/40032862 for background on why fillType is necessary (for now).
     */
    uint32_t genID(uint8_t fillType) const;

    void addGenIDChangeListener(sk_sp<SkIDChangeListener>);   // Threadsafe.
    int genIDChangeListenerCount();                           // Threadsafe

    bool dataMatchesVerbs() const;
    bool isValid() const;
    SkDEBUGCODE(void validate() const { SkASSERT(this->isValid()); } )

    /**
     * Resets this SkPathRef to a clean state.
     */
    void reset();

    bool isInitialEmptyPathRef() const {
        return fGenerationID == kEmptyGenID;
    }

private:
    enum SerializationOffsets {
        kLegacyRRectOrOvalStartIdx_SerializationShift = 28, // requires 3 bits, ignored.
        kLegacyRRectOrOvalIsCCW_SerializationShift = 27,    // requires 1 bit, ignored.
        kLegacyIsRRect_SerializationShift = 26,             // requires 1 bit, ignored.
        kIsFinite_SerializationShift = 25,                  // requires 1 bit
        kLegacyIsOval_SerializationShift = 24,              // requires 1 bit, ignored.
        kSegmentMask_SerializationShift = 0                 // requires 4 bits (deprecated)
    };

    SkPathRef(int numVerbs = 0, int numPoints = 0, int numConics = 0) {
        fBoundsIsDirty = true;    // this also invalidates fIsFinite
        fGenerationID = kEmptyGenID;
        fSegmentMask = 0;
        fType = PathType::kGeneral;
        // The next two values don't matter unless fType is kOval or kRRect
        fRRectOrOvalIsCCW = false;
        fRRectOrOvalStartIdx = 0xAC;
        fArcOval.setEmpty();
        fArcStartAngle = fArcSweepAngle = 0.0f;
        fArcType = SkArc::Type::kArc;
        if (numPoints > 0) {
            fPoints.reserve_exact(numPoints);
        }
        if (numVerbs > 0) {
            fVerbs.reserve_exact(numVerbs);
        }
        if (numConics > 0) {
            fConicWeights.reserve_exact(numConics);
        }
        SkDEBUGCODE(fEditorsAttached.store(0);)
        SkDEBUGCODE(this->validate();)
    }

    void copy(const SkPathRef& ref, int additionalReserveVerbs, int additionalReservePoints, int additionalReserveConics);

    // Return true if the computed bounds are finite.
    static bool ComputePtBounds(SkRect* bounds, const SkPathRef& ref) {
        return bounds->setBoundsCheck({ref.points(), ref.countPoints()});
    }

    // called, if dirty, by getBounds()
    void computeBounds() const {
        SkDEBUGCODE(this->validate();)
        // TODO: remove fBoundsIsDirty and fIsFinite,
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
    void incReserve(int additionalVerbs, int additionalPoints, int additionalConics) {
        SkDEBUGCODE(this->validate();)
        // Use reserve() so that if there is not enough space, the array will grow with some
        // additional space. This ensures repeated calls to grow won't always allocate.
        if (additionalPoints > 0) {
            fPoints.reserve(fPoints.size() + additionalPoints);
        }
        if (additionalVerbs > 0) {
            fVerbs.reserve(fVerbs.size() + additionalVerbs);
        }
        if (additionalConics > 0) {
            fConicWeights.reserve(fConicWeights.size() + additionalConics);
        }
        SkDEBUGCODE(this->validate();)
    }

    /**
     * Resets all state except that of the verbs, points, and conic-weights.
     * Intended to be called from other functions that reset state.
     */
    void commonReset() {
        SkDEBUGCODE(this->validate();)
        this->callGenIDChangeListeners();
        fBoundsIsDirty = true;      // this also invalidates fIsFinite
        fGenerationID = 0;

        fSegmentMask = 0;
        fType = PathType::kGeneral;
    }

    /** Resets the path ref with verbCount verbs and pointCount points, all uninitialized. Also
     *  allocates space for reserveVerb additional verbs and reservePoints additional points.*/
    void resetToSize(int verbCount, int pointCount, int conicCount,
                     int reserveVerbs = 0, int reservePoints = 0,
                     int reserveConics = 0) {
        this->commonReset();
        // Use reserve_exact() so the arrays are sized to exactly fit the data.
        fPoints.reserve_exact(pointCount + reservePoints);
        fPoints.resize_back(pointCount);

        fVerbs.reserve_exact(verbCount + reserveVerbs);
        fVerbs.resize_back(verbCount);

        fConicWeights.reserve_exact(conicCount + reserveConics);
        fConicWeights.resize_back(conicCount);
        SkDEBUGCODE(this->validate();)
    }

    /**
     * Increases the verb count by numVbs and point count by the required amount.
     * The new points are uninitialized. All the new verbs are set to the specified
     * verb. If 'verb' is kConic_Verb, 'weights' will return a pointer to the
     * uninitialized conic weights.
     */
    SkPoint* growForRepeatedVerb(SkPathVerb, int numVbs, SkScalar** weights);

    /**
     * Increases the verb count 1, records the new verb, and creates room for the requisite number
     * of additional points. A pointer to the first point is returned. Any new points are
     * uninitialized.
     */
    SkPoint* growForVerb(SkPathVerb, SkScalar weight);

    /**
     * Concatenates all verbs from 'path' onto our own verbs array. Increases the point count by the
     * number of points in 'path', and the conic weight count by the number of conics in 'path'.
     *
     * Returns pointers to the uninitialized points and conic weights data.
     */
    std::tuple<SkPoint*, SkScalar*> growForVerbsInPath(const SkPathRef& path);

    /**
     * Private, non-const-ptr version of the public function verbsMemBegin().
     */
    uint8_t* verbsBeginWritable() { return (uint8_t*)fVerbs.begin(); }

    /**
     * Called the first time someone calls CreateEmpty to actually create the singleton.
     */
    friend SkPathRef* sk_create_empty_pathref();

    void setIsOval(bool isCCW, unsigned start) {
        fType = PathType::kOval;
        fRRectOrOvalIsCCW = isCCW;
        fRRectOrOvalStartIdx = SkToU8(start);
    }

    void setIsRRect(bool isCCW, unsigned start) {
        fType = PathType::kRRect;
        fRRectOrOvalIsCCW = isCCW;
        fRRectOrOvalStartIdx = SkToU8(start);
    }

    void setIsArc(const SkArc& arc) {
        fType = PathType::kArc;
        fArcOval = arc.fOval;
        fArcStartAngle = arc.fStartAngle;
        fArcSweepAngle = arc.fSweepAngle;
        fArcType = arc.fType;
    }

    // called only by the editor. Note that this is not a const function.
    SkPoint* getWritablePoints() {
        SkDEBUGCODE(this->validate();)
        fType = PathType::kGeneral;
        return fPoints.begin();
    }

    const SkPoint* getPoints() const {
        SkDEBUGCODE(this->validate();)
        return fPoints.begin();
    }

    void callGenIDChangeListeners();

    PointsArray fPoints;
    VerbsArray fVerbs;
    ConicWeightsArray fConicWeights;

    mutable SkRect   fBounds;
    SkRect           fArcOval;

    enum {
        kEmptyGenID = 1, // GenID reserved for path ref with zero points and zero verbs.
    };
    mutable uint32_t    fGenerationID;
    SkIDChangeListener::List fGenIDChangeListeners;

    SkDEBUGCODE(std::atomic<int> fEditorsAttached;) // assert only one editor in use at any time.

    SkScalar    fArcStartAngle;
    SkScalar    fArcSweepAngle;

    PathType fType;

    mutable uint8_t  fBoundsIsDirty;

    uint8_t  fRRectOrOvalStartIdx;
    uint8_t  fSegmentMask;
    // If the path is an arc, these four variables store that information.
    // We should just store an SkArc, but alignment would cost us 8 more bytes.
    SkArc::Type fArcType;

    mutable bool     fIsFinite;    // only meaningful if bounds are valid
    // Both the circle and rrect special cases have a notion of direction and starting point
    // The next two variables store that information for either.
    bool     fRRectOrOvalIsCCW;

    friend class PathRefTest_Private;
    friend class ForceIsRRect_Private; // unit test isRRect
    friend class SkPath;
    friend class SkPathBuilder;
    friend class SkPathPriv;
};

#endif
