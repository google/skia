/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
//
//

#define SKC_ARCH_AVX2

// #define SKC_RASTERIZE_SIMD_USES_SMEM

#define PRINTF_ENABLE       0
#define PRINTF_BLOCK_COUNT  0

//
// REMOVE THESE DEFINES ASAP -- ONLY HERE BECAUSE THE INTEL CODE
// BUILDER UTILITY DOESN'T SUPPORT CREATING AN ATOMIC TYPE BUFFER
//

#if 1
#define SKC_ATOMICS_TYPE  uint
#define SKC_ATOMICS_CAST  (__global atomic_uint volatile * restrict const)
#else
#define SKC_ATOMICS_TYPE  atomic_uint
#define SKC_ATOMICS_CAST
#endif

//
// NOTE:
//
// ON SIMD DEVICES THE BIN COUNT MUST BE POW2 SO THAT WE CAN LOAD IT
// AS A VECTOR AND PERFORM A SWIZZLE/SHUFFLE
//
//

#ifdef SKC_ARCH_AVX2

//
//
//

#define SKC_RASTERIZE_SUBGROUP_SIZE              1
#define SKC_RASTERIZE_VECTOR_SIZE_LOG2           3
#define SKC_RASTERIZE_WORKGROUP_COUNT_SUBGROUP   1

#define SKC_BLOCK_LARGE_ID_COUNT                 16

#define SKC_RASTERIZE_FLOAT                      float8
#define SKC_RASTERIZE_UINT                       uint8
#define SKC_RASTERIZE_INT                        int8
#define SKC_RASTERIZE_PREDICATE                  int8

#define SKC_RASTERIZE_BIN_BLOCK                  uint16
#define SKC_RASTERIZE_BIN                        uint8

#define SKC_RASTERIZE_POOL                       uint8
#define SKC_RASTERIZE_POOL_SCALE                 6

#define SKC_TILE_HASH_X_BITS                     1
#define SKC_TILE_HASH_Y_BITS                     2

#define SKC_RASTERIZE_VECTOR_EXPAND()            SKC_EXPAND_8()

//
//
//

#elif 1 // defined ( SKC_ARCH_GEN9 )

//
//
//

#define SKC_RASTERIZE_SUBGROUP_SIZE              8
#define SKC_RASTERIZE_VECTOR_SIZE_LOG2           0
#define SKC_RASTERIZE_WORKGROUP_COUNT_SUBGROUP   1

#define SKC_BLOCK_LARGE_ID_COUNT                 16

#define SKC_RASTERIZE_FLOAT                      float
#define SKC_RASTERIZE_UINT                       uint
#define SKC_RASTERIZE_INT                        int
#define SKC_RASTERIZE_PREDICATE                  bool

// #define SKC_RASTERIZE_BIN_BLOCK
// #define SKC_RASTERIZE_BIN

#define SKC_RASTERIZE_POOL                       uint
#define SKC_RASTERIZE_POOL_SCALE                 (SKC_RASTERIZE_SUBGROUP_SIZE * 5)

#define SKC_TILE_HASH_X_BITS                     1
#define SKC_TILE_HASH_Y_BITS                     2

// SKC_STATIC_ASSERT(SKC_RASTERIZE_POOL_SIZE > SKC_RASTERIZE_SUBGROUP_SIZE);

//
//
//

#endif

//
//
//

#define SKC_RASTERIZE_VECTOR_SIZE            (1 << SKC_RASTERIZE_VECTOR_SIZE_LOG2)
#define SKC_RASTERIZE_SUBGROUP_SEGMENT_COUNT (SKC_RASTERIZE_SUBGROUP_SIZE * SKC_RASTERIZE_VECTOR_SIZE)

#define SKC_RASTERIZE_POOL_SIZE              ((uint)(SKC_RASTERIZE_POOL_SCALE * sizeof(SKC_RASTERIZE_POOL) / sizeof(uint)))

//
//
//

#define SKC_RASTERIZE_YX_INIT      0x7FFF7FFF  // { +32767, +32767 }
#define SKC_RASTERIZE_YX_INVALID   0x80008000  // { -32768, -32768 }

//
//
//

#define SKC_TTS_INVALID            0xFFFFFFFF // relies on limited range of dx

//
//
//

#define SKC_TILE_HASH_X_MASK       SKC_BITS_TO_MASK(SKC_TILE_HASH_X_BITS)
#define SKC_TILE_HASH_Y_MASK       SKC_BITS_TO_MASK(SKC_TILE_HASH_Y_BITS)

#define SKC_TILE_HASH_BIN_COUNT    (1 << (SKC_TILE_HASH_X_BITS + SKC_TILE_HASH_Y_BITS))

#define SKC_TILE_HASH_BIN_BITS     (SKC_TILE_HASH_X_BITS + SKC_TILE_HASH_Y_BITS + 1) // FIXME -- LOG2(BIN_COUNT)
#define SKC_TILE_HASH_BIN_MASK     SKC_BITS_TO_MASK(SKC_TILE_HASH_BIN_BITS)

//
//
//

#define SKC_TILE_WIDTH_LOG2                  3
#define SKC_TILE_HEIGHT_LOG2                 4

#define SKC_SUBPIXEL_RESL_X_LOG2             5
#define SKC_SUBPIXEL_RESL_Y_LOG2             5

#define SKC_TTSK_BITS_X                      13
#define SKC_TTSK_BITS_Y                      13

//
// PLATFORM SURFACE TILE SIZE
//

#define SKC_TILE_WIDTH                       (1 << SKC_TILE_WIDTH_LOG2)
#define SKC_TILE_HEIGHT                      (1 << SKC_TILE_HEIGHT_LOG2)

#define SKC_TILE_WIDTH_MASK                  SKC_BITS_TO_MASK(SKC_TILE_WIDTH_LOG2)
#define SKC_TILE_HEIGHT_MASK                 SKC_BITS_TO_MASK(SKC_TILE_HEIGHT_LOG2)

//
// TILE SUBPIXEL RESOLUTION
//

#define SKC_SUBPIXEL_RESL_X                  (1 << SKC_SUBPIXEL_RESL_X_LOG2)
#define SKC_SUBPIXEL_RESL_Y                  (1 << SKC_SUBPIXEL_RESL_Y_LOG2)

#define SKC_SUBPIXEL_MASK_X                  SKC_BITS_TO_MASK(SKC_SUBPIXEL_RESL_X_LOG2)
#define SKC_SUBPIXEL_MASK_Y                  SKC_BITS_TO_MASK(SKC_SUBPIXEL_RESL_Y_LOG2)

#define SKC_SUBPIXEL_RESL_X_F32              ((float)(SKC_SUBPIXEL_RESL_X))
#define SKC_SUBPIXEL_RESL_Y_F32              ((float)(SKC_SUBPIXEL_RESL_Y))

#define SKC_SUBPIXEL_X_SCALE_UP              SKC_SUBPIXEL_RESL_X_F32
#define SKC_SUBPIXEL_Y_SCALE_UP              SKC_SUBPIXEL_RESL_Y_F32

#define SKC_SUBPIXEL_X_SCALE_DOWN            (1.0f / SKC_SUBPIXEL_RESL_X_F32)
#define SKC_SUBPIXEL_Y_SCALE_DOWN            (1.0f / SKC_SUBPIXEL_RESL_Y_F32)

//
// SUBTILE RESOLUTION
//

#define SKC_SUBTILE_RESL_X_LOG2              (SKC_TILE_WIDTH_LOG2  + SKC_SUBPIXEL_RESL_X_LOG2)
#define SKC_SUBTILE_RESL_Y_LOG2              (SKC_TILE_HEIGHT_LOG2 + SKC_SUBPIXEL_RESL_Y_LOG2)

#define SKC_SUBTILE_RESL_X                   (1 << SKC_SUBTILE_RESL_X_LOG2)
#define SKC_SUBTILE_RESL_Y                   (1 << SKC_SUBTILE_RESL_Y_LOG2)

#define SKC_SUBTILE_MASK_X                   SKC_BITS_TO_MASK(SKC_SUBTILE_RESL_X_LOG2)
#define SKC_SUBTILE_MASK_Y                   SKC_BITS_TO_MASK(SKC_SUBTILE_RESL_Y_LOG2)

#define SKC_SUBTILE_RESL_X_F32               ((float)(SKC_SUBTILE_RESL_X))
#define SKC_SUBTILE_RESL_Y_F32               ((float)(SKC_SUBTILE_RESL_Y))

#define SKC_SUBTILE_X_SCALE_DOWN             (1.0f / SKC_SUBTILE_RESL_X_F32)
#define SKC_SUBTILE_Y_SCALE_DOWN             (1.0f / SKC_SUBTILE_RESL_Y_F32)

//
//
//

#define SKC_TILE_X_OFFSET_U32     (1 << (SKC_TTSK_BITS_X-1))
#define SKC_TILE_X_SPAN_U32       (1 << (SKC_TTSK_BITS_X))   // exclusive

#define SKC_TILE_Y_OFFSET_U32     (1 << (SKC_TTSK_BITS_Y-1))
#define SKC_TILE_Y_SPAN_U32       (1 << (SKC_TTSK_BITS_Y))   // exclusive

#define SKC_TILE_X_OFFSET_F32     0 // ((float)SKC_TILE_X_OFFSET_U32)
#define SKC_TILE_X_SPAN_F32       ((float)SKC_TILE_X_SPAN_U32)

#define SKC_TILE_Y_OFFSET_F32     0 // ((float)SKC_TILE_Y_OFFSET_U32)
#define SKC_TILE_Y_SPAN_F32       ((float)SKC_TILE_Y_SPAN_U32)

//
//
//

#define SKC_BITS_TO_MASK(n)         ((1U <<(n))-1)
#define SKC_BITS_TO_MASK_64(n)      ((1UL<<(n))-1)

#define SKC_BITS_TO_MASK_AT(n,b)    (SKC_BITS_TO_MASK(n)<<(b))
#define SKC_BITS_TO_MASK_AT_64(n,b) (SKC_BITS_TO_MASK_64(n)<<(b))

#define SKC_BFE(x,n,b)              ((x) >> (b) & SKC_BITS_TO_MASK(n))

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

#define SKC_STRINGIFY2(a)   #a
#define SKC_STRINGIFY(a)    SKC_STRINGIFY2(a)

//
//
//

#define SKC_EVAL(a)      a
#define SKC_CONCAT(a,b)  SKC_EVAL(a)##SKC_EVAL(b)

//
//
//

#define SKC_EMPTY
#define SKC_COMMA  ,

//
// INDEX, SUFFIX, COMPONENT, PUNCTUATION
//

#define SKC_EXPAND_1()                          \
  SKC_EXPAND_X(0,SKC_EMPTY,SKC_EMPTY,SKC_EMPTY)

#define SKC_EXPAND_2()                          \
  SKC_EXPAND_X(0, 0,.s0,SKC_COMMA)              \
  SKC_EXPAND_X(1, 1,.s1,SKC_EMPTY)

#define SKC_EXPAND_4()                          \
  SKC_EXPAND_X(0, 0,.s0,SKC_COMMA)              \
  SKC_EXPAND_X(1, 1,.s1,SKC_COMMA)              \
  SKC_EXPAND_X(2, 2,.s2,SKC_COMMA)              \
  SKC_EXPAND_X(3, 3,.s3,SKC_EMPTY)

#define SKC_EXPAND_8()                          \
  SKC_EXPAND_X(0, 0,.s0,SKC_COMMA)              \
  SKC_EXPAND_X(1, 1,.s1,SKC_COMMA)              \
  SKC_EXPAND_X(2, 2,.s2,SKC_COMMA)              \
  SKC_EXPAND_X(3, 3,.s3,SKC_COMMA)              \
  SKC_EXPAND_X(4, 4,.s4,SKC_COMMA)              \
  SKC_EXPAND_X(5, 5,.s5,SKC_COMMA)              \
  SKC_EXPAND_X(6, 6,.s6,SKC_COMMA)              \
  SKC_EXPAND_X(7, 7,.s7,SKC_EMPTY)

#define SKC_EXPAND_16()                         \
  SKC_EXPAND_X(0, 0,.s0,SKC_COMMA)              \
  SKC_EXPAND_X(1, 1,.s1,SKC_COMMA)              \
  SKC_EXPAND_X(2, 2,.s2,SKC_COMMA)              \
  SKC_EXPAND_X(3, 3,.s3,SKC_COMMA)              \
  SKC_EXPAND_X(4, 4,.s4,SKC_COMMA)              \
  SKC_EXPAND_X(5, 5,.s5,SKC_COMMA)              \
  SKC_EXPAND_X(6, 6,.s6,SKC_COMMA)              \
  SKC_EXPAND_X(7, 7,.s7,SKC_COMMA)              \
  SKC_EXPAND_X(8, 8,.s8,SKC_COMMA)              \
  SKC_EXPAND_X(9, 9,.s9,SKC_COMMA)              \
  SKC_EXPAND_X(10,A,.sA,SKC_COMMA)              \
  SKC_EXPAND_X(11,B,.sB,SKC_COMMA)              \
  SKC_EXPAND_X(12,C,.sC,SKC_COMMA)              \
  SKC_EXPAND_X(13,D,.sD,SKC_COMMA)              \
  SKC_EXPAND_X(14,E,.sE,SKC_COMMA)              \
  SKC_EXPAND_X(15,F,.sF,SKC_EMPTY)

//
//
//

//
// Norbert Juffa notes: "GPU Pro Tip: Lerp Faster in C++"
//
// https://devblogs.nvidia.com/parallelforall/lerp-faster-cuda/
//
// Lerp in two fma/mad ops:
//
//    t * b + ((-t) * a + a)
//
// Note: OpenCL documents mix() as being implemented as:
//
//    a + (b - a) * t
//
// But this may be a native instruction on some devices.  For example,
// on GEN9 there is an LRP "linear interoplation" function but it
// doesn't appear to support half floats.
//

