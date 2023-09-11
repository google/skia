/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/codegen/SkSLWGSLCodeGenerator.h"

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkStringView.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLIntrinsicList.h"
#include "src/sksl/SkSLMemoryLayout.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/SkSLOutputStream.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLConstructorArrayCast.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLConstructorDiagonalMatrix.h"
#include "src/sksl/ir/SkSLConstructorMatrixResize.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLModifiersDeclaration.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSetting.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLStructDefinition.h"
#include "src/sksl/ir/SkSLSwitchCase.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/spirv.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <utility>

using namespace skia_private;

namespace SkSL {

enum class ProgramKind : int8_t;

namespace {

static constexpr char kSamplerSuffix[] = "_Sampler";
static constexpr char kTextureSuffix[] = "_Texture";

// See https://www.w3.org/TR/WGSL/#memory-view-types
enum class PtrAddressSpace {
    kFunction,
    kPrivate,
    kStorage,
};

const char* operator_name(Operator op) {
    switch (op.kind()) {
        case Operator::Kind::LOGICALXOR:  return " != ";
        default:                          return op.operatorName();
    }
}

bool is_reserved_word(std::string_view word) {
    static const THashSet<std::string_view> kReservedWords{
            // Used by SkSL:
            "FSIn",
            "FSOut",
            "VSIn",
            "VSOut",
            "CSIn",
            "_globalUniforms",
            "_GlobalUniforms",
            "_return",
            "_stageIn",
            "_stageOut",
            // Keywords: https://www.w3.org/TR/WGSL/#keyword-summary
            "alias",
            "break",
            "case",
            "const",
            "const_assert",
            "continue",
            "continuing",
            "default",
            "diagnostic",
            "discard",
            "else",
            "enable",
            "false",
            "fn",
            "for",
            "if",
            "let",
            "loop",
            "override",
            "requires",
            "return",
            "struct",
            "switch",
            "true",
            "var",
            "while",
            // Pre-declared types: https://www.w3.org/TR/WGSL/#predeclared-types
            "bool",
            "f16",
            "f32",
            "i32",
            "u32",
            // ... and pre-declared type generators:
            "array",
            "atomic",
            "mat2x2",
            "mat2x3",
            "mat2x4",
            "mat3x2",
            "mat3x3",
            "mat3x4",
            "mat4x2",
            "mat4x3",
            "mat4x4",
            "ptr",
            "texture_1d",
            "texture_2d",
            "texture_2d_array",
            "texture_3d",
            "texture_cube",
            "texture_cube_array",
            "texture_multisampled_2d",
            "texture_storage_1d",
            "texture_storage_2d",
            "texture_storage_2d_array",
            "texture_storage_3d",
            "vec2",
            "vec3",
            "vec4",
            // Pre-declared enumerants: https://www.w3.org/TR/WGSL/#predeclared-enumerants
            "read",
            "write",
            "read_write",
            "function",
            "private",
            "workgroup",
            "uniform",
            "storage",
            "perspective",
            "linear",
            "flat",
            "center",
            "centroid",
            "sample",
            "vertex_index",
            "instance_index",
            "position",
            "front_facing",
            "frag_depth",
            "local_invocation_id",
            "local_invocation_index",
            "global_invocation_id",
            "workgroup_id",
            "num_workgroups",
            "sample_index",
            "sample_mask",
            "rgba8unorm",
            "rgba8snorm",
            "rgba8uint",
            "rgba8sint",
            "rgba16uint",
            "rgba16sint",
            "rgba16float",
            "r32uint",
            "r32sint",
            "r32float",
            "rg32uint",
            "rg32sint",
            "rg32float",
            "rgba32uint",
            "rgba32sint",
            "rgba32float",
            "bgra8unorm",
            // Reserved words: https://www.w3.org/TR/WGSL/#reserved-words
            "_",
            "NULL",
            "Self",
            "abstract",
            "active",
            "alignas",
            "alignof",
            "as",
            "asm",
            "asm_fragment",
            "async",
            "attribute",
            "auto",
            "await",
            "become",
            "binding_array",
            "cast",
            "catch",
            "class",
            "co_await",
            "co_return",
            "co_yield",
            "coherent",
            "column_major",
            "common",
            "compile",
            "compile_fragment",
            "concept",
            "const_cast",
            "consteval",
            "constexpr",
            "constinit",
            "crate",
            "debugger",
            "decltype",
            "delete",
            "demote",
            "demote_to_helper",
            "do",
            "dynamic_cast",
            "enum",
            "explicit",
            "export",
            "extends",
            "extern",
            "external",
            "fallthrough",
            "filter",
            "final",
            "finally",
            "friend",
            "from",
            "fxgroup",
            "get",
            "goto",
            "groupshared",
            "highp",
            "impl",
            "implements",
            "import",
            "inline",
            "instanceof",
            "interface",
            "layout",
            "lowp",
            "macro",
            "macro_rules",
            "match",
            "mediump",
            "meta",
            "mod",
            "module",
            "move",
            "mut",
            "mutable",
            "namespace",
            "new",
            "nil",
            "noexcept",
            "noinline",
            "nointerpolation",
            "noperspective",
            "null",
            "nullptr",
            "of",
            "operator",
            "package",
            "packoffset",
            "partition",
            "pass",
            "patch",
            "pixelfragment",
            "precise",
            "precision",
            "premerge",
            "priv",
            "protected",
            "pub",
            "public",
            "readonly",
            "ref",
            "regardless",
            "register",
            "reinterpret_cast",
            "require",
            "resource",
            "restrict",
            "self",
            "set",
            "shared",
            "sizeof",
            "smooth",
            "snorm",
            "static",
            "static_assert",
            "static_cast",
            "std",
            "subroutine",
            "super",
            "target",
            "template",
            "this",
            "thread_local",
            "throw",
            "trait",
            "try",
            "type",
            "typedef",
            "typeid",
            "typename",
            "typeof",
            "union",
            "unless",
            "unorm",
            "unsafe",
            "unsized",
            "use",
            "using",
            "varying",
            "virtual",
            "volatile",
            "wgsl",
            "where",
            "with",
            "writeonly",
            "yield",
    };

    return kReservedWords.contains(word);
}

std::string_view pipeline_struct_prefix(ProgramKind kind) {
    if (ProgramConfig::IsVertex(kind)) {
        return "VS";
    }
    if (ProgramConfig::IsFragment(kind)) {
        return "FS";
    }
    if (ProgramConfig::IsCompute(kind)) {
        return "CS";
    }
    // Compute programs don't have stage-in/stage-out pipeline structs.
    return "";
}

std::string_view address_space_to_str(PtrAddressSpace addressSpace) {
    switch (addressSpace) {
        case PtrAddressSpace::kFunction:
            return "function";
        case PtrAddressSpace::kPrivate:
            return "private";
        case PtrAddressSpace::kStorage:
            return "storage";
    }
    SkDEBUGFAIL("unsupported ptr address space");
    return "unsupported";
}

std::string_view to_scalar_type(const Type& type) {
    SkASSERT(type.typeKind() == Type::TypeKind::kScalar);
    switch (type.numberKind()) {
        // Floating-point numbers in WebGPU currently always have 32-bit footprint and
        // relaxed-precision is not supported without extensions. f32 is the only floating-point
        // number type in WGSL (see the discussion on https://github.com/gpuweb/gpuweb/issues/658).
        case Type::NumberKind::kFloat:
            return "f32";
        case Type::NumberKind::kSigned:
            return "i32";
        case Type::NumberKind::kUnsigned:
            return "u32";
        case Type::NumberKind::kBoolean:
            return "bool";
        case Type::NumberKind::kNonnumeric:
            [[fallthrough]];
        default:
            break;
    }
    return type.name();
}

// Convert a SkSL type to a WGSL type. Handles all plain types except structure types
// (see https://www.w3.org/TR/WGSL/#plain-types-section).
std::string to_wgsl_type(const Context& context, const Type& type, const Layout* layout = nullptr) {
    switch (type.typeKind()) {
        case Type::TypeKind::kScalar:
            return std::string(to_scalar_type(type));

        case Type::TypeKind::kAtomic:
            SkASSERT(type.matches(*context.fTypes.fAtomicUInt));
            return "atomic<u32>";

        case Type::TypeKind::kVector: {
            std::string_view ct = to_scalar_type(type.componentType());
            return String::printf("vec%d<%.*s>", type.columns(), (int)ct.length(), ct.data());
        }
        case Type::TypeKind::kMatrix: {
            std::string_view ct = to_scalar_type(type.componentType());
            return String::printf("mat%dx%d<%.*s>",
                                  type.columns(), type.rows(), (int)ct.length(), ct.data());
        }
        case Type::TypeKind::kArray: {
            std::string result = "array<" + to_wgsl_type(context, type.componentType(), layout);
            if (!type.isUnsizedArray()) {
                result += ", ";
                result += std::to_string(type.columns());
            }
            return result + '>';
        }
        case Type::TypeKind::kTexture: {
            if (type.matches(*context.fTypes.fWriteOnlyTexture2D)) {
                std::string result = "texture_storage_2d<";
                // Write-only storage texture types require a pixel format, which is in the layout.
                SkASSERT(layout);
                LayoutFlags pixelFormat = layout->fFlags & LayoutFlag::kAllPixelFormats;
                switch (pixelFormat.value()) {
                    case (int)LayoutFlag::kRGBA8:
                        return result + "rgba8unorm, write>";

                    case (int)LayoutFlag::kRGBA32F:
                        return result + "rgba32float, write>";

                    case (int)LayoutFlag::kR32F:
                        return result + "r32float, write>";

                    default:
                        // The front-end should have rejected this.
                        return result + "write>";
                }
            }
            // WGSL only bakes in the pixel format for read-only textures.
            SkASSERTF(type.matches(*context.fTypes.fReadOnlyTexture2D),
                      "unexpected texture type: %s", type.description().c_str());
            return "texture_2d<f32>";
        }

        default:
            break;
    }
    return std::string(type.name());
}

std::string to_ptr_type(const Context& context,
                        const Type& type,
                        const Layout* layout,
                        PtrAddressSpace addressSpace = PtrAddressSpace::kFunction) {
    return "ptr<" + std::string(address_space_to_str(addressSpace)) + ", " +
           to_wgsl_type(context, type, layout) + '>';
}

std::string_view wgsl_builtin_name(WGSLCodeGenerator::Builtin builtin) {
    using Builtin = WGSLCodeGenerator::Builtin;
    switch (builtin) {
        case Builtin::kVertexIndex:
            return "vertex_index";
        case Builtin::kInstanceIndex:
            return "instance_index";
        case Builtin::kPosition:
            return "position";
        case Builtin::kFrontFacing:
            return "front_facing";
        case Builtin::kSampleIndex:
            return "sample_index";
        case Builtin::kFragDepth:
            return "frag_depth";
        case Builtin::kSampleMask:
        case Builtin::kSampleMaskIn:
            return "sample_mask";
        case Builtin::kLocalInvocationId:
            return "local_invocation_id";
        case Builtin::kLocalInvocationIndex:
            return "local_invocation_index";
        case Builtin::kGlobalInvocationId:
            return "global_invocation_id";
        case Builtin::kWorkgroupId:
            return "workgroup_id";
        case Builtin::kNumWorkgroups:
            return "num_workgroups";
        default:
            break;
    }

    SkDEBUGFAIL("unsupported builtin");
    return "unsupported";
}

std::string_view wgsl_builtin_type(WGSLCodeGenerator::Builtin builtin) {
    using Builtin = WGSLCodeGenerator::Builtin;
    switch (builtin) {
        case Builtin::kVertexIndex:
            return "u32";
        case Builtin::kInstanceIndex:
            return "u32";
        case Builtin::kPosition:
            return "vec4<f32>";
        case Builtin::kFrontFacing:
            return "bool";
        case Builtin::kSampleIndex:
            return "u32";
        case Builtin::kFragDepth:
            return "f32";
        case Builtin::kSampleMask:
            return "u32";
        case Builtin::kSampleMaskIn:
            return "u32";
        case Builtin::kLocalInvocationId:
            return "vec3<u32>";
        case Builtin::kLocalInvocationIndex:
            return "u32";
        case Builtin::kGlobalInvocationId:
            return "vec3<u32>";
        case Builtin::kWorkgroupId:
            return "vec3<u32>";
        case Builtin::kNumWorkgroups:
            return "vec3<u32>";
        default:
            break;
    }

    SkDEBUGFAIL("unsupported builtin");
    return "unsupported";
}

// Some built-in variables have a type that differs from their SkSL counterpart (e.g. signed vs
// unsigned integer). We handle these cases with an explicit type conversion during a variable
// reference. Returns the WGSL type of the conversion target if conversion is needed, otherwise
// returns std::nullopt.
std::optional<std::string_view> needs_builtin_type_conversion(const Variable& v) {
    switch (v.layout().fBuiltin) {
        case SK_VERTEXID_BUILTIN:
        case SK_INSTANCEID_BUILTIN:
            return {"i32"};
        default:
            break;
    }
    return std::nullopt;
}

// Map a SkSL builtin flag to a WGSL builtin kind. Returns std::nullopt if `builtin` is not
// not supported for WGSL.
//
// Also see //src/sksl/sksl_vert.sksl and //src/sksl/sksl_frag.sksl for supported built-ins.
std::optional<WGSLCodeGenerator::Builtin> builtin_from_sksl_name(int builtin) {
    using Builtin = WGSLCodeGenerator::Builtin;
    switch (builtin) {
        case SK_POSITION_BUILTIN:
            [[fallthrough]];
        case SK_FRAGCOORD_BUILTIN:
            return Builtin::kPosition;
        case SK_VERTEXID_BUILTIN:
            return Builtin::kVertexIndex;
        case SK_INSTANCEID_BUILTIN:
            return Builtin::kInstanceIndex;
        case SK_CLOCKWISE_BUILTIN:
            // TODO(skia:13092): While `front_facing` is the corresponding built-in, it does not
            // imply a particular winding order. We correctly compute the face orientation based
            // on how Skia configured the render pipeline for all references to this built-in
            // variable (see `SkSL::Program::Interface::fUseFlipRTUniform`).
            return Builtin::kFrontFacing;
        case SK_SAMPLEMASKIN_BUILTIN:
            return Builtin::kSampleMaskIn;
        case SK_SAMPLEMASK_BUILTIN:
            return Builtin::kSampleMask;
        case SK_NUMWORKGROUPS_BUILTIN:
            return Builtin::kNumWorkgroups;
        case SK_WORKGROUPID_BUILTIN:
            return Builtin::kWorkgroupId;
        case SK_LOCALINVOCATIONID_BUILTIN:
            return Builtin::kLocalInvocationId;
        case SK_GLOBALINVOCATIONID_BUILTIN:
            return Builtin::kGlobalInvocationId;
        case SK_LOCALINVOCATIONINDEX_BUILTIN:
            return Builtin::kLocalInvocationIndex;
        default:
            break;
    }
    return std::nullopt;
}

const char* delimiter_to_str(WGSLCodeGenerator::Delimiter delimiter) {
    using Delim = WGSLCodeGenerator::Delimiter;
    switch (delimiter) {
        case Delim::kComma:
            return ",";
        case Delim::kSemicolon:
            return ";";
        case Delim::kNone:
        default:
            break;
    }
    return "";
}

// FunctionDependencyResolver visits the IR tree rooted at a particular function definition and
// computes that function's dependencies on pipeline stage IO parameters. These are later used to
// synthesize arguments when writing out function definitions.
class FunctionDependencyResolver : public ProgramVisitor {
public:
    using Deps = WGSLFunctionDependencies;
    using DepsMap = WGSLCodeGenerator::ProgramRequirements::DepsMap;

    FunctionDependencyResolver(const Program* p,
                               const FunctionDeclaration* f,
                               DepsMap* programDependencyMap)
            : fProgram(p), fFunction(f), fDependencyMap(programDependencyMap) {}

    Deps resolve() {
        fDeps = WGSLFunctionDependency::kNone;
        this->visit(*fProgram);
        return fDeps;
    }

private:
    bool visitProgramElement(const ProgramElement& p) override {
        // Only visit the program that matches the requested function.
        if (p.is<FunctionDefinition>() && &p.as<FunctionDefinition>().declaration() == fFunction) {
            return INHERITED::visitProgramElement(p);
        }
        // Continue visiting other program elements.
        return false;
    }

    bool visitExpression(const Expression& e) override {
        if (e.is<VariableReference>()) {
            const VariableReference& v = e.as<VariableReference>();
            if (v.variable()->storage() == Variable::Storage::kGlobal) {
                ModifierFlags flags = v.variable()->modifierFlags();
                if (flags & ModifierFlag::kIn) {
                    fDeps |= WGSLFunctionDependency::kPipelineInputs;
                }
                if (flags & ModifierFlag::kOut) {
                    fDeps |= WGSLFunctionDependency::kPipelineOutputs;
                }
            }
        } else if (e.is<FunctionCall>()) {
            // The current function that we're processing (`fFunction`) inherits the dependencies of
            // functions that it makes calls to, because the pipeline stage IO parameters need to be
            // passed down as an argument.
            const FunctionCall& callee = e.as<FunctionCall>();

            // Don't process a function again if we have already resolved it.
            Deps* found = fDependencyMap->find(&callee.function());
            if (found) {
                fDeps |= *found;
            } else {
                // Store the dependencies that have been discovered for the current function so far.
                // If `callee` directly or indirectly calls the current function, then this value
                // will prevent an infinite recursion.
                fDependencyMap->set(fFunction, fDeps);

                // Separately traverse the called function's definition and determine its
                // dependencies.
                FunctionDependencyResolver resolver(fProgram, &callee.function(), fDependencyMap);
                Deps calleeDeps = resolver.resolve();

                // Store the callee's dependencies in the global map to avoid processing
                // the function again for future calls.
                fDependencyMap->set(&callee.function(), calleeDeps);

                // Add to the current function's dependencies.
                fDeps |= calleeDeps;
            }
        }
        return INHERITED::visitExpression(e);
    }

