/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/gpu/GrStencilSettings.h"

#include "src/gpu/GrProcessor.h"

constexpr const GrUserStencilSettings gUnused(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kAlwaysIfInClip,
        0xffff,
        GrUserStencilOp::kKeep,
        GrUserStencilOp::kKeep,
        0x0000>()
);

GR_STATIC_ASSERT(kAll_StencilFlags == (gUnused.fFrontFlags[0] & gUnused.fBackFlags[0]));

const GrUserStencilSettings& GrUserStencilSettings::kUnused = gUnused;

void GrStencilSettings::reset(const GrUserStencilSettings& user, bool hasStencilClip,
                              int numStencilBits) {
    uint16_t frontFlags = user.fFrontFlags[hasStencilClip];
    if (frontFlags & kSingleSided_StencilFlag) {
        SkASSERT(frontFlags == user.fBackFlags[hasStencilClip]);
        fFlags = frontFlags;
        if (!this->isDisabled()) {
            fFront.reset(user.fFront, hasStencilClip, numStencilBits);
        }
        return;
    }

    uint16_t backFlags = user.fBackFlags[hasStencilClip];
    fFlags = frontFlags & backFlags;
    if (this->isDisabled()) {
        return;
    }
    if (!(frontFlags & kDisabled_StencilFlag)) {
        fFront.reset(user.fFront, hasStencilClip, numStencilBits);
    } else {
        fFront.setDisabled();
    }
    if (!(backFlags & kDisabled_StencilFlag)) {
        fBack.reset(user.fBack, hasStencilClip, numStencilBits);
    } else {
        fBack.setDisabled();
    }
}

void GrStencilSettings::reset(const GrStencilSettings& that) {
    fFlags = that.fFlags;
    if ((kInvalid_PrivateFlag | kDisabled_StencilFlag) & fFlags) {
        return;
    }
    if (!this->isTwoSided()) {
        memcpy(&fFront, &that.fFront, sizeof(Face));
    } else {
        memcpy(&fFront, &that.fFront, 2 * sizeof(Face));
        GR_STATIC_ASSERT(sizeof(Face) ==
                         offsetof(GrStencilSettings, fBack) - offsetof(GrStencilSettings, fFront));
    }
}

bool GrStencilSettings::operator==(const GrStencilSettings& that) const {
    if ((kInvalid_PrivateFlag | kDisabled_StencilFlag) & (fFlags | that.fFlags)) {
        // At least one is invalid and/or disabled.
        if (kInvalid_PrivateFlag & (fFlags | that.fFlags)) {
            return false; // We never allow invalid stencils to be equal.
        }
        // They're only equal if both are disabled.
        return kDisabled_StencilFlag & (fFlags & that.fFlags);
    }
    if (kSingleSided_StencilFlag & (fFlags & that.fFlags)) {
        return 0 == memcmp(&fFront, &that.fFront, sizeof(Face)); // Both are single sided.
    } else if (kSingleSided_StencilFlag & (fFlags | that.fFlags)) {
        return false;
    } else {
        return 0 == memcmp(&fFront, &that.fFront, 2 * sizeof(Face));
        GR_STATIC_ASSERT(sizeof(Face) ==
                         offsetof(GrStencilSettings, fBack) - offsetof(GrStencilSettings, fFront));
    }
    // memcmp relies on GrStencilSettings::Face being tightly packed.
    GR_STATIC_ASSERT(0 == offsetof(Face, fRef));
    GR_STATIC_ASSERT(2 == sizeof(Face::fRef));
    GR_STATIC_ASSERT(2 == offsetof(Face, fTest));
    GR_STATIC_ASSERT(2 == sizeof(Face::fTest));
    GR_STATIC_ASSERT(4 == offsetof(Face, fTestMask));
    GR_STATIC_ASSERT(2 == sizeof(Face::fTestMask));
    GR_STATIC_ASSERT(6 == offsetof(Face, fPassOp));
    GR_STATIC_ASSERT(1 == sizeof(Face::fPassOp));
    GR_STATIC_ASSERT(7 == offsetof(Face, fFailOp));
    GR_STATIC_ASSERT(1 == sizeof(Face::fFailOp));
    GR_STATIC_ASSERT(8 == offsetof(Face, fWriteMask));
    GR_STATIC_ASSERT(2 == sizeof(Face::fWriteMask));
    GR_STATIC_ASSERT(10 == sizeof(Face));
}

