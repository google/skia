/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "skcms.h"
#include "skcms_internal.h"
#include <assert.h>
#include <float.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#if defined(__ARM_NEON)
    #include <arm_neon.h>
#elif defined(__SSE__)
    #include <immintrin.h>

    #if defined(__clang__)
        // That #include <immintrin.h> is usually enough, but Clang's headers
        // "helpfully" skip including the whole kitchen sink when _MSC_VER is
        // defined, because lots of programs on Windows would include that and
        // it'd be a lot slower.  But we want all those headers included so we
        // can use their features after runtime checks later.
        #include <smmintrin.h>
        #include <avxintrin.h>
        #include <avx2intrin.h>
        #include <avx512fintrin.h>
        #include <avx512dqintrin.h>
    #endif
#endif

// sizeof(x) will return size_t, which is 32-bit on some machines and 64-bit on others.
// We have better testing on 64-bit machines, so force 32-bit machines to behave like 64-bit.
//
// Please do not use sizeof() directly, and size_t only when required.
// (We have no way of enforcing these requests...)
#define SAFE_SIZEOF(x) ((uint64_t)sizeof(x))

// Same sort of thing for _Layout structs with a variable sized array at the end (named "variable").
#define SAFE_FIXED_SIZE(type) ((uint64_t)offsetof(type, variable))

static const union {
    uint32_t bits;
    float    f;
} inf_ = { 0x7f800000 };
#define INFINITY_ inf_.f

static float fmaxf_(float x, float y) { return x > y ? x : y; }
static float fminf_(float x, float y) { return x < y ? x : y; }

static bool isfinitef_(float x) { return 0 == x*0; }

static float minus_1_ulp(float x) {
    int32_t bits;
    memcpy(&bits, &x, sizeof(bits));
    bits = bits - 1;
    memcpy(&x, &bits, sizeof(bits));
    return x;
}

static float eval_curve(const skcms_Curve* curve, float x) {
    if (curve->table_entries == 0) {
        return skcms_TransferFunction_eval(&curve->parametric, x);
    }

    float ix = fmaxf_(0, fminf_(x, 1)) * (curve->table_entries - 1);
    int   lo = (int)                   ix        ,
          hi = (int)(float)minus_1_ulp(ix + 1.0f);
    float t = ix - (float)lo;

    float l, h;
    if (curve->table_8) {
        l = curve->table_8[lo] * (1/255.0f);
        h = curve->table_8[hi] * (1/255.0f);
    } else {
        uint16_t be_l, be_h;
        memcpy(&be_l, curve->table_16 + 2*lo, 2);
        memcpy(&be_h, curve->table_16 + 2*hi, 2);
        uint16_t le_l = ((be_l << 8) | (be_l >> 8)) & 0xffff;
        uint16_t le_h = ((be_h << 8) | (be_h >> 8)) & 0xffff;
        l = le_l * (1/65535.0f);
        h = le_h * (1/65535.0f);
    }
    return l + (h-l)*t;
}

static float max_roundtrip_error(const skcms_Curve* curve, const skcms_TransferFunction* inv_tf) {
    uint32_t N = curve->table_entries > 256 ? curve->table_entries : 256;
    const float dx = 1.0f / (N - 1);
    float err = 0;
    for (uint32_t i = 0; i < N; i++) {
        float x = i * dx,
              y = eval_curve(curve, x);
        err = fmaxf_(err, fabsf_(x - skcms_TransferFunction_eval(inv_tf, y)));
    }
    return err;
}

bool skcms_AreApproximateInverses(const skcms_Curve* curve, const skcms_TransferFunction* inv_tf) {
    return max_roundtrip_error(curve, inv_tf) < (1/512.0f);
}

// Additional ICC signature values that are only used internally
enum {
    // File signature
    skcms_Signature_acsp = 0x61637370,

    // Tag signatures
    skcms_Signature_rTRC = 0x72545243,
    skcms_Signature_gTRC = 0x67545243,
    skcms_Signature_bTRC = 0x62545243,
    skcms_Signature_kTRC = 0x6B545243,

    skcms_Signature_rXYZ = 0x7258595A,
    skcms_Signature_gXYZ = 0x6758595A,
    skcms_Signature_bXYZ = 0x6258595A,

    skcms_Signature_A2B0 = 0x41324230,
    skcms_Signature_A2B1 = 0x41324231,
    skcms_Signature_mAB  = 0x6D414220,

    skcms_Signature_CHAD = 0x63686164,

    // Type signatures
    skcms_Signature_curv = 0x63757276,
    skcms_Signature_mft1 = 0x6D667431,
    skcms_Signature_mft2 = 0x6D667432,
    skcms_Signature_para = 0x70617261,
    skcms_Signature_sf32 = 0x73663332,
    // XYZ is also a PCS signature, so it's defined in skcms.h
    // skcms_Signature_XYZ = 0x58595A20,
};

static uint16_t read_big_u16(const uint8_t* ptr) {
    uint16_t be;
    memcpy(&be, ptr, sizeof(be));
#if defined(_MSC_VER)
    return _byteswap_ushort(be);
#else
    return __builtin_bswap16(be);
#endif
}

static uint32_t read_big_u32(const uint8_t* ptr) {
    uint32_t be;
    memcpy(&be, ptr, sizeof(be));
#if defined(_MSC_VER)
    return _byteswap_ulong(be);
#else
    return __builtin_bswap32(be);
#endif
}

static int32_t read_big_i32(const uint8_t* ptr) {
    return (int32_t)read_big_u32(ptr);
}

static float read_big_fixed(const uint8_t* ptr) {
    return read_big_i32(ptr) * (1.0f / 65536.0f);
}

// Maps to an in-memory profile so that fields line up to the locations specified
// in ICC.1:2010, section 7.2
typedef struct {
    uint8_t size                [ 4];
    uint8_t cmm_type            [ 4];
    uint8_t version             [ 4];
    uint8_t profile_class       [ 4];
    uint8_t data_color_space    [ 4];
    uint8_t pcs                 [ 4];
    uint8_t creation_date_time  [12];
    uint8_t signature           [ 4];
    uint8_t platform            [ 4];
    uint8_t flags               [ 4];
    uint8_t device_manufacturer [ 4];
    uint8_t device_model        [ 4];
    uint8_t device_attributes   [ 8];
    uint8_t rendering_intent    [ 4];
    uint8_t illuminant_X        [ 4];
    uint8_t illuminant_Y        [ 4];
    uint8_t illuminant_Z        [ 4];
    uint8_t creator             [ 4];
    uint8_t profile_id          [16];
    uint8_t reserved            [28];
    uint8_t tag_count           [ 4]; // Technically not part of header, but required
} header_Layout;

typedef struct {
    uint8_t signature [4];
    uint8_t offset    [4];
    uint8_t size      [4];
} tag_Layout;

static const tag_Layout* get_tag_table(const skcms_ICCProfile* profile) {
    return (const tag_Layout*)(profile->buffer + SAFE_SIZEOF(header_Layout));
}

// s15Fixed16ArrayType is technically variable sized, holding N values. However, the only valid
// use of the type is for the CHAD tag that stores exactly nine values.
typedef struct {
    uint8_t type     [ 4];
    uint8_t reserved [ 4];
    uint8_t values   [36];
} sf32_Layout;

bool skcms_GetCHAD(const skcms_ICCProfile* profile, skcms_Matrix3x3* m) {
    skcms_ICCTag tag;
    if (!skcms_GetTagBySignature(profile, skcms_Signature_CHAD, &tag)) {
        return false;
    }

    if (tag.type != skcms_Signature_sf32 || tag.size < SAFE_SIZEOF(sf32_Layout)) {
        return false;
    }

    const sf32_Layout* sf32Tag = (const sf32_Layout*)tag.buf;
    const uint8_t* values = sf32Tag->values;
    for (int r = 0; r < 3; ++r)
    for (int c = 0; c < 3; ++c, values += 4) {
        m->vals[r][c] = read_big_fixed(values);
    }
    return true;
}

// XYZType is technically variable sized, holding N XYZ triples. However, the only valid uses of
// the type are for tags/data that store exactly one triple.
typedef struct {
    uint8_t type     [4];
    uint8_t reserved [4];
    uint8_t X        [4];
    uint8_t Y        [4];
    uint8_t Z        [4];
} XYZ_Layout;

static bool read_tag_xyz(const skcms_ICCTag* tag, float* x, float* y, float* z) {
    if (tag->type != skcms_Signature_XYZ || tag->size < SAFE_SIZEOF(XYZ_Layout)) {
        return false;
    }

    const XYZ_Layout* xyzTag = (const XYZ_Layout*)tag->buf;

    *x = read_big_fixed(xyzTag->X);
    *y = read_big_fixed(xyzTag->Y);
    *z = read_big_fixed(xyzTag->Z);
    return true;
}

static bool read_to_XYZD50(const skcms_ICCTag* rXYZ, const skcms_ICCTag* gXYZ,
                           const skcms_ICCTag* bXYZ, skcms_Matrix3x3* toXYZ) {
    return read_tag_xyz(rXYZ, &toXYZ->vals[0][0], &toXYZ->vals[1][0], &toXYZ->vals[2][0]) &&
           read_tag_xyz(gXYZ, &toXYZ->vals[0][1], &toXYZ->vals[1][1], &toXYZ->vals[2][1]) &&
           read_tag_xyz(bXYZ, &toXYZ->vals[0][2], &toXYZ->vals[1][2], &toXYZ->vals[2][2]);
}

static bool tf_is_valid(const skcms_TransferFunction* tf) {
    // Reject obviously malformed inputs
    if (!isfinitef_(tf->a + tf->b + tf->c + tf->d + tf->e + tf->f + tf->g)) {
        return false;
    }

    // All of these parameters should be non-negative
    if (tf->a < 0 || tf->c < 0 || tf->d < 0 || tf->g < 0) {
        return false;
    }

    return true;
}

typedef struct {
    uint8_t type          [4];
    uint8_t reserved_a    [4];
    uint8_t function_type [2];
    uint8_t reserved_b    [2];
    uint8_t variable      [1/*variable*/];  // 1, 3, 4, 5, or 7 s15.16, depending on function_type
} para_Layout;

static bool read_curve_para(const uint8_t* buf, uint32_t size,
                            skcms_Curve* curve, uint32_t* curve_size) {
    if (size < SAFE_FIXED_SIZE(para_Layout)) {
        return false;
    }

    const para_Layout* paraTag = (const para_Layout*)buf;

    enum { kG = 0, kGAB = 1, kGABC = 2, kGABCD = 3, kGABCDEF = 4 };
    uint16_t function_type = read_big_u16(paraTag->function_type);
    if (function_type > kGABCDEF) {
        return false;
    }

    static const uint32_t curve_bytes[] = { 4, 12, 16, 20, 28 };
    if (size < SAFE_FIXED_SIZE(para_Layout) + curve_bytes[function_type]) {
        return false;
    }

    if (curve_size) {
        *curve_size = SAFE_FIXED_SIZE(para_Layout) + curve_bytes[function_type];
    }

    curve->table_entries = 0;
    curve->parametric.a  = 1.0f;
    curve->parametric.b  = 0.0f;
    curve->parametric.c  = 0.0f;
    curve->parametric.d  = 0.0f;
    curve->parametric.e  = 0.0f;
    curve->parametric.f  = 0.0f;
    curve->parametric.g  = read_big_fixed(paraTag->variable);

    switch (function_type) {
        case kGAB:
            curve->parametric.a = read_big_fixed(paraTag->variable + 4);
            curve->parametric.b = read_big_fixed(paraTag->variable + 8);
            if (curve->parametric.a == 0) {
                return false;
            }
            curve->parametric.d = -curve->parametric.b / curve->parametric.a;
            break;
        case kGABC:
            curve->parametric.a = read_big_fixed(paraTag->variable + 4);
            curve->parametric.b = read_big_fixed(paraTag->variable + 8);
            curve->parametric.e = read_big_fixed(paraTag->variable + 12);
            if (curve->parametric.a == 0) {
                return false;
            }
            curve->parametric.d = -curve->parametric.b / curve->parametric.a;
            curve->parametric.f = curve->parametric.e;
            break;
        case kGABCD:
            curve->parametric.a = read_big_fixed(paraTag->variable + 4);
            curve->parametric.b = read_big_fixed(paraTag->variable + 8);
            curve->parametric.c = read_big_fixed(paraTag->variable + 12);
            curve->parametric.d = read_big_fixed(paraTag->variable + 16);
            break;
        case kGABCDEF:
            curve->parametric.a = read_big_fixed(paraTag->variable + 4);
            curve->parametric.b = read_big_fixed(paraTag->variable + 8);
            curve->parametric.c = read_big_fixed(paraTag->variable + 12);
            curve->parametric.d = read_big_fixed(paraTag->variable + 16);
            curve->parametric.e = read_big_fixed(paraTag->variable + 20);
            curve->parametric.f = read_big_fixed(paraTag->variable + 24);
            break;
    }
    return tf_is_valid(&curve->parametric);
}

