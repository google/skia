/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
//
//

#include "tile.h"
#include "block.h"
#include "styling_types.h"
#include "atomic_cl.h"
#include "kernel_cl_12.h"

//
//
//

#define SKC_RENDER_SUBGROUP_MASK  (SKC_RENDER_SUBGROUP_SIZE - 1)

//
//
//

#if   ( SKC_RENDER_SCANLINE_VECTOR_SIZE == 1 )
#define SKC_RENDER_SCANLINE_VECTOR_EXPAND()           SKC_EXPAND_1()
#define SKC_RENDER_SCANLINE_VECTOR_EXPAND_I_LAST      0

#elif ( SKC_RENDER_SCANLINE_VECTOR_SIZE == 2 )
#define SKC_RENDER_SCANLINE_VECTOR_EXPAND()           SKC_EXPAND_2()
#define SKC_RENDER_SCANLINE_VECTOR_EXPAND_I_LAST      1

#elif ( SKC_RENDER_SCANLINE_VECTOR_SIZE == 4 )
#define SKC_RENDER_SCANLINE_VECTOR_EXPAND()           SKC_EXPAND_4()
#define SKC_RENDER_SCANLINE_VECTOR_EXPAND_I_LAST      3

#elif ( SKC_RENDER_SCANLINE_VECTOR_SIZE == 8 )
#define SKC_RENDER_SCANLINE_VECTOR_EXPAND()           SKC_EXPAND_8()
#define SKC_RENDER_SCANLINE_VECTOR_EXPAND_I_LAST      7

#elif ( SKC_RENDER_SCANLINE_VECTOR_SIZE == 16)
#define SKC_RENDER_SCANLINE_VECTOR_EXPAND()           SKC_EXPAND_16()
#define SKC_RENDER_SCANLINE_VECTOR_EXPAND_I_LAST      15
#endif

//
// tile state flag bits
//

typedef enum skc_tile_flags_e {

  // FLUSH
  SKC_TILE_FLAGS_FLUSH_FINALIZE    = 0x00000001,
  SKC_TILE_FLAGS_FLUSH_UNWIND      = 0x00000002,
  SKC_TILE_FLAGS_FLUSH_COMPLETE    = 0x00000004,

  // OPACITY
  SKC_TILE_FLAGS_SCATTER_SKIP      = 0x00000008,

  //
  // Note: testing for opacity and skipping scattering is on its way
  // to becoming a much more programmable option because sometimes we
  // may be compositing/blending from back-to-front and/or be using
  // group blend rules that ignore opacity.
  //
  // The point is that all of these decisions should be encoded in
  // styling commands and, as much as possible, removed from the final
  // group/layer styling traversal render loop.
  //

} skc_tile_flags_e;

//
// COVER -- assumes availability of either fp16 or fp32
//

union skc_tile_cover
{
  struct {
    SKC_RENDER_TILE_COVER             c[SKC_TILE_WIDTH];
  } aN;

#ifdef SKC_RENDER_TILE_COVER_VECTOR
  struct {
    SKC_RENDER_TILE_COVER_VECTOR      c[SKC_RENDER_TILE_COVER_VECTOR_COUNT];
  } vN;
#endif
};

//
// COLOR -- assumes availability of either fp16 or fp32
//

union skc_tile_color
{
  union {
    struct {
      SKC_RENDER_TILE_COLOR           r;
      SKC_RENDER_TILE_COLOR           g;
      SKC_RENDER_TILE_COLOR           b;
      SKC_RENDER_TILE_COLOR           a;
    } rgba[SKC_TILE_WIDTH];
  } aN;

#ifdef SKC_RENDER_TILE_COLOR_INTERLEAVED
  union {
    SKC_RENDER_TILE_COLOR_INTERLEAVED rgba[SKC_TILE_WIDTH];
  } iN;
#endif

#ifdef SKC_RENDER_TILE_COLOR_VECTOR
  union {
    SKC_RENDER_TILE_COLOR_VECTOR      rgba[SKC_RENDER_TILE_COLOR_VECTOR_COUNT];
  } vN;
#endif

  struct {
    union {
      struct {
        SKC_RENDER_TILE_COLOR         r;
        SKC_RENDER_TILE_COLOR         g;
      };
      SKC_RENDER_GRADIENT_FLOAT       distance;
    };
    union {
      struct {
        SKC_RENDER_TILE_COLOR         b;
        SKC_RENDER_TILE_COLOR         a;
      };
      SKC_RENDER_GRADIENT_FLOAT       stoplerp;
    };
  } grad[SKC_TILE_WIDTH];
};

//
// SHARED MEMORY STATE
//

#define SKC_RENDER_TILE_SMEM_WORDS ((SKC_TILE_WIDTH + 1) * SKC_TILE_HEIGHT)

#define SKC_RENDER_WIDE_AA_BYTES   (SKC_RENDER_TILE_SMEM_WORDS * sizeof(int) / SKC_RENDER_SUBGROUP_SIZE)
#define SKC_RENDER_WIDE_AA_WIDTH   (SKC_RENDER_WIDE_AA_BYTES / sizeof(SKC_RENDER_WIDE_AA))

//
//
//

union skc_subgroup_smem
{
  //
  // The tiles are stored in column-major / height-major order
  //
  // The final column is a guard column that is OK to write to but
  // will never be read.  It simplifies the TTSB scatter but could be
  // predicated if SMEM is really at a premium.
  //
#if ( SKC_RENDER_SUBGROUP_SIZE > 1 )
  struct {
    SKC_ATOMIC_UINT              area[SKC_RENDER_TILE_SMEM_WORDS]; // area[w][h]
  } atomic;
#endif

  struct {
    int                          area[SKC_RENDER_TILE_SMEM_WORDS]; // area[w][h]
  } aN;

  struct { // assumption is that height = subgroup
    SKC_RENDER_AREA_V            area[SKC_TILE_WIDTH + 1][SKC_RENDER_SUBGROUP_SIZE];
  } vN;

  struct { // assumption is that height = subgroup
    SKC_RENDER_WIDE_AA           area[SKC_RENDER_WIDE_AA_WIDTH][SKC_RENDER_SUBGROUP_SIZE];
  } wide;

  union skc_styling_cmd          cmds[(SKC_TILE_WIDTH + 1) * SKC_TILE_HEIGHT];

  half                           gc  [(SKC_TILE_WIDTH + 1) * SKC_TILE_HEIGHT * 2];

#if 0
  //
  // SPILL TO GMEM
  //
#if (SKC_REGS_COLOR_S > 0) || (SKC_REGS_COVER_S > 0)
  struct {

#if (SKC_REGS_COLOR_S > 0)
    union skc_color_r            color[SKC_REGS_COLOR_S][SKC_TILE_HEIGHT][SKC_TILE_WIDTH];
#endif

#if (SKC_REGS_COVER_S > 0)
    union float                  cover[SKC_REGS_COVER_S][SKC_TILE_HEIGHT][SKC_TILE_WIDTH];
#endif

  } regs;
#endif
  //
  //
  //
#endif
};

//
//
//

#if ( SKC_RENDER_SUBGROUP_SIZE == 1 )

#define skc_subgroup_lane()  0

#else

#define skc_subgroup_lane()  get_sub_group_local_id()

#endif

//
//
//

typedef skc_uint  skc_ttsk_lo_t;
typedef skc_uint  skc_ttsk_hi_t;

typedef skc_uint  skc_ttpk_lo_t;
typedef skc_uint  skc_ttpk_hi_t;

typedef skc_uint  skc_ttxk_lo_t;
typedef skc_uint  skc_ttxk_hi_t;

typedef skc_uint  skc_ttck_lo_t;
typedef skc_uint  skc_ttck_hi_t;

typedef skc_uint2 skc_ttck_t;

typedef skc_int   skc_ttxb_t;

//
// TTCK (32-BIT COMPARE) v1:
//
//  0                                                           63
//  | PAYLOAD/TTSB/TTPB ID | PREFIX | ESCAPE | LAYER |  X  |  Y  |
//  +----------------------+--------+--------+-------+-----+-----+
//  |          30          |    1   |    1   |   18  |  7  |  7  |
//
//
// TTCK (32-BIT COMPARE) v2:
//
//  0                                                           63
//  | PAYLOAD/TTSB/TTPB ID | PREFIX | ESCAPE | LAYER |  X  |  Y  |
//  +----------------------+--------+--------+-------+-----+-----+
//  |          30          |    1   |    1   |   15  |  9  |  8  |
//
//
// TTCK (64-BIT COMPARE) -- achieves 4K x 4K with an 8x16 tile:
//
//  0                                                           63
//  | PAYLOAD/TTSB/TTPB ID | PREFIX | ESCAPE | LAYER |  X  |  Y  |
//  +----------------------+--------+--------+-------+-----+-----+
//  |          27          |    1   |    1   |   18  |  9  |  8  |
//

static
skc_uint
skc_ttck_lo_get_ttxb_id(skc_ttck_lo_t const a)
{
  return a & SKC_TTCK_LO_MASK_ID;
}

static
skc_layer_id
skc_ttck_get_layer(skc_ttck_t const a)
{
  //
  // FIXME -- a union with a ulong and a shift down and mask is
  // probably faster on some architectures
  //
  skc_uint const lo = (a.lo >> SKC_TTCK_LO_BITS_ID_PREFIX_ESCAPE);
  skc_uint const hi = (a.hi  & SKC_TTCK_HI_MASK_LAYER) << SKC_TTCK_LO_BITS_LAYER;

  return lo | hi;
}

