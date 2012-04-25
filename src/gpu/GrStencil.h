
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

enum GrStencilFlags {
    kIsDisabled_StencilFlag      = 0x1,
    kNotDisabled_StencilFlag     = 0x2,
    kDoesWrite_StencilFlag       = 0x4,
    kDoesNotWrite_StencilFlag    = 0x8,
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
    int fPad0 : 8;
    int fPad1 : 8;
    unsigned short fFrontFuncMask;   // mask for front face test
    unsigned short fBackFuncMask;    // mask for back face test
    unsigned short fFrontFuncRef;    // reference value for front face test
    unsigned short fBackFuncRef;     // reference value for back face test
    unsigned short fFrontWriteMask;  // stencil write mask for front faces
    unsigned short fBackWriteMask;   // stencil write mask for back faces
    mutable uint32_t fFlags;
};
// We rely on this being packed and aligned (memcmp'ed and memcpy'ed)
GR_STATIC_ASSERT(sizeof(GrStencilSettingsStruct) % 4 == 0);
GR_STATIC_ASSERT(sizeof(GrStencilSettingsStruct) ==
                 4*sizeof(uint8_t) + // ops
                 2*sizeof(uint8_t) + // funcs
                 2*sizeof(uint8_t) + // pads
                 2*sizeof(unsigned short) + // func masks
                 2*sizeof(unsigned short) + // ref values
                 2*sizeof(unsigned short) + // write masks
                 sizeof(uint32_t)); // flags

// This macro is used to compute the GrStencilSettingsStructs flags
// associated to disabling. It is used both to define constant structure
// initializers and inside GrStencilSettings::isDisabled()
//
#define GR_STENCIL_SETTINGS_IS_DISABLED(                                     \
    FRONT_PASS_OP,    BACK_PASS_OP,                                          \
    FRONT_FAIL_OP,    BACK_FAIL_OP,                                          \
    FRONT_FUNC,       BACK_FUNC)                                             \
    ((FRONT_PASS_OP) == kKeep_StencilOp &&                                   \
     (BACK_PASS_OP)  == kKeep_StencilOp &&                                   \
     (FRONT_FAIL_OP) == kKeep_StencilOp &&                                   \
     (BACK_FAIL_OP)  == kKeep_StencilOp &&                                   \
     (FRONT_FUNC)    == kAlways_StencilFunc &&                               \
     (BACK_FUNC)     == kAlways_StencilFunc)

#define GR_STENCIL_SETTINGS_DOES_WRITE(                                      \
    FRONT_PASS_OP,    BACK_PASS_OP,                                          \
    FRONT_FAIL_OP,    BACK_FAIL_OP,                                          \
    FRONT_FUNC,       BACK_FUNC)                                             \
    (!(((FRONT_FUNC) == kNever_StencilFunc  ||                               \
        (FRONT_PASS_OP) == kKeep_StencilOp)  &&                              \
       ((BACK_FUNC) == kNever_StencilFunc  ||                                \
        (BACK_PASS_OP)  == kKeep_StencilOp) &&                               \
       ((FRONT_FUNC) == kAlways_StencilFunc ||                               \
        (FRONT_FAIL_OP) == kKeep_StencilOp) &&                               \
       ((BACK_FUNC)  == kAlways_StencilFunc ||                               \
        (BACK_FAIL_OP)  == kKeep_StencilOp)))

#define GR_STENCIL_SETTINGS_DEFAULT_FLAGS(                                   \
    FRONT_PASS_OP,    BACK_PASS_OP,                                          \
    FRONT_FAIL_OP,    BACK_FAIL_OP,                                          \
    FRONT_FUNC,       BACK_FUNC)                                             \
  ((GR_STENCIL_SETTINGS_IS_DISABLED(FRONT_PASS_OP,BACK_PASS_OP,              \
      FRONT_FAIL_OP,BACK_FAIL_OP,FRONT_FUNC,BACK_FUNC) ?                     \
      kIsDisabled_StencilFlag : kNotDisabled_StencilFlag) |                  \
   (GR_STENCIL_SETTINGS_DOES_WRITE(FRONT_PASS_OP,BACK_PASS_OP,               \
      FRONT_FAIL_OP,BACK_FAIL_OP,FRONT_FUNC,BACK_FUNC) ?                     \
      kDoesWrite_StencilFlag : kDoesNotWrite_StencilFlag))

/**
 * Class representing stencil state.
 */
class GrStencilSettings : private GrStencilSettingsStruct {

public:
    GrStencilSettings() {
        fPad0 = fPad1 = 0;
        this->setDisabled();
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

    void setFrontPassOp(GrStencilOp op) { fFrontPassOp = op; fFlags = 0;}
    void setBackPassOp(GrStencilOp op) { fBackPassOp = op; fFlags = 0;}
    void setFrontFailOp(GrStencilOp op) {fFrontFailOp = op; fFlags = 0;}
    void setBackFailOp(GrStencilOp op) { fBackFailOp = op; fFlags = 0;}
    void setFrontFunc(GrStencilFunc func) { fFrontFunc = func; fFlags = 0;}
    void setBackFunc(GrStencilFunc func) { fBackFunc = func; fFlags = 0;}
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
        fFlags = 0;
    }

    void setDisabled() {
        memset(this, 0, sizeof(*this));
        GR_STATIC_ASSERT(0 == kKeep_StencilOp);
        GR_STATIC_ASSERT(0 == kAlways_StencilFunc);
        fFlags = kIsDisabled_StencilFlag | kDoesNotWrite_StencilFlag;
    }

