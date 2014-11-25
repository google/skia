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

void sk_matrix_set_identity(sk_matrix_t*);

void sk_matrix_set_translate(sk_matrix_t*, float tx, float ty);
void sk_matrix_pre_translate(sk_matrix_t*, float tx, float ty);
void sk_matrix_post_translate(sk_matrix_t*, float tx, float ty);

void sk_matrix_set_scale(sk_matrix_t*, float sx, float sy);
void sk_matrix_pre_scale(sk_matrix_t*, float sx, float sy);
void sk_matrix_post_scale(sk_matrix_t*, float sx, float sy);

SK_C_PLUS_PLUS_END_GUARD

#endif
