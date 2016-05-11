/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrStencil_DEFINED
#define GrStencil_DEFINED

#include "GrTypes.h"
#include "SkRegion.h"

class GrProcessorKeyBuilder;

/**
 * Gr uses the stencil buffer to implement complex clipping inside the
 * GrDrawTarget class. The GrDrawTarget makes a subset of the stencil buffer
 * bits available for other uses by external code (clients). Client code can
 * modify these bits. GrDrawTarget will ignore ref, mask, and writemask bits
 * provided by clients that overlap the bits used to implement clipping.
 *
 * When code outside the GrDrawTarget class uses the stencil buffer the contract
 * is as follows:
 *
 * > Normal stencil funcs allow the client to pass / fail regardless of the
 *   reserved clip bits.
 * > Additional functions allow a test against the clip along with a limited
 *   set of tests against the client bits.
 * > Client can assume all client bits are zero initially.
 * > Client must ensure that after all its passes are finished it has only
 *   written to the color buffer in the region inside the clip. Furthermore, it
 *   must zero all client bits that were modifed (both inside and outside the
 *   clip).
 */

/**
 * Determines which pixels pass / fail the stencil test.
 * Stencil test passes if (ref & mask) FUNC (stencil & mask) is true
 */
enum GrStencilFunc {
    kAlways_StencilFunc = 0,
    kNever_StencilFunc,
    kGreater_StencilFunc,
    kGEqual_StencilFunc,
    kLess_StencilFunc,
    kLEqual_StencilFunc,
    kEqual_StencilFunc,
    kNotEqual_StencilFunc,

    // Gr stores the current clip in the
    // stencil buffer in the high bits that
    // are not directly accessible modifiable
    // via the GrDrawTarget interface. The below
    // stencil funcs test against the current
    // clip in addition to the GrDrawTarget
    // client's stencil bits.

    // pass if inside the clip
    kAlwaysIfInClip_StencilFunc,
    kEqualIfInClip_StencilFunc,
    kLessIfInClip_StencilFunc,
    kLEqualIfInClip_StencilFunc,
    kNonZeroIfInClip_StencilFunc, // this one forces the ref to be 0

    kLast_StencilFunc = kNonZeroIfInClip_StencilFunc
};

static const int kStencilFuncCnt = kLast_StencilFunc + 1;
static const int kClipStencilFuncCnt =
    kNonZeroIfInClip_StencilFunc - kAlwaysIfInClip_StencilFunc + 1;
static const int kBasicStencilFuncCnt = kStencilFuncCnt - kClipStencilFuncCnt;

/**
 * Operations to perform based on whether stencil test passed failed.
 */
enum GrStencilOp {
    kKeep_StencilOp = 0,    // preserve existing stencil value
    kReplace_StencilOp,     // replace with reference value from stencl test
    kIncWrap_StencilOp,     // increment and wrap at max
    kIncClamp_StencilOp,    // increment and clamp at max
    kDecWrap_StencilOp,     // decrement and wrap at 0
    kDecClamp_StencilOp,    // decrement and clamp at 0
    kZero_StencilOp,        // zero stencil bits
    kInvert_StencilOp,      // invert stencil bits
    kLast_StencilOp = kInvert_StencilOp
};
static const int kStencilOpCnt = kLast_StencilOp + 1;

/**
 * Class representing stencil state.
 */
class GrStencilSettings {
public:
    enum Face {
        kFront_Face = 0,
        kBack_Face  = 1,
    };

    constexpr GrStencilSettings(GrStencilOp passOp,
                                GrStencilOp failOp,
                                GrStencilFunc func,
                                unsigned short funcMask,
                                unsigned short funcRef,
                                unsigned short writeMask)
        : fPassOps{(uint8_t)passOp, (uint8_t)passOp}
        , fFailOps{(uint8_t)failOp, (uint8_t)failOp}
        , fFuncs{(uint8_t)func, (uint8_t)func}
        , fPad0(0)
        , fPad1(0)
        , fFuncMasks{funcMask, funcMask}
        , fFuncRefs{funcRef, funcRef}
        , fWriteMasks{writeMask, writeMask}
        , fFlags(ComputeFlags(passOp,    passOp,
                              failOp,    failOp,
                              func,      func,
                              writeMask, writeMask)) {
    }

