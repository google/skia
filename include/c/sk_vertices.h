/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_vertices_DEFINED
#define sk_vertices_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API void sk_vertices_unref(sk_vertices_t* cvertices);
SK_C_API sk_vertices_t* sk_vertices_make_copy(sk_vertices_vertex_mode_t vmode, int vertexCount, const sk_point_t* positions, const sk_point_t* texs, const sk_color_t* colors, int indexCount, const uint16_t* indices);

SK_C_PLUS_PLUS_END_GUARD

#endif
