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

#endif
