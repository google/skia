/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMesh.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/private/SkSLSampleUsage.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkSafeMath.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkMeshPriv.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLStructDefinition.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#include <algorithm>
#include <locale>
#include <optional>
#include <string>
#include <tuple>
#include <utility>

using namespace skia_private;

using Attribute = SkMeshSpecification::Attribute;
using Varying   = SkMeshSpecification::Varying;

using IndexBuffer  = SkMesh::IndexBuffer;
using VertexBuffer = SkMesh::VertexBuffer;

#define RETURN_FAILURE(...) return Result{nullptr, SkStringPrintf(__VA_ARGS__)}

#define RETURN_ERROR(...) return std::make_tuple(false, SkStringPrintf(__VA_ARGS__))

#define RETURN_SUCCESS return std::make_tuple(true, SkString{})

using Uniform = SkMeshSpecification::Uniform;
using Child = SkMeshSpecification::Child;

static std::vector<Uniform>::iterator find_uniform(std::vector<Uniform>& uniforms,
                                                   std::string_view name) {
    return std::find_if(uniforms.begin(), uniforms.end(),
                        [name](const SkMeshSpecification::Uniform& u) { return u.name == name; });
}

static std::tuple<bool, SkString>
gather_uniforms_and_check_for_main(const SkSL::Program& program,
                                   std::vector<Uniform>* uniforms,
                                   std::vector<Child>* children,
                                   SkMeshSpecification::Uniform::Flags stage,
                                   size_t* offset) {
    bool foundMain = false;
    for (const SkSL::ProgramElement* elem : program.elements()) {
        if (elem->is<SkSL::FunctionDefinition>()) {
            const SkSL::FunctionDefinition& defn = elem->as<SkSL::FunctionDefinition>();
            const SkSL::FunctionDeclaration& decl = defn.declaration();
            if (decl.isMain()) {
                foundMain = true;
            }
        } else if (elem->is<SkSL::GlobalVarDeclaration>()) {
            const SkSL::GlobalVarDeclaration& global = elem->as<SkSL::GlobalVarDeclaration>();
            const SkSL::VarDeclaration& varDecl = global.declaration()->as<SkSL::VarDeclaration>();
            const SkSL::Variable& var = *varDecl.var();
            if (var.modifierFlags().isUniform()) {
                if (var.type().isEffectChild()) {
                    // This is a child effect; add it to our list of children.
                    children->push_back(SkRuntimeEffectPriv::VarAsChild(var, children->size()));
                } else {
                    // This is a uniform variable; make sure it exists in our list of uniforms, and
                    // ensure that the type and layout matches between VS and FS.
                    auto iter = find_uniform(*uniforms, var.name());
                    const auto& context = *program.fContext;
                    if (iter == uniforms->end()) {
                        uniforms->push_back(SkRuntimeEffectPriv::VarAsUniform(var, context,
                                                                              offset));
                        uniforms->back().flags |= stage;
                    } else {
                        // Check that the two declarations are equivalent
                        size_t ignoredOffset = 0;
                        auto uniform = SkRuntimeEffectPriv::VarAsUniform(var, context,
                                                                         &ignoredOffset);
                        if (uniform.isArray() != iter->isArray() ||
                            uniform.type      != iter->type      ||
                            uniform.count     != iter->count) {
                            return {false,
                                    SkStringPrintf("Uniform %.*s declared with different types"
                                                   " in vertex and fragment shaders.",
                                                   (int)var.name().size(), var.name().data())};
                        }
                        if (uniform.isColor() != iter->isColor()) {
                            return {false,
                                    SkStringPrintf("Uniform %.*s declared with different color"
                                                   " layout in vertex and fragment shaders.",
                                                   (int)var.name().size(), var.name().data())};
                        }
                        (*iter).flags |= stage;
                    }
                }
            }
        }
    }
    if (!foundMain) {
        return {false, SkString("No main function found.")};
    }
    return {true, {}};
}

using ColorType = SkMeshSpecificationPriv::ColorType;