#if 1
#define SKC_LERP(a,b,t)  mad(t,b,mad(-(t),a,a))
#else
#define SKC_LERP(a,b,t)  mix(a,b,t)
#endif

//
//
//

#define SKC_MAD_UINT(a,b,c) (a * b + c)

//
//
//

union skc_rasterize_cmd
{
  ushort4  b16v4;

  uint2    b32v2;

  struct {
    ushort coord_base;
    ushort transform_idx;
    ushort raster_clip;
    ushort raster_cohort_id;
  };
};

//
//
//

union skc_transform_v
{
  float8    f32v8;

  // float4 f32v4a2[2];
  // float  f32a8  [8];

  struct {
    float   sx;
    float   shx;
    float   tx;

    float   shy;
    float   sy;
    float   ty;

    float   w0;
    float   w1;
    //  --> w2 <-- implicit, always equals 1.0f
  };
};

//
//
//

struct skc_subgroup_smem
{
  //
  // SIMT subgroup scratchpad for max scan
  //
#if ( SKC_RASTERIZE_SUBGROUP_SIZE > 1 ) || defined ( SKC_RASTERIZE_SIMD_USES_SMEM )
  union {
    uint                      winner;
    union {
      uint                    scratch[SKC_RASTERIZE_SUBGROUP_SIZE];
    } aN;
    union {
      SKC_RASTERIZE_UINT      scratch[SKC_RASTERIZE_SUBGROUP_SIZE];
    } vN;
  } subgroup;
#endif

  //
  // work-in-progress TTSB blocks and associated YX keys
  //
  union {
    struct {
      uint                    block  [SKC_TILE_HASH_BIN_COUNT][SKC_BLOCK_LARGE_ID_COUNT];
      uint                    ttsb_id[SKC_TILE_HASH_BIN_COUNT];
      uint                    yx     [SKC_TILE_HASH_BIN_COUNT];
      uint                    count  [SKC_TILE_HASH_BIN_COUNT];
    } aN;
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
    struct {
      SKC_RASTERIZE_BIN_BLOCK block  [SKC_TILE_HASH_BIN_COUNT];
      SKC_RASTERIZE_BIN       ttsb_id;
      SKC_RASTERIZE_BIN       yx;
      SKC_RASTERIZE_BIN       count;
    } vN;
#endif
  } bin;

  //
  // pool of available TTSB blocks and associated YX keys
  //
  union {
    struct {
      uint                    ttsb_id[SKC_RASTERIZE_POOL_SIZE];
      uint                    yx     [SKC_RASTERIZE_POOL_SIZE];
    } aN;
    struct {
      SKC_RASTERIZE_POOL      ttsb_id[SKC_RASTERIZE_POOL_SCALE];
      SKC_RASTERIZE_POOL      yx     [SKC_RASTERIZE_POOL_SCALE];
    } vN;
    uint                      count;
  } pool;

  //
  // save raster id to smem?
  //
};

//
//
//

#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
#define skc_sub_group_local_id()  0
#else
#define skc_sub_group_local_id()  get_sub_group_local_id()
#endif

//
//
//

static
SKC_RASTERIZE_FLOAT
skc_sub_group_scan_inclusive_add_float(SKC_RASTERIZE_FLOAT const v)
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
  // Note that there isn't a built-in horizontal scan for vectors so
  // we'll define some here for various widths.
  //
  // FIXME -- a scalar version might be faster so put in a
  // compile-time switch to selection between implementations
  //

#if   ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 0 )
  return v;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 1 )
  // 01
  //  0 +
  // --
  // 01
  SKC_RASTERIZE_FLOAT const w = mad(v.s10,(SKC_RASTERIZE_FLOAT)(0,1),v);
  return w;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 2 )
  // 0123
  //  012 +
  // ----
  // 0123
  //   01 +
  // ----
  // 0123
  //
  SKC_RASTERIZE_FLOAT const w = mad(v.s3012,(SKC_RASTERIZE_FLOAT)(0,1,1,1),v);
  SKC_RASTERIZE_FLOAT const x = mad(w.s2301,(SKC_RASTERIZE_FLOAT)(0,0,1,1),w);
  return x;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 3 )
  // 01234567
  //  0123456 +
  // --------
  // 01234567
  //   012345 +
  // --------
  // 01234567
  //     0123 +
  // --------
  // 01234567
  //
  SKC_RASTERIZE_FLOAT const w = mad(v.s70123456,(SKC_RASTERIZE_FLOAT)(0,1,1,1,1,1,1,1),v);
  SKC_RASTERIZE_FLOAT const x = mad(w.s67012345,(SKC_RASTERIZE_FLOAT)(0,0,1,1,1,1,1,1),w);
  SKC_RASTERIZE_FLOAT const y = mad(x.s45670123,(SKC_RASTERIZE_FLOAT)(0,0,0,0,1,1,1,1),x);
  return y;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 4 )
  // 0123456789abcdef
  //  0123456789abcde +
  // ----------------
  // 0123456789abcdef
  //   0123456789abcd +
  // ----------------
  // 0123456789abcdef
  //     0123456789ab +
  // ----------------
  // 0123456789abcdef
  //         01234567 +
  // ----------------
  // 0123456789abcdef
  //
  SKC_RASTERIZE_FLOAT const w = mad(v.sf0123456789abcde,(SKC_RASTERIZE_FLOAT)(0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),v);
  SKC_RASTERIZE_FLOAT const x = mad(w.sef0123456789abcd,(SKC_RASTERIZE_FLOAT)(0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1),w);
  SKC_RASTERIZE_FLOAT const y = mad(x.scdef0123456789ab,(SKC_RASTERIZE_FLOAT)(0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1),x);
  SKC_RASTERIZE_FLOAT const z = mad(y.s89abcdef01234567,(SKC_RASTERIZE_FLOAT)(0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1),y);
  return z;

#endif

#else
  //
  // SIMT
  //

  return sub_group_scan_inclusive_add(v);

#endif
}

//
//
//

static
SKC_RASTERIZE_UINT
skc_sub_group_scan_inclusive_add_uint(SKC_RASTERIZE_UINT const v)
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
  // Note that there isn't a built-in horizontal scan for vectors so
  // we'll define some here for various widths.
  //
  // FIXME -- a scalar version might be faster so put in a
  // compile-time switch to selection between implementations
  //

#if   ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 0 )
  return v;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 1 )
  // 01
  //  0 +
  // --
  // 01
  SKC_RASTERIZE_UINT const w = SKC_MAD_UINT(v.s10,(SKC_RASTERIZE_UINT)(0,1),v);
  return w;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 2 )
  // 0123
  //  012 +
  // ----
  // 0123
  //   01 +
  // ----
  // 0123
  //
  SKC_RASTERIZE_UINT const w = SKC_MAD_UINT(v.s3012,(SKC_RASTERIZE_UINT)(0,1,1,1),v);
  SKC_RASTERIZE_UINT const x = SKC_MAD_UINT(w.s2301,(SKC_RASTERIZE_UINT)(0,0,1,1),w);
  return x;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 3 )
  // 01234567
  //  0123456 +
  // --------
  // 01234567
  //   012345 +
  // --------
  // 01234567
  //     0123 +
  // --------
  // 01234567
  //
  SKC_RASTERIZE_UINT const w = SKC_MAD_UINT(v.s70123456,(SKC_RASTERIZE_UINT)(0,1,1,1,1,1,1,1),v);
  SKC_RASTERIZE_UINT const x = SKC_MAD_UINT(w.s67012345,(SKC_RASTERIZE_UINT)(0,0,1,1,1,1,1,1),w);
  SKC_RASTERIZE_UINT const y = SKC_MAD_UINT(x.s45670123,(SKC_RASTERIZE_UINT)(0,0,0,0,1,1,1,1),x);
  return y;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 4 )
  // 0123456789abcdef
  //  0123456789abcde +
  // ----------------
  // 0123456789abcdef
  //   0123456789abcd +
  // ----------------
  // 0123456789abcdef
  //     0123456789ab +
  // ----------------
  // 0123456789abcdef
  //         01234567 +
  // ----------------
  // 0123456789abcdef
  //
  SKC_RASTERIZE_UINT const w = SKC_MAD_UINT(v.sf0123456789abcde,(SKC_RASTERIZE_UINT)(0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),v);
  SKC_RASTERIZE_UINT const x = SKC_MAD_UINT(w.sef0123456789abcd,(SKC_RASTERIZE_UINT)(0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1),w);
  SKC_RASTERIZE_UINT const y = SKC_MAD_UINT(x.scdef0123456789ab,(SKC_RASTERIZE_UINT)(0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1),x);
  SKC_RASTERIZE_UINT const z = SKC_MAD_UINT(y.s89abcdef01234567,(SKC_RASTERIZE_UINT)(0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1),y);
  return z;

#endif

#else
  //
  // SIMT
  //

  return sub_group_scan_inclusive_add(v);

#endif
}

//
//
//

static
SKC_RASTERIZE_UINT
skc_sub_group_scan_inclusive_max(SKC_RASTERIZE_UINT const v)
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
  // Note that there isn't a built-in horizontal scan for vectors so
  // we'll define some here for various widths.
  //
  // FIXME -- a scalar version might be faster so put in a
  // compile-time switch to selection between implementations
  //

#if   ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 0 )
  return v;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 1 )
  // 01
  // 00 max
  // --
  // 01
  SKC_RASTERIZE_UINT const w = max(v.s00,v);
  return w;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 2 )
  // 0123
  // 0012 +
  // ----
  // 0123
  // 0101 +
  // ----
  // 0123
  //
  SKC_RASTERIZE_UINT const w = max(v.s0012,v);
  SKC_RASTERIZE_UINT const x = max(w.s0101,w);
  return x;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 3 )
  // 01234567
  // 00123456 +
  // --------
  // 01234567
  // 01012345 +
  // --------
  // 01234567
  // 01230123 +
  // --------
  // 01234567
  //
  SKC_RASTERIZE_UINT const w = max(v.s00123456,v);
  SKC_RASTERIZE_UINT const x = max(w.s01012345,w);
  SKC_RASTERIZE_UINT const y = max(x.s01230123,x);
  return y;

#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 4 )
  // 0123456789abcdef
  // 00123456789abcde +
  // ----------------
  // 0123456789abcdef
  // 010123456789abcd +
  // ----------------
  // 0123456789abcdef
  // 01230123456789ab +
  // ----------------
  // 0123456789abcdef
  // 0123456701234567 +
  // ----------------
  // 0123456789abcdef
  //
  SKC_RASTERIZE_UINT const w = max(v.s00123456789abcde,v);
  SKC_RASTERIZE_UINT const x = max(w.s010123456789abcd,w);
  SKC_RASTERIZE_UINT const y = max(x.s01230123456789ab,x);
  SKC_RASTERIZE_UINT const z = max(y.s0123456701234567,y);
  return z;

#endif

#else
  //
  // SIMT
  //

  return sub_group_scan_inclusive_max(v);

#endif
}

//
//
//

static
float
skc_sub_group_last_float(SKC_RASTERIZE_FLOAT const v)
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
#if   ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 0 )
  return v;
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 1 )
  return v.s1;
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 2 )
  return v.s3;
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 3 )
  return v.s7;
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 4 )
  return v.sf;
#endif

#else
  //
  // SIMT
  //
  return sub_group_broadcast(v,SKC_RASTERIZE_SUBGROUP_SIZE-1);

#endif
}

//
//
//

static
SKC_RASTERIZE_UINT
skc_sub_group_last_uint(SKC_RASTERIZE_UINT const v)
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
#if   ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 0 )
  return v;
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 1 )
  return v.s1;
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 2 )
  return v.s3;
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 3 )
  return v.s7;
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 4 )
  return v.sf;
#endif

#else
  //
  // SIMT
  //
  return sub_group_broadcast(v,SKC_RASTERIZE_SUBGROUP_SIZE-1);

#endif
}

//
//
//

static
float
skc_sub_group_first(SKC_RASTERIZE_FLOAT const v)
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
#if   ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 0 )
  return v;
#else
  return v.s0;
#endif

#else
  //
  // SIMT
  //
  return sub_group_broadcast(v,0);

#endif
}

//
//
//

static
SKC_RASTERIZE_FLOAT
skc_sub_group_shuffle(SKC_RASTERIZE_FLOAT const v,
                      SKC_RASTERIZE_UINT  const i)
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
#if   ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 0 )
  return v;
#else
  return shuffle(v,i);
#endif

#else
  //
  // SIMT
  //
  return intel_sub_group_shuffle(v,i);

#endif
}

//
//
//

static
SKC_RASTERIZE_FLOAT
skc_sub_group_shuffle_up_1(SKC_RASTERIZE_FLOAT const p, // previous
                           SKC_RASTERIZE_FLOAT const c) // current
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
  // FIXME -- there are alternative formulations here:
  //
  // Option 1:
  //
  //   select(c.rotate(+1),p.rotate(-1),(1,0,0,...))
  //
  // Option 2:
  //
  //   p is a scalar
  //   t    = c.rotate(+1)
  //   t.s0 = p;
  //
  // Option 3: ...
  //
#if   ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 0 )
  return p;
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 1 )
  return shuffle2(p,c,(uint2)(1,2));
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 2 )
  return shuffle2(p,c,(uint4)(3,4,5,6));
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 3 )
  return shuffle2(p,c,(uint8)(7,8,9,10,11,12,13,14));
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 4 )
  return shuffle2(p,c,(uint16)(15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30));
#endif

#else
  //
  // SIMT
  //
  return intel_sub_group_shuffle_up(p,c,1);

#endif
}

//
//
//

static
bool
skc_is_lane_first()
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1)
  //
  // SIMD
  //
  return true;
#else
  //
  // SIMT
  //
  return get_sub_group_local_id() == 0;
#endif
}

//
//
//

