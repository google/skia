/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_matrix_DEFINED
#define sk_matrix_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

/** Set the matrix to identity */
SK_API void sk_matrix_set_identity(sk_matrix_t*);

/** Set the matrix to translate by (tx, ty). */
SK_API void sk_matrix_set_translate(sk_matrix_t*, float tx, float ty);
/**
    Preconcats the matrix with the specified translation.
        M' = M * T(dx, dy)
*/
SK_API void sk_matrix_pre_translate(sk_matrix_t*, float tx, float ty);
/**
    Postconcats the matrix with the specified translation.
        M' = T(dx, dy) * M
*/
SK_API void sk_matrix_post_translate(sk_matrix_t*, float tx, float ty);

/** Set the matrix to scale by sx and sy. */
SK_API void sk_matrix_set_scale(sk_matrix_t*, float sx, float sy);
/**
    Preconcats the matrix with the specified scale.
        M' = M * S(sx, sy)
*/
SK_API void sk_matrix_pre_scale(sk_matrix_t*, float sx, float sy);
/**
    Postconcats the matrix with the specified scale.
        M' = S(sx, sy) * M
*/
SK_API void sk_matrix_post_scale(sk_matrix_t*, float sx, float sy);

SK_C_PLUS_PLUS_END_GUARD

#endif