ColorType get_fs_color_type(const SkSL::Program& fsProgram) {
    for (const SkSL::ProgramElement* elem : fsProgram.elements()) {
        if (elem->is<SkSL::FunctionDefinition>()) {
            const SkSL::FunctionDefinition& defn = elem->as<SkSL::FunctionDefinition>();
            const SkSL::FunctionDeclaration& decl = defn.declaration();
            if (decl.isMain()) {
                SkASSERT(decl.parameters().size() == 1 || decl.parameters().size() == 2);
                if (decl.parameters().size() == 1) {
                    return ColorType::kNone;
                }
                const SkSL::Type& paramType = decl.parameters()[1]->type();
                SkASSERT(paramType.matches(*fsProgram.fContext->fTypes.fHalf4) ||
                         paramType.matches(*fsProgram.fContext->fTypes.fFloat4));
                return paramType.matches(*fsProgram.fContext->fTypes.fHalf4) ? ColorType::kHalf4
                                                                             : ColorType::kFloat4;
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
}

static const char* attribute_type_string(Attribute::Type type) {
    switch (type) {
        case Attribute::Type::kFloat:         return "float";
        case Attribute::Type::kFloat2:        return "float2";
        case Attribute::Type::kFloat3:        return "float3";
        case Attribute::Type::kFloat4:        return "float4";
        case Attribute::Type::kUByte4_unorm:  return "half4";
    }
    SkUNREACHABLE;
}

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
}

std::tuple<bool, SkString>
check_vertex_offsets_and_stride(SkSpan<const Attribute> attributes,
                                size_t                  stride) {
    // Vulkan 1.0 has a minimum maximum attribute count of 2048.
    static_assert(SkMeshSpecification::kMaxStride       <= 2048);
    // ES 2 has a max of 8.
    static_assert(SkMeshSpecification::kMaxAttributes   <= 8);
    // Four bytes alignment is required by Metal.
    static_assert(SkMeshSpecification::kStrideAlignment >= 4);
    static_assert(SkMeshSpecification::kOffsetAlignment >= 4);
    // ES2 has a minimum maximum of 8. We may need one for a broken gl_FragCoord workaround and
    // one for local coords.
    static_assert(SkMeshSpecification::kMaxVaryings     <= 6);

    if (attributes.empty()) {
        RETURN_ERROR("At least 1 attribute is required.");
    }
    if (attributes.size() > SkMeshSpecification::kMaxAttributes) {
        RETURN_ERROR("A maximum of %zu attributes is allowed.",
                     SkMeshSpecification::kMaxAttributes);
    }
    static_assert(SkIsPow2(SkMeshSpecification::kStrideAlignment));
    if (stride == 0 || stride & (SkMeshSpecification::kStrideAlignment - 1)) {
        RETURN_ERROR("Vertex stride must be a non-zero multiple of %zu.",
                     SkMeshSpecification::kStrideAlignment);
    }
    if (stride > SkMeshSpecification::kMaxStride) {
        RETURN_ERROR("Stride cannot exceed %zu.", SkMeshSpecification::kMaxStride);
    }
    for (const auto& a : attributes) {
        if (a.offset & (SkMeshSpecification::kOffsetAlignment - 1)) {
            RETURN_ERROR("Attribute offset must be a multiple of %zu.",
                         SkMeshSpecification::kOffsetAlignment);
        }
        // This equivalent to vertexAttributeAccessBeyondStride==VK_FALSE in
        // VK_KHR_portability_subset. First check is to avoid overflow in second check.
        if (a.offset >= stride || a.offset + attribute_type_size(a.type) > stride) {
            RETURN_ERROR("Attribute offset plus size cannot exceed stride.");
        }
    }
    RETURN_SUCCESS;
}

int check_for_passthrough_local_coords_and_dead_varyings(const SkSL::Program& fsProgram,
                                                         uint32_t* deadVaryingMask) {
    SkASSERT(deadVaryingMask);

    using namespace SkSL;
    static constexpr int kFailed = -2;

    class Visitor final : public SkSL::ProgramVisitor {
    public:
        Visitor(const Context& context) : fContext(context) {}

        void visit(const Program& program) { ProgramVisitor::visit(program); }

        int passthroughFieldIndex() const { return fPassthroughFieldIndex; }

        uint32_t fieldUseMask() const { return fFieldUseMask; }

    protected:
        bool visitProgramElement(const ProgramElement& p) override {
            if (p.is<StructDefinition>()) {
                const auto& def = p.as<StructDefinition>();
                if (def.type().name() == "Varyings") {
                    fVaryingsType = &def.type();
                }
                // No reason to keep looking at this type definition.
                return false;
            }
            if (p.is<FunctionDefinition>() && p.as<FunctionDefinition>().declaration().isMain()) {
                SkASSERT(!fVaryings);
                fVaryings = p.as<FunctionDefinition>().declaration().parameters()[0];

                SkASSERT(fVaryingsType && fVaryingsType->matches(fVaryings->type()));

                fInMain = true;
                bool result = ProgramVisitor::visitProgramElement(p);
                fInMain = false;
                return result;
            }
            return ProgramVisitor::visitProgramElement(p);
        }

        bool visitStatement(const Statement& s) override {
            if (!fInMain) {
                return ProgramVisitor::visitStatement(s);
            }
            // We should only get here if are in main and therefore found the varyings parameter.
            SkASSERT(fVaryings);
            SkASSERT(fVaryingsType);

            if (fPassthroughFieldIndex == kFailed) {
                // We've already determined there are return statements that aren't passthrough
                // or return different fields.
                return ProgramVisitor::visitStatement(s);
            }
            if (!s.is<ReturnStatement>()) {
                return ProgramVisitor::visitStatement(s);
            }

            // We just detect simple cases like "return varyings.foo;"
            const auto& rs = s.as<ReturnStatement>();
            SkASSERT(rs.expression());
            if (!rs.expression()->is<FieldAccess>()) {
                this->passthroughFailed();
                return ProgramVisitor::visitStatement(s);
            }
            const auto& fa = rs.expression()->as<FieldAccess>();
            if (!fa.base()->is<VariableReference>()) {
                this->passthroughFailed();
                return ProgramVisitor::visitStatement(s);
            }
            const auto& baseRef = fa.base()->as<VariableReference>();
            if (baseRef.variable() != fVaryings) {
                this->passthroughFailed();
                return ProgramVisitor::visitStatement(s);
            }
            if (fPassthroughFieldIndex >= 0) {
                // We already found an OK return statement. Check if this one returns the same
                // field.
                if (fa.fieldIndex() != fPassthroughFieldIndex) {
                    this->passthroughFailed();
                    return ProgramVisitor::visitStatement(s);
                }
                // We don't call our base class here because we don't want to hit visitExpression
                // and mark the returned field as used.
                return false;
            }
            const Field& field = fVaryings->type().fields()[fa.fieldIndex()];
            if (!field.fType->matches(*fContext.fTypes.fFloat2)) {
                this->passthroughFailed();
                return ProgramVisitor::visitStatement(s);
            }
            fPassthroughFieldIndex = fa.fieldIndex();
            // We don't call our base class here because we don't want to hit visitExpression and
            // mark the returned field as used.
            return false;
        }

        bool visitExpression(const Expression& e) override {
            // Anything before the Varyings struct is defined doesn't matter.
            if (!fVaryingsType) {
                return false;
            }
            if (!e.is<FieldAccess>()) {
                return ProgramVisitor::visitExpression(e);
            }
            const auto& fa = e.as<FieldAccess>();
            if (!fa.base()->type().matches(*fVaryingsType)) {
                return ProgramVisitor::visitExpression(e);
            }
            fFieldUseMask |= 1 << fa.fieldIndex();
            return false;
        }

    private:
        void passthroughFailed() {
            if (fPassthroughFieldIndex >= 0) {
                fFieldUseMask |= 1 << fPassthroughFieldIndex;
            }
            fPassthroughFieldIndex = kFailed;
        }

        const Context&  fContext;
        const Type*     fVaryingsType          = nullptr;
        const Variable* fVaryings              = nullptr;
        int             fPassthroughFieldIndex = -1;
        bool            fInMain                = false;
        uint32_t        fFieldUseMask          = 0;
    };

    Visitor v(*fsProgram.fContext);
    v.visit(fsProgram);
    *deadVaryingMask = ~v.fieldUseMask();
    return v.passthroughFieldIndex();
}

SkMeshSpecification::Result SkMeshSpecification::Make(SkSpan<const Attribute> attributes,
                                                      size_t vertexStride,
                                                      SkSpan<const Varying> varyings,
                                                      const SkString& vs,
                                                      const SkString& fs) {
    return Make(attributes,
                vertexStride,
                varyings,
                vs,
                fs,
                SkColorSpace::MakeSRGB(),
                kPremul_SkAlphaType);
}

SkMeshSpecification::Result SkMeshSpecification::Make(SkSpan<const Attribute> attributes,
                                                      size_t vertexStride,
                                                      SkSpan<const Varying> varyings,
                                                      const SkString& vs,
                                                      const SkString& fs,
                                                      sk_sp<SkColorSpace> cs) {
    return Make(attributes, vertexStride, varyings, vs, fs, std::move(cs), kPremul_SkAlphaType);
}

SkMeshSpecification::Result SkMeshSpecification::Make(SkSpan<const Attribute> attributes,
                                                      size_t vertexStride,
                                                      SkSpan<const Varying> varyings,
                                                      const SkString& vs,
                                                      const SkString& fs,
                                                      sk_sp<SkColorSpace> cs,
                                                      SkAlphaType at) {
    SkString attributesStruct("struct Attributes {\n");
    for (const auto& a : attributes) {
        attributesStruct.appendf("  %s %s;\n", attribute_type_string(a.type), a.name.c_str());
    }
    attributesStruct.append("};\n");

    bool userProvidedPositionVarying = false;
    for (const auto& v : varyings) {
        if (v.name.equals("position")) {
            if (v.type != Varying::Type::kFloat2) {
                return {nullptr, SkString("Varying \"position\" must have type float2.")};
            }
            userProvidedPositionVarying = true;
        }
    }

    STArray<kMaxVaryings, Varying> tempVaryings;
    if (!userProvidedPositionVarying) {
        // Even though we check the # of varyings in MakeFromSourceWithStructs we check here, too,
        // to avoid overflow with + 1.
        if (varyings.size() > kMaxVaryings - 1) {
            RETURN_FAILURE("A maximum of %zu varyings is allowed.", kMaxVaryings);
        }
        for (const auto& v : varyings) {
            tempVaryings.push_back(v);
        }
        tempVaryings.push_back(Varying{Varying::Type::kFloat2, SkString("position")});
        varyings = tempVaryings;
    }

    SkString varyingStruct("struct Varyings {\n");
    for (const auto& v : varyings) {
        varyingStruct.appendf("  %s %s;\n", varying_type_string(v.type), v.name.c_str());
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

SkMeshSpecification::Result SkMeshSpecification::MakeFromSourceWithStructs(
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

    std::vector<Uniform> uniforms;
    std::vector<Child> children;
    size_t offset = 0;

    SkSL::Compiler compiler;

    // Disable memory pooling; this might slow down compilation slightly, but it will ensure that a
    // long-lived mesh specification doesn't waste memory.
    SkSL::ProgramSettings settings;
    settings.fUseMemoryPool = false;

    // TODO(skia:11209): Add SkCapabilities to the API, check against required version.
    std::unique_ptr<SkSL::Program> vsProgram = compiler.convertProgram(
            SkSL::ProgramKind::kMeshVertex,
            std::string(vs.c_str()),
            settings);
    if (!vsProgram) {
        RETURN_FAILURE("VS: %s", compiler.errorText().c_str());
    }

    if (auto [result, error] = gather_uniforms_and_check_for_main(
                *vsProgram,
                &uniforms,
                &children,
                SkMeshSpecification::Uniform::Flags::kVertex_Flag,
                &offset);
        !result) {
        return {nullptr, std::move(error)};
    }

    if (SkSL::Analysis::CallsColorTransformIntrinsics(*vsProgram)) {
        RETURN_FAILURE("Color transform intrinsics are not permitted in custom mesh shaders");
    }

    std::unique_ptr<SkSL::Program> fsProgram = compiler.convertProgram(
            SkSL::ProgramKind::kMeshFragment,
            std::string(fs.c_str()),
            settings);

    if (!fsProgram) {
        RETURN_FAILURE("FS: %s", compiler.errorText().c_str());
    }

    if (auto [result, error] = gather_uniforms_and_check_for_main(
                *fsProgram,
                &uniforms,
                &children,
                SkMeshSpecification::Uniform::Flags::kFragment_Flag,
                &offset);
        !result) {
        return {nullptr, std::move(error)};
    }

    if (SkSL::Analysis::CallsColorTransformIntrinsics(*fsProgram)) {
        RETURN_FAILURE("Color transform intrinsics are not permitted in custom mesh shaders");
    }

    ColorType ct = get_fs_color_type(*fsProgram);

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

    uint32_t deadVaryingMask;
    int passthroughLocalCoordsVaryingIndex =
            check_for_passthrough_local_coords_and_dead_varyings(*fsProgram, &deadVaryingMask);

    if (passthroughLocalCoordsVaryingIndex >= 0) {
        SkASSERT(varyings[passthroughLocalCoordsVaryingIndex].type == Varying::Type::kFloat2);
    }

    return {sk_sp<SkMeshSpecification>(new SkMeshSpecification(attributes,
                                                               stride,
                                                               varyings,
                                                               passthroughLocalCoordsVaryingIndex,
                                                               deadVaryingMask,
                                                               std::move(uniforms),
                                                               std::move(children),
                                                               std::move(vsProgram),
                                                               std::move(fsProgram),
                                                               ct,
                                                               std::move(cs),
                                                               at)),
            /*error=*/{}};
}

SkMeshSpecification::~SkMeshSpecification() = default;

SkMeshSpecification::SkMeshSpecification(
        SkSpan<const Attribute>              attributes,
        size_t                               stride,
        SkSpan<const Varying>                varyings,
        int                                  passthroughLocalCoordsVaryingIndex,
        uint32_t                             deadVaryingMask,
        std::vector<Uniform>                 uniforms,
        std::vector<Child>                   children,
        std::unique_ptr<const SkSL::Program> vs,
        std::unique_ptr<const SkSL::Program> fs,
        ColorType                            ct,
        sk_sp<SkColorSpace>                  cs,
        SkAlphaType                          at)
        : fAttributes(attributes.begin(), attributes.end())
        , fVaryings(varyings.begin(), varyings.end())
        , fUniforms(std::move(uniforms))
        , fChildren(std::move(children))
        , fVS(std::move(vs))
        , fFS(std::move(fs))
        , fStride(stride)
        , fPassthroughLocalCoordsVaryingIndex(passthroughLocalCoordsVaryingIndex)
        , fDeadVaryingMask(deadVaryingMask)
        , fColorType(ct)
        , fColorSpace(std::move(cs))
        , fAlphaType(at) {
    fHash = SkChecksum::Hash32(fVS->fSource->c_str(), fVS->fSource->size(), 0);
    fHash = SkChecksum::Hash32(fFS->fSource->c_str(), fFS->fSource->size(), fHash);

    // The attributes and varyings SkSL struct declarations are included in the program source.
    // However, the attribute offsets and types need to be included, the latter because the SkSL
    // struct definition has the GPU type but not the CPU data format.
    for (const auto& a : fAttributes) {
        fHash = SkChecksum::Hash32(&a.offset, sizeof(a.offset), fHash);
        fHash = SkChecksum::Hash32(&a.type,   sizeof(a.type),   fHash);
    }

    fHash = SkChecksum::Hash32(&stride, sizeof(stride), fHash);

    uint64_t csHash = fColorSpace ? fColorSpace->hash() : 0;
    fHash = SkChecksum::Hash32(&csHash, sizeof(csHash), fHash);

    auto atInt = static_cast<uint32_t>(fAlphaType);
    fHash = SkChecksum::Hash32(&atInt, sizeof(atInt), fHash);
}

size_t SkMeshSpecification::uniformSize() const {
    return fUniforms.empty() ? 0
                             : SkAlign4(fUniforms.back().offset + fUniforms.back().sizeInBytes());
}

const Uniform* SkMeshSpecification::findUniform(std::string_view name) const {
    for (const Uniform& uniform : fUniforms) {
        if (uniform.name == name) {
            return &uniform;
        }
    }
    return nullptr;
}

const Child* SkMeshSpecification::findChild(std::string_view name) const {
    for (const Child& child : fChildren) {
        if (child.name == name) {
            return &child;
        }
    }
    return nullptr;
}

const Attribute* SkMeshSpecification::findAttribute(std::string_view name) const {
    for (const Attribute& attr : fAttributes) {
        if (name == attr.name.c_str()) {
            return &attr;
        }
    }
    return nullptr;
}

const Varying* SkMeshSpecification::findVarying(std::string_view name) const {
    for (const Varying& varying : fVaryings) {
        if (name == varying.name.c_str()) {
            return &varying;
        }
    }
    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////

SkMesh::SkMesh()  = default;
SkMesh::~SkMesh() = default;

SkMesh::SkMesh(const SkMesh&) = default;
SkMesh::SkMesh(SkMesh&&)      = default;

SkMesh& SkMesh::operator=(const SkMesh&) = default;
SkMesh& SkMesh::operator=(SkMesh&&)      = default;

SkMesh::Result SkMesh::Make(sk_sp<SkMeshSpecification> spec,
                            Mode mode,
                            sk_sp<VertexBuffer> vb,
                            size_t vertexCount,
                            size_t vertexOffset,
                            sk_sp<const SkData> uniforms,
                            SkSpan<ChildPtr> children,
                            const SkRect& bounds) {
    SkMesh mesh;
    mesh.fSpec     = std::move(spec);
    mesh.fMode     = mode;
    mesh.fVB       = std::move(vb);
    mesh.fUniforms = std::move(uniforms);
    mesh.fChildren.push_back_n(children.size(), children.data());
    mesh.fVCount   = vertexCount;
    mesh.fVOffset  = vertexOffset;
    mesh.fBounds   = bounds;
    auto [valid, msg] = mesh.validate();
    if (!valid) {
        mesh = {};
    }
    return {std::move(mesh), std::move(msg)};
}

SkMesh::Result SkMesh::MakeIndexed(sk_sp<SkMeshSpecification> spec,
                                   Mode mode,
                                   sk_sp<VertexBuffer> vb,
                                   size_t vertexCount,
                                   size_t vertexOffset,
                                   sk_sp<IndexBuffer> ib,
                                   size_t indexCount,
                                   size_t indexOffset,
                                   sk_sp<const SkData> uniforms,
                                   SkSpan<ChildPtr> children,
                                   const SkRect& bounds) {
    if (!ib) {
        // We check this before calling validate to disambiguate from a non-indexed mesh where
        // IB is expected to be null.
        return {{}, SkString{"An index buffer is required."}};
    }
    SkMesh mesh;
    mesh.fSpec     = std::move(spec);
    mesh.fMode     = mode;
    mesh.fVB       = std::move(vb);
    mesh.fVCount   = vertexCount;
    mesh.fVOffset  = vertexOffset;
    mesh.fIB       = std::move(ib);
    mesh.fUniforms = std::move(uniforms);
    mesh.fChildren.push_back_n(children.size(), children.data());
    mesh.fICount   = indexCount;
    mesh.fIOffset  = indexOffset;
    mesh.fBounds   = bounds;
    auto [valid, msg] = mesh.validate();
    if (!valid) {
        mesh = {};
    }
    return {std::move(mesh), std::move(msg)};
}

bool SkMesh::isValid() const {
    bool valid = SkToBool(fSpec);
    SkASSERT(valid == std::get<0>(this->validate()));
    return valid;
}

static size_t min_vcount_for_mode(SkMesh::Mode mode) {
    switch (mode) {
        case SkMesh::Mode::kTriangles:     return 3;
        case SkMesh::Mode::kTriangleStrip: return 3;
    }
    SkUNREACHABLE;
}

std::tuple<bool, SkString> SkMesh::validate() const {
#define FAIL_MESH_VALIDATE(...)  return std::make_tuple(false, SkStringPrintf(__VA_ARGS__))
    if (!fSpec) {
        FAIL_MESH_VALIDATE("SkMeshSpecification is required.");
    }

    if (!fVB) {
        FAIL_MESH_VALIDATE("A vertex buffer is required.");
    }

    if (fSpec->children().size() != SkToSizeT(fChildren.size())) {
        FAIL_MESH_VALIDATE("The mesh specification declares %zu child effects, "
                           "but the mesh supplies %d.",
                           fSpec->children().size(),
                           fChildren.size());
    }

    for (int index = 0; index < fChildren.size(); ++index) {
        const SkRuntimeEffect::Child& meshSpecChild = fSpec->children()[index];
        if (fChildren[index].type().has_value()) {
            if (meshSpecChild.type != fChildren[index].type()) {
                FAIL_MESH_VALIDATE("Child effect '%.*s' was specified as a %s, but passed as a %s.",
                                   (int)meshSpecChild.name.size(), meshSpecChild.name.data(),
                                   SkRuntimeEffectPriv::ChildTypeToStr(meshSpecChild.type),
                                   SkRuntimeEffectPriv::ChildTypeToStr(*fChildren[index].type()));
            }
        }
    }

    auto vb = static_cast<SkMeshPriv::VB*>(fVB.get());
    auto ib = static_cast<SkMeshPriv::IB*>(fIB.get());

    SkSafeMath sm;
    size_t vsize = sm.mul(fSpec->stride(), fVCount);
    if (sm.add(vsize, fVOffset) > vb->size()) {
        FAIL_MESH_VALIDATE("The vertex buffer offset and vertex count reads beyond the end of the"
                           " vertex buffer.");
    }

    if (fVOffset%fSpec->stride() != 0) {
        FAIL_MESH_VALIDATE("The vertex offset (%zu) must be a multiple of the vertex stride (%zu).",
                           fVOffset,
                           fSpec->stride());
    }

    if (size_t uniformSize = fSpec->uniformSize()) {
        if (!fUniforms || fUniforms->size() < uniformSize) {
            FAIL_MESH_VALIDATE("The uniform data is %zu bytes but must be at least %zu.",
                               fUniforms->size(),
                               uniformSize);
        }
    }

    auto modeToStr = [](Mode m) {
        switch (m) {
            case Mode::kTriangles:     return "triangles";
            case Mode::kTriangleStrip: return "triangle-strip";
        }
        SkUNREACHABLE;
    };
    if (ib) {
        if (fICount < min_vcount_for_mode(fMode)) {
            FAIL_MESH_VALIDATE("%s mode requires at least %zu indices but index count is %zu.",
                               modeToStr(fMode),
                               min_vcount_for_mode(fMode),
                               fICount);
        }
        size_t isize = sm.mul(sizeof(uint16_t), fICount);
        if (sm.add(isize, fIOffset) > ib->size()) {
            FAIL_MESH_VALIDATE("The index buffer offset and index count reads beyond the end of the"
                               " index buffer.");

        }
        // If we allow 32 bit indices then this should enforce 4 byte alignment in that case.
        if (!SkIsAlign2(fIOffset)) {
            FAIL_MESH_VALIDATE("The index offset must be a multiple of 2.");
        }
    } else {
        if (fVCount < min_vcount_for_mode(fMode)) {
            FAIL_MESH_VALIDATE("%s mode requires at least %zu vertices but vertex count is %zu.",
                               modeToStr(fMode),
                               min_vcount_for_mode(fMode),
                               fICount);
        }
        SkASSERT(!fICount);
        SkASSERT(!fIOffset);
    }

    if (!sm.ok()) {
        FAIL_MESH_VALIDATE("Overflow");
    }
#undef FAIL_MESH_VALIDATE
    return {true, {}};
}

//////////////////////////////////////////////////////////////////////////////

static inline bool check_update(const void* data, size_t offset, size_t size, size_t bufferSize) {
    SkSafeMath sm;
    return data                                &&
           size                                &&
           SkIsAlign4(offset)                  &&
           SkIsAlign4(size)                    &&
           sm.add(offset, size) <= bufferSize  &&
           sm.ok();
}

bool SkMesh::IndexBuffer::update(GrDirectContext* dc,
                                 const void* data,
                                 size_t offset,
                                 size_t size) {
    return check_update(data, offset, size, this->size()) && this->onUpdate(dc, data, offset, size);
}

bool SkMesh::VertexBuffer::update(GrDirectContext* dc,
                                  const void* data,
                                  size_t offset,
                                  size_t size) {
    return check_update(data, offset, size, this->size()) && this->onUpdate(dc, data, offset, size);
}

namespace SkMeshes {
sk_sp<IndexBuffer> MakeIndexBuffer(const void* data, size_t size) {
    return SkMeshPriv::CpuIndexBuffer::Make(data, size);
}

sk_sp<IndexBuffer> CopyIndexBuffer(const sk_sp<IndexBuffer>& src) {
    if (!src) {
        return nullptr;
    }
    auto* ib = static_cast<SkMeshPriv::IB*>(src.get());
    const void* data = ib->peek();
    if (!data) {
        return nullptr;
    }
    return MakeIndexBuffer(data, ib->size());
}

sk_sp<VertexBuffer> MakeVertexBuffer(const void* data, size_t size) {
    return SkMeshPriv::CpuVertexBuffer::Make(data, size);
}

sk_sp<VertexBuffer> CopyVertexBuffer(const sk_sp<VertexBuffer>& src) {
    if (!src) {
        return nullptr;
    }
    auto* vb = static_cast<SkMeshPriv::VB*>(src.get());
    const void* data = vb->peek();
    if (!data) {
        return nullptr;
    }
    return MakeVertexBuffer(data, vb->size());
}
}  // namespace SkMeshes
