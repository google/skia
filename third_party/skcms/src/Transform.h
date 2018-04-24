/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Op is a type used by both Transform.c and Transform_inl.h.

#pragma once

#define FOREACH_Op(M) \
    M(noop)           \
    M(load_565)       \
    M(load_888)       \
    M(load_8888)      \
    M(load_1010102)   \
    M(load_161616)    \
    M(load_16161616)  \
    M(load_hhh)       \
    M(load_hhhh)      \
    M(load_fff)       \
    M(load_ffff)      \
    M(swap_rb)        \
    M(clamp)          \
    M(invert)         \
    M(force_opaque)   \
    M(premul)         \
    M(unpremul)       \
    M(matrix_3x3)     \
    M(matrix_3x4)     \
    M(lab_to_xyz)     \
    M(tf_r)           \
    M(tf_g)           \
    M(tf_b)           \
    M(tf_a)           \
    M(table_8_r)      \
    M(table_8_g)      \
    M(table_8_b)      \
    M(table_8_a)      \
    M(table_16_r)     \
    M(table_16_g)     \
    M(table_16_b)     \
    M(table_16_a)     \
    M(clut_3D_8)      \
    M(clut_3D_16)     \
    M(clut_4D_8)      \
    M(clut_4D_16)     \
    M(store_565)      \
    M(store_888)      \
    M(store_8888)     \
    M(store_1010102)  \
    M(store_161616)   \
    M(store_16161616) \
    M(store_hhh)      \
    M(store_hhhh)     \
    M(store_fff)      \
    M(store_ffff)

typedef enum {
    #define M(op) Op_##op,
    FOREACH_Op(M)
    #undef M
} Op;
