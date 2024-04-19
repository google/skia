/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/pathops/SkPathOpsTypes.h"

#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkFloatBits.h"

#include <algorithm>
#include <cstdint>
#include <cstring>

static bool arguments_denormalized(float a, float b, int epsilon) {
    float denormalizedCheck = FLT_EPSILON * epsilon / 2;
    return fabsf(a) <= denormalizedCheck && fabsf(b) <= denormalizedCheck;
}

// from http://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
// FIXME: move to SkFloatBits.h
static bool equal_ulps(float a, float b, int epsilon, int depsilon) {
    if (arguments_denormalized(a, b, depsilon)) {
        return true;
    }
    int aBits = SkFloatAs2sCompliment(a);
    int bBits = SkFloatAs2sCompliment(b);
    // Find the difference in ULPs.
    return aBits < bBits + epsilon && bBits < aBits + epsilon;
}

static bool equal_ulps_no_normal_check(float a, float b, int epsilon, int depsilon) {
    int aBits = SkFloatAs2sCompliment(a);
    int bBits = SkFloatAs2sCompliment(b);
    // Find the difference in ULPs.
    return aBits < bBits + epsilon && bBits < aBits + epsilon;
}

static bool equal_ulps_pin(float a, float b, int epsilon, int depsilon) {
    if (!SkIsFinite(a, b)) {
        return false;
    }
    if (arguments_denormalized(a, b, depsilon)) {
        return true;
    }
    int aBits = SkFloatAs2sCompliment(a);
    int bBits = SkFloatAs2sCompliment(b);
    // Find the difference in ULPs.
    return aBits < bBits + epsilon && bBits < aBits + epsilon;
}

static bool d_equal_ulps(float a, float b, int epsilon) {
    int aBits = SkFloatAs2sCompliment(a);
    int bBits = SkFloatAs2sCompliment(b);
    // Find the difference in ULPs.
    return aBits < bBits + epsilon && bBits < aBits + epsilon;
}

static bool not_equal_ulps(float a, float b, int epsilon) {
    if (arguments_denormalized(a, b, epsilon)) {
        return false;
    }
    int aBits = SkFloatAs2sCompliment(a);
    int bBits = SkFloatAs2sCompliment(b);
    // Find the difference in ULPs.
    return aBits >= bBits + epsilon || bBits >= aBits + epsilon;
}

static bool not_equal_ulps_pin(float a, float b, int epsilon) {
    if (!SkIsFinite(a, b)) {
        return false;
    }
    if (arguments_denormalized(a, b, epsilon)) {
        return false;
    }
    int aBits = SkFloatAs2sCompliment(a);
    int bBits = SkFloatAs2sCompliment(b);
    // Find the difference in ULPs.
    return aBits >= bBits + epsilon || bBits >= aBits + epsilon;
}

static bool d_not_equal_ulps(float a, float b, int epsilon) {
    int aBits = SkFloatAs2sCompliment(a);
    int bBits = SkFloatAs2sCompliment(b);
    // Find the difference in ULPs.
    return aBits >= bBits + epsilon || bBits >= aBits + epsilon;
}

static bool less_ulps(float a, float b, int epsilon) {
    if (arguments_denormalized(a, b, epsilon)) {
        return a <= b - FLT_EPSILON * epsilon;
    }
    int aBits = SkFloatAs2sCompliment(a);
    int bBits = SkFloatAs2sCompliment(b);
    // Find the difference in ULPs.
    return aBits <= bBits - epsilon;
}

static bool less_or_equal_ulps(float a, float b, int epsilon) {
    if (arguments_denormalized(a, b, epsilon)) {
        return a < b + FLT_EPSILON * epsilon;
    }
    int aBits = SkFloatAs2sCompliment(a);
    int bBits = SkFloatAs2sCompliment(b);
    // Find the difference in ULPs.
    return aBits < bBits + epsilon;
}

// equality using the same error term as between
bool AlmostBequalUlps(float a, float b) {
    const int UlpsEpsilon = 2;
    return equal_ulps(a, b, UlpsEpsilon, UlpsEpsilon);
}

bool AlmostPequalUlps(float a, float b) {
    const int UlpsEpsilon = 8;
    return equal_ulps(a, b, UlpsEpsilon, UlpsEpsilon);
}

bool AlmostDequalUlps(float a, float b) {
    const int UlpsEpsilon = 16;
    return d_equal_ulps(a, b, UlpsEpsilon);
}