static
skc_uint
skc_ttck_hi_get_x(skc_ttck_hi_t const a)
{
  return SKC_BFE(a,SKC_TTCK_HI_BITS_X,SKC_TTCK_HI_OFFSET_X);
}

static
skc_uint
skc_ttck_hi_get_y(skc_ttck_hi_t const a)
{
  return a >> SKC_TTCK_HI_OFFSET_Y;
}

static
skc_bool
skc_ttck_equal_yxl(skc_ttck_t const a, skc_ttck_t const b)
{
  skc_uint const lo = (a.lo ^ b.lo) & SKC_BITS_TO_MASK_AT(SKC_TTCK_LO_BITS_LAYER,SKC_TTCK_LO_BITS_ID_PREFIX_ESCAPE);
  skc_uint const hi = (a.hi ^ b.hi);

  return (lo | hi) == 0;
}

static
skc_bool
skc_ttck_hi_equal_yx(skc_ttck_hi_t const a, skc_ttck_hi_t const b)
{
  return ((a ^ b) & SKC_TTCK_HI_MASK_YX) == 0;
}

static
skc_bool
skc_ttck_lo_is_prefix(skc_ttck_lo_t const a)
{
  return (a & SKC_TTCK_LO_MASK_PREFIX) != 0;
}

//
// TILE TRACE SUBPIXEL
//
// The subpixels are encoded with either absolute tile coordinates
// (32-bits) or packed in delta-encoded form form.
//
// For 32-bit subpixel packing of a 32x32 tile:
//
// A tile X is encoded as:
//
//   TX : 10 : unsigned min(x0,x1) tile subpixel coordinate.
//
//   SX :  6 : unsigned subpixel span from min to max x with range
//             [0,32]. The original direction is not captured. Would
//             be nice to capture dx but not necessary right now but
//             could be in the future. <--- SPARE VALUES AVAILABLE
//
// A tile Y is encoded as:
//
//   TY : 10 : unsigned min(y0,y1) tile subpixel coordinate.
//
//   DY :  6 : signed subpixel delta y1-y0. The range of delta is
//             [-32,32] but horizontal lines are not encoded so [1,32]
//             is mapped to [0,31]. The resulting range [-32,31] fits
//             in 6 bits.
//
// TTS:
//
//  0                        31
//  |  TX |  SX  |  TY |  DY  |
//  +-----+------+-----+------+
//  |  10 |   6  |  10 |   6  |
//

static
SKC_RENDER_TTS_V_BITFIELD
skc_tts_get_ty_pixel_v(SKC_RENDER_TTS_V const a)
{
  //
  // extract the whole pixel y coordinate
  //
  return SKC_BFE(a,
                 SKC_TTS_BITS_TY   - SKC_SUBPIXEL_RESL_Y_LOG2,
                 SKC_TTS_OFFSET_TY + SKC_SUBPIXEL_RESL_Y_LOG2);
}

static
SKC_RENDER_TTS_V_BITFIELD
skc_tts_get_xy_idx_v(SKC_RENDER_TTS_V const a)
{
  //
  // get the linear array tile index of the pixel
  //
  return (((a & SKC_TTS_MASK_TX_PIXEL)

#if   (SKC_SUBPIXEL_RESL_X_LOG2 > SKC_TILE_HEIGHT_LOG2)
           >> (SKC_SUBPIXEL_RESL_X_LOG2 - SKC_TILE_HEIGHT_LOG2)
#elif (SKC_SUBPIXEL_RESL_X_LOG2 < SKC_TILE_HEIGHT_LOG2)
           << (SKC_TILE_HEIGHT_LOG2     - SKC_SUBPIXEL_RESL_X_LOG2)
#endif

           ) | skc_tts_get_ty_pixel_v(a));
}

#if 0
static
skc_ttx_v_s32_t
skc_tts_get_dy_v(SKC_RENDER_TTS_V const a)
{
  skc_ttx_v_s32_t const dy = SKC_AS(skc_ttx_v_s32_t)a >> SKC_TTS_OFFSET_DY;

  return (dy + SKC_AS(skc_ttx_v_s32_t)(~a >> 31));
}
#else
static
SKC_RENDER_TTS_V_BITFIELD
skc_tts_get_dy_v(SKC_RENDER_TTS_V const a)
{
  SKC_RENDER_TTS_V_BITFIELD const dy = a >> SKC_TTS_OFFSET_DY;

  return dy - (~a >> 31);
}
#endif

static
SKC_RENDER_TTS_V_BITFIELD
skc_tts_get_tx_subpixel_v(SKC_RENDER_TTS_V const a)
{
  return a & SKC_BITS_TO_MASK(SKC_SUBPIXEL_RESL_X_LOG2);
}

static
SKC_RENDER_TTS_V_BITFIELD
skc_tts_get_sx_v(SKC_RENDER_TTS_V const a)
{
  return SKC_BFE(a,SKC_TTS_BITS_SX,SKC_TTS_OFFSET_SX);
}

//
//
//

static
void
skc_tile_aa_zero(__local union skc_subgroup_smem * SKC_RESTRICT const smem)
{
  //
  // SIMD / CPU
  //
  //      &
  //
  // SIMT / GPU
  //
  // Note that atomic_init() is likely implemented as a simple
  // assignment so there is no identifiable performance difference on
  // current targets.
  //
  // If such an architecture appears in the future then we'll probably
  // still want to implement this zero'ing operation as below but
  // follow with an appropriate fence that occurs before any scatter
  // operations.
  //
  // The baroque expansion below improves performance on Intel GEN by,
  // presumably, achieving the 64-byte per clock SLM write as well as
  // minimizing the overall number of SEND() block initializations and
  // launches.
  //
  // Intel GENx has a documented 64 byte per cycle SLM write limit.
  // So having each lane in an 8 lane subgroup zero-write 8 bytes is
  // probably a safe bet (Later: benchmarking backs this up!).
  //
  // Note there is no reason at this time to unroll this loop.
  //
  for (uint ii=0; ii<SKC_RENDER_WIDE_AA_WIDTH; ii++)
    smem->wide.area[ii][skc_subgroup_lane()] = ( 0 );
}

//
// Note this is going to be vectorizable on most architectures.
//
// The return of the key translation feature might complicate things.
//

static
void
skc_scatter_ttpb(__global skc_ttxb_t        const * SKC_RESTRICT const ttxb_extent,
                 __local  union skc_subgroup_smem * SKC_RESTRICT const smem,
                 skc_block_id_t                                  const pb_id)
{
  skc_uint const offset = pb_id * (SKC_DEVICE_SUBBLOCK_WORDS / SKC_TILE_RATIO) + skc_subgroup_lane();

#if   ( SKC_TILE_RATIO == 1 )

  SKC_RENDER_TTP_V const ttp_v = ttxb_extent[offset];

#elif ( SKC_TILE_RATIO == 2 )

  SKC_RENDER_TTP_V const ttp_v = vload2(offset,ttxb_extent);

#else

#error("tile ratio greater than 2 not supported")

#endif

  //
  // Note there is no need to use an atomic for this operation on the
  // current group of target platforms... but this may change if
  // atomic ops truly go through a different path.
  //
  // As noted above, this direct increment is probably faster and can
  // always be followed by a fence.
  //
  // Furthermore, note that the key sorting orders all ttck keys
  // before ttpk keys.
  //

  //
  // FIXME -- if the SMEM store is wider than bank word count then we
  // might want to odd-even interleave the TTP values if the target
  // device can't handle 64-bit stores
  //

  //
  // skipping per-key translation for now
  //
  smem->vN.area[0][skc_subgroup_lane()] += ttp_v << (SKC_SUBPIXEL_RESL_X_LOG2 + 1);
}

//
// Note that skc_scatter_ttsb is *not* vectorizable unless the
// architecture supports a "scatter-add" capability.  All relevant
// GPUs support atomic add on shared/local memory and thus support
// scatter-add.
//

static
void
skc_scatter_ttsb(__global skc_ttxb_t        const * SKC_RESTRICT const ttxb_extent,
                 __local  union skc_subgroup_smem * SKC_RESTRICT const smem,
                 skc_block_id_t                                  const sb_id)
{
  skc_uint         const offset = sb_id * SKC_DEVICE_SUBBLOCK_WORDS + skc_subgroup_lane();

  SKC_RENDER_TTS_V const tts_v  = ttxb_extent[offset];

  //
  // Skipping per-key translation for now
  //

  // Index into tile
  //
  // The tiles are stored in column-major / height-major order
  //
  // The final column is a guard column that is OK to write to but
  // will never be read.  It simplifies the TTSB scatter but could be
  // predicated if SMEM is really at a premium.
  //

  SKC_RENDER_TTS_V_BITFIELD const xy_idx = skc_tts_get_xy_idx_v(tts_v);

#if 0
  if (tts_v != SKC_TTS_INVALID)
    printf("(%08X) = %u\n",tts_v,xy_idx);
#endif

  //
  // adjust subpixel range to max y
  //
  // range is stored as [-32,31] and when read [0,31] is mapped to
  // [1,32] because a dy of 0 is not possible.
  //
  // more succinctly: if dy >= 0 then ++dy
  //
  SKC_RENDER_TTS_V_BITFIELD const dy     = skc_tts_get_dy_v(tts_v);

  //
  // FIXME -- benchmark performance of setting dy to 0 if ttsv.vN is invalid?
  //

  // this "min(x0) * 2 + dx" is equivalent to "x0 + x1"
  SKC_RENDER_TTS_V_BITFIELD const widths = skc_tts_get_tx_subpixel_v(tts_v) * 2 + skc_tts_get_sx_v(tts_v);

  // Calculate left and right coverage contribution trapezoids
  SKC_RENDER_TTS_V_BITFIELD const left   = dy * widths;
  SKC_RENDER_TTS_V_BITFIELD const right  = (dy << (SKC_SUBPIXEL_RESL_X_LOG2 + 1)) - left;

  //
  // Accumulate altitudes and areas
  //
  // Optimization: if the device supports an CPU/SIMD vector-add or
  // GPU/SIMT scatter-add atomic int2 add operation then placing the
  // ALT and AREA values side-by-side would halve the number of
  // additions.
  //
#if ( SKC_RENDER_SUBGROUP_SIZE == 1 )
  //
  // CPU/SIMD
  //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,A)                                 \
  if (tts_v C != SKC_TTS_INVALID) {                             \
    smem->aN.area[SKC_TILE_HEIGHT + xy_idx C] += left  C;       \
    smem->aN.area[                  xy_idx C] += right C;       \
  }

#else
  //
  // GPU/SIMT -- IMPLIES SUPPORT FOR ATOMIC SCATTER-ADD
  //
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,A)                                         \
  if (tts_v C != SKC_TTS_INVALID) {                                     \
    SKC_ATOMIC_ADD_LOCAL_RELAXED_SUBGROUP(smem->atomic.area +           \
                                          SKC_TILE_HEIGHT   + xy_idx C, \
                                          left C);                      \
    SKC_ATOMIC_ADD_LOCAL_RELAXED_SUBGROUP(smem->atomic.area + xy_idx C, \
                                          right C);                     \
  }
