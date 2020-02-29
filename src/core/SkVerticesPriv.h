/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVerticesPriv_DEFINED
#define SkVerticesPriv_DEFINED

#include "include/core/SkVertices.h"

class SkVerticesPriv {
public:
    static SkVertices::VertexMode Mode(const SkVertices* v) { return v->mode(); }

    static bool HasColors(const SkVertices* v) { return v->hasColors(); }
    static bool HasTexCoords(const SkVertices* v) { return v->hasTexCoords(); }
    static bool HasBones(const SkVertices* v) { return v->hasBones(); }
    static bool HasIndices(const SkVertices* v) { return v->hasIndices(); }

    static int VertexCount(const SkVertices* v) { return v->vertexCount(); }
    static const SkPoint* Positions(const SkVertices* v) { return v->positions(); }
    static const SkPoint* TexCoords(const SkVertices* v) { return v->texCoords(); }
    static const SkColor* Colors(const SkVertices* v) { return v->colors(); }

    static const SkVertices::BoneIndices* BoneIndices(const SkVertices* v) { return v->boneIndices(); }
    static const SkVertices::BoneWeights* BoneWeights(const SkVertices* v) { return v->boneWeights(); }

    static int IndexCount(const SkVertices* v) { return v->indexCount(); }
    static const uint16_t* Indices(const SkVertices* v) { return v->indices(); }
};

#endif
