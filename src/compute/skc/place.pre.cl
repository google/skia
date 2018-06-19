

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

union skc_subgroup_smem
{
  skc_uint scratch[16];

  struct
  {
    struct
    {
      skc_ttsk_lo_t
        sk[((((((1u << 6) / 2) - 1)) > (16)) ? ((((1u << 6) / 2) - 1)) : (16))];
      skc_ttpk_lo_t pk[(((1u << 6) / 2) / 2)];
    } lo;

    struct
    {
      skc_ttsk_hi_t
        sk[((((((1u << 6) / 2) - 1)) > (16)) ? ((((1u << 6) / 2) - 1)) : (16))];
      skc_ttpk_hi_t pk[(((1u << 6) / 2) / 2)];
    } hi;
  };
};

static skc_int_v_t
skc_scatter_scan_max(__local union skc_subgroup_smem volatile* const smem,
                     skc_int_v_t const iss,
                     skc_int_v_t const ess)
{

  skc_pred_v_t const is_scratch_store = (iss > 0) && (ess < 16);
  skc_int_v_t const scratch_idx = max(ess, 0);

  smem->scratch[get_sub_group_local_id()] = (0);

  if (is_scratch_store) {
    smem->scratch[scratch_idx] = get_sub_group_local_id();
  }

  skc_int_v_t const scratch = smem->scratch[get_sub_group_local_id()];
  skc_int_v_t const source = sub_group_scan_inclusive_max(scratch);

  return source;
}

static skc_bool
skc_xk_clip(union skc_tile_clip const* const tile_clip, skc_ttxk_t* const xk)
{

  return false;
}

static skc_ttck_t
skc_sk_to_ck(__local union skc_subgroup_smem volatile* const smem,
             union skc_cmd_place const* const cmd,
             skc_uint const sk_idx)
{
  skc_uint const lo = smem->lo.sk[sk_idx];
  skc_uint const hi = smem->hi.sk[sk_idx];

  skc_ttck_t ck;

  ck.lo = lo | (cmd->layer_id << ((27 + 1) + 1));

  skc_uint const x =
    (cmd->tx + (((hi) & ((((skc_uint)1 << (12)) - 1) << (((32 - 12) - 12)))) >>
                (((32 - 12) - 12))))
    << 15;
  skc_uint const y =
    (cmd->ty +
     (((hi) & ((((skc_uint)1 << (12)) - 1) << ((32 - 12)))) >> ((32 - 12))))
    << 24;

  ck.hi = (cmd->layer_id >> (8 + 9 + 18 - 32)) | x | y;

  return ck;
}

static skc_ttck_t
skc_pk_to_ck(__local union skc_subgroup_smem volatile* const smem,
             union skc_cmd_place const* const cmd,
             skc_uint const pk_idx,
             skc_uint const dx)
{
  skc_uint const lo = smem->lo.pk[pk_idx] & (((skc_uint)1 << ((27 + 1))) - 1);
  skc_uint const hi = smem->hi.pk[pk_idx];

  skc_ttck_t ck;

  ck.lo = lo | (cmd->layer_id << ((27 + 1) + 1));

  skc_uint const x =
    (cmd->tx + dx +
     (((hi) & ((((skc_uint)1 << (12)) - 1) << (((32 - 12) - 12)))) >>
      (((32 - 12) - 12))))
    << 15;
  skc_uint const y =
    (cmd->ty +
     (((hi) & ((((skc_uint)1 << (12)) - 1) << ((32 - 12)))) >> ((32 - 12))))
    << 24;

  ck.hi = (cmd->layer_id >> (8 + 9 + 18 - 32)) | x | y;

  return ck;
}

static void
skc_ttsk_flush(__global uint volatile* const place_atomics,
               __global skc_ttck_t* const ck_extent,
               __local union skc_subgroup_smem volatile* const smem,
               union skc_cmd_place const* const cmd,
               skc_uint const sk)
{

  skc_uint ck_base = 0;

  if (get_sub_group_local_id() == 0) {
    ck_base = atomic_add(place_atomics, sk);
  }

  ck_base = sub_group_broadcast(ck_base, 0);

  for (skc_uint ii = get_sub_group_local_id(); ii < sk; ii += 16) {
    ck_extent[ck_base + ii] = skc_sk_to_ck(smem, cmd, ii);
  }
}

