
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrStencil.h"

const GrStencilSettings GrStencilSettings::gDisabled = {
    kKeep_StencilOp,     kKeep_StencilOp,
    kKeep_StencilOp,     kKeep_StencilOp,
    kAlways_StencilFunc, kAlways_StencilFunc,
    0x0,                 0x0,
    0x0,                 0x0,
    0x0,                 0x0
};
GR_STATIC_ASSERT(0 == kKeep_StencilOp);
GR_STATIC_ASSERT(0 == kAlways_StencilFunc);

////////////////////////////////////////////////////////////////////////////////
// Stencil Rules for Merging user stencil space into clip

// We can't include the clip bit in the ref or mask values because the division
// between user and clip bits in the stencil depends on the number of stencil
// bits in the runtime. Comments below indicate what the code should do to
// incorporate the clip bit into these settings.

///////
// Replace

// set the ref to be the clip bit, but mask it out for the test
static const GrStencilSettings gUserToClipReplace = {
    kReplace_StencilOp,  kReplace_StencilOp,
    kZero_StencilOp,     kZero_StencilOp,
    kLess_StencilFunc,   kLess_StencilFunc,
    0xffffffff,          0xffffffff,    // unset clip bit
    0x0,                 0x0,           // set clip bit
    0xffffffff,          0xffffffff
};
static const GrStencilSettings gInvUserToClipReplace = {
    kReplace_StencilOp,  kReplace_StencilOp,
    kZero_StencilOp,     kZero_StencilOp,
    kEqual_StencilFunc,  kEqual_StencilFunc,
    0xffffffff,          0xffffffff,    // unset clip bit
    0x0,                 0x0,           // set clip bit
    0xffffffff,          0xffffffff
};

///////
// Intersect
static const GrStencilSettings gUserToClipIsect = {
    kReplace_StencilOp,  kReplace_StencilOp,
    kZero_StencilOp,     kZero_StencilOp,
    kLess_StencilFunc,   kLess_StencilFunc,
    0xffffffff,          0xffffffff,
    0x0,                 0x0,           // set clip bit
    0xffffffff,          0xffffffff
};
static const GrStencilSettings gInvUserToClipIsect = {
    kReplace_StencilOp,  kReplace_StencilOp,
    kZero_StencilOp,     kZero_StencilOp,
    kEqual_StencilFunc,  kEqual_StencilFunc,
    0xffffffff,          0xffffffff,
    0x0,                 0x0,           // set clip bit
    0xffffffff,          0xffffffff
};

///////
// Difference
static const GrStencilSettings gUserToClipDiff = {
    kReplace_StencilOp,  kReplace_StencilOp,
    kZero_StencilOp,     kZero_StencilOp,
    kEqual_StencilFunc,  kEqual_StencilFunc,
    0xffffffff,          0xffffffff,
    0x0,                 0x0,           // set clip bit
    0xffffffff,          0xffffffff
};
static const GrStencilSettings gInvUserToClipDiff = {
    kReplace_StencilOp,  kReplace_StencilOp,
    kZero_StencilOp,     kZero_StencilOp,
    kLess_StencilFunc,   kLess_StencilFunc,
    0xffffffff,          0xffffffff,
    0x0,                 0x0,           // set clip bit
    0xffffffff,          0xffffffff
};

///////
// Union

// first pass makes all the passing cases >= just clip bit set.
static const GrStencilSettings gUserToClipUnionPass0 = {
    kReplace_StencilOp,  kReplace_StencilOp,
    kKeep_StencilOp,     kKeep_StencilOp,
    kLEqual_StencilFunc, kLEqual_StencilFunc,
    0xffffffff,          0xffffffff,    // unset clip bit
    0x00000001,          0x00000001,    // set clip bit
    0xffffffff,          0xffffffff
};