    bool isDisabled() const {
        if (fFlags & kIsDisabled_StencilFlag) {
            return true;
        }
        if (fFlags & kNotDisabled_StencilFlag) {
            return false;
        }
        bool disabled = GR_STENCIL_SETTINGS_IS_DISABLED(
                            fFrontPassOp, fBackPassOp,
                            fFrontFailOp, fBackFailOp,
                            fFrontFunc ,fBackFunc);
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
        bool writes = GR_STENCIL_SETTINGS_DOES_WRITE(
                        fFrontPassOp, fBackPassOp,
                        fFrontFailOp, fBackFailOp,
                        fFrontFunc, fBackFunc);
        fFlags |= writes ? kDoesWrite_StencilFlag : kDoesNotWrite_StencilFlag;
        return writes;
    }
    
    void invalidate()  {
        // write an illegal value to the first member
        fFrontPassOp = (GrStencilOp)(uint8_t)-1;
        fFlags = 0;
    }

    bool operator == (const GrStencilSettings& s) const {
        static const size_t gCompareSize = sizeof(GrStencilSettings) -
                                           sizeof(fFlags);
        GrAssert((const char*)&fFlags + sizeof(fFlags) == 
                 (const char*)this + sizeof(GrStencilSettings));
        if (this->isDisabled() & s.isDisabled()) { // using & not &&
            return true;
        }
        return 0 == memcmp(this, &s, gCompareSize);
    }
    
    bool operator != (const GrStencilSettings& s) const {
        return !(*this == s);
    }
    
    GrStencilSettings& operator =(const GrStencilSettings& s) {
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
};

GR_STATIC_ASSERT(sizeof(GrStencilSettingsStruct) == sizeof(GrStencilSettings));

#define GR_STATIC_CONST_STENCIL_STRUCT(STRUCT_NAME,                          \
    FRONT_PASS_OP,    BACK_PASS_OP,                                          \
    FRONT_FAIL_OP,    BACK_FAIL_OP,                                          \
    FRONT_FUNC,       BACK_FUNC,                                             \
    FRONT_MASK,       BACK_MASK,                                             \
    FRONT_REF,        BACK_REF,                                              \
    FRONT_WRITE_MASK, BACK_WRITE_MASK)                                       \
    static const GrStencilSettingsStruct STRUCT_NAME = {                     \
        (FRONT_PASS_OP),    (BACK_PASS_OP),                                  \
        (FRONT_FAIL_OP),    (BACK_FAIL_OP),                                  \
        (FRONT_FUNC),       (BACK_FUNC),                                     \
        (0),                (0),                                             \
        (FRONT_MASK),       (BACK_MASK),                                     \
        (FRONT_REF),        (BACK_REF),                                      \
        (FRONT_WRITE_MASK), (BACK_WRITE_MASK),                               \
        GR_STENCIL_SETTINGS_DEFAULT_FLAGS(                                   \
            FRONT_PASS_OP, BACK_PASS_OP, FRONT_FAIL_OP, BACK_FAIL_OP,        \
            FRONT_FUNC, BACK_FUNC)                                           \
    };

#define GR_CONST_STENCIL_SETTINGS_PTR_FROM_STRUCT_PTR(STRUCT_PTR)            \
    reinterpret_cast<const GrStencilSettings*>(STRUCT_PTR)

#define GR_STATIC_CONST_SAME_STENCIL_STRUCT(STRUCT_NAME,                     \
    PASS_OP, FAIL_OP, FUNC, MASK, REF, WRITE_MASK)                           \
    GR_STATIC_CONST_STENCIL_STRUCT(STRUCT_NAME, (PASS_OP), (PASS_OP),        \
    (FAIL_OP),(FAIL_OP), (FUNC), (FUNC), (MASK), (MASK), (REF), (REF),       \
    (WRITE_MASK),(WRITE_MASK))

#define GR_STATIC_CONST_STENCIL(NAME,                                        \
    FRONT_PASS_OP,    BACK_PASS_OP,                                          \
    FRONT_FAIL_OP,    BACK_FAIL_OP,                                          \
    FRONT_FUNC,       BACK_FUNC,                                             \
    FRONT_MASK,       BACK_MASK,                                             \
    FRONT_REF,        BACK_REF,                                              \
    FRONT_WRITE_MASK, BACK_WRITE_MASK)                                       \
    GR_STATIC_CONST_STENCIL_STRUCT(NAME ## _STRUCT,                          \
    (FRONT_PASS_OP),(BACK_PASS_OP),(FRONT_FAIL_OP),(BACK_FAIL_OP),           \
    (FRONT_FUNC),(BACK_FUNC),(FRONT_MASK),(BACK_MASK),                       \
    (FRONT_REF),(BACK_REF),(FRONT_WRITE_MASK),(BACK_WRITE_MASK))             \
    static const GrStencilSettings& NAME =                                   \
        *GR_CONST_STENCIL_SETTINGS_PTR_FROM_STRUCT_PTR(&(NAME ## _STRUCT));


#define GR_STATIC_CONST_SAME_STENCIL(NAME,                                   \
    PASS_OP, FAIL_OP, FUNC, MASK, REF, WRITE_MASK)                           \
    GR_STATIC_CONST_STENCIL(NAME, (PASS_OP), (PASS_OP), (FAIL_OP),           \
    (FAIL_OP), (FUNC), (FUNC), (MASK), (MASK), (REF), (REF), (WRITE_MASK),   \
    (WRITE_MASK))

#endif
