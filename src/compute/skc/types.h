/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#ifndef SKC_ONCE_TYPES
#define SKC_ONCE_TYPES

//
//
//

#if !defined(__OPENCL_C_VERSION__)

//
// What is going on here is that OpenCL defines both host and device
// scalar and vector types.
//
// Our actual device kernels should try to use native compiler types
// for anything that's private to the kernel and the skc_* variant for
// types and composites that get pulled into host code.
//
// I'm not excited about redefining all the basic types but this is a
// reasonable solution that allows the code to bridge the host (C/C++)
// and device (OpenCL) environments as well as provide vector types.
//

#ifdef __APPLE__
#include "OpenCL/opencl.h"
#else
#include "CL/opencl.h"
#endif

//
//
//

#define SKC_TYPE_HELPER(t)  skc_##t
#define SKC_TYPE(t)         SKC_TYPE_HELPER(t)

typedef cl_bool     skc_bool;

typedef cl_char     skc_char;
typedef cl_char2    skc_char2;
typedef cl_char3    skc_char3;
typedef cl_char4    skc_char4;
typedef cl_char8    skc_char8;
typedef cl_char16   skc_char16;

typedef cl_uchar    skc_uchar;
typedef cl_uchar2   skc_uchar2;
typedef cl_uchar3   skc_uchar3;
typedef cl_uchar4   skc_uchar4;
typedef cl_uchar8   skc_uchar8;
typedef cl_uchar16  skc_uchar16;

typedef cl_short    skc_short;
typedef cl_short2   skc_short2;
typedef cl_short3   skc_short3;
typedef cl_short4   skc_short4;
typedef cl_short8   skc_short8;
typedef cl_short16  skc_short16;

typedef cl_ushort   skc_ushort;
typedef cl_ushort2  skc_ushort2;
typedef cl_ushort3  skc_ushort3;
typedef cl_ushort4  skc_ushort4;
typedef cl_ushort8  skc_ushort8;
typedef cl_ushort16 skc_ushort16;

typedef cl_int      skc_int;
typedef cl_int2     skc_int2;
typedef cl_int3     skc_int3;
typedef cl_int4     skc_int4;
typedef cl_int8     skc_int8;
typedef cl_int16    skc_int16;

typedef cl_uint     skc_uint;
typedef cl_uint2    skc_uint2;
typedef cl_uint3    skc_uint3;
typedef cl_uint4    skc_uint4;
typedef cl_uint8    skc_uint8;
typedef cl_uint16   skc_uint16;

typedef cl_ulong    skc_ulong;
typedef cl_ulong2   skc_ulong2;
typedef cl_ulong3   skc_ulong3;
typedef cl_ulong4   skc_ulong4;
typedef cl_ulong8   skc_ulong8;
typedef cl_ulong16  skc_ulong16;

typedef cl_long     skc_long;
typedef cl_long2    skc_long2;
typedef cl_long3    skc_long3;
typedef cl_long4    skc_long4;
typedef cl_long8    skc_long8;
typedef cl_long16   skc_long16;

typedef cl_float    skc_float;
typedef cl_float2   skc_float2;
typedef cl_float3   skc_float3;
typedef cl_float4   skc_float4;
typedef cl_float8   skc_float8;
typedef cl_float16  skc_float16;

typedef cl_half     skc_half;

#if defined(__CL_HALF2__)
typedef cl_half2    skc_half2;
#endif
#if defined(__CL_HALF4__)
typedef cl_half4    skc_half4;
#endif
#if defined(__CL_HALF8__)
typedef cl_half8    skc_half8;
#endif
#if defined(__CL_HALF16__)
typedef cl_half16   skc_half16;
#endif

//
//
//

#else

//
//
//

#define SKC_TYPE(t) t

typedef bool        skc_bool;

typedef char        skc_char;
typedef char2       skc_char2;
typedef char3       skc_char3;
typedef char4       skc_char4;
typedef char8       skc_char8;
typedef char16      skc_char16;

typedef uchar       skc_uchar;
typedef uchar2      skc_uchar2;
typedef uchar3      skc_uchar3;
typedef uchar4      skc_uchar4;
typedef uchar8      skc_uchar8;
typedef uchar16     skc_uchar16;

typedef short       skc_short;
typedef short2      skc_short2;
typedef short3      skc_short3;
typedef short4      skc_short4;
typedef short8      skc_short8;
typedef short16     skc_short16;

typedef ushort      skc_ushort;
typedef ushort2     skc_ushort2;
typedef ushort3     skc_ushort3;
typedef ushort4     skc_ushort4;
typedef ushort8     skc_ushort8;
typedef ushort16    skc_ushort16;

typedef int         skc_int;
typedef int2        skc_int2;
typedef int3        skc_int3;
typedef int4        skc_int4;
typedef int8        skc_int8;
typedef int16       skc_int16;

typedef uint        skc_uint;
typedef uint2       skc_uint2;
typedef uint3       skc_uint3;
typedef uint4       skc_uint4;
typedef uint8       skc_uint8;
typedef uint16      skc_uint16;

typedef ulong       skc_ulong;
typedef ulong2      skc_ulong2;
typedef ulong3      skc_ulong3;
typedef ulong4      skc_ulong4;
typedef ulong8      skc_ulong8;
typedef ulong16     skc_ulong16;

typedef long        skc_long;
typedef long2       skc_long2;
typedef long3       skc_long3;
typedef long4       skc_long4;
typedef long8       skc_long8;
typedef long16      skc_long16;

typedef float       skc_float;
typedef float2      skc_float2;
typedef float3      skc_float3;
typedef float4      skc_float4;
typedef float8      skc_float8;
typedef float16     skc_float16;

typedef half        skc_half;

#if defined(__CL_HALF2__)
typedef half2       skc_half2;
#endif
#if defined(__CL_HALF4__)
typedef half4       skc_half4;
#endif
#if defined(__CL_HALF8__)
typedef half8       skc_half8;
#endif
#if defined(__CL_HALF16__)
typedef half16      skc_half16;
#endif

//
//
//

#define SKC_AS_HELPER(t)              as_##t
#define SKC_AS(t)                     SKC_AS_HELPER(t)

#define SKC_CONVERT_HELPER(t)         convert_##t
#define SKC_CONVERT(t)                SKC_CONVERT_HELPER(t)

// mode is: sat, rte, rtz, rtp, rtn --or-- sat_rte, sat_rtz, etc.
#define SKC_CONVERT_MODE_HELPER(t,m)  convert_##t##_##m
#define SKC_CONVERT_MODE(t,m)         SKC_CONVERT_HELPER(t)

//
//
//

#endif

//
//
//

#define SKC_UCHAR_MAX    0xFF

#define SKC_SHORT_MAX    0x7FFF
#define SKC_SHORT_MIN    (-SKC_SHORT_MAX - 1)
#define SKC_USHORT_MAX   0xFFFF

#define SKC_INT_MAX      0x7FFFFFFF
#define SKC_INT_MIN      (-SKC_INT_MAX - 1)
#define SKC_UINT_MAX     0xFFFFFFFF

#define SKC_ULONG_MAX    0xFFFFFFFFFFFFFFFFL

//
//
//

#endif

//
//
//