    constexpr GrStencilSettings(GrStencilOp frontPassOp,  GrStencilOp backPassOp,
                                GrStencilOp frontFailOp,  GrStencilOp backFailOp,
                                GrStencilFunc frontFunc,  GrStencilFunc backFunc,
                                uint16_t frontFuncMask,   uint16_t backFuncMask,
                                uint16_t frontFuncRef,    uint16_t backFuncRef,
                                uint16_t frontWriteMask,  uint16_t backWriteMask)
        : fPassOps{(uint8_t)frontPassOp, (uint8_t)backPassOp}
        , fFailOps{(uint8_t)frontFailOp, (uint8_t)backFailOp}
        , fFuncs{(uint8_t)frontFunc, (uint8_t)backFunc}
        , fPad0(0)
        , fPad1(0)
        , fFuncMasks{frontFuncMask, backFuncMask}
        , fFuncRefs{frontFuncRef, backFuncRef}
        , fWriteMasks{frontWriteMask, backWriteMask}
        , fFlags(ComputeFlags(frontPassOp,    backPassOp,
                              frontFailOp,    backFailOp,
                              frontFunc,      backFunc,
                              frontWriteMask, backWriteMask)) {
    }

    GrStencilSettings() {
        fPad0 = fPad1 = 0;
        this->setDisabled();
    }

    GrStencilOp passOp(Face f) const { return static_cast<GrStencilOp>(fPassOps[f]); }
    GrStencilOp failOp(Face f) const { return static_cast<GrStencilOp>(fFailOps[f]); }
    GrStencilFunc func(Face f) const { return static_cast<GrStencilFunc>(fFuncs[f]); }
    uint16_t funcMask(Face f) const  { return fFuncMasks[f]; }
    uint16_t funcRef(Face f) const   { return fFuncRefs[f]; }
    uint16_t writeMask(Face f) const { return fWriteMasks[f]; }

    void setPassOp(Face f, GrStencilOp op) { fPassOps[f] = op; fFlags = 0;}
    void setFailOp(Face f, GrStencilOp op) { fFailOps[f] = op; fFlags = 0;}
    void setFunc(Face f, GrStencilFunc func) { fFuncs[f] = func; fFlags = 0;}
    void setFuncMask(Face f, unsigned short mask) { fFuncMasks[f] = mask; }
    void setFuncRef(Face f, unsigned short ref) { fFuncRefs[f] = ref; }
    void setWriteMask(Face f, unsigned short writeMask) { fWriteMasks[f] = writeMask; }

    void copyFrontSettingsToBack() {
        fPassOps[kBack_Face]    = fPassOps[kFront_Face];
        fFailOps[kBack_Face]    = fFailOps[kFront_Face];
        fFuncs[kBack_Face]      = fFuncs[kFront_Face];
        fFuncMasks[kBack_Face]  = fFuncMasks[kFront_Face];
        fFuncRefs[kBack_Face]   = fFuncRefs[kFront_Face];
        fWriteMasks[kBack_Face] = fWriteMasks[kFront_Face];
        fFlags = 0;
    }

    void setDisabled() {
        memset(this, 0, sizeof(*this));
        GR_STATIC_ASSERT(0 == kKeep_StencilOp);
        GR_STATIC_ASSERT(0 == kAlways_StencilFunc);
        fFlags = kIsDisabled_StencilFlag | kDoesNotWrite_StencilFlag;
    }

    bool isTwoSided() const {
        return fPassOps[kFront_Face]    != fPassOps[kBack_Face]   ||
               fFailOps[kFront_Face]    != fFailOps[kBack_Face]   ||
               fFuncs[kFront_Face]      != fFuncs[kBack_Face]     ||
               fFuncMasks[kFront_Face]  != fFuncMasks[kBack_Face] ||
               fFuncRefs[kFront_Face]   != fFuncRefs[kBack_Face]  ||
               fWriteMasks[kFront_Face] != fWriteMasks[kBack_Face];
    }

