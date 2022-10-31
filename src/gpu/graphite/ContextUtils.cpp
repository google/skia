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
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/UniformManager.h"

namespace skgpu::graphite {

std::tuple<SkUniquePaintParamsID, const SkUniformDataBlock*, const SkTextureDataBlock*>
ExtractPaintData(Recorder* recorder,
                 SkPipelineDataGatherer* gatherer,
                 SkPaintParamsKeyBuilder* builder,
                 const SkM44& local2Dev,
                 const PaintParams& p) {

    SkDEBUGCODE(gatherer->checkReset());
    SkDEBUGCODE(builder->checkReset());

    SkKeyContext keyContext(recorder, local2Dev);

    p.toKey(keyContext, builder, gatherer);

    auto dict = recorder->priv().shaderCodeDictionary();
    UniformDataCache* uniformDataCache = recorder->priv().uniformDataCache();
    TextureDataCache* textureDataCache = recorder->priv().textureDataCache();

    auto entry = dict->findOrCreate(builder);

    const SkUniformDataBlock* uniforms =
            gatherer->hasUniforms() ? uniformDataCache->insert(gatherer->finishUniformDataBlock())
                                    : nullptr;
    const SkTextureDataBlock* textures =
            gatherer->hasTextures() ? textureDataCache->insert(gatherer->textureDataBlock())
                                    : nullptr;

    gatherer->reset();

    return { entry->uniqueID(), uniforms, textures };
}

std::tuple<const SkUniformDataBlock*, const SkTextureDataBlock*>
ExtractRenderStepData(UniformDataCache* uniformDataCache,
                      TextureDataCache* textureDataCache,
                      SkPipelineDataGatherer* gatherer,
                      const RenderStep* step,
                      const DrawParams& params) {
    SkDEBUGCODE(gatherer->checkReset());

    step->writeUniformsAndTextures(params, gatherer);

    const SkUniformDataBlock* uniforms =
            gatherer->hasUniforms() ? uniformDataCache->insert(gatherer->finishUniformDataBlock())
                                    : nullptr;
    const SkTextureDataBlock* textures =
            gatherer->hasTextures() ? textureDataCache->insert(gatherer->textureDataBlock())
                                    : nullptr;

    gatherer->reset();

    return { uniforms, textures };
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
        SkSL::String::appendf(&result,
                              "    layout(offset=%zu) %s %s",
                              offsetter.advanceOffset(u.type(), u.count()),
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
                                    const std::vector<SkPaintParamsKey::BlockReader>& readers) {
    int offset = 0;

    std::string result = get_uniform_header(bufferID, name);
    for (int i = 0; i < (int) readers.size(); ++i) {
        SkSpan<const SkUniform> uniforms = readers[i].entry()->fUniforms;

        if (!uniforms.empty()) {
            SkSL::String::appendf(&result, "// %s uniforms\n", readers[i].entry()->fName);
            result += get_uniforms(uniforms, &offset, i);
        }
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

std::string EmitPaintParamsStorageBuffer(
        int bufferID,
        const char* bufferTypePrefix,
        const char* bufferNamePrefix,
        const std::vector<SkPaintParamsKey::BlockReader>& readers) {

    std::string result;
    SkSL::String::appendf(&result, "struct %sUniformData {\n", bufferTypePrefix);
    for (int i = 0; i < (int)readers.size(); ++i) {
        SkSpan<const SkUniform> uniforms = readers[i].entry()->fUniforms;
        if (uniforms.empty()) {
            continue;
        }
        SkSL::String::appendf(&result, "// %s uniforms\n", readers[i].entry()->fName);
        int manglingSuffix = i;
        for (const SkUniform& u : uniforms) {
            SkSL::String::appendf(
                    &result, "    %s %s_%d", SkSLTypeString(u.type()), u.name(), manglingSuffix);
            if (u.count()) {
                SkSL::String::appendf(&result, "[%u]", u.count());
            }
            result.append(";\n");
        }
    }
    result.append("};\n\n");

    SkSL::String::appendf(&result,
                          "layout (binding=%d) buffer %sUniforms {\n"
                          "    %sUniformData %sUniformData[];\n"
                          "};\n",
                          bufferID,
                          bufferTypePrefix,
                          bufferTypePrefix,
                          bufferNamePrefix);
    return result;
}

std::string EmitStorageBufferAccess(const char* bufferNamePrefix,
                                    const char* ssboIndex,
                                    const char* uniformName) {
    return SkSL::String::printf("%sUniformData[%s].%s", bufferNamePrefix, ssboIndex, uniformName);
}

std::string EmitTexturesAndSamplers(const std::vector<SkPaintParamsKey::BlockReader>& readers,
                                    int* binding) {
    std::string result;
    for (int i = 0; i < (int) readers.size(); ++i) {
        SkSpan<const SkTextureAndSampler> samplers = readers[i].entry()->fTexturesAndSamplers;

        if (!samplers.empty()) {
            SkSL::String::appendf(&result, "// %s samplers\n", readers[i].entry()->fName);

            for (const SkTextureAndSampler& t : samplers) {
                SkSL::String::appendf(&result,
                                      "layout(binding=%d) uniform sampler2D %s_%d;\n",
                                      *binding, t.name(), i);
                (*binding)++;
            }
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

std::string EmitVaryings(const RenderStep* step,
                         const char* direction,
                         bool emitShadingSsboIndexVarying,
                         bool emitLocalCoordsVarying) {
    std::string result;
    int location = 0;

    if (emitShadingSsboIndexVarying) {
        SkSL::String::appendf(&result,
                              "    layout(location=%d) %s int shadingSsboIndexVar;\n",
                              location++,
                              direction);
    }

    if (emitLocalCoordsVarying) {
        SkSL::String::appendf(&result, "    layout(location=%d) %s ", location++, direction);
        result.append(SkSLTypeString(SkSLType::kFloat2));
        SkSL::String::appendf(&result, " localCoordsVar;\n");
    }

    for (auto v : step->varyings()) {
        SkSL::String::appendf(&result, "    layout(location=%d) %s ", location++, direction);
        result.append(SkSLTypeString(v.fType));
        SkSL::String::appendf(&result, " %s;\n", v.fName);
    }

    return result;
}

std::string GetSkSLVS(const RenderStep* step,
                      bool defineShadingSsboIndexVarying,
                      bool defineLocalCoordsVarying) {
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
    sksl += EmitVaryings(step, "out", defineShadingSsboIndexVarying, defineLocalCoordsVarying);

    // Vertex shader function declaration
    sksl += "void main() {";
    // Create stepLocalCoords which render steps can write to.
    sksl += "float2 stepLocalCoords = float2(0);";
    // Vertex shader body
    sksl += step->vertexSkSL();
    sksl += "sk_Position = float4(devPosition.xy * rtAdjust.xy + devPosition.ww * rtAdjust.zw,"
            "                     devPosition.zw);";

    if (defineShadingSsboIndexVarying) {
        // Assign SSBO index value to the SSBO index varying
        SkSL::String::appendf(&sksl, "shadingSsboIndexVar = %s;", step->ssboIndex());
    }

    if (defineLocalCoordsVarying) {
        // Assign Render Step's stepLocalCoords to the localCoordsVar varying.
        sksl += "localCoordsVar = stepLocalCoords;";
    }
    sksl += "}";

    return sksl;
}

std::string GetSkSLFS(const SkShaderCodeDictionary* dict,
                      const SkRuntimeEffectDictionary* rteDict,
                      const RenderStep* step,
                      SkUniquePaintParamsID paintID,
                      bool useStorageBuffers,
                      BlendInfo* blendInfo,
                      bool* requiresLocalCoordsVarying) {
    if (!paintID.isValid()) {
        // TODO: we should return the error shader code here
        return {};
    }

    const char* shadingSsboIndexVar = useStorageBuffers ? "shadingSsboIndexVar" : nullptr;
    SkShaderInfo shaderInfo(rteDict, shadingSsboIndexVar);

    dict->getShaderInfo(paintID, &shaderInfo);
    *blendInfo = shaderInfo.blendInfo();
    *requiresLocalCoordsVarying = shaderInfo.needsLocalCoords();

    std::string sksl;
    sksl += shaderInfo.toSkSL(step, useStorageBuffers, *requiresLocalCoordsVarying);

    return sksl;
}

} // namespace skgpu::graphite
