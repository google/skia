/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef GrScalar_DEFINED
#define GrScalar_DEFINED

#include "GrTypes.h"

#include <float.h>
#include <math.h>

#define GR_Int32Max            (0x7fffffff)
#define GR_Int32Min            (0x80000000)

/**
 *  Convert an int to fixed point 
 */
#if GR_DEBUG
    inline GrFixed GrIntToFixed(int i) {
        GrAssert(((i & 0xffff0000) == 0xffff0000) || ((i & 0xffff0000) == 0x0));
        return i << 16;
    }
#else
    #define GrIntToFixed(i) (GrFixed)((i) << 16)
#endif

#define GR_Fixed1              (1 << 16)
#define GR_FixedHalf           (1 << 15)
#define GR_FixedMax            GR_Int32Max
#define GR_FixedMin            GR_Int32Min

#define GrFixedFloorToFixed(x)  ((x) & ~0xFFFF)
#define GrFixedFloorToInt(x)    ((x) >> 16)

/**
 *  Convert fixed point to floating point
 */
#define GrFixedToFloat(x)      ((x) * 0.0000152587890625f)

/**
 *  Convert floating point to fixed point
 */
#define GrFloatToFixed(x)      ((GrFixed)((x) * GR_Fixed1))

inline GrFixed GrFixedAbs(GrFixed x) {
    int32_t s = (x & 0x80000000) >> 31;
    return (GrFixed)(((int32_t)x ^ s) - s);  
}

///////////////////////////////////////////////////////////////////////////////

#if GR_SCALAR_IS_FIXED
    typedef GrFixed                 GrScalar;
    #define GrIntToScalar(x)        GrIntToFixed(x)
    #define GrFixedToScalar(x)      (x)
    #define GrScalarToFloat(x)      GrFixedToFloat(x)
    #define GrFloatToScalar(x)      GrFloatToFixed(x)
    #define GrScalarHalf(x)         ((x) >> 1)
    #define GrScalarAve(x,y)        (((x)+(y)) >> 1)
    #define GrScalarAbs(x)          GrFixedAbs(x)
    #define GR_Scalar1              GR_Fixed1
    #define GR_ScalarHalf           GR_FixedHalf
    #define GR_ScalarMax            GR_FixedMax
    #define GR_ScalarMin            GR_FixedMin
#elif GR_SCALAR_IS_FLOAT
    typedef float                   GrScalar;
    #define GrIntToScalar(x)        ((GrScalar)x)
    #define GrFixedToScalar(x)      GrFixedToFloat(x)
    #define GrScalarToFloat(x)      (x)
    #define GrFloatToScalar(x)      (x)
    #define GrScalarHalf(x)         ((x) * 0.5f)
    #define GrScalarAbs(x)          fabsf(x)
    #define GrScalarAve(x,y)        (((x) + (y)) * 0.5f)
    #define GR_Scalar1              1.f    
    #define GR_ScalarHalf           0.5f
    #define GR_ScalarMax            (FLT_MAX)
    #define GR_ScalarMin            (-FLT_MAX)

    static inline int32_t GrScalarFloorToInt(float x) {
        return (int32_t)::floorf(x);
    }
    static inline int32_t GrScalarCeilToInt(float x) {
        return (int32_t)::ceilf(x);
    }
#else
    #error "Scalar type not defined"
#endif

/**
 *  Multiply two GrScalar values
 */
static inline GrScalar GrMul(GrScalar a, GrScalar b) {
#if GR_SCALAR_IS_FLOAT
    return a * b;
#else
    int64_t tmp = (int64_t)a * b;
    return (tmp + GR_FixedHalf) >> 16;
#endif
}

#endif

