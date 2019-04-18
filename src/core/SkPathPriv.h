/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathPriv_DEFINED
#define SkPathPriv_DEFINED

#include "SkPath.h"

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
        auto verbs = path.fPathRef->verbs();
        for (int i = 0; i < verbCount; i++) {
            switch (verbs[~i]) { // verbs are stored backwards; we use [~i] to get the i'th verb
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
            void operator++() { --fVerb; } // verbs are laid out backwards in memory.
            bool operator!=(const Iter& b) { return fVerb != b.fVerb; }
            SkPath::Verb operator*() { return static_cast<SkPath::Verb>(*fVerb); }
            const uint8_t* fVerb;
        };
        Iter begin() { return Iter{fPathRef->verbs() - 1}; }
        Iter end() { return Iter{fPathRef->verbs() - fPathRef->countVerbs() - 1}; }
    private:
        Verbs(const Verbs&) = delete;
        Verbs& operator=(const Verbs&) = delete;
        SkPathRef* fPathRef;
    };

    /**
     * Returns a pointer to the verb data. Note that the verbs are stored backwards in memory and
     * thus the returned pointer is the last verb.
     */
    static const uint8_t* VerbData(const SkPath& path) {
        return path.fPathRef->verbsMemBegin();
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
};

#endif