// second pass allows anything greater than just clip bit set to pass
static const GrStencilSettings gUserToClipUnionPass1 = {
    kReplace_StencilOp,  kReplace_StencilOp,
    kZero_StencilOp,     kZero_StencilOp,
    kLEqual_StencilFunc, kLEqual_StencilFunc,
    0xffffffff,          0xffffffff,
    0x00000000,          0x00000000,    // set clip bit
    0xffffffff,          0xffffffff
};

// for inverse first pass finds non-zerp user with clip bit set
// and converts it to just clip bit set
static const GrStencilSettings gInvUserToClipUnionPass0 = {
    kReplace_StencilOp,  kReplace_StencilOp,
    kKeep_StencilOp,     kKeep_StencilOp,
    kLess_StencilFunc,   kLess_StencilFunc,
    0xffffffff,          0xffffffff,
    0x00000000,          0x00000000,    // set clip bit
    0xffffffff,          0xffffffff
};

// second pass lets anything through with a nonzero user portion
// and writes a ref value with just the clip bit set to it.
static const GrStencilSettings gInvUserToClipUnionPass1 = {
    kReplace_StencilOp,  kReplace_StencilOp,
    kZero_StencilOp,     kZero_StencilOp,
    kLess_StencilFunc,   kLess_StencilFunc,
    0xffffffff,          0xffffffff,    // unset clip bit
    0x00000000,          0x00000000,    // set clip bit
    0xffffffff,          0xffffffff
};

///////
// Xor
static const GrStencilSettings gUserToClipXorPass0 = {
    kInvert_StencilOp,   kInvert_StencilOp,
    kKeep_StencilOp,     kKeep_StencilOp,
    kEqual_StencilFunc,  kEqual_StencilFunc,
    0xffffffff,          0xffffffff,    // unset clip bit
    0x00000000,          0x00000000,
    0xffffffff,          0xffffffff
};

static const GrStencilSettings gUserToClipXorPass1 = {
    kReplace_StencilOp,   kReplace_StencilOp,
    kZero_StencilOp,      kZero_StencilOp,
    kGreater_StencilFunc, kGreater_StencilFunc,
    0xffffffff,           0xffffffff,
    0x00000000,           0x00000000,   // set clip bit
    0xffffffff,           0xffffffff
};

static const GrStencilSettings gInvUserToClipXorPass0 = {
    kInvert_StencilOp,   kInvert_StencilOp,
    kKeep_StencilOp,     kKeep_StencilOp,
    kEqual_StencilFunc,  kEqual_StencilFunc,
    0xffffffff,          0xffffffff,    // unset clip bit
    0x00000000,          0x00000000,
    0xffffffff,          0xffffffff
};

static const GrStencilSettings gInvUserToClipXorPass1 = {
    kReplace_StencilOp,   kReplace_StencilOp,
    kZero_StencilOp,      kZero_StencilOp,
    kLess_StencilFunc,    kLess_StencilFunc,
    0xffffffff,           0xffffffff,
    0x00000000,           0x00000000,   // set clip bit
    0xffffffff,           0xffffffff
};

///////
// Reverse Diff
static const GrStencilSettings gUserToClipRDiffPass0 = {
    kInvert_StencilOp,   kInvert_StencilOp,
    kZero_StencilOp,     kZero_StencilOp,
    kLess_StencilFunc,   kLess_StencilFunc,
    0xffffffff,          0xffffffff,  // unset clip bit
    0x00000000,          0x00000000,  // set clip bit
    0xffffffff,          0xffffffff
};

static const GrStencilSettings gUserToClipRDiffPass1 = {
    kReplace_StencilOp,   kReplace_StencilOp,
    kZero_StencilOp,      kZero_StencilOp,
    kEqual_StencilFunc,   kEqual_StencilFunc,
    0x00000000,           0x00000000,   // set clip bit
    0x00000000,           0x00000000,   // set clip bit
    0xffffffff,           0xffffffff
};

