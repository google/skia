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

static_assert(kAll_StencilFlags == (gUnused.fCWFlags[0] & gUnused.fCCWFlags[0]));

const GrUserStencilSettings& GrUserStencilSettings::kUnused = gUnused;

void GrStencilSettings::reset(const GrUserStencilSettings& user, bool hasStencilClip,
                              int numStencilBits) {
    uint16_t cwFlags = user.fCWFlags[hasStencilClip];
    if (cwFlags & kSingleSided_StencilFlag) {
        SkASSERT(cwFlags == user.fCCWFlags[hasStencilClip]);
        fFlags = cwFlags;
        if (!this->isDisabled()) {
            fCWFace.reset(user.fCWFace, hasStencilClip, numStencilBits);
        }
        return;
    }

    uint16_t ccwFlags = user.fCCWFlags[hasStencilClip];
    fFlags = cwFlags & ccwFlags;
    if (this->isDisabled()) {
        return;
    }
    if (!(cwFlags & kDisabled_StencilFlag)) {
        fCWFace.reset(user.fCWFace, hasStencilClip, numStencilBits);
    } else {
        fCWFace.setDisabled();
    }
    if (!(ccwFlags & kDisabled_StencilFlag)) {
        fCCWFace.reset(user.fCCWFace, hasStencilClip, numStencilBits);
    } else {
        fCCWFace.setDisabled();
    }
}