#endif

  SKC_RENDER_TTSB_EXPAND();
}

//
// Note that 2048.0 can be represented exactly with fp16... fortuitous!
//

#define SKC_RENDER_FILL_MAX_AREA          (2u * SKC_SUBPIXEL_RESL_X * SKC_SUBPIXEL_RESL_Y)
#define SKC_RENDER_FILL_MAX_AREA_2        (2u * SKC_RENDER_FILL_MAX_AREA)
#define SKC_RENDER_FILL_EVEN_ODD_MASK     (SKC_RENDER_FILL_MAX_AREA_2 - 1)
#define SKC_RENDER_FILL_MAX_AREA_RCP_F32  (SKC_RENDER_TILE_COVER)(1.0f / SKC_RENDER_FILL_MAX_AREA)

//
//
//

static
void
skc_tile_cover_nonzero(__local union skc_subgroup_smem * SKC_RESTRICT const smem,
                       union skc_tile_cover            * SKC_RESTRICT const cover,
                       union skc_tile_color            * SKC_RESTRICT const color)
{
  SKC_RENDER_ACC_COVER_INT area = 0;

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH))) // doesn't help on AVX2
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    {
      area                                   += smem->vN.area[ii][skc_subgroup_lane()];
      SKC_RENDER_ACC_COVER_UINT const trapabs = abs(area);
      SKC_RENDER_TILE_COVER     const nonzero = SKC_CONVERT(SKC_RENDER_TILE_COVER)(min(trapabs,SKC_RENDER_FILL_MAX_AREA));

      cover->aN.c[ii] = nonzero * (SKC_RENDER_TILE_COVER)(SKC_RENDER_FILL_MAX_AREA_RCP_F32);
    }
}

static
void
skc_tile_cover_evenodd(__local union skc_subgroup_smem * SKC_RESTRICT const smem,
                       union skc_tile_cover            * SKC_RESTRICT const cover,
                       union skc_tile_color            * SKC_RESTRICT const color)
{
  SKC_RENDER_ACC_COVER_INT area = 0;

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH))) // doesn't help on AVX2
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    {
      area                                   += smem->vN.area[ii][skc_subgroup_lane()];
      SKC_RENDER_ACC_COVER_UINT const trapabs = abs(area);
      SKC_RENDER_ACC_COVER_UINT const reflect = abs(SKC_AS(SKC_RENDER_ACC_COVER_INT)((trapabs & SKC_RENDER_FILL_EVEN_ODD_MASK) - SKC_RENDER_FILL_MAX_AREA));

      cover->aN.c[ii] = SKC_CONVERT(SKC_RENDER_TILE_COVER)(SKC_RENDER_FILL_MAX_AREA - reflect) * (SKC_RENDER_TILE_COVER)SKC_RENDER_FILL_MAX_AREA_RCP_F32;
    }
}

//
//
//

static
void
skc_tile_color_fill_solid(__global union skc_styling_cmd const * SKC_RESTRICT const commands,
                          uint                                 * SKC_RESTRICT const cmd_next,
                          union skc_tile_color                 * SKC_RESTRICT const color)
{
  //
  // rgba = solid fill
  //
  __global half const * const rgba_ptr = commands[*cmd_next].f16a2 + 0;

  *cmd_next += 2;

#if !defined( SKC_RENDER_TILE_COLOR_VECTOR )

  SKC_RENDER_TILE_COLOR_PAIR const rg = SKC_RENDER_TILE_COLOR_PAIR_LOAD(0,rgba_ptr);

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH-1)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    color->aN.rgba[ii].r = rg.lo;

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH-1)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    color->aN.rgba[ii].g = rg.hi;

  SKC_RENDER_TILE_COLOR_PAIR const ba = SKC_RENDER_TILE_COLOR_PAIR_LOAD(1,rgba_ptr);

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH-1)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    color->aN.rgba[ii].b = ba.lo;

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH-1)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    color->aN.rgba[ii].a = ba.hi;

#else

  SKC_RENDER_TILE_COLOR_PAIR const rg = SKC_RENDER_TILE_COLOR_PAIR_LOAD(0,rgba_ptr);
  SKC_RENDER_TILE_COLOR      const r  = rg.lo;

  // __attribute__((opencl_unroll_hint(SKC_RENDER_TILE_COLOR_VECTOR_COUNT)))
  for (uint ii=0; ii<SKC_RENDER_TILE_COLOR_VECTOR_COUNT; ii++)
    color->vN.rgba[ii].even.even = SKC_AS(SKC_RENDER_TILE_COLOR_VECTOR_COMPONENT)(r);

  SKC_RENDER_TILE_COLOR      const g  = rg.hi;

  // __attribute__((opencl_unroll_hint(SKC_RENDER_TILE_COLOR_VECTOR_COUNT)))
  for (uint ii=0; ii<SKC_RENDER_TILE_COLOR_VECTOR_COUNT; ii++)
    color->vN.rgba[ii].odd.even  = SKC_AS(SKC_RENDER_TILE_COLOR_VECTOR_COMPONENT)(g);

  SKC_RENDER_TILE_COLOR_PAIR const ba = SKC_RENDER_TILE_COLOR_PAIR_LOAD(1,rgba_ptr);
  SKC_RENDER_TILE_COLOR      const b  = ba.lo;

  // __attribute__((opencl_unroll_hint(SKC_RENDER_TILE_COLOR_VECTOR_COUNT)))
  for (uint ii=0; ii<SKC_RENDER_TILE_COLOR_VECTOR_COUNT; ii++)
    color->vN.rgba[ii].even.odd  = SKC_AS(SKC_RENDER_TILE_COLOR_VECTOR_COMPONENT)(b);

  SKC_RENDER_TILE_COLOR      const a  = ba.hi;

  // __attribute__((opencl_unroll_hint(SKC_RENDER_TILE_COLOR_VECTOR_COUNT)))
  for (uint ii=0; ii<SKC_RENDER_TILE_COLOR_VECTOR_COUNT; ii++)
    color->vN.rgba[ii].odd.odd   = SKC_AS(SKC_RENDER_TILE_COLOR_VECTOR_COMPONENT)(a);

#endif
}

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
// CPUs have a mock local address space so copying the gradient header
// is probably not useful.  Just read directly from global.
//

#ifndef SKC_RENDER_GRADIENT_IS_GLOBAL
#define SKC_RENDER_GRADIENT_SPACE  __local
#else
#define SKC_RENDER_GRADIENT_SPACE  __global
#endif

//
// gradient is non-vertical
//
// removed the vertical (actually, horizontal) special case
//

