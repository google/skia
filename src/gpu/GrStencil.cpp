
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrStencil.h"

////////////////////////////////////////////////////////////////////////////////
// Stencil Rules for Merging user stencil space into clip

// We can't include the clip bit in the ref or mask values because the division
// between user and clip bits in the stencil depends on the number of stencil
// bits in the runtime. Comments below indicate what the code should do to
// incorporate the clip bit into these settings.

///////
// Replace

// set the ref to be the clip bit, but mask it out for the test
GR_STATIC_CONST_SAME_STENCIL(gUserToClipReplace,
    kReplace_StencilOp,
    kZero_StencilOp,
    kLess_StencilFunc,
    0xffff,           // unset clip bit
    0x0000,           // set clip bit
    0xffff);

GR_STATIC_CONST_SAME_STENCIL(gInvUserToClipReplace,
    kReplace_StencilOp,
    kZero_StencilOp,
    kEqual_StencilFunc,
    0xffff,           // unset clip bit
    0x0000,           // set clip bit
    0xffff);

///////
// Intersect
GR_STATIC_CONST_SAME_STENCIL(gUserToClipIsect,
    kReplace_StencilOp,
    kZero_StencilOp,
    kLess_StencilFunc,
    0xffff,
    0x0000,           // set clip bit
    0xffff);

GR_STATIC_CONST_SAME_STENCIL(gInvUserToClipIsect,
    kReplace_StencilOp,
    kZero_StencilOp,
    kEqual_StencilFunc,
    0xffff,
    0x0000,           // set clip bit
    0xffff);

///////
// Difference
GR_STATIC_CONST_SAME_STENCIL(gUserToClipDiff,
    kReplace_StencilOp,
    kZero_StencilOp,
    kEqual_StencilFunc,
    0xffff,
    0x0000,           // set clip bit
    0xffff);

GR_STATIC_CONST_SAME_STENCIL(gInvUserToClipDiff,
    kReplace_StencilOp,
    kZero_StencilOp,
    kLess_StencilFunc,
    0xffff,
    0x0000,           // set clip bit
    0xffff);

///////
// Union

// first pass makes all the passing cases >= just clip bit set.
GR_STATIC_CONST_SAME_STENCIL(gUserToClipUnionPass0,
    kReplace_StencilOp,
    kKeep_StencilOp,
    kLEqual_StencilFunc,
    0xffff,
    0x0001,           // set clip bit
    0xffff);

// second pass allows anything greater than just clip bit set to pass
GR_STATIC_CONST_SAME_STENCIL(gUserToClipUnionPass1,
    kReplace_StencilOp,
    kZero_StencilOp,
    kLEqual_StencilFunc,
    0xffff,
    0x0000,           // set clip bit
    0xffff);

// first pass finds zeros in the user bits and if found sets
// the clip bit to 1
GR_STATIC_CONST_SAME_STENCIL(gInvUserToClipUnionPass0,
    kReplace_StencilOp,
    kKeep_StencilOp,
    kEqual_StencilFunc,
    0xffff,
    0x0000,           // set clip bit
    0x0000            // set clip bit
);

// second pass zeros the user bits
GR_STATIC_CONST_SAME_STENCIL(gInvUserToClipUnionPass1,
    kZero_StencilOp,
    kZero_StencilOp,
    kLess_StencilFunc,
    0xffff,
    0x0000,
    0xffff            // unset clip bit
);

///////
// Xor
GR_STATIC_CONST_SAME_STENCIL(gUserToClipXorPass0,
    kInvert_StencilOp,
    kKeep_StencilOp,
    kEqual_StencilFunc,
    0xffff,           // unset clip bit
    0x0000,
    0xffff);

GR_STATIC_CONST_SAME_STENCIL(gUserToClipXorPass1,
    kReplace_StencilOp,
    kZero_StencilOp,
    kGreater_StencilFunc,
    0xffff,
    0x0000,          // set clip bit
    0xffff);

GR_STATIC_CONST_SAME_STENCIL(gInvUserToClipXorPass0,
    kInvert_StencilOp,
    kKeep_StencilOp,
    kEqual_StencilFunc,
    0xffff,           // unset clip bit
    0x0000,
    0xffff);

GR_STATIC_CONST_SAME_STENCIL(gInvUserToClipXorPass1,
    kReplace_StencilOp,
    kZero_StencilOp,
    kLess_StencilFunc,
    0xffff,
    0x0000,          // set clip bit
    0xffff);

///////
// Reverse Diff
GR_STATIC_CONST_SAME_STENCIL(gUserToClipRDiffPass0,
    kInvert_StencilOp,
    kZero_StencilOp,
    kLess_StencilFunc,
    0xffff,         // unset clip bit
    0x0000,         // set clip bit
    0xffff);