static skc_int
skc_ttpk_get_span(__local union skc_subgroup_smem volatile* const smem,
                  skc_uint const idx)
{
  skc_uint const lo = smem->lo.pk[idx];
  skc_uint const hi = smem->hi.pk[idx];

  skc_uint const span_lo = lo >> (27 + 1);
  skc_uint const span_hi =
    (hi & (((skc_uint)1 << ((12 - (32 - (27 + 1))))) - 1)) << (32 - (27 + 1));

  return (span_lo | span_hi) + 1;
}

static void
skc_ttpk_flush(__global uint volatile* const place_atomics,
               __global skc_ttck_t* const ck_extent,
               __local union skc_subgroup_smem volatile* const smem,
               union skc_cmd_place const* const cmd,
               skc_uint const pk)
{

  if (pk == 0)
    return;

  skc_uint const pk_ru = (pk + 16 - 1) & ~(16 - 1);
  skc_uint ii = 0;

  {
    skc_uint idx = ii + get_sub_group_local_id();
    skc_int span = 0;

    if (idx < pk)
      span = skc_ttpk_get_span(smem, idx);

    skc_int iss = sub_group_scan_inclusive_add(span);
    skc_int ess = iss - span;
    skc_int rem = sub_group_broadcast(iss, 16 - 1);

    skc_uint ck_base = 0;

    if (get_sub_group_local_id() == 0) {
      ck_base = atomic_add(place_atomics, rem);
    }

    skc_uint ck_idx =
      sub_group_broadcast(ck_base, 0) + get_sub_group_local_id();

    while (true) {
      skc_int const source = skc_scatter_scan_max(smem, iss, ess);
      skc_int const dx =
        get_sub_group_local_id() - intel_sub_group_shuffle(ess, source);

      if (get_sub_group_local_id() < rem) {
        ck_extent[ck_idx] = skc_pk_to_ck(smem, cmd, ii + source, dx);
      }

      rem -= 16;

      if (rem <= 0)
        break;

      ck_idx += 16;
      iss -= 16;
      ess -= 16;
    }
  }
}

static skc_uint
skc_ballot(skc_uint* const xk, skc_uint const is_xk)
{

  skc_uint const prefix = sub_group_scan_inclusive_add(is_xk);

  skc_uint const xk_idx = *xk + prefix - is_xk;

  *xk += sub_group_broadcast(prefix, (16 - 1));

  return xk_idx;
}

