

typedef bool skc_bool;

typedef char skc_char;
typedef char2 skc_char2;
typedef char3 skc_char3;
typedef char4 skc_char4;
typedef char8 skc_char8;
typedef char16 skc_char16;

typedef uchar skc_uchar;
typedef uchar2 skc_uchar2;
typedef uchar3 skc_uchar3;
typedef uchar4 skc_uchar4;
typedef uchar8 skc_uchar8;
typedef uchar16 skc_uchar16;

typedef short skc_short;
typedef short2 skc_short2;
typedef short3 skc_short3;
typedef short4 skc_short4;
typedef short8 skc_short8;
typedef short16 skc_short16;

typedef ushort skc_ushort;
typedef ushort2 skc_ushort2;
typedef ushort3 skc_ushort3;
typedef ushort4 skc_ushort4;
typedef ushort8 skc_ushort8;
typedef ushort16 skc_ushort16;

typedef int skc_int;
typedef int2 skc_int2;
typedef int3 skc_int3;
typedef int4 skc_int4;
typedef int8 skc_int8;
typedef int16 skc_int16;

typedef uint skc_uint;
typedef uint2 skc_uint2;
typedef uint3 skc_uint3;
typedef uint4 skc_uint4;
typedef uint8 skc_uint8;
typedef uint16 skc_uint16;

typedef ulong skc_ulong;
typedef ulong2 skc_ulong2;
typedef ulong3 skc_ulong3;
typedef ulong4 skc_ulong4;
typedef ulong8 skc_ulong8;
typedef ulong16 skc_ulong16;

typedef long skc_long;
typedef long2 skc_long2;
typedef long3 skc_long3;
typedef long4 skc_long4;
typedef long8 skc_long8;
typedef long16 skc_long16;

typedef float skc_float;
typedef float2 skc_float2;
typedef float3 skc_float3;
typedef float4 skc_float4;
typedef float8 skc_float8;
typedef float16 skc_float16;

typedef half skc_half;

typedef enum skc_block_id_tag {

  SKC_BLOCK_ID_TAG_PATH_LINE,
  SKC_BLOCK_ID_TAG_PATH_QUAD,
  SKC_BLOCK_ID_TAG_PATH_CUBIC,
  SKC_BLOCK_ID_TAG_PATH_RAT_QUAD,
  SKC_BLOCK_ID_TAG_PATH_RAT_CUBIC,
  SKC_BLOCK_ID_TAG_PATH_NEXT,

  SKC_BLOCK_ID_TAG_INVALID = (1u << 5) - 1,
  SKC_BLOCK_ID_TAG_COUNT,

} skc_block_id_tag;

typedef skc_uint skc_block_id_t;

typedef skc_uint skc_tagged_block_id_t;

union skc_tagged_block_id
{
  skc_uint u32;
};

typedef skc_uint skc_paths_copy_elem;
typedef skc_uint skc_pb_idx_v;

typedef skc_block_id_t skc_block_id_v_t;
typedef skc_uint2 skc_ttsk_v_t;
typedef skc_uint2 skc_ttsk_s_t;

typedef skc_uint skc_bp_elem_t;

typedef skc_uint2 skc_ttrk_e_t;
typedef skc_uint2 skc_ttsk_v_t;
typedef skc_uint2 skc_ttsk_s_t;
typedef skc_uint2 skc_ttpk_s_t;
typedef skc_uint2 skc_ttxk_v_t;

typedef skc_int skc_tts_v_t;

typedef skc_int skc_ttp_t;

typedef skc_uint skc_raster_yx_s;

typedef skc_block_id_t skc_block_id_v_t;
typedef skc_block_id_t skc_block_id_s_t;

typedef skc_uint skc_bp_elem_t;

typedef skc_uint skc_ttsk_lo_t;
typedef skc_uint skc_ttsk_hi_t;

typedef skc_uint skc_ttpk_lo_t;
typedef skc_uint skc_ttpk_hi_t;

typedef skc_uint skc_ttxk_lo_t;
typedef skc_uint skc_ttxk_hi_t;

typedef skc_uint2 skc_ttck_t;

typedef skc_bool skc_pred_v_t;
typedef skc_int skc_int_v_t;

#pragma OPENCL EXTENSION cl_khr_fp16 : enable

union skc_transform
{

  skc_float f32a8[8];

  skc_float8 f32v8;

  struct
  {
    skc_float sx;
    skc_float shx;
    skc_float tx;

    skc_float shy;
    skc_float sy;
    skc_float ty;

    skc_float w0;
    skc_float w1;
  };
};

union skc_path_clip
{
  skc_float f32a4[4];