static constexpr GrStencilTest gUserStencilTestToRaw[kGrUserStencilTestCount] = {
    // Tests that respect the clip.
    GrStencilTest::kAlways,  // kAlwaysIfInClip (This is only for when there is not a stencil clip).
    GrStencilTest::kEqual,   // kEqualIfInClip.
    GrStencilTest::kLess,    // kLessIfInClip.
    GrStencilTest::kLEqual,  // kLEqualIfInClip.

    // Tests that ignore the clip.
    GrStencilTest::kAlways,
    GrStencilTest::kNever,
    GrStencilTest::kGreater,
    GrStencilTest::kGEqual,
    GrStencilTest::kLess,
    GrStencilTest::kLEqual,
    GrStencilTest::kEqual,
    GrStencilTest::kNotEqual
};

GR_STATIC_ASSERT(0 == (int)GrUserStencilTest::kAlwaysIfInClip);
GR_STATIC_ASSERT(1 == (int)GrUserStencilTest::kEqualIfInClip);
GR_STATIC_ASSERT(2 == (int)GrUserStencilTest::kLessIfInClip);
GR_STATIC_ASSERT(3 == (int)GrUserStencilTest::kLEqualIfInClip);
GR_STATIC_ASSERT(4 == (int)GrUserStencilTest::kAlways);
GR_STATIC_ASSERT(5 == (int)GrUserStencilTest::kNever);
GR_STATIC_ASSERT(6 == (int)GrUserStencilTest::kGreater);
GR_STATIC_ASSERT(7 == (int)GrUserStencilTest::kGEqual);
GR_STATIC_ASSERT(8 == (int)GrUserStencilTest::kLess);
GR_STATIC_ASSERT(9 == (int)GrUserStencilTest::kLEqual);
GR_STATIC_ASSERT(10 == (int)GrUserStencilTest::kEqual);
GR_STATIC_ASSERT(11 == (int)GrUserStencilTest::kNotEqual);

static constexpr GrStencilOp gUserStencilOpToRaw[kGrUserStencilOpCount] = {
    GrStencilOp::kKeep,

    // Ops that only modify user bits.
    GrStencilOp::kZero,
    GrStencilOp::kReplace,
    GrStencilOp::kInvert,
    GrStencilOp::kIncWrap,
    GrStencilOp::kDecWrap,
    GrStencilOp::kIncClamp,  // kIncMaybeClamp.
    GrStencilOp::kDecClamp,  // kDecMaybeClamp.

    // Ops that only modify the clip bit.
    GrStencilOp::kZero,      // kZeroClipBit.
    GrStencilOp::kReplace,   // kSetClipBit.
    GrStencilOp::kInvert,    // kInvertClipBit.

    // Ops that modify clip and user bits.
    GrStencilOp::kReplace,   // kSetClipAndReplaceUserBits.
    GrStencilOp::kZero       // kZeroClipAndUserBits.
};

GR_STATIC_ASSERT(0 == (int)GrUserStencilOp::kKeep);
GR_STATIC_ASSERT(1 == (int)GrUserStencilOp::kZero);
GR_STATIC_ASSERT(2 == (int)GrUserStencilOp::kReplace);
GR_STATIC_ASSERT(3 == (int)GrUserStencilOp::kInvert);
GR_STATIC_ASSERT(4 == (int)GrUserStencilOp::kIncWrap);
GR_STATIC_ASSERT(5 == (int)GrUserStencilOp::kDecWrap);
GR_STATIC_ASSERT(6 == (int)GrUserStencilOp::kIncMaybeClamp);
GR_STATIC_ASSERT(7 == (int)GrUserStencilOp::kDecMaybeClamp);
GR_STATIC_ASSERT(8 == (int)GrUserStencilOp::kZeroClipBit);
GR_STATIC_ASSERT(9 == (int)GrUserStencilOp::kSetClipBit);
GR_STATIC_ASSERT(10 == (int)GrUserStencilOp::kInvertClipBit);
GR_STATIC_ASSERT(11 == (int)GrUserStencilOp::kSetClipAndReplaceUserBits);
GR_STATIC_ASSERT(12 == (int)GrUserStencilOp::kZeroClipAndUserBits);