static
SKC_RASTERIZE_FLOAT
skc_delta_offset()
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
#if   ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 0 )
  return 1;
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 1 )
  return (SKC_RASTERIZE_FLOAT)( 1, 2 );
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 2 )
  return (SKC_RASTERIZE_FLOAT)( 1, 2, 3, 4 );
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 3 )
  return (SKC_RASTERIZE_FLOAT)( 1, 2, 3, 4, 5, 6, 7, 8 );
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 4 )
  return (SKC_RASTERIZE_FLOAT)( 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 );
#endif

#else
  //
  // SIMT
  //
  return 1.0f + get_sub_group_local_id();

#endif

}

//
//
//

static
int
skc_sub_group_any(SKC_RASTERIZE_PREDICATE const p)
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
  return any(p);
#else
  //
  // SIMT
  //
  return sub_group_any(p);
#endif
}

//
//
//

static
SKC_RASTERIZE_FLOAT
skc_native_length(SKC_RASTERIZE_FLOAT const x, SKC_RASTERIZE_FLOAT const y)
{
  return native_sqrt(x * x + y * y);
}

//
// Wang's Formula (1985)
//

#define SKC_WANG_PIXEL_RESL   0.25f // <-- this can be tuned

#define SKC_WANG_EPSILON      (SKC_WANG_PIXEL_RESL * SKC_SUBPIXEL_RESL_X_F32)

#define SKC_WANG_CUBIC        ((3.0f * 2.0f) / (8.0f * SKC_WANG_EPSILON))
#define SKC_WANG_QUADRATIC    ((2.0f       ) / (8.0f * SKC_WANG_EPSILON))

#define SKC_WANG_LENGTH(x,y)  skc_native_length(x,y)
#define SKC_WANG_SQRT(x)      native_sqrt(x)

//
//
//

static
SKC_RASTERIZE_FLOAT
skc_wangs_formula_cubic(SKC_RASTERIZE_FLOAT const t0x, SKC_RASTERIZE_FLOAT const t0y,
                        SKC_RASTERIZE_FLOAT const t1x, SKC_RASTERIZE_FLOAT const t1y,
                        SKC_RASTERIZE_FLOAT const t2x, SKC_RASTERIZE_FLOAT const t2y,
                        SKC_RASTERIZE_FLOAT const t3x, SKC_RASTERIZE_FLOAT const t3y)
{
  //
  // Return the number of evenly spaced (in the parametric sense) line
  // segments that are guaranteed to be within "epsilon" error of the
  // curve.
  //
  // We're then going to take multiples of the reciprocal of this
  // number so that the segmentation can be distributed across the
  // subgroup.
  //
  // Note, this can probably be slightly optimized per architecture
  // but it's probably far from being a hotspot since it's all
  // straight-line unpredicated code.
  //
  // The result is an integer ranging from [1.0,#segments]
  //
  // Note that even if all of the control points are coincident, the
  // max(1.0f) will categorize this as a line of 1 segment.
  //
  // This is what we want!  We want to convert cubics to lines as
  // easily as possible and *then* cull lines that are either
  // horizontal or zero length.
  //
  return max(1.0f,
             ceil(SKC_WANG_SQRT(SKC_WANG_CUBIC *
                                SKC_WANG_LENGTH(max(fabs(t2x - 2.0f * t1x + t0x),
                                                    fabs(t3x - 2.0f * t2x + t1x)),
                                                max(fabs(t2y - 2.0f * t1y + t0y),
                                                    fabs(t3y - 2.0f * t2y + t1y))))));
}

static
SKC_RASTERIZE_FLOAT
skc_wangs_formula_quadratic(SKC_RASTERIZE_FLOAT const t0x, SKC_RASTERIZE_FLOAT const t0y,
                            SKC_RASTERIZE_FLOAT const t1x, SKC_RASTERIZE_FLOAT const t1y,
                            SKC_RASTERIZE_FLOAT const t2x, SKC_RASTERIZE_FLOAT const t2y)
{
  return max(1.0f,
             ceil(SKC_WANG_SQRT(SKC_WANG_QUADRATIC *
                                SKC_WANG_LENGTH(fabs(t2x - 2.0f * t1x + t0x),
                                                fabs(t2y - 2.0f * t1y + t0y)))));
}

//
// Requirements:
//
//   - A TTXB extent that is at least 1GB for all TTXB sizes
//  
//   - A virtual surface of at least 8K x 8K
//
//   - A physical surface of __don't really care__ because it's
//     advantageous to tile the physical surface since it's likely
//     to shrink the post-place TTCK sorting step.
//
//
//      EXTENT                 TTXB BITS
//     SIZE (MB) +------------------------------------+
//               |  22    23    24    25    26    27  |
//          +----+------------------------------------+
//          |  8 |  128   256   512  1024  2048  4096 |
//     TTXB | 16 |  256   512  1024  2048  4096  8192 |
//    WORDS | 32 |  512  1024  2048  4096  8192 16384 |
//          | 64 | 1024  2048  4096  8192 16384 32768 |
//          +----+------------------------------------+
//
//
//         SURF                        X/Y BITS
//         TILE  +------------------------------------------------------+
//               |   5     6     7     8     9    10    11    12    13  | 
//          +----+------------------------------------------------------+
//          |  3 |  256   512  1024  2048  4096  8192 16384 32768 65536 |
//     TILE |  4 |  512  1024  2048  4096  8192 16384 32768 65536  128K |
//     SIDE |  5 | 1024  2048  4096  8192 16384 32768 65536  128K  256K |
//     BITS |  6 | 2048  4096  8192 16384 32768 65536  128K  256K  512K |
//          |  7 | 4096  8192 16384 32768 65536  128K  256K  512K 1024K |
//          +----+------------------------------------------------------+
//      TILES^2  | 1024  4096 16384 65536  256K    1M    4M   16M   64M |
//               +------------------------------------------------------+
//
// The following values should be pretty future-proof across all GPUs:
//
//   - 27 bits of TTXB_ID space implies a max of 4GB-32GB of
//     rasterized paths depending on the size of the TTXB block.
//     This could enable interesting use cases.
//
//   - A virtual rasterization surface that's from +/-16K to +/-128K
//     depending on the size of the TTXB block.
//
//   - Keys that only require a 32-bit high word comparison.
//
//   - Support for a minimum of 256K layers. This can be practically
//     raised to 1m or 2m layers.
//
//
// TTSK_RYX:
//
//  0                                         63
//  | TTSB ID |   X  |   Y  | RASTER COHORT ID |
//  +---------+------+------+------------------+
//  |    27   |  12  |  12  |        13        |
//
//
// TTSK_RYX (32-BIT COMPARE)
//
//  0                                               63
//  | TTSB ID | N/A |   X  |   Y  | RASTER COHORT ID |
//  +---------+-----+------+------+------------------+
//  |    27   |  5  |  12  |  12  |        8         |
//
//
// TTSK_YX:
//
//  0                              63
//  | TTSB ID |  N/A  |   X  |   Y  |
//  +---------+-------+------+------+
//  |    27   |   13  |  12  |  12  |
//
//
// TTPK:
//
//  0                                        63
//  | TTPB ID | ALL ZEROES | SPAN |  X  |  Y  |
//  +---------+------------+------+-----+-----+
//  |    27   |      1     |  12  | 12  | 12  |
//
//
// TTCK (32-BIT COMPARE):
//
//  0                                                             63
//  | PAYLOAD/TTSB/TTPB ID | PREFIX | ESCAPE | STYLING |  X  |  Y  |
//  +----------------------+--------+--------+---------+-----+-----+
//  |          30          |    1   |    1   |    18   |  7  |  7  |
//

//
// construct the ttsk_ryx key
//

static
ulong
skc_make_ttsk_ryx(__local struct skc_subgroup_smem volatile * restrict const smem,
                  ushort                                               const raster_id,
                  uint                                                 const idx)
{
#if 1
  //
  // Requires 64-bit key sort
  //
  union {
    ulong u64;
    uint2 u32v2;
  } ttsk_ryx = { smem->pool.aN.ttsb_id[idx] };

  ttsk_ryx.u64      |= smem->pool.aN.yx[idx] << 27;
  ttsk_ryx.u32v2.hi |= raster_id             << 19;
#else
  //
  // 8-BIT RASTER COHORT 
  //
  // Allows 32-bit key + 32-bit payload sort
  //
  union {
    ulong   u64;
    uint2   u32v2;
    ushort4 u8v4;
  } ttsk_ryx = { 0L };

  ttsk_ryx.u32v2.lo   = smem->pool.aN.ttsb_id[idx];
  ttsk_ryx.u32v2.hi   = smem->pool.aN.yx     [idx];
  ttsk_ryx.u8v4.hi.hi = raster_id;
#endif

  return ttsk_ryx.u64;
}

//
// flush any work-in-progress blocks and return unused block ids
//

static
void
skc_finalize(__global SKC_ATOMICS_TYPE         volatile * restrict const atomics_context,
             __global SKC_ATOMICS_TYPE         volatile * restrict const atomics_cohort,
             __global uint                              * restrict const block_pool_ids,
             __global uint                              * restrict const block_pool,
             __global ulong                             * restrict const ttsk_ryx,
             __local  struct skc_subgroup_smem volatile * restrict const smem,
             union skc_rasterize_cmd                               const cmd)
{
  //
  // flush non-empty bins
  //
  // accelerate this iteration/search with a subgroup operation
  //
  for (int jj=0; jj<SKC_TILE_HASH_BIN_COUNT; jj++)
    {
      if (smem->bin.aN.count[jj] > 0)
        {
#if PRINTF_BLOCK_COUNT
          if (skc_sub_group_local_id() == 0)
            printf("%u\n",min(smem->bin.aN.count[jj],(uint)SKC_BLOCK_LARGE_ID_COUNT));
#endif

          uint const pool_idx = smem->bin.aN.ttsb_id[jj] * SKC_BLOCK_LARGE_ID_COUNT + skc_sub_group_local_id();

          for (uint ii=0; ii<SKC_BLOCK_LARGE_ID_COUNT; ii+=SKC_RASTERIZE_SUBGROUP_SIZE)
            block_pool[pool_idx + ii] = smem->bin.aN.block[jj][skc_sub_group_local_id() + ii];
        }

      //
      // FIXME -- vectorize
      //
    }

  //
  // flush work-in-progress ryx keys
  //
  uint const ttsb_id_count = smem->pool.count;

  if (ttsb_id_count > 0)
    {
      uint idx_base;

#if ( SKC_RASTERIZE_SUBGROUP_SIZE > 1 )
      idx_base = 0;
      if (skc_is_lane_first())
#endif
        {
          idx_base = atomic_fetch_add_explicit(SKC_ATOMICS_CAST(atomics_cohort + 0), // ->ttsk_ryx_count,
                                               ttsb_id_count,
                                               memory_order_relaxed,
                                               memory_scope_device);
        }

#if ( SKC_RASTERIZE_SUBGROUP_SIZE > 1 )
      idx_base = skc_sub_group_first(idx_base);
#endif

      for (uint ii=skc_sub_group_local_id(); ii<ttsb_id_count; ii+=SKC_RASTERIZE_SUBGROUP_SIZE)
        {
          ttsk_ryx[idx_base + ii] = skc_make_ttsk_ryx(smem,cmd.raster_cohort_id,ii);          
        }
    }

  //
  // return remaining block ids back to the pool
  //
  if (ttsb_id_count < SKC_RASTERIZE_POOL_SIZE)
    {
      uint const ttsb_id_rem = SKC_RASTERIZE_POOL_SIZE - ttsb_id_count;
      uint       id_base;

#if ( SKC_RASTERIZE_SUBGROUP_SIZE > 1 )
      id_base = 0;
      if (skc_is_lane_first())
#endif
        {
          id_base = atomic_fetch_add_explicit(SKC_ATOMICS_CAST(atomics_context + 1), // ->ring_writes
                                              ttsb_id_rem,
                                              memory_order_relaxed,
                                              memory_scope_device);
        }

#if ( SKC_RASTERIZE_SUBGROUP_SIZE > 1 )
      id_base = skc_sub_group_first(id_base);
#endif

      for (uint ii=skc_sub_group_local_id(); ii<ttsb_id_rem; ii+=SKC_RASTERIZE_SUBGROUP_SIZE)
        block_pool_ids[id_base + ii] = smem->pool.aN.ttsb_id[ttsb_id_count + ii];
    }
}

//
// If there are lanes that were unable to append to a bin because
// their hashes collided with a bin's current ryx key then those bins
// must be ejected.
//
// Note that we do not eject "full" bins because lazily waiting for a
// collision results in simpler code.
//