static
void
skc_tile_color_fill_gradient_linear_nonvertical(__local  union skc_subgroup_smem     * SKC_RESTRICT const smem,
                                                __global union skc_styling_cmd const * SKC_RESTRICT const commands,
                                                uint                                 * SKC_RESTRICT const cmd_next,
                                                union skc_tile_color                 * SKC_RESTRICT const color,
                                                skc_ttck_hi_t                                       const ttck_hi)
{
  //
  // Where is this tile?
  //
  // Note that the gradient is being sampled from pixel centers.
  //
  SKC_RENDER_GRADIENT_FLOAT const y =
#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,A) I##.5f P
    (SKC_RENDER_GRADIENT_FLOAT)( SKC_RENDER_SCANLINE_VECTOR_EXPAND() ) +
    (skc_ttck_hi_get_y(ttck_hi) * SKC_TILE_HEIGHT + (skc_subgroup_lane() * SKC_RENDER_SCANLINE_VECTOR_SIZE));

  float                     const x = 0.5f + (skc_ttck_hi_get_x(ttck_hi) * SKC_TILE_WIDTH);

  //
  // Get starting numerator and denominator
  //
  // Note: if gh[0].dx is exactly 0.0f then this is a vertical
  // gradient and can be handled by a special opcode.
  //
  // Note: the mad() ordering is slightly different than the original
  // CUDA implementation.
  //
  union skc_gradient_vector const gv       = { vload4(0,&commands[*cmd_next].f32) };

  *cmd_next += 4;

  float                     const gv_x_dot = mad(x,gv.dx,gv.p0);
  SKC_RENDER_GRADIENT_FLOAT const gv_numer = mad(y,gv.dy,gv_x_dot);

  //
  // Where are columns along gradient vector?
  //
  // TODO: Note that the gv_denom isn't multiplied through.
  //
  // Please doublecheck this... but I recall that in certain cases
  // this wipes out some precision and results in minor but noticeable
  // gradient artifacts.
  //
  // All arguments are scalars except gv_numer so a simpler
  // evaluation might save some flops.
  //

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    color->grad[ii].distance = mad(gv.dx,(float)ii,gv_numer) * gv.denom;

  //
  // is gradient non-repeating, repeating or reflecting?
  //
  switch (commands[(*cmd_next)++].u32)
    {
    case SKC_STYLING_GRADIENT_TYPE_LINEAR_NON_REPEATING:
      // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
      for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
        color->grad[ii].distance = clamp(color->grad[ii].distance,0.0f,1.0f);
      break;

    case SKC_STYLING_GRADIENT_TYPE_LINEAR_REPEATING:
      // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
      for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
        color->grad[ii].distance -= floor(color->grad[ii].distance);
      break;

    default: // PXL_STYLING_GRADIENT_TYPE_LINEAR_REFLECTING
      //
      // OPTIMIZATION: Can this be done in fewer than ~4 ops?
      //
      // Note: OpenCL "rint()" is round-to-nearest-even integer!
      //
      // Note: the floor() "round to -inf" op is implemented in the
      // GEN op 'FRC' so probably don't use trunc() when floor will
      // suffice.
      //

      // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
      for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
        {
          SKC_RENDER_GRADIENT_FLOAT dist_abs = fabs(color->grad[ii].distance);
          color->grad[ii].distance = fabs(dist_abs - rint(dist_abs));
        }
    }

  //
  // initialize "stoplerp" for all columns
  //
  uint const slope_count = commands[(*cmd_next)++].u32;
  uint const gd_n_v1     = commands[(*cmd_next)++].u32; // REMOVE ME

  {
    float const slope = commands[(*cmd_next)++].f32;

    // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
    for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
      color->grad[ii].stoplerp = color->grad[ii].distance * slope;
  }

  //
  // compute stoplerp for remaining stops
  //
  for (int jj=1; jj<slope_count; jj++)
    {
      float const floor = (float)jj;
      float const slope = commands[(*cmd_next)++].f32;

      // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
      for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
        color->grad[ii].stoplerp = mad(min(0, color->grad[ii].stoplerp - floor),slope,color->grad[ii].stoplerp);
    }

  //
  // copy gradient colors to local memory
  //
  uint const gd_n = slope_count + 1;

#ifndef SKC_RENDER_GRADIENT_IS_GLOBAL
  //
  // copy entire gradient descriptor to local memory
  //
  for (uint ii=skc_subgroup_lane(); ii<gd_n*4; ii+=SKC_RENDER_SUBGROUP_SIZE)
    smem->cmds[ii].u32 = commands[*cmd_next + ii].u32;

  __local  half const * const SKC_RESTRICT gc = smem->gc + 0;
#else
  //
  // prefetch entire gradient header
  //
  // no noticeable impact on performance
  //
  // prefetch(&commands[*cmd_next].u32,gh_words);
  //
  __global half const * const SKC_RESTRICT gc = commands[*cmd_next].f16a2 + 0;
#endif

  //
  // adjust cmd_next so that V1 structure is consumed -- FIXME
  //
  *cmd_next += SKC_GRADIENT_CMD_WORDS_V2_ADJUST(gd_n_v1,gd_n);

  //
  // lerp between color pair stops
  //
  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    {
      //
      // Finally, we have the gradient stop index and the color stop
      // pair lerp fraction
      //
      // Note that if these are vector values then a gather operation
      // must occur -- there may be platforms (AVX-512?) that can
      // perform an explicit gather on a vector type but it's not
      // really expressible in OpenCL except implicitly with a
      // workgroup of work items.
      //
      // ***********************
      //
      // FIXME -- USE HERB'S SINGLE FMA LERP
      //
      // ***********************
      //
      SKC_RENDER_GRADIENT_STOP const gc_stop = SKC_CONVERT(SKC_RENDER_GRADIENT_STOP)(color->grad[ii].stoplerp);
      SKC_RENDER_GRADIENT_FRAC const gc_frac = SKC_CONVERT(SKC_RENDER_GRADIENT_FRAC)(color->grad[ii].stoplerp - floor(color->grad[ii].stoplerp));

      {
        SKC_RENDER_TILE_COLOR lo, hi;

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,A) {                                       \
          SKC_RENDER_TILE_COLOR_PAIR const cc = SKC_RENDER_TILE_COLOR_PAIR_LOAD(gc_stop C + 0,gc); \
          lo C                                = cc.lo;                  \
          hi C                                = cc.hi;                  \
        }

        SKC_RENDER_SCANLINE_VECTOR_EXPAND();

        color->aN.rgba[ii].r = SKC_LERP(lo,hi,gc_frac);
      }

      //
      //
      //
      {
        SKC_RENDER_TILE_COLOR lo, hi;

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,A) {                                       \
          SKC_RENDER_TILE_COLOR_PAIR const cc = SKC_RENDER_TILE_COLOR_PAIR_LOAD(gc_stop C + gd_n,gc); \
          lo C                                = cc.lo;                  \
          hi C                                = cc.hi;                  \
        }

        SKC_RENDER_SCANLINE_VECTOR_EXPAND();

        color->aN.rgba[ii].g = SKC_LERP(lo,hi,gc_frac);
      }

      //
      //
      //
      {
        SKC_RENDER_TILE_COLOR lo, hi;

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,A) {                                       \
          SKC_RENDER_TILE_COLOR_PAIR const cc = SKC_RENDER_TILE_COLOR_PAIR_LOAD(gc_stop C + gd_n*2,gc); \
          lo C                                = cc.lo;                  \
          hi C                                = cc.hi;                  \
        }

        SKC_RENDER_SCANLINE_VECTOR_EXPAND();

        color->aN.rgba[ii].b = SKC_LERP(lo,hi,gc_frac);
      }

      //
      //
      //
      {
        SKC_RENDER_TILE_COLOR lo, hi;

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,A) {                                       \
          SKC_RENDER_TILE_COLOR_PAIR const cc = SKC_RENDER_TILE_COLOR_PAIR_LOAD(gc_stop C + gd_n*3,gc); \
          lo C                                = cc.lo;                  \
          hi C                                = cc.hi;                  \
        }

        SKC_RENDER_SCANLINE_VECTOR_EXPAND();

        color->aN.rgba[ii].a = SKC_LERP(lo,hi,gc_frac);
      }
    }
}

//
//
//

static
void
skc_tile_blend_over(union skc_tile_color       * SKC_RESTRICT const color_acc,
                    union skc_tile_cover const * SKC_RESTRICT const cover_wip,
                    union skc_tile_color const * SKC_RESTRICT const color_wip)
{
  //
  // fralunco = cover.wip * acc.a
  //
  // acc.r    =  fralunco * wip.r + acc.r
  // acc.g    =  fralunco * wip.g + acc.g
  // acc.b    =  fralunco * wip.b + acc.b
  // acc.a    = -fralunco * wip.a + acc.a
  //

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    {
      SKC_RENDER_TILE_COVER const fralunco = cover_wip->aN.c[ii] * color_acc->aN.rgba[ii].a;

      color_acc->aN.rgba[ii].r = mad(+fralunco,color_wip->aN.rgba[ii].r,color_acc->aN.rgba[ii].r);
      color_acc->aN.rgba[ii].g = mad(+fralunco,color_wip->aN.rgba[ii].g,color_acc->aN.rgba[ii].g);
      color_acc->aN.rgba[ii].b = mad(+fralunco,color_wip->aN.rgba[ii].b,color_acc->aN.rgba[ii].b);
      color_acc->aN.rgba[ii].a = mad(-fralunco,color_wip->aN.rgba[ii].a,color_acc->aN.rgba[ii].a);
    }
}

//
//
//

static
void
skc_tile_blend_plus(union skc_tile_color       * SKC_RESTRICT const color_acc,
                    union skc_tile_cover const * SKC_RESTRICT const cover_wip,
                    union skc_tile_color const * SKC_RESTRICT const color_wip)
{
  //
  // cover_min = min(cover.wip,a.acc)
  //
  // r.acc =  cover_min * r.wip + r.acc
  // g.acc =  cover_min * g.wip + g.acc
  // b.acc =  cover_min * b.wip + b.acc
  // a.acc = -cover_min * a.wip + a.acc
  //

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    {
      SKC_RENDER_TILE_COVER const cover_min = fmin(cover_wip->aN.c[ii],color_acc->aN.rgba[ii].a);

      color_acc->aN.rgba[ii].r = mad(+cover_min,color_wip->aN.rgba[ii].r,color_acc->aN.rgba[ii].r);
      color_acc->aN.rgba[ii].g = mad(+cover_min,color_wip->aN.rgba[ii].g,color_acc->aN.rgba[ii].g);
      color_acc->aN.rgba[ii].b = mad(+cover_min,color_wip->aN.rgba[ii].b,color_acc->aN.rgba[ii].b);
      color_acc->aN.rgba[ii].a = mad(-cover_min,color_wip->aN.rgba[ii].a,color_acc->aN.rgba[ii].a);
    }
}

//
//
//