void GrStencilSettings::Face::reset(const GrUserStencilSettings::Face& user, bool hasStencilClip,
                                    int numStencilBits) {
    SkASSERT(user.fTest < (GrUserStencilTest)kGrUserStencilTestCount);
    SkASSERT(user.fPassOp < (GrUserStencilOp)kGrUserStencilOpCount);
    SkASSERT(user.fFailOp < (GrUserStencilOp)kGrUserStencilOpCount);
    SkASSERT(numStencilBits > 0 && numStencilBits <= 16);
    int clipBit = 1 << (numStencilBits - 1);
    int userMask = clipBit - 1;

    GrUserStencilOp maxOp = SkTMax(user.fPassOp, user.fFailOp);
    SkDEBUGCODE(GrUserStencilOp otherOp = SkTMin(user.fPassOp, user.fFailOp);)
    if (maxOp <= kLastUserOnlyStencilOp) {
        // Ops that only modify user bits.
        fWriteMask = user.fWriteMask & userMask;
        SkASSERT(otherOp <= kLastUserOnlyStencilOp);
    } else if (maxOp <= kLastClipOnlyStencilOp) {
        // Ops that only modify the clip bit.
        fWriteMask = clipBit;
        SkASSERT(GrUserStencilOp::kKeep == otherOp ||
                 (otherOp > kLastUserOnlyStencilOp && otherOp <= kLastClipOnlyStencilOp));
    } else {
        // Ops that modify both clip and user bits.
        fWriteMask = clipBit | (user.fWriteMask & userMask);
        SkASSERT(GrUserStencilOp::kKeep == otherOp || otherOp > kLastClipOnlyStencilOp);
    }

    fFailOp = gUserStencilOpToRaw[(int)user.fFailOp];
    fPassOp = gUserStencilOpToRaw[(int)user.fPassOp];

    if (!hasStencilClip || user.fTest > kLastClippedStencilTest) {
        // Ignore the clip.
        fTestMask = user.fTestMask & userMask;
        fTest = gUserStencilTestToRaw[(int)user.fTest];
    } else if (GrUserStencilTest::kAlwaysIfInClip != user.fTest) {
        // Respect the clip.
        fTestMask = clipBit | (user.fTestMask & userMask);
        fTest = gUserStencilTestToRaw[(int)user.fTest];
    } else {
        // Test only for clip.
        fTestMask = clipBit;
        fTest = GrStencilTest::kEqual;
    }

    fRef = (clipBit | user.fRef) & (fTestMask | fWriteMask);
}

void GrStencilSettings::Face::setDisabled() {
    memset(this, 0, sizeof(*this));
    GR_STATIC_ASSERT(0 == (int)GrStencilTest::kAlways);
    GR_STATIC_ASSERT(0 == (int)GrStencilOp::kKeep);
}

////////////////////////////////////////////////////////////////////////////////
// Stencil Rules for Merging user stencil space into clip
//

///////
// Replace
static constexpr GrUserStencilSettings gUserToClipReplace(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kNotEqual,
        0xffff,
        GrUserStencilOp::kSetClipAndReplaceUserBits,
        GrUserStencilOp::kZeroClipAndUserBits,
        0xffff>()
);

static constexpr GrUserStencilSettings gInvUserToClipReplace(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kEqual,
        0xffff,
        GrUserStencilOp::kSetClipAndReplaceUserBits,
        GrUserStencilOp::kZeroClipAndUserBits,
        0xffff>()
);

///////
// Intersect
static constexpr GrUserStencilSettings gUserToClipIsect(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kLessIfInClip, // "0 < userBits" is equivalent to "0 != userBits".
        0xffff,
        GrUserStencilOp::kSetClipAndReplaceUserBits,
        GrUserStencilOp::kZeroClipAndUserBits,
        0xffff>()
);

///////
// Difference
static constexpr GrUserStencilSettings gUserToClipDiff(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kEqualIfInClip,
        0xffff,
        GrUserStencilOp::kSetClipAndReplaceUserBits,
        GrUserStencilOp::kZeroClipAndUserBits,
        0xffff>()
);

///////
// Union
static constexpr GrUserStencilSettings gUserToClipUnion(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kNotEqual,
        0xffff,
        GrUserStencilOp::kSetClipAndReplaceUserBits,
        GrUserStencilOp::kKeep,
        0xffff>()
);

static constexpr GrUserStencilSettings gInvUserToClipUnionPass0( // Does not zero user bits.
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kEqual,
        0xffff,
        GrUserStencilOp::kSetClipBit,
        GrUserStencilOp::kKeep,
        0x0000>()
);