static const GrStencilSettings gInvUserToClipRDiff = {
    kInvert_StencilOp,    kInvert_StencilOp,
    kZero_StencilOp,      kZero_StencilOp,
    kEqual_StencilFunc,   kEqual_StencilFunc,
    0xffffffff,           0xffffffff,
    0x00000000,           0x00000000, 
    0x00000000,           0x00000000    // set clip bit
};
///////
// Direct to Stencil

// We can render a clip element directly without first writing to the client
// portion of the clip when the fill is not inverse and the set operation will
// only modify the in/out status of samples covered by the clip element.

// this one only works if used right after stencil clip was cleared.
// Our GrClip doesn't allow midstream replace ops.
static const GrStencilSettings gReplaceClip = {
    kReplace_StencilOp,  kReplace_StencilOp,
    kReplace_StencilOp,  kReplace_StencilOp,
    kAlways_StencilFunc, kAlways_StencilFunc,
    0xffffffff,          0xffffffff,
    0x00000000,          0x00000000,    // set clip bit
    0x00000000,          0x00000000     // set clipBit
};

static const GrStencilSettings gUnionClip = {
    kReplace_StencilOp,  kReplace_StencilOp,
    kReplace_StencilOp,  kReplace_StencilOp,
    kAlways_StencilFunc, kAlways_StencilFunc,
    0xffffffff,          0xffffffff,
    0x00000000,          0x00000000,    // set clip bit
    0x00000000,          0x00000000     // set clip bit
};

static const GrStencilSettings gXorClip = {
    kInvert_StencilOp,   kInvert_StencilOp,
    kInvert_StencilOp,   kInvert_StencilOp,
    kAlways_StencilFunc, kAlways_StencilFunc,
    0xffffffff,          0xffffffff,
    0x00000000,          0x00000000,
    0x00000000,          0x00000000     // set clip bit
};

static const GrStencilSettings gDiffClip = {
    kZero_StencilOp,     kZero_StencilOp,
    kZero_StencilOp,     kZero_StencilOp,
    kAlways_StencilFunc, kAlways_StencilFunc,
    0xffffffff,          0xffffffff,
    0x00000000,          0x00000000,
    0x00000000,          0x00000000     // set clip bit
};