static
void
skc_tile_blend_multiply(union skc_tile_color       * SKC_RESTRICT const color_acc,
                        union skc_tile_cover const * SKC_RESTRICT const cover_wip,
                        union skc_tile_color const * SKC_RESTRICT const color_wip)
{
  //
  // r.acc = (cover.wip * r.wip) * r.acc
  // g.acc = (cover.wip * g.wip) * g.acc
  // b.acc = (cover.wip * b.wip) * b.acc
  // a.acc = (cover.wip * a.wip) * (1.0 - a.acc) <-- a.acc is already (1.0 - alpha)
  //

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    {
      color_acc->aN.rgba[ii].r *= cover_wip->aN.c[ii] * color_wip->aN.rgba[ii].r;
      color_acc->aN.rgba[ii].g *= cover_wip->aN.c[ii] * color_wip->aN.rgba[ii].g;
      color_acc->aN.rgba[ii].b *= cover_wip->aN.c[ii] * color_wip->aN.rgba[ii].b;
      color_acc->aN.rgba[ii].a *= cover_wip->aN.c[ii] * color_wip->aN.rgba[ii].a;
    }
}

//
//
//

static
void
skc_tile_blend_knockout(union skc_tile_cover       * SKC_RESTRICT const cover_acc,
                        union skc_tile_color       * SKC_RESTRICT const color_acc,
                        union skc_tile_cover const * SKC_RESTRICT const cover_wip,
                        union skc_tile_color const * SKC_RESTRICT const color_wip)
{
  //
  // cover.wip.contrib = (1.0 - cover.acc) * cover.wip
  // cover.acc         = cover.acc + cover.wip.contrib
  //
  // r.acc =  cover.wip.contrib * r.wip + r.acc
  // g.acc =  cover.wip.contrib * g.wip + g.acc
  // b.acc =  cover.wip.contrib * b.wip + b.acc
  // a.acc = -cover.wip.contrib * a.wip * a.acc
  //

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    {
      SKC_RENDER_TILE_COVER const contrib = (1 - cover_acc->aN.c[ii]) * cover_wip->aN.c[ii];

      cover_acc->aN.c[ii]     += contrib;

      color_acc->aN.rgba[ii].r = mad(+contrib,color_wip->aN.rgba[ii].r,color_acc->aN.rgba[ii].r);
      color_acc->aN.rgba[ii].g = mad(+contrib,color_wip->aN.rgba[ii].g,color_acc->aN.rgba[ii].g);
      color_acc->aN.rgba[ii].b = mad(+contrib,color_wip->aN.rgba[ii].b,color_acc->aN.rgba[ii].b);
      color_acc->aN.rgba[ii].a = mad(-contrib,color_wip->aN.rgba[ii].a,color_acc->aN.rgba[ii].a);
    }
}

//
//
//

static
void
skc_tile_cover_msk_copy_wip(union skc_tile_cover       * SKC_RESTRICT const cover_msk,
                            union skc_tile_cover const * SKC_RESTRICT const cover_wip)
{
#if !defined( SKC_RENDER_TILE_COVER_VECTOR ) || defined( SKC_ARCH_GEN9 )

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    cover_msk->aN.c[ii] = cover_wip->aN.c[ii];

#else

  // __attribute__((opencl_unroll_hint(SKC_RENDER_TILE_COVER_VECTOR_COUNT)))
  for (uint ii=0; ii<SKC_RENDER_TILE_COVER_VECTOR_COUNT; ii++)
    cover_msk->vN.c[ii] = cover_wip->vN.c[ii];

#endif
}

//
//
//

static
void
skc_tile_cover_msk_copy_acc(union skc_tile_cover       * SKC_RESTRICT const cover_msk,
                            union skc_tile_cover const * SKC_RESTRICT const cover_acc)
{
#if !defined( SKC_RENDER_TILE_COVER_VECTOR ) || defined( SKC_ARCH_GEN9 )

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    cover_msk->aN.c[ii] = cover_acc->aN.c[ii];

#else

  // __attribute__((opencl_unroll_hint(SKC_RENDER_TILE_COVER_VECTOR_COUNTN)))
  for (uint ii=0; ii<SKC_RENDER_TILE_COVER_VECTOR_COUNT; ii++)
    cover_msk->vN.c[ii] = cover_acc->vN.c[ii];

#endif
}

//
//
//

static
void
skc_tile_cover_accumulate(union skc_tile_cover       * SKC_RESTRICT const cover_acc,
                          union skc_tile_cover const * SKC_RESTRICT const cover_wip)
{
  //
  // cover.wip.contrib = (1.0 - cover.acc) * cover.wip
  // cover.acc         = cover.acc + cover.wip.contrib
  //

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    cover_acc->aN.c[ii] = mad(1 - cover_acc->aN.c[ii],cover_wip->aN.c[ii],cover_acc->aN.c[ii]);
}

//
//
//

static
void
skc_tile_cover_wip_mask(union skc_tile_cover       * SKC_RESTRICT const cover_wip,
                        union skc_tile_cover const * SKC_RESTRICT const cover_msk)
{
  //
  // cover.wip *= cover.msk
  //

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    cover_wip->aN.c[ii] *= cover_msk->aN.c[ii];
}

//
//
//

static
void
skc_tile_cover_wip_zero(union skc_tile_cover * SKC_RESTRICT const cover)
{
#if !defined( SKC_RENDER_TILE_COVER_VECTOR ) // || defined( SKC_ARCH_GEN9 )

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    cover->aN.c[ii] = 0;

#else
  //
  // GEN9 compiler underperforms on this
  //

  // __attribute__((opencl_unroll_hint(SKC_RENDER_TILE_COVER_VECTOR_COUNT)))
  for (uint ii=0; ii<SKC_RENDER_TILE_COVER_VECTOR_COUNT; ii++)
    cover->vN.c[ii] = 0;

#endif
}

static
void
skc_tile_cover_acc_zero(union skc_tile_cover * SKC_RESTRICT const cover)
{
#if !defined( SKC_RENDER_TILE_COVER_VECTOR ) // || defined( SKC_ARCH_GEN9 )

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    cover->aN.c[ii] = 0;

#else
  //
  // GEN9 compiler underperforms on this
  //

  // __attribute__((opencl_unroll_hint(SKC_RENDER_TILE_COVER_VECTOR_COUNT)))
  for (uint ii=0; ii<SKC_RENDER_TILE_COVER_VECTOR_COUNT; ii++)
    cover->vN.c[ii] = 0;

#endif
}

static
void
skc_tile_cover_msk_zero(union skc_tile_cover * SKC_RESTRICT const cover)
{
#if !defined( SKC_RENDER_TILE_COVER_VECTOR ) || defined( SKC_ARCH_GEN9 )

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    cover->aN.c[ii] = 0;

#else
  //
  // GEN9 compiler underperforms on this
  //

  // __attribute__((opencl_unroll_hint(SKC_RENDER_TILE_COVER_VECTOR_COUNT)))
  for (uint ii=0; ii<SKC_RENDER_TILE_COVER_VECTOR_COUNT; ii++)
    cover->vN.c[ii] = 0;

#endif
}

//
//
//

static
void
skc_tile_cover_msk_one(union skc_tile_cover * SKC_RESTRICT const cover)
{
#if !defined( SKC_RENDER_TILE_COVER_VECTOR ) || defined( SKC_ARCH_GEN9 )

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    cover->aN.c[ii] = 1;

#else
  //
  // GEN9 compiler underperforms on this
  //

  // __attribute__((opencl_unroll_hint(SKC_RENDER_TILE_COVER_VECTOR_COUNT)))
  for (uint ii=0; ii<SKC_RENDER_TILE_COVER_VECTOR_COUNT; ii++)
    cover->vN.c[ii] = SKC_RENDER_TILE_COVER_VECTOR_ONE;

#endif
}

//
//
//

static
void
skc_tile_cover_msk_invert(union skc_tile_cover * SKC_RESTRICT const cover)
{
#if !defined( SKC_RENDER_TILE_COVER_VECTOR ) || defined( SKC_ARCH_GEN9 )

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    cover->aN.c[ii] = 1 - cover->aN.c[ii];

#else

  // __attribute__((opencl_unroll_hint(SKC_RENDER_TILE_COVER_VECTOR_COUNT)))
  for (uint ii=0; ii<SKC_RENDER_TILE_COVER_VECTOR_COUNT; ii++)
    cover->vN.c[ii] = 1 - cover->vN.c[ii];

#endif
}

//
//
//

static
void
skc_tile_color_wip_zero(union skc_tile_color * SKC_RESTRICT const color)
{
#if !defined( SKC_RENDER_TILE_COLOR_VECTOR ) || defined( SKC_ARCH_GEN9 )

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    {
      color->aN.rgba[ii].r = 0;
      color->aN.rgba[ii].g = 0;
      color->aN.rgba[ii].b = 0;
      color->aN.rgba[ii].a = 1;
    }

#else
  //
  // DISABLED ON GEN9 -- probably a compiler bug
  //
  // __attribute__((opencl_unroll_hint(SKC_RENDER_TILE_COLOR_VECTOR_COUNT)))
  for (uint ii=0; ii<SKC_RENDER_TILE_COLOR_VECTOR_COUNT; ii++)
    color->vN.rgba[ii].even.even = 0;

  // __attribute__((opencl_unroll_hint(SKC_RENDER_TILE_COLOR_VECTOR_COUNT)))
  for (uint ii=0; ii<SKC_RENDER_TILE_COLOR_VECTOR_COUNT; ii++)
    color->vN.rgba[ii].odd.even  = 0;

  // __attribute__((opencl_unroll_hint(SKC_RENDER_TILE_COLOR_VECTOR_COUNT)))
  for (uint ii=0; ii<SKC_RENDER_TILE_COLOR_VECTOR_COUNT; ii++)
    color->vN.rgba[ii].even.odd  = 0;

  // __attribute__((opencl_unroll_hint(SKC_RENDER_TILE_COLOR_VECTOR_COUNT)))
  for (uint ii=0; ii<SKC_RENDER_TILE_COLOR_VECTOR_COUNT; ii++)
    color->vN.rgba[ii].odd.odd   = 1;
#endif
}