void GrStencilSettings::reset(const GrStencilSettings& that) {
    fFlags = that.fFlags;
    if ((kInvalid_PrivateFlag | kDisabled_StencilFlag) & fFlags) {
        return;
    }
    if (!this->isTwoSided()) {
        memcpy(&fCWFace, &that.fCWFace, sizeof(Face));
    } else {
        memcpy(&fCWFace, &that.fCWFace, 2 * sizeof(Face));
        static_assert(sizeof(Face) ==
                      offsetof(GrStencilSettings, fCCWFace) - offsetof(GrStencilSettings, fCWFace));
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
        return 0 == memcmp(&fCWFace, &that.fCWFace, sizeof(Face)); // Both are single sided.
    } else if (kSingleSided_StencilFlag & (fFlags | that.fFlags)) {
        return false;
    } else {
        return 0 == memcmp(&fCWFace, &that.fCWFace, 2 * sizeof(Face));
        static_assert(sizeof(Face) ==
                      offsetof(GrStencilSettings, fCCWFace) - offsetof(GrStencilSettings, fCWFace));
    }
    // memcmp relies on GrStencilSettings::Face being tightly packed.
    static_assert(0 == offsetof(Face, fRef));
    static_assert(2 == sizeof(Face::fRef));
    static_assert(2 == offsetof(Face, fTest));
    static_assert(2 == sizeof(Face::fTest));
    static_assert(4 == offsetof(Face, fTestMask));
    static_assert(2 == sizeof(Face::fTestMask));
    static_assert(6 == offsetof(Face, fPassOp));
    static_assert(1 == sizeof(Face::fPassOp));
    static_assert(7 == offsetof(Face, fFailOp));
    static_assert(1 == sizeof(Face::fFailOp));
    static_assert(8 == offsetof(Face, fWriteMask));
    static_assert(2 == sizeof(Face::fWriteMask));
    static_assert(10 == sizeof(Face));
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

static_assert(0 == (int)GrUserStencilTest::kAlwaysIfInClip);
static_assert(1 == (int)GrUserStencilTest::kEqualIfInClip);
static_assert(2 == (int)GrUserStencilTest::kLessIfInClip);
static_assert(3 == (int)GrUserStencilTest::kLEqualIfInClip);
static_assert(4 == (int)GrUserStencilTest::kAlways);
static_assert(5 == (int)GrUserStencilTest::kNever);
static_assert(6 == (int)GrUserStencilTest::kGreater);
static_assert(7 == (int)GrUserStencilTest::kGEqual);
static_assert(8 == (int)GrUserStencilTest::kLess);
static_assert(9 == (int)GrUserStencilTest::kLEqual);
static_assert(10 == (int)GrUserStencilTest::kEqual);
static_assert(11 == (int)GrUserStencilTest::kNotEqual);

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

static_assert(0 == (int)GrUserStencilOp::kKeep);
static_assert(1 == (int)GrUserStencilOp::kZero);
static_assert(2 == (int)GrUserStencilOp::kReplace);
static_assert(3 == (int)GrUserStencilOp::kInvert);
static_assert(4 == (int)GrUserStencilOp::kIncWrap);
static_assert(5 == (int)GrUserStencilOp::kDecWrap);
static_assert(6 == (int)GrUserStencilOp::kIncMaybeClamp);
static_assert(7 == (int)GrUserStencilOp::kDecMaybeClamp);
static_assert(8 == (int)GrUserStencilOp::kZeroClipBit);
static_assert(9 == (int)GrUserStencilOp::kSetClipBit);
static_assert(10 == (int)GrUserStencilOp::kInvertClipBit);
static_assert(11 == (int)GrUserStencilOp::kSetClipAndReplaceUserBits);
static_assert(12 == (int)GrUserStencilOp::kZeroClipAndUserBits);

void GrStencilSettings::Face::reset(const GrUserStencilSettings::Face& user, bool hasStencilClip,
                                    int numStencilBits) {
    SkASSERT(user.fTest < (GrUserStencilTest)kGrUserStencilTestCount);
    SkASSERT(user.fPassOp < (GrUserStencilOp)kGrUserStencilOpCount);
    SkASSERT(user.fFailOp < (GrUserStencilOp)kGrUserStencilOpCount);
    SkASSERT(numStencilBits > 0 && numStencilBits <= 16);
    int clipBit = 1 << (numStencilBits - 1);
    int userMask = clipBit - 1;

    GrUserStencilOp maxOp = std::max(user.fPassOp, user.fFailOp);
    SkDEBUGCODE(GrUserStencilOp otherOp = std::min(user.fPassOp, user.fFailOp);)
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
    static_assert(0 == (int)GrStencilTest::kAlways);
    static_assert(0 == (int)GrStencilOp::kKeep);
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

static_assert(0 == SkRegion::kDifference_Op);
static_assert(1 == SkRegion::kIntersect_Op);
static_assert(2 == SkRegion::kUnion_Op);
static_assert(3 == SkRegion::kXOR_Op);
static_assert(4 == SkRegion::kReverseDifference_Op);
static_assert(5 == SkRegion::kReplace_Op);

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

static_assert(0 == SkRegion::kDifference_Op);
static_assert(1 == SkRegion::kIntersect_Op);
static_assert(2 == SkRegion::kUnion_Op);
static_assert(3 == SkRegion::kXOR_Op);
static_assert(4 == SkRegion::kReverseDifference_Op);
static_assert(5 == SkRegion::kReplace_Op);

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
        static_assert(0 == sizeof(Face) % sizeof(uint16_t));
        uint16_t* key = reinterpret_cast<uint16_t*>(b->add32n((kCount16 + 1) / 2));
        memcpy(key, &fCWFace, sizeof(Face));
        key[kCount16] = 0;
        static_assert(1 == kCount16 % 2);
    } else {
        constexpr int kCount32 = (2 * sizeof(Face)) / sizeof(uint32_t);
        static_assert(0 == (2 * sizeof(Face)) % sizeof(uint32_t));
        uint32_t* key = b->add32n(kCount32);
        memcpy(key, &fCWFace, 2 * sizeof(Face));
        static_assert(sizeof(Face) ==
                      offsetof(GrStencilSettings, fCCWFace) - offsetof(GrStencilSettings, fCWFace));
    }
    // We rely on GrStencilSettings::Face being tightly packed for the key to be reliable.
    static_assert(0 == offsetof(Face, fRef));
    static_assert(2 == sizeof(Face::fRef));
    static_assert(2 == offsetof(Face, fTest));
    static_assert(2 == sizeof(Face::fTest));
    static_assert(4 == offsetof(Face, fTestMask));
    static_assert(2 == sizeof(Face::fTestMask));
    static_assert(6 == offsetof(Face, fPassOp));
    static_assert(1 == sizeof(Face::fPassOp));
    static_assert(7 == offsetof(Face, fFailOp));
    static_assert(1 == sizeof(Face::fFailOp));
    static_assert(8 == offsetof(Face, fWriteMask));
    static_assert(2 == sizeof(Face::fWriteMask));
    static_assert(10 == sizeof(Face));
}