  skc_float4 f32v4;

  struct
  {
    skc_float x0;
    skc_float y0;
    skc_float x1;
    skc_float y1;
  };
};

typedef skc_uint skc_path_h;

union skc_cmd_fill
{
  skc_ulong u64;

  skc_uint2 u32v2;

  struct
  {
    skc_path_h path;

    skc_uint tcc;
  };
};

typedef skc_uint skc_raster_h;

union skc_cmd_place
{
  skc_uint4 u32v4;

  struct
  {
    skc_raster_h raster_h;
    skc_uint layer_id;
    skc_uint tx;
    skc_uint ty;
  };
};

;

;
;
;

union skc_cmd_rasterize
{
  skc_ulong u64;

  skc_uint2 u32v2;

  struct
  {

    skc_uint nodeword;

    skc_uint tcc;
  };
};

;

union skc_raster_cohort_meta_in
{
  skc_uint4 u32v4;

  struct
  {
    skc_uint blocks;
    skc_uint offset;
    skc_uint pk;
    skc_uint rk;
  };
};

union skc_raster_cohort_meta_out
{
  skc_uint4 u32v4;

  struct
  {
    skc_uint blocks;
    skc_uint offset;
    skc_uint nodes;
    skc_uint keys;
  };
};

union skc_raster_cohort_meta_inout
{
  union skc_raster_cohort_meta_in in;
  union skc_raster_cohort_meta_out out;
};

struct skc_raster_cohort_meta
{
  union skc_raster_cohort_meta_inout inout[(1 << 8)];
  skc_uint reads[(1 << 8)];
};

struct skc_raster_cohort_atomic
{

  skc_uint cmds;

  skc_uint keys;
};

;
;

typedef skc_uint2 skc_ttsk_t;
typedef skc_uint2 skc_ttpk_t;
typedef skc_uint2 skc_ttxk_t;

union skc_raster_node_next
{
  skc_uint2 u32v2;

  struct
  {
    skc_block_id_t node;
    skc_uint na;
  };
};

union skc_raster_node_elem
{
  skc_uint2 u32v2;
  skc_ttsk_t sk;
  skc_ttpk_t pk;
  skc_ttxk_t xk;
  union skc_raster_node_next next;
};

union skc_raster_header
{
  skc_uint4 u32v4;

  struct
  {
    skc_uint blocks;
    skc_uint na;
    skc_uint nodes;
    skc_uint keys;
  };
};

struct skc_raster_head
{
  union skc_raster_header header;
  skc_int4 bounds;
  union skc_raster_node_elem elems[];
};

union skc_tile_coord
{
  skc_uint u32;

  struct
  {

    skc_uint xy;
  };
};

;

union skc_tile_clip
{
  skc_uint u32a2[2];

  skc_uint2 u32v2;

  struct
  {
    union skc_tile_coord xy0;
    union skc_tile_coord xy1;
  };
};

;

union skc_subgroup_accum
{
  struct
  {
    int ttp[(1 << (3 + 1))];
  } atomic;

  struct
  {
    skc_ttp_t ttp[(1 << (3 + 1))];
  } aN;

  struct
  {
    skc_uint2 ttp[8];
  } vN;

  struct
  {
    ulong ttp[(1 << (3 + 1)) / (sizeof(ulong) / sizeof(skc_ttp_t))];
  } zero;
};

struct skc_subgroup_smem
{

  union skc_subgroup_accum accum;
};

static skc_uint
skc_subgroup_lane()
{

  return get_sub_group_local_id();
}

static skc_int
skc_tts_get_dy(skc_tts_v_t const ttsv)
{

  skc_int const dy = ttsv >> (10 + 6 + 10);

  return dy - (~ttsv >> 31);
}

static skc_int
skc_tts_get_py(skc_tts_v_t const ttsv)
{
  return (((ttsv) & ((((skc_uint)1 << (10 - 5)) - 1) << ((10 + 6) + 5))) >>
          ((10 + 6) + 5));
}

static void
skc_accum_scatter(__local struct skc_subgroup_smem* const smem,
                  skc_tts_v_t const tts_v)
{

  skc_int dy = skc_tts_get_dy(tts_v);

  skc_int py = skc_tts_get_py(tts_v);

  if (tts_v != (0xFFFFFFFF)) {
    atomic_add(smem->accum.atomic.ttp + py, dy);
  };
}

static void
skc_accum_flush(__local struct skc_subgroup_smem* const smem,
                __global skc_bp_elem_t* const bp_elems,
                skc_block_id_t const pb_id)
{

  skc_uint2 const ttp_v = smem->accum.vN.ttp[get_sub_group_local_id()];
  skc_uint const offset =
    pb_id * ((1u << 3) / ((1 << (3 + 1)) / (1 << 3))) + skc_subgroup_lane();

  vstore2(ttp_v, offset, bp_elems);
}

