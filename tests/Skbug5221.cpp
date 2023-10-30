/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorType.h"
#include "include/core/SkFont.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/fonts/FontToolUtils.h"

struct GrContextOptions;

// This passes by not crashing.
static void test(SkCanvas* canvas) {
    canvas->scale(63, 0);
    canvas->drawString("A", 50, 50, ToolUtils::DefaultFont(), SkPaint());
}

DEF_TEST(skbug5221, r) {
    sk_sp<SkSurface> surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(256, 256)));
    test(surface->getCanvas());
}

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(skbug5221_GPU, r, contextInfo, CtsEnforcement::kNever) {
    sk_sp<SkSurface> surface(SkSurfaces::RenderTarget(
            contextInfo.directContext(),
            skgpu::Budgeted::kYes,
            SkImageInfo::Make(256, 256, kRGBA_8888_SkColorType, kPremul_SkAlphaType)));
    test(surface->getCanvas());
}