    bool usesWrapOp() const {
        return kIncWrap_StencilOp == fPassOps[kFront_Face] ||
               kDecWrap_StencilOp == fPassOps[kFront_Face] ||
               kIncWrap_StencilOp == fPassOps[kBack_Face]  ||
               kDecWrap_StencilOp == fPassOps[kBack_Face]  ||
               kIncWrap_StencilOp == fFailOps[kFront_Face] ||
               kDecWrap_StencilOp == fFailOps[kFront_Face] ||
               kIncWrap_StencilOp == fFailOps[kBack_Face]  ||
               kDecWrap_StencilOp == fFailOps[kBack_Face];
    }

    bool isDisabled() const {
        if (fFlags & kIsDisabled_StencilFlag) {
            return true;
        }
        if (fFlags & kNotDisabled_StencilFlag) {
            return false;
        }
        bool disabled = this->computeIsDisabled();
        fFlags |= disabled ? kIsDisabled_StencilFlag : kNotDisabled_StencilFlag;
        return disabled;
    }

    bool doesWrite() const {
        if (fFlags & kDoesWrite_StencilFlag) {
            return true;
        }
        if (fFlags & kDoesNotWrite_StencilFlag) {
            return false;
        }
        bool writes = this->computeDoesWrite();
        fFlags |= writes ? kDoesWrite_StencilFlag : kDoesNotWrite_StencilFlag;
        return writes;
    }

    void invalidate()  {
        // write an illegal value to the first member
        fPassOps[0] = kStencilOpCnt;
        fFlags = 0;
    }

    bool isValid() const { return fPassOps[0] < kStencilOpCnt; }

    void genKey(GrProcessorKeyBuilder* b) const;

    bool operator==(const GrStencilSettings& s) const {
        static const size_t gCompareSize = sizeof(GrStencilSettings) -
                                           sizeof(fFlags);
        SkASSERT((const char*)&fFlags + sizeof(fFlags) ==
                 (const char*)this + sizeof(GrStencilSettings));
        if (this->isDisabled() & s.isDisabled()) { // using & not &&
            return true;
        }
        return 0 == memcmp(this, &s, gCompareSize);
    }

    bool operator!=(const GrStencilSettings& s) const {
        return !(*this == s);
    }

    GrStencilSettings& operator=(const GrStencilSettings& s) {
        memcpy(this, &s, sizeof(GrStencilSettings));
        return *this;
    }

private:
    friend class GrClipMaskManager;

    enum {
        kMaxStencilClipPasses = 2  // maximum number of passes to add a clip
                                   // element to the stencil buffer.
    };

    /**
     * Given a thing to draw into the stencil clip, a fill type, and a set op
     * this function determines:
     *      1. Whether the thing can be draw directly to the stencil clip or
     *      needs to be drawn to the client portion of the stencil first.
     *      2. How many passes are needed.
     *      3. What those passes are.
     *      4. The fill rule that should actually be used to render (will
     *         always be non-inverted).
     *
     * @param op                the set op to combine this element with the
     *                          existing clip
     * @param stencilClipMask   mask with just the stencil bit used for clipping
     *                          enabled.
     * @param invertedFill      is this path inverted
     * @param numPasses         out: the number of passes needed to add the
     *                               element to the clip.
     * @param settings          out: the stencil settings to use for each pass
     *
     * @return true if the clip element's geometry can be drawn directly to the
     *         stencil clip bit. Will only be true if canBeDirect is true.
     *         numPasses will be 1 if return value is true.
     */
    static bool GetClipPasses(SkRegion::Op op,
                              bool canBeDirect,
                              unsigned int stencilClipMask,
                              bool invertedFill,
                              int* numPasses,
                              GrStencilSettings settings[kMaxStencilClipPasses]);

    constexpr static bool IsDisabled(GrStencilOp frontPassOp,  GrStencilOp backPassOp,
                                     GrStencilOp frontFailOp,  GrStencilOp backFailOp,
                                     GrStencilFunc frontFunc,  GrStencilFunc backFunc) {
        return (((frontPassOp == kKeep_StencilOp && frontFailOp == kKeep_StencilOp)) &&
                ((backPassOp  == kKeep_StencilOp &&  backFailOp == kKeep_StencilOp))  &&
                frontFunc   == kAlways_StencilFunc &&
                backFunc   == kAlways_StencilFunc);
    }