static void
skc_accum_reset(__local struct skc_subgroup_smem* const smem)
{
  for (uint ii = 0;
       ii < (1 << (3 + 1)) / (sizeof(ulong) / sizeof(skc_ttp_t)) / 8;
       ii++)
    smem->accum.zero.ttp[ii * 8 + skc_subgroup_lane()] = (0);
}

static skc_ttsk_s_t
skc_ttsk_v_get_next(skc_ttsk_v_t* const sk_v,
                    skc_uint* const sk_next,
                    skc_int* const rkpk_rem)
{

  *rkpk_rem -= 1;

  skc_ttsk_s_t sk_s;

  sk_s.lo = sub_group_broadcast(sk_v->lo, *sk_next);
  sk_s.hi = sub_group_broadcast(sk_v->hi, *sk_next);
  *sk_next += 1;

  return sk_s;
}

static skc_raster_yx_s
skc_ttsk_v_first(skc_ttsk_v_t* const sk_v, skc_uint const sk_next)
{

  skc_raster_yx_s const yx_s = sub_group_broadcast(sk_v->hi, sk_next);

  return yx_s;
}

static skc_block_id_s_t
skc_ttsk_s_get_ttsb_id(skc_ttsk_s_t const* const sk_s)
{
  return (sk_s->lo & (((skc_uint)1 << (27)) - 1));
}

static skc_tts_v_t
skc_load_tts(__global skc_bp_elem_t* const bp_elems,
             skc_block_id_s_t const sb_id)
{
  return (bp_elems[sb_id * (1u << 3) + skc_subgroup_lane()]);
}

static void
skc_ttrk_to_ttsk(skc_ttsk_v_t* const sk_v)
{
  sk_v->lo = sk_v->lo & (((skc_uint)1 << (27)) - 1);
  sk_v->hi = sk_v->hi << 8;
}

static void
skc_ttsk_v_replenish(skc_ttsk_v_t* const sk_v,
                     skc_uint* const sk_next,
                     skc_uint* const rks_next,
                     __global skc_ttrk_e_t const* const rks)
{

  if (*sk_next < 8)
    return;

  *sk_next = 0;
  *rks_next += 8;
  *sk_v = rks[*rks_next];

  skc_ttrk_to_ttsk(sk_v);
}

static void
skc_blocks_replenish(skc_uint* const blocks_next,
                     skc_uint* const blocks_idx,
                     skc_block_id_v_t* const blocks,
                     skc_uint const bp_mask,
                     __global skc_block_id_t const* const bp_ids)

{
  *blocks_idx += 8;
  *blocks = bp_ids[*blocks_idx & bp_mask];
  *blocks_next = 0;
}

static skc_block_id_t
skc_blocks_get_next(skc_uint* const blocks_next,
                    skc_uint* const blocks_idx,
                    skc_block_id_v_t* const blocks,
                    skc_uint const bp_mask,
                    __global skc_block_id_t const* const bp_ids)
{

  if (*blocks_next == 8) {
    skc_blocks_replenish(blocks_next, blocks_idx, blocks, bp_mask, bp_ids);
  }

  skc_block_id_t id = sub_group_broadcast(*blocks, *blocks_next);

  *blocks_next += 1;

  return id;
}

static skc_block_id_t
skc_subblocks_get_next_pb_id(skc_block_id_t* const subblocks,
                             skc_uint* const blocks_next,
                             skc_uint* const blocks_idx,
                             skc_block_id_v_t* const blocks,
                             skc_uint const bp_mask,
                             __global skc_block_id_t const* const bp_ids)
{
  if ((*subblocks & (((skc_uint)1 << (6 - 3)) - 1)) == 0) {
    *subblocks =
      skc_blocks_get_next(blocks_next, blocks_idx, blocks, bp_mask, bp_ids);
  }

  skc_block_id_t const pb_id = *subblocks;

  *subblocks += ((1 << (3 + 1)) / (1 << 3));

  return pb_id;
}

