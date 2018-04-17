/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../skcms.h"
#include "Macros.h"
#include "PortableMath.h"
#include "RandomBytes.h"
#include "TransferFunction.h"
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

// A macro so that it can be used in the initialization of skcms_sRGB_profile.
// Think, static uint32_t make_signature(uint8_t, uint8_t, uint8_t, uint8_t).
#define make_signature(a,b,c,d) \
    ( (uint32_t)((a) << 24) | (uint32_t)((b) << 16) | (uint32_t)((c) << 8) | (uint32_t)((d) << 0) )

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
    if (tag->type != make_signature('X','Y','Z',' ') || tag->size < SAFE_SIZEOF(XYZ_Layout)) {
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

typedef struct {
    uint8_t type          [4];
    uint8_t reserved_a    [4];
    uint8_t function_type [2];
    uint8_t reserved_b    [2];
    uint8_t parameters    [ ];  // 1, 3, 4, 5, or 7 s15.16 parameters, depending on function_type
} para_Layout;

static bool read_curve_para(const uint8_t* buf, uint32_t size,
                            skcms_Curve* curve, uint32_t* curve_size) {
    if (size < SAFE_SIZEOF(para_Layout)) {
        return false;
    }

    const para_Layout* paraTag = (const para_Layout*)buf;

    enum { kG = 0, kGAB = 1, kGABC = 2, kGABCD = 3, kGABCDEF = 4 };
    uint16_t function_type = read_big_u16(paraTag->function_type);
    if (function_type > kGABCDEF) {
        return false;
    }

    static const uint32_t curve_bytes[] = { 4, 12, 16, 20, 28 };
    if (size < SAFE_SIZEOF(para_Layout) + curve_bytes[function_type]) {
        return false;
    }

    if (curve_size) {
        *curve_size = SAFE_SIZEOF(para_Layout) + curve_bytes[function_type];
    }

    curve->table_entries = 0;
    curve->parametric.a  = 1.0f;
    curve->parametric.b  = 0.0f;
    curve->parametric.c  = 0.0f;
    curve->parametric.d  = 0.0f;
    curve->parametric.e  = 0.0f;
    curve->parametric.f  = 0.0f;
    curve->parametric.g  = read_big_fixed(paraTag->parameters);

    switch (function_type) {
        case kGAB:
            curve->parametric.a = read_big_fixed(paraTag->parameters + 4);
            curve->parametric.b = read_big_fixed(paraTag->parameters + 8);
            if (curve->parametric.a == 0) {
                return false;
            }
            curve->parametric.d = -curve->parametric.b / curve->parametric.a;
            break;
        case kGABC:
            curve->parametric.a = read_big_fixed(paraTag->parameters + 4);
            curve->parametric.b = read_big_fixed(paraTag->parameters + 8);
            curve->parametric.e = read_big_fixed(paraTag->parameters + 12);
            if (curve->parametric.a == 0) {
                return false;
            }
            curve->parametric.d = -curve->parametric.b / curve->parametric.a;
            curve->parametric.f = curve->parametric.e;
            break;
        case kGABCD:
            curve->parametric.a = read_big_fixed(paraTag->parameters + 4);
            curve->parametric.b = read_big_fixed(paraTag->parameters + 8);
            curve->parametric.c = read_big_fixed(paraTag->parameters + 12);
            curve->parametric.d = read_big_fixed(paraTag->parameters + 16);
            break;
        case kGABCDEF:
            curve->parametric.a = read_big_fixed(paraTag->parameters + 4);
            curve->parametric.b = read_big_fixed(paraTag->parameters + 8);
            curve->parametric.c = read_big_fixed(paraTag->parameters + 12);
            curve->parametric.d = read_big_fixed(paraTag->parameters + 16);
            curve->parametric.e = read_big_fixed(paraTag->parameters + 20);
            curve->parametric.f = read_big_fixed(paraTag->parameters + 24);
            break;
    }
    return true;
}

typedef struct {
    uint8_t type          [4];
    uint8_t reserved      [4];
    uint8_t value_count   [4];
    uint8_t parameters    [ ];  // value_count parameters (8.8 if 1, uint16 (n*65535) if > 1)
} curv_Layout;

static bool read_curve_curv(const uint8_t* buf, uint32_t size,
                            skcms_Curve* curve, uint32_t* curve_size) {
    if (size < SAFE_SIZEOF(curv_Layout)) {
        return false;
    }

    const curv_Layout* curvTag = (const curv_Layout*)buf;

    uint32_t value_count = read_big_u32(curvTag->value_count);
    if (size < SAFE_SIZEOF(curv_Layout) + value_count * SAFE_SIZEOF(uint16_t)) {
        return false;
    }

    if (curve_size) {
        *curve_size = SAFE_SIZEOF(curv_Layout) + value_count * SAFE_SIZEOF(uint16_t);
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
            curve->parametric.g = read_big_u16(curvTag->parameters) * (1.0f / 256.0f);
        }
    } else {
        curve->table_8       = NULL;
        curve->table_16      = curvTag->parameters;
        curve->table_entries = value_count;
    }

    return true;
}