    constexpr static bool DoesWrite(GrStencilOp frontPassOp,  GrStencilOp backPassOp,
                                    GrStencilOp frontFailOp,  GrStencilOp backFailOp,
                                    GrStencilFunc frontFunc,  GrStencilFunc backFunc,
                                    uint16_t frontWriteMask,  uint16_t backWriteMask) {
        return (0 != (frontWriteMask | backWriteMask)) &&
                // Can we write due to a front face passing the stencil test?
               ((frontFunc != kNever_StencilFunc  && frontPassOp != kKeep_StencilOp) ||
                // Can we write due to a back face passing the stencil test?
                (backFunc  != kNever_StencilFunc  && backPassOp  != kKeep_StencilOp) ||
                // Can we write due to a front face failing the stencil test?
                (frontFunc != kAlways_StencilFunc && frontFailOp != kKeep_StencilOp) ||
                // Can we write due to a back face failing the stencil test?
                (backFunc  != kAlways_StencilFunc && backFailOp  != kKeep_StencilOp));
    }

    constexpr static uint32_t ComputeFlags(GrStencilOp frontPassOp,  GrStencilOp backPassOp,
                                           GrStencilOp frontFailOp,  GrStencilOp backFailOp,
                                           GrStencilFunc frontFunc,  GrStencilFunc backFunc,
                                           uint16_t frontWriteMask,  uint16_t backWriteMask) {
        return (IsDisabled(frontPassOp, backPassOp, frontFailOp, backFailOp,
                           frontFunc, backFunc)
                    ? kIsDisabled_StencilFlag
                    : kNotDisabled_StencilFlag) |
               (DoesWrite(frontPassOp, backPassOp, frontFailOp, backFailOp,
                          frontFunc, backFunc, frontWriteMask, backWriteMask)
                    ? kDoesWrite_StencilFlag
                    : kDoesNotWrite_StencilFlag);
    }

    bool computeIsDisabled() const {
        return IsDisabled((GrStencilOp) fPassOps[kFront_Face], (GrStencilOp) fPassOps[kBack_Face],
                          (GrStencilOp) fFailOps[kFront_Face], (GrStencilOp) fFailOps[kBack_Face],
                          (GrStencilFunc) fFuncs[kFront_Face], (GrStencilFunc) fFuncs[kBack_Face]);
    }
    bool computeDoesWrite() const {
        return DoesWrite((GrStencilOp)fPassOps[kFront_Face], (GrStencilOp)fPassOps[kBack_Face],
                         (GrStencilOp)fFailOps[kFront_Face], (GrStencilOp)fFailOps[kBack_Face],
                         (GrStencilFunc)fFuncs[kFront_Face], (GrStencilFunc)fFuncs[kBack_Face],
                         fWriteMasks[kFront_Face],           fWriteMasks[kBack_Face]);
    }

    enum GrStencilFlags {
        kIsDisabled_StencilFlag      = 0x1,
        kNotDisabled_StencilFlag     = 0x2,
        kDoesWrite_StencilFlag       = 0x4,
        kDoesNotWrite_StencilFlag    = 0x8,
    };

    uint8_t fPassOps[2];     // op to perform when faces pass (GrStencilOp)
    uint8_t fFailOps[2];     // op to perform when faces fail (GrStencilOp)
    uint8_t fFuncs[2];       // test function for faces (GrStencilFunc)
    uint8_t fPad0;
    uint8_t fPad1;
    uint16_t fFuncMasks[2];  // mask for face tests
    uint16_t fFuncRefs[2];   // reference values for face tests
    uint16_t fWriteMasks[2]; // stencil write masks
    mutable uint32_t fFlags;

};

// We rely on this being packed and aligned (memcmp'ed and memcpy'ed)
GR_STATIC_ASSERT(sizeof(GrStencilSettings) % 4 == 0);
GR_STATIC_ASSERT(sizeof(GrStencilSettings) ==
                 4*sizeof(uint8_t) + // ops
                 2*sizeof(uint8_t) + // funcs
                 2*sizeof(uint8_t) + // pads
                 2*sizeof(uint16_t) + // func masks
                 2*sizeof(uint16_t) + // ref values
                 2*sizeof(uint16_t) + // write masks
                 sizeof(uint32_t)); // flags

#endif
