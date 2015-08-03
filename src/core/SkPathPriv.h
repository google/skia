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

    static bool LastVerbIsClose(const SkPath& path) {
        int count = path.countVerbs();
        return count >= 1 && path.fPathRef->verbs()[~(count - 1)] == SkPath::Verb::kClose_Verb;
    }

    static void AddGenIDChangeListener(const SkPath& path, SkPathRef::GenIDChangeListener* listener) {
        path.fPathRef->addGenIDChangeListener(listener);
    }
};

#endif