static
void
skc_flush(__global SKC_ATOMICS_TYPE         volatile * restrict const atomics_context,
          __global SKC_ATOMICS_TYPE         volatile * restrict const atomics_cohort,
          __global uint                              * restrict const block_pool_ids,
          __global uint                              * restrict const block_pool,
          __global ulong                             * restrict const ttsk_ryx,
          __local  struct skc_subgroup_smem volatile * restrict const smem,
          union skc_rasterize_cmd                               const cmd,
          SKC_RASTERIZE_UINT                                    const hash,
          SKC_RASTERIZE_UINT                                    const yx,
          SKC_RASTERIZE_PREDICATE                                     is_collision) // pass by value
{
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //

  //
  // get local TTSB ID queue count
  //
  uint ttsb_id_count  = smem->pool.count; // scalar

  // init hash bit mask
  uint component_mask = 0;

  for (int cc=0; cc<SKC_RASTERIZE_VECTOR_SIZE; cc++)
    {
      // if no collision continue
      if (((int*)&is_collision)[cc] == 0)
        continue;

      uint const winner        = ((uint*)&hash)[cc];
      uint const component_bit = 1u << winner;

      // if already processed this hash then continue
      if (component_mask & component_bit)
        continue;

      // update component mask
      component_mask |= component_bit;

        //
        // ensure there is at least one TTSK and TTSB ID
        //
        if (ttsb_id_count >= SKC_RASTERIZE_POOL_SIZE)
          {
            //
            // update remaining count
            //
            ttsb_id_count = 0;

            //
            // flush accumulated ttsk_ryx keys
            //
            uint const idx = atomic_fetch_add_explicit(SKC_ATOMICS_CAST(atomics_cohort + 0), // ->ttsk_ryx_count,
                                                       SKC_RASTERIZE_POOL_SIZE,
                                                       memory_order_relaxed,
                                                       memory_scope_device);

            for (uint ii=0; ii<SKC_RASTERIZE_POOL_SIZE; ii+=SKC_RASTERIZE_SUBGROUP_SIZE)
              {
                ttsk_ryx[idx + ii] = skc_make_ttsk_ryx(smem,cmd.raster_cohort_id,ii);
              }

            //
            // allocate more ttsb ids from pool
            //
            uint const id = atomic_fetch_add_explicit(SKC_ATOMICS_CAST(atomics_context + 0), // ->ring_reads
                                                      SKC_RASTERIZE_POOL_SIZE,
                                                      memory_order_relaxed,
                                                      memory_scope_device);

            for (uint ii=0; ii<SKC_RASTERIZE_POOL_SIZE; ii+=SKC_RASTERIZE_SUBGROUP_SIZE)
              smem->pool.aN.ttsb_id[ii] = block_pool_ids[id + ii];
          }

      //
      // flush this block to the pool
      //
      if (smem->bin.aN.count[winner] > 0)
        {
#if PRINTF_BLOCK_COUNT
          if (skc_sub_group_local_id() == 0)
            printf("%u\n",min(smem->bin.aN.count[winner],(uint)SKC_BLOCK_LARGE_ID_COUNT));
#endif
          uint const pool_idx = smem->bin.aN.ttsb_id[winner] * SKC_BLOCK_LARGE_ID_COUNT;

          for (uint ii=0; ii<SKC_BLOCK_LARGE_ID_COUNT; ii+=SKC_RASTERIZE_SUBGROUP_SIZE)
            block_pool[pool_idx + ii] = smem->bin.aN.block[winner][ii];
        }

      //
      // invalidate the winning block
      //
      smem->bin.vN.block[winner] = ( SKC_TTS_INVALID );

      //
      // update bin with winning yx, new ttsb id and zero count
      //
      // all lanes are loading/storing from/to the same index
      //
      smem->bin.aN.count  [winner] = 0;
      smem->bin.aN.ttsb_id[winner] = smem->pool.aN.ttsb_id[ttsb_id_count];
      smem->bin.aN.yx     [winner] = smem->pool.aN.yx     [ttsb_id_count] = ((uint*)&yx)[cc];

      //
      // update count
      //
      ttsb_id_count += 1;
    }

  //
  // save count
  //
  smem->pool.count = ttsb_id_count;

#else
  //
  // SIMT
  //

  //
  // get local TTSB ID queue count
  //
  uint ttsb_id_count = smem->pool.count; // scalar

  do {
    //
    // only one lane will win!
    //
    if (is_collision)
      smem->subgroup.winner = hash;

    //
    // ensure there is at least one TTSK and TTSB ID
    //
    if (ttsb_id_count >= SKC_RASTERIZE_POOL_SIZE)
      {
        //
        // update remaining count
        //
        ttsb_id_count = 0;

        //
        // flush accumulated ttsk_ryx keys
        //
        uint idx = 0;

        if (skc_is_lane_first())
          {
            idx = atomic_fetch_add_explicit(SKC_ATOMICS_CAST(atomics_cohort + 0), // ->ttsk_ryx_count,
                                            SKC_RASTERIZE_POOL_SIZE,
                                            memory_order_relaxed,
                                            memory_scope_device);
          }

        idx = skc_sub_group_first(idx) + skc_sub_group_local_id();

        for (uint ii=0; ii<SKC_RASTERIZE_POOL_SIZE; ii+=SKC_RASTERIZE_SUBGROUP_SIZE)
          {
            ttsk_ryx[idx + ii] = skc_make_ttsk_ryx(smem,cmd.raster_cohort_id,skc_sub_group_local_id() + ii);
          }

        //
        // allocate more ttsb ids from pool
        //
        uint id = 0;

        if (skc_is_lane_first())
          {
            id = atomic_fetch_add_explicit(SKC_ATOMICS_CAST(atomics_context + 0), // ->ring_reads
                                           SKC_RASTERIZE_POOL_SIZE,
                                           memory_order_relaxed,
                                           memory_scope_device);
          }

        id  = skc_sub_group_first(id) + skc_sub_group_local_id();
        for (uint ii=0; ii<SKC_RASTERIZE_POOL_SIZE; ii+=SKC_RASTERIZE_SUBGROUP_SIZE)
          smem->pool.aN.ttsb_id[skc_sub_group_local_id() + ii] = block_pool_ids[id + ii];
      }

    //
    // which bin is being ejected?
    //
    uint const winner = smem->subgroup.winner;

    //
    // which colliding hash is taking over the bin?
    //
    SKC_RASTERIZE_PREDICATE const is_winner = is_collision && (hash == winner);

    //
    // all lanes with the same has will try to store but
    // only one lane will win
    //
    if (is_winner)
      smem->pool.aN.yx[ttsb_id_count] = yx;

    //
    // flush this block to the pool
    //
    if (smem->bin.aN.count[winner] > 0)
      {
#if PRINTF_BLOCK_COUNT
        if (skc_sub_group_local_id() == 0)
          printf("%u\n",min(smem->bin.aN.count[winner],(uint)SKC_BLOCK_LARGE_ID_COUNT));
#endif
        SKC_RASTERIZE_UINT const pool_idx = smem->bin.aN.ttsb_id[winner] * SKC_BLOCK_LARGE_ID_COUNT + skc_sub_group_local_id();

        for (uint ii=0; ii<SKC_BLOCK_LARGE_ID_COUNT; ii+=SKC_RASTERIZE_SUBGROUP_SIZE)
          block_pool[pool_idx + ii] = smem->bin.aN.block[winner][skc_sub_group_local_id() + ii];
      }

    //
    // invalidate the winning block
    //
    // FIXME -- use the widest type possible
    //
    for (uint ii=0; ii<SKC_BLOCK_LARGE_ID_COUNT; ii+=SKC_RASTERIZE_SUBGROUP_SIZE)
      smem->bin.aN.block[winner][skc_sub_group_local_id() + ii] = SKC_TTS_INVALID;

    //
    // remove all lanes with this hash
    //
    is_collision = is_collision && !is_winner;

    //
    // update bin with winning yx, new ttsb id and zero count
    //
    // all lanes are loading/storing from/to the same index
    //
    smem->bin.aN.count  [winner] = 0;
    smem->bin.aN.ttsb_id[winner] = smem->pool.aN.ttsb_id[ttsb_id_count];
    smem->bin.aN.yx     [winner] = smem->pool.aN.yx     [ttsb_id_count];

    //
    // update count
    //
    ttsb_id_count += 1;

    //
    // exit if nothing left to do
    //
  } while (sub_group_any(is_collision));

  //
  // save count
  //
  smem->pool.count = ttsb_id_count;

#endif
}

//
// scatter scan max
//
static
SKC_RASTERIZE_UINT
skc_scatter_scan_max(__local struct skc_subgroup_smem volatile * restrict const smem,
                     SKC_RASTERIZE_FLOAT                                  const iss,
                     SKC_RASTERIZE_FLOAT                                  const ess)
{
  //
  // prefix sums determine which lanes we're going to work on next
  //
  SKC_RASTERIZE_PREDICATE const is_scratch_store = (iss > 0) && (ess < SKC_RASTERIZE_SUBGROUP_SEGMENT_COUNT);
  SKC_RASTERIZE_UINT      const scratch_idx      = SKC_CONVERT(SKC_RASTERIZE_UINT)(max(ess,0.0f));

#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
#ifdef SKC_RASTERIZE_SIMD_USES_SMEM
  //
  // SIMD APPROACH 1: SIMT'ISH
  //

  // zero the volatile smem scratchpad using vector syntax
  smem->subgroup.vN.scratch[0] = ( 0 );

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P)                           \
  if (is_scratch_store C)                               \
    smem->subgroup.aN.scratch[scratch_idx C] = I;

  SKC_RASTERIZE_VECTOR_EXPAND();

  // propagate lanes to right using max scan
  SKC_RASTERIZE_UINT const scratch = smem->subgroup.vN.scratch[0];
  SKC_RASTERIZE_UINT const source  = skc_sub_group_scan_inclusive_max(scratch);

#else
  //
  // SIMD APPROACH 2: SCALAR'ISH
  //

  SKC_RASTERIZE_UINT source = ( 0 );

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P)                   \
  if (is_scratch_store C)                       \
    ((uint *)&source)[scratch_idx C] = I;

  SKC_RASTERIZE_VECTOR_EXPAND();

  for (uint ii=1; ii<SKC_RASTERIZE_SUBGROUP_SEGMENT_COUNT; ii++)
    ((uint *)&source)[ii] = max(((uint *)&source)[ii-1],((uint *)&source)[ii]);
#endif

#else
  //
  // SIMT
  //

  //
  // zero the volatile smem scratchpad using vector syntax
  //
  smem->subgroup.vN.scratch[skc_sub_group_local_id()] = ( 0 );

  //
  // store source lane at starting lane
  //
  if (is_scratch_store)
    smem->subgroup.aN.scratch[scratch_idx] = skc_sub_group_local_id();

  //
  // propagate lanes to right using max scan
  //
  SKC_RASTERIZE_UINT const scratch = smem->subgroup.vN.scratch[skc_sub_group_local_id()];
  SKC_RASTERIZE_UINT const source  = skc_sub_group_scan_inclusive_max(scratch);
#endif

  return source;
}

//
// sliver lines into subpixels
//

