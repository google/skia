/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrUserStencilSettings_DEFINED
#define GrUserStencilSettings_DEFINED

#include "include/gpu/GrTypes.h"

/**
 * Gr uses the stencil buffer to implement complex clipping inside the
 * GrOpList class. The GrOpList makes a subset of the stencil buffer
 * bits available for other uses by external code (user bits). Client code can
 * modify these bits. GrOpList will ignore ref, mask, and writemask bits
 * provided by clients that fall outside the user range.
 *
 * When code outside the GrOpList class uses the stencil buffer the contract
 * is as follows:
 *
 * > Normal stencil funcs allow the client to pass / fail regardless of the
 *   reserved clip bits.
 * > Additional functions allow a test against the clip along with a limited
 *   set of tests against the user bits.
 * > Client can assume all user bits are zero initially.
 * > Client must ensure that after all its passes are finished it has only
 *   written to the color buffer in the region inside the clip. Furthermore, it
 *   must zero all user bits that were modifed (both inside and outside the
 *   clip).
 */

enum GrStencilFlags {
    kDisabled_StencilFlag         = (1 << 0),
    kTestAlwaysPasses_StencilFlag = (1 << 1),
    kNoModifyStencil_StencilFlag  = (1 << 2),
    kNoWrapOps_StencilFlag        = (1 << 3),
    kSingleSided_StencilFlag      = (1 << 4),

    kLast_StencilFlag = kSingleSided_StencilFlag,
    kAll_StencilFlags = kLast_StencilFlag | (kLast_StencilFlag - 1)
};

template<typename TTest, typename TOp> struct GrTStencilFaceSettings {
    uint16_t   fRef;        // Reference value for stencil test and ops.
    TTest      fTest;       // Stencil test function, where fRef is on the left side.
    uint16_t   fTestMask;   // Bitwise "and" to perform on fRef and stencil values before testing.
                            // (e.g. (fRef & fTestMask) < (stencil & fTestMask))
    TOp        fPassOp;     // Op to perform when the test passes.
    TOp        fFailOp;     // Op to perform when the test fails.
    uint16_t   fWriteMask;  // Indicates which bits in the stencil buffer should be updated.
                            // (e.g. stencil = (newValue & fWriteMask) | (stencil & ~fWriteMask))
};

enum class GrUserStencilTest : uint16_t {
    // Tests that respect the clip bit. If a stencil clip is not in effect, the "IfInClip" is
    // ignored and these only act on user bits.
    kAlwaysIfInClip,
    kEqualIfInClip,
    kLessIfInClip,
    kLEqualIfInClip,

    // Tests that ignore the clip bit. The client is responsible to ensure no color write occurs
    // outside the clip if it is in use.
    kAlways,
    kNever,
    kGreater,
    kGEqual,
    kLess,
    kLEqual,
    kEqual,
    kNotEqual
};
constexpr static GrUserStencilTest kLastClippedStencilTest = GrUserStencilTest::kLEqualIfInClip;
constexpr static int kGrUserStencilTestCount = 1 + (int)GrUserStencilTest::kNotEqual;

enum class GrUserStencilOp : uint8_t {
    kKeep,

    // Ops that only modify user bits. These must not be paired with ops that modify the clip bit.
    kZero,
    kReplace, // Replace stencil value with fRef (only the bits enabled in fWriteMask).
    kInvert,
    kIncWrap,
    kDecWrap,
    // These two should only be used if wrap ops are not supported, or if the math is guaranteed
    // to not overflow. The user bits may or may not clamp, depending on the state of non-user bits.
    kIncMaybeClamp,
    kDecMaybeClamp,

    // Ops that only modify the clip bit. These must not be paired with ops that modify user bits.
    kZeroClipBit,
    kSetClipBit,
    kInvertClipBit,

    // Ops that modify both clip and user bits. These can only be paired with kKeep or each other.
    kSetClipAndReplaceUserBits,
    kZeroClipAndUserBits
};
constexpr static GrUserStencilOp kLastUserOnlyStencilOp = GrUserStencilOp::kDecMaybeClamp;
constexpr static GrUserStencilOp kLastClipOnlyStencilOp = GrUserStencilOp::kInvertClipBit;
constexpr static int kGrUserStencilOpCount = 1 + (int)GrUserStencilOp::kZeroClipAndUserBits;

/**
 * This struct is a compile-time constant representation of user stencil settings. It describes in
 * abstract terms how a draw will use the stencil buffer. It gets ODR-used at runtime to define a
 * draw's stencil settings, and is later translated into concrete settings when the pipeline is
 * finalized.
 */
struct GrUserStencilSettings {
    typedef GrTStencilFaceSettings<GrUserStencilTest, GrUserStencilOp> Face;