bool AlmostDequalUlps(double a, double b) {
    if (fabs(a) < SK_ScalarMax && fabs(b) < SK_ScalarMax) {
        return AlmostDequalUlps(SkDoubleToScalar(a), SkDoubleToScalar(b));
    }
    // We allow divide-by-zero here. It only happens if one of a,b is zero, and the other is NaN.
    // (Otherwise, we'd hit the condition above). Thus, if std::max returns 0, we compute NaN / 0,
    // which will produce NaN. The comparison will return false, which is the correct answer.
    return sk_ieee_double_divide(fabs(a - b), std::max(fabs(a), fabs(b))) < FLT_EPSILON * 16;
}

bool AlmostEqualUlps(float a, float b) {
    const int UlpsEpsilon = 16;
    return equal_ulps(a, b, UlpsEpsilon, UlpsEpsilon);
}

bool AlmostEqualUlpsNoNormalCheck(float a, float b) {
    const int UlpsEpsilon = 16;
    return equal_ulps_no_normal_check(a, b, UlpsEpsilon, UlpsEpsilon);
}

bool AlmostEqualUlps_Pin(float a, float b) {
    const int UlpsEpsilon = 16;
    return equal_ulps_pin(a, b, UlpsEpsilon, UlpsEpsilon);
}

bool NotAlmostEqualUlps(float a, float b) {
    const int UlpsEpsilon = 16;
    return not_equal_ulps(a, b, UlpsEpsilon);
}

bool NotAlmostEqualUlps_Pin(float a, float b) {
    const int UlpsEpsilon = 16;
    return not_equal_ulps_pin(a, b, UlpsEpsilon);
}

bool NotAlmostDequalUlps(float a, float b) {
    const int UlpsEpsilon = 16;
    return d_not_equal_ulps(a, b, UlpsEpsilon);
}

bool RoughlyEqualUlps(float a, float b) {
    const int UlpsEpsilon = 256;
    const int DUlpsEpsilon = 1024;
    return equal_ulps(a, b, UlpsEpsilon, DUlpsEpsilon);
}

bool AlmostBetweenUlps(float a, float b, float c) {
    const int UlpsEpsilon = 2;
    return a <= c ? less_or_equal_ulps(a, b, UlpsEpsilon) && less_or_equal_ulps(b, c, UlpsEpsilon)
        : less_or_equal_ulps(b, a, UlpsEpsilon) && less_or_equal_ulps(c, b, UlpsEpsilon);
}

bool AlmostLessUlps(float a, float b) {
    const int UlpsEpsilon = 16;
    return less_ulps(a, b, UlpsEpsilon);
}

bool AlmostLessOrEqualUlps(float a, float b) {
    const int UlpsEpsilon = 16;
    return less_or_equal_ulps(a, b, UlpsEpsilon);
}

int UlpsDistance(float a, float b) {
    int32_t floatIntA, floatIntB;
    memcpy(&floatIntA, &a, sizeof(int32_t));
    memcpy(&floatIntB, &b, sizeof(int32_t));
    // Different signs means they do not match.
    if ((floatIntA < 0) != (floatIntB < 0)) {
        // Check for equality to make sure +0 == -0
        return a == b ? 0 : SK_MaxS32;
    }
    // Find the difference in ULPs.
    return SkTAbs(floatIntA - floatIntB);
}

SkOpGlobalState::SkOpGlobalState(SkOpContourHead* head,
                                 SkArenaAlloc* allocator
                                 SkDEBUGPARAMS(bool debugSkipAssert)
                                 SkDEBUGPARAMS(const char* testName))
    : fAllocator(allocator)
    , fCoincidence(nullptr)
    , fContourHead(head)
    , fNested(0)
    , fWindingFailed(false)
    , fPhase(SkOpPhase::kIntersecting)
    SkDEBUGPARAMS(fDebugTestName(testName))
    SkDEBUGPARAMS(fAngleID(0))
    SkDEBUGPARAMS(fCoinID(0))
    SkDEBUGPARAMS(fContourID(0))
    SkDEBUGPARAMS(fPtTID(0))
    SkDEBUGPARAMS(fSegmentID(0))
    SkDEBUGPARAMS(fSpanID(0))
    SkDEBUGPARAMS(fDebugSkipAssert(debugSkipAssert)) {
#if DEBUG_T_SECT_LOOP_COUNT
    debugResetLoopCounts();
#endif
#if DEBUG_COIN
    fPreviousFuncName = nullptr;
#endif
}
