/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathPriv_DEFINED
#define SkPathPriv_DEFINED

#include "include/core/SkPath.h"

class SkPathPriv {
public:
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    static const int kPathRefGenIDBitCnt = 30; // leave room for the fill type (skbug.com/1762)
#else
    static const int kPathRefGenIDBitCnt = 32;
#endif

    enum FirstDirection : int {
        kCW_FirstDirection,         // == SkPath::kCW_Direction
        kCCW_FirstDirection,        // == SkPath::kCCW_Direction
        kUnknown_FirstDirection,
    };

    static FirstDirection AsFirstDirection(SkPath::Direction dir) {
        // since we agree numerically for the values in Direction, we can just cast.
        return (FirstDirection)dir;
    }

    /**
     *  Return the opposite of the specified direction. kUnknown is its own
     *  opposite.
     */
    static FirstDirection OppositeFirstDirection(FirstDirection dir) {
        static const FirstDirection gOppositeDir[] = {
            kCCW_FirstDirection, kCW_FirstDirection, kUnknown_FirstDirection,
        };
        return gOppositeDir[dir];
    }

    /**
     *  Tries to quickly compute the direction of the first non-degenerate
     *  contour. If it can be computed, return true and set dir to that
     *  direction. If it cannot be (quickly) determined, return false and ignore
     *  the dir parameter. If the direction was determined, it is cached to make
     *  subsequent calls return quickly.
     */
    static bool CheapComputeFirstDirection(const SkPath&, FirstDirection* dir);

    /**
     *  Returns true if the path's direction can be computed via
     *  cheapComputDirection() and if that computed direction matches the
     *  specified direction. If dir is kUnknown, returns true if the direction
     *  cannot be computed.
     */
    static bool CheapIsFirstDirection(const SkPath& path, FirstDirection dir) {
        FirstDirection computedDir = kUnknown_FirstDirection;
        (void)CheapComputeFirstDirection(path, &computedDir);
        return computedDir == dir;
    }

    static bool IsClosedSingleContour(const SkPath& path) {
        int verbCount = path.countVerbs();
        if (verbCount == 0)
            return false;
        int moveCount = 0;
        auto verbs = path.fPathRef->verbsBegin();
        for (int i = 0; i < verbCount; i++) {
            switch (verbs[i]) {
                case SkPath::Verb::kMove_Verb:
                    moveCount += 1;
                    if (moveCount > 1) {
                        return false;
                    }
                    break;
                case SkPath::Verb::kClose_Verb:
                    if (i == verbCount - 1) {
                        return true;
                    }
                    return false;
                default: break;
            }
        }
        return false;
    }

    static void AddGenIDChangeListener(const SkPath& path,
                                       sk_sp<SkPathRef::GenIDChangeListener> listener) {
        path.fPathRef->addGenIDChangeListener(std::move(listener));
    }

    /**
     * This returns true for a rect that begins and ends at the same corner and has either a move
     * followed by four lines or a move followed by 3 lines and a close. None of the parameters are
     * optional. This does not permit degenerate line or point rectangles.
     */
    static bool IsSimpleClosedRect(const SkPath& path, SkRect* rect, SkPath::Direction* direction,
                                   unsigned* start);

    /**
     * Creates a path from arc params using the semantics of SkCanvas::drawArc. This function
     * assumes empty ovals and zero sweeps have already been filtered out.
     */
    static void CreateDrawArcPath(SkPath* path, const SkRect& oval, SkScalar startAngle,
                                  SkScalar sweepAngle, bool useCenter, bool isFillNoPathEffect);

    /**
     * Determines whether an arc produced by CreateDrawArcPath will be convex. Assumes a non-empty
     * oval.
     */
    static bool DrawArcIsConvex(SkScalar sweepAngle, bool useCenter, bool isFillNoPathEffect);

    /**
     * Returns a C++11-iterable object that traverses a path's verbs in order. e.g:
     *
     *   for (SkPath::Verb verb : SkPathPriv::Verbs(path)) {
     *       ...
     *   }
     */
    struct Verbs {
    public:
        Verbs(const SkPath& path) : fPathRef(path.fPathRef.get()) {}
        struct Iter {
            void operator++() { fVerb++; }
            bool operator!=(const Iter& b) { return fVerb != b.fVerb; }
            SkPath::Verb operator*() { return static_cast<SkPath::Verb>(*fVerb); }
            const uint8_t* fVerb;
        };
        Iter begin() { return Iter{fPathRef->verbsBegin()}; }
        Iter end() { return Iter{fPathRef->verbsEnd()}; }
    private:
        Verbs(const Verbs&) = delete;
        Verbs& operator=(const Verbs&) = delete;
        SkPathRef* fPathRef;
    };