    template<GrUserStencilTest, GrUserStencilOp PassOp, GrUserStencilOp FailOp> struct Attrs;

    // Unfortunately, this is the only way to pass template arguments to a constructor.
    template<uint16_t Ref, GrUserStencilTest Test, uint16_t TestMask,
             GrUserStencilOp PassOp, GrUserStencilOp FailOp, uint16_t WriteMask> struct Init {};

    template<uint16_t FtRef,            uint16_t BkRef,
             GrUserStencilTest FtTest,  GrUserStencilTest BkTest,
             uint16_t FtTestMask,       uint16_t BkTestMask,
             GrUserStencilOp FtPassOp,  GrUserStencilOp BkPassOp,
             GrUserStencilOp FtFailOp,  GrUserStencilOp BkFailOp,
             uint16_t FtWriteMask,      uint16_t BkWriteMask> struct InitSeparate {};

    template<uint16_t Ref, GrUserStencilTest Test, uint16_t TestMask,
             GrUserStencilOp PassOp, GrUserStencilOp FailOp, uint16_t WriteMask>
    constexpr static Init<Ref, Test, TestMask, PassOp, FailOp, WriteMask> StaticInit() {
        return Init<Ref, Test, TestMask, PassOp, FailOp, WriteMask>();
    }

    template<uint16_t FtRef,            uint16_t BkRef,
             GrUserStencilTest FtTest,  GrUserStencilTest BkTest,
             uint16_t FtTestMask,       uint16_t BkTestMask,
             GrUserStencilOp FtPassOp,  GrUserStencilOp BkPassOp,
             GrUserStencilOp FtFailOp,  GrUserStencilOp BkFailOp,
             uint16_t FtWriteMask,      uint16_t BkWriteMask>
    constexpr static InitSeparate<FtRef, BkRef, FtTest, BkTest, FtTestMask, BkTestMask,
                                  FtPassOp, BkPassOp, FtFailOp, BkFailOp, FtWriteMask,
                                  BkWriteMask> StaticInitSeparate() {
        return InitSeparate<FtRef, BkRef, FtTest, BkTest, FtTestMask, BkTestMask,
                            FtPassOp, BkPassOp, FtFailOp, BkFailOp, FtWriteMask, BkWriteMask>();
    }

    // We construct with template arguments in order to enforce that the struct be compile-time
    // constant and to make use of static asserts.
    template<uint16_t Ref, GrUserStencilTest Test, uint16_t TestMask,
             GrUserStencilOp PassOp, GrUserStencilOp FailOp, uint16_t WriteMask,
             typename Attrs = Attrs<Test, PassOp, FailOp> >
    constexpr explicit GrUserStencilSettings(
            const Init<Ref, Test, TestMask, PassOp, FailOp, WriteMask>&)
        : fFrontFlags{(uint16_t)(Attrs::Flags(false) | kSingleSided_StencilFlag),
                      (uint16_t)(Attrs::Flags(true) | kSingleSided_StencilFlag)}
        , fFront{Ref, Test, Attrs::EffectiveTestMask(TestMask), PassOp, FailOp,
                 Attrs::EffectiveWriteMask(WriteMask)}
        , fBackFlags{(uint16_t)(Attrs::Flags(false) | kSingleSided_StencilFlag),
                     (uint16_t)(Attrs::Flags(true) | kSingleSided_StencilFlag)}
        , fBack{Ref, Test, Attrs::EffectiveTestMask(TestMask), PassOp, FailOp,
                Attrs::EffectiveWriteMask(WriteMask)} {
    }

    template<uint16_t FtRef,            uint16_t BkRef,
             GrUserStencilTest FtTest,  GrUserStencilTest BkTest,
             uint16_t FtTestMask,       uint16_t BkTestMask,
             GrUserStencilOp FtPassOp,  GrUserStencilOp BkPassOp,
             GrUserStencilOp FtFailOp,  GrUserStencilOp BkFailOp,
             uint16_t FtWriteMask,      uint16_t BkWriteMask,
             typename FtAttrs = Attrs<FtTest, FtPassOp, FtFailOp>,
             typename BkAttrs = Attrs<BkTest, BkPassOp, BkFailOp> >
    constexpr explicit GrUserStencilSettings(
            const InitSeparate<FtRef, BkRef, FtTest, BkTest, FtTestMask, BkTestMask,
                               FtPassOp, BkPassOp, FtFailOp, BkFailOp, FtWriteMask, BkWriteMask>&)
        : fFrontFlags{FtAttrs::Flags(false), FtAttrs::Flags(true)}
        , fFront{FtRef, FtTest, FtAttrs::EffectiveTestMask(FtTestMask), FtPassOp, FtFailOp,
                 FtAttrs::EffectiveWriteMask(FtWriteMask)}
        , fBackFlags{BkAttrs::Flags(false), BkAttrs::Flags(true)}
        , fBack{BkRef, BkTest, BkAttrs::EffectiveTestMask(BkTestMask), BkPassOp, BkFailOp,
                BkAttrs::EffectiveWriteMask(BkWriteMask)} {}