typedef struct {
    uint8_t type          [4];
    uint8_t reserved      [4];
    uint8_t value_count   [4];
    uint8_t variable      [1/*variable*/];  // value_count, 8.8 if 1, uint16 (n*65535) if > 1
} curv_Layout;

static bool read_curve_curv(const uint8_t* buf, uint32_t size,
                            skcms_Curve* curve, uint32_t* curve_size) {
    if (size < SAFE_FIXED_SIZE(curv_Layout)) {
        return false;
    }

    const curv_Layout* curvTag = (const curv_Layout*)buf;

    uint32_t value_count = read_big_u32(curvTag->value_count);
    if (size < SAFE_FIXED_SIZE(curv_Layout) + value_count * SAFE_SIZEOF(uint16_t)) {
        return false;
    }

    if (curve_size) {
        *curve_size = SAFE_FIXED_SIZE(curv_Layout) + value_count * SAFE_SIZEOF(uint16_t);
    }

    if (value_count < 2) {
        curve->table_entries = 0;
        curve->parametric.a  = 1.0f;
        curve->parametric.b  = 0.0f;
        curve->parametric.c  = 0.0f;
        curve->parametric.d  = 0.0f;
        curve->parametric.e  = 0.0f;
        curve->parametric.f  = 0.0f;
        if (value_count == 0) {
            // Empty tables are a shorthand for an identity curve
            curve->parametric.g = 1.0f;
        } else {
            // Single entry tables are a shorthand for simple gamma
            curve->parametric.g = read_big_u16(curvTag->variable) * (1.0f / 256.0f);
        }
    } else {
        curve->table_8       = nullptr;
        curve->table_16      = curvTag->variable;
        curve->table_entries = value_count;
    }

    return true;
}

// Parses both curveType and parametricCurveType data. Ensures that at most 'size' bytes are read.
// If curve_size is not nullptr, writes the number of bytes used by the curve in (*curve_size).
static bool read_curve(const uint8_t* buf, uint32_t size,
                       skcms_Curve* curve, uint32_t* curve_size) {
    if (!buf || size < 4 || !curve) {
        return false;
    }

    uint32_t type = read_big_u32(buf);
    if (type == skcms_Signature_para) {
        return read_curve_para(buf, size, curve, curve_size);
    } else if (type == skcms_Signature_curv) {
        return read_curve_curv(buf, size, curve, curve_size);
    }

    return false;
}

// mft1 and mft2 share a large chunk of data
typedef struct {
    uint8_t type                 [ 4];
    uint8_t reserved_a           [ 4];
    uint8_t input_channels       [ 1];
    uint8_t output_channels      [ 1];
    uint8_t grid_points          [ 1];
    uint8_t reserved_b           [ 1];
    uint8_t matrix               [36];
} mft_CommonLayout;

typedef struct {
    mft_CommonLayout common      [1];

    uint8_t variable             [1/*variable*/];
} mft1_Layout;

typedef struct {
    mft_CommonLayout common      [1];

    uint8_t input_table_entries  [2];
    uint8_t output_table_entries [2];
    uint8_t variable             [1/*variable*/];
} mft2_Layout;

static bool read_mft_common(const mft_CommonLayout* mftTag, skcms_A2B* a2b) {
    // MFT matrices are applied before the first set of curves, but must be identity unless the
    // input is PCSXYZ. We don't support PCSXYZ profiles, so we ignore this matrix. Note that the
    // matrix in skcms_A2B is applied later in the pipe, so supporting this would require another
    // field/flag.
    a2b->matrix_channels = 0;

    a2b->input_channels  = mftTag->input_channels[0];
    a2b->output_channels = mftTag->output_channels[0];

    // We require exactly three (ie XYZ/Lab/RGB) output channels
    if (a2b->output_channels != ARRAY_COUNT(a2b->output_curves)) {
        return false;
    }
    // We require at least one, and no more than four (ie CMYK) input channels
    if (a2b->input_channels < 1 || a2b->input_channels > ARRAY_COUNT(a2b->input_curves)) {
        return false;
    }

    for (uint32_t i = 0; i < a2b->input_channels; ++i) {
        a2b->grid_points[i] = mftTag->grid_points[0];
    }
    // The grid only makes sense with at least two points along each axis
    if (a2b->grid_points[0] < 2) {
        return false;
    }

    return true;
}

static bool init_a2b_tables(const uint8_t* table_base, uint64_t max_tables_len, uint32_t byte_width,
                            uint32_t input_table_entries, uint32_t output_table_entries,
                            skcms_A2B* a2b) {
    // byte_width is 1 or 2, [input|output]_table_entries are in [2, 4096], so no overflow
    uint32_t byte_len_per_input_table  = input_table_entries * byte_width;
    uint32_t byte_len_per_output_table = output_table_entries * byte_width;

    // [input|output]_channels are <= 4, so still no overflow
    uint32_t byte_len_all_input_tables  = a2b->input_channels * byte_len_per_input_table;
    uint32_t byte_len_all_output_tables = a2b->output_channels * byte_len_per_output_table;

    uint64_t grid_size = a2b->output_channels * byte_width;
    for (uint32_t axis = 0; axis < a2b->input_channels; ++axis) {
        grid_size *= a2b->grid_points[axis];
    }

    if (max_tables_len < byte_len_all_input_tables + grid_size + byte_len_all_output_tables) {
        return false;
    }

    for (uint32_t i = 0; i < a2b->input_channels; ++i) {
        a2b->input_curves[i].table_entries = input_table_entries;
        if (byte_width == 1) {
            a2b->input_curves[i].table_8  = table_base + i * byte_len_per_input_table;
            a2b->input_curves[i].table_16 = nullptr;
        } else {
            a2b->input_curves[i].table_8  = nullptr;
            a2b->input_curves[i].table_16 = table_base + i * byte_len_per_input_table;
        }
    }

    if (byte_width == 1) {
        a2b->grid_8  = table_base + byte_len_all_input_tables;
        a2b->grid_16 = nullptr;
    } else {
        a2b->grid_8  = nullptr;
        a2b->grid_16 = table_base + byte_len_all_input_tables;
    }

    const uint8_t* output_table_base = table_base + byte_len_all_input_tables + grid_size;
    for (uint32_t i = 0; i < a2b->output_channels; ++i) {
        a2b->output_curves[i].table_entries = output_table_entries;
        if (byte_width == 1) {
            a2b->output_curves[i].table_8  = output_table_base + i * byte_len_per_output_table;
            a2b->output_curves[i].table_16 = nullptr;
        } else {
            a2b->output_curves[i].table_8  = nullptr;
            a2b->output_curves[i].table_16 = output_table_base + i * byte_len_per_output_table;
        }
    }

    return true;
}

static bool read_tag_mft1(const skcms_ICCTag* tag, skcms_A2B* a2b) {
    if (tag->size < SAFE_FIXED_SIZE(mft1_Layout)) {
        return false;
    }

    const mft1_Layout* mftTag = (const mft1_Layout*)tag->buf;
    if (!read_mft_common(mftTag->common, a2b)) {
        return false;
    }

    uint32_t input_table_entries  = 256;
    uint32_t output_table_entries = 256;

    return init_a2b_tables(mftTag->variable, tag->size - SAFE_FIXED_SIZE(mft1_Layout), 1,
                           input_table_entries, output_table_entries, a2b);
}

static bool read_tag_mft2(const skcms_ICCTag* tag, skcms_A2B* a2b) {
    if (tag->size < SAFE_FIXED_SIZE(mft2_Layout)) {
        return false;
    }

    const mft2_Layout* mftTag = (const mft2_Layout*)tag->buf;
    if (!read_mft_common(mftTag->common, a2b)) {
        return false;
    }

    uint32_t input_table_entries = read_big_u16(mftTag->input_table_entries);
    uint32_t output_table_entries = read_big_u16(mftTag->output_table_entries);

    // ICC spec mandates that 2 <= table_entries <= 4096
    if (input_table_entries < 2 || input_table_entries > 4096 ||
        output_table_entries < 2 || output_table_entries > 4096) {
        return false;
    }

    return init_a2b_tables(mftTag->variable, tag->size - SAFE_FIXED_SIZE(mft2_Layout), 2,
                           input_table_entries, output_table_entries, a2b);
}

static bool read_curves(const uint8_t* buf, uint32_t size, uint32_t curve_offset,
                        uint32_t num_curves, skcms_Curve* curves) {
    for (uint32_t i = 0; i < num_curves; ++i) {
        if (curve_offset > size) {
            return false;
        }

        uint32_t curve_bytes;
        if (!read_curve(buf + curve_offset, size - curve_offset, &curves[i], &curve_bytes)) {
            return false;
        }

        if (curve_bytes > UINT32_MAX - 3) {
            return false;
        }
        curve_bytes = (curve_bytes + 3) & ~3U;

        uint64_t new_offset_64 = (uint64_t)curve_offset + curve_bytes;
        curve_offset = (uint32_t)new_offset_64;
        if (new_offset_64 != curve_offset) {
            return false;
        }
    }

    return true;
}

typedef struct {
    uint8_t type                 [ 4];
    uint8_t reserved_a           [ 4];
    uint8_t input_channels       [ 1];
    uint8_t output_channels      [ 1];
    uint8_t reserved_b           [ 2];
    uint8_t b_curve_offset       [ 4];
    uint8_t matrix_offset        [ 4];
    uint8_t m_curve_offset       [ 4];
    uint8_t clut_offset          [ 4];
    uint8_t a_curve_offset       [ 4];
} mAB_Layout;

typedef struct {
    uint8_t grid_points          [16];
    uint8_t grid_byte_width      [ 1];
    uint8_t reserved             [ 3];
    uint8_t variable             [1/*variable*/];
} mABCLUT_Layout;