bool GrStencilSettings::GetClipPasses(GrSetOp op, 
                                      bool canBeDirect,
                                      unsigned int stencilClipMask,
                                      bool invertedFill,
                                      int* numPasses,
                                      GrStencilSettings settings[kMaxStencilClipPasses]) {
    if (canBeDirect && !invertedFill) {
        *numPasses = 0;
        switch (op) {
            case kReplace_SetOp:
                *numPasses = 1;
                settings[0] = gReplaceClip;
                break;
            case kUnion_SetOp:
                *numPasses = 1;
                settings[0] = gUnionClip;
                break;
            case kXor_SetOp:
                *numPasses = 1;
                settings[0] = gXorClip;
                break;
            case kDifference_SetOp:
                *numPasses = 1;
                settings[0] = gDiffClip;
                break;
            default: // suppress warning
                break;
        }
        if (1 == *numPasses) {
            settings[0].fFrontFuncRef |= stencilClipMask;
            settings[0].fFrontWriteMask |= stencilClipMask;
            settings[0].fBackFuncRef = settings[0].fFrontFuncRef;
            settings[0].fBackWriteMask = settings[0].fFrontWriteMask;
            return true;
        }
    }
    switch (op) {
        // if we make the path renderer go to stencil we always give it a
        // non-inverted fill and we use the stencil rules on the client->clipbit
        // pass to select either the zeros or nonzeros.
        case kReplace_SetOp:
            *numPasses= 1;
            settings[0] = invertedFill ? gInvUserToClipReplace : gUserToClipReplace;
            settings[0].fFrontFuncMask &= ~stencilClipMask;
            settings[0].fFrontFuncRef |= stencilClipMask;
            settings[0].fBackFuncMask = settings[0].fFrontFuncMask;
            settings[0].fBackFuncRef = settings[0].fFrontFuncRef;
            break;
        case kIntersect_SetOp:
            *numPasses = 1;
            settings[0] = invertedFill ? gInvUserToClipIsect : gUserToClipIsect;
            settings[0].fFrontFuncRef = stencilClipMask;
            settings[0].fBackFuncRef = settings[0].fFrontFuncRef;
            break;
        case kUnion_SetOp:
            *numPasses = 2;
            if (invertedFill) {
                settings[0] = gInvUserToClipUnionPass0;
                settings[0].fFrontFuncRef |= stencilClipMask;
                settings[0].fBackFuncRef = settings[0].fFrontFuncMask;

                settings[1] = gInvUserToClipUnionPass1;
                settings[1].fFrontFuncMask &= ~stencilClipMask;
                settings[1].fFrontFuncRef |= stencilClipMask;
                settings[1].fBackFuncMask = settings[1].fFrontFuncMask;
                settings[1].fBackFuncRef = settings[1].fFrontFuncRef;

            } else {
                settings[0] = gUserToClipUnionPass0;
                settings[0].fFrontFuncMask &= ~stencilClipMask;
                settings[0].fFrontFuncRef |= stencilClipMask;
                settings[0].fBackFuncMask = settings[0].fFrontFuncMask;
                settings[0].fBackFuncRef = settings[0].fFrontFuncRef;

                settings[1] = gUserToClipUnionPass1;
                settings[1].fFrontFuncRef |= stencilClipMask;
                settings[1].fBackFuncRef = settings[1].fFrontFuncRef;
            }
            break;
        case kXor_SetOp:
            *numPasses = 2;
            if (invertedFill) {
                settings[0] = gInvUserToClipXorPass0;
                settings[0].fFrontFuncMask &= ~stencilClipMask;
                settings[0].fBackFuncMask = settings[0].fFrontFuncMask;

                settings[1] = gInvUserToClipXorPass1;
                settings[1].fFrontFuncRef |= stencilClipMask;
                settings[1].fBackFuncRef = settings[1].fFrontFuncRef;
            } else {
                settings[0] = gUserToClipXorPass0;
                settings[0].fFrontFuncMask &= ~stencilClipMask;
                settings[0].fBackFuncMask = settings[0].fFrontFuncMask;

                settings[1] = gUserToClipXorPass1;
                settings[1].fFrontFuncRef |= stencilClipMask;
                settings[1].fBackFuncRef = settings[1].fFrontFuncRef;
            }
            break;
        case kDifference_SetOp:
            *numPasses = 1;
            settings[0] = invertedFill ? gInvUserToClipDiff : gUserToClipDiff;
            settings[0].fFrontFuncRef |= stencilClipMask;
            settings[0].fBackFuncRef = settings[0].fFrontFuncRef;
            break;
        case kReverseDifference_SetOp:
            if (invertedFill) {
                *numPasses = 1;
                settings[0] = gInvUserToClipRDiff;
                settings[0].fFrontWriteMask |= stencilClipMask;
                settings[0].fBackWriteMask = settings[0].fFrontWriteMask;
            } else {
                *numPasses = 2;
                settings[0] = gUserToClipRDiffPass0;
                settings[0].fFrontFuncMask &= ~stencilClipMask;
                settings[0].fBackFuncMask = settings[0].fFrontFuncMask;
                settings[0].fFrontFuncRef |= stencilClipMask;
                settings[0].fBackFuncRef = settings[0].fFrontFuncRef;

                settings[1] = gUserToClipRDiffPass1;
                settings[1].fFrontFuncMask |= stencilClipMask;
                settings[1].fFrontFuncRef |= stencilClipMask;
                settings[1].fBackFuncMask = settings[1].fFrontFuncMask;
                settings[1].fBackFuncRef = settings[1].fFrontFuncRef;
            }
            break;
        default:
            GrCrash("Unknown set op");
    }
    return false;
}