    const Program* const fProgram;
    const FunctionDeclaration* const fFunction;
    DepsMap* const fDependencyMap;
    Deps fDeps = WGSLFunctionDependency::kNone;

    using INHERITED = ProgramVisitor;
};

WGSLCodeGenerator::ProgramRequirements resolve_program_requirements(const Program* program) {
    WGSLCodeGenerator::ProgramRequirements requirements;

    for (const ProgramElement* e : program->elements()) {
        switch (e->kind()) {
            case ProgramElement::Kind::kFunction: {
                const FunctionDeclaration& decl = e->as<FunctionDefinition>().declaration();

                FunctionDependencyResolver resolver(program, &decl, &requirements.fDependencies);
                requirements.fDependencies.set(&decl, resolver.resolve());
                break;
            }
            case ProgramElement::Kind::kGlobalVar: {
                const GlobalVarDeclaration& decl = e->as<GlobalVarDeclaration>();
                if (decl.varDeclaration().var()->modifierFlags().isPixelLocal()) {
                    requirements.fPixelLocalExtension = true;
                }
                break;
            }
            default:
                break;
        }
    }

    return requirements;
}

void collect_pipeline_io_vars(const Program* program,
                              TArray<const Variable*>* ioVars,
                              ModifierFlag ioType) {
    for (const ProgramElement* e : program->elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const Variable* v = e->as<GlobalVarDeclaration>().varDeclaration().var();
            if (v->modifierFlags() & ioType) {
                ioVars->push_back(v);
            }
        } else if (e->is<InterfaceBlock>()) {
            const Variable* v = e->as<InterfaceBlock>().var();
            if (v->modifierFlags() & ioType) {
                ioVars->push_back(v);
            }
        }
    }
}

bool is_in_global_uniforms(const Variable& var) {
    SkASSERT(var.storage() == VariableStorage::kGlobal);
    return  var.modifierFlags().isUniform() &&
           !var.type().isOpaque() &&
           !var.interfaceBlock();
}

}  // namespace

class WGSLCodeGenerator::LValue {
public:
    virtual ~LValue() = default;

    // Returns a WGSL expression that loads from the lvalue with no side effects.
    // (e.g. `array[index].field`)
    virtual std::string load() = 0;

    // Returns a WGSL statement that stores into the lvalue with no side effects.
    // (e.g. `array[index].field = the_passed_in_value_string;`)
    virtual std::string store(const std::string& value) = 0;
};

class WGSLCodeGenerator::PointerLValue : public WGSLCodeGenerator::LValue {
public:
    // `name` must be a WGSL expression with no side-effects, which we can safely take the address
    // of. (e.g. `array[index].field` would be valid, but `array[Func()]` or `vector.x` are not.)
    PointerLValue(std::string name) : fName(std::move(name)) {}

    std::string load() override {
        return fName;
    }

    std::string store(const std::string& value) override {
        return fName + " = " + value + ";";
    }

private:
    std::string fName;
};

class WGSLCodeGenerator::VectorComponentLValue : public WGSLCodeGenerator::LValue {
public:
    // `name` must be a WGSL expression with no side-effects that points to a single component of a
    // WGSL vector.
    VectorComponentLValue(std::string name) : fName(std::move(name)) {}

    std::string load() override {
        return fName;
    }

    std::string store(const std::string& value) override {
        return fName + " = " + value + ";";
    }

private:
    std::string fName;
};

class WGSLCodeGenerator::SwizzleLValue : public WGSLCodeGenerator::LValue {
public:
    // `name` must be a WGSL expression with no side-effects that points to a WGSL vector.
    SwizzleLValue(const Context& ctx, std::string name, const Type& t, const ComponentArray& c)
            : fContext(ctx)
            , fName(std::move(name))
            , fType(t)
            , fComponents(c) {
        // If the component array doesn't cover the entire value, we need to create masks for
        // writing back into the lvalue. For example, if the type is vec4 and the component array
        // holds `zx`, a GLSL assignment would look like:
        //     name.zx = new_value;
        //
        // The equivalent WGSL assignment statement would look like:
        //     name = vec4<f32>(new_value, name.xw).yzxw;
        //
        // This replaces name.zy with new_value.xy, and leaves name.xw at their original values.
        // By convention, we always put the new value first and the original values second; it might
        // be possible to find better arrangements which simplify the assignment overall, but we
        // don't attempt this.
        int fullSlotCount = fType.slotCount();
        SkASSERT(fullSlotCount <= 4);

        // First, see which components are used.
        // The assignment swizzle must not reuse components.
        bool used[4] = {};
        for (int8_t component : fComponents) {
            SkASSERT(!used[component]);
            used[component] = true;
        }

        // Any untouched components will need to be fetched from the original value.
        for (int index = 0; index < fullSlotCount; ++index) {
            if (!used[index]) {
                fUntouchedComponents.push_back(index);
            }
        }

        // The reintegration swizzle needs to move the components back into their proper slots.
        // First, place the new-value components into the proper slots.
        fReintegrationSwizzle.resize(fullSlotCount);
        for (int index = 0; index < fComponents.size(); ++index) {
            fReintegrationSwizzle[fComponents[index]] = index;
        }
        // Then, refill the untouched slots with the original values.
        int originalValueComponentIndex = fComponents.size();
        for (int index = 0; index < fullSlotCount; ++index) {
            if (!used[index]) {
                fReintegrationSwizzle[index] = originalValueComponentIndex++;
            }
        }
    }

    std::string load() override {
        return fName + "." + Swizzle::MaskString(fComponents);
    }

    std::string store(const std::string& value) override {
        // `variable = `
        std::string result = fName;
        result += " = ";

        if (fUntouchedComponents.empty()) {
            // `(new_value).wzyx;`
            result += '(';
            result += value;
            result += ").";
            result += Swizzle::MaskString(fReintegrationSwizzle);
        } else {
            // `vec4<f32>((new_value), `
            result += to_wgsl_type(fContext, fType);
            result += "((";
            result += value;
            result += "), ";

            // `variable.yz).xzwy;`
            result += fName;
            result += '.';
            result += Swizzle::MaskString(fUntouchedComponents);
            result += ").";
            result += Swizzle::MaskString(fReintegrationSwizzle);
        }
        return result + ';';
    }

private:
    const Context& fContext;
    std::string fName;
    const Type& fType;
    ComponentArray fComponents;
    ComponentArray fUntouchedComponents;
    ComponentArray fReintegrationSwizzle;
};

bool WGSLCodeGenerator::generateCode() {
    // The resources of a WGSL program are structured in the following way:
    // - Stage attribute inputs and outputs are bundled inside synthetic structs called
    //   VSIn/VSOut/FSIn/FSOut/CSIn.
    // - All uniform and storage type resources are declared in global scope.
    this->preprocessProgram();

    {
        AutoOutputStream outputToHeader(this, &fHeader, &fIndentation);
        this->writeEnables();
        this->writeStageInputStruct();
        this->writeStageOutputStruct();
        this->writeUniformsAndBuffers();
        this->writeNonBlockUniformsForTests();
    }
    StringStream body;
    {
        // Emit the program body.
        AutoOutputStream outputToBody(this, &body, &fIndentation);
        const FunctionDefinition* mainFunc = nullptr;
        for (const ProgramElement* e : fProgram.elements()) {
            this->writeProgramElement(*e);

            if (e->is<FunctionDefinition>()) {
                const FunctionDefinition& func = e->as<FunctionDefinition>();
                if (func.declaration().isMain()) {
                    mainFunc = &func;
                }
            }
        }

        // At the bottom of the program body, emit the entrypoint function.
        // The entrypoint relies on state that has been collected while we emitted the rest of the
        // program, so it's important to do it last to make sure we don't miss anything.
        if (mainFunc) {
            this->writeEntryPoint(*mainFunc);
        }
    }

    write_stringstream(fHeader, *fOut);
    write_stringstream(body, *fOut);

    this->writeUniformPolyfills();

    return fContext.fErrors->errorCount() == 0;
}

void WGSLCodeGenerator::writeUniformPolyfills() {
    // If we didn't encounter any uniforms that need polyfilling, there is nothing to do.
    if (fFieldPolyfillMap.empty()) {
        return;
    }

    // We store the list of polyfilled fields as pointers in a hash-map, so the order can be
    // inconsistent across runs. For determinism, we sort the polyfilled objects by name here.
    TArray<const FieldPolyfillMap::Pair*> orderedFields;
    orderedFields.reserve_exact(fFieldPolyfillMap.count());

    fFieldPolyfillMap.foreach([&](const FieldPolyfillMap::Pair& pair) {
        orderedFields.push_back(&pair);
    });

    std::sort(orderedFields.begin(),
              orderedFields.end(),
              [](const FieldPolyfillMap::Pair* a, const FieldPolyfillMap::Pair* b) {
                  return a->second.fReplacementName < b->second.fReplacementName;
              });

    THashSet<const Type*> writtenArrayElementPolyfill;
    bool writtenUniformMatrixPolyfill[5][5] = {};  // m[column][row] for each matrix type
    bool writtenUniformRowPolyfill[5] = {};        // for each matrix row-size
    bool anyFieldAccessed = false;
    for (const FieldPolyfillMap::Pair* pair : orderedFields) {
        const auto& [field, info] = *pair;
        const Type* fieldType = field->fType;
        const Layout* fieldLayout = &field->fLayout;

        if (info.fIsArray) {
            fieldType = &fieldType->componentType();
            if (!writtenArrayElementPolyfill.contains(fieldType)) {
                writtenArrayElementPolyfill.add(fieldType);
                this->write("struct _skArrayElement_");
                this->write(fieldType->abbreviatedName());
                this->writeLine(" {");

                if (info.fIsMatrix) {
                    // Create a struct representing the array containing std140-padded matrices.
                    this->write("  e : _skMatrix");
                    this->write(std::to_string(fieldType->columns()));
                    this->writeLine(std::to_string(fieldType->rows()));
                } else {
                    // Create a struct representing the array with extra padding between elements.
                    this->write("  @size(16) e : ");
                    this->writeLine(to_wgsl_type(fContext, *fieldType, fieldLayout));
                }
                this->writeLine("};");
            }
        }

        if (info.fIsMatrix) {
            // Create structs representing the matrix as an array of vectors, whether or not the
            // matrix is ever accessed by the SkSL. (The struct itself is mentioned in the list of
            // uniforms.)
            int c = fieldType->columns();
            int r = fieldType->rows();
            if (!writtenUniformRowPolyfill[r]) {
                writtenUniformRowPolyfill[r] = true;

                this->write("struct _skRow");
                this->write(std::to_string(r));
                this->writeLine(" {");
                this->write("  @size(16) r : vec");
                this->write(std::to_string(r));
                this->write("<");
                this->write(to_wgsl_type(fContext, fieldType->componentType(), fieldLayout));
                this->writeLine(">");
                this->writeLine("};");
            }

            if (!writtenUniformMatrixPolyfill[c][r]) {
                writtenUniformMatrixPolyfill[c][r] = true;

                this->write("struct _skMatrix");
                this->write(std::to_string(c));
                this->write(std::to_string(r));
                this->writeLine(" {");
                this->write("  c : array<_skRow");
                this->write(std::to_string(r));
                this->write(", ");
                this->write(std::to_string(c));
                this->writeLine(">");
                this->writeLine("};");
            }
        }

        // We create a polyfill variable only if the uniform was actually accessed.
        if (!info.fWasAccessed) {
            continue;
        }
        anyFieldAccessed = true;
        this->write("var<private> ");
        this->write(info.fReplacementName);
        this->write(": ");

        const Type& interfaceBlockType = info.fInterfaceBlock->var()->type();
        if (interfaceBlockType.isArray()) {
            this->write("array<");
            this->write(to_wgsl_type(fContext, *field->fType, fieldLayout));
            this->write(", ");
            this->write(std::to_string(interfaceBlockType.columns()));
            this->write(">");
        } else {
            this->write(to_wgsl_type(fContext, *field->fType, fieldLayout));
        }
        this->writeLine(";");
    }

    // If no fields were actually accessed, _skInitializePolyfilledUniforms will not be called and
    // we can avoid emitting an empty, dead function.
    if (!anyFieldAccessed) {
        return;
    }

    this->writeLine("fn _skInitializePolyfilledUniforms() {");
    ++fIndentation;

    for (const FieldPolyfillMap::Pair* pair : orderedFields) {
        // Only initialize a polyfill global if the uniform was actually accessed.
        const auto& [field, info] = *pair;
        if (!info.fWasAccessed) {
            continue;
        }

        // Synthesize the name of this uniform variable
        std::string_view instanceName = info.fInterfaceBlock->instanceName();
        const Type& interfaceBlockType = info.fInterfaceBlock->var()->type();
        if (instanceName.empty()) {
            instanceName = fInterfaceBlockNameMap[&interfaceBlockType.componentType()];
        }

        // Initialize the global variable associated with this uniform.
        // If the interface block is arrayed, the associated global will be arrayed as well.
        int numIBElements = interfaceBlockType.isArray() ? interfaceBlockType.columns() : 1;
        for (int ibIdx = 0; ibIdx < numIBElements; ++ibIdx) {
            this->write(info.fReplacementName);
            if (interfaceBlockType.isArray()) {
                this->write("[");
                this->write(std::to_string(ibIdx));
                this->write("]");
            }
            this->write(" = ");

            const Type* fieldType = field->fType;
            const Layout* fieldLayout = &field->fLayout;

            int numArrayElements;
            if (info.fIsArray) {
                this->write(to_wgsl_type(fContext, *fieldType, fieldLayout));
                this->write("(");
                numArrayElements = fieldType->columns();
                fieldType = &fieldType->componentType();
            } else {
                numArrayElements = 1;
            }

            auto arraySeparator = String::Separator();
            for (int arrayIdx = 0; arrayIdx < numArrayElements; arrayIdx++) {
                this->write(arraySeparator());

                std::string fieldName{instanceName};
                if (interfaceBlockType.isArray()) {
                    fieldName += '[';
                    fieldName += std::to_string(ibIdx);
                    fieldName += ']';
                }
                fieldName += '.';
                fieldName += this->assembleName(field->fName);

                if (info.fIsArray) {
                    fieldName += '[';
                    fieldName += std::to_string(arrayIdx);
                    fieldName += "].e";
                }

                if (info.fIsMatrix) {
                    this->write(to_wgsl_type(fContext, *fieldType, fieldLayout));
                    this->write("(");
                    int numColumns = fieldType->columns();
                    auto matrixSeparator = String::Separator();
                    for (int column = 0; column < numColumns; column++) {
                        this->write(matrixSeparator());
                        this->write(fieldName);
                        this->write(".c[");
                        this->write(std::to_string(column));
                        this->write("].r");
                    }
                    this->write(")");
                } else {
                    this->write(fieldName);
                }
            }

            if (info.fIsArray) {
                this->write(")");
            }

            this->writeLine(";");
        }
    }

    --fIndentation;
    this->writeLine("}");
}


void WGSLCodeGenerator::preprocessProgram() {
    fRequirements = resolve_program_requirements(&fProgram);
    collect_pipeline_io_vars(&fProgram, &fPipelineInputs, ModifierFlag::kIn);
    collect_pipeline_io_vars(&fProgram, &fPipelineOutputs, ModifierFlag::kOut);
}

void WGSLCodeGenerator::write(std::string_view s) {
    if (s.empty()) {
        return;
    }
    if (fAtLineStart) {
        for (int i = 0; i < fIndentation; i++) {
            fOut->writeText("  ");
        }
    }
    fOut->writeText(std::string(s).c_str());
    fAtLineStart = false;
}

void WGSLCodeGenerator::writeLine(std::string_view s) {
    this->write(s);
    fOut->writeText("\n");
    fAtLineStart = true;
}

void WGSLCodeGenerator::finishLine() {
    if (!fAtLineStart) {
        this->writeLine();
    }
}

std::string WGSLCodeGenerator::assembleName(std::string_view name) {
    if (name.empty()) {
        // WGSL doesn't allow anonymous function parameters.
        return "_skAnonymous" + std::to_string(fScratchCount++);
    }
    // Add `R_` before reserved names to avoid any potential reserved-word conflict.
    return (skstd::starts_with(name, "_sk") ||
            skstd::starts_with(name, "R_") ||
            is_reserved_word(name))
                   ? std::string("R_") + std::string(name)
                   : std::string(name);
}

void WGSLCodeGenerator::writeVariableDecl(const Layout& layout,
                                          const Type& type,
                                          std::string_view name,
                                          Delimiter delimiter) {
    this->write(this->assembleName(name));
    this->write(": " + to_wgsl_type(fContext, type, &layout));
    this->writeLine(delimiter_to_str(delimiter));
}

void WGSLCodeGenerator::writePipelineIODeclaration(const Layout& layout,
                                                   const Type& type,
                                                   std::string_view name,
                                                   Delimiter delimiter) {
    // In WGSL, an entry-point IO parameter is "one of either a built-in value or assigned a
    // location". However, some SkSL declarations, specifically sk_FragColor, can contain both a
    // location and a builtin modifier. In addition, WGSL doesn't have a built-in equivalent for
    // sk_FragColor as it relies on the user-defined location for a render target.
    //
    // Instead of special-casing sk_FragColor, we just give higher precedence to a location modifier
    // if a declaration happens to both have a location and it's a built-in.
    //
    // Also see:
    // https://www.w3.org/TR/WGSL/#input-output-locations
    // https://www.w3.org/TR/WGSL/#attribute-location
    // https://www.w3.org/TR/WGSL/#builtin-inputs-outputs
    if (layout.fLocation >= 0) {
        this->writeUserDefinedIODecl(layout, type, name, delimiter);
    } else if (layout.fBuiltin >= 0) {
        auto builtin = builtin_from_sksl_name(layout.fBuiltin);
        if (builtin.has_value()) {
            this->writeBuiltinIODecl(type, name, *builtin, delimiter);
        }
    }
}