static bool read_tag_mab(const skcms_ICCTag* tag, skcms_A2B* a2b, bool pcs_is_xyz) {
    if (tag->size < SAFE_SIZEOF(mAB_Layout)) {
        return false;
    }

    const mAB_Layout* mABTag = (const mAB_Layout*)tag->buf;

    a2b->input_channels  = mABTag->input_channels[0];
    a2b->output_channels = mABTag->output_channels[0];

    // We require exactly three (ie XYZ/Lab/RGB) output channels
    if (a2b->output_channels != ARRAY_COUNT(a2b->output_curves)) {
        return false;
    }
    // We require no more than four (ie CMYK) input channels
    if (a2b->input_channels > ARRAY_COUNT(a2b->input_curves)) {
        return false;
    }

    uint32_t b_curve_offset = read_big_u32(mABTag->b_curve_offset);
    uint32_t matrix_offset  = read_big_u32(mABTag->matrix_offset);
    uint32_t m_curve_offset = read_big_u32(mABTag->m_curve_offset);
    uint32_t clut_offset    = read_big_u32(mABTag->clut_offset);
    uint32_t a_curve_offset = read_big_u32(mABTag->a_curve_offset);

    // "B" curves must be present
    if (0 == b_curve_offset) {
        return false;
    }

    if (!read_curves(tag->buf, tag->size, b_curve_offset, a2b->output_channels,
                     a2b->output_curves)) {
        return false;
    }

    // "M" curves and Matrix must be used together
    if (0 != m_curve_offset) {
        if (0 == matrix_offset) {
            return false;
        }
        a2b->matrix_channels = a2b->output_channels;
        if (!read_curves(tag->buf, tag->size, m_curve_offset, a2b->matrix_channels,
                         a2b->matrix_curves)) {
            return false;
        }

        // Read matrix, which is stored as a row-major 3x3, followed by the fourth column
        if (tag->size < matrix_offset + 12 * SAFE_SIZEOF(uint32_t)) {
            return false;
        }
        float encoding_factor = pcs_is_xyz ? 65535 / 32768.0f : 1.0f;
        const uint8_t* mtx_buf = tag->buf + matrix_offset;
        a2b->matrix.vals[0][0] = encoding_factor * read_big_fixed(mtx_buf + 0);
        a2b->matrix.vals[0][1] = encoding_factor * read_big_fixed(mtx_buf + 4);
        a2b->matrix.vals[0][2] = encoding_factor * read_big_fixed(mtx_buf + 8);
        a2b->matrix.vals[1][0] = encoding_factor * read_big_fixed(mtx_buf + 12);
        a2b->matrix.vals[1][1] = encoding_factor * read_big_fixed(mtx_buf + 16);
        a2b->matrix.vals[1][2] = encoding_factor * read_big_fixed(mtx_buf + 20);
        a2b->matrix.vals[2][0] = encoding_factor * read_big_fixed(mtx_buf + 24);
        a2b->matrix.vals[2][1] = encoding_factor * read_big_fixed(mtx_buf + 28);
        a2b->matrix.vals[2][2] = encoding_factor * read_big_fixed(mtx_buf + 32);
        a2b->matrix.vals[0][3] = encoding_factor * read_big_fixed(mtx_buf + 36);
        a2b->matrix.vals[1][3] = encoding_factor * read_big_fixed(mtx_buf + 40);
        a2b->matrix.vals[2][3] = encoding_factor * read_big_fixed(mtx_buf + 44);
    } else {
        if (0 != matrix_offset) {
            return false;
        }
        a2b->matrix_channels = 0;
    }

    // "A" curves and CLUT must be used together
    if (0 != a_curve_offset) {
        if (0 == clut_offset) {
            return false;
        }
        if (!read_curves(tag->buf, tag->size, a_curve_offset, a2b->input_channels,
                         a2b->input_curves)) {
            return false;
        }

        if (tag->size < clut_offset + SAFE_FIXED_SIZE(mABCLUT_Layout)) {
            return false;
        }
        const mABCLUT_Layout* clut = (const mABCLUT_Layout*)(tag->buf + clut_offset);

        if (clut->grid_byte_width[0] == 1) {
            a2b->grid_8  = clut->variable;
            a2b->grid_16 = nullptr;
        } else if (clut->grid_byte_width[0] == 2) {
            a2b->grid_8  = nullptr;
            a2b->grid_16 = clut->variable;
        } else {
            return false;
        }

        uint64_t grid_size = a2b->output_channels * clut->grid_byte_width[0];
        for (uint32_t i = 0; i < a2b->input_channels; ++i) {
            a2b->grid_points[i] = clut->grid_points[i];
            // The grid only makes sense with at least two points along each axis
            if (a2b->grid_points[i] < 2) {
                return false;
            }
            grid_size *= a2b->grid_points[i];
        }
        if (tag->size < clut_offset + SAFE_FIXED_SIZE(mABCLUT_Layout) + grid_size) {
            return false;
        }
    } else {
        if (0 != clut_offset) {
            return false;
        }

        // If there is no CLUT, the number of input and output channels must match
        if (a2b->input_channels != a2b->output_channels) {
            return false;
        }

        // Zero out the number of input channels to signal that we're skipping this stage
        a2b->input_channels = 0;
    }

    return true;
}

// If you pass f, we'll fit a possibly-non-zero value for *f.
// If you pass nullptr, we'll assume you want *f to be treated as zero.
static int fit_linear(const skcms_Curve* curve, int N, float tol,
                      float* c, float* d, float* f = nullptr) {
    assert(N > 1);
    // We iteratively fit the first points to the TF's linear piece.
    // We want the cx + f line to pass through the first and last points we fit exactly.
    //
    // As we walk along the points we find the minimum and maximum slope of the line before the
    // error would exceed our tolerance.  We stop when the range [slope_min, slope_max] becomes
    // emtpy, when we definitely can't add any more points.
    //
    // Some points' error intervals may intersect the running interval but not lie fully
    // within it.  So we keep track of the last point we saw that is a valid end point candidate,
    // and once the search is done, back up to build the line through *that* point.
    const float dx = 1.0f / (N - 1);

    int lin_points = 1;

    float f_zero = 0.0f;
    if (f) {
        *f = eval_curve(curve, 0);
    } else {
        f = &f_zero;
    }


    float slope_min = -INFINITY_;
    float slope_max = +INFINITY_;
    for (int i = 1; i < N; ++i) {
        float x = i * dx;
        float y = eval_curve(curve, x);

        float slope_max_i = (y + tol - *f) / x,
              slope_min_i = (y - tol - *f) / x;
        if (slope_max_i < slope_min || slope_max < slope_min_i) {
            // Slope intervals would no longer overlap.
            break;
        }
        slope_max = fminf_(slope_max, slope_max_i);
        slope_min = fmaxf_(slope_min, slope_min_i);

        float cur_slope = (y - *f) / x;
        if (slope_min <= cur_slope && cur_slope <= slope_max) {
            lin_points = i + 1;
            *c = cur_slope;
        }
    }

    // Set D to the last point that met our tolerance.
    *d = (lin_points - 1) * dx;
    return lin_points;
}

static bool read_a2b(const skcms_ICCTag* tag, skcms_A2B* a2b, bool pcs_is_xyz) {
    bool ok = false;
    if (tag->type == skcms_Signature_mft1) {
        ok = read_tag_mft1(tag, a2b);
    } else if (tag->type == skcms_Signature_mft2) {
        ok = read_tag_mft2(tag, a2b);
    } else if (tag->type == skcms_Signature_mAB) {
        ok = read_tag_mab(tag, a2b, pcs_is_xyz);
    }
    if (!ok) {
        return false;
    }

    // Detect and canonicalize identity tables.
    skcms_Curve* curves[] = {
        a2b->input_channels  > 0 ? a2b->input_curves  + 0 : nullptr,
        a2b->input_channels  > 1 ? a2b->input_curves  + 1 : nullptr,
        a2b->input_channels  > 2 ? a2b->input_curves  + 2 : nullptr,
        a2b->input_channels  > 3 ? a2b->input_curves  + 3 : nullptr,
        a2b->matrix_channels > 0 ? a2b->matrix_curves + 0 : nullptr,
        a2b->matrix_channels > 1 ? a2b->matrix_curves + 1 : nullptr,
        a2b->matrix_channels > 2 ? a2b->matrix_curves + 2 : nullptr,
        a2b->output_channels > 0 ? a2b->output_curves + 0 : nullptr,
        a2b->output_channels > 1 ? a2b->output_curves + 1 : nullptr,
        a2b->output_channels > 2 ? a2b->output_curves + 2 : nullptr,
    };

    for (int i = 0; i < ARRAY_COUNT(curves); i++) {
        skcms_Curve* curve = curves[i];

        if (curve && curve->table_entries && curve->table_entries <= (uint32_t)INT_MAX) {
            int N = (int)curve->table_entries;

            float c = 0.0f, d = 0.0f, f = 0.0f;
            if (N == fit_linear(curve, N, 1.0f/(2*N), &c,&d,&f)
                && c == 1.0f
                && f == 0.0f) {
                curve->table_entries = 0;
                curve->table_8       = nullptr;
                curve->table_16      = nullptr;
                curve->parametric    = skcms_TransferFunction{1,1,0,0,0,0,0};
            }
        }
    }

    return true;
}

void skcms_GetTagByIndex(const skcms_ICCProfile* profile, uint32_t idx, skcms_ICCTag* tag) {
    if (!profile || !profile->buffer || !tag) { return; }
    if (idx > profile->tag_count) { return; }
    const tag_Layout* tags = get_tag_table(profile);
    tag->signature = read_big_u32(tags[idx].signature);
    tag->size      = read_big_u32(tags[idx].size);
    tag->buf       = read_big_u32(tags[idx].offset) + profile->buffer;
    tag->type      = read_big_u32(tag->buf);
}

bool skcms_GetTagBySignature(const skcms_ICCProfile* profile, uint32_t sig, skcms_ICCTag* tag) {
    if (!profile || !profile->buffer || !tag) { return false; }
    const tag_Layout* tags = get_tag_table(profile);
    for (uint32_t i = 0; i < profile->tag_count; ++i) {
        if (read_big_u32(tags[i].signature) == sig) {
            tag->signature = sig;
            tag->size      = read_big_u32(tags[i].size);
            tag->buf       = read_big_u32(tags[i].offset) + profile->buffer;
            tag->type      = read_big_u32(tag->buf);
            return true;
        }
    }
    return false;
}

static bool usable_as_src(const skcms_ICCProfile* profile) {
    return profile->has_A2B
       || (profile->has_trc && profile->has_toXYZD50);
}

bool skcms_Parse(const void* buf, size_t len, skcms_ICCProfile* profile) {
    assert(SAFE_SIZEOF(header_Layout) == 132);

    if (!profile) {
        return false;
    }
    memset(profile, 0, SAFE_SIZEOF(*profile));

    if (len < SAFE_SIZEOF(header_Layout)) {
        return false;
    }

    // Byte-swap all header fields
    const header_Layout* header  = (const header_Layout*)buf;
    profile->buffer              = (const uint8_t*)buf;
    profile->size                = read_big_u32(header->size);
    uint32_t version             = read_big_u32(header->version);
    profile->data_color_space    = read_big_u32(header->data_color_space);
    profile->pcs                 = read_big_u32(header->pcs);
    uint32_t signature           = read_big_u32(header->signature);
    float illuminant_X           = read_big_fixed(header->illuminant_X);
    float illuminant_Y           = read_big_fixed(header->illuminant_Y);
    float illuminant_Z           = read_big_fixed(header->illuminant_Z);
    profile->tag_count           = read_big_u32(header->tag_count);

    // Validate signature, size (smaller than buffer, large enough to hold tag table),
    // and major version
    uint64_t tag_table_size = profile->tag_count * SAFE_SIZEOF(tag_Layout);
    if (signature != skcms_Signature_acsp ||
        profile->size > len ||
        profile->size < SAFE_SIZEOF(header_Layout) + tag_table_size ||
        (version >> 24) > 4) {
        return false;
    }

    // Validate that illuminant is D50 white
    if (fabsf_(illuminant_X - 0.9642f) > 0.0100f ||
        fabsf_(illuminant_Y - 1.0000f) > 0.0100f ||
        fabsf_(illuminant_Z - 0.8249f) > 0.0100f) {
        return false;
    }

    // Validate that all tag entries have sane offset + size
    const tag_Layout* tags = get_tag_table(profile);
    for (uint32_t i = 0; i < profile->tag_count; ++i) {
        uint32_t tag_offset = read_big_u32(tags[i].offset);
        uint32_t tag_size   = read_big_u32(tags[i].size);
        uint64_t tag_end    = (uint64_t)tag_offset + (uint64_t)tag_size;
        if (tag_size < 4 || tag_end > profile->size) {
            return false;
        }
    }

    if (profile->pcs != skcms_Signature_XYZ && profile->pcs != skcms_Signature_Lab) {
        return false;
    }

    bool pcs_is_xyz = profile->pcs == skcms_Signature_XYZ;

    // Pre-parse commonly used tags.
    skcms_ICCTag kTRC;
    if (profile->data_color_space == skcms_Signature_Gray &&
        skcms_GetTagBySignature(profile, skcms_Signature_kTRC, &kTRC)) {
        if (!read_curve(kTRC.buf, kTRC.size, &profile->trc[0], nullptr)) {
            // Malformed tag
            return false;
        }
        profile->trc[1] = profile->trc[0];
        profile->trc[2] = profile->trc[0];
        profile->has_trc = true;

        if (pcs_is_xyz) {
            profile->toXYZD50.vals[0][0] = illuminant_X;
            profile->toXYZD50.vals[1][1] = illuminant_Y;
            profile->toXYZD50.vals[2][2] = illuminant_Z;
            profile->has_toXYZD50 = true;
        }
    } else {
        skcms_ICCTag rTRC, gTRC, bTRC;
        if (skcms_GetTagBySignature(profile, skcms_Signature_rTRC, &rTRC) &&
            skcms_GetTagBySignature(profile, skcms_Signature_gTRC, &gTRC) &&
            skcms_GetTagBySignature(profile, skcms_Signature_bTRC, &bTRC)) {
            if (!read_curve(rTRC.buf, rTRC.size, &profile->trc[0], nullptr) ||
                !read_curve(gTRC.buf, gTRC.size, &profile->trc[1], nullptr) ||
                !read_curve(bTRC.buf, bTRC.size, &profile->trc[2], nullptr)) {
                // Malformed TRC tags
                return false;
            }
            profile->has_trc = true;
        }

        skcms_ICCTag rXYZ, gXYZ, bXYZ;
        if (skcms_GetTagBySignature(profile, skcms_Signature_rXYZ, &rXYZ) &&
            skcms_GetTagBySignature(profile, skcms_Signature_gXYZ, &gXYZ) &&
            skcms_GetTagBySignature(profile, skcms_Signature_bXYZ, &bXYZ)) {
            if (!read_to_XYZD50(&rXYZ, &gXYZ, &bXYZ, &profile->toXYZD50)) {
                // Malformed XYZ tags
                return false;
            }
            profile->has_toXYZD50 = true;
        }
    }

    skcms_ICCTag a2b_tag;

    // For now, we're preferring A2B0, like Skia does and the ICC spec tells us to.
    // TODO: prefer A2B1 (relative colormetric) over A2B0 (perceptual)?
    // This breaks with the ICC spec, but we think it's a good idea, given that TRC curves
    // and all our known users are thinking exclusively in terms of relative colormetric.
    const uint32_t sigs[] = { skcms_Signature_A2B0, skcms_Signature_A2B1 };
    for (int i = 0; i < ARRAY_COUNT(sigs); i++) {
        if (skcms_GetTagBySignature(profile, sigs[i], &a2b_tag)) {
            if (!read_a2b(&a2b_tag, &profile->A2B, pcs_is_xyz)) {
                // Malformed A2B tag
                return false;
            }
            profile->has_A2B = true;
            break;
        }
    }

    return usable_as_src(profile);
}


