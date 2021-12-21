/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkCustomMeshPriv.h"

std::unique_ptr<const char[]> SkCopyCustomMeshVB(const SkCustomMesh& cm) {
    SkASSERT(cm.spec);
    size_t size = cm.spec->stride()*cm.vcount;

    auto vb = std::make_unique<char[]>(size);
    std::memcpy(vb.get(), cm.vb, size);

    return std::move(vb);
}