GR_STATIC_CONST_SAME_STENCIL(gUserToClipRDiffPass1,
    kReplace_StencilOp,
    kZero_StencilOp,
    kEqual_StencilFunc,
    0x0000,          // set clip bit
    0x0000,          // set clip bit
    0xffff);

GR_STATIC_CONST_SAME_STENCIL(gInvUserToClipRDiff,
    kInvert_StencilOp,
    kZero_StencilOp,
    kEqual_StencilFunc,
    0xffff,
    0x0000,
    0x0000           // set clip bit
);
///////
// Direct to Stencil

// We can render a clip element directly without first writing to the client
// portion of the clip when the fill is not inverse and the set operation will
// only modify the in/out status of samples covered by the clip element.

// this one only works if used right after stencil clip was cleared.
// Our GrClip doesn't allow midstream replace ops.
GR_STATIC_CONST_SAME_STENCIL(gReplaceClip,
    kReplace_StencilOp,
    kReplace_StencilOp,
    kAlways_StencilFunc,
    0xffff,
    0x0000,           // set clip bit
    0x0000            // set clipBit
);

GR_STATIC_CONST_SAME_STENCIL(gUnionClip,
    kReplace_StencilOp,
    kReplace_StencilOp,
    kAlways_StencilFunc,
    0xffff,
    0x0000,           // set clip bit
    0x0000            // set clip bit
);

GR_STATIC_CONST_SAME_STENCIL(gXorClip,
    kInvert_StencilOp,
    kInvert_StencilOp,
    kAlways_StencilFunc,
    0xffff,
    0x0000,
    0x0000            // set clip bit
);

GR_STATIC_CONST_SAME_STENCIL(gDiffClip,
    kZero_StencilOp,
    kZero_StencilOp,
    kAlways_StencilFunc,
    0xffff,
    0x0000,
    0x0000            // set clip bit
);

bool GrStencilSettings::GetClipPasses(SkRegion::Op op, 
                                      bool canBeDirect,
                                      unsigned int stencilClipMask,
                                      bool invertedFill,
                                      int* numPasses,
                                      GrStencilSettings settings[kMaxStencilClipPasses]) {
    if (canBeDirect && !invertedFill) {
        *numPasses = 0;
        switch (op) {
            case SkRegion::kReplace_Op:
                *numPasses = 1;
                settings[0] = gReplaceClip;
                break;
            case SkRegion::kUnion_Op:
                *numPasses = 1;
                settings[0] = gUnionClip;
                break;
            case SkRegion::kXOR_Op:
                *numPasses = 1;
                settings[0] = gXorClip;
                break;
            case SkRegion::kDifference_Op:
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
        case SkRegion::kReplace_Op:
            *numPasses= 1;
            settings[0] = invertedFill ? gInvUserToClipReplace : gUserToClipReplace;
            settings[0].fFrontFuncMask &= ~stencilClipMask;
            settings[0].fFrontFuncRef |= stencilClipMask;
            settings[0].fBackFuncMask = settings[0].fFrontFuncMask;
            settings[0].fBackFuncRef = settings[0].fFrontFuncRef;
            break;
        case SkRegion::kIntersect_Op:
            *numPasses = 1;
            settings[0] = invertedFill ? gInvUserToClipIsect : gUserToClipIsect;
            settings[0].fFrontFuncRef = stencilClipMask;
            settings[0].fBackFuncRef = settings[0].fFrontFuncRef;
            break;
        case SkRegion::kUnion_Op:
            *numPasses = 2;
            if (invertedFill) {
                settings[0] = gInvUserToClipUnionPass0;
                settings[0].fFrontFuncMask &= ~stencilClipMask;
                settings[0].fBackFuncMask = settings[0].fFrontFuncMask;
                settings[0].fFrontFuncRef |= stencilClipMask;
                settings[0].fBackFuncRef = settings[0].fFrontFuncRef;
                settings[0].fFrontWriteMask |= stencilClipMask;
                settings[0].fBackWriteMask = settings[0].fFrontWriteMask;

                settings[1] = gInvUserToClipUnionPass1;
                settings[1].fFrontWriteMask &= ~stencilClipMask;
                settings[1].fBackWriteMask &= settings[1].fFrontWriteMask;

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
        case SkRegion::kXOR_Op:
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
        case SkRegion::kDifference_Op:
            *numPasses = 1;
            settings[0] = invertedFill ? gInvUserToClipDiff : gUserToClipDiff;
            settings[0].fFrontFuncRef |= stencilClipMask;
            settings[0].fBackFuncRef = settings[0].fFrontFuncRef;
            break;
        case SkRegion::kReverseDifference_Op:
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
