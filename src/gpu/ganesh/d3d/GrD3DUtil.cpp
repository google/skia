/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/d3d/GrD3DUtil.h"

#include "src/gpu/ganesh/GrDataUtils.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/d3d/GrD3DGpu.h"
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

std::wstring GrD3DMultiByteToWide(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), nullptr, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], size_needed);
    return wstr;
}
