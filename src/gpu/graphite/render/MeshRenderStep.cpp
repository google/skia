/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/MeshRenderStep.h"

#include "src/gpu/graphite/PaintParamsKey.h"

#include "include/private/SkAssert.h"
#include "include/private/SkDebug.h"
#include "src/core/SkMeshPriv.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkSLTypeShared.h"
#include "src/core/SkVertState.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "src/sksl/SkSLString.h"

#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

#include <cstdint>

namespace skgpu::graphite {

static constexpr char kMeshFSLocalCoordsName[] = "meshLocalCoordsOverride";

static constexpr std::initializer_list<Uniform> kStepUniforms =
        {{"depth", SkSLType::kFloat}, {"localToDevice", SkSLType::kFloat4x4}};

static SkVertices::VertexMode vertex_mode(SkMesh::Mode mode) {
    if (mode == SkMesh::Mode::kTriangles) {
        return SkVertices::VertexMode::kTriangles_VertexMode;
    } else {
        SkASSERT(mode == SkMesh::Mode::kTriangleStrip);
        return SkVertices::VertexMode::kTriangleStrip_VertexMode;
    }
}

MeshRenderStep::MeshRenderStep(Layout layout)
        : RenderStep(layout,
                     RenderStep::RenderStepID::kMesh,
                     Flags::kPerformsShading | Flags::kAppendVertices
                                             | Flags::kEmitsPrimitiveColor,
                     kStepUniforms,
                     PrimitiveType::kTriangles,
                     kDirectDepthLEqualPass,
                     /*staticAttrs=*/{},
                     /*appendAttrs=*/{{"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt}},
                     /*storageUniforms=*/{}) {}

MeshRenderStep::~MeshRenderStep() {}

std::string MeshRenderStep::vertexSkSL(const RootNodesInfo& roots) const {
    const SkMeshSpecification* spec = roots.fMeshSpec;
    SkASSERT(spec);

    // Any attributes and varyings defined by the SkMeshSpecification will be emitted within
    // `ShaderInfo::generateVertexSkSL` with the same names as defined in the mesh specification,
    // we mangle the varyings' names to ensure there is not a name conflict between an attribute
    // and varying.
    std::string attrs = "Attributes attributes;\n";
    for (const auto& attr : spec->attributes()) {
        attrs += SkSL::String::printf("attributes.%s = %s;\n",
                                      attr.name.c_str(), attr.name.c_str());
    }

    std::string varyingAssignments;
    for (const auto& v : SkMeshSpecificationPriv::Varyings(*spec)) {
        varyingAssignments += SkSL::String::printf("%s%s = varyings.%s;\n",
                                                   v.name.c_str(), kMeshVaryingMangleSuffix,
                                                   v.name.c_str());
    }

    return SkSL::String::printf("%s\n"
                                "Varyings varyings = %s(attributes);\n"
                                "float4 devPosition"
                                "       = localToDevice * float4(varyings.position, depth, 1.0);\n"
                                "stepLocalCoords = varyings.position;\n"
                                "%s", attrs.c_str(), kMeshVSMainName,
                                varyingAssignments.c_str());
}

std::string MeshRenderStep::fragmentColorSkSL(const RootNodesInfo& roots) const {
    const SkMeshSpecification* spec = roots.fMeshSpec;
    SkASSERT(spec);

    // Varyings defined by the SkMeshSpecification will be emitted within
    // `ShaderInfo::generateFragmentSkSL` following the same varying name mangling.
    std::string s = "Varyings varyings;\n";
    for (const auto& v : SkMeshSpecificationPriv::Varyings(*spec)) {
        s += SkSL::String::printf("varyings.%s = %s%s;\n",
                                  v.name.c_str(), v.name.c_str(),
                                  kMeshVaryingMangleSuffix);
    }

    bool needsColorConversion = SkMeshSpecificationPriv::GetColorType(*spec)
                                         == SkMeshSpecificationPriv::ColorType::kFloat4;
    std::string outColorName = "primitiveColor";
    if (needsColorConversion) {
        outColorName = "primitiveColorFloat4";
        s += "float4 " + outColorName + ";\n";
    }

    // Check if the mesh FS should have a primitive color output parameter.
    bool hasColorOutput = SkMeshSpecificationPriv::HasColors(*spec);
    std::string methodCall = hasColorOutput ? SkSL::String::printf("%s(varyings, %s)",
                                                                   kMeshFSMainName,
                                                                   outColorName.c_str())
                                            : SkSL::String::printf("%s(varyings)",
                                                                   kMeshFSMainName);

    s += SkSL::String::printf("float2 %s = %s;\n",
                              kMeshFSLocalCoordsName,
                              methodCall.c_str());
    if (needsColorConversion) {
        s += "primitiveColor = half4(" + outColorName + ");\n";
    }
    return s;
}

const char* MeshRenderStep::fragmentColorSkSLLocalCoordsVariable() const {
    return kMeshFSLocalCoordsName;
}

void MeshRenderStep::writeVertices(DrawWriter* writer,
                                   StorageContext* /*storageContext*/,
                                   const DrawParams& params,
                                   uint32_t ssboIndex) const {
    const SkMesh& mesh = params.geometry().mesh();
    const SkMeshSpecification* spec = mesh.spec();

    SkASSERT(mesh.vertexBuffer()); // SkMesh::isValid() should catch this in Device::drawMesh().
    auto* cpuVertexBuffer = static_cast<const SkMeshPriv::CpuVertexBuffer*>(mesh.vertexBuffer());
    size_t vertexStride = spec->stride();
    size_t vertexCount = mesh.vertexCount();
    const uint8_t* vertexData = static_cast<const uint8_t*>(
                                    cpuVertexBuffer->peek()) + mesh.vertexOffset();

    const uint16_t* indices = nullptr;
    size_t indexCount = mesh.indexCount();
    if (mesh.indexBuffer()) {
        auto* cpuIndexBuffer = static_cast<const SkMeshPriv::CpuIndexBuffer*>(mesh.indexBuffer());
        indices = reinterpret_cast<const uint16_t*>(static_cast<const uint8_t*>(
                            cpuIndexBuffer->peek()) + mesh.indexOffset());
    }

    DrawWriter::Vertices verts(*writer);
    verts.reserve(indices ? indexCount : vertexCount);

    VertState state(vertexCount, indices, indexCount);
    VertState::Proc vertProc = state.chooseProc(vertex_mode(mesh.mode()));
    while (vertProc(&state)) {
        VertexWriter vertWriter = verts.append(3);
        for (uint32_t i = 0; i < 3; ++i) {
            uint32_t vertIndex = i == 0 ? state.f0 : (i == 1 ? state.f1 : state.f2);
            const uint8_t* vertexDataBase = vertexData + vertIndex * vertexStride;

            vertWriter << ssboIndex;
            for (const SkMeshSpecification::Attribute& attr : spec->attributes()) {
                vertWriter << VertexWriter::Array(
                                    vertexDataBase + attr.offset,
                                    SkMeshSpecificationPriv::AttrTypeByteSize(attr.type));
            }
        }
    }
}

void MeshRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                              PipelineDataGatherer* gatherer) const {
    const SkMesh& mesh = params.geometry().mesh();
    const SkMeshSpecification* spec = mesh.spec();

#if defined(SK_DEBUG)
    skia_private::TArray<Uniform> uniforms(kStepUniforms);
    uniforms.reserve_exact(this->numUniforms() + spec->uniforms().size());

    skia_private::TArray<std::string> uniformNames;
    uniformNames.reserve_exact(spec->uniforms().size());
    for (const auto& u : spec->uniforms()) {
        uniformNames.push_back(std::string(u.name));
        uniforms.push_back(Uniform(uniformNames.back().c_str(),
                                   ShaderCodeDictionary::UniformTypeToSkSLType(u),
                                   u.isArray() ? u.count : Uniform::kNonArray));
    }
    gatherer->checkRewind();
    UniformExpectationsValidator uev(gatherer, uniforms);
#endif

    // Write step uniforms.
    gatherer->write(params.order().depthAsFloat());
    gatherer->write(params.transform().matrix());

    // Write SkMeshSpecification uniforms.
    sk_sp<const SkData> transformedUniforms = SkRuntimeEffectPriv::TransformUniforms(
            spec->uniforms(), mesh.refUniforms(), SkMeshSpecificationPriv::ColorSpace(*spec));
    for (const SkMeshSpecification::Uniform& u: spec->uniforms()) {
        // Since we are doing pointer arithmetic, make sure we are operating on bytes.
        static_assert(std::is_same_v<decltype(transformedUniforms->bytes()), const uint8_t*>);

        // We only need the type information for writing the uniform so we can leave the name null.
        gatherer->write(Uniform(nullptr,
                                ShaderCodeDictionary::UniformTypeToSkSLType(u),
                                u.isArray() ? u.count : Uniform::kNonArray),
                        transformedUniforms->bytes() + u.offset);
    }
}

size_t MeshRenderStep::appendDataStride(const DrawParams& params) const {
    const SkMeshSpecification* spec = params.geometry().mesh().spec();
    size_t stride = this->RenderStep::appendDataStride(params);
    for (const auto& attr : spec->attributes()) {
        stride += Attribute::MakeFromSkMeshAttribute(attr).sizeAlign4();
    }
    return stride;
}

}  // namespace skgpu::graphite