void WGSLCodeGenerator::writeUserDefinedIODecl(const Layout& layout,
                                               const Type& type,
                                               std::string_view name,
                                               Delimiter delimiter) {
    this->write("@location(" + std::to_string(layout.fLocation) + ") ");

    // "User-defined IO of scalar or vector integer type must always be specified as
    // @interpolate(flat)" (see https://www.w3.org/TR/WGSL/#interpolation)
    if (type.isInteger() || (type.isVector() && type.componentType().isInteger())) {
        this->write("@interpolate(flat) ");
    }

    this->writeVariableDecl(layout, type, name, delimiter);
}

void WGSLCodeGenerator::writeBuiltinIODecl(const Type& type,
                                           std::string_view name,
                                           Builtin builtin,
                                           Delimiter delimiter) {
    this->write("@builtin(");
    this->write(wgsl_builtin_name(builtin));
    this->write(") ");

    this->write(this->assembleName(name));
    this->write(": ");
    this->write(wgsl_builtin_type(builtin));
    this->writeLine(delimiter_to_str(delimiter));
}

void WGSLCodeGenerator::writeFunction(const FunctionDefinition& f) {
    const FunctionDeclaration& decl = f.declaration();
    fHasUnconditionalReturn = false;
    fConditionalScopeDepth = 0;

    SkASSERT(!fAtFunctionScope);
    fAtFunctionScope = true;

    // WGSL parameters are immutable and are considered as taking no storage, but SkSL parameters
    // are real variables. To work around this, we make var-based copies of parameters. It's
    // wasteful to make a copy of every single parameter--even if the compiler can eventually
    // optimize them all away, that takes time and generates bloated code. So, we only make
    // parameter copies if the variable is actually written-to.
    STArray<32, bool> paramNeedsDedicatedStorage;
    paramNeedsDedicatedStorage.push_back_n(decl.parameters().size(), true);

    for (size_t index = 0; index < decl.parameters().size(); ++index) {
        const Variable& param = *decl.parameters()[index];
        if (param.type().isOpaque() || param.name().empty()) {
            // Opaque-typed or anonymous parameters don't need dedicated storage.
            paramNeedsDedicatedStorage[index] = false;
            continue;
        }

        const ProgramUsage::VariableCounts counts = fProgram.fUsage->get(param);
        if ((param.modifierFlags() & ModifierFlag::kOut) || counts.fWrite == 0) {
            // Variables which are never written-to don't need dedicated storage.
            // Out-parameters are passed as pointers; the pointer itself is never modified, so
            // it doesn't need dedicated storage.
            paramNeedsDedicatedStorage[index] = false;
        }
    }

    this->writeFunctionDeclaration(decl, paramNeedsDedicatedStorage);
    this->writeLine(" {");
    ++fIndentation;

    // The parameters were given generic names like `_skParam1`, because WGSL parameters don't have
    // storage and are immutable. If mutability is required, we create variables here; otherwise, we
    // create properly-named `let` aliases.
    for (size_t index = 0; index < decl.parameters().size(); ++index) {
        if (paramNeedsDedicatedStorage[index]) {
            const Variable& param = *decl.parameters()[index];
            this->write("var ");
            this->write(this->assembleName(param.mangledName()));
            this->write(" = _skParam");
            this->write(std::to_string(index));
            this->writeLine(";");
        }
    }

    this->writeBlock(f.body()->as<Block>());

    // If fConditionalScopeDepth isn't zero, we have an unbalanced +1 or -1 when updating the depth.
    SkASSERT(fConditionalScopeDepth == 0);
    if (!fHasUnconditionalReturn && !f.declaration().returnType().isVoid()) {
        this->write("return ");
        this->write(to_wgsl_type(fContext, f.declaration().returnType()));
        this->writeLine("();");
    }

    --fIndentation;
    this->writeLine("}");

    SkASSERT(fAtFunctionScope);
    fAtFunctionScope = false;
}

void WGSLCodeGenerator::writeFunctionDeclaration(const FunctionDeclaration& decl,
                                                 SkSpan<const bool> paramNeedsDedicatedStorage) {
    this->write("fn ");
    if (decl.isMain()) {
        this->write("_skslMain(");
    } else {
        this->write(this->assembleName(decl.mangledName()));
        this->write("(");
    }
    auto separator = SkSL::String::Separator();
    if (this->writeFunctionDependencyParams(decl)) {
        separator();  // update the separator as parameters have been written
    }
    for (size_t index = 0; index < decl.parameters().size(); ++index) {
        this->write(separator());

        const Variable& param = *decl.parameters()[index];
        if (param.type().isOpaque()) {
            SkASSERT(!paramNeedsDedicatedStorage[index]);
            if (param.type().isSampler()) {
                // Create parameters for both the texture and associated sampler.
                this->write(param.name());
                this->write(kTextureSuffix);
                this->write(": texture_2d<f32>, ");
                this->write(param.name());
                this->write(kSamplerSuffix);
                this->write(": sampler");
            } else {
                // Create a parameter for the opaque object.
                this->write(param.name());
                this->write(": ");
                this->write(to_wgsl_type(fContext, param.type(), &param.layout()));
            }
        } else {
            if (paramNeedsDedicatedStorage[index] || param.name().empty()) {
                // Create an unnamed parameter. If the parameter needs dedicated storage, it will
                // later be assigned a `var` in the function body. (If it's anonymous, a var isn't
                // needed.)
                this->write("_skParam");
                this->write(std::to_string(index));
            } else {
                // Use the name directly from the SkSL program.
                this->write(this->assembleName(param.name()));
            }
            this->write(": ");
            if (param.modifierFlags() & ModifierFlag::kOut) {
                // Declare an "out" function parameter as a pointer.
                this->write(to_ptr_type(fContext, param.type(), &param.layout()));
            } else {
                this->write(to_wgsl_type(fContext, param.type(), &param.layout()));
            }
        }
    }
    this->write(")");
    if (!decl.returnType().isVoid()) {
        this->write(" -> ");
        this->write(to_wgsl_type(fContext, decl.returnType()));
    }
}

void WGSLCodeGenerator::writeEntryPoint(const FunctionDefinition& main) {
    SkASSERT(main.declaration().isMain());
    const ProgramKind programKind = fProgram.fConfig->fKind;

#if defined(SKSL_STANDALONE)
    if (ProgramConfig::IsRuntimeShader(programKind)) {
        // Synthesize a basic entrypoint which just calls straight through to main.
        // This is only used by skslc and just needs to pass the WGSL validator; Skia won't ever
        // emit functions like this.
        this->writeLine("@fragment fn main(@location(0) _coords: vec2<f32>) -> "
                                     "@location(0) vec4<f32> {");
        ++fIndentation;
        this->writeLine("return _skslMain(_coords);");
        --fIndentation;
        this->writeLine("}");
        return;
    }
#endif

    // The input and output parameters for a vertex/fragment stage entry point function have the
    // FSIn/FSOut/VSIn/VSOut/CSIn struct types that have been synthesized in generateCode(). An
    // entrypoint always has a predictable signature and acts as a trampoline to the user-defined
    // main function.
    if (ProgramConfig::IsVertex(programKind)) {
        this->write("@vertex");
    } else if (ProgramConfig::IsFragment(programKind)) {
        this->write("@fragment");
    } else if (ProgramConfig::IsCompute(programKind)) {
        this->write("@compute @workgroup_size(");
        this->write(std::to_string(fLocalSizeX));
        this->write(", ");
        this->write(std::to_string(fLocalSizeY));
        this->write(", ");
        this->write(std::to_string(fLocalSizeZ));
        this->write(")");
    } else {
        fContext.fErrors->error(Position(), "program kind not supported");
        return;
    }

    this->write(" fn main(");
    // The stage input struct is a parameter passed to main().
    if (this->needsStageInputStruct()) {
        this->write("_stageIn: ");
        this->write(pipeline_struct_prefix(programKind));
        this->write("In");
    }
    // The stage output struct is returned from main().
    if (this->needsStageOutputStruct()) {
        this->write(") -> ");
        this->write(pipeline_struct_prefix(programKind));
        this->writeLine("Out {");
    } else {
        this->writeLine(") {");
    }
    // Initialize polyfilled matrix uniforms if any were used.
    fIndentation++;
    for (const auto& [field, info] : fFieldPolyfillMap) {
        if (info.fWasAccessed) {
            this->writeLine("_skInitializePolyfilledUniforms();");
            break;
        }
    }
    // Declare the stage output struct.
    if (this->needsStageOutputStruct()) {
        this->write("var _stageOut: ");
        this->write(pipeline_struct_prefix(programKind));
        this->writeLine("Out;");
    }

#if defined(SKSL_STANDALONE)
    // We are compiling a Runtime Effect as a fragment shader, for testing purposes. We assign the
    // result from _skslMain into sk_FragColor if the user-defined main returns a color. This
    // doesn't actually matter, but it is more indicative of what a real program would do.
    // `addImplicitFragColorWrite` from Transform::FindAndDeclareBuiltinVariables has already
    // injected sk_FragColor into our stage outputs even if it wasn't explicitly referenced.
    if (ProgramConfig::IsFragment(programKind)) {
        if (main.declaration().returnType().matches(*fContext.fTypes.fHalf4)) {
            this->write("_stageOut.sk_FragColor = ");
        }
    }
#endif

    // Generate a function call to the user-defined main.
    this->write("_skslMain(");
    auto separator = SkSL::String::Separator();
    WGSLFunctionDependencies* deps = fRequirements.fDependencies.find(&main.declaration());
    if (deps) {
        if (*deps & WGSLFunctionDependency::kPipelineInputs) {
            this->write(separator());
            this->write("_stageIn");
        }
        if (*deps & WGSLFunctionDependency::kPipelineOutputs) {
            this->write(separator());
            this->write("&_stageOut");
        }
    }

#if defined(SKSL_STANDALONE)
    if (const Variable* v = main.declaration().getMainCoordsParameter()) {
        // We are compiling a Runtime Effect as a fragment shader, for testing purposes.
        // We need to synthesize a coordinates parameter, but the coordinates don't matter.
        SkASSERT(ProgramConfig::IsFragment(programKind));
        const Type& type = v->type();
        if (!type.matches(*fContext.fTypes.fFloat2)) {
            fContext.fErrors->error(main.fPosition, "main function has unsupported parameter: " +
                                                    type.description());
            return;
        }
        this->write(separator());
        this->write("/*fragcoord*/ vec2<f32>()");
    }
#endif

    this->writeLine(");");

    if (this->needsStageOutputStruct()) {
        // Return the stage output struct.
        this->writeLine("return _stageOut;");
    }

    fIndentation--;
    this->writeLine("}");
}

void WGSLCodeGenerator::writeStatement(const Statement& s) {
    switch (s.kind()) {
        case Statement::Kind::kBlock:
            this->writeBlock(s.as<Block>());
            break;
        case Statement::Kind::kBreak:
            this->writeLine("break;");
            break;
        case Statement::Kind::kContinue:
            this->writeLine("continue;");
            break;
        case Statement::Kind::kDiscard:
            this->writeLine("discard;");
            break;
        case Statement::Kind::kDo:
            this->writeDoStatement(s.as<DoStatement>());
            break;
        case Statement::Kind::kExpression:
            this->writeExpressionStatement(*s.as<ExpressionStatement>().expression());
            break;
        case Statement::Kind::kFor:
            this->writeForStatement(s.as<ForStatement>());
            break;
        case Statement::Kind::kIf:
            this->writeIfStatement(s.as<IfStatement>());
            break;
        case Statement::Kind::kNop:
            this->writeLine(";");
            break;
        case Statement::Kind::kReturn:
            this->writeReturnStatement(s.as<ReturnStatement>());
            break;
        case Statement::Kind::kSwitch:
            this->writeSwitchStatement(s.as<SwitchStatement>());
            break;
        case Statement::Kind::kSwitchCase:
            SkDEBUGFAIL("switch-case statements should only be present inside a switch");
            break;
        case Statement::Kind::kVarDeclaration:
            this->writeVarDeclaration(s.as<VarDeclaration>());
            break;
    }
}

void WGSLCodeGenerator::writeStatements(const StatementArray& statements) {
    for (const auto& s : statements) {
        if (!s->isEmpty()) {
            this->writeStatement(*s);
            this->finishLine();
        }
    }
}

void WGSLCodeGenerator::writeBlock(const Block& b) {
    // Write scope markers if this block is a scope, or if the block is empty (since we need to emit
    // something here to make the code valid).
    bool isScope = b.isScope() || b.isEmpty();
    if (isScope) {
        this->writeLine("{");
        fIndentation++;
    }
    this->writeStatements(b.children());
    if (isScope) {
        fIndentation--;
        this->writeLine("}");
    }
}

void WGSLCodeGenerator::writeExpressionStatement(const Expression& expr) {
    // Any expression-related side effects must be emitted as separate statements when
    // `assembleExpression` is called.
    // The final result of the expression will be a variable, let-reference, or an expression with
    // no side effects (`foo + bar`). Discarding this result is safe, as the program never uses it.
    (void)this->assembleExpression(expr, Precedence::kStatement);
}

void WGSLCodeGenerator::writeDoStatement(const DoStatement& s) {
    // Generate a loop structure like this:
    //   loop {
    //       body-statement;
    //       continuing {
    //           break if inverted-test-expression;
    //       }
    //   }

    ++fConditionalScopeDepth;

    std::unique_ptr<Expression> invertedTestExpr = PrefixExpression::Make(
            fContext, s.test()->fPosition, OperatorKind::LOGICALNOT, s.test()->clone());

    this->writeLine("loop {");
    fIndentation++;
    this->writeStatement(*s.statement());
    this->finishLine();

    this->writeLine("continuing {");
    fIndentation++;
    std::string breakIfExpr = this->assembleExpression(*invertedTestExpr, Precedence::kExpression);
    this->write("break if ");
    this->write(breakIfExpr);
    this->writeLine(";");
    fIndentation--;
    this->writeLine("}");
    fIndentation--;
    this->writeLine("}");

    --fConditionalScopeDepth;
}

void WGSLCodeGenerator::writeForStatement(const ForStatement& s) {
    // Generate a loop structure wrapped in an extra scope:
    //   {
    //     initializer-statement;
    //     loop;
    //   }
    // The outer scope is necessary to prevent the initializer-variable from leaking out into the
    // rest of the code. In practice, the generated code actually tends to be scoped even more
    // deeply, as the body-statement almost always contributes an extra block.

    ++fConditionalScopeDepth;

    if (s.initializer()) {
        this->writeLine("{");
        fIndentation++;
        this->writeStatement(*s.initializer());
        this->writeLine();
    }

    this->writeLine("loop {");
    fIndentation++;

    if (s.unrollInfo()) {
        if (s.unrollInfo()->fCount <= 0) {
            // Loops which are known to never execute don't need to be emitted at all.
            // (However, the front end should have already replaced this loop with a Nop.)
        } else {
            // Loops which are known to execute at least once can use this form:
            //
            //     loop {
            //         body-statement;
            //         continuing {
            //             next-expression;
            //             break if inverted-test-expression;
            //         }
            //     }

            this->writeStatement(*s.statement());
            this->finishLine();
            this->writeLine("continuing {");
            ++fIndentation;

            if (s.next()) {
                this->writeExpressionStatement(*s.next());
                this->finishLine();
            }

            if (s.test()) {
                std::unique_ptr<Expression> invertedTestExpr = PrefixExpression::Make(
                        fContext, s.test()->fPosition, OperatorKind::LOGICALNOT, s.test()->clone());

                std::string breakIfExpr =
                        this->assembleExpression(*invertedTestExpr, Precedence::kExpression);
                this->write("break if ");
                this->write(breakIfExpr);
                this->writeLine(";");
            }

            --fIndentation;
            this->writeLine("}");
        }
    } else {
        // Loops without a known execution count are emitted in this form:
        //
        //     loop {
        //         if test-expression {
        //             body-statement;
        //         } else {
        //             break;
        //         }
        //         continuing {
        //             next-expression;
        //         }
        //     }

        if (s.test()) {
            std::string testExpr = this->assembleExpression(*s.test(), Precedence::kExpression);
            this->write("if ");
            this->write(testExpr);
            this->writeLine(" {");

            fIndentation++;
            this->writeStatement(*s.statement());
            this->finishLine();
            fIndentation--;

            this->writeLine("} else {");

            fIndentation++;
            this->writeLine("break;");
            fIndentation--;

            this->writeLine("}");
        }
        else {
            this->writeStatement(*s.statement());
            this->finishLine();
        }

        if (s.next()) {
            this->writeLine("continuing {");
            fIndentation++;
            this->writeExpressionStatement(*s.next());
            this->finishLine();
            fIndentation--;
            this->writeLine("}");
        }
    }

    // This matches an open-brace at the top of the loop.
    fIndentation--;
    this->writeLine("}");

    if (s.initializer()) {
        // This matches an open-brace before the initializer-statement.
        fIndentation--;
        this->writeLine("}");
    }

    --fConditionalScopeDepth;
}

void WGSLCodeGenerator::writeIfStatement(const IfStatement& s) {
    ++fConditionalScopeDepth;

    std::string testExpr = this->assembleExpression(*s.test(), Precedence::kExpression);
    this->write("if ");
    this->write(testExpr);
    this->writeLine(" {");
    fIndentation++;
    this->writeStatement(*s.ifTrue());
    this->finishLine();
    fIndentation--;
    if (s.ifFalse()) {
        this->writeLine("} else {");
        fIndentation++;
        this->writeStatement(*s.ifFalse());
        this->finishLine();
        fIndentation--;
    }
    this->writeLine("}");

    --fConditionalScopeDepth;
}

