/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/vk/VulkanBackendContext.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/vk/VulkanExtensions.h"
#include "include/gpu/graphite/vk/VulkanGraphiteUtils.h"

#define WIDTH 200
#define HEIGHT 400

int main(int argc, char** argv) {
    // This will not run (since it's missing all the Vulkan setup code),
    // but it should compile and link to test the build system.
    skgpu::VulkanBackendContext backendContext;

    std::unique_ptr<skgpu::VulkanExtensions> extensions(new skgpu::VulkanExtensions());
    backendContext.fInstance = VK_NULL_HANDLE;
    backendContext.fDevice = VK_NULL_HANDLE;
    skgpu::graphite::ContextOptions options;

    // This call will fail if run, due to the context not being set up.
    std::unique_ptr<skgpu::graphite::Context> context =
        skgpu::graphite::ContextFactory::MakeVulkan(backendContext, options);
    if (!context) {
        printf("Could not make Graphite Native Vulkan context\n");
        return 1;
    }
    printf("Context made, now to make the surface\n");

    SkImageInfo imageInfo =
            SkImageInfo::Make(WIDTH, HEIGHT, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();
    if (!recorder) {
        printf("Could not make recorder\n");
        return 1;
    }
    sk_sp<SkSurface> surface =
            SkSurfaces::RenderTarget(recorder.get(), imageInfo);
    if (!surface) {
        printf("Could not make surface from Metal Recorder\n");
        return 1;
    }

    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorLTGRAY);
    SkRRect rrect = SkRRect::MakeRectXY(SkRect::MakeLTRB(10, 20, 50, 70), 10, 10);

    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    paint.setAntiAlias(true);

    canvas->drawRRect(rrect, paint);

    // There would need to be vulkan cleanup here.
    return 0;
}