static
void
skc_sliver(__global SKC_ATOMICS_TYPE         volatile * restrict const atomics_context,
           __global SKC_ATOMICS_TYPE         volatile * restrict const atomics_cohort,
           __global uint                              * restrict const block_pool_ids,
           __global uint                              * restrict const block_pool,
           __global ulong                             * restrict const ttsk_ryx,
           __local  struct skc_subgroup_smem volatile * restrict const smem,
           union skc_rasterize_cmd                               const cmd,
           SKC_RASTERIZE_FLOAT                                   const l0x,
           SKC_RASTERIZE_FLOAT                                   const l0y,
           SKC_RASTERIZE_FLOAT                                   const l1x,
           SKC_RASTERIZE_FLOAT                                   const l1y)
{
  //
  // Y-SLIVERING
  // -----------
  //
  // immediately sliver all multi-pixel lines in into 1-pixel high
  // lines
  //
  // note this implicitly squelches horizontal lines
  //
  // there is another test for horizontal lines after x-slivering
  // is complete
  //

  //
  // save original direction and work with increasing y
  //
  SKC_RASTERIZE_PREDICATE is_dy_neg = l0y > l1y;

  //
  // branchless select
  //
  SKC_RASTERIZE_FLOAT const a0y     = select(l0y,l1y,is_dy_neg);
  SKC_RASTERIZE_FLOAT const a0x     = select(l0x,l1x,is_dy_neg);
  SKC_RASTERIZE_FLOAT const a1y     = select(l1y,l0y,is_dy_neg);
  SKC_RASTERIZE_FLOAT const a1x     = select(l1x,l0x,is_dy_neg);

  //
  // save 1/dy
  //
  SKC_RASTERIZE_FLOAT const y_denom = native_recip(a1y - a0y);

  //
  // how many non-horizontal subpixel y-axis slivers are there?
  //
  SKC_RASTERIZE_FLOAT const y_min   = floor(a0y * SKC_SUBPIXEL_Y_SCALE_DOWN);
  SKC_RASTERIZE_FLOAT const y_max   = ceil (a1y * SKC_SUBPIXEL_Y_SCALE_DOWN);
  SKC_RASTERIZE_FLOAT       y_segs  = y_max - y_min;

  //
  // inclusive subgroup scan of y_segs
  //
  SKC_RASTERIZE_FLOAT       y_iss   = skc_sub_group_scan_inclusive_add_float(y_segs);
  SKC_RASTERIZE_FLOAT       y_ess   = y_iss - y_segs;
  float                     y_rem   = skc_sub_group_last_float(y_iss);

  //
  // these values don't matter on first iteration
  //
  SKC_RASTERIZE_FLOAT c1x_prev = 0;
  SKC_RASTERIZE_FLOAT c1y_prev = 0;

  //
  // loop until done
  //
  while (y_rem > 0)
    {
      //
      // distribute work across lanes
      //
      SKC_RASTERIZE_UINT const y_source = skc_scatter_scan_max(smem,y_iss,y_ess);

      //
      // get line at y_source line
      //
      SKC_RASTERIZE_FLOAT const b0y = skc_sub_group_shuffle(a0y,y_source);
      SKC_RASTERIZE_FLOAT const b0x = skc_sub_group_shuffle(a0x,y_source);
      SKC_RASTERIZE_FLOAT const b1y = skc_sub_group_shuffle(a1y,y_source);
      SKC_RASTERIZE_FLOAT const b1x = skc_sub_group_shuffle(a1x,y_source);

      //
      // every lane will create a 1 pixel tall line "sliver"
      //
      // FIXME -- this gets expanded on SIMD
      //
      // if numerator == 1 then this is the first lane
      // if numerator == s then this is the last  lane
      //
      SKC_RASTERIZE_FLOAT     const y_delta    = skc_delta_offset() - skc_sub_group_shuffle(y_ess,y_source);
      SKC_RASTERIZE_FLOAT     const y_count    = skc_sub_group_shuffle(y_segs,y_source);

      SKC_RASTERIZE_PREDICATE const is_y_first = (y_delta == 1.0f);
      SKC_RASTERIZE_PREDICATE const is_y_last  = (y_delta >= y_count);

      //
      // calculate "right" line segment endpoint
      //
      SKC_RASTERIZE_FLOAT       c1y = (y_delta + skc_sub_group_shuffle(y_min,y_source)) * SKC_SUBPIXEL_Y_SCALE_UP;
      SKC_RASTERIZE_FLOAT const c_t = (c1y - b0y) * skc_sub_group_shuffle(y_denom,y_source);
      SKC_RASTERIZE_FLOAT       c1x = round(SKC_LERP(b0x,b1x,c_t));

      //
      // override c1 if this is last point
      //
      c1y = select(c1y,b1y,is_y_last);
      c1x = select(c1x,b1x,is_y_last);

      //
      // shuffle up "left" line segment endpoint
      //
      // NOTE: Intel's shuffle_up is unique with its elegant
      // "previous" argument so don't get used to it
      //
      SKC_RASTERIZE_FLOAT c0y = skc_sub_group_shuffle_up_1(c1y_prev,c1y);
      SKC_RASTERIZE_FLOAT c0x = skc_sub_group_shuffle_up_1(c1x_prev,c1x);

      //
      // override shuffle up if this is the first line segment
      //
      c0y = select(c0y,b0y,is_y_first);
      c0x = select(c0x,b0x,is_y_first);

      //
      // save previous right endpoint
      //
      c1x_prev = c1x;
      c1y_prev = c1y;

      //
      // decrement by subgroup size
      //
      y_iss -= SKC_RASTERIZE_SUBGROUP_SEGMENT_COUNT;
      y_ess -= SKC_RASTERIZE_SUBGROUP_SEGMENT_COUNT;
      y_rem -= SKC_RASTERIZE_SUBGROUP_SEGMENT_COUNT;

      //
      // debug
      //
#if 0
      if (is_y_valid)
        printf("{ { %5.0f, %5.0f }, { %5.0f, %5.0f } },\n",
               is_dy_neg ? c1x : c0x,
               is_dy_neg ? c1y : c0y,
               is_dy_neg ? c0x : c1x,
               is_dy_neg ? c0y : c1y);
#endif

      //
      // X-SLIVERING
      // -----------
      //
      // now sliver 1-pixel high lines into 1-pixel wide lines
      //
      // save original direction and work with increasing x
      //
      SKC_RASTERIZE_PREDICATE is_dx_neg = c0x > c1x;

      //
      // branchless select
      //
      SKC_RASTERIZE_FLOAT const d0y      = select(c0y,c1y,is_dx_neg);
      SKC_RASTERIZE_FLOAT const d0x      = select(c0x,c1x,is_dx_neg);
      SKC_RASTERIZE_FLOAT const d1y      = select(c1y,c0y,is_dx_neg);
      SKC_RASTERIZE_FLOAT const d1x      = select(c1x,c0x,is_dx_neg);

      //
      // save 1/dy
      //
      SKC_RASTERIZE_FLOAT const x_denom  = native_recip(d1x - d0x);

      //
      // how many non-horizontal subpixel y-axis slivers are there?
      //
      SKC_RASTERIZE_FLOAT const x_min    = floor(d0x * SKC_SUBPIXEL_X_SCALE_DOWN);
      SKC_RASTERIZE_FLOAT const x_max    = ceil (d1x * SKC_SUBPIXEL_X_SCALE_DOWN);
      SKC_RASTERIZE_FLOAT const x_segs   = x_max - x_min;

      //
      // inclusive subgroup scan of y_segs
      //
      SKC_RASTERIZE_FLOAT       x_iss    = skc_sub_group_scan_inclusive_add_float(x_segs);
      SKC_RASTERIZE_FLOAT       x_ess    = x_iss - x_segs;
      float                     x_rem    = skc_sub_group_last_float(x_iss);

      //
      // these values don't matter on first iteration
      //
      SKC_RASTERIZE_FLOAT       f1x_prev = 0;
      SKC_RASTERIZE_FLOAT       f1y_prev = 0;

      //
      // loop until done
      //
      while (x_rem > 0)
        {
          //
          // distribute work across lanes
          //
          SKC_RASTERIZE_UINT const x_source = skc_scatter_scan_max(smem,x_iss,x_ess);

          //
          // get line at y_source line
          //
          SKC_RASTERIZE_FLOAT const e0x = skc_sub_group_shuffle(d0x,x_source);
          SKC_RASTERIZE_FLOAT const e0y = skc_sub_group_shuffle(d0y,x_source);
          SKC_RASTERIZE_FLOAT const e1x = skc_sub_group_shuffle(d1x,x_source);
          SKC_RASTERIZE_FLOAT const e1y = skc_sub_group_shuffle(d1y,x_source);

          //
          // every lane will create a 1 pixel tall line "sliver"
          //i
          // FIXME -- this gets expanded on SIMD
          //
          // if numerator == 1 then this is the first lane
          // if numerator == s then this is the last  lane
          //
          SKC_RASTERIZE_FLOAT     const x_delta    = skc_delta_offset() - skc_sub_group_shuffle(x_ess,x_source);
          SKC_RASTERIZE_FLOAT     const x_count    = skc_sub_group_shuffle(x_segs,x_source);

          SKC_RASTERIZE_PREDICATE const is_x_first = (x_delta == 1.0f);
          SKC_RASTERIZE_PREDICATE const is_x_last  = (x_delta >= x_count);

          //
          // calculate "right" line segment endpoint
          //
          SKC_RASTERIZE_FLOAT       f1x = (x_delta + skc_sub_group_shuffle(x_min,x_source)) * SKC_SUBPIXEL_X_SCALE_UP;
          SKC_RASTERIZE_FLOAT const f_t = (f1x - e0x) * skc_sub_group_shuffle(x_denom,x_source);
          SKC_RASTERIZE_FLOAT       f1y = round(SKC_LERP(e0y,e1y,f_t));

          //
          // override c1 if this is last point
          //
          f1x = select(f1x,e1x,is_x_last);
          f1y = select(f1y,e1y,is_x_last);

          //
          // shuffle up "left" line segment endpoint
          //
          // NOTE: Intel's shuffle_up is unique with its elegant
          // "previous" argument so don't get used to it
          //
          SKC_RASTERIZE_FLOAT f0x = skc_sub_group_shuffle_up_1(f1x_prev,f1x);
          SKC_RASTERIZE_FLOAT f0y = skc_sub_group_shuffle_up_1(f1y_prev,f1y);

          //
          // override shuffle up if this is the first line segment
          //
          f0x = select(f0x,e0x,is_x_first);
          f0y = select(f0y,e0y,is_x_first);

          //
          // save previous right endpoint
          //
          f1x_prev = f1x;
          f1y_prev = f1y;

          //
          // decrement by subgroup size
          //
          x_iss -= SKC_RASTERIZE_SUBGROUP_SEGMENT_COUNT;
          x_ess -= SKC_RASTERIZE_SUBGROUP_SEGMENT_COUNT;
          x_rem -= SKC_RASTERIZE_SUBGROUP_SEGMENT_COUNT;

          //
          // only non-horizontal subpixel lines are valid
          //
          SKC_RASTERIZE_PREDICATE is_active = f0y != f1y;

          //
          // if no lanes are active then continue
          //
          // FIXME -- THIS SIMPLE SUB_GROUP_ANY TEST SIGNIFICANTLY
          // IMPACTS PERFORMANCE (+12% ?)
          //
          // IT SHOULDN'T !!!
          //
#if 0
          if (!skc_sub_group_any(is_active))
            continue;
#endif

          //
          // Option 1: use SLM for explicitly managed coalesced stores
          //
          // 1. which tile does this line belong?
          // 2. hash tile coordinates
          // 3. lookup hash
          // 4. if tile matches then SLM append keys
          // 5. if tile doesn't match
          //   a. flush
          //   b. create new TTSK_RYX
          //   c. obtain TTSB block from pool
          //   d. goto 3.
          //

          //
          // Option 2: rely on L1/L2/L3 to mitigate non-coalesced stores
          //
          // 1. which tile does this line belong?
          // 2. hash tile coordinates
          // 3. lookup hash
          // 4. if tile matches then GMEM append keys
          // 5. if tile doesn't match
          //   a. flush (and invalidate empty elems)
          //   b. create new TTSK_RYX
          //   c. obtain TTSB block from pool
          //   d. goto 3.
          //

          //
          // The virtual rasterization surface is very large and
          // signed: +/- ~64K-256K, depending on the architecture.
          //
          // Rasters must be clipped to the virtual surface and,
          // optionally, clipped even further on a per raster
          // basis.
          //

          //
          // Clip to the per-raster clip
          //

          /*

            CLIP HERE

          */

          //
          // Hash the tile coordinates
          //
          // This table lists nominal values for each architecture.
          // We want to choose values that are naturally fit the
          // "width" of the architecture.
          //
          //   SIMD   RANGE   BITS  MAX RANGE  MAX BINS  HASH BITS
          //   ----  -------  ----  ---------  --------  ---------
          //     4   [0,  4]    3    [0,  7]      10      mod(10)  <-- SSE42, ?
          //     8   [0,  8]    4    [0, 15]       8         3     <-- GEN*,AVX*
          //    16   [0, 16]    5    [0, 31]       6      mod(6)   <-- GEN*,?
          //    32   [0, 32]    6    [0, 63]       5      mod(5)   <-- CUDA,PowerVR,Adreno,GEN*
          //    64   [0, 64]    7    [0,127]       4         2     <-- AMD Radeon
          //
          // NOTE: When possible, bias the hash toward using more y
          // bits because of:
          //
          //   1. the 90 degree counter-clockwise rotation that we put
          //      in place to offset the render-time clockwise
          //      rotation
          //
          //   2. the likely presence of left-to-right or
          //      right-to-left glyphs.
          //
          // For power-of-two bins, the hash is easy.
          //
          // For non-power-of-two, we may want to either implement a
          // fast mod (compiler should do this for us... hahahaha) or
          // drop down to the next power-of-two.
          //

          SKC_RASTERIZE_PREDICATE const swap   = is_dy_neg ^ is_dx_neg;

          SKC_RASTERIZE_INT       const z0y    = SKC_CONVERT(SKC_RASTERIZE_INT)(select(f0y,f1y,swap));
          SKC_RASTERIZE_INT       const z1y    = SKC_CONVERT(SKC_RASTERIZE_INT)(select(f1y,f0y,swap));

          SKC_RASTERIZE_INT       const z0x    = SKC_CONVERT(SKC_RASTERIZE_INT)(select(f0x,f1x,swap));
          SKC_RASTERIZE_INT       const z1x    = SKC_CONVERT(SKC_RASTERIZE_INT)(select(f1x,f0x,swap));

          SKC_RASTERIZE_INT       const min_y  = min(z0y,z1y);
          SKC_RASTERIZE_INT       const max_y  = max(z0y,z1y);
          SKC_RASTERIZE_INT       const tile_y = min_y >> SKC_SUBTILE_RESL_Y_LOG2;

          SKC_RASTERIZE_UINT      const ty     = SKC_AS(SKC_RASTERIZE_UINT)(max_y) - (SKC_AS(SKC_RASTERIZE_UINT)(min_y) & SKC_SUBTILE_MASK_Y);
          SKC_RASTERIZE_UINT            dy     = SKC_AS(SKC_RASTERIZE_UINT)(z1y - z0y);

          dy = (dy - (dy >> 31)) << 26; // map [1,32] to [0,31]

          SKC_RASTERIZE_INT       const min_x  = min(z0x,z1x);
          SKC_RASTERIZE_INT       const max_x  = max(z0x,z1x);
          SKC_RASTERIZE_INT       const tile_x = min_x >> SKC_SUBTILE_RESL_X_LOG2;

          SKC_RASTERIZE_UINT      const tx     = SKC_AS(SKC_RASTERIZE_UINT)(max_x) - (SKC_AS(SKC_RASTERIZE_UINT)(min_x) & SKC_SUBTILE_MASK_X);
          SKC_RASTERIZE_UINT      const sx     = SKC_AS(SKC_RASTERIZE_UINT)(max_x - min_x);

          SKC_RASTERIZE_UINT      const tts    = dy | (ty << 16) | (sx << 10) | tx;

          SKC_RASTERIZE_UINT      const hash   = ((SKC_AS(SKC_RASTERIZE_UINT)(tile_y) & SKC_TILE_HASH_Y_MASK) << SKC_TILE_HASH_X_BITS |
                                                  (SKC_AS(SKC_RASTERIZE_UINT)(tile_x) & SKC_TILE_HASH_X_MASK));

          SKC_RASTERIZE_UINT      const yx     = ((SKC_AS(SKC_RASTERIZE_UINT)(tile_y) << 12) |
                                                  (SKC_AS(SKC_RASTERIZE_UINT)(tile_x) & 0xFFF));

          //
          // debug
          //
#if PRINTF_ENABLE

#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P)                                           \
          if (is_active C)                                              \
            printf("{ { %5d, %5d }, { %5d, %5d } (* %2u *) },\n",z0x C,z0y C,z1x C,z1y C,hash C);

          SKC_RASTERIZE_VECTOR_EXPAND();
#else
          if (is_active)
            printf("{ { %5d, %5d }, { %5d, %5d } } (* %2u *),\n",z0x,z0y,z1x,z1y,hash);
#endif

#endif
          //
          // flush all active lanes
          //
          while (true)
            {
              //
              // either gather load or vector load+shuffle the yx keys
              //
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1)
              SKC_RASTERIZE_BIN       const yx_bin     = smem->bin.vN.yx;
              SKC_RASTERIZE_UINT      const yx_cur     = shuffle(yx_bin,hash);
#else
              SKC_RASTERIZE_UINT      const yx_cur     = smem->bin.aN.yx[hash];
#endif

              //
              // does yx for lane match yx for hash?
              //
              SKC_RASTERIZE_UINT      const active_yx  = is_active ? yx : SKC_RASTERIZE_YX_INVALID;
              SKC_RASTERIZE_PREDICATE const is_match   = (yx_cur == active_yx);

              //
              // OpenCL spec: "When casting a bool to a vector integer
              // data type, the vector components will be set to -1
              // (i.e. all bits set) if the vector bool value is true
              // and 0 otherwise.
              //
#if ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 0 )
              SKC_RASTERIZE_UINT      const h_match    = (SKC_RASTERIZE_UINT)is_match;
#else
              SKC_RASTERIZE_UINT      const h_match    = abs(is_match); // {-1,0} -> {+1,0}
#endif
              //
              // how many elements already in the bin?
              //
              SKC_RASTERIZE_UINT      const h_shl      = hash * SKC_TILE_HASH_BIN_BITS;
              SKC_RASTERIZE_UINT      const h          = h_match << h_shl;

              //
              // prefix sum all of the bins in parallel
              //
              SKC_RASTERIZE_UINT      const h_iss      = skc_sub_group_scan_inclusive_add_uint(h);
              SKC_RASTERIZE_UINT      const h_total    = skc_sub_group_last_uint(h_iss);

              //
              // current bin counts
              //
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1)
              SKC_RASTERIZE_BIN       const count_bin  = smem->bin.vN.count;
              SKC_RASTERIZE_UINT      const count_cur  = shuffle(count_bin,hash);
#else
              SKC_RASTERIZE_UINT      const count_cur  = smem->bin.aN.count[hash];
#endif

              //
              // calculate where each cache-hit and in-bounds tts should be stored
              //
              SKC_RASTERIZE_UINT      const ttsb_index = (h_iss   >> h_shl & SKC_TILE_HASH_BIN_MASK) + count_cur - 1;
              SKC_RASTERIZE_UINT      const count_new  = (h_total >> h_shl & SKC_TILE_HASH_BIN_MASK) + count_cur;

              //
              // which lanes can append to a matching bin?
              //
              SKC_RASTERIZE_PREDICATE const is_append  = is_match && (ttsb_index < SKC_BLOCK_LARGE_ID_COUNT);

              //
              // scatter append tts elements to bin blocks
              //
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1)
              //
              // SIMD
              //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P)                                           \
              if (is_append C)                                          \
                {                                                       \
                  smem->bin.aN.block[hash C][ttsb_index C] = tts       C; \
                  smem->bin.aN.count[hash C]               = count_new C; \
                }

              SKC_RASTERIZE_VECTOR_EXPAND();
#else
              //
              // SIMT
              //
              if (is_append)
                {
                  smem->bin.aN.block[hash][ttsb_index] = tts;
                  smem->bin.aN.count[hash]             = count_new; // it's ok if this is > SKC_BLOCK_LARGE_ID_COUNT
                }
#endif
              //
              // try to keep predicate updates SIMD-friendly and
              // outside of predicated code paths -- this is not
              // always how we would normally do things on SIMT but
              // either approach is acceptable
              //

              //
              // mask off lanes/components that successfully appended
              //
              is_active = is_active && !is_append;

              //
              // are there any active lanes left?
              //
              if (!skc_sub_group_any(is_active))
                break;

              //
              // If there are lanes that were unable to append to a
              // bin because their hashes collided with a bin's
              // current ryx key then those bins must be ejected.
              //
              // Note that we do not eject "full" bins because lazily
              // waiting for a collision results in simpler code.
              //
              skc_flush(atomics_context,
                        atomics_cohort,
                        block_pool_ids,
                        block_pool,
                        ttsk_ryx,
                        smem,
                        cmd,
                        hash,
                        yx,
                        is_active);
            }
        }
    }
}