void WGSLCodeGenerator::writeReturnStatement(const ReturnStatement& s) {
    fHasUnconditionalReturn |= (fConditionalScopeDepth == 0);

    std::string expr = s.expression()
                               ? this->assembleExpression(*s.expression(), Precedence::kExpression)
                               : std::string();
    this->write("return ");
    this->write(expr);
    this->write(";");
}

void WGSLCodeGenerator::writeSwitchCaseList(SkSpan<const SwitchCase* const> cases) {
    auto separator = SkSL::String::Separator();
    for (const SwitchCase* const sc : cases) {
        this->write(separator());
        if (sc->isDefault()) {
            this->write("default");
        } else {
            this->write(std::to_string(sc->value()));
        }
    }
}

void WGSLCodeGenerator::writeSwitchCases(SkSpan<const SwitchCase* const> cases) {
    if (!cases.empty()) {
        // Only the last switch-case should have a non-empty statement.
        SkASSERT(std::all_of(cases.begin(), std::prev(cases.end()), [](const SwitchCase* sc) {
            return sc->statement()->isEmpty();
        }));

        // Emit the cases in a comma-separated list.
        this->write("case ");
        this->writeSwitchCaseList(cases);
        this->writeLine(" {");
        ++fIndentation;

        // Emit the switch-case body.
        this->writeStatement(*cases.back()->statement());
        this->finishLine();

        --fIndentation;
        this->writeLine("}");
    }
}

void WGSLCodeGenerator::writeEmulatedSwitchFallthroughCases(SkSpan<const SwitchCase* const> cases,
                                                            std::string_view switchValue) {
    // There's no need for fallthrough handling unless we actually have multiple case blocks.
    if (cases.size() < 2) {
        this->writeSwitchCases(cases);
        return;
    }

    // Match against the entire case group.
    this->write("case ");
    this->writeSwitchCaseList(cases);
    this->writeLine(" {");
    ++fIndentation;

    std::string fallthroughVar = this->writeScratchVar(*fContext.fTypes.fBool, "false");
    const size_t secondToLastCaseIndex = cases.size() - 2;
    const size_t lastCaseIndex = cases.size() - 1;

    for (size_t index = 0; index < cases.size(); ++index) {
        const SwitchCase& sc = *cases[index];
        if (index < lastCaseIndex) {
            // The default case must come last in SkSL, and this case isn't the last one, so it
            // can't possibly be the default.
            SkASSERT(!sc.isDefault());

            this->write("if ");
            if (index > 0) {
                this->write(fallthroughVar);
                this->write(" || ");
            }
            this->write(switchValue);
            this->write(" == ");
            this->write(std::to_string(sc.value()));
            this->writeLine(" {");
            fIndentation++;

            // We write the entire case-block statement here, and then set `switchFallthrough`
            // to 1. If the case-block had a break statement in it, we break out of the outer
            // for-loop entirely, meaning the `switchFallthrough` assignment never occurs, nor
            // does any code after it inside the switch. We've forbidden `continue` statements
            // inside switch case-blocks entirely, so we don't need to consider their effect on
            // control flow; see the Finalizer in FunctionDefinition::Convert.
            this->writeStatement(*sc.statement());
            this->finishLine();

            if (index < secondToLastCaseIndex) {
                // Set a variable to indicate falling through to the next block. The very last
                // case-block is reached by process of elimination and doesn't need this
                // variable, so we don't actually need to set it if we are on the second-to-last
                // case block.
                this->write(fallthroughVar);
                this->write(" = true;  ");
            }
            this->writeLine("// fallthrough");

            fIndentation--;
            this->writeLine("}");
        } else {
            // This is the final case. Since it's always last, we can just dump in the code.
            // (If we didn't match any of the other values, we must have matched this one by
            // process of elimination. If we did match one of the other values, we either hit a
            // `break` statement earlier--and won't get this far--or we're falling through.)
            this->writeStatement(*sc.statement());
            this->finishLine();
        }
    }

    --fIndentation;
    this->writeLine("}");
}

void WGSLCodeGenerator::writeSwitchStatement(const SwitchStatement& s) {
    // WGSL supports the `switch` statement in a limited capacity. A default case must always be
    // specified. Each switch-case must be scoped inside braces. Fallthrough is not supported; a
    // trailing break is implied at the end of each switch-case block. (Explicit breaks are also
    // allowed.)  One minor improvement over a traditional switch is that switch-cases take a list
    // of values to match, instead of a single value:
    //   case 1, 2       { foo(); }
    //   case 3, default { bar(); }
    //
    // We will use the native WGSL switch statement for any switch-cases in the SkSL which can be
    // made to conform to these limitations. The remaining cases which cannot conform will be
    // emulated with if-else blocks (similar to our GLSL ES2 switch-statement emulation path). This
    // should give us good performance in the common case, since most switches naturally conform.

    // First, let's emit the switch itself.
    std::string valueExpr = this->writeNontrivialScratchLet(*s.value(), Precedence::kExpression);
    this->write("switch ");
    this->write(valueExpr);
    this->writeLine(" {");
    ++fIndentation;

    // Now let's go through the switch-cases, and emit the ones that don't fall through.
    TArray<const SwitchCase*> nativeCases;
    TArray<const SwitchCase*> fallthroughCases;
    bool previousCaseFellThrough = false;
    bool foundNativeDefault = false;
    [[maybe_unused]] bool foundFallthroughDefault = false;

    const int lastSwitchCaseIdx = s.cases().size() - 1;
    for (int index = 0; index <= lastSwitchCaseIdx; ++index) {
        const SwitchCase& sc = s.cases()[index]->as<SwitchCase>();

        if (sc.statement()->isEmpty()) {
            // This is a `case X:` that immediately falls through to the next case.
            // If we aren't already falling through, we can handle this via a comma-separated list.
            if (previousCaseFellThrough) {
                fallthroughCases.push_back(&sc);
                foundFallthroughDefault |= sc.isDefault();
            } else {
                nativeCases.push_back(&sc);
                foundNativeDefault |= sc.isDefault();
            }
            continue;
        }

        if (index == lastSwitchCaseIdx || Analysis::SwitchCaseContainsUnconditionalExit(sc)) {
            // This is a `case X:` that never falls through.
            if (previousCaseFellThrough) {
                // Because the previous cases fell through, we can't use a native switch-case here.
                fallthroughCases.push_back(&sc);
                foundFallthroughDefault |= sc.isDefault();

                this->writeEmulatedSwitchFallthroughCases(fallthroughCases, valueExpr);
                fallthroughCases.clear();

                // Fortunately, we're no longer falling through blocks, so we might be able to use a
                // native switch-case list again.
                previousCaseFellThrough = false;
            } else {
                // Emit a native switch-case block with a comma-separated case list.
                nativeCases.push_back(&sc);
                foundNativeDefault |= sc.isDefault();

                this->writeSwitchCases(nativeCases);
                nativeCases.clear();
            }
            continue;
        }

        // This case falls through, so it will need to be handled via emulation.
        // If we have put together a collection of "native" cases (cases that fall through with no
        // actual case-body), we will need to slide them over into the fallthrough-case list.
        fallthroughCases.push_back_n(nativeCases.size(), nativeCases.data());
        nativeCases.clear();

        fallthroughCases.push_back(&sc);
        foundFallthroughDefault |= sc.isDefault();
        previousCaseFellThrough = true;
    }

    // Finish out the remaining switch-cases.
    this->writeSwitchCases(nativeCases);
    nativeCases.clear();

    this->writeEmulatedSwitchFallthroughCases(fallthroughCases, valueExpr);
    fallthroughCases.clear();

    // WGSL requires a default case.
    if (!foundNativeDefault && !foundFallthroughDefault) {
        this->writeLine("case default {}");
    }

    --fIndentation;
    this->writeLine("}");
}

void WGSLCodeGenerator::writeVarDeclaration(const VarDeclaration& varDecl) {
    std::string initialValue =
            varDecl.value() ? this->assembleExpression(*varDecl.value(), Precedence::kAssignment)
                            : std::string();

    if (varDecl.var()->modifierFlags().isConst()) {
        // Use `const` at global scope, or if the value is a compile-time constant.
        SkASSERTF(varDecl.value(), "a constant variable must specify a value");
        this->write((!fAtFunctionScope || Analysis::IsCompileTimeConstant(*varDecl.value()))
                            ? "const "
                            : "let ");
    } else {
        this->write("var ");
    }
    this->write(this->assembleName(varDecl.var()->mangledName()));
    this->write(": ");
    this->write(to_wgsl_type(fContext, varDecl.var()->type(), &varDecl.var()->layout()));

    if (varDecl.value()) {
        this->write(" = ");
        this->write(initialValue);
    }

    this->write(";");
}

std::unique_ptr<WGSLCodeGenerator::LValue> WGSLCodeGenerator::makeLValue(const Expression& e) {
    if (e.is<VariableReference>()) {
        return std::make_unique<PointerLValue>(
                this->variableReferenceNameForLValue(e.as<VariableReference>()));
    }
    if (e.is<FieldAccess>()) {
        return std::make_unique<PointerLValue>(this->assembleFieldAccess(e.as<FieldAccess>()));
    }
    if (e.is<IndexExpression>()) {
        const IndexExpression& idx = e.as<IndexExpression>();
        if (idx.base()->type().isVector()) {
            // Rewrite indexed-swizzle accesses like `myVec.zyx[i]` into an index onto `myVec`.
            if (std::unique_ptr<Expression> rewrite =
                        Transform::RewriteIndexedSwizzle(fContext, idx)) {
                return std::make_unique<VectorComponentLValue>(
                        this->assembleExpression(*rewrite, Precedence::kAssignment));
            } else {
                return std::make_unique<VectorComponentLValue>(this->assembleIndexExpression(idx));
            }
        } else {
            return std::make_unique<PointerLValue>(this->assembleIndexExpression(idx));
        }
    }
    if (e.is<Swizzle>()) {
        const Swizzle& swizzle = e.as<Swizzle>();
        if (swizzle.components().size() == 1) {
            return std::make_unique<VectorComponentLValue>(this->assembleSwizzle(swizzle));
        } else {
            return std::make_unique<SwizzleLValue>(
                    fContext,
                    this->assembleExpression(*swizzle.base(), Precedence::kAssignment),
                    swizzle.base()->type(),
                    swizzle.components());
        }
    }

    fContext.fErrors->error(e.fPosition, "unsupported lvalue type");
    return nullptr;
}

std::string WGSLCodeGenerator::assembleExpression(const Expression& e,
                                                  Precedence parentPrecedence) {
    switch (e.kind()) {
        case Expression::Kind::kBinary:
            return this->assembleBinaryExpression(e.as<BinaryExpression>(), parentPrecedence);

        case Expression::Kind::kConstructorCompound:
            return this->assembleConstructorCompound(e.as<ConstructorCompound>());

        case Expression::Kind::kConstructorArrayCast:
            // This is a no-op, since WGSL 1.0 doesn't have any concept of precision qualifiers.
            // When we add support for f16, this will need to copy the array contents.
            return this->assembleExpression(*e.as<ConstructorArrayCast>().argument(),
                                            parentPrecedence);

        case Expression::Kind::kConstructorArray:
        case Expression::Kind::kConstructorCompoundCast:
        case Expression::Kind::kConstructorScalarCast:
        case Expression::Kind::kConstructorSplat:
        case Expression::Kind::kConstructorStruct:
            return this->assembleAnyConstructor(e.asAnyConstructor());

        case Expression::Kind::kConstructorDiagonalMatrix:
            return this->assembleConstructorDiagonalMatrix(e.as<ConstructorDiagonalMatrix>());

        case Expression::Kind::kConstructorMatrixResize:
            return this->assembleConstructorMatrixResize(e.as<ConstructorMatrixResize>());

        case Expression::Kind::kEmpty:
            return "false";

        case Expression::Kind::kFieldAccess:
            return this->assembleFieldAccess(e.as<FieldAccess>());

        case Expression::Kind::kFunctionCall:
            return this->assembleFunctionCall(e.as<FunctionCall>(), parentPrecedence);

        case Expression::Kind::kIndex:
            return this->assembleIndexExpression(e.as<IndexExpression>());

        case Expression::Kind::kLiteral:
            return this->assembleLiteral(e.as<Literal>());

        case Expression::Kind::kPrefix:
            return this->assemblePrefixExpression(e.as<PrefixExpression>(), parentPrecedence);

        case Expression::Kind::kPostfix:
            return this->assemblePostfixExpression(e.as<PostfixExpression>(), parentPrecedence);

        case Expression::Kind::kSetting:
            return this->assembleExpression(*e.as<Setting>().toLiteral(fContext), parentPrecedence);

        case Expression::Kind::kSwizzle:
            return this->assembleSwizzle(e.as<Swizzle>());

        case Expression::Kind::kTernary:
            return this->assembleTernaryExpression(e.as<TernaryExpression>(), parentPrecedence);

        case Expression::Kind::kVariableReference:
            return this->assembleVariableReference(e.as<VariableReference>());

        default:
            SkDEBUGFAILF("unsupported expression:\n%s", e.description().c_str());
            return {};
    }
}

static bool is_nontrivial_expression(const Expression& expr) {
    // We consider a "trivial expression" one which we can repeat multiple times in the output
    // without being dangerous or spammy. We avoid emitting temporary variables for very trivial
    // expressions: literals, unadorned variable references, or constant vectors.
    if (expr.is<VariableReference>() || expr.is<Literal>()) {
        // Variables and literals are trivial; adding a let-declaration won't simplify anything.
        return false;
    }
    if (expr.type().isVector() && Analysis::IsConstantExpression(expr)) {
        // Compile-time constant vectors are also considered trivial; they're short and sweet.
        return false;
    }
    return true;
}

static bool binary_op_is_ambiguous_in_wgsl(Operator op) {
    // WGSL always requires parentheses for some operators which are deemed to be ambiguous.
    // (8.19. Operator Precedence and Associativity)
    switch (op.kind()) {
        case OperatorKind::LOGICALOR:
        case OperatorKind::LOGICALAND:
        case OperatorKind::BITWISEOR:
        case OperatorKind::BITWISEAND:
        case OperatorKind::BITWISEXOR:
        case OperatorKind::SHL:
        case OperatorKind::SHR:
        case OperatorKind::LT:
        case OperatorKind::GT:
        case OperatorKind::LTEQ:
        case OperatorKind::GTEQ:
            return true;

        default:
            return false;
    }
}

bool WGSLCodeGenerator::binaryOpNeedsComponentwiseMatrixPolyfill(const Type& left,
                                                                 const Type& right,
                                                                 Operator op) {
    switch (op.kind()) {
        case OperatorKind::SLASH:
            // WGSL does not natively support componentwise matrix-op-matrix for division.
            if (left.isMatrix() && right.isMatrix()) {
                return true;
            }
            [[fallthrough]];

        case OperatorKind::PLUS:
        case OperatorKind::MINUS:
            // WGSL does not natively support componentwise matrix-op-scalar or scalar-op-matrix for
            // addition, subtraction or division.
            return (left.isMatrix() && right.isScalar()) ||
                   (left.isScalar() && right.isMatrix());

        default:
            return false;
    }
}

std::string WGSLCodeGenerator::assembleBinaryExpression(const BinaryExpression& b,
                                                        Precedence parentPrecedence) {
    return this->assembleBinaryExpression(*b.left(), b.getOperator(), *b.right(), b.type(),
                                          parentPrecedence);
}