static
void
skc_tile_color_acc_zero(union skc_tile_color * SKC_RESTRICT const color)
{
#if !defined( SKC_RENDER_TILE_COLOR_VECTOR ) || defined( SKC_ARCH_GEN9 )

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    {
      color->aN.rgba[ii].r = 0;
      color->aN.rgba[ii].g = 0;
      color->aN.rgba[ii].b = 0;
      color->aN.rgba[ii].a = 1;
    }

#else
  //
  // DISABLED ON GEN9 -- probably a compiler bug
  //
  // __attribute__((opencl_unroll_hint(SKC_RENDER_TILE_COLOR_VECTOR_COUNT)))
  for (uint ii=0; ii<SKC_RENDER_TILE_COLOR_VECTOR_COUNT; ii++)
    color->vN.rgba[ii].even.even = 0;

  // __attribute__((opencl_unroll_hint(SKC_RENDER_TILE_COLOR_VECTOR_COUNT)))
  for (uint ii=0; ii<SKC_RENDER_TILE_COLOR_VECTOR_COUNT; ii++)
    color->vN.rgba[ii].odd.even  = 0;

  // __attribute__((opencl_unroll_hint(SKC_RENDER_TILE_COLOR_VECTOR_COUNT)))
  for (uint ii=0; ii<SKC_RENDER_TILE_COLOR_VECTOR_COUNT; ii++)
    color->vN.rgba[ii].even.odd  = 0;

  // __attribute__((opencl_unroll_hint(SKC_RENDER_TILE_COLOR_VECTOR_COUNT)))
  for (uint ii=0; ii<SKC_RENDER_TILE_COLOR_VECTOR_COUNT; ii++)
    color->vN.rgba[ii].odd.odd   = 1;
#endif
}

//
//
//

static
bool
skc_tile_color_test_opacity(union skc_tile_color const * SKC_RESTRICT const color)
{
  //
  // returns true if tile is opaque
  //
  // various hacks to test for complete tile opacity
  //
  // note that front-to-back currently has alpha at 0.0f -- this can
  // be harmonized to use a traditional alpha if we want to support
  // rendering in either direction
  //
  // hack -- ADD/MAX/OR all alphas together and test for non-zero
  //
  SKC_RENDER_TILE_COLOR t = color->aN.rgba[0].a;

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH-1)))
  for (uint ii=1; ii<SKC_TILE_WIDTH; ii++)
    t += color->aN.rgba[ii].a;

#if ( SKC_RENDER_SUBGROUP_SIZE == 1 )
  //
  // SIMD
  //
  return !any(t != ( 0 ));

#elif ( SKC_RENDER_SCANLINE_VECTOR_SIZE == 1 )
  //
  // SIMT - scalar per lane
  //
  return !sub_group_any(t != 0);

#else
  //
  // SIMT - vector per lane
  //
  return !sub_group_any(any(t != ( 0 )));

#endif

  //
  // TODO: The alternative vector-per-lane implementation below is
  // *not* believed to be performant because the terse vector-wide
  // test is just hiding a series of comparisons and is likely worse
  // than the blind ADD/MAX/OR'ing of all alphas followed by a single
  // test.
  //
#if 0
  //
  // SIMT - vector per lane
  //

  // __attribute__((opencl_unroll_hint(SKC_RENDER_TILE_COLOR_VECTOR_COUNT-1)))
  for (uint ii=0; ii<SKC_RENDER_TILE_COLOR_VECTOR_COUNT; ii++)
    {
      if (sub_group_any(any(color->vN.ba[ii].a != ( 0 ))))
        return false;
    }

  return true;
#endif
}

//
//
//

static
void
skc_tile_background_over(__global union skc_styling_cmd const * SKC_RESTRICT const commands,
                         uint                                 * SKC_RESTRICT const cmd_next,
                         union skc_tile_color                 * SKC_RESTRICT const color)
{
  //
  // acc.r = acc.a * r + acc.r
  // acc.g = acc.a * g + acc.g
  // acc.b = acc.a * b + acc.b
  //
  __global half const * const rgba_ptr = commands[*cmd_next].f16a2 + 0;

  *cmd_next += 2;

  SKC_RENDER_TILE_COLOR_PAIR const rg = SKC_RENDER_TILE_COLOR_PAIR_LOAD(0,rgba_ptr);

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    color->aN.rgba[ii].r = mad(color->aN.rgba[ii].a,rg.lo,color->aN.rgba[ii].r);

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    color->aN.rgba[ii].g = mad(color->aN.rgba[ii].a,rg.hi,color->aN.rgba[ii].g);

  SKC_RENDER_TILE_COLOR_PAIR const ba = SKC_RENDER_TILE_COLOR_PAIR_LOAD(1,rgba_ptr);

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    color->aN.rgba[ii].b = mad(color->aN.rgba[ii].a,ba.lo,color->aN.rgba[ii].b);
}

//
//
//

// #define SKC_SURFACE_IS_BUFFER
#ifdef  SKC_SURFACE_IS_BUFFER

static
void
skc_surface_composite_u8_rgba(__global SKC_RENDER_SURFACE_U8_RGBA * SKC_RESTRICT const surface,
                              skc_uint                                           const surface_pitch,
                              union skc_tile_color          const * SKC_RESTRICT const color,
                              skc_ttck_hi_t                                      const ttck_hi)
{
  //
  // NEW MAJOR OPTIMIZATION:
  //
  // Rotating and rasterizing the original world transform by -90
  // degrees and then rendering the scene scene by +90 degrees enables
  // all the final surface composite to be perfomed in perfectly
  // coalesced wide transactions.
  //
  // For this reason, linear access to the framebuffer is preferred.
  //
  // vvvvvvvvvvvv OLD NOTE BELOW vvvvvvvvvvvvv
  //
  // NOTE THIS IS TRANSPOSED BY 90 DEGREES
  //
  // INTEL HAS A "BLOCK STORE" FEATURE THAT SOLVES THIS AND TEXTURE
  // CACHES ARE ALSO PROBABLY SOMEWHAT FORGIVING.
  //
  // IT'S EASY TO TRANSPOSE THIS IN SMEM BEFORE STORING BUT IN THIS
  // CPU EXAMPLE WE CAN PROBABLY DO WELL BY JUST WRITING OUT SCALARS
  //
  // FIXME -- NEED TO HARMONIZE BYTE AND COMPONENT COLOR CHANNEL
  // ORDERING SO THAT COLOR CHANNELS MATCH 0xAARRGGBBAA ORDER
  //
  uint const pitch = surface_pitch / SKC_RENDER_SCANLINE_VECTOR_SIZE;
  uint const x     = skc_ttck_hi_get_x(ttck_hi);
  uint const y     = skc_ttck_hi_get_y(ttck_hi) ;
  uint const base  = x * SKC_TILE_WIDTH * pitch + y * (SKC_TILE_HEIGHT / SKC_RENDER_SCANLINE_VECTOR_SIZE) + skc_subgroup_lane();

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    {
      SKC_RENDER_SURFACE_U8_RGBA rgba = ( 0xFF000000 );

      rgba |= SKC_CONVERT(SKC_RENDER_SURFACE_U8_RGBA)(color->aN.rgba[ii].r * 255);
      rgba |= SKC_CONVERT(SKC_RENDER_SURFACE_U8_RGBA)(color->aN.rgba[ii].g * 255) << 8;
      rgba |= SKC_CONVERT(SKC_RENDER_SURFACE_U8_RGBA)(color->aN.rgba[ii].b * 255) << 16;

      surface[base + ii * pitch] = rgba;

      // printf("%08v2X\n",rgba);
    }
}

#else

static
void
skc_surface_composite_u8_rgba(__write_only image2d_t                          surface,
                              union skc_tile_color const * SKC_RESTRICT const color,
                              skc_ttck_hi_t                                   const ttck_hi)
{
  //
  // NEW MAJOR OPTIMIZATION:
  //
  // Rotating and rasterizing the original world transform by -90
  // degrees and then rendering the scene scene by +90 degrees enables
  // all the final surface composite to be perfomed in perfectly
  // coalesced wide transactions.
  //
  // For this reason, linear access to the framebuffer is preferred.
  //
  // vvvvvvvvvvvv OLD NOTE BELOW vvvvvvvvvvvvv
  //
  // NOTE THIS IS TRANSPOSED BY 90 DEGREES
  //
  // INTEL HAS A "BLOCK STORE" FEATURE THAT SOLVES THIS AND TEXTURE
  // CACHES ARE ALSO PROBABLY SOMEWHAT FORGIVING.
  //
  // IT'S EASY TO TRANSPOSE THIS IN SMEM BEFORE STORING BUT IN THIS
  // CPU EXAMPLE WE CAN PROBABLY DO WELL BY JUST WRITING OUT SCALARS
  //
  // FIXME -- NEED TO HARMONIZE BYTE AND COMPONENT COLOR CHANNEL
  // ORDERING SO THAT COLOR CHANNELS MATCH 0xAARRGGBBAA ORDER
  //

#if 1
  int x = skc_ttck_hi_get_x(ttck_hi) * SKC_TILE_WIDTH;
  int y = skc_ttck_hi_get_y(ttck_hi) * SKC_TILE_HEIGHT + (skc_subgroup_lane() * SKC_RENDER_SCANLINE_VECTOR_SIZE);

  // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
  for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
    {
#ifdef SKC_RENDER_TILE_COLOR_INTERLEAVED

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,A) {                       \
        SKC_RENDER_SURFACE_WRITE(surface,               \
                                 (int2)(x,y+I),         \
                                 color->iN.rgba[ii] A); \
      }

#else

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,A) {                               \
        SKC_RENDER_SURFACE_COLOR const rgba =                   \
          (SKC_RENDER_SURFACE_COLOR)                            \
          (color->aN.rgba[ii].r C,                              \
           color->aN.rgba[ii].g C,                              \
           color->aN.rgba[ii].b C,                              \
           1.0);                                                \
        SKC_RENDER_SURFACE_WRITE(surface,(int2)(x,y+I),rgba);   \
      }

