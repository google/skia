/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
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

using namespace emscripten;

wgpu::ShaderModule createShaderModule(wgpu::Device device, const char* source) {
    // https://github.com/emscripten-core/emscripten/blob/da842597941f425e92df0b902d3af53f1bcc2713/system/include/webgpu/webgpu_cpp.h#L1415
    wgpu::ShaderModuleWGSLDescriptor wDesc;
    wDesc.source = source;
    wgpu::ShaderModuleDescriptor desc = {.nextInChain = &wDesc};
    return device.CreateShaderModule(&desc);
}

wgpu::RenderPipeline createRenderPipeline(wgpu::Device device, wgpu::ShaderModule vertexShader,
                                          wgpu::ShaderModule fragmentShader) {
    wgpu::ColorTargetState colorTargetState{};
    colorTargetState.format = wgpu::TextureFormat::BGRA8Unorm;

    wgpu::FragmentState fragmentState{};
    fragmentState.module = fragmentShader;
    fragmentState.entryPoint = "main"; // assumes main() is defined in fragment shader code
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTargetState;

    wgpu::PipelineLayoutDescriptor pl{};

    // Inspired by https://github.com/kainino0x/webgpu-cross-platform-demo/blob/4061dd13096580eb5525619714145087b0d5acf6/main.cpp#L129
    wgpu::RenderPipelineDescriptor pipelineDescriptor{};
    pipelineDescriptor.layout = device.CreatePipelineLayout(&pl);
    pipelineDescriptor.vertex.module = vertexShader;
    pipelineDescriptor.vertex.entryPoint = "main";  // assumes main() is defined in vertex code
    pipelineDescriptor.fragment = &fragmentState;
    pipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    return device.CreateRenderPipeline(&pipelineDescriptor);
}

wgpu::SwapChain getSwapChainForCanvas(wgpu::Device device, std::string canvasSelector, int width, int height) {
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

void drawPipeline(wgpu::Device device, wgpu::TextureView view, wgpu::RenderPipeline pipeline,
                  wgpu::Color clearColor) {
    wgpu::RenderPassColorAttachment attachment{};
    attachment.view = view;
    attachment.loadOp = wgpu::LoadOp::Clear;
    attachment.storeOp = wgpu::StoreOp::Store;
    attachment.clearColor = clearColor;

    wgpu::RenderPassDescriptor renderpass{};
    renderpass.colorAttachmentCount = 1;
    renderpass.colorAttachments = &attachment;
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderpass);
    pass.SetPipeline(pipeline);
    pass.Draw(3, // vertexCount
              1, // instanceCount
              0, // firstIndex
              0  // firstInstance
    );
    pass.EndPass();
    wgpu::CommandBuffer commands = encoder.Finish();
    device.GetQueue().Submit(1, &commands);
}

class WebGPUSurface {
public:
    WebGPUSurface(std::string canvasSelector, int width, int height) {
        fDevice = wgpu::Device::Acquire(emscripten_webgpu_get_device());
        fCanvasSwap = getSwapChainForCanvas(fDevice, canvasSelector, width, height);
    }

    wgpu::ShaderModule makeShader(std::string source) {
        return createShaderModule(fDevice, source.c_str());
    }

    wgpu::RenderPipeline makeRenderPipeline(wgpu::ShaderModule vertexShader,
                                            wgpu::ShaderModule fragmentShader) {
        return createRenderPipeline(fDevice, vertexShader, fragmentShader);
    }

    void drawPipeline(wgpu::RenderPipeline pipeline, float r, float g, float b, float a) {
        // We cannot cache the TextureView because it will be destroyed after use.
        ::drawPipeline(fDevice, fCanvasSwap.GetCurrentTextureView(), pipeline, {r, g, b, a});
    }

private:
    wgpu::Device fDevice;
    wgpu::SwapChain fCanvasSwap;
};

EMSCRIPTEN_BINDINGS(Skia) {
    class_<WebGPUSurface>("WebGPUSurface")
        .constructor<std::string, int, int>()
        .function("MakeShader", &WebGPUSurface::makeShader)
        .function("MakeRenderPipeline", &WebGPUSurface::makeRenderPipeline)
        .function("drawPipeline", &WebGPUSurface::drawPipeline);

    class_<wgpu::ShaderModule>("ShaderModule");
    class_<wgpu::RenderPipeline>("RenderPipeline");
}
