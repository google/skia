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
    enum FirstDirection {
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

    static void AddGenIDChangeListener(const SkPath& path, SkPathRef::GenIDChangeListener* listener) {
        path.fPathRef->addGenIDChangeListener(listener);
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
};

#endif