    // This struct can only be constructed with static initializers.
    GrUserStencilSettings() = delete;
    GrUserStencilSettings(const GrUserStencilSettings&) = delete;

    uint16_t flags(bool hasStencilClip) const {
        return fFrontFlags[hasStencilClip] & fBackFlags[hasStencilClip];
    }
    bool isDisabled(bool hasStencilClip) const {
        return this->flags(hasStencilClip) & kDisabled_StencilFlag;
    }
    bool testAlwaysPasses(bool hasStencilClip) const {
        return this->flags(hasStencilClip) & kTestAlwaysPasses_StencilFlag;
    }
    bool isTwoSided(bool hasStencilClip) const {
        return !(this->flags(hasStencilClip) & kSingleSided_StencilFlag);
    }
    bool usesWrapOp(bool hasStencilClip) const {
        return !(this->flags(hasStencilClip) & kNoWrapOps_StencilFlag);
    }

    const uint16_t   fFrontFlags[2]; // frontFlagsForDraw = fFrontFlags[hasStencilClip].
    const Face       fFront;
    const uint16_t   fBackFlags[2]; // backFlagsForDraw = fBackFlags[hasStencilClip].
    const Face       fBack;

    static const GrUserStencilSettings& kUnused;

    bool isUnused() const { return this == &kUnused; }
};

template<GrUserStencilTest Test, GrUserStencilOp PassOp, GrUserStencilOp FailOp>
struct GrUserStencilSettings::Attrs {
    // Ensure an op that only modifies user bits isn't paired with one that modifies clip bits.
    GR_STATIC_ASSERT(GrUserStencilOp::kKeep == PassOp || GrUserStencilOp::kKeep == FailOp ||
                     (PassOp <= kLastUserOnlyStencilOp) == (FailOp <= kLastUserOnlyStencilOp));
    // Ensure an op that only modifies clip bits isn't paired with one that modifies clip and user.
    GR_STATIC_ASSERT(GrUserStencilOp::kKeep == PassOp || GrUserStencilOp::kKeep == FailOp ||
                     (PassOp <= kLastClipOnlyStencilOp) == (FailOp <= kLastClipOnlyStencilOp));

    constexpr static bool TestAlwaysPasses(bool hasStencilClip) {
        return (!hasStencilClip && GrUserStencilTest::kAlwaysIfInClip == Test) ||
                GrUserStencilTest::kAlways == Test;
    }
    constexpr static bool DoesNotModifyStencil(bool hasStencilClip) {
        return (GrUserStencilTest::kNever == Test || GrUserStencilOp::kKeep == PassOp) &&
                (TestAlwaysPasses(hasStencilClip) || GrUserStencilOp::kKeep == FailOp);
    }
    constexpr static bool IsDisabled(bool hasStencilClip) {
        return TestAlwaysPasses(hasStencilClip) && DoesNotModifyStencil(hasStencilClip);
    }
    constexpr static bool UsesWrapOps() {
        return GrUserStencilOp::kIncWrap == PassOp || GrUserStencilOp::kDecWrap == PassOp ||
               GrUserStencilOp::kIncWrap == FailOp || GrUserStencilOp::kDecWrap == FailOp;
    }
    constexpr static bool TestIgnoresRef() {
        return (GrUserStencilTest::kAlwaysIfInClip == Test || GrUserStencilTest::kAlways == Test ||
                GrUserStencilTest::kNever == Test);
    }
    constexpr static uint16_t Flags(bool hasStencilClip) {
        return (IsDisabled(hasStencilClip) ? kDisabled_StencilFlag : 0) |
               (TestAlwaysPasses(hasStencilClip) ? kTestAlwaysPasses_StencilFlag : 0) |
               (DoesNotModifyStencil(hasStencilClip) ? kNoModifyStencil_StencilFlag : 0) |
               (UsesWrapOps() ? 0 : kNoWrapOps_StencilFlag);
    }
    constexpr static uint16_t EffectiveTestMask(uint16_t testMask) {
        return TestIgnoresRef() ? 0 : testMask;
    }
    constexpr static uint16_t EffectiveWriteMask(uint16_t writeMask) {
        // We don't modify the mask differently when hasStencilClip=false because either the entire
        // face gets disabled in that case (e.g. Test=kAlwaysIfInClip, PassOp=kKeep), or else the
        // effective mask stays the same either way.
        return DoesNotModifyStencil(true) ? 0 : writeMask;
    }
};

#endif