static void
skc_node_v_append_sk(skc_ttsk_s_t const* const sk_s,

                     skc_ttxk_v_t* const xk_v,
                     skc_uint* const xk_v_next,
                     skc_uint* const xk_v_idx,
                     __global skc_bp_elem_t* const bp_elems,

                     skc_int const rkpk_rem,

                     skc_uint* const blocks_next,
                     skc_uint* const blocks_idx,
                     skc_block_id_v_t* const blocks,
                     skc_uint const bp_mask,
                     __global skc_block_id_t const* const bp_ids)
{

  if (get_sub_group_local_id() == (*xk_v_next & (8 - 1))) {
    *xk_v = *sk_s;
  }

  *xk_v_next += 1;

  if (rkpk_rem > 0) {

    if (*xk_v_next == ((1u << 6) / 2) - 1) {
      skc_block_id_t const id =
        skc_blocks_get_next(blocks_next, blocks_idx, blocks, bp_mask, bp_ids);

      if (get_sub_group_local_id() == 8 - 1) {
        xk_v->lo = id;
        xk_v->hi = 0xFFFFFFFF;
      }

      bp_elems[*xk_v_idx] = xk_v->lo;
      bp_elems[*xk_v_idx + 8] = xk_v->hi;

      xk_v->lo = 0xFFFFFFFF;
      xk_v->hi = 0xFFFFFFFF;

      *xk_v_idx = id * (1u << 3) + get_sub_group_local_id();

      *xk_v_next = 0;
    }

    else if ((*xk_v_next & (8 - 1)) == 0) {

      bp_elems[*xk_v_idx] = xk_v->lo;
      bp_elems[*xk_v_idx + 8] = xk_v->hi;

      xk_v->lo = 0xFFFFFFFF;
      xk_v->hi = 0xFFFFFFFF;

      *xk_v_idx += 8 * 2;
    }
  } else {
    bp_elems[*xk_v_idx] = xk_v->lo;
    bp_elems[*xk_v_idx + 8] = xk_v->hi;

    while ((*xk_v_idx & (((skc_uint)1 << (6)) - 1)) < (1u << 6) - 8 * 2) {
      *xk_v_idx += 8 * 2;

      bp_elems[*xk_v_idx] = 0xFFFFFFFF;
      bp_elems[*xk_v_idx + 8] = 0xFFFFFFFF;
    }
  }
}

static skc_ttpk_s_t
skc_ttpk_create(skc_raster_yx_s const yx_prev,
                skc_raster_yx_s const yx_next,
                skc_block_id_t const pb_id)
{

  skc_uint const yx_span = yx_next - yx_prev;

  skc_ttpk_s_t pk;

  pk.lo = pb_id | ((((skc_uint)1 << (1)) - 1) << (27)) |
          (yx_span << ((27 + 1) - ((32 - 12) - 12)));

  pk.hi = (yx_span >> (((32 - 12) - 12) + (32 - (27 + 1)))) | yx_prev;

  return pk;
}

static void
skc_node_v_append_pk(skc_ttpk_s_t const* const pk_s,

                     skc_ttxk_v_t* const xk_v,
                     skc_uint* const xk_v_next,
                     skc_uint* const xk_v_idx,
                     __global skc_bp_elem_t* const bp_elems,

                     skc_uint* const blocks_next,
                     skc_uint* const blocks_idx,
                     skc_block_id_v_t* const blocks,
                     skc_uint const bp_mask,
                     __global skc_block_id_t const* const bp_ids)
{

  if (get_sub_group_local_id() == (*xk_v_next & (8 - 1))) {
    *xk_v = *pk_s;
  }

  *xk_v_next += 1;

  if (*xk_v_next == ((1u << 6) / 2) - 1) {
    skc_block_id_t const id =
      skc_blocks_get_next(blocks_next, blocks_idx, blocks, bp_mask, bp_ids);

    if (get_sub_group_local_id() == 8 - 1) {
      xk_v->lo = id;
      xk_v->hi = 0xFFFFFFFF;
    }

    bp_elems[*xk_v_idx] = xk_v->lo;
    bp_elems[*xk_v_idx + 8] = xk_v->hi;

    xk_v->lo = 0xFFFFFFFF;
    xk_v->hi = 0xFFFFFFFF;

    *xk_v_idx = id * (1u << 3) + get_sub_group_local_id();

    *xk_v_next = 0;
  }

  else if ((*xk_v_next & (8 - 1)) == 0) {

    bp_elems[*xk_v_idx] = xk_v->lo;
    bp_elems[*xk_v_idx + 8] = xk_v->hi;

    xk_v->lo = 0xFFFFFFFF;
    xk_v->hi = 0xFFFFFFFF;

    *xk_v_idx += 8 * 2;
  }
}

static void
skc_node_v_init_header(skc_ttxk_v_t* const xk_v,
                       skc_uint* const xk_v_next,
                       union skc_raster_cohort_meta_out const* const meta)
{

  if (get_sub_group_local_id() < 2) {
    *xk_v =
      ((get_sub_group_local_id() & 1) == 0) ? meta->u32v4.lo : meta->u32v4.hi;
  }