//
// INITIALIZE SMEM
//
// Note that SIMD/SIMT have nearly the same syntax.
//
static
void
skc_smem_init(__global SKC_ATOMICS_TYPE         volatile * restrict const atomics_context,
              __global SKC_ATOMICS_TYPE         volatile * restrict const atomics_cohort,
              __global uint                              * restrict const block_pool_ids,
              __local  struct skc_subgroup_smem volatile * restrict const smem)
{
  //
  // initialize smem bins
  //
#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )
  smem->bin.vN.count = ( 0 );
  smem->bin.vN.yx    = ( SKC_RASTERIZE_YX_INIT );
#else
  {
    int ii = skc_sub_group_local_id();

#if   ( SKC_TILE_HASH_BIN_COUNT < SKC_RASTERIZE_SUBGROUP_SEGMENT_COUNT )
    if (ii < SKC_TILE_HASH_BIN_COUNT)
#elif ( SKC_TILE_HASH_BIN_COUNT > SKC_RASTERIZE_SUBGROUP_SEGMENT_COUNT )
    for (; ii<SKC_TILE_HASH_BIN_COUNT; ii+=SKC_RASTERIZE_SUBGROUP_SIZE)
#endif
      {
        smem->bin.aN.count[ii] = ( 0 );
        smem->bin.aN.yx   [ii] = ( SKC_RASTERIZE_YX_INIT );
      }
  }
#endif

  //
  // initialize smem pool
  //
  {
    uint id;

#if ( SKC_RASTERIZE_SUBGROUP_SIZE > 1)
    id = 0;

    if (skc_is_lane_first())
#endif
      {
        id = atomic_fetch_add_explicit(SKC_ATOMICS_CAST(atomics_context + 0), // ->ring_reads
                                       SKC_RASTERIZE_POOL_SIZE,
                                       memory_order_relaxed,
                                       memory_scope_device);

      }
#if ( SKC_RASTERIZE_SUBGROUP_SIZE > 1)
    id  = skc_sub_group_first(id);
    id += skc_sub_group_local_id();
#endif

    for (int ii=0; ii<SKC_RASTERIZE_POOL_SIZE; ii+=SKC_RASTERIZE_SUBGROUP_SIZE)
      smem->pool.aN.ttsb_id[skc_sub_group_local_id() + ii] = block_pool_ids[id + ii];
  }

  //
  // pool is unused
  //
  smem->pool.count = 0;
}

//
// RASTERIZE CUBIC KERNEL
//

__kernel

#ifdef SKC_ARCH_AVX2
// __attribute__((vec_type_hint(float8))) // REMOVE ME -- no need for this since there is no impact
#else
__attribute__((intel_reqd_sub_group_size(SKC_RASTERIZE_SUBGROUP_SIZE)))
#endif

__attribute__((work_group_size_hint(SKC_RASTERIZE_SUBGROUP_SIZE * SKC_RASTERIZE_WORKGROUP_COUNT_SUBGROUP, 1, 1)))

void
skc_rasterize_cubic_kernel(__global ushort4             const    * restrict const cmds,
                           __global SKC_RASTERIZE_FLOAT const    * restrict const coords,
                           __global float8              const    * restrict const transforms,
                           __global SKC_ATOMICS_TYPE    volatile * restrict const atomics_context,
                           __global SKC_ATOMICS_TYPE    volatile * restrict const atomics_cohort,
                           __global uint                         * restrict const block_pool_ids,
                           __global uint                         * restrict const block_pool,
                           __global ulong                        * restrict const ttsk_ryx) // FIXME -- should be ulong
{
  //
  // declare shared memory block
  //
#if ( SKC_RASTERIZE_WORKGROUP_COUNT_SUBGROUP == 1 )
  __local struct skc_subgroup_smem volatile                  smem[1];
#else
  __local struct skc_subgroup_smem volatile                  smem_wg[SKC_RASTERIZE_WORKGROUP_COUNT_SUBGROUP];
  __local struct skc_subgroup_smem volatile * restrict const smem = smem_wg + get_sub_group_id();
#endif

  //
  // this is a subgroup/warp-centric kernel
  //
  // which subgroup in the grid is this?
  //
  // TAKE NOTE: the Intel GEN compiler appears to be recognizing
  // get_group_id(0) as a uniform but the alternative calculation used
  // when there are multiple subgroups per workgroup is not
  // cooperating and driving spillage elsewhere.
  //
#if ( SKC_RASTERIZE_WORKGROUP_COUNT_SUBGROUP == 1 )
  uint const cmd_idx = get_group_id(0);
#else
  uint const cmd_idx = get_group_id(0) * SKC_RASTERIZE_WORKGROUP_COUNT_SUBGROUP + get_sub_group_id();
#endif

  //
  // load single command for the subgroup
  //
  union skc_rasterize_cmd const cmd = { .b16v4 = cmds[cmd_idx] };

  //
  // load vertices
  //
  // The cubic coord extent is packed in slabs that are subgroup wide
  // and H float coordinates high:
  //
  //   segment | H
  //  ---------+---
  //    cubic  | 8
  //    quad   | 6
  //    line   | 4
  //
  uint const coord_idx = cmd.coord_base * (SKC_RASTERIZE_SUBGROUP_SIZE * 8) + skc_sub_group_local_id();

  SKC_RASTERIZE_FLOAT const c0x = coords[coord_idx+SKC_RASTERIZE_SUBGROUP_SIZE*0];
  SKC_RASTERIZE_FLOAT const c0y = coords[coord_idx+SKC_RASTERIZE_SUBGROUP_SIZE*1];

  SKC_RASTERIZE_FLOAT const c1x = coords[coord_idx+SKC_RASTERIZE_SUBGROUP_SIZE*2];
  SKC_RASTERIZE_FLOAT const c1y = coords[coord_idx+SKC_RASTERIZE_SUBGROUP_SIZE*3];

  SKC_RASTERIZE_FLOAT const c2x = coords[coord_idx+SKC_RASTERIZE_SUBGROUP_SIZE*4];
  SKC_RASTERIZE_FLOAT const c2y = coords[coord_idx+SKC_RASTERIZE_SUBGROUP_SIZE*5];

  SKC_RASTERIZE_FLOAT const c3x = coords[coord_idx+SKC_RASTERIZE_SUBGROUP_SIZE*6];
  SKC_RASTERIZE_FLOAT const c3y = coords[coord_idx+SKC_RASTERIZE_SUBGROUP_SIZE*7];

  //
  // load transform -- uniform across subgroup
  //
  // v8: { sx shx tx shy sy ty w0 w1 }
  //
  // NOTE THAT WE'RE SCALING UP THE TRANSFORM BY:
  //
  //   [ SKC_SUBPIXEL_RESL_X_F32, SKC_SUBPIXEL_RESL_Y_F32, 1.0f ]
  //
  // Coordinates are scaled to subpixel resolution.  All that matters
  // is that continuity is maintained between end path element
  // endpoints.
  //
  // It's the responsibility of the host to ensure that the transforms
  // are properly scaled either via intitializing a transform stack
  // with the subpixel resolution scaled identity or scaling the
  // transform before its loaded by a rasterization grid.
  //
  union skc_transform_v const tv = { .f32v8 = transforms[cmd.transform_idx] };

  //
  // apply transform
  //
  // note that we only care if the end points are rounded to subpixel precision
  //
  // FIXME -- transformation is currently affine-only support perspective later
  //
  // the affine transformation requires 8 FMA + 2 ROUND operations
  //
  SKC_RASTERIZE_FLOAT const b0x = round(c0x * tv.sx  + c0y * tv.shx + tv.tx);
  SKC_RASTERIZE_FLOAT const b0y = round(c0x * tv.shy + c0y * tv.sy  + tv.ty);

  SKC_RASTERIZE_FLOAT const t1x = c1x * tv.sx  + c1y * tv.shx + tv.tx;
  SKC_RASTERIZE_FLOAT const t1y = c1x * tv.shy + c1y * tv.sy  + tv.ty;

  SKC_RASTERIZE_FLOAT const t2x = c2x * tv.sx  + c2y * tv.shx + tv.tx;
  SKC_RASTERIZE_FLOAT const t2y = c2x * tv.shy + c2y * tv.sy  + tv.ty;

  SKC_RASTERIZE_FLOAT const t3x = round(c3x * tv.sx  + c3y * tv.shx + tv.tx);
  SKC_RASTERIZE_FLOAT const t3y = round(c3x * tv.shy + c3y * tv.sy  + tv.ty);

  //
  //
  //
#if PRINTF_ENABLE

#if ( SKC_RASTERIZE_SUBGROUP_SIZE == 1 )

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P)                                           \
  printf("{ { %.02f, %.02f }, { %.02f, %.02f },"                        \
         "  { %.02f, %.02f }, { %.02f, %.02f } },\n",                   \
         b0x C,b0y C,t1x C,t1y C,                                       \
         t2x C,t2y C,t3x C,t3y C);

  SKC_RASTERIZE_VECTOR_EXPAND();

#else

  printf("{ { %.02f, %.02f }, { %.02f, %.02f }, { %.02f, %.02f }, { %.02f, %.02f } },\n",
         b0x,b0y,t1x,t1y,t2x,t2y,t3x,t3y);

#endif