#endif

      SKC_RENDER_SCANLINE_VECTOR_EXPAND();

      x += 1;
    }
#else
    int x = skc_ttck_hi_get_y(ttck_hi) * SKC_TILE_HEIGHT + (skc_subgroup_lane() * SKC_RENDER_SCANLINE_VECTOR_SIZE);
    int y = skc_ttck_hi_get_x(ttck_hi) * SKC_TILE_WIDTH;

    // __attribute__((opencl_unroll_hint(SKC_TILE_WIDTH)))
    for (uint ii=0; ii<SKC_TILE_WIDTH; ii++)
      {
#ifdef SKC_RENDER_TILE_COLOR_INTERLEAVED

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,A) {                       \
        SKC_RENDER_SURFACE_WRITE(surface,               \
                                 (int2)(x+I,y+ii),      \
                                 color->iN.rgba[ii] A); \
      }

#else

#undef  SKC_EXPAND_X
#define SKC_EXPAND_X(I,S,C,P,A) {                               \
      SKC_RENDER_SURFACE_COLOR const rgba =                     \
        (SKC_RENDER_SURFACE_COLOR)                              \
        (color->aN.rgba[ii].r C,                                \
        color->aN.rgba[ii].g C,                                 \
        color->aN.rgba[ii].b C,                                 \
        1.0);                                                   \
      SKC_RENDER_SURFACE_WRITE(surface,(int2)(x+I,y+ii),rgba);  \
    }

#endif

      SKC_RENDER_SCANLINE_VECTOR_EXPAND();
    }

#endif
}

#endif

//
//
//
static
uint const
skc_ttck_lane(uint const ttck_idx)
{
  return ttck_idx & SKC_RENDER_SUBGROUP_MASK;
}

//
// RENDER KERNEL
//

