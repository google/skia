/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/codegen/SkSLWGSLCodeGenerator.h"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/SkBitmaskEnum.h"
#include "include/private/SkSLIRNode.h"
#include "include/private/SkSLLayout.h"
#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "include/private/SkSLString.h"
#include "include/private/SkSLSymbol.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "include/sksl/SkSLErrorReporter.h"
#include "include/sksl/SkSLOperator.h"
#include "include/sksl/SkSLPosition.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLOutputStream.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLConstructorDiagonalMatrix.h"
#include "src/sksl/ir/SkSLConstructorMatrixResize.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLStructDefinition.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"

// TODO(skia:13092): This is a temporary debug feature. Remove when the implementation is
// complete and this is no longer needed.
#define DUMP_SRC_IR 0

namespace SkSL {

enum class ProgramKind : int8_t;

namespace {

// See https://www.w3.org/TR/WGSL/#memory-view-types
enum class PtrAddressSpace {
    kFunction,
    kPrivate,
    kStorage,
};

std::string_view pipeline_struct_prefix(ProgramKind kind) {
    if (ProgramConfig::IsVertex(kind)) {
        return "VS";
    }
    if (ProgramConfig::IsFragment(kind)) {
        return "FS";
    }
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
std::string to_wgsl_type(const Type& type) {
    switch (type.typeKind()) {
        case Type::TypeKind::kScalar:
            return std::string(to_scalar_type(type));
        case Type::TypeKind::kVector: {
            std::string_view ct = to_scalar_type(type.componentType());
            return String::printf("vec%d<%.*s>", type.columns(), (int)ct.length(), ct.data());
        }
        case Type::TypeKind::kMatrix: {
            std::string_view ct = to_scalar_type(type.componentType());
            return String::printf(
                    "mat%dx%d<%.*s>", type.columns(), type.rows(), (int)ct.length(), ct.data());
        }
        case Type::TypeKind::kArray: {
            std::string elementType = to_wgsl_type(type.componentType());
            if (type.isUnsizedArray()) {
                return String::printf("array<%s>", elementType.c_str());
            }
            return String::printf("array<%s, %d>", elementType.c_str(), type.columns());
        }
        default:
            break;
    }
    return std::string(type.name());
}

// Create a mangled WGSL type name that can be used in function and variable declarations (regular
// type names cannot be used in this manner since they may contain tokens that are not allowed in
// symbol names).
std::string to_mangled_wgsl_type_name(const Type& type) {
    switch (type.typeKind()) {
        case Type::TypeKind::kScalar:
            return std::string(to_scalar_type(type));
        case Type::TypeKind::kVector: {
            std::string_view ct = to_scalar_type(type.componentType());
            return String::printf("vec%d%.*s", type.columns(), (int)ct.length(), ct.data());
        }
        case Type::TypeKind::kMatrix: {
            std::string_view ct = to_scalar_type(type.componentType());
            return String::printf(
                    "mat%dx%d%.*s", type.columns(), type.rows(), (int)ct.length(), ct.data());
        }
        case Type::TypeKind::kArray: {
            std::string elementType = to_wgsl_type(type.componentType());
            if (type.isUnsizedArray()) {
                return String::printf("arrayof%s", elementType.c_str());
            }
            return String::printf("array%dof%s", type.columns(), elementType.c_str());
        }
        default:
            break;
    }
    return std::string(type.name());
}

std::string to_ptr_type(const Type& type,
                        PtrAddressSpace addressSpace = PtrAddressSpace::kFunction) {
    return "ptr<" + std::string(address_space_to_str(addressSpace)) + ", " + to_wgsl_type(type) +
           ">";
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
    switch (v.modifiers().fLayout.fBuiltin) {
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
            return {Builtin::kPosition};
        case SK_VERTEXID_BUILTIN:
            return {Builtin::kVertexIndex};
        case SK_INSTANCEID_BUILTIN:
            return {Builtin::kInstanceIndex};
        case SK_CLOCKWISE_BUILTIN:
            // TODO(skia:13092): While `front_facing` is the corresponding built-in, it does not
            // imply a particular winding order. We correctly compute the face orientation based
            // on how Skia configured the render pipeline for all references to this built-in
            // variable (see `SkSL::Program::Inputs::fUseFlipRTUniform`).
            return {Builtin::kFrontFacing};
        default:
            break;
    }
    return std::nullopt;
}

const SymbolTable* top_level_symbol_table(const FunctionDefinition& f) {
    return f.body()->as<Block>().symbolTable()->fParent.get();
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
    using Deps = WGSLCodeGenerator::FunctionDependencies;
    using DepsMap = WGSLCodeGenerator::ProgramRequirements::DepsMap;

    FunctionDependencyResolver(const Program* p,
                               const FunctionDeclaration* f,
                               DepsMap* programDependencyMap)
            : fProgram(p), fFunction(f), fDependencyMap(programDependencyMap) {}

    Deps resolve() {
        fDeps = Deps::kNone;
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
            const Modifiers& modifiers = v.variable()->modifiers();
            if (v.variable()->storage() == Variable::Storage::kGlobal) {
                if (modifiers.fFlags & Modifiers::kIn_Flag) {
                    fDeps |= Deps::kPipelineInputs;
                }
                if (modifiers.fFlags & Modifiers::kOut_Flag) {
                    fDeps |= Deps::kPipelineOutputs;
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
    Deps fDeps = Deps::kNone;

    using INHERITED = ProgramVisitor;
};

WGSLCodeGenerator::ProgramRequirements resolve_program_requirements(const Program* program) {
    bool mainNeedsCoordsArgument = false;
    WGSLCodeGenerator::ProgramRequirements::DepsMap dependencies;

    for (const ProgramElement* e : program->elements()) {
        if (!e->is<FunctionDefinition>()) {
            continue;
        }

        const FunctionDeclaration& decl = e->as<FunctionDefinition>().declaration();
        if (decl.isMain()) {
            for (const Variable* v : decl.parameters()) {
                if (v->modifiers().fLayout.fBuiltin == SK_MAIN_COORDS_BUILTIN) {
                    mainNeedsCoordsArgument = true;
                    break;
                }
            }
        }

        FunctionDependencyResolver resolver(program, &decl, &dependencies);
        dependencies.set(&decl, resolver.resolve());
    }

    return WGSLCodeGenerator::ProgramRequirements(std::move(dependencies), mainNeedsCoordsArgument);
}

int count_pipeline_inputs(const Program* program) {
    int inputCount = 0;
    for (const ProgramElement* e : program->elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const Variable* v = e->as<GlobalVarDeclaration>().varDeclaration().var();
            if (v->modifiers().fFlags & Modifiers::kIn_Flag) {
                inputCount++;
            }
        } else if (e->is<InterfaceBlock>()) {
            const Variable* v = e->as<InterfaceBlock>().var();
            if (v->modifiers().fFlags & Modifiers::kIn_Flag) {
                inputCount++;
            }
        }
    }
    return inputCount;
}

static bool is_in_global_uniforms(const Variable& var) {
    SkASSERT(var.storage() == VariableStorage::kGlobal);
    return var.modifiers().fFlags & Modifiers::kUniform_Flag && !var.type().isOpaque();
}

}  // namespace

bool WGSLCodeGenerator::generateCode() {
    // The resources of a WGSL program are structured in the following way:
    // - Vertex and fragment stage attribute inputs and outputs are bundled
    //   inside synthetic structs called VSIn/VSOut/FSIn/FSOut.
    // - All uniform and storage type resources are declared in global scope.
    this->preprocessProgram();

    StringStream header;
    {
        AutoOutputStream outputToHeader(this, &header, &fIndentation);
        // TODO(skia:13092): Implement the following:
        // - global uniform/storage resource declarations, including interface blocks.
        this->writeStageInputStruct();
        this->writeStageOutputStruct();
        this->writeNonBlockUniformsForTests();
    }
    StringStream body;
    {
        AutoOutputStream outputToBody(this, &body, &fIndentation);
        for (const ProgramElement* e : fProgram.elements()) {
            this->writeProgramElement(*e);
        }

// TODO(skia:13092): This is a temporary debug feature. Remove when the implementation is
// complete and this is no longer needed.
#if DUMP_SRC_IR
        this->writeLine("\n----------");
        this->writeLine("Source IR:\n");
        for (const ProgramElement* e : fProgram.elements()) {
            this->writeLine(e->description().c_str());
        }
#endif
    }

    write_stringstream(header, *fOut);
    write_stringstream(fExtraFunctions, *fOut);
    write_stringstream(body, *fOut);
    return fContext.fErrors->errorCount() == 0;
}

void WGSLCodeGenerator::preprocessProgram() {
    fRequirements = resolve_program_requirements(&fProgram);
    fPipelineInputCount = count_pipeline_inputs(&fProgram);
}

void WGSLCodeGenerator::write(std::string_view s) {
    if (s.empty()) {
        return;
    }
    if (fAtLineStart) {
        for (int i = 0; i < fIndentation; i++) {
            fOut->writeText("    ");
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

void WGSLCodeGenerator::writeName(std::string_view name) {
    // Add underscore before name to avoid conflict with reserved words.
    if (fReservedWords.contains(name)) {
        this->write("_");
    }
    this->write(name);
}

void WGSLCodeGenerator::writeVariableDecl(const Type& type,
                                          std::string_view name,
                                          Delimiter delimiter) {
    this->writeName(name);
    this->write(": " + to_wgsl_type(type));
    this->writeLine(delimiter_to_str(delimiter));
}

void WGSLCodeGenerator::writePipelineIODeclaration(Modifiers modifiers,
                                                   const Type& type,
                                                   std::string_view name,
                                                   Delimiter delimiter) {
    // In WGSL, an entry-point IO parameter is "one of either a built-in value or
    // assigned a location". However, some SkSL declarations, specifically sk_FragColor, can
    // contain both a location and a builtin modifier. In addition, WGSL doesn't have a built-in
    // equivalent for sk_FragColor as it relies on the user-defined location for a render
    // target.
    //
    // Instead of special-casing sk_FragColor, we just give higher precedence to a location
    // modifier if a declaration happens to both have a location and it's a built-in.
    //
    // Also see:
    // https://www.w3.org/TR/WGSL/#input-output-locations
    // https://www.w3.org/TR/WGSL/#attribute-location
    // https://www.w3.org/TR/WGSL/#builtin-inputs-outputs
    int location = modifiers.fLayout.fLocation;
    if (location >= 0) {
        this->writeUserDefinedIODecl(type, name, location, delimiter);
    } else if (modifiers.fLayout.fBuiltin >= 0) {
        auto builtin = builtin_from_sksl_name(modifiers.fLayout.fBuiltin);
        if (builtin.has_value()) {
            this->writeBuiltinIODecl(type, name, *builtin, delimiter);
        }
    }
}

void WGSLCodeGenerator::writeUserDefinedIODecl(const Type& type,
                                               std::string_view name,
                                               int location,
                                               Delimiter delimiter) {
    this->write("@location(" + std::to_string(location) + ") ");

    // "User-defined IO of scalar or vector integer type must always be specified as
    // @interpolate(flat)" (see https://www.w3.org/TR/WGSL/#interpolation)
    if (type.isInteger() || (type.isVector() && type.componentType().isInteger())) {
        this->write("@interpolate(flat) ");
    }

    this->writeVariableDecl(type, name, delimiter);
}

void WGSLCodeGenerator::writeBuiltinIODecl(const Type& type,
                                           std::string_view name,
                                           Builtin builtin,
                                           Delimiter delimiter) {
    this->write("@builtin(");
    this->write(wgsl_builtin_name(builtin));
    this->write(") ");

    this->writeName(name);
    this->write(": ");
    this->write(wgsl_builtin_type(builtin));
    this->writeLine(delimiter_to_str(delimiter));
}

void WGSLCodeGenerator::writeFunction(const FunctionDefinition& f) {
    this->writeFunctionDeclaration(f.declaration());
    this->write(" ");
    this->writeBlock(f.body()->as<Block>());

    if (f.declaration().isMain()) {
        // We just emitted the user-defined main function. Next, we generate a program entry point
        // that calls the user-defined main.
        this->writeEntryPoint(f);
    }
}

void WGSLCodeGenerator::writeFunctionDeclaration(const FunctionDeclaration& f) {
    this->write("fn ");
    this->write(f.mangledName());
    this->write("(");
    auto separator = SkSL::String::Separator();
    if (this->writeFunctionDependencyParams(f)) {
        separator();  // update the separator as parameters have been written
    }
    for (const Variable* param : f.parameters()) {
        this->write(separator());
        this->writeName(param->mangledName());
        this->write(": ");

        // Declare an "out" function parameter as a pointer.
        if (param->modifiers().fFlags & Modifiers::kOut_Flag) {
            this->write(to_ptr_type(param->type()));
        } else {
            this->write(to_wgsl_type(param->type()));
        }
    }
    this->write(")");
    if (!f.returnType().isVoid()) {
        this->write(" -> ");
        this->write(to_wgsl_type(f.returnType()));
    }
}

void WGSLCodeGenerator::writeEntryPoint(const FunctionDefinition& main) {
    SkASSERT(main.declaration().isMain());

    // The input and output parameters for a vertex/fragment stage entry point function have the
    // FSIn/FSOut/VSIn/VSOut struct types that have been synthesized in generateCode(). An entry
    // point always has the same signature and acts as a trampoline to the user-defined main
    // function.
    std::string outputType;
    if (ProgramConfig::IsVertex(fProgram.fConfig->fKind)) {
        this->write("@vertex fn vertexMain(");
        if (fPipelineInputCount > 0) {
            this->write("_stageIn: VSIn");
        }
        this->writeLine(") -> VSOut {");
        outputType = "VSOut";
    } else if (ProgramConfig::IsFragment(fProgram.fConfig->fKind)) {
        this->write("@fragment fn fragmentMain(");
        if (fPipelineInputCount > 0) {
            this->write("_stageIn: FSIn");
        }
        this->writeLine(") -> FSOut {");
        outputType = "FSOut";
    } else {
        fContext.fErrors->error(Position(), "program kind not supported");
        return;
    }

    // Declare the stage output struct.
    fIndentation++;
    this->write("var _stageOut: ");
    this->write(outputType);
    this->writeLine(";");

    // Generate assignment to sk_FragColor built-in if the user-defined main returns a color.
    if (ProgramConfig::IsFragment(fProgram.fConfig->fKind)) {
        const SymbolTable* symbolTable = top_level_symbol_table(main);
        const Symbol* symbol = symbolTable->find("sk_FragColor");
        SkASSERT(symbol);
        if (main.declaration().returnType().matches(symbol->type())) {
            this->write("_stageOut.sk_FragColor = ");
        }
    }

    // Generate the function call to the user-defined main:
    this->write(main.declaration().mangledName());
    this->write("(");
    auto separator = SkSL::String::Separator();
    FunctionDependencies* deps = fRequirements.dependencies.find(&main.declaration());
    if (deps) {
        if ((*deps & FunctionDependencies::kPipelineInputs) != FunctionDependencies::kNone) {
            this->write(separator());
            this->write("_stageIn");
        }
        if ((*deps & FunctionDependencies::kPipelineOutputs) != FunctionDependencies::kNone) {
            this->write(separator());
            this->write("&_stageOut");
        }
    }
    // TODO(armansito): Handle arbitrary parameters.
    if (main.declaration().parameters().size() != 0) {
        const Variable* v = main.declaration().parameters()[0];
        const Type& type = v->type();
        if (v->modifiers().fLayout.fBuiltin == SK_MAIN_COORDS_BUILTIN) {
            if (!type.matches(*fContext.fTypes.fFloat2)) {
                fContext.fErrors->error(
                        main.fPosition,
                        "main function has unsupported parameter: " + type.description());
                return;
            }

            this->write(separator());
            this->write("_stageIn.sk_FragCoord.xy");
        }
    }
    this->writeLine(");");
    this->writeLine("return _stageOut;");

    fIndentation--;
    this->writeLine("}");
}

void WGSLCodeGenerator::writeStatement(const Statement& s) {
    switch (s.kind()) {
        case Statement::Kind::kBlock:
            this->writeBlock(s.as<Block>());
            break;
        case Statement::Kind::kExpression:
            this->writeExpressionStatement(s.as<ExpressionStatement>());
            break;
        case Statement::Kind::kIf:
            this->writeIfStatement(s.as<IfStatement>());
            break;
        case Statement::Kind::kReturn:
            this->writeReturnStatement(s.as<ReturnStatement>());
            break;
        case Statement::Kind::kVarDeclaration:
            this->writeVarDeclaration(s.as<VarDeclaration>());
            break;
        default:
            SkDEBUGFAILF("unsupported statement (kind: %d) %s",
                         static_cast<int>(s.kind()), s.description().c_str());
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

void WGSLCodeGenerator::writeExpressionStatement(const ExpressionStatement& s) {
    if (Analysis::HasSideEffects(*s.expression())) {
        this->writeExpression(*s.expression(), Precedence::kTopLevel);
        this->write(";");
    }
}

void WGSLCodeGenerator::writeIfStatement(const IfStatement& s) {
    this->write("if (");
    this->writeExpression(*s.test(), Precedence::kTopLevel);
    this->write(") ");
    this->writeStatement(*s.ifTrue());
    if (s.ifFalse()) {
        this->write("else ");
        this->writeStatement(*s.ifFalse());
    }
}

void WGSLCodeGenerator::writeReturnStatement(const ReturnStatement& s) {
    this->write("return");
    if (s.expression()) {
        this->write(" ");
        this->writeExpression(*s.expression(), Precedence::kTopLevel);
    }
    this->write(";");
}

void WGSLCodeGenerator::writeVarDeclaration(const VarDeclaration& varDecl) {
    bool isConst = varDecl.var()->modifiers().fFlags & Modifiers::kConst_Flag;
    if (isConst) {
        this->write("let ");
    } else {
        this->write("var ");
    }
    this->writeName(varDecl.var()->mangledName());
    this->write(": ");
    this->write(to_wgsl_type(varDecl.var()->type()));

    if (varDecl.value()) {
        this->write(" = ");
        this->writeExpression(*varDecl.value(), Precedence::kTopLevel);
    } else if (isConst) {
        SkDEBUGFAILF("A let-declared constant must specify a value");
    }

    this->write(";");
}

void WGSLCodeGenerator::writeExpression(const Expression& e, Precedence parentPrecedence) {
    switch (e.kind()) {
        case Expression::Kind::kBinary:
            this->writeBinaryExpression(e.as<BinaryExpression>(), parentPrecedence);
            break;
        case Expression::Kind::kConstructorCompound:
            this->writeConstructorCompound(e.as<ConstructorCompound>(), parentPrecedence);
            break;
        case Expression::Kind::kConstructorCompoundCast:
        case Expression::Kind::kConstructorScalarCast:
        case Expression::Kind::kConstructorSplat:
            this->writeAnyConstructor(e.asAnyConstructor(), parentPrecedence);
            break;
        case Expression::Kind::kConstructorDiagonalMatrix:
            this->writeConstructorDiagonalMatrix(e.as<ConstructorDiagonalMatrix>(),
                                                 parentPrecedence);
            break;
        case Expression::Kind::kConstructorMatrixResize:
            this->writeConstructorMatrixResize(e.as<ConstructorMatrixResize>(), parentPrecedence);
            break;
        case Expression::Kind::kFieldAccess:
            this->writeFieldAccess(e.as<FieldAccess>());
            break;
        case Expression::Kind::kFunctionCall:
            this->writeFunctionCall(e.as<FunctionCall>());
            break;
        case Expression::Kind::kIndex:
            this->writeIndexExpression(e.as<IndexExpression>());
            break;
        case Expression::Kind::kLiteral:
            this->writeLiteral(e.as<Literal>());
            break;
        case Expression::Kind::kSwizzle:
            this->writeSwizzle(e.as<Swizzle>());
            break;
        case Expression::Kind::kTernary:
            this->writeTernaryExpression(e.as<TernaryExpression>(), parentPrecedence);
            break;
        case Expression::Kind::kVariableReference:
            this->writeVariableReference(e.as<VariableReference>());
            break;
        default:
            SkDEBUGFAILF("unsupported expression (kind: %d) %s",
                         static_cast<int>(e.kind()),
                         e.description().c_str());
            break;
    }
}

void WGSLCodeGenerator::writeBinaryExpression(const BinaryExpression& b,
                                              Precedence parentPrecedence) {
    const Expression& left = *b.left();
    const Expression& right = *b.right();
    Operator op = b.getOperator();

    // The equality and comparison operators are only supported for scalar and vector types.
    if (op.isEquality() && !left.type().isScalar() && !left.type().isVector()) {
        if (left.type().isMatrix()) {
            if (op.kind() == OperatorKind::NEQ) {
                this->write("!");
            }
            this->writeMatrixEquality(left, right);
            return;
        }

        // TODO(skia:13092): Synthesize helper functions for structs and arrays.
        return;
    }

    Precedence precedence = op.getBinaryPrecedence();
    bool needParens = precedence >= parentPrecedence;

    // The equality operators ('=='/'!=') in WGSL apply component-wise to vectors and result in a
    // vector. We need to reduce the value to a boolean.
    if (left.type().isVector()) {
        if (op.kind() == Operator::Kind::EQEQ) {
            this->write("all");
            needParens = true;
        } else if (op.kind() == Operator::Kind::NEQ) {
            this->write("any");
            needParens = true;
        }
    }

    if (needParens) {
        this->write("(");
    }

    // TODO(skia:13092): Correctly handle the case when lhs is a pointer.

    this->writeExpression(left, precedence);
    this->write(op.operatorName());
    this->writeExpression(right, precedence);

    if (needParens) {
        this->write(")");
    }
}

void WGSLCodeGenerator::writeFieldAccess(const FieldAccess& f) {
    const Type::Field* field = &f.base()->type().fields()[f.fieldIndex()];
    if (FieldAccess::OwnerKind::kDefault == f.ownerKind()) {
        this->writeExpression(*f.base(), Precedence::kPostfix);
        this->write(".");
    } else {
        // We are accessing a field in an anonymous interface block. If the field refers to a
        // pipeline IO parameter, then we access it via the synthesized IO structs. We make an
        // explicit exception for `sk_PointSize` which we declare as a placeholder variable in
        // global scope as it is not supported by WebGPU as a pipeline IO parameter (see comments
        // in `writeStageOutputStruct`).
        const Variable& v = *f.base()->as<VariableReference>().variable();
        if (v.modifiers().fFlags & Modifiers::kIn_Flag) {
            this->write("_stageIn.");
        } else if (v.modifiers().fFlags & Modifiers::kOut_Flag &&
                   field->fModifiers.fLayout.fBuiltin != SK_POINTSIZE_BUILTIN) {
            this->write("(*_stageOut).");
        } else {
            // TODO(skia:13092): Reference the variable using the base name used for its
            // uniform/storage block global declaration.
        }
    }
    this->writeName(field->fName);
}

void WGSLCodeGenerator::writeFunctionCall(const FunctionCall& c) {
    const FunctionDeclaration& func = c.function();

    // TODO(skia:13092): Handle intrinsic call as many of them need to be rewritten.

    // We implement function out-parameters by declaring them as pointers. SkSL follows GLSL's
    // out-parameter semantics, in which out-parameters are only written back to the original
    // variable after the function's execution is complete (see
    // https://www.khronos.org/opengl/wiki/Core_Language_(GLSL)#Parameters).
    //
    // In addition, SkSL supports swizzles and array index expressions to be passed into
    // out-parameters however WGSL does not allow taking their address into a pointer.
    //
    // We support these by wrapping each function call in a special helper, which internally stores
    // all out parameters in temporaries.

    // First detect which arguments are passed to out-parameters.
    const ExpressionArray& args = c.arguments();
    const std::vector<Variable*>& params = func.parameters();
    SkASSERT(SkToSizeT(args.size()) == params.size());

    bool foundOutParam = false;
    SkSTArray<16, VariableReference*> outVars;
    outVars.push_back_n(args.size(), static_cast<VariableReference*>(nullptr));

    for (int i = 0; i < args.size(); ++i) {
        if (params[i]->modifiers().fFlags & Modifiers::kOut_Flag) {
            // Find the expression's inner variable being written to. Assignability was verified at
            // IR generation time, so this should always succeed.
            Analysis::AssignmentInfo info;
            SkAssertResult(Analysis::IsAssignable(*args[i], &info));
            outVars[i] = info.fAssignedVar;
            foundOutParam = true;
        }
    }

    if (foundOutParam) {
        this->writeName(this->writeOutParamHelper(c, args, outVars));
    } else {
        this->writeName(func.mangledName());
    }

    this->write("(");
    auto separator = SkSL::String::Separator();
    if (this->writeFunctionDependencyArgs(func)) {
        separator();
    }
    for (int i = 0; i < args.size(); ++i) {
        this->write(separator());
        if (outVars[i]) {
            // We need to take the address of the variable and pass it down as a pointer.
            this->write("&");
            this->writeExpression(*outVars[i], Precedence::kSequence);
        } else {
            this->writeExpression(*args[i], Precedence::kSequence);
        }
    }
    this->write(")");
}

void WGSLCodeGenerator::writeIndexExpression(const IndexExpression& i) {
    this->writeExpression(*i.base(), Precedence::kPostfix);
    this->write("[");
    this->writeExpression(*i.index(), Precedence::kTopLevel);
    this->write("]");
}

void WGSLCodeGenerator::writeLiteral(const Literal& l) {
    const Type& type = l.type();
    if (type.isFloat() || type.isBoolean()) {
        this->write(l.description(OperatorPrecedence::kTopLevel));
        return;
    }
    SkASSERT(type.isInteger());
    if (type.matches(*fContext.fTypes.fUInt)) {
        this->write(std::to_string(l.intValue() & 0xffffffff));
        this->write("u");
    } else if (type.matches(*fContext.fTypes.fUShort)) {
        this->write(std::to_string(l.intValue() & 0xffff));
        this->write("u");
    } else {
        this->write(std::to_string(l.intValue()));
    }
}

void WGSLCodeGenerator::writeSwizzle(const Swizzle& swizzle) {
    this->writeExpression(*swizzle.base(), Precedence::kPostfix);
    this->write(".");
    for (int c : swizzle.components()) {
        SkASSERT(c >= 0 && c <= 3);
        this->write(&("x\0y\0z\0w\0"[c * 2]));
    }
}

void WGSLCodeGenerator::writeTernaryExpression(const TernaryExpression& t,
                                               Precedence parentPrecedence) {
    bool needParens = Precedence::kTernary >= parentPrecedence;
    if (needParens) {
        this->write("(");
    }

    // The trivial case is when neither branch has side effects and evaluate to a scalar or vector
    // type. This can be represented with a call to the WGSL `select` intrinsic although it doesn't
    // support short-circuiting.
    if ((t.type().isScalar() || t.type().isVector()) && !Analysis::HasSideEffects(*t.ifTrue()) &&
        !Analysis::HasSideEffects(*t.ifFalse())) {
        this->write("select(");
        this->writeExpression(*t.ifFalse(), Precedence::kTernary);
        this->write(", ");
        this->writeExpression(*t.ifTrue(), Precedence::kTernary);
        this->write(", ");

        bool isVector = t.type().isVector();
        if (isVector) {
            // Splat the condition expression into a vector.
            this->write(String::printf("vec%d<bool>(", t.type().columns()));
        }
        this->writeExpression(*t.test(), Precedence::kTernary);
        if (isVector) {
            this->write(")");
        }
        this->write(")");
        if (needParens) {
            this->write(")");
        }
        return;
    }

    // TODO(skia:13092): WGSL does not support ternary expressions. To replicate the required
    // short-circuting behavior we need to hoist the expression out into the surrounding block,
    // convert it into an if statement that writes the result to a synthesized variable, and replace
    // the original expression with a reference to that variable.
    //
    // Once hoisting is supported, we may want to use that for vector type expressions as well,
    // since select above does a component-wise select
}

void WGSLCodeGenerator::writeVariableReference(const VariableReference& r) {
    // TODO(skia:13092): Correctly handle RTflip for built-ins.
    const Variable& v = *r.variable();

    // Insert a conversion expression if this is a built-in variable whose type differs from the
    // SkSL.
    std::optional<std::string_view> conversion = needs_builtin_type_conversion(v);
    if (conversion.has_value()) {
        this->write(*conversion);
        this->write("(");
    }

    bool needsDeref = false;
    bool isSynthesizedOutParamArg = fOutParamArgVars.contains(&v);

    // When a variable is referenced in the context of a synthesized out-parameter helper argument,
    // two special rules apply:
    //     1. If it's accessed via a pipeline I/O or global uniforms struct, it should instead
    //        be referenced by name (since it's actually referring to a function parameter).
    //     2. Its type should be treated as a pointer and should be dereferenced as such.
    if (v.storage() == Variable::Storage::kGlobal && !isSynthesizedOutParamArg) {
        if (v.modifiers().fFlags & Modifiers::kIn_Flag) {
            this->write("_stageIn.");
        } else if (v.modifiers().fFlags & Modifiers::kOut_Flag) {
            this->write("(*_stageOut).");
        } else if (is_in_global_uniforms(v)) {
            this->write("_globalUniforms.");
        }
    } else if ((v.storage() == Variable::Storage::kParameter &&
                v.modifiers().fFlags & Modifiers::kOut_Flag) ||
               isSynthesizedOutParamArg) {
        // This is an out-parameter and its type is a pointer, which we need to dereference.
        // We wrap the dereference in parentheses in case the value is used in an access expression
        // later.
        needsDeref = true;
        this->write("(*");
    }

    this->writeName(v.mangledName());
    if (needsDeref) {
        this->write(")");
    }
    if (conversion.has_value()) {
        this->write(")");
    }
}

void WGSLCodeGenerator::writeAnyConstructor(const AnyConstructor& c, Precedence parentPrecedence) {
    this->write(to_wgsl_type(c.type()));
    this->write("(");
    auto separator = SkSL::String::Separator();
    for (const auto& e : c.argumentSpan()) {
        this->write(separator());
        this->writeExpression(*e, Precedence::kSequence);
    }
    this->write(")");
}

void WGSLCodeGenerator::writeConstructorCompound(const ConstructorCompound& c,
                                                 Precedence parentPrecedence) {
    if (c.type().isVector()) {
        this->writeConstructorCompoundVector(c, parentPrecedence);
    } else if (c.type().isMatrix()) {
        this->writeConstructorCompoundMatrix(c, parentPrecedence);
    } else {
        fContext.fErrors->error(c.fPosition, "unsupported compound constructor");
    }
}

void WGSLCodeGenerator::writeConstructorCompoundVector(const ConstructorCompound& c,
                                                       Precedence parentPrecedence) {
    // WGSL supports constructing vectors from a mix of scalars and vectors but
    // not matrices (see https://www.w3.org/TR/WGSL/#type-constructor-expr).
    //
    // SkSL supports vec4(mat2x2) which we handle specially.
    if (c.type().columns() == 4 && c.argumentSpan().size() == 1) {
        const Expression& arg = *c.argumentSpan().front();
        if (arg.type().isMatrix()) {
            // This is the vec4(mat2x2) case.
            SkASSERT(arg.type().columns() == 2);
            SkASSERT(arg.type().rows() == 2);

            // Generate a helper so that the argument expression gets evaluated once.
            std::string name = String::printf("%s_from_%s",
                                              to_mangled_wgsl_type_name(c.type()).c_str(),
                                              to_mangled_wgsl_type_name(arg.type()).c_str());
            if (!fHelpers.contains(name)) {
                fHelpers.add(name);
                std::string returnType = to_wgsl_type(c.type());
                std::string argType = to_wgsl_type(arg.type());
                fExtraFunctions.printf(
                        "fn %s(x: %s) -> %s {\n    return %s(x[0].xy, x[1].xy);\n}\n",
                        name.c_str(),
                        argType.c_str(),
                        returnType.c_str(),
                        returnType.c_str());
            }
            this->write(name);
            this->write("(");
            this->writeExpression(arg, Precedence::kSequence);
            this->write(")");
            return;
        }
    }
    this->writeAnyConstructor(c, parentPrecedence);
}

void WGSLCodeGenerator::writeConstructorCompoundMatrix(const ConstructorCompound& c,
                                                       Precedence parentPrecedence) {
    SkASSERT(c.type().isMatrix());

    // Emit and invoke a matrix-constructor helper method if one is necessary.
    if (this->isMatrixConstructorHelperNeeded(c)) {
        this->write(this->getMatrixConstructorHelper(c));
        this->write("(");
        auto separator = String::Separator();
        for (const std::unique_ptr<Expression>& expr : c.arguments()) {
            this->write(separator());
            this->writeExpression(*expr, Precedence::kSequence);
        }
        this->write(")");
        return;
    }

    // WGSL doesn't allow creating matrices by passing in scalars and vectors in a jumble; it
    // requires your scalars to be grouped up into columns. As `isMatrixConstructorHelperNeeded`
    // returned false, we know that none of our scalars/vectors "wrap" across across a column, so we
    // can group our inputs up and synthesize a constructor for each column.
    const Type& matrixType = c.type();
    const Type& columnType = matrixType.componentType().toCompound(
            fContext, /*columns=*/matrixType.rows(), /*rows=*/1);

    this->write(to_wgsl_type(matrixType));
    this->write("(");
    auto separator = String::Separator();
    int scalarCount = 0;
    for (const std::unique_ptr<Expression>& arg : c.arguments()) {
        this->write(separator());
        if (arg->type().columns() < matrixType.rows()) {
            // Write a `floatN(` constructor to group scalars and smaller vectors together.
            if (!scalarCount) {
                this->write(to_wgsl_type(columnType));
                this->write("(");
            }
            scalarCount += arg->type().columns();
        }
        this->writeExpression(*arg, Precedence::kSequence);
        if (scalarCount && scalarCount == matrixType.rows()) {
            // Close our `floatN(...` constructor block from above.
            this->write(")");
            scalarCount = 0;
        }
    }
    this->write(")");
}

void WGSLCodeGenerator::writeConstructorDiagonalMatrix(const ConstructorDiagonalMatrix& c,
                                                       Precedence parentPrecedence) {
    const Type& type = c.type();
    SkASSERT(type.isMatrix());
    SkASSERT(c.argument()->type().isScalar());

    // Generate a helper so that the argument expression gets evaluated once.
    std::string name = String::printf("%s_diagonal", to_mangled_wgsl_type_name(type).c_str());
    if (!fHelpers.contains(name)) {
        fHelpers.add(name);

        std::string typeName = to_wgsl_type(type);
        fExtraFunctions.printf("fn %s(x: %s) -> %s {\n",
                               name.c_str(),
                               to_wgsl_type(c.argument()->type()).c_str(),
                               typeName.c_str());
        fExtraFunctions.printf("    return %s(", typeName.c_str());
        auto separator = String::Separator();
        for (int col = 0; col < type.columns(); ++col) {
            for (int row = 0; row < type.rows(); ++row) {
                fExtraFunctions.printf("%s%s", separator().c_str(), (col == row) ? "x" : "0.0");
            }
        }
        fExtraFunctions.printf(");\n}\n");
    }
    this->write(name);
    this->write("(");
    this->writeExpression(*c.argument(), Precedence::kSequence);
    this->write(")");
}

void WGSLCodeGenerator::writeConstructorMatrixResize(const ConstructorMatrixResize& c,
                                                     Precedence parentPrecedence) {
    this->write(this->getMatrixConstructorHelper(c));
    this->write("(");
    this->writeExpression(*c.argument(), Precedence::kSequence);
    this->write(")");
}

bool WGSLCodeGenerator::isMatrixConstructorHelperNeeded(const ConstructorCompound& c) {
    // WGSL supports 3 categories of matrix constructors:
    //     1. Identity construction from a matrix of identical dimensions (handled as
    //        ConstructorCompoundCast);
    //     2. Column-major construction by elements (scalars);
    //     3. Column-by-column construction from vectors.
    //
    // WGSL does not have a diagonal constructor. In addition, SkSL (like GLSL) supports free-form
    // inputs that combine vectors, matrices, and scalars.
    //
    // Some cases are simple to translate and so we handle those inline--e.g. a list of scalars can
    // be constructed trivially. In more complex cases, we generate a helper function that converts
    // our inputs into a properly-shaped matrix.
    //
    // A matrix constructor helper method is always used if any input argument is a matrix.
    // Helper methods are also necessary when any argument would span multiple rows. For instance:
    //
    // float2 x = (1, 2);
    // float3x2(x, 3, 4, 5, 6) = | 1 3 5 | = no helper needed; conversion can be done inline
    //                           | 2 4 6 |
    //
    // float2 x = (2, 3);
    // float3x2(1, x, 4, 5, 6) = | 1 3 5 | = x spans multiple rows; a helper method will be used
    //                           | 2 4 6 |
    //
    // float4 x = (1, 2, 3, 4);
    // float2x2(x) = | 1 3 | = x spans multiple rows; a helper method will be used
    //               | 2 4 |
    //
    int position = 0;
    for (const std::unique_ptr<Expression>& expr : c.arguments()) {
        if (expr->type().isMatrix()) {
            return true;
        }
        position += expr->type().columns();
        if (position > c.type().rows()) {
            // An input argument would span multiple rows; a helper function is required.
            return true;
        }
        if (position == c.type().rows()) {
            // We've advanced to the end of a row. Wrap to the start of the next row.
            position = 0;
        }
    }
    return false;
}

std::string WGSLCodeGenerator::getMatrixConstructorHelper(const AnyConstructor& c) {
    const Type& type = c.type();
    int columns = type.columns();
    int rows = type.rows();
    auto args = c.argumentSpan();
    std::string typeName = to_wgsl_type(type);

    // Create the helper-method name and use it as our lookup key.
    std::string name = String::printf("%s_from", to_mangled_wgsl_type_name(type).c_str());
    for (const std::unique_ptr<Expression>& expr : args) {
        String::appendf(&name, "_%s", to_mangled_wgsl_type_name(expr->type()).c_str());
    }

    // If a helper-method has not been synthesized yet, create it now.
    if (!fHelpers.contains(name)) {
        fHelpers.add(name);

        fExtraFunctions.printf("fn %s(", name.c_str());

        auto separator = String::Separator();
        for (size_t i = 0; i < args.size(); ++i) {
            fExtraFunctions.printf(
                    "%sx%zu: %s", separator().c_str(), i, to_wgsl_type(args[i]->type()).c_str());
        }

        fExtraFunctions.printf(") -> %s {\n    return %s(", typeName.c_str(), typeName.c_str());

        if (args.size() == 1 && args.front()->type().isMatrix()) {
            this->writeMatrixFromMatrixArgs(args.front()->type(), columns, rows);
        } else {
            this->writeMatrixFromScalarAndVectorArgs(c, columns, rows);
        }

        fExtraFunctions.writeText(");\n}\n");
    }
    return name;
}

// Assembles a matrix by resizing another matrix named `x0`.
// Cells that don't exist in the source matrix will be populated with identity-matrix values.
void WGSLCodeGenerator::writeMatrixFromMatrixArgs(const Type& sourceMatrix, int columns, int rows) {
    SkASSERT(rows <= 4);
    SkASSERT(columns <= 4);

    const char* separator = "";
    std::string matrixType = to_wgsl_type(sourceMatrix.componentType());
    for (int c = 0; c < columns; ++c) {
        fExtraFunctions.printf("%svec%d<%s>(", separator, rows, matrixType.c_str());
        separator = "), ";

        // Determine how many values to take from the source matrix for this row.
        int swizzleLength = 0;
        if (c < sourceMatrix.columns()) {
            swizzleLength = std::min<>(rows, sourceMatrix.rows());
        }

        // Emit all the values from the source matrix row.
        bool firstItem;
        switch (swizzleLength) {
            case 0:
                firstItem = true;
                break;
            case 1:
                firstItem = false;
                fExtraFunctions.printf("x0[%d].x", c);
                break;
            case 2:
                firstItem = false;
                fExtraFunctions.printf("x0[%d].xy", c);
                break;
            case 3:
                firstItem = false;
                fExtraFunctions.printf("x0[%d].xyz", c);
                break;
            case 4:
                firstItem = false;
                fExtraFunctions.printf("x0[%d].xyzw", c);
                break;
            default:
                SkUNREACHABLE;
        }

        // Emit the placeholder identity-matrix cells.
        for (int r = swizzleLength; r < rows; ++r) {
            fExtraFunctions.printf("%s%s", firstItem ? "" : ", ", (r == c) ? "1.0" : "0.0");
            firstItem = false;
        }
    }

    fExtraFunctions.writeText(")");
}

// Assembles a matrix of type by concatenating an arbitrary mix of scalar and vector values, named
// `x0`, `x1`, etc. An error is written if the expression list don't contain exactly C*R scalars.
void WGSLCodeGenerator::writeMatrixFromScalarAndVectorArgs(const AnyConstructor& ctor,
                                                           int columns,
                                                           int rows) {
    SkASSERT(rows <= 4);
    SkASSERT(columns <= 4);

    std::string matrixType = to_wgsl_type(ctor.type().componentType());
    size_t argIndex = 0;
    int argPosition = 0;
    auto args = ctor.argumentSpan();

    static constexpr char kSwizzle[] = "xyzw";
    const char* separator = "";
    for (int c = 0; c < columns; ++c) {
        fExtraFunctions.printf("%svec%d<%s>(", separator, rows, matrixType.c_str());
        separator = "), ";

        auto columnSeparator = String::Separator();
        for (int r = 0; r < rows;) {
            fExtraFunctions.writeText(columnSeparator().c_str());
            if (argIndex < args.size()) {
                const Type& argType = args[argIndex]->type();
                switch (argType.typeKind()) {
                    case Type::TypeKind::kScalar: {
                        fExtraFunctions.printf("x%zu", argIndex);
                        ++r;
                        ++argPosition;
                        break;
                    }
                    case Type::TypeKind::kVector: {
                        fExtraFunctions.printf("x%zu.", argIndex);
                        do {
                            fExtraFunctions.write8(kSwizzle[argPosition]);
                            ++r;
                            ++argPosition;
                        } while (r < rows && argPosition < argType.columns());
                        break;
                    }
                    case Type::TypeKind::kMatrix: {
                        fExtraFunctions.printf("x%zu[%d].", argIndex, argPosition / argType.rows());
                        do {
                            fExtraFunctions.write8(kSwizzle[argPosition]);
                            ++r;
                            ++argPosition;
                        } while (r < rows && (argPosition % argType.rows()) != 0);
                        break;
                    }
                    default: {
                        SkDEBUGFAIL("incorrect type of argument for matrix constructor");
                        fExtraFunctions.writeText("<error>");
                        break;
                    }
                }

                if (argPosition >= argType.columns() * argType.rows()) {
                    ++argIndex;
                    argPosition = 0;
                }
            } else {
                SkDEBUGFAIL("not enough arguments for matrix constructor");
                fExtraFunctions.writeText("<error>");
            }
        }
    }

    if (argPosition != 0 || argIndex != args.size()) {
        SkDEBUGFAIL("incorrect number of arguments for matrix constructor");
        fExtraFunctions.writeText(", <error>");
    }

    fExtraFunctions.writeText(")");
}

void WGSLCodeGenerator::writeMatrixEquality(const Expression& left, const Expression& right) {
    const Type& leftType = left.type();
    const Type& rightType = right.type();
    SkASSERT(leftType.isMatrix());
    SkASSERT(rightType.isMatrix());
    SkASSERT(leftType.rows() == rightType.rows());
    SkASSERT(leftType.columns() == rightType.columns());

    std::string name = String::printf("%s_eq_%s",
                                      to_mangled_wgsl_type_name(leftType).c_str(),
                                      to_mangled_wgsl_type_name(rightType).c_str());
    if (!fHelpers.contains(name)) {
        fHelpers.add(name);
        fExtraFunctions.printf("fn %s(left: %s, right: %s) -> bool {\n    return ",
                               name.c_str(),
                               to_wgsl_type(leftType).c_str(),
                               to_wgsl_type(rightType).c_str());
        const char* separator = "";
        for (int i = 0; i < leftType.columns(); ++i) {
            fExtraFunctions.printf("%sall(left[%d] == right[%d])", separator, i, i);
            separator = " &&\n           ";
        }
        fExtraFunctions.printf(";\n}\n");
    }
    this->write(name);
    this->write("(");
    this->writeExpression(left, Precedence::kSequence);
    this->write(", ");
    this->writeExpression(right, Precedence::kSequence);
    this->write(")");
}

void WGSLCodeGenerator::writeProgramElement(const ProgramElement& e) {
    switch (e.kind()) {
        case ProgramElement::Kind::kExtension:
            // TODO(skia:13092): WGSL supports extensions via the "enable" directive
            // (https://www.w3.org/TR/WGSL/#language-extensions). While we could easily emit this
            // directive, we should first ensure that all possible SkSL extension names are
            // converted to their appropriate WGSL extension. Currently there are no known supported
            // WGSL extensions aside from the hypotheticals listed in the spec.
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
        default:
            SkDEBUGFAILF("unsupported program element: %s\n", e.description().c_str());
            break;
    }
}

void WGSLCodeGenerator::writeGlobalVarDeclaration(const GlobalVarDeclaration& d) {
    const Variable& var = *d.declaration()->as<VarDeclaration>().var();
    if ((var.modifiers().fFlags & (Modifiers::kIn_Flag | Modifiers::kOut_Flag)) ||
        is_in_global_uniforms(var)) {
        // Pipeline stage I/O parameters and top-level (non-block) uniforms are handled specially
        // in generateCode().
        return;
    }

    // TODO(skia:13092): Implement workgroup variable decoration
    this->write("var<private> ");
    this->writeVariableDecl(var.type(), var.name(), Delimiter::kSemicolon);
}

void WGSLCodeGenerator::writeStructDefinition(const StructDefinition& s) {
    const Type& type = s.type();
    this->writeLine("struct " + type.displayName() + " {");
    fIndentation++;
    this->writeFields(SkSpan(type.fields()), type.fPosition);
    fIndentation--;
    this->writeLine("};");
}

void WGSLCodeGenerator::writeFields(SkSpan<const Type::Field> fields,
                                    Position parentPos,
                                    const MemoryLayout*) {
    // TODO(skia:13092): Check alignment against `layout` constraints, if present. A layout
    // constraint will be specified for interface blocks and for structs that appear in a block.
    for (const Type::Field& field : fields) {
        const Type* fieldType = field.fType;
        this->writeVariableDecl(*fieldType, field.fName, Delimiter::kComma);
    }
}

void WGSLCodeGenerator::writeStageInputStruct() {
    std::string_view structNamePrefix = pipeline_struct_prefix(fProgram.fConfig->fKind);
    if (structNamePrefix.empty()) {
        // There's no need to declare pipeline stage outputs.
        return;
    }

    // It is illegal to declare a struct with no members.
    if (fPipelineInputCount < 1) {
        return;
    }

    this->write("struct ");
    this->write(structNamePrefix);
    this->writeLine("In {");
    fIndentation++;

    bool declaredFragCoordsBuiltin = false;
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const Variable* v = e->as<GlobalVarDeclaration>().declaration()
                                 ->as<VarDeclaration>().var();
            if (v->modifiers().fFlags & Modifiers::kIn_Flag) {
                this->writePipelineIODeclaration(v->modifiers(), v->type(), v->mangledName(),
                                                 Delimiter::kComma);
                if (v->modifiers().fLayout.fBuiltin == SK_FRAGCOORD_BUILTIN) {
                    declaredFragCoordsBuiltin = true;
                }
            }
        } else if (e->is<InterfaceBlock>()) {
            const Variable* v = e->as<InterfaceBlock>().var();
            // Merge all the members of `in` interface blocks to the input struct, which are
            // specified as either "builtin" or with a "layout(location=".
            //
            // TODO(armansito): Is it legal to have an interface block without a storage qualifier
            // but with members that have individual storage qualifiers?
            if (v->modifiers().fFlags & Modifiers::kIn_Flag) {
                for (const auto& f : v->type().fields()) {
                    this->writePipelineIODeclaration(f.fModifiers, *f.fType, f.fName,
                                                     Delimiter::kComma);
                    if (f.fModifiers.fLayout.fBuiltin == SK_FRAGCOORD_BUILTIN) {
                        declaredFragCoordsBuiltin = true;
                    }
                }
            }
        }
    }

    if (ProgramConfig::IsFragment(fProgram.fConfig->fKind) &&
        fRequirements.mainNeedsCoordsArgument && !declaredFragCoordsBuiltin) {
        this->writeLine("@builtin(position) sk_FragCoord: vec4<f32>,");
    }

    fIndentation--;
    this->writeLine("};");
}

void WGSLCodeGenerator::writeStageOutputStruct() {
    std::string_view structNamePrefix = pipeline_struct_prefix(fProgram.fConfig->fKind);
    if (structNamePrefix.empty()) {
        // There's no need to declare pipeline stage outputs.
        return;
    }

    this->write("struct ");
    this->write(structNamePrefix);
    this->writeLine("Out {");
    fIndentation++;

    // TODO(skia:13092): Remember all variables that are added to the output struct here so they
    // can be referenced correctly when handling variable references.
    bool declaredPositionBuiltin = false;
    bool requiresPointSizeBuiltin = false;
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const Variable* v = e->as<GlobalVarDeclaration>().declaration()
                                 ->as<VarDeclaration>().var();
            if (v->modifiers().fFlags & Modifiers::kOut_Flag) {
                this->writePipelineIODeclaration(v->modifiers(), v->type(), v->mangledName(),
                                                 Delimiter::kComma);
            }
        } else if (e->is<InterfaceBlock>()) {
            const Variable* v = e->as<InterfaceBlock>().var();
            // Merge all the members of `out` interface blocks to the output struct, which are
            // specified as either "builtin" or with a "layout(location=".
            //
            // TODO(armansito): Is it legal to have an interface block without a storage qualifier
            // but with members that have individual storage qualifiers?
            if (v->modifiers().fFlags & Modifiers::kOut_Flag) {
                for (const auto& f : v->type().fields()) {
                    this->writePipelineIODeclaration(f.fModifiers, *f.fType, f.fName,
                                                     Delimiter::kComma);
                    if (f.fModifiers.fLayout.fBuiltin == SK_POSITION_BUILTIN) {
                        declaredPositionBuiltin = true;
                    } else if (f.fModifiers.fLayout.fBuiltin == SK_POINTSIZE_BUILTIN) {
                        // sk_PointSize is explicitly not supported by `builtin_from_sksl_name` so
                        // writePipelineIODeclaration will never write it. We mark it here if the
                        // declaration is needed so we can synthesize it below.
                        requiresPointSizeBuiltin = true;
                    }
                }
            }
        }
    }

    // A vertex program must include the `position` builtin in its entry point return type.
    if (ProgramConfig::IsVertex(fProgram.fConfig->fKind) && !declaredPositionBuiltin) {
        this->writeLine("@builtin(position) sk_Position: vec4<f32>,");
    }

    fIndentation--;
    this->writeLine("};");

    // In WebGPU/WGSL, the vertex stage does not support a point-size output and the size
    // of a point primitive is always 1 pixel (see https://github.com/gpuweb/gpuweb/issues/332).
    //
    // There isn't anything we can do to emulate this correctly at this stage so we
    // synthesize a placeholder variable that has no effect. Programs should not rely on
    // sk_PointSize when using the Dawn backend.
    if (ProgramConfig::IsVertex(fProgram.fConfig->fKind) && requiresPointSizeBuiltin) {
        this->writeLine("/* unsupported */ var<private> sk_PointSize: f32;");
    }
}

void WGSLCodeGenerator::writeNonBlockUniformsForTests() {
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const GlobalVarDeclaration& decls = e->as<GlobalVarDeclaration>();
            const Variable& var = *decls.varDeclaration().var();
            if (is_in_global_uniforms(var)) {
                if (!fDeclaredUniformsStruct) {
                    this->write("struct _GlobalUniforms {\n");
                    fDeclaredUniformsStruct = true;
                }
                this->write("    ");
                this->writeVariableDecl(var.type(), var.mangledName(), Delimiter::kComma);
            }
        }
    }
    if (fDeclaredUniformsStruct) {
        int binding = fProgram.fConfig->fSettings.fDefaultUniformBinding;
        int set = fProgram.fConfig->fSettings.fDefaultUniformSet;
        this->write("};\n");
        this->write("@binding(" + std::to_string(binding) + ") ");
        this->write("@group(" + std::to_string(set) + ") ");
        this->writeLine("var<uniform> _globalUniforms: _GlobalUniforms;");
    }
}

bool WGSLCodeGenerator::writeFunctionDependencyArgs(const FunctionDeclaration& f) {
    FunctionDependencies* deps = fRequirements.dependencies.find(&f);
    if (!deps || *deps == FunctionDependencies::kNone) {
        return false;
    }

    const char* separator = "";
    if ((*deps & FunctionDependencies::kPipelineInputs) != FunctionDependencies::kNone) {
        this->write("_stageIn");
        separator = ", ";
    }
    if ((*deps & FunctionDependencies::kPipelineOutputs) != FunctionDependencies::kNone) {
        this->write(separator);
        this->write("_stageOut");
    }
    return true;
}

bool WGSLCodeGenerator::writeFunctionDependencyParams(const FunctionDeclaration& f) {
    FunctionDependencies* deps = fRequirements.dependencies.find(&f);
    if (!deps || *deps == FunctionDependencies::kNone) {
        return false;
    }

    std::string_view structNamePrefix = pipeline_struct_prefix(fProgram.fConfig->fKind);
    if (structNamePrefix.empty()) {
        return false;
    }
    const char* separator = "";
    if ((*deps & FunctionDependencies::kPipelineInputs) != FunctionDependencies::kNone) {
        this->write("_stageIn: ");
        separator = ", ";
        this->write(structNamePrefix);
        this->write("In");
    }
    if ((*deps & FunctionDependencies::kPipelineOutputs) != FunctionDependencies::kNone) {
        this->write(separator);
        this->write("_stageOut: ptr<function, ");
        this->write(structNamePrefix);
        this->write("Out>");
    }
    return true;
}

std::string WGSLCodeGenerator::writeOutParamHelper(const FunctionCall& c,
                                                   const ExpressionArray& args,
                                                   const SkTArray<VariableReference*>& outVars) {
    // It's possible for out-param function arguments to contain an out-param function call
    // expression. Emit the function into a temporary stream to prevent the nested helper from
    // clobbering the current helper as we recursively evaluate argument expressions.
    StringStream tmpStream;
    AutoOutputStream outputToExtraFunctions(this, &tmpStream, &fIndentation);

    // Reset the line start state while the AutoOutputStream is active. We restore it later before
    // the function returns.
    bool atLineStart = fAtLineStart;
    fAtLineStart = false;
    const FunctionDeclaration& func = c.function();

    // Synthesize a helper function that takes the same inputs as `function`, except in places where
    // `outVars` is non-null; in those places, we take the type of the VariableReference.
    //
    // float _outParamHelper_0_originalFuncName(float _var0, float _var1, float& outParam) {
    std::string name =
            "_outParamHelper_" + std::to_string(fSwizzleHelperCount++) + "_" + func.mangledName();
    auto separator = SkSL::String::Separator();
    this->write("fn ");
    this->write(name);
    this->write("(");
    if (this->writeFunctionDependencyParams(func)) {
        separator();
    }

    SkASSERT(outVars.size() == args.size());
    SkASSERT(SkToSizeT(outVars.size()) == func.parameters().size());

    // We need to detect cases where the caller passes the same variable as an out-param more than
    // once and avoid redeclaring the variable name. This is also a situation that is not permitted
    // by WGSL aliasing rules (see https://www.w3.org/TR/WGSL/#aliasing). Because the parameter is
    // redundant and we don't actually ever reference it, we give it a placeholder name.
    auto parentOutParamArgVars = std::move(fOutParamArgVars);
    SkASSERT(fOutParamArgVars.empty());

    for (int i = 0; i < args.size(); ++i) {
        this->write(separator());

        if (outVars[i]) {
            const Variable* var = outVars[i]->variable();
            if (!fOutParamArgVars.contains(var)) {
                fOutParamArgVars.add(var);
                this->writeName(var->mangledName());
            } else {
                this->write("_unused");
                this->write(std::to_string(i));
            }
        } else {
            this->write("_var");
            this->write(std::to_string(i));
        }

        this->write(": ");

        // Declare the parameter using the type of argument variable. If the complete argument is an
        // access or swizzle expression, the target assignment will be resolved below when we copy
        // the value to the out-parameter.
        const Type& type = outVars[i] ? outVars[i]->type() : args[i]->type();

        // Declare an out-parameter as a pointer.
        if (func.parameters()[i]->modifiers().fFlags & Modifiers::kOut_Flag) {
            this->write(to_ptr_type(type));
        } else {
            this->write(to_wgsl_type(type));
        }
    }

    this->write(")");
    if (!func.returnType().isVoid()) {
        this->write(" -> ");
        this->write(to_wgsl_type(func.returnType()));
    }
    this->writeLine(" {");
    ++fIndentation;

    // Declare a temporary variable for each out-parameter.
    for (int i = 0; i < outVars.size(); ++i) {
        if (!outVars[i]) {
            continue;
        }
        this->write("var ");
        this->write("_var");
        this->write(std::to_string(i));
        this->write(": ");
        this->write(to_wgsl_type(args[i]->type()));

        // If this is an inout parameter then we need to copy the input argument into the parameter
        // per https://www.khronos.org/opengl/wiki/Core_Language_(GLSL)#Parameters.
        if (func.parameters()[i]->modifiers().fFlags & Modifiers::kIn_Flag) {
            this->write(" = ");
            this->writeExpression(*args[i], Precedence::kAssignment);
        }

        this->writeLine(";");
    }

    // Call the function we're wrapping. If it has a return type, then store it so it can be
    // returned later.
    bool hasReturn = !c.type().isVoid();
    if (hasReturn) {
        this->write("var _return: ");
        this->write(to_wgsl_type(c.type()));
        this->write(" = ");
    }

    // Write the function call.
    this->writeName(func.mangledName());
    this->write("(");
    auto newSeparator = SkSL::String::Separator();
    if (this->writeFunctionDependencyArgs(func)) {
        newSeparator();
    }
    for (int i = 0; i < args.size(); ++i) {
        this->write(newSeparator());
        // All forwarded arguments now have a name that looks like "_var[i]" (e.g. _var0, var1,
        // etc.). All such variables should be of value type and those that have been passed in as
        // inout should have been dereferenced when they were stored in a local temporary. We need
        // to take their address again when forwarding to a pointer.
        if (outVars[i]) {
            this->write("&");
        }
        this->write("_var");
        this->write(std::to_string(i));
    }
    this->writeLine(");");

    // Copy the temporary variables back into the original out-parameters.
    for (int i = 0; i < outVars.size(); ++i) {
        if (!outVars[i]) {
            continue;
        }
        // TODO(skia:13092): WGSL does not support assigning to a swizzle
        // (see https://github.com/gpuweb/gpuweb/issues/737). These will require special treatment
        // when they appear on the lhs of an assignment.
        this->writeExpression(*args[i], Precedence::kAssignment);
        this->write(" = _var");
        this->write(std::to_string(i));
        this->writeLine(";");
    }

    // Return
    if (hasReturn) {
        this->writeLine("return _return;");
    }

    --fIndentation;
    this->writeLine("}");

    // Write the function out to `fExtraFunctions`.
    write_stringstream(tmpStream, fExtraFunctions);

    // Restore any global state
    fOutParamArgVars = std::move(parentOutParamArgVars);
    fAtLineStart = atLineStart;
    return name;
}

}  // namespace SkSL