    /**
     * Returns a pointer to the verb data.
     */
    static const uint8_t* VerbData(const SkPath& path) {
        return path.fPathRef->verbsBegin();
    }

    /** Returns a raw pointer to the path points */
    static const SkPoint* PointData(const SkPath& path) {
        return path.fPathRef->points();
    }

    /** Returns the number of conic weights in the path */
    static int ConicWeightCnt(const SkPath& path) {
        return path.fPathRef->countWeights();
    }

    /** Returns a raw pointer to the path conic weights. */
    static const SkScalar* ConicWeightData(const SkPath& path) {
        return path.fPathRef->conicWeights();
    }

#ifndef SK_LEGACY_PATH_CONVEXITY
    /** Returns true if path formed by pts is convex.

        @param pts    SkPoint array of path
        @param count  number of entries in array

        @return       true if pts represent a convex geometry
    */
    static bool IsConvex(const SkPoint pts[], int count);
#endif

    /** Returns true if the underlying SkPathRef has one single owner. */
    static bool TestingOnly_unique(const SkPath& path) {
        return path.fPathRef->unique();
    }

    /** Returns true if constructed by addCircle(), addOval(); and in some cases,
     addRoundRect(), addRRect(). SkPath constructed with conicTo() or rConicTo() will not
     return true though SkPath draws oval.

     rect receives bounds of oval.
     dir receives SkPath::Direction of oval: kCW_Direction if clockwise, kCCW_Direction if
     counterclockwise.
     start receives start of oval: 0 for top, 1 for right, 2 for bottom, 3 for left.

     rect, dir, and start are unmodified if oval is not found.

     Triggers performance optimizations on some GPU surface implementations.

     @param rect   storage for bounding SkRect of oval; may be nullptr
     @param dir    storage for SkPath::Direction; may be nullptr
     @param start  storage for start of oval; may be nullptr
     @return       true if SkPath was constructed by method that reduces to oval
     */
    static bool IsOval(const SkPath& path, SkRect* rect, SkPath::Direction* dir, unsigned* start) {
        bool isCCW = false;
        bool result = path.fPathRef->isOval(rect, &isCCW, start);
        if (dir && result) {
            *dir = isCCW ? SkPath::kCCW_Direction : SkPath::kCW_Direction;
        }
        return result;
    }

    /** Returns true if constructed by addRoundRect(), addRRect(); and if construction
     is not empty, not SkRect, and not oval. SkPath constructed with other calls
     will not return true though SkPath draws SkRRect.

     rrect receives bounds of SkRRect.
     dir receives SkPath::Direction of oval: kCW_Direction if clockwise, kCCW_Direction if
     counterclockwise.
     start receives start of SkRRect: 0 for top, 1 for right, 2 for bottom, 3 for left.

     rrect, dir, and start are unmodified if SkRRect is not found.

     Triggers performance optimizations on some GPU surface implementations.

     @param rrect  storage for bounding SkRect of SkRRect; may be nullptr
     @param dir    storage for SkPath::Direction; may be nullptr
     @param start  storage for start of SkRRect; may be nullptr
     @return       true if SkPath contains only SkRRect
     */
    static bool IsRRect(const SkPath& path, SkRRect* rrect, SkPath::Direction* dir,
                        unsigned* start) {
        bool isCCW = false;
        bool result = path.fPathRef->isRRect(rrect, &isCCW, start);
        if (dir && result) {
            *dir = isCCW ? SkPath::kCCW_Direction : SkPath::kCW_Direction;
        }
        return result;
    }

    /**
     *  Sometimes in the drawing pipeline, we have to perform math on path coordinates, even after
     *  the path is in device-coordinates. Tessellation and clipping are two examples. Usually this
     *  is pretty modest, but it can involve subtracting/adding coordinates, or multiplying by
     *  small constants (e.g. 2,3,4). To try to preflight issues where these optionations could turn
     *  finite path values into infinities (or NaNs), we allow the upper drawing code to reject
     *  the path if its bounds (in device coordinates) is too close to max float.
     */
    static bool TooBigForMath(const SkRect& bounds) {
        // This value is just a guess. smaller is safer, but we don't want to reject largish paths
        // that we don't have to.
        constexpr SkScalar scale_down_to_allow_for_small_multiplies = 0.25f;
        constexpr SkScalar max = SK_ScalarMax * scale_down_to_allow_for_small_multiplies;

        // use ! expression so we return true if bounds contains NaN
        return !(bounds.fLeft >= -max && bounds.fTop >= -max &&
                 bounds.fRight <= max && bounds.fBottom <= max);
    }
    static bool TooBigForMath(const SkPath& path) {
        return TooBigForMath(path.getBounds());
    }