///////
// Xor
static constexpr GrUserStencilSettings gUserToClipXorPass0( // Does not zero user bits.
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kNotEqual,
        0xffff,
        GrUserStencilOp::kInvertClipBit,
        GrUserStencilOp::kKeep,
        0x0000>()
);

static constexpr GrUserStencilSettings gInvUserToClipXorPass0( // Does not zero user bits.
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kEqual,
        0xffff,
        GrUserStencilOp::kInvertClipBit,
        GrUserStencilOp::kKeep,
        0x0000>()
);

///////
// Reverse Diff
static constexpr GrUserStencilSettings gUserToClipRDiffPass0( // Does not zero user bits.
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kNotEqual,
        0xffff,
        GrUserStencilOp::kInvertClipBit,
        GrUserStencilOp::kZeroClipBit,
        0x0000>()
);

static constexpr GrUserStencilSettings gInvUserToClipRDiffPass0( // Does not zero user bits.
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kEqual,
        0xffff,
        GrUserStencilOp::kInvertClipBit,
        GrUserStencilOp::kZeroClipBit,
        0x0000>()
);

///////
// Second pass to clear user bits (only needed sometimes)
static constexpr GrUserStencilSettings gZeroUserBits(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kNotEqual,
        0xffff,
        GrUserStencilOp::kZero,
        GrUserStencilOp::kKeep,
        0xffff>()
);

static constexpr const GrUserStencilSettings* gUserToClipTable[2][1 + SkRegion::kLastOp][3] = {
    {  /* Normal fill. */
        {&gUserToClipDiff,           nullptr,         nullptr},  // kDifference_Op.
        {&gUserToClipIsect,          nullptr,         nullptr},  // kIntersect_Op.
        {&gUserToClipUnion,          nullptr,         nullptr},  // kUnion_Op.
        {&gUserToClipXorPass0,       &gZeroUserBits,  nullptr},  // kXOR_Op.
        {&gUserToClipRDiffPass0,     &gZeroUserBits,  nullptr},  // kReverseDifference_Op.
        {&gUserToClipReplace,        nullptr,         nullptr}   // kReplace_Op.

    }, /* Inverse fill. */ {
        {&gUserToClipIsect,          nullptr,         nullptr},  // ~diff (aka isect).
        {&gUserToClipDiff,           nullptr,         nullptr},  // ~isect (aka diff).
        {&gInvUserToClipUnionPass0,  &gZeroUserBits,  nullptr},  // ~union.
        {&gInvUserToClipXorPass0,    &gZeroUserBits,  nullptr},  // ~xor.
        {&gInvUserToClipRDiffPass0,  &gZeroUserBits,  nullptr},  // ~reverse diff.
        {&gInvUserToClipReplace,     nullptr,         nullptr}   // ~replace.
    }
};

GR_STATIC_ASSERT(0 == SkRegion::kDifference_Op);
GR_STATIC_ASSERT(1 == SkRegion::kIntersect_Op);
GR_STATIC_ASSERT(2 == SkRegion::kUnion_Op);
GR_STATIC_ASSERT(3 == SkRegion::kXOR_Op);
GR_STATIC_ASSERT(4 == SkRegion::kReverseDifference_Op);
GR_STATIC_ASSERT(5 == SkRegion::kReplace_Op);

///////
// Direct to Stencil

// We can render a clip element directly without first writing to the client
// portion of the clip when the fill is not inverse and the set operation will
// only modify the in/out status of samples covered by the clip element.

// this one only works if used right after stencil clip was cleared.
// Our clip mask creation code doesn't allow midstream replace ops.
static constexpr GrUserStencilSettings gReplaceClip(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kAlways,
        0xffff,
        GrUserStencilOp::kSetClipBit,
        GrUserStencilOp::kSetClipBit,
        0x0000>()
);

static constexpr GrUserStencilSettings gUnionClip(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kAlwaysIfInClip,
        0xffff,
        GrUserStencilOp::kKeep,
        GrUserStencilOp::kSetClipBit,
        0x0000>()
);

static constexpr GrUserStencilSettings gXorClip(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kAlways,
        0xffff,
        GrUserStencilOp::kInvertClipBit,
        GrUserStencilOp::kInvertClipBit,
        0x0000>()
);

static constexpr GrUserStencilSettings gDiffClip(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kAlwaysIfInClip,
        0xffff,
        GrUserStencilOp::kZeroClipBit,
        GrUserStencilOp::kKeep,
        0x0000>()
);

