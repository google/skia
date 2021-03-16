/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurfacePriv_DEFINED
#define SkSurfacePriv_DEFINED

#include "include/core/SkMath.h"
#include "include/core/SkSurfaceProps.h"

struct SkImageInfo;

static inline SkSurfaceProps SkSurfacePropsCopyOrDefault(const SkSurfaceProps* props) {
    return props ? *props : SkSurfaceProps();
}

enum SkSurfacePropsPrivateFlags {
    // Use internal MSAA to render to non-MSAA GPU surfaces.
    kDMSAA_SkSurfacePropsPrivateFlag = SkSurfaceProps::kAll_Flags + 1
};

static_assert(SkIsPow2(kDMSAA_SkSurfacePropsPrivateFlag));

static inline bool SkSurfacePropsIsDMSAA(const SkSurfaceProps& props) {
    return props.flags() & kDMSAA_SkSurfacePropsPrivateFlag;
};

constexpr size_t kIgnoreRowBytesValue = static_cast<size_t>(~0);

bool SkSurfaceValidateRasterInfo(const SkImageInfo&, size_t rb = kIgnoreRowBytesValue);

#endif
