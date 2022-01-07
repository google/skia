/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCustomMesh.h"

#ifdef SK_ENABLE_SKSL

#include "src/core/SkCustomMeshPriv.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLSharedCompiler.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

#include <locale>

using Attribute = SkCustomMeshSpecification::Attribute;
using Varying   = SkCustomMeshSpecification::Varying;

#define RETURN_FAILURE(...) return Result{nullptr, SkStringPrintf(__VA_ARGS__)}

#define RETURN_ERROR(...) return std::make_tuple(false, SkStringPrintf(__VA_ARGS__))

#define RETURN_SUCCESS return std::make_tuple(true, SkString{})

static bool has_main(const SkSL::Program& p) {
    for (const SkSL::ProgramElement* elem : p.elements()) {
        if (elem->is<SkSL::FunctionDefinition>()) {
            const SkSL::FunctionDefinition& defn = elem->as<SkSL::FunctionDefinition>();
            const SkSL::FunctionDeclaration& decl = defn.declaration();
            if (decl.isMain()) {
                return true;
            }
        }
    }
    return false;
}

using ColorType = SkCustomMeshSpecificationPriv::ColorType;

static std::tuple<ColorType, bool>
get_fs_color_type_and_local_coords(const SkSL::Program& fsProgram) {
    for (const SkSL::ProgramElement* elem : fsProgram.elements()) {
        if (elem->is<SkSL::FunctionDefinition>()) {
            const SkSL::FunctionDefinition& defn = elem->as<SkSL::FunctionDefinition>();
            const SkSL::FunctionDeclaration& decl = defn.declaration();
            if (decl.isMain()) {

                SkCustomMeshSpecificationPriv::ColorType ct;
                SkASSERT(decl.parameters().size() == 1 || decl.parameters().size() == 2);
                if (decl.parameters().size() == 1) {
                    ct = ColorType::kNone;
                } else {
                    const SkSL::Type& paramType = decl.parameters()[1]->type();
                    SkASSERT(paramType.matches(*fsProgram.fContext->fTypes.fHalf4) ||
                             paramType.matches(*fsProgram.fContext->fTypes.fFloat4));
                    ct = paramType.matches(*fsProgram.fContext->fTypes.fHalf4)
                                 ? ColorType::kHalf4
                                 : ColorType::kFloat4;
                }

                const SkSL::Type& returnType = decl.returnType();
                SkASSERT(returnType.matches(*fsProgram.fContext->fTypes.fVoid) ||
                         returnType.matches(*fsProgram.fContext->fTypes.fFloat2));
                bool hasLocalCoords = returnType.matches(*fsProgram.fContext->fTypes.fFloat2);

                return std::make_tuple(ct, hasLocalCoords);
            }
        }
    }
    SkUNREACHABLE;
}

// This is a non-exhaustive check for the validity of a variable name. The SkSL compiler will
// actually process the name. We're just guarding against having multiple tokens embedded in the
// name before we put it into a struct definition.
static bool check_name(const SkString& name) {
    if (name.isEmpty()) {
        return false;
    }
    for (size_t i = 0; i < name.size(); ++i) {
        if (name[i] != '_' && !std::isalnum(name[i], std::locale::classic())) {
            return false;
        }
    }
    return true;
}

static size_t attribute_type_size(Attribute::Type type) {
    switch (type) {
        case Attribute::Type::kFloat:         return 4;
        case Attribute::Type::kFloat2:        return 2*4;
        case Attribute::Type::kFloat3:        return 3*4;
        case Attribute::Type::kFloat4:        return 4*4;
        case Attribute::Type::kUByte4_unorm:  return 4;
    }
    SkUNREACHABLE;
};

static const char* attribute_type_string(Attribute::Type type) {
    switch (type) {
        case Attribute::Type::kFloat:         return "float";
        case Attribute::Type::kFloat2:        return "float2";
        case Attribute::Type::kFloat3:        return "float3";
        case Attribute::Type::kFloat4:        return "float4";
        case Attribute::Type::kUByte4_unorm:  return "half4";
    }
    SkUNREACHABLE;
};

static const char* varying_type_string(Varying::Type type) {
    switch (type) {
        case Varying::Type::kFloat:  return "float";
        case Varying::Type::kFloat2: return "float2";
        case Varying::Type::kFloat3: return "float3";
        case Varying::Type::kFloat4: return "float4";
        case Varying::Type::kHalf:   return "half";
        case Varying::Type::kHalf2:  return "half2";
        case Varying::Type::kHalf3:  return "half3";
        case Varying::Type::kHalf4:  return "half4";
    }
    SkUNREACHABLE;
};