std::string WGSLCodeGenerator::assembleBinaryExpression(const Expression& left,
                                                        Operator op,
                                                        const Expression& right,
                                                        const Type& resultType,
                                                        Precedence parentPrecedence) {
    // If the operator is && or ||, we need to handle short-circuiting properly. Specifically, we
    // sometimes need to emit extra statements to paper over functionality that WGSL lacks, like
    // assignment in the middle of an expression. We need to guard those extra statements, to ensure
    // that they don't occur if the expression evaluation is short-circuited. Converting the
    // expression into an if-else block keeps the short-circuit property intact even when extra
    // statements are involved.
    // If the RHS doesn't have any side effects, then it's safe to just leave the expression as-is,
    // since we know that any possible extra statements are non-side-effecting.
    std::string expr;
    if (op.kind() == OperatorKind::LOGICALAND && Analysis::HasSideEffects(right)) {
        // Converts `left_expression && right_expression` into the following block:

        // var _skTemp1: bool;
        // [[ prepare left_expression ]]
        // if left_expression {
        //     [[ prepare right_expression ]]
        //     _skTemp1 = right_expression;
        // } else {
        //     _skTemp1 = false;
        // }

        expr = this->writeScratchVar(resultType);

        std::string leftExpr = this->assembleExpression(left, Precedence::kExpression);
        this->write("if ");
        this->write(leftExpr);
        this->writeLine(" {");

        ++fIndentation;
        std::string rightExpr = this->assembleExpression(right, Precedence::kAssignment);
        this->write(expr);
        this->write(" = ");
        this->write(rightExpr);
        this->writeLine(";");
        --fIndentation;

        this->writeLine("} else {");

        ++fIndentation;
        this->write(expr);
        this->writeLine(" = false;");
        --fIndentation;

        this->writeLine("}");
        return expr;
    }

    if (op.kind() == OperatorKind::LOGICALOR && Analysis::HasSideEffects(right)) {
        // Converts `left_expression || right_expression` into the following block:

        // var _skTemp1: bool;
        // [[ prepare left_expression ]]
        // if left_expression {
        //     _skTemp1 = true;
        // } else {
        //     [[ prepare right_expression ]]
        //     _skTemp1 = right_expression;
        // }

        expr = this->writeScratchVar(resultType);

        std::string leftExpr = this->assembleExpression(left, Precedence::kExpression);
        this->write("if ");
        this->write(leftExpr);
        this->writeLine(" {");

        ++fIndentation;
        this->write(expr);
        this->writeLine(" = true;");
        --fIndentation;

        this->writeLine("} else {");

        ++fIndentation;
        std::string rightExpr = this->assembleExpression(right, Precedence::kAssignment);
        this->write(expr);
        this->write(" = ");
        this->write(rightExpr);
        this->writeLine(";");
        --fIndentation;

        this->writeLine("}");
        return expr;
    }

    // Handle comma-expressions.
    if (op.kind() == OperatorKind::COMMA) {
        // The result from the left-expression is ignored, but its side effects must occur.
        this->assembleExpression(left, Precedence::kStatement);

        // Evaluate the right side normally.
        return this->assembleExpression(right, parentPrecedence);
    }

    // Handle assignment-expressions.
    if (op.isAssignment()) {
        std::unique_ptr<LValue> lvalue = this->makeLValue(left);
        if (!lvalue) {
            return "";
        }

        if (op.kind() == OperatorKind::EQ) {
            // Evaluate the right-hand side of simple assignment (`a = b` --> `b`).
            expr = this->assembleExpression(right, Precedence::kAssignment);
        } else {
            // Evaluate the right-hand side of compound-assignment (`a += b` --> `a + b`).
            op = op.removeAssignment();

            std::string lhs = lvalue->load();
            std::string rhs = this->assembleExpression(right, op.getBinaryPrecedence());

            if (this->binaryOpNeedsComponentwiseMatrixPolyfill(left.type(), right.type(), op)) {
                if (is_nontrivial_expression(right)) {
                    rhs = this->writeScratchLet(rhs);
                }

                expr = this->assembleComponentwiseMatrixBinary(left.type(), right.type(),
                                                               lhs, rhs, op);
            } else {
                expr = lhs + operator_name(op) + rhs;
            }
        }

        // Emit the assignment statement (`a = a + b`).
        this->writeLine(lvalue->store(expr));

        // Return the lvalue (`a`) as the result, since the value might be used by the caller.
        return lvalue->load();
    }

    if (op.isEquality()) {
        return this->assembleEqualityExpression(left, right, op, parentPrecedence);
    }

    Precedence precedence = op.getBinaryPrecedence();
    bool needParens = precedence >= parentPrecedence;
    if (binary_op_is_ambiguous_in_wgsl(op)) {
        precedence = Precedence::kParentheses;
    }
    if (needParens) {
        expr = "(";
    }

    // If we are emitting `constant + constant`, this generally indicates that the values could not
    // be constant-folded. This happens when the values overflow or become nan. WGSL will refuse to
    // compile such expressions, as WGSL 1.0 has no infinity/nan support. However, the WGSL
    // compile-time check can be dodged by putting one side into a let-variable. This technically
    // gives us an indeterminate result, but the vast majority of backends will just calculate an
    // infinity or nan here, as we would expect. (skia:14385)
    bool bothSidesConstant = ConstantFolder::GetConstantValueOrNull(left) &&
                             ConstantFolder::GetConstantValueOrNull(right);

    std::string lhs = this->assembleExpression(left, precedence);
    std::string rhs = this->assembleExpression(right, precedence);

    if (this->binaryOpNeedsComponentwiseMatrixPolyfill(left.type(), right.type(), op)) {
        if (bothSidesConstant || is_nontrivial_expression(left)) {
            lhs = this->writeScratchLet(lhs);
        }
        if (is_nontrivial_expression(right)) {
            rhs = this->writeScratchLet(rhs);
        }

        expr += this->assembleComponentwiseMatrixBinary(left.type(), right.type(), lhs, rhs, op);
    } else {
        if (bothSidesConstant) {
            lhs = this->writeScratchLet(lhs);
        }

        expr += lhs + operator_name(op) + rhs;
    }

    if (needParens) {
        expr += ')';
    }

    return expr;
}

std::string WGSLCodeGenerator::assembleFieldAccess(const FieldAccess& f) {
    const Field* field = &f.base()->type().fields()[f.fieldIndex()];
    std::string expr;

    if (FieldPolyfillInfo* polyfillInfo = fFieldPolyfillMap.find(field)) {
        // We found a matrix uniform. We are required to pass some matrix uniforms as array vectors,
        // since the std140 layout for a matrix assumes 4-column vectors for each row, and WGSL
        // tightly packs 2-column matrices. When emitting code, we replace the field-access
        // expression with a global variable which holds an unpacked version of the uniform.
        polyfillInfo->fWasAccessed = true;

        // The polyfill can either be based directly onto a uniform in an interface block, or it
        // might be based on an index-expression onto a uniform if the interface block is arrayed.
        const Expression* base = f.base().get();
        const IndexExpression* indexExpr = nullptr;
        if (base->is<IndexExpression>()) {
            indexExpr = &base->as<IndexExpression>();
            base = indexExpr->base().get();
        }

        SkASSERT(base->is<VariableReference>());
        expr = polyfillInfo->fReplacementName;

        // If we had an index expression, we must append the index.
        if (indexExpr) {
            expr += '[';
            expr += this->assembleExpression(*indexExpr->index(), Precedence::kSequence);
            expr += ']';
        }
        return expr;
    }

    switch (f.ownerKind()) {
        case FieldAccess::OwnerKind::kDefault:
            expr = this->assembleExpression(*f.base(), Precedence::kPostfix) + '.';
            break;

        case FieldAccess::OwnerKind::kAnonymousInterfaceBlock:
            if (f.base()->is<VariableReference>() &&
                field->fLayout.fBuiltin != SK_POINTSIZE_BUILTIN) {
                expr = this->variablePrefix(*f.base()->as<VariableReference>().variable());
            }
            break;
    }

    expr += this->assembleName(field->fName);
    return expr;
}

static bool all_arguments_constant(const ExpressionArray& arguments) {
    // Returns true if all arguments in the ExpressionArray are compile-time constants. If we are
    // calling an intrinsic and all of its inputs are constant, but we didn't constant-fold it, this
    // generally indicates that constant-folding resulted in an infinity or nan. The WGSL compiler
    // will reject such an expression with a compile-time error. We can dodge the error, taking on
    // the risk of indeterminate behavior instead, by replacing one of the constant values with a
    // scratch let-variable. (skia:14385)
    for (const std::unique_ptr<Expression>& arg : arguments) {
        if (!ConstantFolder::GetConstantValueOrNull(*arg)) {
            return false;
        }
    }
    return true;
}

std::string WGSLCodeGenerator::assembleSimpleIntrinsic(std::string_view intrinsicName,
                                                       const FunctionCall& call) {
    // Invoke the function, passing each function argument.
    std::string expr = std::string(intrinsicName);
    expr.push_back('(');
    const ExpressionArray& args = call.arguments();
    auto separator = SkSL::String::Separator();
    bool allConstant = all_arguments_constant(call.arguments());
    for (int index = 0; index < args.size(); ++index) {
        expr += separator();

        std::string argument = this->assembleExpression(*args[index], Precedence::kSequence);
        if (args[index]->type().isAtomic()) {
            // WGSL passes atomic values to intrinsics as pointers.
            expr += '&';
            expr += argument;
        } else if (allConstant && index == 0) {
            // We can use a scratch-let for argument 0 to dodge WGSL overflow errors. (skia:14385)
            expr += this->writeScratchLet(argument);
        } else {
            expr += argument;
        }
    }
    expr.push_back(')');

    if (call.type().isVoid()) {
        this->write(expr);
        this->writeLine(";");
        return std::string();
    } else {
        return this->writeScratchLet(expr);
    }
}

std::string WGSLCodeGenerator::assembleVectorizedIntrinsic(std::string_view intrinsicName,
                                                           const FunctionCall& call) {
    SkASSERT(!call.type().isVoid());

    // Invoke the function, passing each function argument.
    std::string expr = std::string(intrinsicName);
    expr.push_back('(');

    auto separator = SkSL::String::Separator();
    const ExpressionArray& args = call.arguments();
    bool returnsVector = call.type().isVector();
    bool allConstant = all_arguments_constant(call.arguments());
    for (int index = 0; index < args.size(); ++index) {
        expr += separator();

        bool vectorize = returnsVector && args[index]->type().isScalar();
        if (vectorize) {
            expr += to_wgsl_type(fContext, call.type());
            expr.push_back('(');
        }

        // We can use a scratch-let for argument 0 to dodge WGSL overflow errors. (skia:14385)
        std::string argument = this->assembleExpression(*args[index], Precedence::kSequence);
        expr += (allConstant && index == 0) ? this->writeScratchLet(argument)
                                            : argument;
        if (vectorize) {
            expr.push_back(')');
        }
    }
    expr.push_back(')');

    return this->writeScratchLet(expr);
}

std::string WGSLCodeGenerator::assembleUnaryOpIntrinsic(Operator op,
                                                        const FunctionCall& call,
                                                        Precedence parentPrecedence) {
    SkASSERT(!call.type().isVoid());

    bool needParens = Precedence::kPrefix >= parentPrecedence;

    std::string expr;
    if (needParens) {
        expr.push_back('(');
    }

    expr += operator_name(op);
    expr += this->assembleExpression(*call.arguments()[0], Precedence::kPrefix);

    if (needParens) {
        expr.push_back(')');
    }

    return expr;
}

std::string WGSLCodeGenerator::assembleBinaryOpIntrinsic(Operator op,
                                                         const FunctionCall& call,
                                                         Precedence parentPrecedence) {
    SkASSERT(!call.type().isVoid());

    Precedence precedence = op.getBinaryPrecedence();
    bool needParens = precedence >= parentPrecedence ||
                      binary_op_is_ambiguous_in_wgsl(op);
    std::string expr;
    if (needParens) {
        expr.push_back('(');
    }

    // We can use a scratch-let for argument 0 to dodge WGSL overflow errors. (skia:14385)
    std::string argument = this->assembleExpression(*call.arguments()[0], precedence);
    expr += all_arguments_constant(call.arguments()) ? this->writeScratchLet(argument)
                                                     : argument;
    expr += operator_name(op);
    expr += this->assembleExpression(*call.arguments()[1], precedence);

    if (needParens) {
        expr.push_back(')');
    }

    return expr;
}

std::string WGSLCodeGenerator::assemblePartialSampleCall(std::string_view functionName,
                                                         const Expression& sampler,
                                                         const Expression& coords) {
    // This function returns `functionName(inSampler_texture, inSampler_sampler, coords` without a
    // terminating comma or close-parenthesis. This allows the caller to add more arguments as
    // needed.
    SkASSERT(sampler.type().typeKind() == Type::TypeKind::kSampler);
    std::string expr = std::string(functionName) + '(';
    expr += this->assembleExpression(sampler, Precedence::kSequence);
    expr += kTextureSuffix;
    expr += ", ";
    expr += this->assembleExpression(sampler, Precedence::kSequence);
    expr += kSamplerSuffix;
    expr += ", ";

    // Compute the sample coordinates, dividing out the Z if a vec3 was provided.
    SkASSERT(coords.type().isVector());
    if (coords.type().columns() == 3) {
        // The coordinates were passed as a vec3, so we need to emit `coords.xy / coords.z`.
        std::string vec3Coords = this->writeScratchLet(coords, Precedence::kMultiplicative);
        expr += vec3Coords + ".xy / " + vec3Coords + ".z";
    } else {
        // The coordinates should be a plain vec2; emit the expression as-is.
        SkASSERT(coords.type().columns() == 2);
        expr += this->assembleExpression(coords, Precedence::kSequence);
    }

    return expr;
}

std::string WGSLCodeGenerator::assembleComponentwiseMatrixBinary(const Type& leftType,
                                                                 const Type& rightType,
                                                                 const std::string& left,
                                                                 const std::string& right,
                                                                 Operator op) {
    bool leftIsMatrix = leftType.isMatrix();
    bool rightIsMatrix = rightType.isMatrix();
    const Type& matrixType = leftIsMatrix ? leftType : rightType;

    std::string expr = to_wgsl_type(fContext, matrixType) + '(';
    auto separator = String::Separator();
    int columns = matrixType.columns();
    for (int c = 0; c < columns; ++c) {
        expr += separator();
        expr += left;
        if (leftIsMatrix) {
            expr += '[';
            expr += std::to_string(c);
            expr += ']';
        }
        expr += op.operatorName();
        expr += right;
        if (rightIsMatrix) {
            expr += '[';
            expr += std::to_string(c);
            expr += ']';
        }
    }
    return expr + ')';
}

std::string WGSLCodeGenerator::assembleIntrinsicCall(const FunctionCall& call,
                                                     IntrinsicKind kind,
                                                     Precedence parentPrecedence) {
    // Be careful: WGSL 1.0 will reject any intrinsic calls which can be constant-evaluated to
    // infinity or nan with a compile error. If all arguments to an intrinsic are compile-time
    // constants (`all_arguments_constant`), it is safest to copy one argument into a scratch-let so
    // that the call will be seen as runtime-evaluated, which defuses the overflow checks.
    // Don't worry; a competent driver should still optimize it away.

    const ExpressionArray& arguments = call.arguments();
    switch (kind) {
        case k_atan_IntrinsicKind: {
            const char* name = (arguments.size() == 1) ? "atan" : "atan2";
            return this->assembleSimpleIntrinsic(name, call);
        }
        case k_dFdx_IntrinsicKind:
            return this->assembleSimpleIntrinsic("dpdx", call);

        case k_dFdy_IntrinsicKind:
            // TODO(b/294274678): apply RTFlip here
            return this->assembleSimpleIntrinsic("dpdy", call);

        case k_dot_IntrinsicKind: {
            if (arguments[0]->type().isScalar()) {
                return this->assembleBinaryOpIntrinsic(OperatorKind::STAR, call, parentPrecedence);
            }
            return this->assembleSimpleIntrinsic("dot", call);
        }
        case k_equal_IntrinsicKind:
            return this->assembleBinaryOpIntrinsic(OperatorKind::EQEQ, call, parentPrecedence);

        case k_faceforward_IntrinsicKind: {
            if (arguments[0]->type().isScalar()) {
                // select(-N, N, (I * Nref) < 0)
                std::string N = this->writeNontrivialScratchLet(*arguments[0],
                                                                Precedence::kAssignment);
                return this->writeScratchLet(
                        "select(-" + N + ", " + N + ", " +
                        this->assembleBinaryExpression(*arguments[1],
                                                       OperatorKind::STAR,
                                                       *arguments[2],
                                                       arguments[1]->type(),
                                                       Precedence::kRelational) +
                        " < 0)");
            }
            return this->assembleSimpleIntrinsic("faceForward", call);
        }
        case k_greaterThan_IntrinsicKind:
            return this->assembleBinaryOpIntrinsic(OperatorKind::GT, call, parentPrecedence);

        case k_greaterThanEqual_IntrinsicKind:
            return this->assembleBinaryOpIntrinsic(OperatorKind::GTEQ, call, parentPrecedence);

        case k_inverse_IntrinsicKind:
            return this->assembleInversePolyfill(call);

        case k_inversesqrt_IntrinsicKind:
            return this->assembleSimpleIntrinsic("inverseSqrt", call);

        case k_lessThan_IntrinsicKind:
            return this->assembleBinaryOpIntrinsic(OperatorKind::LT, call, parentPrecedence);

        case k_lessThanEqual_IntrinsicKind:
            return this->assembleBinaryOpIntrinsic(OperatorKind::LTEQ, call, parentPrecedence);

        case k_matrixCompMult_IntrinsicKind: {
            // We use a scratch-let for arg0 to avoid the potential for WGSL overflow. (skia:14385)
            std::string arg0 = all_arguments_constant(arguments)
                            ? this->writeScratchLet(*arguments[0], Precedence::kPostfix)
                            : this->writeNontrivialScratchLet(*arguments[0], Precedence::kPostfix);
            std::string arg1 = this->writeNontrivialScratchLet(*arguments[1], Precedence::kPostfix);
            return this->writeScratchLet(
                    this->assembleComponentwiseMatrixBinary(arguments[0]->type(),
                                                            arguments[1]->type(),
                                                            arg0,
                                                            arg1,
                                                            OperatorKind::STAR));
        }
        case k_mix_IntrinsicKind: {
            const char* name = arguments[2]->type().componentType().isBoolean() ? "select" : "mix";
            return this->assembleVectorizedIntrinsic(name, call);
        }
        case k_mod_IntrinsicKind: {
            // WGSL has no intrinsic equivalent to `mod`. Synthesize `x - y * floor(x / y)`.
            // We can use a scratch-let on one side to dodge WGSL overflow errors.  In practice, I
            // can't find any values of x or y which would overflow, but it can't hurt. (skia:14385)
            std::string arg0 = all_arguments_constant(arguments)
                            ? this->writeScratchLet(*arguments[0], Precedence::kAdditive)
                            : this->writeNontrivialScratchLet(*arguments[0], Precedence::kAdditive);
            std::string arg1 = this->writeNontrivialScratchLet(*arguments[1],
                                                               Precedence::kAdditive);
            return this->writeScratchLet(arg0 + " - " + arg1 + " * floor(" +
                                         arg0 + " / " + arg1 + ")");
        }
        case k_normalize_IntrinsicKind: {
            const char* name = arguments[0]->type().isScalar() ? "sign" : "normalize";
            return this->assembleSimpleIntrinsic(name, call);
        }
        case k_not_IntrinsicKind:
            return this->assembleUnaryOpIntrinsic(OperatorKind::LOGICALNOT, call, parentPrecedence);

        case k_notEqual_IntrinsicKind:
            return this->assembleBinaryOpIntrinsic(OperatorKind::NEQ, call, parentPrecedence);

        case k_reflect_IntrinsicKind:
            if (arguments[0]->type().isScalar()) {
                // I - 2 * N * I * N
                // We can use a scratch-let for N to dodge WGSL overflow errors. (skia:14385)
                std::string I = this->writeNontrivialScratchLet(*arguments[0],
                                                                Precedence::kAdditive);
                std::string N = all_arguments_constant(arguments)
                      ? this->writeScratchLet(*arguments[1], Precedence::kMultiplicative)
                      : this->writeNontrivialScratchLet(*arguments[1], Precedence::kMultiplicative);
                return this->writeScratchLet(String::printf("%s - 2 * %s * %s * %s",
                                                            I.c_str(), N.c_str(),
                                                            I.c_str(), N.c_str()));
            }
            return this->assembleSimpleIntrinsic("reflect", call);

        case k_refract_IntrinsicKind:
            if (arguments[0]->type().isScalar()) {
                // WGSL only implements refract for vectors; rather than reimplementing refract from
                // scratch, we can replace the call with `refract(float2(I,0), float2(N,0), eta).x`.
                std::string I = this->writeNontrivialScratchLet(*arguments[0],
                                                                Precedence::kSequence);
                std::string N = this->writeNontrivialScratchLet(*arguments[1],
                                                                Precedence::kSequence);
                // We can use a scratch-let for Eta to avoid WGSL overflow errors. (skia:14385)
                std::string Eta = all_arguments_constant(arguments)
                      ? this->writeScratchLet(*arguments[2], Precedence::kSequence)
                      : this->writeNontrivialScratchLet(*arguments[2], Precedence::kSequence);
                return this->writeScratchLet(
                        String::printf("refract(vec2<%s>(%s, 0), vec2<%s>(%s, 0), %s).x",
                                       to_wgsl_type(fContext, arguments[0]->type()).c_str(),
                                       I.c_str(),
                                       to_wgsl_type(fContext, arguments[1]->type()).c_str(),
                                       N.c_str(),
                                       Eta.c_str()));
            }
            return this->assembleSimpleIntrinsic("refract", call);

        case k_sample_IntrinsicKind: {
            // Determine if a bias argument was passed in.
            SkASSERT(arguments.size() == 2 || arguments.size() == 3);
            bool callIncludesBias = (arguments.size() == 3);

            if (fProgram.fConfig->fSettings.fSharpenTextures || callIncludesBias) {
                // We need to supply a bias argument; this is a separate intrinsic in WGSL.
                std::string expr = this->assemblePartialSampleCall("textureSampleBias",
                                                                   *arguments[0],
                                                                   *arguments[1]);
                expr += ", ";
                if (callIncludesBias) {
                    expr += this->assembleExpression(*arguments[2], Precedence::kAdditive) +
                            " + ";
                }
                expr += skstd::to_string(fProgram.fConfig->fSettings.fSharpenTextures
                                                 ? kSharpenTexturesBias
                                                 : 0.0f);
                return expr + ')';
            }

            // No bias is necessary, so we can call `textureSample` directly.
            return this->assemblePartialSampleCall("textureSample",
                                                   *arguments[0],
                                                   *arguments[1]) + ')';
        }
        case k_sampleLod_IntrinsicKind: {
            std::string expr = this->assemblePartialSampleCall("textureSampleLevel",
                                                               *arguments[0],
                                                               *arguments[1]);
            expr += ", " + this->assembleExpression(*arguments[2], Precedence::kSequence);
            return expr + ')';
        }
        case k_sampleGrad_IntrinsicKind: {
            std::string expr = this->assemblePartialSampleCall("textureSampleGrad",
                                                               *arguments[0],
                                                               *arguments[1]);
            expr += ", " + this->assembleExpression(*arguments[2], Precedence::kSequence);
            expr += ", " + this->assembleExpression(*arguments[3], Precedence::kSequence);
            return expr + ')';
        }
        case k_textureHeight_IntrinsicKind:
            return this->assembleSimpleIntrinsic("textureDimensions", call) + ".y";

        case k_textureRead_IntrinsicKind: {
            // We need to inject an extra argument for the mip-level. We don't plan on using mipmaps
            // in our storage textures, so we can just pass zero.
            std::string tex = this->assembleExpression(*arguments[0], Precedence::kSequence);
            std::string pos = this->writeScratchLet(*arguments[1], Precedence::kSequence);
            return std::string("textureLoad(") + tex + ", " + pos + ", 0)";
        }
        case k_textureWidth_IntrinsicKind:
            return this->assembleSimpleIntrinsic("textureDimensions", call) + ".x";

        case k_textureWrite_IntrinsicKind:
            return this->assembleSimpleIntrinsic("textureStore", call);

        case k_clamp_IntrinsicKind:
        case k_max_IntrinsicKind:
        case k_min_IntrinsicKind:
        case k_smoothstep_IntrinsicKind:
        case k_step_IntrinsicKind:
            return this->assembleVectorizedIntrinsic(call.function().name(), call);

        case k_abs_IntrinsicKind:
        case k_acos_IntrinsicKind:
        case k_all_IntrinsicKind:
        case k_any_IntrinsicKind:
        case k_asin_IntrinsicKind:
        case k_atomicAdd_IntrinsicKind:
        case k_atomicLoad_IntrinsicKind:
        case k_atomicStore_IntrinsicKind:
        case k_ceil_IntrinsicKind:
        case k_cos_IntrinsicKind:
        case k_cross_IntrinsicKind:
        case k_degrees_IntrinsicKind:
        case k_distance_IntrinsicKind:
        case k_exp_IntrinsicKind:
        case k_exp2_IntrinsicKind:
        case k_floor_IntrinsicKind:
        case k_fract_IntrinsicKind:
        case k_length_IntrinsicKind:
        case k_log_IntrinsicKind:
        case k_log2_IntrinsicKind:
        case k_radians_IntrinsicKind:
        case k_pow_IntrinsicKind:
        case k_sign_IntrinsicKind:
        case k_sin_IntrinsicKind:
        case k_sqrt_IntrinsicKind:
        case k_storageBarrier_IntrinsicKind:
        case k_tan_IntrinsicKind:
        case k_workgroupBarrier_IntrinsicKind:
        default:
            return this->assembleSimpleIntrinsic(call.function().name(), call);
    }
}