#endif

  //
  // OLD APPROACH
  // ------------
  //
  // The Spinel CUDA rasterizer was significantly more complex and
  // performed a few different tasks that are probably best kept
  // separate.
  //
  // The Spinel rasterizer Bezier held 4-element x and y coordinates
  // in adjacent lanes. This simplified intermingling of single lane
  // 4-coordinate line segments with two-lane cubic Beziers.
  //
  // After transformation of the input segments, the Spinel rasterizer
  // would test cubics for flatness and, if flat, collapse the
  // adjacent lanes into a single line lane and an empty lane.
  //
  // Any lines would then be appended to a line queue.
  //
  // Any cubics would then be subdivided.
  //
  // The reclassification process would be repeated.
  //
  // NEW APPROACH
  // ------------
  //
  // Assume we're only working with cubics in this kernel.
  //
  // Optimization: if the line segment is a special case -- a cusp,
  // has 1+ inflections, or a loop -- it might be beneficial to
  // subdivide the control cage 1+ times in order to separate the
  // flatter segments the high-velocity region(s).
  //
  // This means we want to split using [a,b] formulation to _directly_
  // subdivide producing a new control cage.
  //
  // Wang's Formula is still useful even if we subdivide once or twice
  // as it's so cheap that it might give some useful hints about where
  // the high-velocity sections of curve reside.
  //
  // But it seems like using Wang's and directly flattening to line
  // segments without any subdivision is good enough for the limited
  // set of test cases that I've tried.
  //
  // So... use Wang's Formula to estimate how many line segment are
  // required to properly flatten the cubics.
  //
  // Then use inclusive/exclusive scans to put all the lanes to work:
  //
  //   1. segmenting cubics to line segments
  //
  //   2. slivering line segments into 1-pixel high line segments
  //
  //   3. slivering 1-pixel high line segments into 1-pixel wide line
  //      segments
  //
  // MORE BACKGROUND ON NEW APPROACH
  // -------------------------------
  //
  // Two options for handling line segments:
  //
  // 1. append the line segments onto an SLM array until enough
  //    work has been accrued (Spinel does this)
  //
  // 2. immediately sliver the potentially multi-pixel line
  //    segments into subpixel lines
  //
  // The advantage of (1) is that it guarantees the slivering
  // process will, on average, always be emitting a full subgroup
  // of subpixel lines.
  //
  // The advantage of (2) is that it reduces code complexity and
  // leaves more room for SLM tile bins. The difference between
  // Spinel and Skia Compute is that Wang's Formula guarantees
  // there will be a full subgroup of multi-pixel lines unless
  // this is the final iteration.
  //
  // Note that wider GPU architectures might benefit from (1) and
  // other work accumulation strategies because it will minimize
  // partial warp workloads in the final iteration of each stage.
  //
  // So let's implement (2) for now...
  //

  //
  // And... begin!
  //
  // Estimate how many line segments are in quad/cubic curve.
  //
  // Wang's Formula will return zero if the control points are
  // collinear but we bump it up to 1.0f.
  //
  SKC_RASTERIZE_FLOAT const s_segs  = skc_wangs_formula_cubic(b0x,b0y,t1x,t1y,t2x,t2y,t3x,t3y);

  //
  // if there are free registers then precalculate the reciprocal for
  // each estimated segments since it will never change
  //
  SKC_RASTERIZE_FLOAT const s_denom = native_recip(s_segs);


  //
  // inclusive add scan of estimated line segments
  // exclusive add scan of estimated line segments
  // total number       of estimated line segments
  //
  SKC_RASTERIZE_FLOAT       s_iss   = skc_sub_group_scan_inclusive_add_float(s_segs);
  SKC_RASTERIZE_FLOAT       s_ess   = s_iss - s_segs;
  float                     s_rem   = skc_sub_group_last_float(s_iss); // scalar

  //
  // debug
  //
#if PRINTF_ENABLE

#if   ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 0)
  printf("segs: (%d) %.0f %.0f %.0f\n",skc_sub_group_local_id(),s_segs,s_iss,s_rem);
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 1)
  printf("segs: (%.0v2f) (%.0v2f) %.0f\n",s_segs,s_iss,s_rem);
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 2)
  printf("segs: (%.0v4f) (%.0v4f) %.0f\n",s_segs,s_iss,s_rem);
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 3)
  printf("segs: (%.0v8f) (%.0v8f) %.0f\n",s_segs,s_iss,s_rem);
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 4)
  printf("segs: (%.0v16f) (%.0v16f) %.0f\n",s_segs,s_iss,s_rem);
#endif

#endif

  //
  // Precompute cubic polynomial coefficients from transformed control
  // cage so we can shuffle them in on each iteration of the outer
  // loop and then evaluate the polynomial in Horner form.
  //
  //                            |  1  0  0  0 | | c0 |
  //                            |             | |    |
  //                            | -3  3  0  0 | | c1 |
  //   B(t) = [ 1 t^1 t^2 t^3 ] |             | |    |
  //                            |  3 -6  3  0 | | c2 |
  //                            |             | |    |
  //                            | -1  3 -3  1 | | c3 |
  //
  //
  SKC_RASTERIZE_FLOAT const b1x = mad(-3.0f,b0x,3.0f*t1x);                // 2 - 1 MAD + MUL
  SKC_RASTERIZE_FLOAT const b1y = mad(-3.0f,b0y,3.0f*t1y);                // 2 - 1 MAD + MUL

  SKC_RASTERIZE_FLOAT const b2x = mad(3.0f,b0x,mad(-6.0f,t1x,3.0f*t2x));  // 3 - 2 MAD + MUL
  SKC_RASTERIZE_FLOAT const b2y = mad(3.0f,b0y,mad(-6.0f,t1y,3.0f*t2y));  // 3 - 2 MAD + MUL

  SKC_RASTERIZE_FLOAT const b3x = mad(3.0f,t1x,mad(-3.0f,t2x,t3x)) - b0x; // 3 - 2 MAD + SUB
  SKC_RASTERIZE_FLOAT const b3y = mad(3.0f,t1y,mad(-3.0f,t2y,t3y)) - b0y; // 3 - 2 MAD + SUB

  //
  // these values don't matter on the first iteration
  //
  SKC_RASTERIZE_FLOAT l1x_prev  = 0;
  SKC_RASTERIZE_FLOAT l1y_prev  = 0;

  //
  // initialize smem bins
  //
  skc_smem_init(atomics_context,atomics_context,block_pool_ids,smem);

  //
  // loop until done
  //
  while (s_rem > 0)
    {
      //
      // distribute work across lanes
      //
      SKC_RASTERIZE_UINT const s_source = skc_scatter_scan_max(smem,s_iss,s_ess);

      //
      // every lane has a fraction to work off of
      //
      // FIXME -- this gets expanded on SIMD
      //
      // if delta == 1      then this is the first lane
      // if count == s_segs then this is the last  lane
      //
      SKC_RASTERIZE_FLOAT     const s_delta    = skc_delta_offset() - skc_sub_group_shuffle(s_ess,s_source);
      SKC_RASTERIZE_FLOAT     const s_count    = skc_sub_group_shuffle(s_segs,s_source);

      SKC_RASTERIZE_PREDICATE const is_s_first = (s_delta == 1.0f);
      SKC_RASTERIZE_PREDICATE const is_s_last  = (s_delta >= s_count);

      //
      // init parametric t
      //
      SKC_RASTERIZE_FLOAT s_t = s_delta * skc_sub_group_shuffle(s_denom,s_source); // faster than native_recip(s_count)?

      //
      // if last then override to a hard 1.0f
      //
      s_t    = is_s_last ? 1.0f : s_t;

      //
      // decrement by subgroup size
      //
      s_iss -= SKC_RASTERIZE_SUBGROUP_SEGMENT_COUNT;
      s_ess -= SKC_RASTERIZE_SUBGROUP_SEGMENT_COUNT;
      s_rem -= SKC_RASTERIZE_SUBGROUP_SEGMENT_COUNT;

      //
      // now every lane knows what to do and the following lines will
      // pump out up to SUBGROUP_SIZE line segments
      //
      // obtain the src vertices through shared or via a shuffle
      //

      //
      // shuffle in the polynomial coefficients their source lane
      //
      SKC_RASTERIZE_FLOAT const s0x = skc_sub_group_shuffle(b0x,s_source);
      SKC_RASTERIZE_FLOAT const s0y = skc_sub_group_shuffle(b0y,s_source);

      SKC_RASTERIZE_FLOAT const s1x = skc_sub_group_shuffle(b1x,s_source);
      SKC_RASTERIZE_FLOAT const s1y = skc_sub_group_shuffle(b1y,s_source);

      SKC_RASTERIZE_FLOAT const s2x = skc_sub_group_shuffle(b2x,s_source);
      SKC_RASTERIZE_FLOAT const s2y = skc_sub_group_shuffle(b2y,s_source);

      SKC_RASTERIZE_FLOAT const s3x = skc_sub_group_shuffle(b3x,s_source);
      SKC_RASTERIZE_FLOAT const s3y = skc_sub_group_shuffle(b3y,s_source);

      //
      // calculate "right" line segment endpoint using Horner form
      //
      SKC_RASTERIZE_FLOAT       l1x = round(mad(mad(mad(s3x,s_t,s2x),s_t,s1x),s_t,s0x)); // 3 MAD + ROUND
      SKC_RASTERIZE_FLOAT       l1y = round(mad(mad(mad(s3y,s_t,s2y),s_t,s1y),s_t,s0y)); // 3 MAD + ROUND

      //
      // shuffle up "left" line segment endpoint
      //
      // NOTE: Intel's shuffle_up is unique with its elegant
      // "previous" argument so don't get used to it
      //
      SKC_RASTERIZE_FLOAT       l0x = skc_sub_group_shuffle_up_1(l1x_prev,l1x);
      SKC_RASTERIZE_FLOAT       l0y = skc_sub_group_shuffle_up_1(l1y_prev,l1y);

      //
      // save previous right endpoint
      //
      l1x_prev = l1x;
      l1y_prev = l1y;

      //
      // override shuffle up if this is the first line segment
      //
      l0x = select(l0x,s0x,is_s_first);
      l0y = select(l0y,s0y,is_s_first);

      //
      // sliver lines
      //
      skc_sliver(atomics_context,
                 atomics_cohort,
                 block_pool_ids,
                 block_pool,
                 ttsk_ryx,
                 smem,
                 cmd,
                 l0x,l0y,l1x,l1y);
    }

  //
  // - flush work-in-progress blocks
  // - return unused block ids
  //
  skc_finalize(atomics_context,
               atomics_cohort,
               block_pool_ids,
               block_pool,
               ttsk_ryx,
               smem,
               cmd);
}

//
// RASTERIZE QUAD KERNEL
//

