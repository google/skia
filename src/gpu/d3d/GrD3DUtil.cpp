/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DUtil.h"

#include "src/gpu/GrDataUtils.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/sksl/SkSLCompiler.h"

bool GrDxgiFormatIsCompressed(DXGI_FORMAT format) {
    switch (format) {
        case DXGI_FORMAT_BC1_UNORM:
            return true;
        default:
            return false;
    }
    SkUNREACHABLE;
}