static constexpr char kInverse2x2[] =
     "fn mat2_inverse(m: mat2x2<f32>) -> mat2x2<f32> {"
"\n"     "return mat2x2<f32>(m[1].y, -m[0].y, -m[1].x, m[0].x) * (1/determinant(m));"
"\n" "}"
"\n";

static constexpr char kInverse3x3[] =
     "fn mat3_inverse(m: mat3x3<f32>) -> mat3x3<f32> {"
"\n"     "let a00 = m[0].x; let a01 = m[0].y; let a02 = m[0].z;"
"\n"     "let a10 = m[1].x; let a11 = m[1].y; let a12 = m[1].z;"
"\n"     "let a20 = m[2].x; let a21 = m[2].y; let a22 = m[2].z;"
"\n"     "let b01 =  a22*a11 - a12*a21;"
"\n"     "let b11 = -a22*a10 + a12*a20;"
"\n"     "let b21 =  a21*a10 - a11*a20;"
"\n"     "let det = a00*b01 + a01*b11 + a02*b21;"
"\n"     "return mat3x3<f32>(b01, (-a22*a01 + a02*a21), ( a12*a01 - a02*a11),"
"\n"                        "b11, ( a22*a00 - a02*a20), (-a12*a00 + a02*a10),"
"\n"                        "b21, (-a21*a00 + a01*a20), ( a11*a00 - a01*a10)) * (1/det);"
"\n" "}"
"\n";

static constexpr char kInverse4x4[] =
     "fn mat4_inverse(m: mat4x4<f32>) -> mat4x4<f32>{"
"\n"     "let a00 = m[0].x; let a01 = m[0].y; let a02 = m[0].z; let a03 = m[0].w;"
"\n"     "let a10 = m[1].x; let a11 = m[1].y; let a12 = m[1].z; let a13 = m[1].w;"
"\n"     "let a20 = m[2].x; let a21 = m[2].y; let a22 = m[2].z; let a23 = m[2].w;"
"\n"     "let a30 = m[3].x; let a31 = m[3].y; let a32 = m[3].z; let a33 = m[3].w;"
"\n"     "let b00 = a00*a11 - a01*a10;"
"\n"     "let b01 = a00*a12 - a02*a10;"
"\n"     "let b02 = a00*a13 - a03*a10;"
"\n"     "let b03 = a01*a12 - a02*a11;"
"\n"     "let b04 = a01*a13 - a03*a11;"
"\n"     "let b05 = a02*a13 - a03*a12;"
"\n"     "let b06 = a20*a31 - a21*a30;"
"\n"     "let b07 = a20*a32 - a22*a30;"
"\n"     "let b08 = a20*a33 - a23*a30;"
"\n"     "let b09 = a21*a32 - a22*a31;"
"\n"     "let b10 = a21*a33 - a23*a31;"
"\n"     "let b11 = a22*a33 - a23*a32;"
"\n"     "let det = b00*b11 - b01*b10 + b02*b09 + b03*b08 - b04*b07 + b05*b06;"
"\n"     "return mat4x4<f32>(a11*b11 - a12*b10 + a13*b09,"
"\n"                        "a02*b10 - a01*b11 - a03*b09,"
"\n"                        "a31*b05 - a32*b04 + a33*b03,"
"\n"                        "a22*b04 - a21*b05 - a23*b03,"
"\n"                        "a12*b08 - a10*b11 - a13*b07,"
"\n"                        "a00*b11 - a02*b08 + a03*b07,"
"\n"                        "a32*b02 - a30*b05 - a33*b01,"
"\n"                        "a20*b05 - a22*b02 + a23*b01,"
"\n"                        "a10*b10 - a11*b08 + a13*b06,"
"\n"                        "a01*b08 - a00*b10 - a03*b06,"
"\n"                        "a30*b04 - a31*b02 + a33*b00,"
"\n"                        "a21*b02 - a20*b04 - a23*b00,"
"\n"                        "a11*b07 - a10*b09 - a12*b06,"
"\n"                        "a00*b09 - a01*b07 + a02*b06,"
"\n"                        "a31*b01 - a30*b03 - a32*b00,"
"\n"                        "a20*b03 - a21*b01 + a22*b00) * (1/det);"
"\n" "}"
"\n";

std::string WGSLCodeGenerator::assembleInversePolyfill(const FunctionCall& call) {
    const ExpressionArray& arguments = call.arguments();
    const Type& type = arguments.front()->type();

    // The `inverse` intrinsic should only accept a single-argument square matrix.
    // Once we implement f16 support, these polyfills will need to be updated to support `hmat`;
    // for the time being, all floats in WGSL are f32, so we don't need to worry about precision.
    SkASSERT(arguments.size() == 1);
    SkASSERT(type.isMatrix());
    SkASSERT(type.rows() == type.columns());

    switch (type.slotCount()) {
        case 4:
            if (!fWrittenInverse2) {
                fWrittenInverse2 = true;
                fHeader.writeText(kInverse2x2);
            }
            return this->assembleSimpleIntrinsic("mat2_inverse", call);

        case 9:
            if (!fWrittenInverse3) {
                fWrittenInverse3 = true;
                fHeader.writeText(kInverse3x3);
            }
            return this->assembleSimpleIntrinsic("mat3_inverse", call);

        case 16:
            if (!fWrittenInverse4) {
                fWrittenInverse4 = true;
                fHeader.writeText(kInverse4x4);
            }
            return this->assembleSimpleIntrinsic("mat4_inverse", call);

        default:
            // We only support square matrices.
            SkUNREACHABLE;
    }
}

std::string WGSLCodeGenerator::assembleFunctionCall(const FunctionCall& call,
                                                    Precedence parentPrecedence) {
    const FunctionDeclaration& func = call.function();
    std::string result;

    // Many intrinsics need to be rewritten in WGSL.
    if (func.isIntrinsic()) {
        return this->assembleIntrinsicCall(call, func.intrinsicKind(), parentPrecedence);
    }

    // We implement function out-parameters by declaring them as pointers. SkSL follows GLSL's
    // out-parameter semantics, in which out-parameters are only written back to the original
    // variable after the function's execution is complete (see
    // https://www.khronos.org/opengl/wiki/Core_Language_(GLSL)#Parameters).
    //
    // In addition, SkSL supports swizzles and array index expressions to be passed into
    // out-parameters; however, WGSL does not allow taking their address into a pointer.
    //
    // We support these by using LValues to create temporary copies and then pass pointers to the
    // copies. Once the function returns, we copy the values back to the LValue.

    // First detect which arguments are passed to out-parameters.
    // TODO: rewrite this method in terms of LValues.
    const ExpressionArray& args = call.arguments();
    SkSpan<Variable* const> params = func.parameters();
    SkASSERT(SkToSizeT(args.size()) == params.size());

    STArray<16, std::unique_ptr<LValue>> writeback;
    STArray<16, std::string> substituteArgument;
    writeback.reserve_exact(args.size());
    substituteArgument.reserve_exact(args.size());

    for (int index = 0; index < args.size(); ++index) {
        if (params[index]->modifierFlags() & ModifierFlag::kOut) {
            std::unique_ptr<LValue> lvalue = this->makeLValue(*args[index]);
            if (params[index]->modifierFlags() & ModifierFlag::kIn) {
                // Load the lvalue's contents into the substitute argument.
                substituteArgument.push_back(this->writeScratchVar(args[index]->type(),
                                                                   lvalue->load()));
            } else {
                // Create a substitute argument, but leave it uninitialized.
                substituteArgument.push_back(this->writeScratchVar(args[index]->type()));
            }
            writeback.push_back(std::move(lvalue));
        } else {
            substituteArgument.push_back(std::string());
            writeback.push_back(nullptr);
        }
    }

    std::string expr = this->assembleName(func.mangledName());
    expr.push_back('(');
    auto separator = SkSL::String::Separator();

    if (std::string funcDepArgs = this->functionDependencyArgs(func); !funcDepArgs.empty()) {
        expr += funcDepArgs;
        separator();
    }

    // Pass the function arguments, or any substitutes as needed.
    for (int index = 0; index < args.size(); ++index) {
        expr += separator();
        if (!substituteArgument[index].empty()) {
            // We need to take the address of the variable and pass it down as a pointer.
            expr += '&' + substituteArgument[index];
        } else if (args[index]->type().isSampler()) {
            // If the argument is a sampler, we need to pass the texture _and_ its associated
            // sampler. (Function parameter lists also convert sampler parameters into a matching
            // texture/sampler parameter pair.)
            expr += this->assembleExpression(*args[index], Precedence::kSequence);
            expr += kTextureSuffix;
            expr += ", ";
            expr += this->assembleExpression(*args[index], Precedence::kSequence);
            expr += kSamplerSuffix;
        } else {
            expr += this->assembleExpression(*args[index], Precedence::kSequence);
        }
    }
    expr += ')';

    if (call.type().isVoid()) {
        // Making function calls that result in `void` is only valid in on the left side of a
        // comma-sequence, or in a top-level statement. Emit the function call as a top-level
        // statement and return an empty string, as the result will not be used.
        SkASSERT(parentPrecedence >= Precedence::kSequence);
        this->write(expr);
        this->writeLine(";");
    } else {
        result = this->writeScratchLet(expr);
    }

    // Write the substitute arguments back into their lvalues.
    for (int index = 0; index < args.size(); ++index) {
        if (!substituteArgument[index].empty()) {
            this->writeLine(writeback[index]->store(substituteArgument[index]));
        }
    }

    // Return the result of invoking the function.
    return result;
}

std::string WGSLCodeGenerator::assembleIndexExpression(const IndexExpression& i) {
    // Put the index value into a let-expression.
    std::string idx = this->writeNontrivialScratchLet(*i.index(), Precedence::kExpression);
    return this->assembleExpression(*i.base(), Precedence::kPostfix) + "[" + idx + "]";
}

std::string WGSLCodeGenerator::assembleLiteral(const Literal& l) {
    const Type& type = l.type();
    if (type.isFloat() || type.isBoolean()) {
        return l.description(OperatorPrecedence::kExpression);
    }
    SkASSERT(type.isInteger());
    if (type.matches(*fContext.fTypes.fUInt)) {
        return std::to_string(l.intValue() & 0xffffffff) + "u";
    } else if (type.matches(*fContext.fTypes.fUShort)) {
        return std::to_string(l.intValue() & 0xffff) + "u";
    } else {
        return std::to_string(l.intValue());
    }
}

std::string WGSLCodeGenerator::assembleIncrementExpr(const Type& type) {
    // `type(`
    std::string expr = to_wgsl_type(fContext, type);
    expr.push_back('(');

    // `1, 1, 1...)`
    auto separator = SkSL::String::Separator();
    for (int slots = type.slotCount(); slots > 0; --slots) {
        expr += separator();
        expr += "1";
    }
    expr.push_back(')');
    return expr;
}

std::string WGSLCodeGenerator::assemblePrefixExpression(const PrefixExpression& p,
                                                        Precedence parentPrecedence) {
    // Unary + does nothing, so we omit it from the output.
    Operator op = p.getOperator();
    if (op.kind() == Operator::Kind::PLUS) {
        return this->assembleExpression(*p.operand(), Precedence::kPrefix);
    }

    // Pre-increment/decrement expressions have no direct equivalent in WGSL.
    if (op.kind() == Operator::Kind::PLUSPLUS || op.kind() == Operator::Kind::MINUSMINUS) {
        std::unique_ptr<LValue> lvalue = this->makeLValue(*p.operand());
        if (!lvalue) {
            return "";
        }

        // Generate the new value: `lvalue + type(1, 1, 1...)`.
        std::string newValue =
                lvalue->load() +
                (p.getOperator().kind() == Operator::Kind::PLUSPLUS ? " + " : " - ") +
                this->assembleIncrementExpr(p.operand()->type());
        this->writeLine(lvalue->store(newValue));
        return lvalue->load();
    }

    // WGSL natively supports unary negation/not expressions (!,~,-).
    SkASSERT(op.kind() == OperatorKind::LOGICALNOT ||
             op.kind() == OperatorKind::BITWISENOT ||
             op.kind() == OperatorKind::MINUS);

    // The unary negation operator only applies to scalars and vectors. For other mathematical
    // objects (such as matrices) we can express it as a multiplication by -1.
    std::string expr;
    const bool needsNegation = op.kind() == Operator::Kind::MINUS &&
                               !p.operand()->type().isScalar() && !p.operand()->type().isVector();
    const bool needParens = needsNegation || Precedence::kPrefix >= parentPrecedence;

    if (needParens) {
        expr.push_back('(');
    }

    if (needsNegation) {
        expr += "-1.0 * ";
        expr += this->assembleExpression(*p.operand(), Precedence::kMultiplicative);
    } else {
        expr += p.getOperator().tightOperatorName();
        expr += this->assembleExpression(*p.operand(), Precedence::kPrefix);
    }

    if (needParens) {
        expr.push_back(')');
    }

    return expr;
}