__kernel
// #ifdef SKC_ARCH_GEN9 // this ensures we get the subgroup size we wanted
__attribute__((intel_reqd_sub_group_size(SKC_RASTERIZE_SUBGROUP_SIZE)))
// #endif
__attribute__((work_group_size_hint(SKC_RASTERIZE_SUBGROUP_SIZE * SKC_RASTERIZE_WORKGROUP_COUNT_SUBGROUP, 1, 1)))
void
skc_rasterize_quad_kernel(__global ushort4             const    * restrict const cmds,
                          __global SKC_RASTERIZE_FLOAT const    * restrict const coords,
                          __global float8              const    * restrict const transforms,
                          __global SKC_ATOMICS_TYPE    volatile * restrict const atomics_context,
                          __global SKC_ATOMICS_TYPE    volatile * restrict const atomics_cohort,
                          __global uint                         * restrict const block_pool_ids,
                          __global uint                         * restrict const block_pool,
                          __global ulong                        * restrict const ttsk_ryx) // FIXME -- should be ulong
{
  //
  // declare shared memory block
  //
#if ( SKC_RASTERIZE_WORKGROUP_COUNT_SUBGROUP == 1 )
  __local struct skc_subgroup_smem volatile                  smem[1];
#else
  __local struct skc_subgroup_smem volatile                  smem_wg[SKC_RASTERIZE_WORKGROUP_COUNT_SUBGROUP];
  __local struct skc_subgroup_smem volatile * restrict const smem = smem_wg + get_sub_group_id();
#endif

  //
  // this is a subgroup/warp-centric kernel
  //
  // which subgroup in the grid is this?
  //
  // TAKE NOTE: the Intel GEN compiler appears to be recognizing
  // get_group_id(0) as a uniform but the alternative calculation used
  // when there are multiple subgroups per workgroup is not
  // cooperating and driving spillage elsewhere.
  //
#if ( SKC_RASTERIZE_WORKGROUP_COUNT_SUBGROUP == 1 )
  uint const cmd_idx = get_group_id(0);
#else
  uint const cmd_idx = get_group_id(0) * SKC_RASTERIZE_WORKGROUP_COUNT_SUBGROUP + get_sub_group_id();
#endif

  //
  // load single command for the subgroup
  //
  union skc_rasterize_cmd const cmd = { .b16v4 = cmds[cmd_idx] };

  //
  // load vertices
  //
  // The cubic coord extent is packed in slabs that are subgroup wide
  // and H float coordinates high:
  //
  //   segment | H
  //  ---------+---
  //    cubic  | 8
  //    quad   | 6
  //    line   | 4
  //
  uint const coord_idx = cmd.coord_base * (SKC_RASTERIZE_SUBGROUP_SIZE * 6) + skc_sub_group_local_id();

  SKC_RASTERIZE_FLOAT const c0x = coords[coord_idx+SKC_RASTERIZE_SUBGROUP_SIZE*0];
  SKC_RASTERIZE_FLOAT const c0y = coords[coord_idx+SKC_RASTERIZE_SUBGROUP_SIZE*1];

  SKC_RASTERIZE_FLOAT const c1x = coords[coord_idx+SKC_RASTERIZE_SUBGROUP_SIZE*2];
  SKC_RASTERIZE_FLOAT const c1y = coords[coord_idx+SKC_RASTERIZE_SUBGROUP_SIZE*3];

  SKC_RASTERIZE_FLOAT const c2x = coords[coord_idx+SKC_RASTERIZE_SUBGROUP_SIZE*4];
  SKC_RASTERIZE_FLOAT const c2y = coords[coord_idx+SKC_RASTERIZE_SUBGROUP_SIZE*5];

  //
  // load transform -- uniform across subgroup
  //
  // v8: { sx shx tx shy sy ty w0 w1 }
  //
  // NOTE THAT WE'RE SCALING UP THE TRANSFORM BY:
  //
  //   [ SKC_SUBPIXEL_RESL_X_F32, SKC_SUBPIXEL_RESL_Y_F32, 1.0f ]
  //
  // Coordinates are scaled to subpixel resolution.  All that matters
  // is that continuity is maintained between end path element
  // endpoints.
  //
  // It's the responsibility of the host to ensure that the transforms
  // are properly scaled either via intitializing a transform stack
  // with the subpixel resolution scaled identity or scaling the
  // transform before its loaded by a rasterization grid.
  //
  union skc_transform_v const tv = { .f32v8 = transforms[cmd.transform_idx] };

  //
  // apply transform
  //
  // note that we only care if the end points are rounded to subpixel precision
  //
  // FIXME -- transformation is currently affine-only support perspective later
  //
  // the affine transformation requires 8 FMA + 2 ROUND operations
  //
  SKC_RASTERIZE_FLOAT const b0x = round(c0x * tv.sx  + c0y * tv.shx + tv.tx);
  SKC_RASTERIZE_FLOAT const b0y = round(c0x * tv.shy + c0y * tv.sy  + tv.ty);

  SKC_RASTERIZE_FLOAT const t1x = c1x * tv.sx  + c1y * tv.shx + tv.tx;
  SKC_RASTERIZE_FLOAT const t1y = c1x * tv.shy + c1y * tv.sy  + tv.ty;

  SKC_RASTERIZE_FLOAT const t2x = round(c2x * tv.sx  + c2y * tv.shx + tv.tx);
  SKC_RASTERIZE_FLOAT const t2y = round(c2x * tv.shy + c2y * tv.sy  + tv.ty);

  //
  // Estimate how many line segments are in quad/cubic curve.
  //
  // Wang's Formula will return zero if the control points are
  // collinear but we bump it up to 1.0f.
  //
  SKC_RASTERIZE_FLOAT const s_segs  = skc_wangs_formula_quadratic(b0x,b0y,t1x,t1y,t2x,t2y);

  //
  // if there are free registers then precalculate the reciprocal for
  // each estimated segments since it will never change
  //
  SKC_RASTERIZE_FLOAT const s_denom = native_recip(s_segs);


  //
  // inclusive add scan of estimated line segments
  // exclusive add scan of estimated line segments
  // total number       of estimated line segments
  //
  SKC_RASTERIZE_FLOAT       s_iss   = skc_sub_group_scan_inclusive_add_float(s_segs);
  SKC_RASTERIZE_FLOAT       s_ess   = s_iss - s_segs;
  float                     s_rem   = skc_sub_group_last_float(s_iss); // scalar

  //
  // Precompute quadtracit polynomial coefficients from control cage
  // so we can shuffle them in on each iteration of the outer loop and
  // then evaluate the polynomial in Horner form.
  //

  //                        |  1  0  0  | | c0 |
  //                        |           | |    |
  //   B(t) = [ 1 t^1 t^2 ] | -2  2  0  | | c1 |
  //                        |           | |    |
  //                        |  1 -2  1  | | c2 |
  //
  //
  SKC_RASTERIZE_FLOAT const b1x = mad(-2.0f,b0x,2.0f*t1x); // 2 - 1 MAD + MUL
  SKC_RASTERIZE_FLOAT const b1y = mad(-2.0f,b0y,2.0f*t1y); // 2 - 1 MAD + MUL

  SKC_RASTERIZE_FLOAT const b2x = mad(-2.0f,t1x,b0x+t2x);  // 2 - 1 MAD + ADD
  SKC_RASTERIZE_FLOAT const b2y = mad(-2.0f,t1y,b0y+t2y);  // 2 - 1 MAD + ADD

  //
  // these values don't matter on the first iteration
  //
  SKC_RASTERIZE_FLOAT l1x_prev  = 0;
  SKC_RASTERIZE_FLOAT l1y_prev  = 0;

  //
  // initialize smem bins
  //
  skc_smem_init(atomics_context,atomics_context,block_pool_ids,smem);

  //
  // loop until done
  //
  while (s_rem > 0)
    {
      //
      // distribute work across lanes
      //
      SKC_RASTERIZE_UINT const s_source = skc_scatter_scan_max(smem,s_iss,s_ess);

      //
      // every lane has a fraction to work off of
      //
      // FIXME -- this gets expanded on SIMD
      //
      // if delta == 1      then this is the first lane
      // if count == s_segs then this is the last  lane
      //
      SKC_RASTERIZE_FLOAT     const s_delta    = skc_delta_offset() - skc_sub_group_shuffle(s_ess,s_source);
      SKC_RASTERIZE_FLOAT     const s_count    = skc_sub_group_shuffle(s_segs,s_source);

      SKC_RASTERIZE_PREDICATE const is_s_first = (s_delta == 1.0f);
      SKC_RASTERIZE_PREDICATE const is_s_last  = (s_delta >= s_count);

      //
      // init parametric t
      //
      SKC_RASTERIZE_FLOAT s_t = s_delta * skc_sub_group_shuffle(s_denom,s_source); // faster than native_recip(s_count)?

      //
      // if last then override to a hard 1.0f
      //
      s_t    = is_s_last ? 1.0f : s_t;

      //
      // decrement by subgroup size
      //
      s_iss -= SKC_RASTERIZE_SUBGROUP_SEGMENT_COUNT;
      s_ess -= SKC_RASTERIZE_SUBGROUP_SEGMENT_COUNT;
      s_rem -= SKC_RASTERIZE_SUBGROUP_SEGMENT_COUNT;

      //
      // now every lane knows what to do and the following lines will
      // pump out up to SUBGROUP_SIZE line segments
      //
      // obtain the src vertices through shared or via a shuffle
      //

      //
      // shuffle in the polynomial coefficients their source lane
      //
      SKC_RASTERIZE_FLOAT const s0x = skc_sub_group_shuffle(b0x,s_source);
      SKC_RASTERIZE_FLOAT const s0y = skc_sub_group_shuffle(b0y,s_source);

      SKC_RASTERIZE_FLOAT const s1x = skc_sub_group_shuffle(b1x,s_source);
      SKC_RASTERIZE_FLOAT const s1y = skc_sub_group_shuffle(b1y,s_source);

      SKC_RASTERIZE_FLOAT const s2x = skc_sub_group_shuffle(b2x,s_source);
      SKC_RASTERIZE_FLOAT const s2y = skc_sub_group_shuffle(b2y,s_source);

      //
      // calculate "right" line segment endpoint using Horner form
      //
      SKC_RASTERIZE_FLOAT       l1x = round(mad(mad(s2x,s_t,s1x),s_t,s0x)); // 2 MAD + ROUND
      SKC_RASTERIZE_FLOAT       l1y = round(mad(mad(s2y,s_t,s1y),s_t,s0y)); // 2 MAD + ROUND

      //
      // shuffle up "left" line segment endpoint
      //
      // NOTE: Intel's shuffle_up is unique with its elegant
      // "previous" argument so don't get used to it
      //
      SKC_RASTERIZE_FLOAT       l0x = skc_sub_group_shuffle_up_1(l1x_prev,l1x);
      SKC_RASTERIZE_FLOAT       l0y = skc_sub_group_shuffle_up_1(l1y_prev,l1y);

      //
      // save previous right endpoint
      //
      l1x_prev = l1x;
      l1y_prev = l1y;

      //
      // override shuffle up if this is the first line segment
      //
      l0x = select(l0x,s0x,is_s_first);
      l0y = select(l0y,s0y,is_s_first);

      //
      // sliver lines
      //
      skc_sliver(atomics_context,
                 atomics_cohort,
                 block_pool_ids,
                 block_pool,
                 ttsk_ryx,
                 smem,
                 cmd,
                 l0x,l0y,l1x,l1y);
    }

  //
  // - flush work-in-progress blocks
  // - return unused block ids
  //
  skc_finalize(atomics_context,
               atomics_cohort,
               block_pool_ids,
               block_pool,
               ttsk_ryx,
               smem,
               cmd);
}

//
// RASTERIZE LINE KERNEL
//

__kernel
// #ifdef SKC_ARCH_GEN9 // this ensures we get the subgroup size we wanted
__attribute__((intel_reqd_sub_group_size(SKC_RASTERIZE_SUBGROUP_SIZE)))
// #endif
__attribute__((work_group_size_hint(SKC_RASTERIZE_SUBGROUP_SIZE * SKC_RASTERIZE_WORKGROUP_COUNT_SUBGROUP, 1, 1)))
void
skc_rasterize_line_kernel(__global ushort4             const    * restrict const cmds,
                          __global SKC_RASTERIZE_FLOAT const    * restrict const coords,
                          __global float8              const    * restrict const transforms,
                          __global SKC_ATOMICS_TYPE    volatile * restrict const atomics_context,
                          __global SKC_ATOMICS_TYPE    volatile * restrict const atomics_cohort,
                          __global uint                         * restrict const block_pool_ids,
                          __global uint                         * restrict const block_pool,
                          __global ulong                        * restrict const ttsk_ryx) // FIXME -- should be ulong
{
  //
  // declare shared memory block
  //
#if ( SKC_RASTERIZE_WORKGROUP_COUNT_SUBGROUP == 1 )
  __local struct skc_subgroup_smem volatile                  smem[1];
#else
  __local struct skc_subgroup_smem volatile                  smem_wg[SKC_RASTERIZE_WORKGROUP_COUNT_SUBGROUP];
  __local struct skc_subgroup_smem volatile * restrict const smem = smem_wg + get_sub_group_id();
#endif

  //
  // this is a subgroup/warp-centric kernel
  //
  // which subgroup in the grid is this?
  //
  // TAKE NOTE: the Intel GEN compiler appears to be recognizing
  // get_group_id(0) as a uniform but the alternative calculation used
  // when there are multiple subgroups per workgroup is not
  // cooperating and driving spillage elsewhere.
  //
#if ( SKC_RASTERIZE_WORKGROUP_COUNT_SUBGROUP == 1 )
  uint const cmd_idx = get_group_id(0);
#else
  uint const cmd_idx = get_group_id(0) * SKC_RASTERIZE_WORKGROUP_COUNT_SUBGROUP + get_sub_group_id();
#endif

  //
  // load single command for the subgroup
  //
  union skc_rasterize_cmd const cmd = { .b16v4 = cmds[cmd_idx] };

  //
  // load vertices
  //
  // The cubic coord extent is packed in slabs that are subgroup wide
  // and H float coordinates high:
  //
  //   segment | H
  //  ---------+---
  //    cubic  | 8
  //    quad   | 6
  //    line   | 4
  //
  uint const coord_idx = cmd.coord_base * (SKC_RASTERIZE_SUBGROUP_SIZE * 2) + skc_sub_group_local_id();

  SKC_RASTERIZE_FLOAT const c0x = coords[coord_idx+SKC_RASTERIZE_SUBGROUP_SIZE*0];
  SKC_RASTERIZE_FLOAT const c0y = coords[coord_idx+SKC_RASTERIZE_SUBGROUP_SIZE*1];

  SKC_RASTERIZE_FLOAT const c1x = coords[coord_idx+SKC_RASTERIZE_SUBGROUP_SIZE*2];
  SKC_RASTERIZE_FLOAT const c1y = coords[coord_idx+SKC_RASTERIZE_SUBGROUP_SIZE*3];

  //
  // load transform -- uniform across subgroup
  //
  // v8: { sx shx tx shy sy ty w0 w1 }
  //
  // NOTE THAT WE'RE SCALING UP THE TRANSFORM BY:
  //
  //   [ SKC_SUBPIXEL_RESL_X_F32, SKC_SUBPIXEL_RESL_Y_F32, 1.0f ]
  //
  // Coordinates are scaled to subpixel resolution.  All that matters
  // is that continuity is maintained between end path element
  // endpoints.
  //
  // It's the responsibility of the host to ensure that the transforms
  // are properly scaled either via intitializing a transform stack
  // with the subpixel resolution scaled identity or scaling the
  // transform before its loaded by a rasterization grid.
  //
  union skc_transform_v const tv = { .f32v8 = transforms[cmd.transform_idx] };

  //
  // apply transform
  //
  // note that we only care if the end points are rounded to subpixel precision
  //
  // FIXME -- transformation is currently affine-only support perspective later
  //
  // the affine transformation requires 8 FMA + 2 ROUND operations
  //
  SKC_RASTERIZE_FLOAT const l0x = round(c0x * tv.sx  + c0y * tv.shx + tv.tx);
  SKC_RASTERIZE_FLOAT const l0y = round(c0x * tv.shy + c0y * tv.sy  + tv.ty);

  SKC_RASTERIZE_FLOAT const l1x = round(c1x * tv.sx  + c1y * tv.shx + tv.tx);
  SKC_RASTERIZE_FLOAT const l1y = round(c1x * tv.shy + c1y * tv.sy  + tv.ty);

  //
  // sliver lines
  //
  skc_sliver(atomics_context,
             atomics_cohort,
             block_pool_ids,
             block_pool,
             ttsk_ryx,
             smem,
             cmd,
             l0x,l0y,l1x,l1y);

  //
  // - flush work-in-progress blocks
  // - return unused block ids
  //
  skc_finalize(atomics_context,
               atomics_cohort,
               block_pool_ids,
               block_pool,
               ttsk_ryx,
               smem,
               cmd);
}

//
//
//

#if 0 // PRINTF_ENABLE

#if   ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 0)
      printf("scan: (%d) %u %u\n",skc_sub_group_local_id(),s_scratch,s_source);
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 1)
      printf("scan: (%#v2u) (%#v2u)\n",s_scratch,s_source);
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 2)
      printf("scan: (%#v4u) (%#v4u)\n",s_scratch,s_source);
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 3)
      printf("scan: (%#v8u) (%#v8u)\n",s_scratch,s_source);
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 4)
      printf("scan: (%#v16u) (%#v16u)\n",s_scratch,s_source);
#endif

#endif

#if 0 // PRINTF_ENABLE

#if   ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 0)
      printf("dlta: (%d) %.0f\n",skc_sub_group_local_id(),s_delta);
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 1)
      printf("dlta: (%.0v2f)\n",s_delta);
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 2)
      printf("dlta: (%.0v4f)\n",s_delta);
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 3)
      printf("dlta: (%.0v8f)\n",s_delta);
#elif ( SKC_RASTERIZE_VECTOR_SIZE_LOG2 == 4)
      printf("dlta: (%.0v16f)\n",s_delta);
#endif

#endif

//
//
//
