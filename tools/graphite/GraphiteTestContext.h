/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_graphite_test_GraphiteTestContext_DEFINED
#define sk_graphite_test_GraphiteTestContext_DEFINED

#include "experimental/graphite/include/GraphiteTypes.h"
#include "include/core/SkRefCnt.h"

namespace skgpu { class Context; }

namespace sk_graphite_test {

/**
 * An offscreen 3D context. This class is intended for Skia's internal testing needs and not
 * for general use.
 */
class GraphiteTestContext {
public:
    GraphiteTestContext(const GraphiteTestContext&) = delete;
    GraphiteTestContext& operator=(const GraphiteTestContext&) = delete;

    virtual ~GraphiteTestContext();

    virtual skgpu::BackendApi backend() = 0;

    virtual sk_sp<skgpu::Context> makeContext() = 0;

protected:
    GraphiteTestContext();
};


}  // namespace sk_graphite_test

#endif // sk_graphite_test_GraphiteTestContext_DEFINED
