/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrMeshDrawTarget.h"

#include "src/gpu/GrResourceProvider.h"

uint32_t GrMeshDrawTarget::contextUniqueID() const {
    return this->resourceProvider()->contextUniqueID();
}