const skcms_ICCProfile* skcms_sRGB_profile() {
    static const skcms_ICCProfile sRGB_profile = {
        nullptr,               // buffer, moot here

        0,                     // size, moot here
        skcms_Signature_RGB,   // data_color_space
        skcms_Signature_XYZ,   // pcs
        0,                     // tag count, moot here

        // We choose to represent sRGB with its canonical transfer function,
        // and with its canonical XYZD50 gamut matrix.
        true,  // has_trc, followed by the 3 trc curves
        {
            {{0, {2.4f, (float)(1/1.055), (float)(0.055/1.055), (float)(1/12.92), 0.04045f, 0, 0}}},
            {{0, {2.4f, (float)(1/1.055), (float)(0.055/1.055), (float)(1/12.92), 0.04045f, 0, 0}}},
            {{0, {2.4f, (float)(1/1.055), (float)(0.055/1.055), (float)(1/12.92), 0.04045f, 0, 0}}},
        },

        true,  // has_toXYZD50, followed by 3x3 toXYZD50 matrix
        {{
            { 0.436065674f, 0.385147095f, 0.143066406f },
            { 0.222488403f, 0.716873169f, 0.060607910f },
            { 0.013916016f, 0.097076416f, 0.714096069f },
        }},

        false, // has_A2B, followed by a2b itself which we don't care about.
        {
            0,
            {
                {{0, {0,0, 0,0,0,0,0}}},
                {{0, {0,0, 0,0,0,0,0}}},
                {{0, {0,0, 0,0,0,0,0}}},
                {{0, {0,0, 0,0,0,0,0}}},
            },
            {0,0,0,0},
            nullptr,
            nullptr,

            0,
            {
                {{0, {0,0, 0,0,0,0,0}}},
                {{0, {0,0, 0,0,0,0,0}}},
                {{0, {0,0, 0,0,0,0,0}}},
            },
            {{
                { 0,0,0,0 },
                { 0,0,0,0 },
                { 0,0,0,0 },
            }},

            0,
            {
                {{0, {0,0, 0,0,0,0,0}}},
                {{0, {0,0, 0,0,0,0,0}}},
                {{0, {0,0, 0,0,0,0,0}}},
            },
        },
    };
    return &sRGB_profile;
}

const skcms_ICCProfile* skcms_XYZD50_profile() {
    // Just like sRGB above, but with identity transfer functions and toXYZD50 matrix.
    static const skcms_ICCProfile XYZD50_profile = {
        nullptr,               // buffer, moot here

        0,                     // size, moot here
        skcms_Signature_RGB,   // data_color_space
        skcms_Signature_XYZ,   // pcs
        0,                     // tag count, moot here

        true,  // has_trc, followed by the 3 trc curves
        {
            {{0, {1,1, 0,0,0,0,0}}},
            {{0, {1,1, 0,0,0,0,0}}},
            {{0, {1,1, 0,0,0,0,0}}},
        },

        true,  // has_toXYZD50, followed by 3x3 toXYZD50 matrix
        {{
            { 1,0,0 },
            { 0,1,0 },
            { 0,0,1 },
        }},

        false, // has_A2B, followed by a2b itself which we don't care about.
        {
            0,
            {
                {{0, {0,0, 0,0,0,0,0}}},
                {{0, {0,0, 0,0,0,0,0}}},
                {{0, {0,0, 0,0,0,0,0}}},
                {{0, {0,0, 0,0,0,0,0}}},
            },
            {0,0,0,0},
            nullptr,
            nullptr,

            0,
            {
                {{0, {0,0, 0,0,0,0,0}}},
                {{0, {0,0, 0,0,0,0,0}}},
                {{0, {0,0, 0,0,0,0,0}}},
            },
            {{
                { 0,0,0,0 },
                { 0,0,0,0 },
                { 0,0,0,0 },
            }},

            0,
            {
                {{0, {0,0, 0,0,0,0,0}}},
                {{0, {0,0, 0,0,0,0,0}}},
                {{0, {0,0, 0,0,0,0,0}}},
            },
        },
    };

    return &XYZD50_profile;
}

const skcms_TransferFunction* skcms_sRGB_TransferFunction() {
    return &skcms_sRGB_profile()->trc[0].parametric;
}

const skcms_TransferFunction* skcms_sRGB_Inverse_TransferFunction() {
    static const skcms_TransferFunction sRGB_inv =
        {0.416666657f, 1.137283325f, -0.0f, 12.920000076f, 0.003130805f, -0.054969788f, -0.0f};
    return &sRGB_inv;
}

const skcms_TransferFunction* skcms_Identity_TransferFunction() {
    static const skcms_TransferFunction identity = {1,1,0,0,0,0,0};
    return &identity;
}

const uint8_t skcms_252_random_bytes[] = {
    8, 179, 128, 204, 253, 38, 134, 184, 68, 102, 32, 138, 99, 39, 169, 215,
    119, 26, 3, 223, 95, 239, 52, 132, 114, 74, 81, 234, 97, 116, 244, 205, 30,
    154, 173, 12, 51, 159, 122, 153, 61, 226, 236, 178, 229, 55, 181, 220, 191,
    194, 160, 126, 168, 82, 131, 18, 180, 245, 163, 22, 246, 69, 235, 252, 57,
    108, 14, 6, 152, 240, 255, 171, 242, 20, 227, 177, 238, 96, 85, 16, 211,
    70, 200, 149, 155, 146, 127, 145, 100, 151, 109, 19, 165, 208, 195, 164,
    137, 254, 182, 248, 64, 201, 45, 209, 5, 147, 207, 210, 113, 162, 83, 225,
    9, 31, 15, 231, 115, 37, 58, 53, 24, 49, 197, 56, 120, 172, 48, 21, 214,
    129, 111, 11, 50, 187, 196, 34, 60, 103, 71, 144, 47, 203, 77, 80, 232,
    140, 222, 250, 206, 166, 247, 139, 249, 221, 72, 106, 27, 199, 117, 54,
    219, 135, 118, 40, 79, 41, 251, 46, 93, 212, 92, 233, 148, 28, 121, 63,
    123, 158, 105, 59, 29, 42, 143, 23, 0, 107, 176, 87, 104, 183, 156, 193,
    189, 90, 188, 65, 190, 17, 198, 7, 186, 161, 1, 124, 78, 125, 170, 133,
    174, 218, 67, 157, 75, 101, 89, 217, 62, 33, 141, 228, 25, 35, 91, 230, 4,
    2, 13, 73, 86, 167, 237, 84, 243, 44, 185, 66, 130, 110, 150, 142, 216, 88,
    112, 36, 224, 136, 202, 76, 94, 98, 175, 213
};

bool skcms_ApproximatelyEqualProfiles(const skcms_ICCProfile* A, const skcms_ICCProfile* B) {
    // Test for exactly equal profiles first.
    if (A == B || 0 == memcmp(A,B, sizeof(skcms_ICCProfile))) {
        return true;
    }

    // For now this is the essentially the same strategy we use in test_only.c
    // for our skcms_Transform() smoke tests:
    //    1) transform A to XYZD50
    //    2) transform B to XYZD50
    //    3) return true if they're similar enough
    // Our current criterion in 3) is maximum 1 bit error per XYZD50 byte.

    // skcms_252_random_bytes are 252 of a random shuffle of all possible bytes.
    // 252 is evenly divisible by 3 and 4.  Only 192, 10, 241, and 43 are missing.

    if (A->data_color_space != B->data_color_space) {
        return false;
    }

    // Interpret as RGB_888 if data color space is RGB or GRAY, RGBA_8888 if CMYK.
    // TODO: working with RGBA_8888 either way is probably fastest.
    skcms_PixelFormat fmt = skcms_PixelFormat_RGB_888;
    size_t npixels = 84;
    if (A->data_color_space == skcms_Signature_CMYK) {
        fmt = skcms_PixelFormat_RGBA_8888;
        npixels = 63;
    }

    // TODO: if A or B is a known profile (skcms_sRGB_profile, skcms_XYZD50_profile),
    // use pre-canned results and skip that skcms_Transform() call?
    uint8_t dstA[252],
            dstB[252];
    if (!skcms_Transform(
                skcms_252_random_bytes,     fmt, skcms_AlphaFormat_Unpremul, A,
                dstA, skcms_PixelFormat_RGB_888, skcms_AlphaFormat_Unpremul, skcms_XYZD50_profile(),
                npixels)) {
        return false;
    }
    if (!skcms_Transform(
                skcms_252_random_bytes,     fmt, skcms_AlphaFormat_Unpremul, B,
                dstB, skcms_PixelFormat_RGB_888, skcms_AlphaFormat_Unpremul, skcms_XYZD50_profile(),
                npixels)) {
        return false;
    }

    // TODO: make sure this final check has reasonable codegen.
    for (size_t i = 0; i < 252; i++) {
        if (abs((int)dstA[i] - (int)dstB[i]) > 1) {
            return false;
        }
    }
    return true;
}

bool skcms_TRCs_AreApproximateInverse(const skcms_ICCProfile* profile,
                                      const skcms_TransferFunction* inv_tf) {
    if (!profile || !profile->has_trc) {
        return false;
    }

    return skcms_AreApproximateInverses(&profile->trc[0], inv_tf) &&
           skcms_AreApproximateInverses(&profile->trc[1], inv_tf) &&
           skcms_AreApproximateInverses(&profile->trc[2], inv_tf);
}

static bool is_zero_to_one(float x) {
    return 0 <= x && x <= 1;
}

typedef struct { float vals[3]; } skcms_Vector3;