std::tuple<bool, SkString>
check_vertex_offsets_and_stride(SkSpan<const Attribute> attributes,
                                size_t                  stride) {
    // Vulkan 1.0 has a minimum maximum attribute count of 2048.
    static_assert(SkCustomMeshSpecification::kMaxStride       <= 2048);
    // ES 2 has a max of 8.
    static_assert(SkCustomMeshSpecification::kMaxAttributes   <= 8);
    // Four bytes alignment is required by Metal.
    static_assert(SkCustomMeshSpecification::kStrideAlignment >= 4);
    static_assert(SkCustomMeshSpecification::kOffsetAlignment >= 4);
    // ES2 has a minimum maximum of 8. We may need one for a broken gl_FragCoord workaround and
    // one for local coords.
    static_assert(SkCustomMeshSpecification::kMaxVaryings     <= 6);

    if (attributes.empty()) {
        RETURN_ERROR("At least 1 attribute is required.");
    }
    if (attributes.size() > SkCustomMeshSpecification::kMaxAttributes) {
        RETURN_ERROR("A maximum of %zu attributes is allowed.",
                     SkCustomMeshSpecification::kMaxAttributes);
    }
    static_assert(SkIsPow2(SkCustomMeshSpecification::kStrideAlignment));
    if (stride == 0 || stride & (SkCustomMeshSpecification::kStrideAlignment - 1)) {
        RETURN_ERROR("Vertex stride must be a non-zero multiple of %zu.",
                     SkCustomMeshSpecification::kStrideAlignment);
    }
    if (stride > SkCustomMeshSpecification::kMaxStride) {
        RETURN_ERROR("Stride cannot exceed %zu.", SkCustomMeshSpecification::kMaxStride);
    }
    for (const auto& a : attributes) {
        if (a.offset & (SkCustomMeshSpecification::kOffsetAlignment - 1)) {
            RETURN_ERROR("Attribute offset must be a multiple of %zu.",
                         SkCustomMeshSpecification::kOffsetAlignment);
        }
        // This equivalent to vertexAttributeAccessBeyondStride==VK_FALSE in
        // VK_KHR_portability_subset. First check is to avoid overflow in second check.
        if (a.offset >= stride || a.offset + attribute_type_size(a.type) > stride) {
            RETURN_ERROR("Attribute offset plus size cannot exceed stride.");
        }
    }
    RETURN_SUCCESS;
}

SkCustomMeshSpecification::Result SkCustomMeshSpecification::Make(
        SkSpan<const Attribute> attributes,
        size_t                  vertexStride,
        SkSpan<const Varying>   varyings,
        const SkString&         vs,
        const SkString&         fs,
        sk_sp<SkColorSpace>     cs,
        SkAlphaType             at) {
    SkString attributesStruct("struct Attributes {\n");
    for (const auto& a : attributes) {
        attributesStruct.appendf("  %s %s;\n", attribute_type_string(a.type), a.name.c_str());
    }
    attributesStruct.append("};\n");

    SkString varyingStruct("struct Varyings {\n");
    for (const auto& v : varyings) {
        varyingStruct.appendf("  %s %s;\n", varying_type_string(v.type), v.name.c_str());
    }
    // Throw in an unused variable to avoid an empty struct, which is illegal.
    if (varyings.empty()) {
        varyingStruct.append("  bool _empty_;\n");
    }
    varyingStruct.append("};\n");

    SkString fullVS;
    fullVS.append(varyingStruct.c_str());
    fullVS.append(attributesStruct.c_str());
    fullVS.append(vs.c_str());

    SkString fullFS;
    fullFS.append(varyingStruct.c_str());
    fullFS.append(fs.c_str());

    return MakeFromSourceWithStructs(attributes,
                                     vertexStride,
                                     varyings,
                                     fullVS,
                                     fullFS,
                                     std::move(cs),
                                     at);
}

