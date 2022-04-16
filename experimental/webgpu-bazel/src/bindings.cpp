/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"

#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
// https://github.com/emscripten-core/emscripten/blob/main/system/include/emscripten/html5_webgpu.h
// The import/export functions defined here should allow us to fetch a handle to a given JS
// Texture/Sampler/Device etc if needed.
#include <emscripten/html5_webgpu.h>
// https://github.com/emscripten-core/emscripten/blob/main/system/include/webgpu/webgpu.h
// This defines WebGPU constants and such. It also includes a lot of typedefs that make something
// like WGPUDevice defined as a pointer to something external. These "pointers" are actually just
// a small integer that refers to an array index of JS objects being held by a "manager"
// https://github.com/emscripten-core/emscripten/blob/f47bef371f3464471c6d30b631cffcdd06ced004/src/library_webgpu.js#L192
#include <webgpu/webgpu.h>
// https://github.com/emscripten-core/emscripten/blob/main/system/include/webgpu/webgpu_cpp.h
// This defines the C++ equivalents to the JS WebGPU API.
#include <webgpu/webgpu_cpp.h>

static wgpu::SwapChain getSwapChainForCanvas(wgpu::Device device,
                                             std::string canvasSelector,
                                             int width,
                                             int height) {
    wgpu::SurfaceDescriptorFromCanvasHTMLSelector surfaceSelector;
    surfaceSelector.selector = canvasSelector.c_str();

    wgpu::SurfaceDescriptor surface_desc;
    surface_desc.nextInChain = &surfaceSelector;
    wgpu::Instance instance;
    wgpu::Surface surface = instance.CreateSurface(&surface_desc);

    wgpu::SwapChainDescriptor swap_chain_desc;
    swap_chain_desc.format = wgpu::TextureFormat::BGRA8Unorm;
    swap_chain_desc.usage = wgpu::TextureUsage::RenderAttachment;
    swap_chain_desc.presentMode = wgpu::PresentMode::Fifo;
    swap_chain_desc.width = width;
    swap_chain_desc.height = height;
    return device.CreateSwapChain(surface, &swap_chain_desc);
}

bool drawWithSkia(std::string canvasSelector, int width, int height, bool flipColor) {
    GrContextOptions ctxOpts;

    wgpu::Device device = wgpu::Device::Acquire(emscripten_webgpu_get_device());
    sk_sp<GrDirectContext> context = GrDirectContext::MakeDawn(device, ctxOpts);
    if (!context) {
        SkDebugf("Could not create GrDirectContext\n");
        return false;
    }

    wgpu::SwapChain canvasSwap = getSwapChainForCanvas(device, canvasSelector, width, height);

    GrDawnRenderTargetInfo rtInfo;
    rtInfo.fTextureView = canvasSwap.GetCurrentTextureView();
    rtInfo.fFormat = wgpu::TextureFormat::BGRA8Unorm;
    rtInfo.fLevelCount = 1;
    GrBackendRenderTarget backendRenderTarget(width, height, 1, 8, rtInfo);
    SkSurfaceProps surfaceProps(0, kRGB_H_SkPixelGeometry);

    sk_sp<SkSurface> surface = SkSurface::MakeFromBackendRenderTarget(context.get(),
                                                                      backendRenderTarget,
                                                                      kTopLeft_GrSurfaceOrigin,
                                                                      kN32_SkColorType,
                                                                      nullptr,
                                                                      &surfaceProps);

    SkPaint paint;
    paint.setColor(flipColor ? SK_ColorCYAN : SK_ColorMAGENTA);
    surface->getCanvas()->drawPaint(paint);

    // Schedule the recorded commands and wait until the GPU has executed them.
    surface->flushAndSubmit(true);
    return true;
}

EMSCRIPTEN_BINDINGS(Skia) {
    emscripten::function("drawWithSkia", drawWithSkia);
}
