
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrStencil_DEFINED
#define GrStencil_DEFINED

#include "GrTypes.h"
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

    // counts
    kStencilFuncCount,
    kClipStencilFuncCount = kNonZeroIfInClip_StencilFunc -
                            kAlwaysIfInClip_StencilFunc + 1,
    kBasicStencilFuncCount = kStencilFuncCount - kClipStencilFuncCount
};

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

    kStencilOpCount
};

/**
 * GrStencilState needs to be a class with accessors and setters so that it
 * can maintain flags related to its current state. However, we also want to
 * be able to declare pre-made stencil settings at compile time (without
 * inserting static initializer code). So all the data members are in this
 * struct. A macro defined after the class can be used to jam an instance of
 * this struct that is created from an initializer list into a
 * GrStencilSettings. (We hang our heads in shame.)
 */
struct GrStencilSettingsStruct {
    GrStencilOp fFrontPassOp : 8;    // op to perform when front faces pass
    GrStencilOp fBackPassOp : 8;     // op to perform when back faces pass
    GrStencilOp fFrontFailOp : 8;    // op to perform when front faces fail
    GrStencilOp fBackFailOp : 8;     // op to perform when back faces fail
    GrStencilFunc fFrontFunc : 8;    // test function for front faces
    GrStencilFunc fBackFunc : 8;     // test function for back faces
    unsigned short fFrontFuncMask;   // mask for front face test
    unsigned short fBackFuncMask;    // mask for back face test
    unsigned short fFrontFuncRef;    // reference value for front face test
    unsigned short fBackFuncRef;     // reference value for back face test
    unsigned short fFrontWriteMask;  // stencil write mask for front faces
    unsigned short fBackWriteMask;   // stencil write mask for back faces
};

/**
 * Class representing stencil state.
 */
class GrStencilSettings : private GrStencilSettingsStruct {

public:
    bool operator == (const GrStencilSettings& s) const {
        // make sure this is tightly packed (< 4B padding).
        GR_STATIC_ASSERT(sizeof(GrStencilSettings) / 4 ==
                        (4*sizeof(uint8_t) +
                         2*sizeof(uint8_t) +
                         6*sizeof(unsigned short) + 3) / 4);
        return 0 == memcmp(this, &s, sizeof(GrStencilSettings));
    }

    bool operator != (const GrStencilSettings& s) const {
        return !(*this == s);
    }

    GrStencilSettings& operator =(const GrStencilSettings& s) {
        memcpy(this, &s, sizeof(GrStencilSettings));
        return *this;
    }

    GrStencilOp frontPassOp() const { return fFrontPassOp; }
    GrStencilOp backPassOp() const { return fBackPassOp; }
    GrStencilOp frontFailOp() const { return fFrontFailOp; }
    GrStencilOp backFailOp() const { return fBackFailOp; }
    GrStencilFunc frontFunc() const { return fFrontFunc; }
    GrStencilFunc backFunc() const { return fBackFunc; }
    unsigned short frontFuncMask() const { return fFrontFuncMask; }
    unsigned short backFuncMask() const { return fBackFuncMask; }
    unsigned short frontFuncRef() const { return fFrontFuncRef; }
    unsigned short backFuncRef() const { return fBackFuncRef; }
    unsigned short frontWriteMask() const {return fFrontWriteMask; }
    unsigned short backWriteMask() const { return fBackWriteMask; }

    void setFrontPassOp(GrStencilOp op) { fFrontPassOp = op; }
    void setBackPassOp(GrStencilOp op) { fBackPassOp = op; }
    void setFrontFailOp(GrStencilOp op) {fFrontFailOp = op; }
    void setBackFailOp(GrStencilOp op) { fBackFailOp = op; }
    void setFrontFunc(GrStencilFunc func) { fFrontFunc = func; }
    void setBackFunc(GrStencilFunc func) { fBackFunc = func; }
    void setFrontFuncMask(unsigned short mask) { fFrontFuncMask = mask; }
    void setBackFuncMask(unsigned short mask) { fBackFuncMask = mask; }
    void setFrontFuncRef(unsigned short ref) { fFrontFuncRef = ref; }
    void setBackFuncRef(unsigned short ref) { fBackFuncRef = ref; }
    void setFrontWriteMask(unsigned short writeMask) { fFrontWriteMask = writeMask; }
    void setBackWriteMask(unsigned short writeMask) { fBackWriteMask = writeMask; }

