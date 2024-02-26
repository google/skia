/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/GrStencilSettings.h"

#include "include/private/base/SkDebug.h"
#include "src/gpu/KeyBuilder.h"

#include <algorithm>
#include <cstddef>
#include <cstring>

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

static constexpr GrUserStencilSettings gZeroStencilClipBit(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kAlways,
        0xffff,
        GrUserStencilOp::kZeroClipBit,
        GrUserStencilOp::kZeroClipBit,
        0x0000>()
);
static constexpr GrUserStencilSettings gSetStencilClipBit(
    GrUserStencilSettings::StaticInit<
        0x0000,
        GrUserStencilTest::kAlways,
        0xffff,
        GrUserStencilOp::kSetClipBit,
        GrUserStencilOp::kSetClipBit,
        0x0000>()
);

const GrUserStencilSettings* GrStencilSettings::SetClipBitSettings(bool setToInside) {
    return setToInside ? &gSetStencilClipBit : &gZeroStencilClipBit;
}

void GrStencilSettings::genKey(skgpu::KeyBuilder* b, bool includeRefs) const {
    b->addBits(6, fFlags, "stencilFlags");
    if (this->isDisabled()) {
        return;
    }
    if (!this->isTwoSided()) {
        if (includeRefs) {
            b->addBytes(sizeof(Face), &fCWFace, "stencilCWFace");
        } else {
            Face tempFace = fCWFace;
            tempFace.fRef = 0;
            b->addBytes(sizeof(Face), &tempFace, "stencilCWFace");
        }
    } else {
        if (includeRefs) {
            b->addBytes(sizeof(Face), &fCWFace, "stencilCWFace");
            b->addBytes(sizeof(Face), &fCCWFace, "stencilCCWFace");
        } else {
            Face tempFaces[2];
            tempFaces[0] = fCWFace;
            tempFaces[0].fRef = 0;
            tempFaces[1] = fCCWFace;
            tempFaces[1].fRef = 0;
            b->addBytes(sizeof(Face), &tempFaces[0], "stencilCWFace");
            b->addBytes(sizeof(Face), &tempFaces[1], "stencilCCWFace");
        }
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