// Parses both curveType and parametricCurveType data. Ensures that at most 'size' bytes are read.
// If curve_size is not NULL, writes the number of bytes used by the curve in (*curve_size).
static bool read_curve(const uint8_t* buf, uint32_t size,
                       skcms_Curve* curve, uint32_t* curve_size) {
    if (!buf || size < 4 || !curve) {
        return false;
    }

    uint32_t type = read_big_u32(buf);
    if (type == make_signature('p', 'a', 'r', 'a')) {
        return read_curve_para(buf, size, curve, curve_size);
    } else if (type == make_signature('c', 'u', 'r', 'v')) {
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
    mft_CommonLayout common      [ 1];

    uint8_t tables               [  ];
} mft1_Layout;

typedef struct {
    mft_CommonLayout common      [ 1];

    uint8_t input_table_entries  [ 2];
    uint8_t output_table_entries [ 2];
    uint8_t tables               [  ];
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
            a2b->input_curves[i].table_16 = NULL;
        } else {
            a2b->input_curves[i].table_8  = NULL;
            a2b->input_curves[i].table_16 = table_base + i * byte_len_per_input_table;
        }
    }

    if (byte_width == 1) {
        a2b->grid_8  = table_base + byte_len_all_input_tables;
        a2b->grid_16 = NULL;
    } else {
        a2b->grid_8  = NULL;
        a2b->grid_16 = table_base + byte_len_all_input_tables;
    }

    const uint8_t* output_table_base = table_base + byte_len_all_input_tables + grid_size;
    for (uint32_t i = 0; i < a2b->output_channels; ++i) {
        a2b->output_curves[i].table_entries = output_table_entries;
        if (byte_width == 1) {
            a2b->output_curves[i].table_8  = output_table_base + i * byte_len_per_output_table;
            a2b->output_curves[i].table_16 = NULL;
        } else {
            a2b->output_curves[i].table_8  = NULL;
            a2b->output_curves[i].table_16 = output_table_base + i * byte_len_per_output_table;
        }
    }

    return true;
}

static bool read_tag_mft1(const skcms_ICCTag* tag, skcms_A2B* a2b) {
    if (tag->size < SAFE_SIZEOF(mft1_Layout)) {
        return false;
    }

    const mft1_Layout* mftTag = (const mft1_Layout*)tag->buf;
    if (!read_mft_common(mftTag->common, a2b)) {
        return false;
    }

    uint32_t input_table_entries  = 256;
    uint32_t output_table_entries = 256;

    return init_a2b_tables(mftTag->tables, tag->size - SAFE_SIZEOF(mft1_Layout), 1,
                           input_table_entries, output_table_entries, a2b);
}

