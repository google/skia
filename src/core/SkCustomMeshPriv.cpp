/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkCustomMeshPriv.h"

#ifdef SK_ENABLE_SKSL

static int min_vcount_for_mode(SkCustomMesh::Mode mode) {
    switch (mode) {
        case SkCustomMesh::Mode::kTriangles:     return 3;
        case SkCustomMesh::Mode::kTriangleStrip: return 3;
    }
    SkUNREACHABLE;
}

bool SkValidateCustomMesh(const SkCustomMesh& cm) {
    if (!cm.spec) {
        return false;
    }

    if (!cm.vb) {
        return false;
    }

    if (cm.vcount <= 0) {
        return false;
    }

    if (cm.indices) {
        if (cm.icount < min_vcount_for_mode(cm.mode)) {
            return false;
        }
    } else {
        if (cm.icount > 0) {
            return false;
        }
    }

    return true;
}

std::unique_ptr<const char[]> SkCopyCustomMeshVB(const SkCustomMesh& cm) {
    SkASSERT(cm.spec);
    size_t size = cm.spec->stride()*cm.vcount;

    auto vb = std::make_unique<char[]>(size);
    std::memcpy(vb.get(), cm.vb, size);

    return std::move(vb);
}

std::unique_ptr<const uint16_t[]> SkCopyCustomMeshIB(const SkCustomMesh& cm) {
    SkASSERT(cm.spec);
    SkASSERT(SkToBool(cm.indices) == SkToBool(cm.icount));
    if (!cm.indices) {
        return nullptr;
    }
    auto ib = std::make_unique<uint16_t[]>(cm.icount);
    std::copy_n(cm.indices, cm.icount, ib.get());

    return std::move(ib);
}

#endif