static skcms_Vector3 mv_mul(const skcms_Matrix3x3* m, const skcms_Vector3* v) {
    skcms_Vector3 dst = {{0,0,0}};
    for (int row = 0; row < 3; ++row) {
        dst.vals[row] = m->vals[row][0] * v->vals[0]
                      + m->vals[row][1] * v->vals[1]
                      + m->vals[row][2] * v->vals[2];
    }
    return dst;
}

bool skcms_PrimariesToXYZD50(float rx, float ry,
                             float gx, float gy,
                             float bx, float by,
                             float wx, float wy,
                             skcms_Matrix3x3* toXYZD50) {
    if (!is_zero_to_one(rx) || !is_zero_to_one(ry) ||
        !is_zero_to_one(gx) || !is_zero_to_one(gy) ||
        !is_zero_to_one(bx) || !is_zero_to_one(by) ||
        !is_zero_to_one(wx) || !is_zero_to_one(wy) ||
        !toXYZD50) {
        return false;
    }

    // First, we need to convert xy values (primaries) to XYZ.
    skcms_Matrix3x3 primaries = {{
        { rx, gx, bx },
        { ry, gy, by },
        { 1 - rx - ry, 1 - gx - gy, 1 - bx - by },
    }};
    skcms_Matrix3x3 primaries_inv;
    if (!skcms_Matrix3x3_invert(&primaries, &primaries_inv)) {
        return false;
    }

    // Assumes that Y is 1.0f.
    skcms_Vector3 wXYZ = { { wx / wy, 1, (1 - wx - wy) / wy } };
    skcms_Vector3 XYZ = mv_mul(&primaries_inv, &wXYZ);

    skcms_Matrix3x3 toXYZ = {{
        { XYZ.vals[0],           0,           0 },
        {           0, XYZ.vals[1],           0 },
        {           0,           0, XYZ.vals[2] },
    }};
    toXYZ = skcms_Matrix3x3_concat(&primaries, &toXYZ);

    // Now convert toXYZ matrix to toXYZD50.
    skcms_Vector3 wXYZD50 = { { 0.96422f, 1.0f, 0.82521f } };

    // Calculate the chromatic adaptation matrix.  We will use the Bradford method, thus
    // the matrices below.  The Bradford method is used by Adobe and is widely considered
    // to be the best.
    skcms_Matrix3x3 xyz_to_lms = {{
        {  0.8951f,  0.2664f, -0.1614f },
        { -0.7502f,  1.7135f,  0.0367f },
        {  0.0389f, -0.0685f,  1.0296f },
    }};
    skcms_Matrix3x3 lms_to_xyz = {{
        {  0.9869929f, -0.1470543f, 0.1599627f },
        {  0.4323053f,  0.5183603f, 0.0492912f },
        { -0.0085287f,  0.0400428f, 0.9684867f },
    }};

    skcms_Vector3 srcCone = mv_mul(&xyz_to_lms, &wXYZ);
    skcms_Vector3 dstCone = mv_mul(&xyz_to_lms, &wXYZD50);

    skcms_Matrix3x3 DXtoD50 = {{
        { dstCone.vals[0] / srcCone.vals[0], 0, 0 },
        { 0, dstCone.vals[1] / srcCone.vals[1], 0 },
        { 0, 0, dstCone.vals[2] / srcCone.vals[2] },
    }};
    DXtoD50 = skcms_Matrix3x3_concat(&DXtoD50, &xyz_to_lms);
    DXtoD50 = skcms_Matrix3x3_concat(&lms_to_xyz, &DXtoD50);

    *toXYZD50 = skcms_Matrix3x3_concat(&DXtoD50, &toXYZ);
    return true;
}


bool skcms_Matrix3x3_invert(const skcms_Matrix3x3* src, skcms_Matrix3x3* dst) {
    double a00 = src->vals[0][0],
           a01 = src->vals[1][0],
           a02 = src->vals[2][0],
           a10 = src->vals[0][1],
           a11 = src->vals[1][1],
           a12 = src->vals[2][1],
           a20 = src->vals[0][2],
           a21 = src->vals[1][2],
           a22 = src->vals[2][2];

    double b0 = a00*a11 - a01*a10,
           b1 = a00*a12 - a02*a10,
           b2 = a01*a12 - a02*a11,
           b3 = a20,
           b4 = a21,
           b5 = a22;

    double determinant = b0*b5
                       - b1*b4
                       + b2*b3;

    if (determinant == 0) {
        return false;
    }

    double invdet = 1.0 / determinant;
    if (invdet > +FLT_MAX || invdet < -FLT_MAX || !isfinitef_((float)invdet)) {
        return false;
    }

    b0 *= invdet;
    b1 *= invdet;
    b2 *= invdet;
    b3 *= invdet;
    b4 *= invdet;
    b5 *= invdet;

    dst->vals[0][0] = (float)( a11*b5 - a12*b4 );
    dst->vals[1][0] = (float)( a02*b4 - a01*b5 );
    dst->vals[2][0] = (float)(        +     b2 );
    dst->vals[0][1] = (float)( a12*b3 - a10*b5 );
    dst->vals[1][1] = (float)( a00*b5 - a02*b3 );
    dst->vals[2][1] = (float)(        -     b1 );
    dst->vals[0][2] = (float)( a10*b4 - a11*b3 );
    dst->vals[1][2] = (float)( a01*b3 - a00*b4 );
    dst->vals[2][2] = (float)(        +     b0 );

    for (int r = 0; r < 3; ++r)
    for (int c = 0; c < 3; ++c) {
        if (!isfinitef_(dst->vals[r][c])) {
            return false;
        }
    }
    return true;
}

skcms_Matrix3x3 skcms_Matrix3x3_concat(const skcms_Matrix3x3* A, const skcms_Matrix3x3* B) {
    skcms_Matrix3x3 m = { { { 0,0,0 },{ 0,0,0 },{ 0,0,0 } } };
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++) {
            m.vals[r][c] = A->vals[r][0] * B->vals[0][c]
                         + A->vals[r][1] * B->vals[1][c]
                         + A->vals[r][2] * B->vals[2][c];
        }
    return m;
}

#if defined(__clang__) || defined(__GNUC__)
    #define small_memcpy __builtin_memcpy
#else
    #define small_memcpy memcpy
#endif

static float log2f_(float x) {
    // The first approximation of log2(x) is its exponent 'e', minus 127.
    int32_t bits;
    small_memcpy(&bits, &x, sizeof(bits));

    float e = (float)bits * (1.0f / (1<<23));

    // If we use the mantissa too we can refine the error signficantly.
    int32_t m_bits = (bits & 0x007fffff) | 0x3f000000;
    float m;
    small_memcpy(&m, &m_bits, sizeof(m));

    return (e - 124.225514990f
              -   1.498030302f*m
              -   1.725879990f/(0.3520887068f + m));
}

static float exp2f_(float x) {
    float fract = x - floorf_(x);

    float fbits = (1.0f * (1<<23)) * (x + 121.274057500f
                                        -   1.490129070f*fract
                                        +  27.728023300f/(4.84252568f - fract));
    if (fbits > INT_MAX) {
        return INFINITY_;
    } else if (fbits < INT_MIN) {
        return -INFINITY_;
    }
    int32_t bits = (int32_t)fbits;
    small_memcpy(&x, &bits, sizeof(x));
    return x;
}

float powf_(float x, float y) {
    return (x == 0) || (x == 1) ? x
                                : exp2f_(log2f_(x) * y);
}

float skcms_TransferFunction_eval(const skcms_TransferFunction* tf, float x) {
    float sign = x < 0 ? -1.0f : 1.0f;
    x *= sign;

    return sign * (x < tf->d ? tf->c * x + tf->f
                             : powf_(tf->a * x + tf->b, tf->g) + tf->e);
}

#if defined(__clang__)
    [[clang::no_sanitize("float-divide-by-zero")]]  // Checked for by tf_is_valid() on the way out.