    // Returns number of valid points for each SkPath::Iter verb
    static int PtsInIter(unsigned verb) {
        static const uint8_t gPtsInVerb[] = {
            1,  // kMove    pts[0]
            2,  // kLine    pts[0..1]
            3,  // kQuad    pts[0..2]
            3,  // kConic   pts[0..2]
            4,  // kCubic   pts[0..3]
            0,  // kClose
            0   // kDone
        };

        SkASSERT(verb < SK_ARRAY_COUNT(gPtsInVerb));
        return gPtsInVerb[verb];
    }

    static bool IsAxisAligned(const SkPath& path) {
        SkRect tmp;
        return (path.fPathRef->fIsRRect | path.fPathRef->fIsOval) || path.isRect(&tmp);
    }

    static bool AllPointsEq(const SkPoint pts[], int count) {
        for (int i = 1; i < count; ++i) {
            if (pts[0] != pts[i]) {
                return false;
            }
        }
        return true;
    }
};

// Lightweight variant of SkPath::Iter that only returns segments (e.g. lines/conics).
// Does not return kMove or kClose.
// Always "auto-closes" each contour.
// Roughly the same as SkPath::Iter(path, true), but does not return moves or closes
//
class SkPathEdgeIter {
    const uint8_t*  fVerbs;
    const uint8_t*  fVerbsStop;
    const SkPoint*  fPts;
    const SkPoint*  fMoveToPtr;
    const SkScalar* fConicWeights;
    SkPoint         fScratch[2];    // for auto-close lines
    bool            fNeedsCloseLine;
    SkDEBUGCODE(bool fIsConic);

    enum {
        kIllegalEdgeValue = 99
    };

public:
    SkPathEdgeIter(const SkPath& path);

    SkScalar conicWeight() const {
        SkASSERT(fIsConic);
        return *fConicWeights;
    }

    enum class Edge {
        kLine  = SkPath::kLine_Verb,
        kQuad  = SkPath::kQuad_Verb,
        kConic = SkPath::kConic_Verb,
        kCubic = SkPath::kCubic_Verb,
    };

    static SkPath::Verb EdgeToVerb(Edge e) {
        return SkPath::Verb(e);
    }

    struct Result {
        const SkPoint*  fPts;   // points for the segment, or null if done
        Edge            fEdge;

        // Returns true when it holds an Edge, false when the path is done.
        operator bool() { return fPts != nullptr; }
    };

    Result next() {
        auto closeline = [&]() {
            fScratch[0] = fPts[-1];
            fScratch[1] = *fMoveToPtr;
            fNeedsCloseLine = false;
            return Result{ fScratch, Edge::kLine };
        };

        for (;;) {
            SkASSERT(fVerbs <= fVerbsStop);
            if (fVerbs == fVerbsStop) {
                return fNeedsCloseLine
                    ? closeline()
                    : Result{ nullptr, Edge(kIllegalEdgeValue) };
            }

            SkDEBUGCODE(fIsConic = false;)

            const auto v = *fVerbs++;
            switch (v) {
                case SkPath::kMove_Verb: {
                    if (fNeedsCloseLine) {
                        auto res = closeline();
                        fMoveToPtr = fPts++;
                        return res;
                    }
                    fMoveToPtr = fPts++;
                } break;
                case SkPath::kClose_Verb:
                    if (fNeedsCloseLine) return closeline();
                    break;
                default: {
                    // Actual edge.
                    const int pts_count = (v+2) / 2,
                              cws_count = (v & (v-1)) / 2;
                    SkASSERT(pts_count == SkPathPriv::PtsInIter(v) - 1);

                    fNeedsCloseLine = true;
                    fPts           += pts_count;
                    fConicWeights  += cws_count;

                    SkDEBUGCODE(fIsConic = (v == SkPath::kConic_Verb);)
                    SkASSERT(fIsConic == (cws_count > 0));

                    return { &fPts[-(pts_count + 1)], Edge(v) };
                }
            }
        }
    }
};

#endif