  *xk_v_next = 2 + 2;
}

__kernel __attribute__((intel_reqd_sub_group_size(8)))
__attribute__((reqd_work_group_size(8 * 1, 1, 1))) void
skc_kernel_prefix(__global skc_uint const* const bp_atomics,
                  __global skc_block_id_t const* const bp_ids,
                  __global skc_bp_elem_t* const bp_elems,
                  skc_uint const bp_mask,
                  __global skc_ttrk_e_t const* const rks,
                  __global skc_block_id_t* const map,
                  __global skc_uint const* const metas,
                  skc_uint const count)
{

  __local struct skc_subgroup_smem smem[1];

  skc_uint const sgi = get_group_id(0);

  skc_uint const sgl = get_sub_group_local_id();

  union skc_raster_cohort_meta_out const meta = { vload4(sgi, metas) };
  skc_uint const reads =
    metas[(((size_t) & (((struct skc_raster_cohort_meta*)0)->reads)) /
           sizeof(skc_uint)) +
          sgi];

  skc_uint blocks_idx = (reads & ~(8 - 1)) + skc_subgroup_lane();
  skc_block_id_v_t blocks = bp_ids[blocks_idx & bp_mask];
  skc_uint blocks_next = (reads & (8 - 1));

  skc_uint xk_v_idx = sub_group_broadcast(blocks, blocks_next++) * (1u << 3) +
                      get_sub_group_local_id();

  skc_ttxk_v_t xk_v = { 0xFFFFFFFF, 0xFFFFFFFF };
  skc_uint xk_v_next;

  skc_node_v_init_header(&xk_v, &xk_v_next, &meta);

  if (meta.keys == 0) {
    bp_elems[xk_v_idx] = xk_v.lo;
    bp_elems[xk_v_idx + 8] = xk_v.hi;

    while ((xk_v_idx & (((skc_uint)1 << (6)) - 1)) < (1u << 6) - 8 * 2) {
      xk_v_idx += 8 * 2;

      bp_elems[xk_v_idx] = 0xFFFFFFFF;
      bp_elems[xk_v_idx + 8] = 0xFFFFFFFF;
    }

    return;
  }

  skc_uint rks_next = (meta.offset & ~(8 - 1)) + skc_subgroup_lane();
  skc_ttsk_v_t sk_v = rks[rks_next];
  skc_uint sk_next = (meta.offset & (8 - 1));
  skc_int rkpk_rem = meta.keys;

  skc_ttrk_to_ttsk(&sk_v);

  skc_block_id_t subblocks = 0;

  skc_raster_yx_s yx_prev = skc_ttsk_v_first(&sk_v, sk_next);

  skc_accum_reset(smem);

  while (true) {

    skc_ttsk_s_t const sk_s = skc_ttsk_v_get_next(&sk_v, &sk_next, &rkpk_rem);

    skc_block_id_s_t const sb_id = skc_ttsk_s_get_ttsb_id(&sk_s);

    skc_tts_v_t const tts_v = skc_load_tts(bp_elems, sb_id);

    if (sk_s.hi != yx_prev) {

      if (((sk_s.hi ^ yx_prev) &
           ((((skc_uint)1 << (12)) - 1) << ((32 - 12)))) == 0) {

        skc_block_id_t const pb_id = skc_subblocks_get_next_pb_id(
          &subblocks, &blocks_next, &blocks_idx, &blocks, bp_mask, bp_ids);

        skc_accum_flush(smem, bp_elems, pb_id);

        rkpk_rem -= 1;

        skc_ttpk_s_t const pk_s =
          skc_ttpk_create(yx_prev + (1u << ((32 - 12) - 12)), sk_s.hi, pb_id);

        skc_node_v_append_pk(&pk_s,

                             &xk_v,
                             &xk_v_next,
                             &xk_v_idx,
                             bp_elems,

                             &blocks_next,
                             &blocks_idx,
                             &blocks,
                             bp_mask,
                             bp_ids);
      } else if (rkpk_rem > 0) {
        skc_accum_reset(smem);
      }
    }

    skc_node_v_append_sk(&sk_s,

                         &xk_v,
                         &xk_v_next,
                         &xk_v_idx,
                         bp_elems,

                         rkpk_rem,

                         &blocks_next,
                         &blocks_idx,
                         &blocks,
                         bp_mask,
                         bp_ids);

    if (rkpk_rem == 0)
      break;

    yx_prev = sk_s.hi;

    skc_accum_scatter(smem, tts_v);

    skc_ttsk_v_replenish(&sk_v, &sk_next, &rks_next, rks);
  }
}