__kernel
SKC_RENDER_KERNEL_ATTRIBS
void
skc_kernel_render(__global   union  skc_layer_node   const * SKC_RESTRICT const layers,
                  __global   struct skc_group_node   const * SKC_RESTRICT const groups,
                  __global   union  skc_styling_cmd  const * SKC_RESTRICT const commands,     // FIXME -- rename

                  __global   skc_ttck_t              const * SKC_RESTRICT const ttck_keys,    // rename: keys
                  skc_uint                                                const ttck_count,   // rename: key_count

                  __global   uint                    const * SKC_RESTRICT const ttck_offsets, // rename: offsets
                  skc_uint                                                const tile_count,   // rename: offset_count

                  __global   skc_ttxb_t              const * SKC_RESTRICT const ttxb_extent,
#ifdef SKC_SURFACE_IS_BUFFER
                  __global   void                          * SKC_RESTRICT const surface,
#else
                  __write_only image2d_t                                        surface,
#endif
#ifdef SKC_SURFACE_IS_BUFFER
                  skc_uint                                                const surface_pitch,
#endif
                  uint4                                                   const tile_clip)    // rename: clip
{
  //
  // Each subgroup is responsible for a tile.  No extra subgroups are
  // launched.
  //
  // FIXME -- might be better implemented as a "grid stride loop" if
  // Intel GEN really has a local memory "quantum" of 4KB which means
  // we would need to launch 4 subgroups per workgroup.
  //
  // Confirmed: GEN8 has 4KB SLM workgroup min while GEN9 is 1KB.
  //

  //
  // declare tile cover and color registers
  //
  // this used to be a neat unified struct but the Intel GEN compiler
  // wasn't cooperating and spilling to private memory even though all
  // registers were indexed by constants
  //
  union skc_tile_color  color_wip;
  union skc_tile_color  color_acc;

  union skc_tile_cover  cover_wip;
  union skc_tile_cover  cover_acc;
  union skc_tile_cover  cover_msk;

  //
  // which subgroup in the grid is this?
  //
  // TAKE NOTE: the Intel GEN compiler is recognizing get_group_id(0)
  // as a uniform but the alternative calculation used when there are
  // multiple subgroups per workgroup is not cooperating and
  // driving spillage elsewhere.
  //
#if ( SKC_RENDER_WORKGROUP_SUBGROUPS == 1 )
  skc_uint const ttck_offset_idx = get_group_id(0);
#else
  skc_uint const ttck_offset_idx = get_group_id(0) * SKC_RENDER_WORKGROUP_SUBGROUPS + get_sub_group_id();
#endif

  //
  // load the starting ttck for this offset and get a bound on the max
  // number of keys that might be loaded
  //
  // these are uniform across all subgroup lanes
  //
  skc_uint ttck_idx = ttck_offsets[ttck_offset_idx];

  //
  // FIXME -- SIMD/CPU version should probaby load a 256-bit (4-wide)
  // vector of ttck keys
  //
#ifndef SKC_TARGET_ARCH_COALESCED_LOAD_TTCK

  skc_ttck_t ttck = ttck_keys[ttck_idx];

#else

  uint const ttck_base = ttck_idx & ~SKC_RENDER_SUBGROUP_MASK;
  uint const ttck_lane = ttck_idx &  SKC_RENDER_SUBGROUP_MASK;
  skc_ttck_t ttck_s    = ttck_keys[min(ttck_base+max(get_sub_group_local_id(),ttck_lane),ttck_count-1)]

#endif

  //
  // set up style group/layer state
  //
  struct skc_styling_group {
    union skc_group_range range;
    skc_uint              depth;
    skc_uint              id;
  } group;

  group.range.lo = 0;
  group.range.hi = SKC_UINT_MAX;
  group.depth    = 0;
  group.id       = SKC_UINT_MAX;

  //
  // start with clear tile opacity, knockout and flag bits
  //
  // uint color_acc_opacity  = 0; // per lane bit mask -- assumes a PIXEL_TILE_HEIGHT <= 32
  // uint cover_acc_knockout = 0; // per lane bit mask -- assumes a PIXEL_TILE_HEIGHT <= 32
  //
  skc_uint flags = 0;

  //
  // declare and initialize accumulators
  //
#if ( SKC_RENDER_WORKGROUP_SUBGROUPS == 1 )
  __local union skc_subgroup_smem                      smem[1];
#else
  __local union skc_subgroup_smem                      smem_wg[SKC_RENDER_WORKGROUP_SUBGROUPS];
  __local union skc_subgroup_smem * SKC_RESTRICT const smem = smem_wg + get_sub_group_id();
#endif

#ifdef SKC_TARGET_ARCH_COALESCED_LOAD_TTCK
  //
  // select the initial ttck key
  //
  skc_ttck_t ttck;
#if 0
  ttck    = sub_group_broadcast(ttck_s,ttck_lane);    // SHOULD WORK BUT .4454 COMPILER IS BROKEN
#else
  ttck.lo = sub_group_broadcast(ttck_s.lo,ttck_lane); // EXPLICIT WORKAROUND
  ttck.hi = sub_group_broadcast(ttck_s.hi,ttck_lane);
#endif

#endif

  //
  // save the first key so we know what tile we're in
  //
  skc_ttck_t ttck0 = ttck;

  //
  // evaluate the coarse clip as late as possible
  //
  skc_uint const ttck_hi_x = skc_ttck_hi_get_x(ttck0.hi);

  if ((ttck_hi_x < tile_clip.lo.x) || (ttck_hi_x >= tile_clip.hi.x))
    return;

  skc_uint const ttck_hi_y = skc_ttck_hi_get_y(ttck0.hi);

  if ((ttck_hi_y < tile_clip.lo.y) || (ttck_hi_y >= tile_clip.hi.y))
    return;

#if 0
  printf("< %u, %u >\n",ttck_hi_x,ttck_hi_y);
#endif

  //
  // load -> scatter -> flush
  //
  while (true)
    {
      // if scattering is disabled then just run through ttck keys
      bool const is_scatter_enabled = (flags & SKC_TILE_FLAGS_SCATTER_SKIP) == 0;

      // need to clear accumulators before a scatter loop
      if (is_scatter_enabled)
        {
          skc_tile_aa_zero(smem);
        }

      do {
        // skip scattering?
        if (is_scatter_enabled)
          {
            skc_block_id_t const xb_id = skc_ttck_lo_get_ttxb_id(ttck.lo);

            if (skc_ttck_lo_is_prefix(ttck.lo)) {
              skc_scatter_ttpb(ttxb_extent,smem,xb_id);
            } else {
              skc_scatter_ttsb(ttxb_extent,smem,xb_id);
            }
          }

        //
        // any ttck keys left?
        //
        if (++ttck_idx >= ttck_count)
          {
            flags |= SKC_TILE_FLAGS_FLUSH_FINALIZE;
            break;
          }

        //
        // process next ttck key
        //
#ifndef SKC_TARGET_ARCH_COALESCED_LOAD_TTCK
        //
        // SIMD -- read next key
        //
        ttck = ttck_keys[ttck_idx];
#else
        //
        // SIMT -- refresh the ttck_s?
        //
        uint const ttck_lane_next = ttck_idx & SKC_RENDER_SUBGROUP_MASK;

        if (ttck_lane_next == 0)
          ttck_s = ttck_keys[min(ttck_idx+get_sub_group_local_id(),ttck_count-1)];

        //
        // broadcast next key to entire subgroup
        //
#if 0
        ttck    = sub_group_broadcast(ttck_s,ttck_lane_next);    // SHOULD WORK BUT .4454 COMPILER IS BROKEN
#else
        ttck.lo = sub_group_broadcast(ttck_s.lo,ttck_lane_next); // EXPLICIT WORKAROUND
        ttck.hi = sub_group_broadcast(ttck_s.hi,ttck_lane_next);
#endif
#endif
        // continue scattering if on same YXL layer
      } while (skc_ttck_equal_yxl(ttck0,ttck));

      // finalize if no longer on same YX tile
      if (!skc_ttck_hi_equal_yx(ttck0.hi,ttck.hi))
        {
          // otherwise, unwind the tile styling and exit
          flags |= SKC_TILE_FLAGS_FLUSH_FINALIZE;
        }

      //
      // given: new layer id from ttxk key
      //
      // load [layer id]{ group id, depth }
      //
      // if within current group's layer range
      //
      //   if at same depth
      //
      //     load and execute cover>[mask>]color>blend commands
      //
      //   else if not at same depth then move deeper
      //
      //     for all groups in group trail from cur depth to new depth
      //       enter group, saving and initializing regs as necessary
      //     increment depth and update layer range
      //     load and execute cover>[mask>]color>blend commands
      //
      // else not within layer range
      //
      //   exit current group, restoring regs as necessary
      //   decrement depth and update layer range
      //
      //
      skc_layer_id         const layer_id_new   = skc_ttck_get_layer(ttck0); // FIXME -- this was ttck_hi
      union skc_layer_node const layer_node_new = layers[layer_id_new];

      // clear flag that controls group/layer traversal
      flags &= ~SKC_TILE_FLAGS_FLUSH_COMPLETE;

      do {
        bool const unwind = (flags & SKC_TILE_FLAGS_FLUSH_UNWIND) != 0;

        //
        // is layer a child of the current parent group?
        //
        uint cmd_next = 0;

        if (!unwind && (layer_node_new.parent == group.id))
          {
            // execute this layer's cmds
            cmd_next = layer_node_new.cmds;

            // if this is final then configure so groups get unwound, otherwise we're done
            flags   |= ((flags & SKC_TILE_FLAGS_FLUSH_FINALIZE) ? SKC_TILE_FLAGS_FLUSH_UNWIND : SKC_TILE_FLAGS_FLUSH_COMPLETE);
          }
        else if (!unwind && (layer_id_new >= group.range.lo && layer_id_new <= group.range.hi))
          {
            //
            // is layer in a child group?
            //
            union skc_group_parents const gp = groups[layer_node_new.parent].parents;
            uint                    const gn = gp.depth - ++group.depth;

            if (gn == 0)
              group.id = layer_node_new.parent;
            else
              group.id = commands[gp.base + gn - 1].parent;

            // update group layer range
            group.range = groups[group.id].range;

            // enter current group
            cmd_next    = groups[group.id].cmds.enter;
          }
        else // otherwise, exit this group
          {
            // enter current group
            cmd_next = groups[group.id].cmds.leave;

            // decrement group depth
            if (--group.depth == 0)
              {
                flags |= SKC_TILE_FLAGS_FLUSH_COMPLETE;
              }
            else
              {
                // get path_base of current group
                uint const gnpb = groups[group.id].parents.base;

                // get parent of current group
                group.id    = commands[gnpb].parent;

                // update group layer range
                group.range = groups[group.id].range;
              }
          }

        //
        // execute cmds
        //
        while (true)
          {
            union skc_styling_cmd const cmd = commands[cmd_next++];

            switch (cmd.u32 & SKC_STYLING_OPCODE_MASK_OPCODE)
              {
              case SKC_STYLING_OPCODE_NOOP:
                break;

              case SKC_STYLING_OPCODE_COVER_NONZERO:
                skc_tile_cover_nonzero(smem,&cover_wip,&color_wip);
                break;

              case SKC_STYLING_OPCODE_COVER_EVENODD:
                skc_tile_cover_evenodd(smem,&cover_wip,&color_wip);
                break;

              case SKC_STYLING_OPCODE_COVER_ACCUMULATE:
                skc_tile_cover_accumulate(&cover_acc,&cover_wip);
                break;

              case SKC_STYLING_OPCODE_COVER_MASK:
                skc_tile_cover_wip_mask(&cover_wip,&cover_msk);
                break;

              case SKC_STYLING_OPCODE_COVER_WIP_ZERO:
                skc_tile_cover_wip_zero(&cover_wip);
                break;

              case SKC_STYLING_OPCODE_COVER_ACC_ZERO:
                skc_tile_cover_acc_zero(&cover_acc);
                break;

              case SKC_STYLING_OPCODE_COVER_MASK_ZERO:
                skc_tile_cover_msk_zero(&cover_msk);
                break;

              case SKC_STYLING_OPCODE_COVER_MASK_ONE:
                skc_tile_cover_msk_one(&cover_msk);
                break;

              case SKC_STYLING_OPCODE_COVER_MASK_INVERT:
                skc_tile_cover_msk_invert(&cover_msk);
                break;

              case SKC_STYLING_OPCODE_COLOR_FILL_SOLID:
                skc_tile_color_fill_solid(commands,&cmd_next,&color_wip);
                break;

              case SKC_STYLING_OPCODE_COLOR_FILL_GRADIENT_LINEAR:
                //
                // FIXME -- gradients shouldn't be executing so much
                // conditional driven code at runtime since we *know*
                // the gradient style on the host can just create a
                // new styling command to exploit this.
                //
                // FIXME -- it might be time to try using the GPU's
                // sampler on a linear array of half4 vectors -- it
                // might outperform the explicit load/lerp routines.
                //
                // FIXME -- optimizing for vertical gradients (uhhh,
                // they're actually horizontal due to the -90 degree
                // view transform) is nice but is it worthwhile to
                // have this in the kernel?  Easy to add it back...
                //
#if defined( SKC_ARCH_GEN9 )
                // disable gradients due to exessive spillage -- fix later
                cmd_next += SKC_GRADIENT_CMD_WORDS_V1(commands[cmd_next+6].u32);
#else
                skc_tile_color_fill_gradient_linear_nonvertical(smem,commands,&cmd_next,&color_wip,ttck0.hi);
#endif
                break;

              case SKC_STYLING_OPCODE_COLOR_WIP_ZERO:
                skc_tile_color_wip_zero(&color_wip);
                break;

              case SKC_STYLING_OPCODE_COLOR_ACC_ZERO:
                skc_tile_color_acc_zero(&color_acc);
                break;

              case SKC_STYLING_OPCODE_BLEND_OVER:
                skc_tile_blend_over(&color_acc,&cover_wip,&color_wip);
                break;

              case SKC_STYLING_OPCODE_BLEND_PLUS:
                skc_tile_blend_plus(&color_acc,&cover_wip,&color_wip);
                break;

              case SKC_STYLING_OPCODE_BLEND_MULTIPLY:
                skc_tile_blend_multiply(&color_acc,&cover_wip,&color_wip);
                break;

              case SKC_STYLING_OPCODE_BLEND_KNOCKOUT:
                skc_tile_blend_knockout(&cover_acc,&color_acc,&cover_wip,&color_wip);
                break;

              case SKC_STYLING_OPCODE_COVER_WIP_MOVE_TO_MASK:
                // skc_tile_cover_msk_copy_wip(&cover_msk,&cover_wip);
                break;

              case SKC_STYLING_OPCODE_COVER_ACC_MOVE_TO_MASK:
                // skc_tile_cover_msk_copy_acc(&cover_msk,&cover_acc);
                break;

              case SKC_STYLING_OPCODE_BACKGROUND_OVER:
                skc_tile_background_over(commands,&cmd_next,&color_acc);
                break;

              case SKC_STYLING_OPCODE_SURFACE_COMPOSITE:
#ifdef SKC_SURFACE_IS_BUFFER
                skc_surface_composite_u8_rgba(surface,surface_pitch,&color_acc,ttck0.hi);
#else
                skc_surface_composite_u8_rgba(surface,              &color_acc,ttck0.hi);
#endif
                break;

              case SKC_STYLING_OPCODE_COLOR_ACC_TEST_OPACITY:
                if (skc_tile_color_test_opacity(&color_acc))
                  flags |= SKC_TILE_FLAGS_SCATTER_SKIP;
                break;

              default:
                return; // this is an illegal opcode -- trap and die!
              }

            //
            // if sign bit is set then this was final command
            //
            if (cmd.s32 < 0)
              break;
          }

        // continue as long as tile flush isn't complete
      } while ((flags & SKC_TILE_FLAGS_FLUSH_COMPLETE) == 0);

      // return if was the final flush
      if (flags & SKC_TILE_FLAGS_FLUSH_FINALIZE)
        return;

      // update wip ttck_hi
      ttck0 = ttck;
    }
}

//
//
//
