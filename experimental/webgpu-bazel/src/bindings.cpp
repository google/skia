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
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"

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

enum class DemoKind {
    SOLID_COLOR,
    GRADIENT,
    RUNTIME_EFFECT,
};

struct DemoUniforms {
    float width;
    float height;
    float time;
};

class Demo final {
public:
    bool init(std::string canvasSelector, int width, int height) {
        GrContextOptions ctxOpts;

        wgpu::Device device = wgpu::Device::Acquire(emscripten_webgpu_get_device());
        sk_sp<GrDirectContext> context = GrDirectContext::MakeDawn(device, ctxOpts);
        if (!context) {
            SkDebugf("Could not create GrDirectContext\n");
            return false;
        }

        const char* sksl =
                "uniform float2 iResolution;"
                "uniform float iTime;"
                "vec2 d;"
                "float b(float a) {"
                "  return step(max(d.x, d.y), a);"
                "}"
                "half4 main(float2 C) {"
                "  vec4 O = vec4(0);"
                "  C.y = iResolution.y - C.y;"
                "  for (float i = 0; i < 3; ++i) {"
                "    vec2 U = C.yx / iResolution.yx;"
                "    U.y -= .5;"
                "    U.x = U.x * .4 + U.y * U.y;"
                "    U.y += U.x * sin(-iTime * 9. + i * 2. + U.x * 25.) * .2;"
                "    U.x -= asin(sin(U.y * 34.))/20.;"
                "    d = abs(U);"
                "    O += .3 * vec4(.8 * b(.3) + b(.2), b(.2), b(.1), -1.);"
                "  }"
                "  return O.xyz1;"
                "}";

        auto [effect, err] = SkRuntimeEffect::MakeForShader(SkString(sksl));
        if (!effect) {
            SkDebugf("Failed to compile SkSL: %s\n", err.c_str());
            return false;
        }

        fWidth = width;
        fHeight = height;
        fCanvasSwapChain = getSwapChainForCanvas(device, canvasSelector, width, height);
        fContext = context;
        fEffect = effect;

        return true;
    }

    void setKind(DemoKind kind) { fDemoKind = kind; }

    void draw(int timestamp) {
        GrDawnRenderTargetInfo rtInfo;
        rtInfo.fTextureView = fCanvasSwapChain.GetCurrentTextureView();
        rtInfo.fFormat = wgpu::TextureFormat::BGRA8Unorm;
        rtInfo.fLevelCount = 1;
        GrBackendRenderTarget backendRenderTarget(fWidth, fHeight, 1, 8, rtInfo);
        SkSurfaceProps surfaceProps(0, kRGB_H_SkPixelGeometry);

        sk_sp<SkSurface> surface = SkSurfaces::WrapBackendRenderTarget(fContext.get(),
                                                                       backendRenderTarget,
                                                                       kTopLeft_GrSurfaceOrigin,
                                                                       kN32_SkColorType,
                                                                       nullptr,
                                                                       &surfaceProps);

        SkPaint paint;
        if (fDemoKind == DemoKind::SOLID_COLOR) {
            drawSolidColor(&paint);
        } else if (fDemoKind == DemoKind::GRADIENT) {
            drawGradient(&paint);
        } else if (fDemoKind == DemoKind::RUNTIME_EFFECT) {
            drawRuntimeEffect(&paint, timestamp);
        }

        // Schedule the recorded commands and wait until the GPU has executed them.
        surface->getCanvas()->drawPaint(paint);
        fContext->flushAndSubmit(surface, true);
        fFrameCount++;
    }

    void drawSolidColor(SkPaint* paint) {
        bool flipColor = fFrameCount % 2 == 0;
        paint->setColor(flipColor ? SK_ColorCYAN : SK_ColorMAGENTA);
    }

    void drawGradient(SkPaint* paint) {
        bool flipColor = fFrameCount % 2 == 0;
        SkColor colors1[2] = {SK_ColorMAGENTA, SK_ColorCYAN};
        SkColor colors2[2] = {SK_ColorCYAN, SK_ColorMAGENTA};

        float x = (float)fWidth / 2.f;
        float y = (float)fHeight / 2.f;
        paint->setShader(SkGradientShader::MakeRadial(SkPoint::Make(x, y),
                                                      std::min(x, y),
                                                      flipColor ? colors1 : colors2,
                                                      nullptr,
                                                      2,
                                                      SkTileMode::kClamp));
    }

    void drawRuntimeEffect(SkPaint* paint, int timestamp) {
        DemoUniforms uniforms;
        uniforms.width = fWidth;
        uniforms.height = fHeight;
        uniforms.time = static_cast<float>(timestamp) / 1000.f;

        sk_sp<SkData> uniformData = SkData::MakeWithCopy(&uniforms, sizeof(uniforms));
        sk_sp<SkShader> shader = fEffect->makeShader(std::move(uniformData), /*children=*/{});
        paint->setShader(shader);
    }

private:
    int fFrameCount = 0;
    int fWidth;
    int fHeight;
    wgpu::SwapChain fCanvasSwapChain;
    sk_sp<GrDirectContext> fContext;
    sk_sp<SkRuntimeEffect> fEffect;
    DemoKind fDemoKind = DemoKind::SOLID_COLOR;
};

EMSCRIPTEN_BINDINGS(Skia) {
    emscripten::enum_<DemoKind>("DemoKind")
            .value("SOLID_COLOR", DemoKind::SOLID_COLOR)
            .value("GRADIENT", DemoKind::GRADIENT)
            .value("RUNTIME_EFFECT", DemoKind::RUNTIME_EFFECT);
    emscripten::class_<Demo>("Demo")
            .constructor()
            .function("init", &Demo::init)
            .function("setKind", &Demo::setKind)
            .function("draw", &Demo::draw);
}
