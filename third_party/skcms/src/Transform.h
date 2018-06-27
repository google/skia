/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Op is a type used by both Transform.c and Transform_inl.h.

#pragma once

typedef enum {
    Op_noop,

    Op_load_565,
    Op_load_888,
    Op_load_8888,
    Op_load_1010102,
    Op_load_161616,
    Op_load_16161616,
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
    Op_table_8_r,
    Op_table_8_g,
    Op_table_8_b,
    Op_table_8_a,
    Op_table_16_r,
    Op_table_16_g,
    Op_table_16_b,
    Op_table_16_a,

    Op_clut_3D_8,
    Op_clut_3D_16,
    Op_clut_4D_8,
    Op_clut_4D_16,

    Op_store_565,
    Op_store_888,
    Op_store_8888,
    Op_store_1010102,
    Op_store_161616,
    Op_store_16161616,
    Op_store_hhh,
    Op_store_hhhh,
    Op_store_fff,
    Op_store_ffff,
} Op;