static bool read_tag_mft2(const skcms_ICCTag* tag, skcms_A2B* a2b) {
    if (tag->size < SAFE_SIZEOF(mft2_Layout)) {
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

    return init_a2b_tables(mftTag->tables, tag->size - SAFE_SIZEOF(mft2_Layout), 2,
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
    uint8_t data                 [  ];
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

        if (tag->size < clut_offset + SAFE_SIZEOF(mABCLUT_Layout)) {
            return false;
        }
        const mABCLUT_Layout* clut = (const mABCLUT_Layout*)(tag->buf + clut_offset);

        if (clut->grid_byte_width[0] == 1) {
            a2b->grid_8  = clut->data;
            a2b->grid_16 = NULL;
        } else if (clut->grid_byte_width[0] == 2) {
            a2b->grid_8  = NULL;
            a2b->grid_16 = clut->data;
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
        if (tag->size < clut_offset + SAFE_SIZEOF(mABCLUT_Layout) + grid_size) {
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

static bool read_a2b(const skcms_ICCTag* tag, skcms_A2B* a2b, bool pcs_is_xyz) {
    bool ok = false;
    if (tag->type == make_signature('m', 'f', 't', '1')) {
        ok = read_tag_mft1(tag, a2b);
    } else if (tag->type == make_signature('m', 'f', 't', '2')) {
        ok = read_tag_mft2(tag, a2b);
    } else if (tag->type == make_signature('m', 'A', 'B', ' ')) {
        ok = read_tag_mab(tag, a2b, pcs_is_xyz);
    }
    if (!ok) {
        return false;
    }

    // Detect and canonicalize identity tables.
    skcms_Curve* curves[] = {
        a2b->input_channels  > 0 ? a2b->input_curves  + 0 : NULL,
        a2b->input_channels  > 1 ? a2b->input_curves  + 1 : NULL,
        a2b->input_channels  > 2 ? a2b->input_curves  + 2 : NULL,
        a2b->input_channels  > 3 ? a2b->input_curves  + 3 : NULL,
        a2b->matrix_channels > 0 ? a2b->matrix_curves + 0 : NULL,
        a2b->matrix_channels > 1 ? a2b->matrix_curves + 1 : NULL,
        a2b->matrix_channels > 2 ? a2b->matrix_curves + 2 : NULL,
        a2b->output_channels > 0 ? a2b->output_curves + 0 : NULL,
        a2b->output_channels > 1 ? a2b->output_curves + 1 : NULL,
        a2b->output_channels > 2 ? a2b->output_curves + 2 : NULL,
    };

    for (int i = 0; i < ARRAY_COUNT(curves); i++) {
        skcms_Curve* curve = curves[i];

        if (curve && curve->table_entries && curve->table_entries <= (uint32_t)INT_MAX) {
            int N = (int)curve->table_entries;

            skcms_TransferFunction tf;
            if (N == skcms_fit_linear(curve, N, 1.0f/(2*N), &tf)
                && tf.c == 1.0f
                && tf.f == 0.0f) {
                curve->table_entries = 0;
                curve->table_8       = NULL;
                curve->table_16      = NULL;
                curve->parametric    = (skcms_TransferFunction){1,1,0,0,0,0,0};
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
    const header_Layout* header = buf;
    profile->buffer              = buf;
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
    if (signature != make_signature('a', 'c', 's', 'p') ||
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

    if (profile->pcs != make_signature('X', 'Y', 'Z', ' ') &&
        profile->pcs != make_signature('L', 'a', 'b', ' ')) {
        return false;
    }

    bool pcs_is_xyz = profile->pcs == make_signature('X', 'Y', 'Z', ' ');

    // Pre-parse commonly used tags.
    skcms_ICCTag kTRC;
    if (profile->data_color_space == make_signature('G', 'R', 'A', 'Y') &&
        skcms_GetTagBySignature(profile, make_signature('k', 'T', 'R', 'C'), &kTRC)) {
        if (!read_curve(kTRC.buf, kTRC.size, &profile->trc[0], NULL)) {
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
        if (skcms_GetTagBySignature(profile, make_signature('r', 'T', 'R', 'C'), &rTRC) &&
            skcms_GetTagBySignature(profile, make_signature('g', 'T', 'R', 'C'), &gTRC) &&
            skcms_GetTagBySignature(profile, make_signature('b', 'T', 'R', 'C'), &bTRC)) {
            if (!read_curve(rTRC.buf, rTRC.size, &profile->trc[0], NULL) ||
                !read_curve(gTRC.buf, gTRC.size, &profile->trc[1], NULL) ||
                !read_curve(bTRC.buf, bTRC.size, &profile->trc[2], NULL)) {
                // Malformed TRC tags
                return false;
            }
            profile->has_trc = true;
        }

        skcms_ICCTag rXYZ, gXYZ, bXYZ;
        if (skcms_GetTagBySignature(profile, make_signature('r', 'X', 'Y', 'Z'), &rXYZ) &&
            skcms_GetTagBySignature(profile, make_signature('g', 'X', 'Y', 'Z'), &gXYZ) &&
            skcms_GetTagBySignature(profile, make_signature('b', 'X', 'Y', 'Z'), &bXYZ)) {
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
    const uint32_t sigs[] = { make_signature('A','2','B','0'), make_signature('A','2','B','1') };
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

    return true;
}

const skcms_ICCProfile skcms_sRGB_profile = {
    // These fields are moot when not a skcms_Parse()'d profile.
    .buffer    = NULL,
    .size      =    0,
    .tag_count =    0,

    // We choose to represent sRGB with its canonical transfer function,
    // and with its canonical XYZD50 gamut matrix.
    .data_color_space = make_signature('R', 'G', 'B', ' '),
    .pcs              = make_signature('X', 'Y', 'Z', ' '),
    .has_trc      = true,
    .has_toXYZD50 = true,
    .has_A2B      = false,

    .trc = {
        {{0, {2.4f, (float)(1/1.055), (float)(0.055/1.055), (float)(1/12.92), 0.04045f, 0, 0 }}},
        {{0, {2.4f, (float)(1/1.055), (float)(0.055/1.055), (float)(1/12.92), 0.04045f, 0, 0 }}},
        {{0, {2.4f, (float)(1/1.055), (float)(0.055/1.055), (float)(1/12.92), 0.04045f, 0, 0 }}},
    },

    .toXYZD50 = {{
        { 0.436065674f, 0.385147095f, 0.143066406f },
        { 0.222488403f, 0.716873169f, 0.060607910f },
        { 0.013916016f, 0.097076416f, 0.714096069f },
    }},
};

const skcms_ICCProfile skcms_XYZD50_profile = {
    .buffer           = NULL,
    .size             = 0,
    .tag_count        = 0,

    .data_color_space = make_signature('R', 'G', 'B', ' '),
    .pcs              = make_signature('X', 'Y', 'Z', ' '),
    .has_trc          = true,
    .has_toXYZD50     = true,
    .has_A2B          = false,

    .trc = {
        {{0, {1,1,0,0,0,0,0}}},
        {{0, {1,1,0,0,0,0,0}}},
        {{0, {1,1,0,0,0,0,0}}},
    },

    .toXYZD50 = {{
        {1,0,0},
        {0,1,0},
        {0,0,1},
    }},
};

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
    // For now this is the essentially the same strategy we use in test_only.c
    // for our skcms_Transform() smoke tests:
    //    1) transform A to XYZD50
    //    2) transform B to XYZD50
    //    3) return true if they're similar enough
    // Our current criterion in 3) is maximum 1 bit error per XYZD50 byte.

    // Here are 252 of a random shuffle of all possible bytes.
    // 252 is evenly divisible by 3 and 4.  Only 192, 10, 241, and 43 are missing.

    if (A->data_color_space != B->data_color_space) {
        return false;
    }

    // Interpret as RGB_888 if data color space is RGB or GRAY, RGBA_8888 if CMYK.
    skcms_PixelFormat fmt = skcms_PixelFormat_RGB_888;
    size_t npixels = 84;
    if (A->data_color_space == make_signature('C', 'M', 'Y', 'K')) {
        fmt = skcms_PixelFormat_RGBA_8888;
        npixels = 63;
    }

    uint8_t dstA[252],
            dstB[252];
    if (!skcms_Transform(
                skcms_252_random_bytes,     fmt, skcms_AlphaFormat_Unpremul, A,
                dstA, skcms_PixelFormat_RGB_888, skcms_AlphaFormat_Unpremul, &skcms_XYZD50_profile,
                npixels)) {
        return false;
    }
    if (!skcms_Transform(
                skcms_252_random_bytes,     fmt, skcms_AlphaFormat_Unpremul, B,
                dstB, skcms_PixelFormat_RGB_888, skcms_AlphaFormat_Unpremul, &skcms_XYZD50_profile,
                npixels)) {
        return false;
    }

    for (size_t i = 0; i < 252; i++) {
        if (abs((int)dstA[i] - (int)dstB[i]) > 1) {
            return false;
        }
    }
    return true;
}