SkCustomMeshSpecification::Result SkCustomMeshSpecification::MakeFromSourceWithStructs(
        SkSpan<const Attribute> attributes,
        size_t                  stride,
        SkSpan<const Varying>   varyings,
        const SkString&         vs,
        const SkString&         fs,
        sk_sp<SkColorSpace>     cs,
        SkAlphaType             at) {
    if (auto [ok, error] = check_vertex_offsets_and_stride(attributes, stride); !ok) {
        return {nullptr, error};
    }

    for (const auto& a : attributes) {
        if (!check_name(a.name)) {
            RETURN_FAILURE("\"%s\" is not a valid attribute name.", a.name.c_str());
        }
    }

    if (varyings.size() > kMaxVaryings) {
        RETURN_FAILURE("A maximum of %zu varyings is allowed.", kMaxVaryings);
    }

    for (const auto& v : varyings) {
        if (!check_name(v.name)) {
            return {nullptr, SkStringPrintf("\"%s\" is not a valid varying name.", v.name.c_str())};
        }
    }

    SkSL::SharedCompiler compiler;
    SkSL::Program::Settings settings;
    settings.fEnforceES2Restrictions = true;
    std::unique_ptr<SkSL::Program> vsProgram = compiler->convertProgram(
            SkSL::ProgramKind::kCustomMeshVertex,
            SkSL::String(vs.c_str()),
            settings);
    if (!vsProgram) {
        RETURN_FAILURE("VS: %s", compiler->errorText().c_str());
    }
    if (!has_main(*vsProgram)) {
        RETURN_FAILURE("Vertex shader must have main function.");
    }
    if (SkSL::Analysis::CallsColorTransformIntrinsics(*vsProgram)) {
        RETURN_FAILURE("Color transform intrinsics are not permitted in custom mesh shaders");
    }

    std::unique_ptr<SkSL::Program> fsProgram = compiler->convertProgram(
            SkSL::ProgramKind::kCustomMeshFragment,
            SkSL::String(fs.c_str()),
            settings);

    if (!fsProgram) {
        RETURN_FAILURE("FS: %s", compiler->errorText().c_str());
    }
    if (!has_main(*fsProgram)) {
        RETURN_FAILURE("Fragment shader must have main function.");
    }
    if (SkSL::Analysis::CallsColorTransformIntrinsics(*fsProgram)) {
        RETURN_FAILURE("Color transform intrinsics are not permitted in custom mesh shaders");
    }

    auto [ct, hasLocalCoords] = get_fs_color_type_and_local_coords(*fsProgram);

    if (ct == ColorType::kNone) {
        cs = nullptr;
        at = kPremul_SkAlphaType;
    } else {
        if (!cs) {
            return {nullptr, SkString{"Must provide a color space if FS returns a color."}};
        }
        if (at == kUnknown_SkAlphaType) {
            return {nullptr, SkString{"Must provide a valid alpha type if FS returns a color."}};
        }
    }

    return {sk_sp<SkCustomMeshSpecification>(new SkCustomMeshSpecification(attributes,
                                                                           stride,
                                                                           varyings,
                                                                           std::move(vsProgram),
                                                                           std::move(fsProgram),
                                                                           ct,
                                                                           hasLocalCoords,
                                                                           std::move(cs),
                                                                           at)),
            /*error=*/ {}};
}

SkCustomMeshSpecification::~SkCustomMeshSpecification() = default;

SkCustomMeshSpecification::SkCustomMeshSpecification(SkSpan<const Attribute>        attributes,
                                                     size_t                         stride,
                                                     SkSpan<const Varying>          varyings,
                                                     std::unique_ptr<SkSL::Program> vs,
                                                     std::unique_ptr<SkSL::Program> fs,
                                                     ColorType                      ct,
                                                     bool                           hasLocalCoords,
                                                     sk_sp<SkColorSpace>            cs,
                                                     SkAlphaType                    at)
        : fAttributes(attributes.begin(), attributes.end())
        , fVaryings(varyings.begin(), varyings.end())
        , fVS(std::move(vs))
        , fFS(std::move(fs))
        , fStride(stride)
        , fColorType(ct)
        , fHasLocalCoords(hasLocalCoords)
        , fColorSpace(std::move(cs))
        , fAlphaType(at) {
    fHash = SkOpts::hash_fn(fVS->fSource->c_str(), fVS->fSource->size(), 0);
    fHash = SkOpts::hash_fn(fFS->fSource->c_str(), fFS->fSource->size(), fHash);

    // The attributes and varyings SkSL struct declarations are included in the program source.
    // However, the attribute offsets and types need to be included, the latter because the SkSL
    // struct definition has the GPU type but not the CPU data format.
    for (const auto& a : fAttributes) {
        fHash = SkOpts::hash_fn(&a.offset, sizeof(a.offset), fHash);
        fHash = SkOpts::hash_fn(&a.type,   sizeof(a.type),   fHash);
    }

    fHash = SkOpts::hash_fn(&stride, sizeof(stride), fHash);

    uint64_t csHash = fColorSpace ? fColorSpace->hash() : 0;
    fHash = SkOpts::hash_fn(&csHash, sizeof(csHash), fHash);

    auto atInt = static_cast<uint32_t>(fAlphaType);
    fHash = SkOpts::hash_fn(&atInt, sizeof(atInt), fHash);
}
#endif //SK_ENABLE_SKSL
