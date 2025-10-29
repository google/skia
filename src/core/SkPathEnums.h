/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * This file contains private enums related to paths. See also skbug.com/40042016
 */

#ifndef SkPathEnums_DEFINED
#define SkPathEnums_DEFINED

#include "include/core/SkPathTypes.h"

#include <optional>

enum class SkPathConvexity {
    kConvex_CW,
    kConvex_CCW,
    kConvex_Degenerate,  // known to not have a determinable direction, but convex

    kConcave,

    kUnknown,   // todo: can we eliminate this, and use optional or other patterns?
};

enum class SkResolveConvexity {
    kNo = false,
    kYes = true,
};

static inline bool SkPathConvexity_IsConvex(SkPathConvexity cv) {
    return cv == SkPathConvexity::kConvex_CW
        || cv == SkPathConvexity::kConvex_CCW
        || cv == SkPathConvexity::kConvex_Degenerate;
}

static inline SkPathConvexity SkPathConvexity_OppositeConvexDirection(SkPathConvexity cv) {
    SkASSERT(SkPathConvexity_IsConvex(cv));
    switch (cv) {
        case SkPathConvexity::kConvex_CW:  cv = SkPathConvexity::kConvex_CCW; break;
        case SkPathConvexity::kConvex_CCW: cv = SkPathConvexity::kConvex_CW;  break;
        default: break;
    }
    return cv;
}

enum class SkPathFirstDirection {
    kCW,         // == SkPathDirection::kCW
    kCCW,        // == SkPathDirection::kCCW
    kUnknown,
};

static inline SkPathConvexity SkPathDirection_ToConvexity(SkPathDirection dir) {
    switch (dir) {
        case SkPathDirection::kCW:  return SkPathConvexity::kConvex_CW;
        case SkPathDirection::kCCW: return SkPathConvexity::kConvex_CCW;
    }
    SkUNREACHABLE;
}

static inline SkPathConvexity SkPathFirstDirection_ToConvexity(SkPathFirstDirection dir) {
    switch (dir) {
        case SkPathFirstDirection::kCW:      return SkPathConvexity::kConvex_CW;
        case SkPathFirstDirection::kCCW:     return SkPathConvexity::kConvex_CCW;
        case SkPathFirstDirection::kUnknown: return SkPathConvexity::kConvex_Degenerate;
    }
    SkUNREACHABLE;
}

static inline std::optional<SkPathDirection> SkPathConvexity_ToDirection(SkPathConvexity cv) {
    if (cv == SkPathConvexity::kConvex_CW) {
        return SkPathDirection::kCW;
    }
    if (cv == SkPathConvexity::kConvex_CCW) {
        return SkPathDirection::kCCW;
    }
    return {};
}

static inline SkPathFirstDirection SkPathConvexity_ToFirstDirection(SkPathConvexity cv) {
    if (cv == SkPathConvexity::kConvex_CW) {
        return SkPathFirstDirection::kCW;
    }
    if (cv == SkPathConvexity::kConvex_CCW) {
        return SkPathFirstDirection::kCCW;
    }
    return SkPathFirstDirection::kUnknown;
}

static inline SkPathFirstDirection SkPathDirectionToFirst(SkPathDirection dir) {
    return dir == SkPathDirection::kCW ? SkPathFirstDirection::kCW
                                       : SkPathFirstDirection::kCCW;
}

#endif