static constexpr const GrUserStencilSettings* gDirectDrawTable[1 + SkRegion::kLastOp][2] = {
    {&gDiffClip,     nullptr},  // kDifference_Op.
    {nullptr,        nullptr},  // kIntersect_Op.
    {&gUnionClip,    nullptr},  // kUnion_Op.
    {&gXorClip,      nullptr},  // kXOR_Op.
    {nullptr,        nullptr},  // kReverseDifference_Op.
    {&gReplaceClip,  nullptr}   // kReplace_Op.
};

GR_STATIC_ASSERT(0 == SkRegion::kDifference_Op);
GR_STATIC_ASSERT(1 == SkRegion::kIntersect_Op);
GR_STATIC_ASSERT(2 == SkRegion::kUnion_Op);
GR_STATIC_ASSERT(3 == SkRegion::kXOR_Op);
GR_STATIC_ASSERT(4 == SkRegion::kReverseDifference_Op);
GR_STATIC_ASSERT(5 == SkRegion::kReplace_Op);

GrUserStencilSettings const* const* GrStencilSettings::GetClipPasses(SkRegion::Op op,
                                                                     bool canBeDirect,
                                                                     bool invertedFill,
                                                                     bool* drawDirectToClip) {
    SkASSERT((unsigned)op <= SkRegion::kLastOp);
    if (canBeDirect && !invertedFill) { // TODO: inverse fill + intersect op can be direct.
        GrUserStencilSettings const* const* directPass = gDirectDrawTable[op];
        if (directPass[0]) {
            *drawDirectToClip = true;
            return directPass;
        }
    }
    *drawDirectToClip = false;
    return gUserToClipTable[invertedFill][op];
}

static constexpr GrUserStencilSettings gZeroStencilClipBit(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kAlways,
        0xffff,
        GrUserStencilOp::kZeroClipBit,
        GrUserStencilOp::kZeroClipBit,
        0x0000>()
);

const GrUserStencilSettings* GrStencilSettings::SetClipBitSettings(bool setToInside) {
    return setToInside ? &gReplaceClip : &gZeroStencilClipBit;
}

void GrStencilSettings::genKey(GrProcessorKeyBuilder* b) const {
    b->add32(fFlags);
    if (this->isDisabled()) {
        return;
    }
    if (!this->isTwoSided()) {
        constexpr int kCount16 = sizeof(Face) / sizeof(uint16_t);
        GR_STATIC_ASSERT(0 == sizeof(Face) % sizeof(uint16_t));
        uint16_t* key = reinterpret_cast<uint16_t*>(b->add32n((kCount16 + 1) / 2));
        memcpy(key, &fFront, sizeof(Face));
        key[kCount16] = 0;
        GR_STATIC_ASSERT(1 == kCount16 % 2);
    } else {
        constexpr int kCount32 = (2 * sizeof(Face)) / sizeof(uint32_t);
        GR_STATIC_ASSERT(0 == (2 * sizeof(Face)) % sizeof(uint32_t));
        uint32_t* key = b->add32n(kCount32);
        memcpy(key, &fFront, 2 * sizeof(Face));
        GR_STATIC_ASSERT(sizeof(Face) ==
                         offsetof(GrStencilSettings, fBack) - offsetof(GrStencilSettings, fFront));
    }
    // We rely on GrStencilSettings::Face being tightly packed for the key to be reliable.
    GR_STATIC_ASSERT(0 == offsetof(Face, fRef));
    GR_STATIC_ASSERT(2 == sizeof(Face::fRef));
    GR_STATIC_ASSERT(2 == offsetof(Face, fTest));
    GR_STATIC_ASSERT(2 == sizeof(Face::fTest));
    GR_STATIC_ASSERT(4 == offsetof(Face, fTestMask));
    GR_STATIC_ASSERT(2 == sizeof(Face::fTestMask));
    GR_STATIC_ASSERT(6 == offsetof(Face, fPassOp));
    GR_STATIC_ASSERT(1 == sizeof(Face::fPassOp));
    GR_STATIC_ASSERT(7 == offsetof(Face, fFailOp));
    GR_STATIC_ASSERT(1 == sizeof(Face::fFailOp));
    GR_STATIC_ASSERT(8 == offsetof(Face, fWriteMask));
    GR_STATIC_ASSERT(2 == sizeof(Face::fWriteMask));
    GR_STATIC_ASSERT(10 == sizeof(Face));
}