__kernel __attribute__((intel_reqd_sub_group_size(16)))
__attribute__((reqd_work_group_size(16 * 1, 1, 1))) void
skc_kernel_place(__global skc_bp_elem_t* const bp_elems,
                 __global uint volatile* const place_atomics,
                 __global skc_ttck_t* const ck_extent,
                 __global union skc_cmd_place const* const cmds,
                 __global skc_block_id_t* const map,
                 skc_uint4 const clip,
                 skc_uint const count)
{

  __local union skc_subgroup_smem volatile smem[1];

  skc_uint const cmd_idx = get_group_id(0);

  union skc_cmd_place const cmd = cmds[cmd_idx];

  skc_block_id_t id = map[cmd.raster_h];

  skc_uint const head_id =
    id * (1u << 3) + (((get_sub_group_local_id()) & ~(16 / (16 / 8) - 1)) * 2 +
                      ((get_sub_group_local_id()) & (16 / (16 / 8) - 1)));

  union skc_raster_node_elem const h0 = {
    .u32v2 = { bp_elems[head_id + (0 * 2 * 16)],
               bp_elems[head_id + ((0 * 2 * 16) + 16 / (16 / 8))] }
  };
  union skc_raster_node_elem const h1 = {
    .u32v2 = { bp_elems[head_id + (1 * 2 * 16)],
               bp_elems[head_id + ((1 * 2 * 16) + 16 / (16 / 8))] }
  };
  ;

  skc_uint nodes = sub_group_broadcast(h0.u32v2.lo, 1);
  skc_uint keys = sub_group_broadcast(h0.u32v2.hi, 1);

  {

    skc_uint bits_keys = 0;
    skc_uint bits_skpk = 0;

    if (!(((0) + 1) * 16 <= (8 / 2))) {
      skc_uint const idx = 0 * 16 + get_sub_group_local_id() - (8 / 2);
      if (idx < keys) {
        bits_keys |= (1u << 0);
      }
      if ((((0) + 1) * 16 == ((1u << 6) / 2))) {
        if (keys > ((((1u << 6) / 2) - 1) - (8 / 2))) {
          if (get_sub_group_local_id() == (16 - 1)) {
            bits_keys &= ~(1u << 0);
          }
        }
      }
    }
    if (!(((1) + 1) * 16 <= (8 / 2))) {
      skc_uint const idx = 1 * 16 + get_sub_group_local_id() - (8 / 2);
      if (idx < keys) {
        bits_keys |= (1u << 1);
      }
      if ((((1) + 1) * 16 == ((1u << 6) / 2))) {
        if (keys > ((((1u << 6) / 2) - 1) - (8 / 2))) {
          if (get_sub_group_local_id() == (16 - 1)) {
            bits_keys &= ~(1u << 1);
          }
        }
      }
    };

    if (!(((0) + 1) * 16 <= (8 / 2))) {
      bits_skpk |=
        (h0.xk.lo & ((((skc_uint)1 << (1)) - 1) << (27))) >> (27 - 0);
    }
    if (!(((1) + 1) * 16 <= (8 / 2))) {
      bits_skpk |=
        (h1.xk.lo & ((((skc_uint)1 << (1)) - 1) << (27))) >> (27 - 1);
    };

    id = sub_group_broadcast(h1.next.node, (16 - 1));

    skc_uint const bits_sk = bits_keys & ~bits_skpk;
    skc_uint sk = 0;

    if (!(((0) + 1) * 16 <= (8 / 2))) {
      skc_uint is_sk = (bits_sk >> 0) & 1;
      skc_uint sk_idx = skc_ballot(&sk, is_sk);
      if (is_sk) {
        smem->lo.sk[sk_idx] = h0.xk.lo;
        smem->hi.sk[sk_idx] = h0.xk.hi;
      }
    }
    if (!(((1) + 1) * 16 <= (8 / 2))) {
      skc_uint is_sk = (bits_sk >> 1) & 1;
      skc_uint sk_idx = skc_ballot(&sk, is_sk);
      if (is_sk) {
        smem->lo.sk[sk_idx] = h1.xk.lo;
        smem->hi.sk[sk_idx] = h1.xk.hi;
      }
    };

    skc_uint const bits_pk = bits_keys & bits_skpk;
    skc_uint pk = 0;

    if (!(((0) + 1) * 16 <= (8 / 2))) {
      skc_uint is_pk = (bits_pk >> 0) & 1;
      skc_uint pk_idx = skc_ballot(&pk, is_pk);
      if (is_pk) {
        smem->lo.pk[pk_idx] = h0.xk.lo;
        smem->hi.pk[pk_idx] = h0.xk.hi;
      }
    }
    if (!(((1) + 1) * 16 <= (8 / 2))) {
      skc_uint is_pk = (bits_pk >> 1) & 1;
      skc_uint pk_idx = skc_ballot(&pk, is_pk);
      if (is_pk) {
        smem->lo.pk[pk_idx] = h1.xk.lo;
        smem->hi.pk[pk_idx] = h1.xk.hi;
      }
    };

    skc_ttsk_flush(place_atomics, ck_extent, smem, &cmd, sk);
    skc_ttpk_flush(place_atomics, ck_extent, smem, &cmd, pk);
  }

  if (nodes == 0)
    return;

  keys -= ((((1u << 6) / 2) - 1) - (8 / 2));

  while (true) {

    skc_uint const node_id =
      id * (1u << 3) +
      (((get_sub_group_local_id()) & ~(16 / (16 / 8) - 1)) * 2 +
       ((get_sub_group_local_id()) & (16 / (16 / 8) - 1)));

    union skc_raster_node_elem const n0 = {
      .u32v2 = { bp_elems[node_id + (0 * 2 * 16)],
                 bp_elems[node_id + ((0 * 2 * 16) + 16 / (16 / 8))] }
    };
    union skc_raster_node_elem const n1 = {
      .u32v2 = { bp_elems[node_id + (1 * 2 * 16)],
                 bp_elems[node_id + ((1 * 2 * 16) + 16 / (16 / 8))] }
    };
    ;

    skc_uint bits_keys = 0;
    skc_uint bits_skpk = 0;

    {
      skc_uint const idx = 0 * 16 + get_sub_group_local_id();
      if (idx < keys) {
        bits_keys |= (1u << 0);
      }
      if ((((0) + 1) * 16 == ((1u << 6) / 2))) {
        if (keys > (((1u << 6) / 2) - 1)) {
          if (get_sub_group_local_id() == (16 - 1)) {
            bits_keys &= ~(1u << 0);
          }
        }
      }
    }
    {
      skc_uint const idx = 1 * 16 + get_sub_group_local_id();
      if (idx < keys) {
        bits_keys |= (1u << 1);
      }
      if ((((1) + 1) * 16 == ((1u << 6) / 2))) {
        if (keys > (((1u << 6) / 2) - 1)) {
          if (get_sub_group_local_id() == (16 - 1)) {
            bits_keys &= ~(1u << 1);
          }
        }
      }
    };

    {
      bits_skpk |=
        (n0.xk.lo & ((((skc_uint)1 << (1)) - 1) << (27))) >> (27 - 0);
    }
    {
      bits_skpk |=
        (n1.xk.lo & ((((skc_uint)1 << (1)) - 1) << (27))) >> (27 - 1);
    };

    id = sub_group_broadcast(n1.next.node, (16 - 1));

    skc_uint const bits_sk = bits_keys & ~bits_skpk;
    skc_uint sk = 0;

    {
      skc_uint is_sk = (bits_sk >> 0) & 1;
      skc_uint sk_idx = skc_ballot(&sk, is_sk);
      if (is_sk) {
        smem->lo.sk[sk_idx] = n0.xk.lo;
        smem->hi.sk[sk_idx] = n0.xk.hi;
      }
    }
    {
      skc_uint is_sk = (bits_sk >> 1) & 1;
      skc_uint sk_idx = skc_ballot(&sk, is_sk);
      if (is_sk) {
        smem->lo.sk[sk_idx] = n1.xk.lo;
        smem->hi.sk[sk_idx] = n1.xk.hi;
      }
    };

    skc_uint const bits_pk = bits_keys & bits_skpk;
    skc_uint pk = 0;

    {
      skc_uint is_pk = (bits_pk >> 0) & 1;
      skc_uint pk_idx = skc_ballot(&pk, is_pk);
      if (is_pk) {
        smem->lo.pk[pk_idx] = n0.xk.lo;
        smem->hi.pk[pk_idx] = n0.xk.hi;
      }
    }
    {
      skc_uint is_pk = (bits_pk >> 1) & 1;
      skc_uint pk_idx = skc_ballot(&pk, is_pk);
      if (is_pk) {
        smem->lo.pk[pk_idx] = n1.xk.lo;
        smem->hi.pk[pk_idx] = n1.xk.hi;
      }
    };

    skc_ttsk_flush(place_atomics, ck_extent, smem, &cmd, sk);
    skc_ttpk_flush(place_atomics, ck_extent, smem, &cmd, pk);

    if (--nodes == 0)
      return;

    keys -= (((1u << 6) / 2) - 1);
  }
}
