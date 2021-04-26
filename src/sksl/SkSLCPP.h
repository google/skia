/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CPP
#define SKSL_CPP

// functions used by CPP programs created by skslc

#include <cmath>
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"

using std::abs;

struct SkSLFloat4 {
    SkSLFloat4(float x, float y, float z, float w)
    : fX(x)
    , fY(y)
    , fZ(z)
    , fW(w) {}

    operator SkRect() const {
        return SkRect::MakeLTRB(fX, fY, fZ, fW);
    }

private:
    float fX;
    float fY;
    float fZ;
    float fW;
};

// macros to make sk_Caps.<cap name> work from C++ code
#define sk_Caps (*args.fShaderCaps)

#define fbFetchSupport                              fbFetchSupport()
#define fbFetchNeedsCustomOutput                    fbFetchNeedsCustomOutput()
#define flatInterpolationSupport                    flatInterpolationSupport()
#define noperspectiveInterpolationSupport           noperspectiveInterpolationSupport()
#define externalTextureSupport                      externalTextureSupport()
#define mustEnableAdvBlendEqs                       mustEnableAdvBlendEqs()
#define mustDeclareFragmentShaderOutput             mustDeclareFragmentShaderOutput()
#define mustDoOpBetweenFloorAndAbs                  mustDoOpBetweenFloorAndAbs()
#define mustGuardDivisionEvenAfterExplicitZeroCheck mustGuardDivisionEvenAfterExplicitZeroCheck()
#define inBlendModesFailRandomlyForAllZeroVec       inBlendModesFailRandomlyForAllZeroVec()
#define atan2ImplementedAsAtanYOverX                atan2ImplementedAsAtanYOverX()
#define canUseAnyFunctionInShader                   canUseAnyFunctionInShader()
#define floatIs32Bits                               floatIs32Bits()
#define integerSupport                              integerSupport()
#define builtinFMASupport                           builtinFMASupport()
#define builtinDeterminantSupport                   builtinDeterminantSupport()
#define rewriteMatrixVectorMultiply                 rewriteMatrixVectorMultiply()

// functions to make GLSL constructors work from C++ code
inline SkPoint float2(float xy) { return SkPoint::Make(xy, xy); }

inline SkPoint float2(float x, float y) { return SkPoint::Make(x, y); }

inline SkSLFloat4 float4(float xyzw) { return SkSLFloat4(xyzw, xyzw, xyzw, xyzw); }

inline SkSLFloat4 float4(float x, float y, float z, float w) { return SkSLFloat4(x, y, z, w); }

#define half2 float2

#define half3 float3

#define half4 float4

#endif
