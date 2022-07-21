/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ContextUtils.h"

#include <string>
#include "include/private/SkSLString.h"
#include "include/private/SkUniquePaintParamsID.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkKeyContext.h"
#include "src/core/SkPipelineData.h"
#include "src/core/SkShaderCodeDictionary.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/UniformManager.h"

namespace skgpu::graphite {

std::tuple<SkUniquePaintParamsID, UniformDataCache::Index, TextureDataCache::Index>
ExtractPaintData(Recorder* recorder,
                 SkPipelineDataGatherer* gatherer,
                 SkPaintParamsKeyBuilder* builder,
                 const SkM44& dev2Local,
                 const PaintParams& p) {

    SkDEBUGCODE(gatherer->checkReset());
    SkDEBUGCODE(builder->checkReset());

    SkKeyContext keyContext(recorder, dev2Local);

    p.toKey(keyContext, builder, gatherer);

    auto dict = recorder->priv().resourceProvider()->shaderCodeDictionary();
    UniformDataCache* uniformDataCache = recorder->priv().uniformDataCache();
    TextureDataCache* textureDataCache = recorder->priv().textureDataCache();

    auto entry = dict->findOrCreate(builder);
    UniformDataCache::Index uniformIndex;
    if (gatherer->hasUniforms()) {
        uniformIndex = uniformDataCache->insert(gatherer->peekUniformData());
    }
    TextureDataCache::Index textureIndex;
    if (gatherer->hasTextures()) {
        textureIndex = textureDataCache->insert(gatherer->textureDataBlock());
    }

    gatherer->reset();

    return { entry->uniqueID(), uniformIndex, textureIndex };
}

std::tuple<UniformDataCache::Index, TextureDataCache::Index>
ExtractRenderStepData(UniformDataCache* geometryUniformDataCache,
                      TextureDataCache* textureDataCache,
                      SkPipelineDataGatherer* gatherer,
                      const RenderStep* step,
                      const DrawParams& params) {
    SkDEBUGCODE(gatherer->checkReset());

    step->writeUniformsAndTextures(params, gatherer);

    UniformDataCache::Index uIndex = geometryUniformDataCache->insert(gatherer->peekUniformData());

    TextureDataCache::Index textureIndex;
    if (step->hasTextures()) {
        textureIndex = textureDataCache->insert(gatherer->textureDataBlock());
    }

    gatherer->reset();

    return { uIndex, textureIndex };
}

namespace {
std::string get_uniform_header(int bufferID, const char* name) {
    std::string result;

    SkSL::String::appendf(&result, "layout (binding=%d) uniform %sUniforms {\n", bufferID, name);

    return result;
}

std::string get_uniforms(SkSpan<const SkUniform> uniforms, int* offset, int manglingSuffix) {
    std::string result;
    UniformOffsetCalculator offsetter(Layout::kMetal, *offset);

    for (const SkUniform& u : uniforms) {
        SkSL::String::appendf(&result, "    layout(offset=%zu) %s %s",
                              offsetter.calculateOffset(u.type(), u.count()),
                              SkSLTypeString(u.type()),
                              u.name());
        if (manglingSuffix >= 0) {
            result.append("_");
            result.append(std::to_string(manglingSuffix));
        }
        if (u.count()) {
            result.append("[");
            result.append(std::to_string(u.count()));
            result.append("]");
        }
        result.append(";\n");
    }

    *offset = offsetter.size();
    return result;
}
}  // anonymous namespace

std::string EmitPaintParamsUniforms(int bufferID,
                                    const char* name,
                                    const std::vector<SkPaintParamsKey::BlockReader>& readers,
                                    bool needsLocalCoords) {
    size_t numUniforms = 0;
    for (auto r : readers) {
        numUniforms += r.entry()->fUniforms.size();
    }

    if (!numUniforms) {
        return {};
    }

    int offset = 0;

    std::string result = get_uniform_header(bufferID, name);
    for (int i = 0; i < (int) readers.size(); ++i) {
        SkSL::String::appendf(&result,
                              "// %s\n",
                              readers[i].entry()->fName);
        result += get_uniforms(readers[i].entry()->fUniforms, &offset, i);
    }
    if (needsLocalCoords) {
        static constexpr SkUniform kDev2LocalUniform[] = {{ "dev2LocalUni", SkSLType::kFloat4x4 }};
        result += "// NeedsLocalCoords\n";
        result += get_uniforms(SkSpan<const SkUniform>(kDev2LocalUniform, 1), &offset, -1);
    }
    result.append("};\n\n");

    return result;
}

std::string EmitRenderStepUniforms(int bufferID, const char* name,
                                   SkSpan<const SkUniform> uniforms) {
    int offset = 0;

    std::string result = get_uniform_header(bufferID, name);
    result += get_uniforms(uniforms, &offset, -1);
    result.append("};\n\n");

    return result;
}

std::string EmitTexturesAndSamplers(const std::vector<SkPaintParamsKey::BlockReader>& readers,
                                    int* binding) {
    std::string result;
    for (int i = 0; i < (int) readers.size(); ++i) {
        auto texturesAndSamplers = readers[i].entry()->fTexturesAndSamplers;

        for (int j = 0; j < (int) texturesAndSamplers.size(); ++j) {
            const SkTextureAndSampler& t = texturesAndSamplers[j];
            SkSL::String::appendf(&result,
                                  "layout(binding=%d) uniform sampler2D %s_%d;\n",
                                  *binding, t.name(), i);
            (*binding)++;
        }
    }

    return result;
}

namespace {
std::string emit_attributes(SkSpan<const Attribute> vertexAttrs,
                            SkSpan<const Attribute> instanceAttrs) {
    std::string result;

    int attr = 0;
    auto add_attrs = [&](SkSpan<const Attribute> attrs) {
        for (auto a : attrs) {
            SkSL::String::appendf(&result, "    layout(location=%d) in ", attr++);
            result.append(SkSLTypeString(a.gpuType()));
            SkSL::String::appendf(&result, " %s;\n", a.name());
        }
    };

    if (!vertexAttrs.empty()) {
        result.append("// vertex attrs\n");
        add_attrs(vertexAttrs);
    }
    if (!instanceAttrs.empty()) {
        result.append("// instance attrs\n");
        add_attrs(instanceAttrs);
    }

    return result;
}
}  // anonymous namespace

std::string EmitVaryings(const RenderStep* step, const char* direction) {
    std::string result;

    int location = 0;
    for (auto v : step->varyings()) {
        SkSL::String::appendf(&result, "    layout(location=%d) %s ", location++, direction);
        result.append(SkSLTypeString(v.fType));
        SkSL::String::appendf(&result, " %s;\n", v.fName);
    }

    return result;
}

std::string GetSkSLVS(const GraphicsPipelineDesc& desc) {
    const RenderStep* step = desc.renderStep();
    // TODO: To more completely support end-to-end rendering, this will need to be updated so that
    // the RenderStep shader snippet can produce a device coord, a local coord, and depth.
    // If the paint combination doesn't need the local coord it can be ignored, otherwise we need
    // a varying for it. The fragment function's output will need to be updated to have a color and
    // the depth, or when there's no combination, just the depth. Lastly, we also should add the
    // static/intrinsic uniform binding point so that we can handle normalizing the device position
    // produced by the RenderStep automatically.

    // Fixed program header
    std::string sksl =
        "layout (binding=0) uniform intrinsicUniforms {\n"
        "    layout(offset=0) float4 rtAdjust;\n"
        "};\n"
        "\n";

    if (step->numVertexAttributes() > 0 || step->numInstanceAttributes() > 0) {
        sksl += emit_attributes(step->vertexAttributes(), step->instanceAttributes());
    }

    // Uniforms needed by RenderStep
    // The uniforms are mangled by having their index in 'fEntries' as a suffix (i.e., "_%d")
    // TODO: replace hard-coded bufferID with the backend's renderstep uniform-buffer index.
    if (step->numUniforms() > 0) {
        sksl += EmitRenderStepUniforms(1, "Step", step->uniforms());
    }

    // Varyings needed by RenderStep
    if (step->numVaryings() > 0) {
        sksl += EmitVaryings(step, "out");
    }

    // Vertex shader function declaration
    sksl += "void main() {\n";
    // Vertex shader body
    sksl += step->vertexSkSL();
    sksl += "sk_Position = float4(devPosition.xy * rtAdjust.xy + rtAdjust.zw, devPosition.zw);\n"
            "}\n";

    return sksl;
}

std::string GetSkSLFS(SkShaderCodeDictionary* dict,
                      SkRuntimeEffectDictionary* rteDict,
                      const GraphicsPipelineDesc& desc,
                      BlendInfo* blendInfo) {
    if (!desc.paintParamsID().isValid()) {
        // TODO: we should return the error shader code here
        return {};
    }

    SkShaderInfo shaderInfo(rteDict);

    dict->getShaderInfo(desc.paintParamsID(), &shaderInfo);
    *blendInfo = shaderInfo.blendInfo();

    std::string sksl;
    const RenderStep* step = desc.renderStep();

    // Varyings needed by RenderStep
     if (step->numVaryings() > 0) {
         sksl += EmitVaryings(step, "in");
     }

    sksl += shaderInfo.toSkSL();

    return sksl;
}

} // namespace skgpu::graphite