#endif
bool skcms_TransferFunction_invert(const skcms_TransferFunction* src, skcms_TransferFunction* dst) {
    if (!tf_is_valid(src)) {
        return false;
    }

    // We're inverting this function, solving for x in terms of y.
    //   y = (cx + f)         x < d
    //       (ax + b)^g + e   x ≥ d
    // The inverse of this function can be expressed in the same piecewise form.
    skcms_TransferFunction inv = {0,0,0,0,0,0,0};

    // We'll start by finding the new threshold inv.d.
    // In principle we should be able to find that by solving for y at x=d from either side.
    // (If those two d values aren't the same, it's a discontinuous transfer function.)
    float d_l =       src->c * src->d + src->f,
          d_r = powf_(src->a * src->d + src->b, src->g) + src->e;
    if (fabsf_(d_l - d_r) > 1/512.0f) {
        return false;
    }
    inv.d = d_l;  // TODO(mtklein): better in practice to choose d_r?

    // When d=0, the linear section collapses to a point.  We leave c,d,f all zero in that case.
    if (inv.d > 0) {
        // Inverting the linear section is pretty straightfoward:
        //        y       = cx + f
        //        y - f   = cx
        //   (1/c)y - f/c = x
        inv.c =    1.0f/src->c;
        inv.f = -src->f/src->c;
    }

    // The interesting part is inverting the nonlinear section:
    //         y                = (ax + b)^g + e.
    //         y - e            = (ax + b)^g
    //        (y - e)^1/g       =  ax + b
    //        (y - e)^1/g - b   =  ax
    //   (1/a)(y - e)^1/g - b/a =   x
    //
    // To make that fit our form, we need to move the (1/a) term inside the exponentiation:
    //   let k = (1/a)^g
    //   (1/a)( y -  e)^1/g - b/a = x
    //        (ky - ke)^1/g - b/a = x

    float k = powf_(src->a, -src->g);  // (1/a)^g == a^-g
    inv.g = 1.0f / src->g;
    inv.a = k;
    inv.b = -k * src->e;
    inv.e = -src->b / src->a;

    // Now in principle we're done.
    // But to preserve the valuable invariant inv(src(1.0f)) == 1.0f,
    // we'll tweak e.  These two values should be close to each other,
    // just down to numerical precision issues, especially from powf_.
    float s = powf_(src->a + src->b, src->g) + src->e;
    inv.e = 1.0f - powf_(inv.a * s + inv.b, inv.g);

    *dst = inv;
    return tf_is_valid(dst);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

// From here below we're approximating an skcms_Curve with an skcms_TransferFunction{g,a,b,c,d,e,f}:
//
//   tf(x) =  cx + f          x < d
//   tf(x) = (ax + b)^g + e   x ≥ d
//
// When fitting, we add the additional constraint that both pieces meet at d:
//
//   cd + f = (ad + b)^g + e
//
// Solving for e and folding it through gives an alternate formulation of the non-linear piece:
//
//   tf(x) =                           cx + f   x < d
//   tf(x) = (ax + b)^g - (ad + b)^g + cd + f   x ≥ d
//
// Our overall strategy is then:
//    For a couple tolerances,
//       - fit_linear():    fit c,d,f iteratively to as many points as our tolerance allows
//       - invert c,d,f
//       - fit_nonlinear(): fit g,a,b using Gauss-Newton given those inverted c,d,f
//                          (and by constraint, inverted e) to the inverse of the table.
//    Return the parameters with least maximum error.
//
// To run Gauss-Newton to find g,a,b, we'll also need the gradient of the residuals
// of round-trip f_inv(x), the inverse of the non-linear piece of f(x).
//
//    let y = Table(x)
//    r(x) = x - f_inv(y)
//
//    ∂r/∂g = ln(ay + b)*(ay + b)^g
//          - ln(ad + b)*(ad + b)^g
//    ∂r/∂a = yg(ay + b)^(g-1)
//          - dg(ad + b)^(g-1)
//    ∂r/∂b =  g(ay + b)^(g-1)
//          -  g(ad + b)^(g-1)

// Return the residual of roundtripping skcms_Curve(x) through f_inv(y) with parameters P,
// and fill out the gradient of the residual into dfdP.
static float rg_nonlinear(float x,
                          const skcms_Curve* curve,
                          const skcms_TransferFunction* tf,
                          const float P[3],
                          float dfdP[3]) {
    const float y = eval_curve(curve, x);

    const float g = P[0],  a = P[1],  b = P[2],
                c = tf->c, d = tf->d, f = tf->f;

    const float Y = fmaxf_(a*y + b, 0.0f),
                D =        a*d + b;
    assert (D >= 0);

    // The gradient.
    dfdP[0] = 0.69314718f*log2f_(Y)*powf_(Y, g)
            - 0.69314718f*log2f_(D)*powf_(D, g);
    dfdP[1] = y*g*powf_(Y, g-1)
            - d*g*powf_(D, g-1);
    dfdP[2] =   g*powf_(Y, g-1)
            -   g*powf_(D, g-1);

    // The residual.
    const float f_inv = powf_(Y, g)
                      - powf_(D, g)
                      + c*d + f;
    return x - f_inv;
}

static bool gauss_newton_step(const skcms_Curve* curve,
                              const skcms_TransferFunction* tf,
                              float P[3],
                              float x0, float dx, int N) {
    // We'll sample x from the range [x0,x1] (both inclusive) N times with even spacing.
    //
    // We want to do P' = P + (Jf^T Jf)^-1 Jf^T r(P),
    //   where r(P) is the residual vector
    //   and Jf is the Jacobian matrix of f(), ∂r/∂P.
    //
    // Let's review the shape of each of these expressions:
    //   r(P)   is [N x 1], a column vector with one entry per value of x tested
    //   Jf     is [N x 3], a matrix with an entry for each (x,P) pair
    //   Jf^T   is [3 x N], the transpose of Jf
    //
    //   Jf^T Jf   is [3 x N] * [N x 3] == [3 x 3], a 3x3 matrix,
    //                                              and so is its inverse (Jf^T Jf)^-1
    //   Jf^T r(P) is [3 x N] * [N x 1] == [3 x 1], a column vector with the same shape as P
    //
    // Our implementation strategy to get to the final ∆P is
    //   1) evaluate Jf^T Jf,   call that lhs
    //   2) evaluate Jf^T r(P), call that rhs
    //   3) invert lhs
    //   4) multiply inverse lhs by rhs
    //
    // This is a friendly implementation strategy because we don't have to have any
    // buffers that scale with N, and equally nice don't have to perform any matrix
    // operations that are variable size.
    //
    // Other implementation strategies could trade this off, e.g. evaluating the
    // pseudoinverse of Jf ( (Jf^T Jf)^-1 Jf^T ) directly, then multiplying that by
    // the residuals.  That would probably require implementing singular value
    // decomposition, and would create a [3 x N] matrix to be multiplied by the
    // [N x 1] residual vector, but on the upside I think that'd eliminate the
    // possibility of this gauss_newton_step() function ever failing.

    // 0) start off with lhs and rhs safely zeroed.
    skcms_Matrix3x3 lhs = {{ {0,0,0}, {0,0,0}, {0,0,0} }};
    skcms_Vector3   rhs = {  {0,0,0} };

    // 1,2) evaluate lhs and evaluate rhs
    //   We want to evaluate Jf only once, but both lhs and rhs involve Jf^T,
    //   so we'll have to update lhs and rhs at the same time.
    for (int i = 0; i < N; i++) {
        float x = x0 + i*dx;

        float dfdP[3] = {0,0,0};
        float resid = rg_nonlinear(x,curve,tf,P, dfdP);

        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 3; c++) {
                lhs.vals[r][c] += dfdP[r] * dfdP[c];
            }
            rhs.vals[r] += dfdP[r] * resid;
        }
    }

    // If any of the 3 P parameters are unused, this matrix will be singular.
    // Detect those cases and fix them up to indentity instead, so we can invert.
    for (int k = 0; k < 3; k++) {
        if (lhs.vals[0][k]==0 && lhs.vals[1][k]==0 && lhs.vals[2][k]==0 &&
            lhs.vals[k][0]==0 && lhs.vals[k][1]==0 && lhs.vals[k][2]==0) {
            lhs.vals[k][k] = 1;
        }
    }

    // 3) invert lhs
    skcms_Matrix3x3 lhs_inv;
    if (!skcms_Matrix3x3_invert(&lhs, &lhs_inv)) {
        return false;
    }

    // 4) multiply inverse lhs by rhs
    skcms_Vector3 dP = mv_mul(&lhs_inv, &rhs);
    P[0] += dP.vals[0];
    P[1] += dP.vals[1];
    P[2] += dP.vals[2];
    return isfinitef_(P[0]) && isfinitef_(P[1]) && isfinitef_(P[2]);
}


// Fit the points in [L,N) to the non-linear piece of tf, or return false if we can't.
static bool fit_nonlinear(const skcms_Curve* curve, int L, int N, skcms_TransferFunction* tf) {
    float P[3] = { tf->g, tf->a, tf->b };

    // No matter where we start, dx should always represent N even steps from 0 to 1.
    const float dx = 1.0f / (N-1);

    for (int j = 0; j < 3/*TODO: tune*/; j++) {
        // These extra constraints a >= 0 and ad+b >= 0 are not modeled in the optimization.
        // We don't really know how to fix up a if it goes negative.
        if (P[1] < 0) {
            return false;
        }
        // If ad+b goes negative, we feel just barely not uneasy enough to tweak b so ad+b is zero.
        if (P[1] * tf->d + P[2] < 0) {
            P[2] = -P[1] * tf->d;
        }
        assert (P[1] >= 0 &&
                P[1] * tf->d + P[2] >= 0);

        if (!gauss_newton_step(curve, tf,
                               P,
                               L*dx, dx, N-L)) {
            return false;
        }
    }

    // We need to apply our fixups one last time
    if (P[1] < 0) {
        return false;
    }
    if (P[1] * tf->d + P[2] < 0) {
        P[2] = -P[1] * tf->d;
    }

    tf->g = P[0];
    tf->a = P[1];
    tf->b = P[2];
    tf->e =   tf->c*tf->d + tf->f
      - powf_(tf->a*tf->d + tf->b, tf->g);
    return true;
}

bool skcms_ApproximateCurve(const skcms_Curve* curve,
                            skcms_TransferFunction* approx,
                            float* max_error) {
    if (!curve || !approx || !max_error) {
        return false;
    }

    if (curve->table_entries == 0) {
        // No point approximating an skcms_TransferFunction with an skcms_TransferFunction!
        return false;
    }

    if (curve->table_entries == 1 || curve->table_entries > (uint32_t)INT_MAX) {
        // We need at least two points, and must put some reasonable cap on the maximum number.
        return false;
    }

    int N = (int)curve->table_entries;
    const float dx = 1.0f / (N - 1);

    *max_error = INFINITY_;
    const float kTolerances[] = { 1.5f / 65535.0f, 1.0f / 512.0f };
    for (int t = 0; t < ARRAY_COUNT(kTolerances); t++) {
        skcms_TransferFunction tf,
                               tf_inv;

        // It's problematic to fit curves with non-zero f, so always force it to zero explicitly.
        tf.f = 0.0f;
        int L = fit_linear(curve, N, kTolerances[t], &tf.c, &tf.d);

        if (L == N) {
            // If the entire data set was linear, move the coefficients to the nonlinear portion
            // with G == 1.  This lets use a canonical representation with d == 0.
            tf.g = 1;
            tf.a = tf.c;
            tf.b = tf.f;
            tf.c = tf.d = tf.e = tf.f = 0;
        } else if (L == N - 1) {
            // Degenerate case with only two points in the nonlinear segment. Solve directly.
            tf.g = 1;
            tf.a = (eval_curve(curve, (N-1)*dx) -
                    eval_curve(curve, (N-2)*dx))
                 / dx;
            tf.b = eval_curve(curve, (N-2)*dx)
                 - tf.a * (N-2)*dx;
            tf.e = 0;
        } else {
            // Start by guessing a gamma-only curve through the midpoint.
            int mid = (L + N) / 2;
            float mid_x = mid / (N - 1.0f);
            float mid_y = eval_curve(curve, mid_x);
            tf.g = log2f_(mid_y) / log2f_(mid_x);
            tf.a = 1;
            tf.b = 0;
            tf.e =    tf.c*tf.d + tf.f
              - powf_(tf.a*tf.d + tf.b, tf.g);


            if (!skcms_TransferFunction_invert(&tf, &tf_inv) ||
                !fit_nonlinear(curve, L,N, &tf_inv)) {
                continue;
            }

            // We fit tf_inv, so calculate tf to keep in sync.
            if (!skcms_TransferFunction_invert(&tf_inv, &tf)) {
                continue;
            }
        }

        // We find our error by roundtripping the table through tf_inv.
        //
        // (The most likely use case for this approximation is to be inverted and
        // used as the transfer function for a destination color space.)
        //
        // We've kept tf and tf_inv in sync above, but we can't guarantee that tf is
        // invertible, so re-verify that here (and use the new inverse for testing).
        if (!skcms_TransferFunction_invert(&tf, &tf_inv)) {
            continue;
        }

        float err = max_roundtrip_error(curve, &tf_inv);
        if (*max_error > err) {
            *max_error = err;
            *approx    = tf;
        }
    }
    return isfinitef_(*max_error);
}

// ~~~~ Impl. of skcms_Transform() ~~~~

typedef enum {
    Op_load_a8,
    Op_load_g8,
    Op_load_8888_palette8,
    Op_load_4444,
    Op_load_565,
    Op_load_888,
    Op_load_8888,
    Op_load_1010102,
    Op_load_161616LE,
    Op_load_16161616LE,
    Op_load_161616BE,
    Op_load_16161616BE,
    Op_load_hhh,
    Op_load_hhhh,
    Op_load_fff,
    Op_load_ffff,

    Op_swap_rb,
    Op_clamp,
    Op_invert,
    Op_force_opaque,
    Op_premul,
    Op_unpremul,
    Op_matrix_3x3,
    Op_matrix_3x4,
    Op_lab_to_xyz,

    Op_tf_r,
    Op_tf_g,
    Op_tf_b,
    Op_tf_a,

    Op_table_r,
    Op_table_g,
    Op_table_b,
    Op_table_a,

    Op_clut,

    Op_store_a8,
    Op_store_g8,
    Op_store_4444,
    Op_store_565,
    Op_store_888,
    Op_store_8888,
    Op_store_1010102,
    Op_store_161616LE,
    Op_store_16161616LE,
    Op_store_161616BE,
    Op_store_16161616BE,
    Op_store_hhh,
    Op_store_hhhh,
    Op_store_fff,
    Op_store_ffff,
} Op;

#if defined(__clang__)
    template <int N, typename T> using Vec = T __attribute__((ext_vector_type(N)));
#elif defined(__GNUC__)
    // For some reason GCC accepts this nonsense, but not the more straightforward version,
    //   template <int N, typename T> using Vec = T __attribute__((vector_size(N*sizeof(T))));
    template <int N, typename T>
    struct VecHelper { typedef T __attribute__((vector_size(N*sizeof(T)))) V; };

    template <int N, typename T> using Vec = typename VecHelper<N,T>::V;
#endif

// First, instantiate our default exec_ops() implementation using the default compiliation target.

namespace baseline {
#if defined(SKCMS_PORTABLE) || !(defined(__clang__) || defined(__GNUC__)) \
                            || (defined(__EMSCRIPTEN_major__) && !defined(__wasm_simd128__))
    #define N 1
    using F   = float;
    using U64 = uint64_t;
    using U32 = uint32_t;
    using I32 = int32_t;
    using U16 = uint16_t;
    using U8  = uint8_t;

#elif defined(__AVX512F__)
    #define N 16
    using   F = Vec<N,float>;
    using I32 = Vec<N,int32_t>;
    using U64 = Vec<N,uint64_t>;
    using U32 = Vec<N,uint32_t>;
    using U16 = Vec<N,uint16_t>;
    using  U8 = Vec<N,uint8_t>;
#elif defined(__AVX__)
    #define N 8
    using   F = Vec<N,float>;
    using I32 = Vec<N,int32_t>;
    using U64 = Vec<N,uint64_t>;
    using U32 = Vec<N,uint32_t>;
    using U16 = Vec<N,uint16_t>;
    using  U8 = Vec<N,uint8_t>;
#else
    #define N 4
    using   F = Vec<N,float>;
    using I32 = Vec<N,int32_t>;
    using U64 = Vec<N,uint64_t>;
    using U32 = Vec<N,uint32_t>;
    using U16 = Vec<N,uint16_t>;
    using  U8 = Vec<N,uint8_t>;
#endif

