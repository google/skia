/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProcessorTestData_DEFINED
#define GrProcessorTestData_DEFINED

#include "include/core/SkTypes.h"

#if GR_TEST_UTILS

#include <memory>
#include <tuple>

#include "include/core/SkImageInfo.h"
#include "include/private/GrTypesPriv.h"
#include "include/private/SkTArray.h"
#include "src/core/SkArenaAlloc.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrSurfaceProxyView.h"

class SkRandom;
class GrRecordingContext;
class GrProxyProvider;

/*
 * GrProcessorTestData is an argument struct to TestCreate functions.
 * fViews are valid surface proxies that can be used to construct a GrTextureEffect if one is
 * desired. The first texture has a RGBA8 format and the second has Alpha8 format.
 * TestCreate functions are also free to create additional textures using the GrContext.
 */
class GrProcessorTestData {
public:
    using ViewInfo = std::tuple<GrSurfaceProxyView, GrColorType, SkAlphaType>;
    GrProcessorTestData(SkRandom*, GrRecordingContext*, int numProxies, const ViewInfo[]);

    GrRecordingContext* context() { return fContext; }
    GrProxyProvider* proxyProvider();
    const GrCaps* caps();
    SkArenaAlloc* allocator() { return fArena.get(); }

    ViewInfo randomView();
    ViewInfo randomAlphaOnlyView();

    SkRandom* fRandom;

private:
    GrRecordingContext* fContext;
    SkTArray<ViewInfo> fViews;
    std::unique_ptr<SkArenaAlloc> fArena;
};

#endif  // GR_TEST_UTILS
#endif  // GrProcessorTestData_DEFINED
