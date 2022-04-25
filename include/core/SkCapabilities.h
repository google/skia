/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCapabilities_DEFINED
#define SkCapabilities_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/sksl/SkSLVersion.h"

namespace skgpu::graphite { class Caps; }

class SK_API SkCapabilities : public SkNVRefCnt<SkCapabilities> {
public:
    static sk_sp<SkCapabilities> RasterBackend();

    SkSL::Version skslVersion() const { return fSkSLVersion; }

private:
    SkCapabilities() = default;

    SkSL::Version fSkSLVersion = SkSL::Version::k100;

    friend class GrCaps;
    friend class skgpu::graphite::Caps;
};

#endif
