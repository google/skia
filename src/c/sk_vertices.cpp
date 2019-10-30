/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkVertices.h"

#include "include/c/sk_vertices.h"

#include "src/c/sk_types_priv.h"

void sk_vertices_unref(sk_vertices_t* cvertices) {
    SkSafeUnref(AsVertices(cvertices));
}

void sk_vertices_ref(sk_vertices_t* cvertices) {
    SkSafeRef(AsVertices(cvertices));
}

sk_vertices_t* sk_vertices_make_copy(sk_vertices_vertex_mode_t vmode, int vertexCount, const sk_point_t* positions, const sk_point_t* texs, const sk_color_t* colors, int indexCount, const uint16_t* indices) {
    return ToVertices(SkVertices::MakeCopy(
        (SkVertices::VertexMode)vmode, vertexCount, AsPoint(positions), AsPoint(texs), colors, indexCount, indices).release());
}