std::string WGSLCodeGenerator::assemblePostfixExpression(const PostfixExpression& p,
                                                         Precedence parentPrecedence) {
    SkASSERT(p.getOperator().kind() == Operator::Kind::PLUSPLUS ||
             p.getOperator().kind() == Operator::Kind::MINUSMINUS);

    // Post-increment/decrement expressions have no direct equivalent in WGSL; they do exist as a
    // standalone statement for convenience, but these aren't the same as SkSL's post-increments.
    std::unique_ptr<LValue> lvalue = this->makeLValue(*p.operand());
    if (!lvalue) {
        return "";
    }

    // If the expression is used, create a let-copy of the original value.
    // (At statement-level precedence, we know the value is unused and can skip this let-copy.)
    std::string originalValue;
    if (parentPrecedence != Precedence::kStatement) {
        originalValue = this->writeScratchLet(lvalue->load());
    }
    // Generate the new value: `lvalue + type(1, 1, 1...)`.
    std::string newValue = lvalue->load() +
                           (p.getOperator().kind() == Operator::Kind::PLUSPLUS ? " + " : " - ") +
                           this->assembleIncrementExpr(p.operand()->type());
    this->writeLine(lvalue->store(newValue));

    return originalValue;
}

std::string WGSLCodeGenerator::assembleSwizzle(const Swizzle& swizzle) {
    return this->assembleExpression(*swizzle.base(), Precedence::kPostfix) + "." +
           Swizzle::MaskString(swizzle.components());
}

std::string WGSLCodeGenerator::writeScratchVar(const Type& type, const std::string& value) {
    std::string scratchVarName = "_skTemp" + std::to_string(fScratchCount++);
    this->write("var ");
    this->write(scratchVarName);
    this->write(": ");
    this->write(to_wgsl_type(fContext, type));
    if (!value.empty()) {
        this->write(" = ");
        this->write(value);
    }
    this->writeLine(";");
    return scratchVarName;
}

std::string WGSLCodeGenerator::writeScratchLet(const std::string& expr) {
    std::string scratchVarName = "_skTemp" + std::to_string(fScratchCount++);
    this->write(fAtFunctionScope ? "let " : "const ");
    this->write(scratchVarName);
    this->write(" = ");
    this->write(expr);
    this->writeLine(";");
    return scratchVarName;
}

std::string WGSLCodeGenerator::writeScratchLet(const Expression& expr,
                                               Precedence parentPrecedence) {
    return this->writeScratchLet(this->assembleExpression(expr, parentPrecedence));
}

std::string WGSLCodeGenerator::writeNontrivialScratchLet(const Expression& expr,
                                                         Precedence parentPrecedence) {
    std::string result = this->assembleExpression(expr, parentPrecedence);
    return is_nontrivial_expression(expr) ? this->writeScratchLet(result)
                                          : result;
}

std::string WGSLCodeGenerator::assembleTernaryExpression(const TernaryExpression& t,
                                                         Precedence parentPrecedence) {
    std::string expr;

    // The trivial case is when neither branch has side effects and evaluate to a scalar or vector
    // type. This can be represented with a call to the WGSL `select` intrinsic although it doesn't
    // support short-circuiting.
    if ((t.type().isScalar() || t.type().isVector()) &&
        !Analysis::HasSideEffects(*t.test()) &&
        !Analysis::HasSideEffects(*t.ifTrue()) &&
        !Analysis::HasSideEffects(*t.ifFalse())) {

        bool needParens = Precedence::kTernary >= parentPrecedence;
        if (needParens) {
            expr.push_back('(');
        }
        expr += "select(";
        expr += this->assembleExpression(*t.ifFalse(), Precedence::kSequence);
        expr += ", ";
        expr += this->assembleExpression(*t.ifTrue(), Precedence::kSequence);
        expr += ", ";

        bool isVector = t.type().isVector();
        if (isVector) {
            // Splat the condition expression into a vector.
            expr += String::printf("vec%d<bool>(", t.type().columns());
        }
        expr += this->assembleExpression(*t.test(), Precedence::kSequence);
        if (isVector) {
            expr.push_back(')');
        }
        expr.push_back(')');
        if (needParens) {
            expr.push_back(')');
        }
    } else {
        // WGSL does not support ternary expressions. Instead, we hoist the expression out into the
        // surrounding block, convert it into an if statement, and write the result to a synthesized
        // variable. Instead of the original expression, we return that variable.
        expr = this->writeScratchVar(t.ifTrue()->type());

        std::string testExpr = this->assembleExpression(*t.test(), Precedence::kExpression);
        this->write("if ");
        this->write(testExpr);
        this->writeLine(" {");

        ++fIndentation;
        std::string trueExpr = this->assembleExpression(*t.ifTrue(), Precedence::kAssignment);
        this->write(expr);
        this->write(" = ");
        this->write(trueExpr);
        this->writeLine(";");
        --fIndentation;

        this->writeLine("} else {");

        ++fIndentation;
        std::string falseExpr = this->assembleExpression(*t.ifFalse(), Precedence::kAssignment);
        this->write(expr);
        this->write(" = ");
        this->write(falseExpr);
        this->writeLine(";");
        --fIndentation;

        this->writeLine("}");
    }
    return expr;
}

std::string WGSLCodeGenerator::variablePrefix(const Variable& v) {
    if (v.storage() == Variable::Storage::kGlobal) {
        // If the field refers to a pipeline IO parameter, then we access it via the synthesized IO
        // structs. We make an explicit exception for `sk_PointSize` which we declare as a
        // placeholder variable in global scope as it is not supported by WebGPU as a pipeline IO
        // parameter (see comments in `writeStageOutputStruct`).
        if (v.modifierFlags() & ModifierFlag::kIn) {
            return "_stageIn.";
        }
        if (v.modifierFlags() & ModifierFlag::kOut) {
            return "(*_stageOut).";
        }

        // If the field refers to an anonymous-interface-block structure, access it via the
        // synthesized `_uniform0` or `_storage1` global.
        if (const InterfaceBlock* ib = v.interfaceBlock()) {
            const Type& ibType = ib->var()->type().componentType();
            if (const std::string* ibName = fInterfaceBlockNameMap.find(&ibType)) {
                return *ibName + '.';
            }
        }

        // If the field refers to an top-level uniform, access it via the synthesized
        // `_globalUniforms` global. (Note that this should only occur in test code; Skia will
        // always put uniforms in an interface block.)
        if (is_in_global_uniforms(v)) {
            return "_globalUniforms.";
        }
    }

    return "";
}

std::string WGSLCodeGenerator::variableReferenceNameForLValue(const VariableReference& r) {
    const Variable& v = *r.variable();

    if ((v.storage() == Variable::Storage::kParameter &&
         v.modifierFlags() & ModifierFlag::kOut)) {
        // This is an out-parameter; it's pointer-typed, so we need to dereference it. We wrap the
        // dereference in parentheses, in case the value is used in an access expression later.
        return "(*" + this->assembleName(v.mangledName()) + ')';
    }

    return this->variablePrefix(v) + this->assembleName(v.mangledName());
}

std::string WGSLCodeGenerator::assembleVariableReference(const VariableReference& r) {
    // TODO(b/294274678): Correctly handle RTFlip for built-ins.
    const Variable& v = *r.variable();

    // Insert a conversion expression if this is a built-in variable whose type differs from the
    // SkSL.
    std::string expr;
    std::optional<std::string_view> conversion = needs_builtin_type_conversion(v);
    if (conversion.has_value()) {
        expr += *conversion;
        expr.push_back('(');
    }

    expr += this->variableReferenceNameForLValue(r);

    if (conversion.has_value()) {
        expr.push_back(')');
    }

    return expr;
}

std::string WGSLCodeGenerator::assembleAnyConstructor(const AnyConstructor& c) {
    std::string expr = to_wgsl_type(fContext, c.type());
    expr.push_back('(');
    auto separator = SkSL::String::Separator();
    for (const auto& e : c.argumentSpan()) {
        expr += separator();
        expr += this->assembleExpression(*e, Precedence::kSequence);
    }
    expr.push_back(')');
    return expr;
}

std::string WGSLCodeGenerator::assembleConstructorCompound(const ConstructorCompound& c) {
    if (c.type().isVector()) {
        return this->assembleConstructorCompoundVector(c);
    } else if (c.type().isMatrix()) {
        return this->assembleConstructorCompoundMatrix(c);
    } else {
        fContext.fErrors->error(c.fPosition, "unsupported compound constructor");
        return {};
    }
}

std::string WGSLCodeGenerator::assembleConstructorCompoundVector(const ConstructorCompound& c) {
    // WGSL supports constructing vectors from a mix of scalars and vectors but
    // not matrices (see https://www.w3.org/TR/WGSL/#type-constructor-expr).
    //
    // SkSL supports vec4(mat2x2) which we handle specially.
    if (c.type().columns() == 4 && c.argumentSpan().size() == 1) {
        const Expression& arg = *c.argumentSpan().front();
        if (arg.type().isMatrix()) {
            SkASSERT(arg.type().columns() == 2);
            SkASSERT(arg.type().rows() == 2);

            std::string matrix = this->writeNontrivialScratchLet(arg, Precedence::kPostfix);
            return String::printf("%s(%s[0], %s[1])", to_wgsl_type(fContext, c.type()).c_str(),
                                                      matrix.c_str(),
                                                      matrix.c_str());
        }
    }
    return this->assembleAnyConstructor(c);
}

std::string WGSLCodeGenerator::assembleConstructorCompoundMatrix(const ConstructorCompound& ctor) {
    SkASSERT(ctor.type().isMatrix());

    std::string expr = to_wgsl_type(fContext, ctor.type()) + '(';
    auto separator = String::Separator();
    for (const std::unique_ptr<Expression>& arg : ctor.arguments()) {
        SkASSERT(arg->type().isScalar() || arg->type().isVector());

        if (arg->type().isScalar()) {
            expr += separator();
            expr += this->assembleExpression(*arg, Precedence::kSequence);
        } else {
            std::string inner = this->writeNontrivialScratchLet(*arg, Precedence::kSequence);
            int numSlots = arg->type().slotCount();
            for (int slot = 0; slot < numSlots; ++slot) {
                String::appendf(&expr, "%s%s[%d]", separator().c_str(), inner.c_str(), slot);
            }
        }
    }
    return expr + ')';
}

std::string WGSLCodeGenerator::assembleConstructorDiagonalMatrix(
        const ConstructorDiagonalMatrix& c) {
    const Type& type = c.type();
    SkASSERT(type.isMatrix());
    SkASSERT(c.argument()->type().isScalar());

    // Evaluate the inner-expression, creating a scratch variable if necessary.
    std::string inner = this->writeNontrivialScratchLet(*c.argument(), Precedence::kAssignment);

    // Assemble a diagonal-matrix expression.
    std::string expr = to_wgsl_type(fContext, type) + '(';
    auto separator = String::Separator();
    for (int col = 0; col < type.columns(); ++col) {
        for (int row = 0; row < type.rows(); ++row) {
            expr += separator();
            if (col == row) {
                expr += inner;
            } else {
                expr += "0.0";
            }
        }
    }
    return expr + ')';
}

std::string WGSLCodeGenerator::assembleConstructorMatrixResize(
        const ConstructorMatrixResize& ctor) {
    std::string source = this->writeScratchLet(this->assembleExpression(*ctor.argument(),
                                                                        Precedence::kSequence));
    int columns = ctor.type().columns();
    int rows = ctor.type().rows();
    int sourceColumns = ctor.argument()->type().columns();
    int sourceRows = ctor.argument()->type().rows();
    auto separator = String::Separator();
    std::string expr = to_wgsl_type(fContext, ctor.type()) + '(';

    for (int c = 0; c < columns; ++c) {
        for (int r = 0; r < rows; ++r) {
            expr += separator();
            if (c < sourceColumns && r < sourceRows) {
                String::appendf(&expr, "%s[%d][%d]", source.c_str(), c, r);
            } else if (r == c) {
                expr += "1.0";
            } else {
                expr += "0.0";
            }
        }
    }

    return expr + ')';
}

std::string WGSLCodeGenerator::assembleEqualityExpression(const Type& left,
                                                          const std::string& leftName,
                                                          const Type& right,
                                                          const std::string& rightName,
                                                          Operator op,
                                                          Precedence parentPrecedence) {
    SkASSERT(op.kind() == OperatorKind::EQEQ || op.kind() == OperatorKind::NEQ);

    std::string expr;
    bool isEqual = (op.kind() == Operator::Kind::EQEQ);
    const char* const combiner = isEqual ? " && " : " || ";

    if (left.isMatrix()) {
        // Each matrix column must be compared as if it were an individual vector.
        SkASSERT(right.isMatrix());
        SkASSERT(left.rows() == right.rows());
        SkASSERT(left.columns() == right.columns());
        int columns = left.columns();
        const Type& vecType = left.componentType().toCompound(fContext,
                                                              /*columns=*/left.rows(),
                                                              /*rows=*/1);
        const char* separator = "(";
        for (int index = 0; index < columns; ++index) {
            expr += separator;
            std::string suffix = '[' + std::to_string(index) + ']';
            expr += this->assembleEqualityExpression(vecType, leftName + suffix,
                                                     vecType, rightName + suffix,
                                                     op, Precedence::kParentheses);
            separator = combiner;
        }
        return expr + ')';
    }

    if (left.isArray()) {
        SkASSERT(right.matches(left));
        const Type& indexedType = left.componentType();
        const char* separator = "(";
        for (int index = 0; index < left.columns(); ++index) {
            expr += separator;
            std::string suffix = '[' + std::to_string(index) + ']';
            expr += this->assembleEqualityExpression(indexedType, leftName + suffix,
                                                     indexedType, rightName + suffix,
                                                     op, Precedence::kParentheses);
            separator = combiner;
        }
        return expr + ')';
    }

    if (left.isStruct()) {
        // Recursively compare every field in the struct.
        SkASSERT(right.matches(left));
        SkSpan<const Field> fields = left.fields();

        const char* separator = "(";
        for (const Field& field : fields) {
            expr += separator;
            expr += this->assembleEqualityExpression(
                            *field.fType, leftName + '.' + this->assembleName(field.fName),
                            *field.fType, rightName + '.' + this->assembleName(field.fName),
                            op, Precedence::kParentheses);
            separator = combiner;
        }
        return expr + ')';
    }

    if (left.isVector()) {
        // Compare vectors via `all(x == y)` or `any(x != y)`.
        SkASSERT(right.isVector());
        SkASSERT(left.slotCount() == right.slotCount());

        expr += isEqual ? "all(" : "any(";
        expr += leftName;
        expr += operator_name(op);
        expr += rightName;
        return expr + ')';
    }

    // Compare scalars via `x == y`.
    SkASSERT(right.isScalar());
    if (parentPrecedence < Precedence::kSequence) {
        expr = '(';
    }
    expr += leftName;
    expr += operator_name(op);
    expr += rightName;
    if (parentPrecedence < Precedence::kSequence) {
        expr += ')';
    }
    return expr;
}

std::string WGSLCodeGenerator::assembleEqualityExpression(const Expression& left,
                                                          const Expression& right,
                                                          Operator op,
                                                          Precedence parentPrecedence) {
    std::string leftName, rightName;
    if (left.type().isScalar() || left.type().isVector()) {
        // WGSL supports scalar and vector comparisons natively. We know the expressions will only
        // be emitted once, so there isn't a benefit to creating a let-declaration.
        leftName = this->assembleExpression(left, Precedence::kParentheses);
        rightName = this->assembleExpression(right, Precedence::kParentheses);
    } else {
        leftName = this->writeNontrivialScratchLet(left, Precedence::kAssignment);
        rightName = this->writeNontrivialScratchLet(right, Precedence::kAssignment);
    }
    return this->assembleEqualityExpression(left.type(), leftName, right.type(), rightName,
                                            op, parentPrecedence);
}

void WGSLCodeGenerator::writeProgramElement(const ProgramElement& e) {
    switch (e.kind()) {
        case ProgramElement::Kind::kExtension:
            // TODO(skia:13092): WGSL supports extensions via the "enable" directive
            // (https://www.w3.org/TR/WGSL/#enable-extensions-sec ). While we could easily emit this
            // directive, we should first ensure that all possible SkSL extension names are
            // converted to their appropriate WGSL extension.
            break;
        case ProgramElement::Kind::kGlobalVar:
            this->writeGlobalVarDeclaration(e.as<GlobalVarDeclaration>());
            break;
        case ProgramElement::Kind::kInterfaceBlock:
            // All interface block declarations are handled explicitly as the "program header" in
            // generateCode().
            break;
        case ProgramElement::Kind::kStructDefinition:
            this->writeStructDefinition(e.as<StructDefinition>());
            break;
        case ProgramElement::Kind::kFunctionPrototype:
            // A WGSL function declaration must contain its body and the function name is in scope
            // for the entire program (see https://www.w3.org/TR/WGSL/#function-declaration and
            // https://www.w3.org/TR/WGSL/#declaration-and-scope).
            //
            // As such, we don't emit function prototypes.
            break;
        case ProgramElement::Kind::kFunction:
            this->writeFunction(e.as<FunctionDefinition>());
            break;
        case ProgramElement::Kind::kModifiers:
            this->writeModifiersDeclaration(e.as<ModifiersDeclaration>());
            break;
        default:
            SkDEBUGFAILF("unsupported program element: %s\n", e.description().c_str());
            break;
    }
}