    #include "src/Transform_inl.h"
    #undef N
}

// Now, instantiate any other versions of run_program() we may want for runtime detection.
#if !defined(SKCMS_PORTABLE) &&                           \
        (( defined(__clang__) && __clang_major__ >= 5) || \
         (!defined(__clang__) && defined(__GNUC__)))      \
     && defined(__x86_64__)

    #if !defined(__AVX2__)
        #if defined(__clang__)
            #pragma clang attribute push(__attribute__((target("avx2,f16c"))), apply_to=function)
        #elif defined(__GNUC__)
            #pragma GCC push_options
            #pragma GCC target("avx2,f16c")
        #endif

        namespace hsw {
            #define USING_AVX
            #define USING_AVX_F16C
            #define USING_AVX2
            #define N 8
            using   F = Vec<N,float>;
            using I32 = Vec<N,int32_t>;
            using U64 = Vec<N,uint64_t>;
            using U32 = Vec<N,uint32_t>;
            using U16 = Vec<N,uint16_t>;
            using  U8 = Vec<N,uint8_t>;

            #include "src/Transform_inl.h"

            // src/Transform_inl.h will undefine USING_* for us.
            #undef N
        }

        #if defined(__clang__)
            #pragma clang attribute pop
        #elif defined(__GNUC__)
            #pragma GCC pop_options
        #endif

        #define TEST_FOR_HSW
    #endif

    #if !defined(__AVX512F__)
        #if defined(__clang__)
            #pragma clang attribute push(__attribute__((target("avx512f,avx512dq,avx512cd,avx512bw,avx512vl"))), apply_to=function)
        #elif defined(__GNUC__)
            #pragma GCC push_options
            #pragma GCC target("avx512f,avx512dq,avx512cd,avx512bw,avx512vl")
        #endif

        namespace skx {
            #define USING_AVX512F
            #define N 16
            using   F = Vec<N,float>;
            using I32 = Vec<N,int32_t>;
            using U64 = Vec<N,uint64_t>;
            using U32 = Vec<N,uint32_t>;
            using U16 = Vec<N,uint16_t>;
            using  U8 = Vec<N,uint8_t>;

            #include "src/Transform_inl.h"

            // src/Transform_inl.h will undefine USING_* for us.
            #undef N
        }

        #if defined(__clang__)
            #pragma clang attribute pop
        #elif defined(__GNUC__)
            #pragma GCC pop_options
        #endif

        #define TEST_FOR_SKX
    #endif

    #if defined(TEST_FOR_HSW) || defined(TEST_FOR_SKX)
        enum class CpuType { None, HSW, SKX };
        static CpuType cpu_type() {
            static const CpuType type = []{
                // See http://www.sandpile.org/x86/cpuid.htm

                // First, a basic cpuid(1) lets us check prerequisites for HSW, SKX.
                uint32_t eax, ebx, ecx, edx;
                __asm__ __volatile__("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                                             : "0"(1), "2"(0));
                if ((edx & (1u<<25)) &&  // SSE
                    (edx & (1u<<26)) &&  // SSE2
                    (ecx & (1u<< 0)) &&  // SSE3
                    (ecx & (1u<< 9)) &&  // SSSE3
                    (ecx & (1u<<12)) &&  // FMA (N.B. not used, avoided even)
                    (ecx & (1u<<19)) &&  // SSE4.1
                    (ecx & (1u<<20)) &&  // SSE4.2
                    (ecx & (1u<<26)) &&  // XSAVE
                    (ecx & (1u<<27)) &&  // OSXSAVE
                    (ecx & (1u<<28)) &&  // AVX
                    (ecx & (1u<<29))) {  // F16C

                    // Call cpuid(7) to check for AVX2 and AVX-512 bits.
                    __asm__ __volatile__("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                                                 : "0"(7), "2"(0));
                    // eax from xgetbv(0) will tell us whether XMM, YMM, and ZMM state is saved.
                    uint32_t xcr0, dont_need_edx;
                    __asm__ __volatile__("xgetbv" : "=a"(xcr0), "=d"(dont_need_edx) : "c"(0));

                    if ((xcr0 & (1u<<1)) &&  // XMM register state saved?
                        (xcr0 & (1u<<2)) &&  // YMM register state saved?
                        (ebx  & (1u<<5))) {  // AVX2
                        // At this point we're at least HSW.  Continue checking for SKX.
                        if ((xcr0 & (1u<< 5)) && // Opmasks state saved?
                            (xcr0 & (1u<< 6)) && // First 16 ZMM registers saved?
                            (xcr0 & (1u<< 7)) && // High 16 ZMM registers saved?
                            (ebx  & (1u<<16)) && // AVX512F
                            (ebx  & (1u<<17)) && // AVX512DQ
                            (ebx  & (1u<<28)) && // AVX512CD
                            (ebx  & (1u<<30)) && // AVX512BW
                            (ebx  & (1u<<31))) { // AVX512VL
                            return CpuType::SKX;
                        }
                        return CpuType::HSW;
                    }
                }
                return CpuType::None;
            }();
            return type;
        }
    #endif

#endif

static bool is_identity_tf(const skcms_TransferFunction* tf) {
    return tf->g == 1 && tf->a == 1
        && tf->b == 0 && tf->c == 0 && tf->d == 0 && tf->e == 0 && tf->f == 0;
}

typedef struct {
    Op          op;
    const void* arg;
} OpAndArg;

static OpAndArg select_curve_op(const skcms_Curve* curve, int channel) {
    static const struct { Op parametric, table; } ops[] = {
        { Op_tf_r, Op_table_r },
        { Op_tf_g, Op_table_g },
        { Op_tf_b, Op_table_b },
        { Op_tf_a, Op_table_a },
    };

    const OpAndArg noop = { Op_load_a8/*doesn't matter*/, nullptr };

    if (curve->table_entries == 0) {
        return is_identity_tf(&curve->parametric)
            ? noop
            : OpAndArg{ ops[channel].parametric, &curve->parametric };
    }

    return OpAndArg{ ops[channel].table, curve };
}

static size_t bytes_per_pixel(skcms_PixelFormat fmt) {
    switch (fmt >> 1) {   // ignore rgb/bgr
        case skcms_PixelFormat_A_8                >> 1: return  1;
        case skcms_PixelFormat_G_8                >> 1: return  1;
        case skcms_PixelFormat_RGBA_8888_Palette8 >> 1: return  1;
        case skcms_PixelFormat_ABGR_4444          >> 1: return  2;
        case skcms_PixelFormat_RGB_565            >> 1: return  2;
        case skcms_PixelFormat_RGB_888            >> 1: return  3;
        case skcms_PixelFormat_RGBA_8888          >> 1: return  4;
        case skcms_PixelFormat_RGBA_1010102       >> 1: return  4;
        case skcms_PixelFormat_RGB_161616LE       >> 1: return  6;
        case skcms_PixelFormat_RGBA_16161616LE    >> 1: return  8;
        case skcms_PixelFormat_RGB_161616BE       >> 1: return  6;
        case skcms_PixelFormat_RGBA_16161616BE    >> 1: return  8;
        case skcms_PixelFormat_RGB_hhh_Norm       >> 1: return  6;
        case skcms_PixelFormat_RGBA_hhhh_Norm     >> 1: return  8;
        case skcms_PixelFormat_RGB_hhh            >> 1: return  6;
        case skcms_PixelFormat_RGBA_hhhh          >> 1: return  8;
        case skcms_PixelFormat_RGB_fff            >> 1: return 12;
        case skcms_PixelFormat_RGBA_ffff          >> 1: return 16;
    }
    assert(false);
    return 0;
}

static bool prep_for_destination(const skcms_ICCProfile* profile,
                                 skcms_Matrix3x3* fromXYZD50,
                                 skcms_TransferFunction* invR,
                                 skcms_TransferFunction* invG,
                                 skcms_TransferFunction* invB) {
    // We only support destinations with parametric transfer functions
    // and with gamuts that can be transformed from XYZD50.
    return profile->has_trc
        && profile->has_toXYZD50
        && profile->trc[0].table_entries == 0
        && profile->trc[1].table_entries == 0
        && profile->trc[2].table_entries == 0
        && skcms_TransferFunction_invert(&profile->trc[0].parametric, invR)
        && skcms_TransferFunction_invert(&profile->trc[1].parametric, invG)
        && skcms_TransferFunction_invert(&profile->trc[2].parametric, invB)
        && skcms_Matrix3x3_invert(&profile->toXYZD50, fromXYZD50);
}

bool skcms_Transform(const void*             src,
                     skcms_PixelFormat       srcFmt,
                     skcms_AlphaFormat       srcAlpha,
                     const skcms_ICCProfile* srcProfile,
                     void*                   dst,
                     skcms_PixelFormat       dstFmt,
                     skcms_AlphaFormat       dstAlpha,
                     const skcms_ICCProfile* dstProfile,
                     size_t                  npixels) {
    return skcms_TransformWithPalette(src, srcFmt, srcAlpha, srcProfile,
                                      dst, dstFmt, dstAlpha, dstProfile,
                                      npixels, nullptr);
}

bool skcms_TransformWithPalette(const void*             src,
                                skcms_PixelFormat       srcFmt,
                                skcms_AlphaFormat       srcAlpha,
                                const skcms_ICCProfile* srcProfile,
                                void*                   dst,
                                skcms_PixelFormat       dstFmt,
                                skcms_AlphaFormat       dstAlpha,
                                const skcms_ICCProfile* dstProfile,
                                size_t                  nz,
                                const void*             palette) {
    const size_t dst_bpp = bytes_per_pixel(dstFmt),
                 src_bpp = bytes_per_pixel(srcFmt);
    // Let's just refuse if the request is absurdly big.
    if (nz * dst_bpp > INT_MAX || nz * src_bpp > INT_MAX) {
        return false;
    }
    int n = (int)nz;

    // Null profiles default to sRGB. Passing null for both is handy when doing format conversion.
    if (!srcProfile) {
        srcProfile = skcms_sRGB_profile();
    }
    if (!dstProfile) {
        dstProfile = skcms_sRGB_profile();
    }

    // We can't transform in place unless the PixelFormats are the same size.
    if (dst == src && dst_bpp != src_bpp) {
        return false;
    }
    // TODO: more careful alias rejection (like, dst == src + 1)?

    if (needs_palette(srcFmt) && !palette) {
        return false;
    }

    Op          program  [32];
    const void* arguments[32];

    Op*          ops  = program;
    const void** args = arguments;

    skcms_TransferFunction inv_dst_tf_r, inv_dst_tf_g, inv_dst_tf_b;
    skcms_Matrix3x3        from_xyz;

    switch (srcFmt >> 1) {
        default: return false;
        case skcms_PixelFormat_A_8             >> 1: *ops++ = Op_load_a8;         break;
        case skcms_PixelFormat_G_8             >> 1: *ops++ = Op_load_g8;         break;
        case skcms_PixelFormat_ABGR_4444       >> 1: *ops++ = Op_load_4444;       break;
        case skcms_PixelFormat_RGB_565         >> 1: *ops++ = Op_load_565;        break;
        case skcms_PixelFormat_RGB_888         >> 1: *ops++ = Op_load_888;        break;
        case skcms_PixelFormat_RGBA_8888       >> 1: *ops++ = Op_load_8888;       break;
        case skcms_PixelFormat_RGBA_1010102    >> 1: *ops++ = Op_load_1010102;    break;
        case skcms_PixelFormat_RGB_161616LE    >> 1: *ops++ = Op_load_161616LE;   break;
        case skcms_PixelFormat_RGBA_16161616LE >> 1: *ops++ = Op_load_16161616LE; break;
        case skcms_PixelFormat_RGB_161616BE    >> 1: *ops++ = Op_load_161616BE;   break;
        case skcms_PixelFormat_RGBA_16161616BE >> 1: *ops++ = Op_load_16161616BE; break;
        case skcms_PixelFormat_RGB_hhh_Norm    >> 1: *ops++ = Op_load_hhh;        break;
        case skcms_PixelFormat_RGBA_hhhh_Norm  >> 1: *ops++ = Op_load_hhhh;       break;
        case skcms_PixelFormat_RGB_hhh         >> 1: *ops++ = Op_load_hhh;        break;
        case skcms_PixelFormat_RGBA_hhhh       >> 1: *ops++ = Op_load_hhhh;       break;
        case skcms_PixelFormat_RGB_fff         >> 1: *ops++ = Op_load_fff;        break;
        case skcms_PixelFormat_RGBA_ffff       >> 1: *ops++ = Op_load_ffff;       break;

        case skcms_PixelFormat_RGBA_8888_Palette8 >> 1: *ops++  = Op_load_8888_palette8;
                                                        *args++ = palette;
                                                        break;
    }
    if (srcFmt == skcms_PixelFormat_RGB_hhh_Norm ||
        srcFmt == skcms_PixelFormat_RGBA_hhhh_Norm) {
        *ops++ = Op_clamp;
    }
    if (srcFmt & 1) {
        *ops++ = Op_swap_rb;
    }
    skcms_ICCProfile gray_dst_profile;
    if ((dstFmt >> 1) == (skcms_PixelFormat_G_8 >> 1)) {
        // When transforming to gray, stop at XYZ (by setting toXYZ to identity), then transform
        // luminance (Y) by the destination transfer function.
        gray_dst_profile = *dstProfile;
        skcms_SetXYZD50(&gray_dst_profile, &skcms_XYZD50_profile()->toXYZD50);
        dstProfile = &gray_dst_profile;
    }

    if (srcProfile->data_color_space == skcms_Signature_CMYK) {
        // Photoshop creates CMYK images as inverse CMYK.
        // These happen to be the only ones we've _ever_ seen.
        *ops++ = Op_invert;
        // With CMYK, ignore the alpha type, to avoid changing K or conflating CMY with K.
        srcAlpha = skcms_AlphaFormat_Unpremul;
    }

    if (srcAlpha == skcms_AlphaFormat_Opaque) {
        *ops++ = Op_force_opaque;
    } else if (srcAlpha == skcms_AlphaFormat_PremulAsEncoded) {
        *ops++ = Op_unpremul;
    }

    if (dstProfile != srcProfile) {

        if (!prep_for_destination(dstProfile,
                                  &from_xyz, &inv_dst_tf_r, &inv_dst_tf_b, &inv_dst_tf_g)) {
            return false;
        }

        if (srcProfile->has_A2B) {
            if (srcProfile->A2B.input_channels) {
                for (int i = 0; i < (int)srcProfile->A2B.input_channels; i++) {
                    OpAndArg oa = select_curve_op(&srcProfile->A2B.input_curves[i], i);
                    if (oa.arg) {
                        *ops++  = oa.op;
                        *args++ = oa.arg;
                    }
                }
                *ops++ = Op_clamp;
                *ops++ = Op_clut;
                *args++ = &srcProfile->A2B;
            }

            if (srcProfile->A2B.matrix_channels == 3) {
                for (int i = 0; i < 3; i++) {
                    OpAndArg oa = select_curve_op(&srcProfile->A2B.matrix_curves[i], i);
                    if (oa.arg) {
                        *ops++  = oa.op;
                        *args++ = oa.arg;
                    }
                }

                static const skcms_Matrix3x4 I = {{
                    {1,0,0,0},
                    {0,1,0,0},
                    {0,0,1,0},
                }};
                if (0 != memcmp(&I, &srcProfile->A2B.matrix, sizeof(I))) {
                    *ops++  = Op_matrix_3x4;
                    *args++ = &srcProfile->A2B.matrix;
                }
            }

            if (srcProfile->A2B.output_channels == 3) {
                for (int i = 0; i < 3; i++) {
                    OpAndArg oa = select_curve_op(&srcProfile->A2B.output_curves[i], i);
                    if (oa.arg) {
                        *ops++  = oa.op;
                        *args++ = oa.arg;
                    }
                }
            }

            if (srcProfile->pcs == skcms_Signature_Lab) {
                *ops++ = Op_lab_to_xyz;
            }

        } else if (srcProfile->has_trc && srcProfile->has_toXYZD50) {
            for (int i = 0; i < 3; i++) {
                OpAndArg oa = select_curve_op(&srcProfile->trc[i], i);
                if (oa.arg) {
                    *ops++  = oa.op;
                    *args++ = oa.arg;
                }
            }
        } else {
            return false;
        }

        // A2B sources should already be in XYZD50 at this point.
        // Others still need to be transformed using their toXYZD50 matrix.
        // N.B. There are profiles that contain both A2B tags and toXYZD50 matrices.
        // If we use the A2B tags, we need to ignore the XYZD50 matrix entirely.
        assert (srcProfile->has_A2B || srcProfile->has_toXYZD50);
        static const skcms_Matrix3x3 I = {{
            { 1.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f },
        }};
        const skcms_Matrix3x3* to_xyz = srcProfile->has_A2B ? &I : &srcProfile->toXYZD50;

        // There's a chance the source and destination gamuts are identical,
        // in which case we can skip the gamut transform.
        if (0 != memcmp(&dstProfile->toXYZD50, to_xyz, sizeof(skcms_Matrix3x3))) {
            // Concat the entire gamut transform into from_xyz,
            // now slightly misnamed but it's a handy spot to stash the result.
            from_xyz = skcms_Matrix3x3_concat(&from_xyz, to_xyz);
            *ops++  = Op_matrix_3x3;
            *args++ = &from_xyz;
        }

        // Encode back to dst RGB using its parametric transfer functions.
        if (!is_identity_tf(&inv_dst_tf_r)) { *ops++ = Op_tf_r; *args++ = &inv_dst_tf_r; }
        if (!is_identity_tf(&inv_dst_tf_g)) { *ops++ = Op_tf_g; *args++ = &inv_dst_tf_g; }
        if (!is_identity_tf(&inv_dst_tf_b)) { *ops++ = Op_tf_b; *args++ = &inv_dst_tf_b; }
    }

    // Clamp here before premul to make sure we're clamping to normalized values _and_ gamut,
    // not just to values that fit in [0,1].
    //
    // E.g. r = 1.1, a = 0.5 would fit fine in fixed point after premul (ra=0.55,a=0.5),
    // but would be carrying r > 1, which is really unexpected for downstream consumers.
    if (dstFmt < skcms_PixelFormat_RGB_hhh) {
        *ops++ = Op_clamp;
    }
    if (dstAlpha == skcms_AlphaFormat_Opaque) {
        *ops++ = Op_force_opaque;
    } else if (dstAlpha == skcms_AlphaFormat_PremulAsEncoded) {
        *ops++ = Op_premul;
    }
    if (dstFmt & 1) {
        *ops++ = Op_swap_rb;
    }
    switch (dstFmt >> 1) {
        default: return false;
        case skcms_PixelFormat_A_8             >> 1: *ops++ = Op_store_a8;         break;
        case skcms_PixelFormat_G_8             >> 1: *ops++ = Op_store_g8;         break;
        case skcms_PixelFormat_ABGR_4444       >> 1: *ops++ = Op_store_4444;       break;
        case skcms_PixelFormat_RGB_565         >> 1: *ops++ = Op_store_565;        break;
        case skcms_PixelFormat_RGB_888         >> 1: *ops++ = Op_store_888;        break;
        case skcms_PixelFormat_RGBA_8888       >> 1: *ops++ = Op_store_8888;       break;
        case skcms_PixelFormat_RGBA_1010102    >> 1: *ops++ = Op_store_1010102;    break;
        case skcms_PixelFormat_RGB_161616LE    >> 1: *ops++ = Op_store_161616LE;   break;
        case skcms_PixelFormat_RGBA_16161616LE >> 1: *ops++ = Op_store_16161616LE; break;
        case skcms_PixelFormat_RGB_161616BE    >> 1: *ops++ = Op_store_161616BE;   break;
        case skcms_PixelFormat_RGBA_16161616BE >> 1: *ops++ = Op_store_16161616BE; break;
        case skcms_PixelFormat_RGB_hhh_Norm    >> 1: *ops++ = Op_store_hhh;        break;
        case skcms_PixelFormat_RGBA_hhhh_Norm  >> 1: *ops++ = Op_store_hhhh;       break;
        case skcms_PixelFormat_RGB_hhh         >> 1: *ops++ = Op_store_hhh;        break;
        case skcms_PixelFormat_RGBA_hhhh       >> 1: *ops++ = Op_store_hhhh;       break;
        case skcms_PixelFormat_RGB_fff         >> 1: *ops++ = Op_store_fff;        break;
        case skcms_PixelFormat_RGBA_ffff       >> 1: *ops++ = Op_store_ffff;       break;
    }

    auto run = baseline::run_program;
#if defined(TEST_FOR_HSW)
    switch (cpu_type()) {
        case CpuType::None:                        break;
        case CpuType::HSW: run = hsw::run_program; break;
        case CpuType::SKX: run = hsw::run_program; break;
    }
#endif
#if defined(TEST_FOR_SKX)
    switch (cpu_type()) {
        case CpuType::None:                        break;
        case CpuType::HSW:                         break;
        case CpuType::SKX: run = skx::run_program; break;
    }
#endif
    run(program, arguments, (const char*)src, (char*)dst, n, src_bpp,dst_bpp);
    return true;
}

static void assert_usable_as_destination(const skcms_ICCProfile* profile) {
#if defined(NDEBUG)
    (void)profile;
#else
    skcms_Matrix3x3 fromXYZD50;
    skcms_TransferFunction invR, invG, invB;
    assert(prep_for_destination(profile, &fromXYZD50, &invR, &invG, &invB));
#endif
}

bool skcms_MakeUsableAsDestination(skcms_ICCProfile* profile) {
    skcms_Matrix3x3 fromXYZD50;
    if (!profile->has_trc || !profile->has_toXYZD50
        || !skcms_Matrix3x3_invert(&profile->toXYZD50, &fromXYZD50)) {
        return false;
    }

    skcms_TransferFunction tf[3];
    for (int i = 0; i < 3; i++) {
        skcms_TransferFunction inv;
        if (profile->trc[i].table_entries == 0
            && skcms_TransferFunction_invert(&profile->trc[i].parametric, &inv)) {
            tf[i] = profile->trc[i].parametric;
            continue;
        }

        float max_error;
        // Parametric curves from skcms_ApproximateCurve() are guaranteed to be invertible.
        if (!skcms_ApproximateCurve(&profile->trc[i], &tf[i], &max_error)) {
            return false;
        }
    }

    for (int i = 0; i < 3; ++i) {
        profile->trc[i].table_entries = 0;
        profile->trc[i].parametric = tf[i];
    }

    assert_usable_as_destination(profile);
    return true;
}

bool skcms_MakeUsableAsDestinationWithSingleCurve(skcms_ICCProfile* profile) {
    // Operate on a copy of profile, so we can choose the best TF for the original curves
    skcms_ICCProfile result = *profile;
    if (!skcms_MakeUsableAsDestination(&result)) {
        return false;
    }

    int best_tf = 0;
    float min_max_error = INFINITY_;
    for (int i = 0; i < 3; i++) {
        skcms_TransferFunction inv;
        if (!skcms_TransferFunction_invert(&result.trc[i].parametric, &inv)) {
            return false;
        }

        float err = 0;
        for (int j = 0; j < 3; ++j) {
            err = fmaxf_(err, max_roundtrip_error(&profile->trc[j], &inv));
        }
        if (min_max_error > err) {
            min_max_error = err;
            best_tf = i;
        }
    }

    for (int i = 0; i < 3; i++) {
        result.trc[i].parametric = result.trc[best_tf].parametric;
    }

    *profile = result;
    assert_usable_as_destination(profile);
    return true;
}
