/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVerticesPriv_DEFINED
#define SkVerticesPriv_DEFINED

#include "include/core/SkVertices.h"

struct SkVertices_DeprecatedBoneIndices { uint32_t values[4]; };
struct SkVertices_DeprecatedBoneWeights {    float values[4]; };
struct SkVertices_DeprecatedBone        {    float values[6]; };

struct SkVertices::Info {
    VertexMode      fMode;
    bool            fIsVolatile;

    int             fVertexCount,
                    fIndexCount,
                    fPerVertexDataCount;

    const SkPoint*  fPositions;
    const uint16_t* fIndices;

    // old version
    const SkPoint*  fTexCoords; // may be null
    const SkColor*  fColors;    // may be null

    // new, per-vertex-data, version
    const float*    fPerVertexData;

    bool hasTexCoords() const { return fTexCoords != nullptr; }
    bool hasColors() const { return fColors != nullptr; }
    bool hasPerVertexData() const { return fPerVertexData != nullptr; }
};

class SkVerticesPriv {
public:
    static int VertexCount(const SkVertices* v) { return v->fVertexCount; }

    static bool HasTexCoords(const SkVertices* v) { return v->fTexs != nullptr; }
    static bool HasColors(const SkVertices* v) { return v->fColors != nullptr; }
    static bool HasIndices(const SkVertices* v) { return v->fIndices != nullptr; }

    static SkVertices::VertexMode Mode(const SkVertices* v) { return v->fMode; }
};

#endif