void WGSLCodeGenerator::writeTextureOrSampler(const Variable& var,
                                              int bindingLocation,
                                              std::string_view suffix,
                                              std::string_view wgslType) {
    if (var.type().dimensions() != SpvDim2D) {
        // Skia currently only uses 2D textures.
        fContext.fErrors->error(var.varDeclaration()->position(), "unsupported texture dimensions");
        return;
    }

    this->write("@group(");
    this->write(std::to_string(std::max(0, var.layout().fSet)));
    this->write(") @binding(");
    this->write(std::to_string(bindingLocation));
    this->write(") var ");
    this->write(this->assembleName(var.mangledName()));
    this->write(suffix);
    this->write(": ");
    this->write(wgslType);
    this->writeLine(";");
}

void WGSLCodeGenerator::writeGlobalVarDeclaration(const GlobalVarDeclaration& d) {
    const VarDeclaration& decl = d.varDeclaration();
    const Variable& var = *decl.var();
    if ((var.modifierFlags() & (ModifierFlag::kIn | ModifierFlag::kOut)) ||
        is_in_global_uniforms(var)) {
        // Pipeline stage I/O parameters and top-level (non-block) uniforms are handled specially
        // in generateCode().
        return;
    }

    const Type::TypeKind varKind = var.type().typeKind();
    if (varKind == Type::TypeKind::kSampler) {
        // If the sampler binding was unassigned, provide a scratch value; this will make
        // golden-output tests pass, but will not actually be usable for drawing.
        int samplerLocation = var.layout().fSampler >= 0 ? var.layout().fSampler
                                                         : 10000 + fScratchCount++;
        this->writeTextureOrSampler(var, samplerLocation, kSamplerSuffix, "sampler");

        // If the texture binding was unassigned, provide a scratch value (for golden-output tests).
        int textureLocation = var.layout().fTexture >= 0 ? var.layout().fTexture
                                                         : 10000 + fScratchCount++;
        this->writeTextureOrSampler(var, textureLocation, kTextureSuffix, "texture_2d<f32>");
        return;
    }

    if (varKind == Type::TypeKind::kTexture) {
        // If a binding location was unassigned, provide a scratch value (for golden-output tests).
        int textureLocation = var.layout().fBinding >= 0 ? var.layout().fBinding
                                                         : 10000 + fScratchCount++;
        // For a texture without an associated sampler, we don't apply a suffix.
        this->writeTextureOrSampler(var, textureLocation, /*suffix=*/"",
                                    to_wgsl_type(fContext, var.type(), &var.layout()));
        return;
    }

    std::string initializer;
    if (decl.value()) {
        // We assume here that the initial-value expression will not emit any helper statements.
        // Initial-value expressions are required to pass IsConstantExpression, which limits the
        // blast radius to constructors, literals, and other constant values/variables.
        initializer += " = ";
        initializer += this->assembleExpression(*decl.value(), Precedence::kAssignment);
    }

    if (var.modifierFlags().isConst()) {
        this->write("const ");
    } else if (var.modifierFlags().isWorkgroup()) {
        this->write("var<workgroup> ");
    } else if (var.modifierFlags().isPixelLocal()) {
        this->write("var<pixel_local> ");
    } else {
        this->write("var<private> ");
    }
    this->write(this->assembleName(var.mangledName()));
    this->write(": " + to_wgsl_type(fContext, var.type(), &var.layout()));
    this->write(initializer);
    this->writeLine(";");
}

void WGSLCodeGenerator::writeStructDefinition(const StructDefinition& s) {
    const Type& type = s.type();
    this->writeLine("struct " + type.displayName() + " {");
    this->writeFields(type.fields(), /*memoryLayout=*/nullptr);
    this->writeLine("};");
}

void WGSLCodeGenerator::writeModifiersDeclaration(const ModifiersDeclaration& modifiers) {
    LayoutFlags flags = modifiers.layout().fFlags;
    flags &= ~(LayoutFlag::kLocalSizeX | LayoutFlag::kLocalSizeY | LayoutFlag::kLocalSizeZ);
    if (flags != LayoutFlag::kNone) {
        fContext.fErrors->error(modifiers.position(), "unsupported declaration");
        return;
    }

    if (modifiers.layout().fLocalSizeX >= 0) {
        fLocalSizeX = modifiers.layout().fLocalSizeX;
    }
    if (modifiers.layout().fLocalSizeY >= 0) {
        fLocalSizeY = modifiers.layout().fLocalSizeY;
    }
    if (modifiers.layout().fLocalSizeZ >= 0) {
        fLocalSizeZ = modifiers.layout().fLocalSizeZ;
    }
}

void WGSLCodeGenerator::writeFields(SkSpan<const Field> fields, const MemoryLayout* memoryLayout) {
    fIndentation++;

    // TODO(skia:14370): array uniforms may need manual fixup for std140 padding. (Those uniforms
    // will also need special handling when they are accessed, or passed to functions.)
    for (size_t index = 0; index < fields.size(); ++index) {
        const Field& field = fields[index];
        if (memoryLayout && !memoryLayout->isSupported(*field.fType)) {
            // Reject types that aren't supported by the memory layout.
            fContext.fErrors->error(field.fPosition, "type '" + std::string(field.fType->name()) +
                                                     "' is not permitted here");
            return;
        }

        // Prepend @size(n) to enforce the offsets from the SkSL layout. (This is effectively
        // a gadget that we can use to insert padding between elements.)
        if (index < fields.size() - 1) {
            int thisFieldOffset = field.fLayout.fOffset;
            int nextFieldOffset = fields[index + 1].fLayout.fOffset;
            if (index == 0 && thisFieldOffset > 0) {
                fContext.fErrors->error(field.fPosition, "field must have an offset of zero");
                return;
            }
            if (thisFieldOffset >= 0 && nextFieldOffset > thisFieldOffset) {
                this->write("@size(");
                this->write(std::to_string(nextFieldOffset - thisFieldOffset));
                this->write(") ");
            }
        }

        this->write(this->assembleName(field.fName));
        this->write(": ");
        if (const FieldPolyfillInfo* info = fFieldPolyfillMap.find(&field)) {
            if (info->fIsArray) {
                // This properly handles arrays of matrices, as well as arrays of other primitives.
                SkASSERT(field.fType->isArray());
                this->write("array<_skArrayElement_");
                this->write(field.fType->abbreviatedName());
                this->write(", ");
                this->write(std::to_string(field.fType->columns()));
                this->write(">");
            } else if (info->fIsMatrix) {
                this->write("_skMatrix");
                this->write(std::to_string(field.fType->columns()));
                this->write(std::to_string(field.fType->rows()));
            } else {
                SkDEBUGFAILF("need polyfill for %s", info->fReplacementName.c_str());
            }
        } else {
            this->write(to_wgsl_type(fContext, *field.fType, &field.fLayout));
        }
        this->writeLine(",");
    }

    fIndentation--;
}

void WGSLCodeGenerator::writeEnables() {
    this->writeLine("diagnostic(off, derivative_uniformity);");
    if (fRequirements.fPixelLocalExtension) {
        this->writeLine("enable chromium_experimental_pixel_local;");
    }
}

bool WGSLCodeGenerator::needsStageInputStruct() const {
    // It is illegal to declare a struct with no members; we can't emit a placeholder empty stage
    // input struct.
    return !fPipelineInputs.empty();
}

void WGSLCodeGenerator::writeStageInputStruct() {
    if (!this->needsStageInputStruct()) {
        return;
    }

    std::string_view structNamePrefix = pipeline_struct_prefix(fProgram.fConfig->fKind);
    SkASSERT(!structNamePrefix.empty());

    this->write("struct ");
    this->write(structNamePrefix);
    this->writeLine("In {");
    fIndentation++;

    for (const Variable* v : fPipelineInputs) {
        if (v->interfaceBlock()) {
            for (const Field& f : v->type().fields()) {
                this->writePipelineIODeclaration(f.fLayout, *f.fType, f.fName, Delimiter::kComma);
            }
        } else {
            this->writePipelineIODeclaration(v->layout(), v->type(), v->mangledName(),
                                             Delimiter::kComma);
        }
    }

    fIndentation--;
    this->writeLine("};");
}

bool WGSLCodeGenerator::needsStageOutputStruct() const {
    // It is illegal to declare a struct with no members. However, vertex programs will _always_
    // have an output stage in WGSL, because the spec requires them to emit `@builtin(position)`.
    // So we always synthesize a reference to `sk_Position` even if the program doesn't need it.
    return !fPipelineOutputs.empty() || ProgramConfig::IsVertex(fProgram.fConfig->fKind);
}

void WGSLCodeGenerator::writeStageOutputStruct() {
    if (!this->needsStageOutputStruct()) {
        return;
    }

    std::string_view structNamePrefix = pipeline_struct_prefix(fProgram.fConfig->fKind);
    SkASSERT(!structNamePrefix.empty());

    this->write("struct ");
    this->write(structNamePrefix);
    this->writeLine("Out {");
    fIndentation++;

    bool declaredPositionBuiltin = false;
    bool requiresPointSizeBuiltin = false;
    for (const Variable* v : fPipelineOutputs) {
        if (v->interfaceBlock()) {
            for (const auto& f : v->type().fields()) {
                this->writePipelineIODeclaration(f.fLayout, *f.fType, f.fName, Delimiter::kComma);
                if (f.fLayout.fBuiltin == SK_POSITION_BUILTIN) {
                    declaredPositionBuiltin = true;
                } else if (f.fLayout.fBuiltin == SK_POINTSIZE_BUILTIN) {
                    // sk_PointSize is explicitly not supported by `builtin_from_sksl_name` so
                    // writePipelineIODeclaration will never write it. We mark it here if the
                    // declaration is needed so we can synthesize it below.
                    requiresPointSizeBuiltin = true;
                }
            }

        } else {
            this->writePipelineIODeclaration(v->layout(), v->type(), v->mangledName(),
                                             Delimiter::kComma);
        }
    }

    // A vertex program must include the `position` builtin in its entrypoint's return type.
    const bool positionBuiltinRequired = ProgramConfig::IsVertex(fProgram.fConfig->fKind);
    if (positionBuiltinRequired && !declaredPositionBuiltin) {
        this->writeLine("@builtin(position) sk_Position: vec4<f32>,");
    }

    fIndentation--;
    this->writeLine("};");

    // In WebGPU/WGSL, the vertex stage does not support a point-size output and the size
    // of a point primitive is always 1 pixel (see https://github.com/gpuweb/gpuweb/issues/332).
    //
    // There isn't anything we can do to emulate this correctly at this stage so we synthesize a
    // placeholder global variable that has no effect. Programs should not rely on sk_PointSize when
    // using the Dawn backend.
    if (ProgramConfig::IsVertex(fProgram.fConfig->fKind) && requiresPointSizeBuiltin) {
        this->writeLine("/* unsupported */ var<private> sk_PointSize: f32;");
    }
}

void WGSLCodeGenerator::prepareUniformPolyfillsForInterfaceBlock(
        const InterfaceBlock* interfaceBlock,
        std::string_view instanceName,
        MemoryLayout::Standard nativeLayout) {
    SkSL::MemoryLayout std140(MemoryLayout::Standard::k140);
    SkSL::MemoryLayout native(nativeLayout);

    const Type& structType = interfaceBlock->var()->type().componentType();
    for (const Field& field : structType.fields()) {
        const Type* type = field.fType;
        bool needsArrayPolyfill = false;
        bool needsMatrixPolyfill = false;

        auto isPolyfillableMatrixType = [&](const Type* type) {
            return type->isMatrix() && std140.stride(*type) != native.stride(*type);
        };

        if (isPolyfillableMatrixType(type)) {
            // Matrices will be represented as 16-byte aligned arrays in std140, and reconstituted
            // into proper matrices as they are later accessed. We need to synthesize polyfill.
            needsMatrixPolyfill = true;
        } else if (type->isArray() && !type->isUnsizedArray() &&
                   !type->componentType().isOpaque()) {
            const Type* innerType = &type->componentType();
            if (isPolyfillableMatrixType(innerType)) {
                // Use a polyfill when the array contains a matrix that requires polyfill.
                needsArrayPolyfill = true;
                needsMatrixPolyfill = true;
            } else if (native.size(*innerType) < 16) {
                // Use a polyfill when the array elements are smaller than 16 bytes, since std140
                // will pad elements to a 16-byte stride.
                needsArrayPolyfill = true;
            }
        }

        if (needsArrayPolyfill || needsMatrixPolyfill) {
            // Add a polyfill for this matrix type.
            FieldPolyfillInfo info;
            info.fInterfaceBlock = interfaceBlock;
            info.fReplacementName = "_skUnpacked_" + std::string(instanceName) + '_' +
                                    this->assembleName(field.fName);
            info.fIsArray = needsArrayPolyfill;
            info.fIsMatrix = needsMatrixPolyfill;
            fFieldPolyfillMap.set(&field, info);
        }
    }
}

void WGSLCodeGenerator::writeUniformsAndBuffers() {
    for (const ProgramElement* e : fProgram.elements()) {
        // Iterate through the interface blocks.
        if (!e->is<InterfaceBlock>()) {
            continue;
        }
        const InterfaceBlock& ib = e->as<InterfaceBlock>();

        // Determine if this interface block holds uniforms, buffers, or something else (skip it).
        std::string_view addressSpace;
        std::string_view accessMode;
        MemoryLayout::Standard nativeLayout;
        if (ib.var()->modifierFlags().isUniform()) {
            addressSpace = "uniform";
            nativeLayout = MemoryLayout::Standard::kWGSLUniform_Base;
        } else if (ib.var()->modifierFlags().isBuffer()) {
            addressSpace = "storage";
            nativeLayout = MemoryLayout::Standard::kWGSLStorage_Base;
            accessMode = ib.var()->modifierFlags().isReadOnly() ? ", read" : ", read_write";
        } else {
            continue;
        }

        // If we have an anonymous interface block, assign a name like `_uniform0` or `_storage1`.
        std::string instanceName;
        if (ib.instanceName().empty()) {
            instanceName = "_" + std::string(addressSpace) + std::to_string(fScratchCount++);
            fInterfaceBlockNameMap[&ib.var()->type().componentType()] = instanceName;
        } else {
            instanceName = std::string(ib.instanceName());
        }

        this->prepareUniformPolyfillsForInterfaceBlock(&ib, instanceName, nativeLayout);

        // Create a struct to hold all of the fields from this InterfaceBlock.
        SkASSERT(!ib.typeName().empty());
        this->write("struct ");
        this->write(ib.typeName());
        this->writeLine(" {");

        // Find the struct type and fields used by this interface block.
        const Type& ibType = ib.var()->type().componentType();
        SkASSERT(ibType.isStruct());

        SkSpan<const Field> ibFields = ibType.fields();
        SkASSERT(!ibFields.empty());

        MemoryLayout layout(MemoryLayout::Standard::k140);
        this->writeFields(ibFields, &layout);
        this->writeLine("};");
        this->write("@group(");
        this->write(std::to_string(std::max(0, ib.var()->layout().fSet)));
        this->write(") @binding(");
        this->write(std::to_string(std::max(0, ib.var()->layout().fBinding)));
        this->write(") var<");
        this->write(addressSpace);
        this->write(accessMode);
        this->write("> ");
        this->write(instanceName);
        this->write(" : ");
        this->write(to_wgsl_type(fContext, ib.var()->type(), &ib.var()->layout()));
        this->writeLine(";");
    }
}

void WGSLCodeGenerator::writeNonBlockUniformsForTests() {
    bool declaredUniformsStruct = false;

    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const GlobalVarDeclaration& decls = e->as<GlobalVarDeclaration>();
            const Variable& var = *decls.varDeclaration().var();
            if (is_in_global_uniforms(var)) {
                if (!declaredUniformsStruct) {
                    this->write("struct _GlobalUniforms {\n");
                    declaredUniformsStruct = true;
                }
                this->write("  ");
                this->writeVariableDecl(var.layout(), var.type(), var.mangledName(),
                                        Delimiter::kComma);
            }
        }
    }
    if (declaredUniformsStruct) {
        int binding = fProgram.fConfig->fSettings.fDefaultUniformBinding;
        int set = fProgram.fConfig->fSettings.fDefaultUniformSet;
        this->write("};\n");
        this->write("@binding(" + std::to_string(binding) + ") ");
        this->write("@group(" + std::to_string(set) + ") ");
        this->writeLine("var<uniform> _globalUniforms: _GlobalUniforms;");
    }
}

std::string WGSLCodeGenerator::functionDependencyArgs(const FunctionDeclaration& f) {
    WGSLFunctionDependencies* deps = fRequirements.fDependencies.find(&f);
    std::string args;
    if (deps && *deps) {
        const char* separator = "";
        if (*deps & WGSLFunctionDependency::kPipelineInputs) {
            args += "_stageIn";
            separator = ", ";
        }
        if (*deps & WGSLFunctionDependency::kPipelineOutputs) {
            args += separator;
            args += "_stageOut";
        }
    }
    return args;
}

bool WGSLCodeGenerator::writeFunctionDependencyParams(const FunctionDeclaration& f) {
    WGSLFunctionDependencies* deps = fRequirements.fDependencies.find(&f);
    if (!deps || !*deps) {
        return false;
    }

    std::string_view structNamePrefix = pipeline_struct_prefix(fProgram.fConfig->fKind);
    if (structNamePrefix.empty()) {
        return false;
    }
    const char* separator = "";
    if (*deps & WGSLFunctionDependency::kPipelineInputs) {
        this->write("_stageIn: ");
        separator = ", ";
        this->write(structNamePrefix);
        this->write("In");
    }
    if (*deps & WGSLFunctionDependency::kPipelineOutputs) {
        this->write(separator);
        this->write("_stageOut: ptr<function, ");
        this->write(structNamePrefix);
        this->write("Out>");
    }
    return true;
}

}  // namespace SkSL