    void setSame(GrStencilOp passOp,
                 GrStencilOp failOp,
                 GrStencilFunc func,
                 unsigned short funcMask,
                 unsigned short funcRef,
                 unsigned short writeMask) {
        fFrontPassOp        = passOp;
        fBackPassOp         = passOp;
        fFrontFailOp        = failOp;
        fBackFailOp         = failOp;
        fFrontFunc          = func;
        fBackFunc           = func;
        fFrontFuncMask      = funcMask;
        fBackFuncMask       = funcMask;
        fFrontFuncRef       = funcRef;
        fBackFuncRef        = funcRef;
        fFrontWriteMask     = writeMask;
        fBackWriteMask      = writeMask;
    }

    void setDisabled() {
        memset(this, 0, sizeof(*this));
        GR_STATIC_ASSERT(0 == kKeep_StencilOp);
        GR_STATIC_ASSERT(0 == kAlways_StencilFunc);
    }
    bool isDisabled() const {
        return kKeep_StencilOp == fFrontPassOp   &&
               kKeep_StencilOp == fBackPassOp    &&
               kKeep_StencilOp == fFrontFailOp   &&
               kKeep_StencilOp == fBackFailOp   &&
               kAlways_StencilFunc == fFrontFunc &&
               kAlways_StencilFunc == fBackFunc;
    }
    bool doesWrite() const {
        return !((kNever_StencilFunc == fFrontFunc ||
                  kKeep_StencilOp == fFrontPassOp)  &&
                 (kNever_StencilFunc == fBackFunc ||
                  kKeep_StencilOp == fBackPassOp)    &&
                 (kAlways_StencilFunc == fFrontFunc ||
                  kKeep_StencilOp == fFrontFailOp)  &&
                 (kAlways_StencilFunc == fBackFunc ||
                  kKeep_StencilOp == fBackFailOp));
    }
    void invalidate()  {
        // just write an illegal value to the first member
        fFrontPassOp = (GrStencilOp)(uint8_t)-1;
    }

private:
    friend class GrGpu;

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
    static bool GetClipPasses(GrSetOp op, 
                              bool canBeDirect,
                              unsigned int stencilClipMask,
                              bool invertedFill,
                              int* numPasses,
                              GrStencilSettings settings[kMaxStencilClipPasses]);
};

GR_STATIC_ASSERT(sizeof(GrStencilSettingsStruct) == sizeof(GrStencilSettings));

#define GR_STATIC_CONST_STENCIL(NAME,                                        \
    FRONT_PASS_OP,    BACK_PASS_OP,                                          \
    FRONT_FAIL_OP,    BACK_FAIL_OP,                                          \
    FRONT_FUNC,       BACK_FUNC,                                             \
    FRONT_MASK,       BACK_MASK,                                             \
    FRONT_REF,        BACK_REF,                                              \
    FRONT_WRITE_MASK, BACK_WRITE_MASK)                                       \
    static const GrStencilSettingsStruct NAME ## _STRUCT = {                 \
        (FRONT_PASS_OP),    (BACK_PASS_OP),                                  \
        (FRONT_FAIL_OP),    (BACK_FAIL_OP),                                  \
        (FRONT_FUNC),       (BACK_FUNC),                                     \
        (FRONT_MASK),       (BACK_MASK),                                     \
        (FRONT_REF),        (BACK_REF),                                      \
        (FRONT_WRITE_MASK), (BACK_WRITE_MASK)                                \
    };                                                                       \
    static const GrStencilSettings& NAME =                                   \
        *static_cast<const GrStencilSettings*>(&(NAME ## _STRUCT))
#endif

#define GR_STATIC_CONST_SAME_STENCIL(NAME,                                   \
    PASS_OP, FAIL_OP, FUNC, MASK, REF, WRITE_MASK)                           \
    GR_STATIC_CONST_STENCIL(NAME, (PASS_OP), (PASS_OP), (FAIL_OP),           \
    (FAIL_OP), (FUNC), (FUNC), (MASK), (MASK), (REF), (REF), (WRITE_MASK),   \
    (WRITE_MASK))
