/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/codegen/SkSLWGSLCodeGenerator.h"

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/SkBitmaskEnum.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLIntrinsicList.h"
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
#include "src/sksl/ir/SkSLModifiers.h"
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
#include "src/sksl/ir/SkSLSymbol.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <optional>
#include <string>

using namespace skia_private;

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
            // variable (see `SkSL::Program::Interface::fUseFlipRTUniform`).
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

bool is_in_global_uniforms(const Variable& var) {
    SkASSERT(var.storage() == VariableStorage::kGlobal);
    return var.modifiers().fFlags & Modifiers::kUniform_Flag && !var.type().isOpaque();
}

}  // namespace

class WGSLCodeGenerator::LValue {
public:
    virtual ~LValue() = default;

    // Returns a pointer to the lvalue, if possible. If the lvalue cannot be directly referenced
    // by a pointer (e.g. vector swizzles), returns "".
    virtual std::string getPointer() {
        return "";
    }

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

    std::string getPointer() override {
        return std::string("&(") + fName + std::string(")");
    }

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

    std::string getPointer() override {
        return "";
    }

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
    SwizzleLValue(std::string name, const Type& t, const ComponentArray& c)
            : fName(std::move(name))
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

    std::string getPointer() override {
        return "";
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
            result += to_wgsl_type(fType);
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
    std::string fName;
    const Type& fType;
    ComponentArray fComponents;
    ComponentArray fUntouchedComponents;
    ComponentArray fReintegrationSwizzle;
};

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
    // Add underscore before name to avoid conflict with reserved words.
    return fReservedWords.contains(name) ? std::string("_") + std::string(name)
                                         : std::string(name);
}

void WGSLCodeGenerator::writeVariableDecl(const Type& type,
                                          std::string_view name,
                                          Delimiter delimiter) {
    this->write(this->assembleName(name));
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

    this->writeFunctionDeclaration(decl);
    this->writeLine(" {");
    ++fIndentation;

    // The parameters were given generic names like `_skParam1`, because WGSL parameters don't have
    // storage and are immutable. If mutability is required, we create variables here; otherwise, we
    // create properly-named `let` aliases.
    for (size_t index = 0; index < decl.parameters().size(); ++index) {
        const Variable& param = *decl.parameters()[index];
        if (!param.name().empty()) {
            const ProgramUsage::VariableCounts counts = fProgram.fUsage->get(param);
            // Variables which are never written-to don't need dedicated storage and can use `let`.
            // Out-parameters are passed as pointers; the pointer itself is never modified, so it
            // doesn't need a dedicated variable and can use `let`.
            this->write(((param.modifiers().fFlags & Modifiers::kOut_Flag) || counts.fWrite == 0)
                                ? "let "
                                : "var ");
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
        this->write(to_wgsl_type(f.declaration().returnType()));
        this->writeLine("();");
    }

    --fIndentation;
    this->writeLine("}");

    if (f.declaration().isMain()) {
        // We just emitted the user-defined main function. Next, we generate a program entry point
        // that calls the user-defined main.
        this->writeEntryPoint(f);
    }

    SkASSERT(fAtFunctionScope);
    fAtFunctionScope = false;
}

void WGSLCodeGenerator::writeFunctionDeclaration(const FunctionDeclaration& decl) {
    this->write("fn ");
    this->write(decl.mangledName());
    this->write("(");
    auto separator = SkSL::String::Separator();
    if (this->writeFunctionDependencyParams(decl)) {
        separator();  // update the separator as parameters have been written
    }
    for (size_t index = 0; index < decl.parameters().size(); ++index) {
        const Variable& param = *decl.parameters()[index];
        this->write(separator());
        this->write("_skParam" + std::to_string(index));
        this->write(": ");

        // Declare an "out" function parameter as a pointer.
        if (param.modifiers().fFlags & Modifiers::kOut_Flag) {
            this->write(to_ptr_type(param.type()));
        } else {
            this->write(to_wgsl_type(param.type()));
        }
    }
    this->write(")");
    if (!decl.returnType().isVoid()) {
        this->write(" -> ");
        this->write(to_wgsl_type(decl.returnType()));
    }
}

void WGSLCodeGenerator::writeEntryPoint(const FunctionDefinition& main) {
    SkASSERT(main.declaration().isMain());

#if defined(SKSL_STANDALONE)
    if (ProgramConfig::IsRuntimeShader(fProgram.fConfig->fKind)) {
        // Synthesize a basic entrypoint which just calls straight through to main.
        // This is only used by skslc and just needs to pass the WGSL validator; Skia won't ever
        // emit functions like this.
        this->writeLine("@fragment fn runtimeShaderMain(@location(0) _coords: vec2<f32>) -> "
                                     "@location(0) vec4<f32> {");
        ++fIndentation;
        this->writeLine("return main(_coords);");
        --fIndentation;
        this->writeLine("}");
        return;
    }
#endif

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
    this->write("if (");
    this->write(testExpr);
    this->writeLine(") {");
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

    if (varDecl.var()->modifiers().fFlags & Modifiers::kConst_Flag) {
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
    this->write(to_wgsl_type(varDecl.var()->type()));

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
            return this->assembleConstructorCompound(e.as<ConstructorCompound>(), parentPrecedence);

        case Expression::Kind::kConstructorArray:
        case Expression::Kind::kConstructorCompoundCast:
        case Expression::Kind::kConstructorScalarCast:
        case Expression::Kind::kConstructorSplat:
        case Expression::Kind::kConstructorStruct:
            return this->assembleAnyConstructor(e.asAnyConstructor(), parentPrecedence);

        case Expression::Kind::kConstructorDiagonalMatrix:
            return this->assembleConstructorDiagonalMatrix(e.as<ConstructorDiagonalMatrix>(),
                                                           parentPrecedence);

        case Expression::Kind::kConstructorMatrixResize:
            return this->assembleConstructorMatrixResize(e.as<ConstructorMatrixResize>(),
                                                         parentPrecedence);

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
            SkDEBUGFAILF("unsupported expression (kind: %d) %s",
                         static_cast<int>(e.kind()),
                         e.description().c_str());
            return {};
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

        std::string result;
        if (op.kind() == OperatorKind::EQ) {
            // Evaluate the right-hand side of simple assignment (`a = b` --> `b`).
            result = this->assembleExpression(right, Precedence::kAssignment);
        } else {
            // Evaluate the right-hand side of compound-assignment (`a += b` --> `a + b`).
            result = this->assembleBinaryExpression(left, op.removeAssignment(), right, resultType,
                                                    Precedence::kAssignment);
        }

        // Emit the assignment statement (`a = a + b`).
        this->writeLine(lvalue->store(result));

        // Return the lvalue (`a`) as the result, since the value might be used by the caller.
        return lvalue->load();
    }

    if (op.isEquality()) {
        return this->assembleEqualityExpression(left, right, op, parentPrecedence);
    }

    Precedence precedence = op.getBinaryPrecedence();
    bool needParens = precedence >= parentPrecedence;

    if (needParens) {
        expr.push_back('(');
    }

    expr += this->assembleExpression(left, precedence);
    expr += op.operatorName();
    expr += this->assembleExpression(right, precedence);

    if (needParens) {
        expr.push_back(')');
    }

    return expr;
}

std::string WGSLCodeGenerator::assembleFieldAccess(const FieldAccess& f) {
    std::string expr;

    const Field* field = &f.base()->type().fields()[f.fieldIndex()];
    if (FieldAccess::OwnerKind::kDefault == f.ownerKind()) {
        expr += this->assembleExpression(*f.base(), Precedence::kPostfix);
        expr.push_back('.');
    } else {
        // We are accessing a field in an anonymous interface block. If the field refers to a
        // pipeline IO parameter, then we access it via the synthesized IO structs. We make an
        // explicit exception for `sk_PointSize` which we declare as a placeholder variable in
        // global scope as it is not supported by WebGPU as a pipeline IO parameter (see comments
        // in `writeStageOutputStruct`).
        const Variable& v = *f.base()->as<VariableReference>().variable();
        if (v.modifiers().fFlags & Modifiers::kIn_Flag) {
            expr += "_stageIn.";
        } else if (v.modifiers().fFlags & Modifiers::kOut_Flag &&
                   field->fModifiers.fLayout.fBuiltin != SK_POINTSIZE_BUILTIN) {
            expr += "(*_stageOut).";
        } else {
            // TODO(skia:13092): Reference the variable using the base name used for its
            // uniform/storage block global declaration.
        }
    }
    expr += field->fName;
    return expr;
}

std::string WGSLCodeGenerator::assembleSimpleIntrinsic(std::string_view intrinsicName,
                                                       const FunctionCall& call) {
    SkASSERT(!call.type().isVoid());

    // Invoke the function, passing each function argument.
    std::string expr = std::string(intrinsicName);
    expr.push_back('(');
    const ExpressionArray& args = call.arguments();
    auto separator = SkSL::String::Separator();
    for (int index = 0; index < args.size(); ++index) {
        expr += separator();
        expr += this->assembleExpression(*args[index], Precedence::kSequence);
    }
    expr.push_back(')');

    return this->writeScratchLet(expr);
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
    for (int index = 0; index < args.size(); ++index) {
        expr += separator();

        bool vectorize = returnsVector && args[index]->type().isScalar();
        if (vectorize) {
            expr += to_wgsl_type(call.type());
            expr.push_back('(');
        }

        expr += this->assembleExpression(*args[index], Precedence::kSequence);

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

    expr += op.operatorName();
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
    bool needParens = precedence >= parentPrecedence;

    std::string expr;
    if (needParens) {
        expr.push_back('(');
    }

    expr += this->assembleExpression(*call.arguments()[0], precedence);
    expr += op.operatorName();
    expr += this->assembleExpression(*call.arguments()[1], precedence);

    if (needParens) {
        expr.push_back(')');
    }

    return expr;
}

std::string WGSLCodeGenerator::assembleIntrinsicCall(const FunctionCall& call,
                                                     IntrinsicKind kind,
                                                     Precedence parentPrecedence) {
    const ExpressionArray& arguments = call.arguments();
    switch (kind) {
        case k_atan_IntrinsicKind: {
            const char* name = (arguments.size() == 1) ? "atan" : "atan2";
            return this->assembleSimpleIntrinsic(name, call);
        }

        case k_dot_IntrinsicKind: {
            if (arguments[0]->type().columns() == 1) {
                return this->assembleBinaryOpIntrinsic(OperatorKind::STAR, call, parentPrecedence);
            }
            return this->assembleSimpleIntrinsic("dot", call);
        }
        case k_equal_IntrinsicKind:
            return this->assembleBinaryOpIntrinsic(OperatorKind::EQEQ, call, parentPrecedence);

        case k_faceforward_IntrinsicKind: {
            if (arguments[0]->type().columns() == 1) {
                // (select(-1.0, 1.0, (I * Nref) < 0) * N)
                return this->writeScratchLet(
                        "(select(-1.0, 1.0, (" +
                        this->assembleExpression(*arguments[1], Precedence::kMultiplicative) +
                        " * " +
                        this->assembleExpression(*arguments[2], Precedence::kMultiplicative) +
                        ") < 0) * " +
                        this->assembleExpression(*arguments[0], Precedence::kMultiplicative) + ")");
            }
            return this->assembleSimpleIntrinsic("faceForward", call);
        }
        case k_greaterThan_IntrinsicKind:
            return this->assembleBinaryOpIntrinsic(OperatorKind::GT, call, parentPrecedence);

        case k_greaterThanEqual_IntrinsicKind:
            return this->assembleBinaryOpIntrinsic(OperatorKind::GTEQ, call, parentPrecedence);

        case k_inversesqrt_IntrinsicKind:
            return this->assembleSimpleIntrinsic("inverseSqrt", call);

        case k_lessThan_IntrinsicKind:
            return this->assembleBinaryOpIntrinsic(OperatorKind::LT, call, parentPrecedence);

        case k_lessThanEqual_IntrinsicKind:
            return this->assembleBinaryOpIntrinsic(OperatorKind::LTEQ, call, parentPrecedence);

        case k_mix_IntrinsicKind: {
            const char* name = arguments[2]->type().componentType().isBoolean() ? "select" : "mix";
            return this->assembleVectorizedIntrinsic(name, call);
        }
        case k_mod_IntrinsicKind: {
            // WGSL has no intrinsic equivalent to `mod`. Synthesize `x - y * floor(x / y)`.
            std::string arg0 = this->writeNontrivialScratchLet(*arguments[0],
                                                               Precedence::kAdditive);
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

        case k_clamp_IntrinsicKind:
        case k_max_IntrinsicKind:
        case k_min_IntrinsicKind:
        case k_smoothstep_IntrinsicKind:
        case k_step_IntrinsicKind:
            return this->assembleVectorizedIntrinsic(call.function().name(), call);

        case k_abs_IntrinsicKind:
        case k_acos_IntrinsicKind:
        case k_asin_IntrinsicKind:
        case k_ceil_IntrinsicKind:
        case k_cos_IntrinsicKind:
        case k_degrees_IntrinsicKind:
        case k_exp_IntrinsicKind:
        case k_exp2_IntrinsicKind:
        case k_floor_IntrinsicKind:
        case k_fract_IntrinsicKind:
        case k_log_IntrinsicKind:
        case k_log2_IntrinsicKind:
        case k_radians_IntrinsicKind:
        case k_pow_IntrinsicKind:
        case k_sign_IntrinsicKind:
        case k_sin_IntrinsicKind:
        case k_sqrt_IntrinsicKind:
        case k_tan_IntrinsicKind:
        default:
            return this->assembleSimpleIntrinsic(call.function().name(), call);
    }
}

std::string WGSLCodeGenerator::assembleFunctionCall(const FunctionCall& call,
                                                    Precedence parentPrecedence) {
    const FunctionDeclaration& func = call.function();
    std::string result;

    // Many intrinsics need to be rewritten in WGSL.
    if (func.isIntrinsic()) {
        result = this->assembleIntrinsicCall(call, func.intrinsicKind(), parentPrecedence);
        if (!result.empty()) {
            return result;
        }
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
        if (params[index]->modifiers().fFlags & Modifiers::kOut_Flag) {
            std::unique_ptr<LValue> lvalue = this->makeLValue(*args[index]);
            if (params[index]->modifiers().fFlags & Modifiers::kIn_Flag) {
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
        } else {
            expr += this->assembleExpression(*args[index], Precedence::kSequence);
        }
    }
    expr.push_back(')');

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

static std::string make_increment_expr(const Type& type) {
    // `type(`
    std::string expr = to_wgsl_type(type);
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
                make_increment_expr(p.operand()->type());
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
                           make_increment_expr(p.operand()->type());
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
    this->write(to_wgsl_type(type));
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

std::string WGSLCodeGenerator::writeNontrivialScratchLet(const Expression& expr,
                                                         Precedence parentPrecedence) {
    std::string result = this->assembleExpression(expr, parentPrecedence);

    if (expr.is<VariableReference>() || expr.is<Literal>()) {
        // Variables and literals are trivial; adding a let-declaration won't simplify anything.
        return result;
    }
    if (expr.type().isVector() && Analysis::IsConstantExpression(expr)) {
        // Compile-time constant vectors are also considered trivial; they're short and sweet.
        return result;
    }
    return this->writeScratchLet(result);
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
        expr += this->assembleExpression(*t.ifFalse(), Precedence::kTernary);
        expr += ", ";
        expr += this->assembleExpression(*t.ifTrue(), Precedence::kTernary);
        expr += ", ";

        bool isVector = t.type().isVector();
        if (isVector) {
            // Splat the condition expression into a vector.
            expr += String::printf("vec%d<bool>(", t.type().columns());
        }
        expr += this->assembleExpression(*t.test(), Precedence::kTernary);
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

std::string WGSLCodeGenerator::variableReferenceNameForLValue(const VariableReference& r) {
    std::string expr;
    const Variable& v = *r.variable();
    bool needsDeref = false;

    // When a variable is referenced in the context of a synthesized out-parameter helper argument,
    // two special rules apply:
    //     1. If it's accessed via a pipeline I/O or global uniforms struct, it should instead
    //        be referenced by name (since it's actually referring to a function parameter).
    //     2. Its type should be treated as a pointer and should be dereferenced as such.
    if (v.storage() == Variable::Storage::kGlobal) {
        if (v.modifiers().fFlags & Modifiers::kIn_Flag) {
            expr += "_stageIn.";
        } else if (v.modifiers().fFlags & Modifiers::kOut_Flag) {
            expr += "(*_stageOut).";
        } else if (is_in_global_uniforms(v)) {
            expr += "_globalUniforms.";
        }
    } else if ((v.storage() == Variable::Storage::kParameter &&
                v.modifiers().fFlags & Modifiers::kOut_Flag)) {
        // This is an out-parameter and its type is a pointer, which we need to dereference.
        // We wrap the dereference in parentheses in case the value is used in an access expression
        // later.
        needsDeref = true;
        expr += "(*";
    }

    expr += this->assembleName(v.mangledName());
    if (needsDeref) {
        expr.push_back(')');
    }

    return expr;
}

std::string WGSLCodeGenerator::assembleVariableReference(const VariableReference& r) {
    // TODO(skia:13092): Correctly handle RTFlip for built-ins.
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

std::string WGSLCodeGenerator::assembleAnyConstructor(const AnyConstructor& c,
                                                      Precedence parentPrecedence) {
    std::string expr = to_wgsl_type(c.type());
    expr.push_back('(');
    auto separator = SkSL::String::Separator();
    for (const auto& e : c.argumentSpan()) {
        expr += separator();
        expr += this->assembleExpression(*e, Precedence::kSequence);
    }
    expr.push_back(')');
    return expr;
}

std::string WGSLCodeGenerator::assembleConstructorCompound(const ConstructorCompound& c,
                                                           Precedence parentPrecedence) {
    if (c.type().isVector()) {
        return this->assembleConstructorCompoundVector(c, parentPrecedence);
    } else if (c.type().isMatrix()) {
        return this->assembleConstructorCompoundMatrix(c, parentPrecedence);
    } else {
        fContext.fErrors->error(c.fPosition, "unsupported compound constructor");
        return {};
    }
}

std::string WGSLCodeGenerator::assembleConstructorCompoundVector(const ConstructorCompound& c,
                                                                 Precedence parentPrecedence) {
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
            return String::printf("%s(%s[0], %s[1])",
                                  to_wgsl_type(c.type()).c_str(), matrix.c_str(), matrix.c_str());
        }
    }
    return this->assembleAnyConstructor(c, parentPrecedence);
}

std::string WGSLCodeGenerator::assembleConstructorCompoundMatrix(const ConstructorCompound& ctor,
                                                                 Precedence parentPrecedence) {
    SkASSERT(ctor.type().isMatrix());

    std::string expr = to_wgsl_type(ctor.type()) + '(';
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

std::string WGSLCodeGenerator::assembleConstructorDiagonalMatrix(const ConstructorDiagonalMatrix& c,
                                                                 Precedence parentPrecedence) {
    const Type& type = c.type();
    SkASSERT(type.isMatrix());
    SkASSERT(c.argument()->type().isScalar());

    // Evaluate the inner-expression, creating a scratch variable if necessary.
    std::string inner = this->writeNontrivialScratchLet(*c.argument(), Precedence::kAssignment);

    // Assemble a diagonal-matrix expression.
    std::string expr = to_wgsl_type(type) + '(';
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

std::string WGSLCodeGenerator::assembleConstructorMatrixResize(const ConstructorMatrixResize& ctor,
                                                               Precedence parentPrecedence) {
    std::string source = this->writeScratchLet(this->assembleExpression(*ctor.argument(),
                                                                        Precedence::kSequence));
    int columns = ctor.type().columns();
    int rows = ctor.type().rows();
    int sourceColumns = ctor.argument()->type().columns();
    int sourceRows = ctor.argument()->type().rows();
    auto separator = String::Separator();
    std::string expr = to_wgsl_type(ctor.type()) + '(';

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
                                                     op, Precedence::kLogicalAnd);
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
                                                     op, Precedence::kLogicalAnd);
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
                            *field.fType, leftName + '.' + std::string(field.fName),
                            *field.fType, rightName + '.' + std::string(field.fName),
                            op, Precedence::kLogicalAnd);
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
        expr += op.operatorName();
        expr += rightName;
        return expr + ')';
    }

    // Compare scalars via `x == y`.
    SkASSERT(right.isScalar());
    if (Precedence::kEquality >= parentPrecedence) {
        expr = '(';
    }
    expr += leftName;
    expr += op.operatorName();
    expr += rightName;
    if (Precedence::kEquality >= parentPrecedence) {
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
        leftName = this->assembleExpression(left, Precedence::kAssignment);
        rightName = this->assembleExpression(right, Precedence::kAssignment);
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
    std::string initializer;
    if (d.varDeclaration().value()) {
        // We assume here that the initial-value expression will not emit any helper statements.
        // Initial-value expressions are required to pass IsConstantExpression, which limits the
        // blast radius to constructors, literals, and other constant values/variables.
        initializer += " = ";
        initializer += this->assembleExpression(*d.varDeclaration().value(),
                                                Precedence::kAssignment);
    }
    this->write((var.modifiers().fFlags & Modifiers::kConst_Flag) ? "const " : "var<private> ");
    this->write(this->assembleName(var.mangledName()));
    this->write(": " + to_wgsl_type(var.type()));
    this->write(initializer);
    this->writeLine(";");
}

void WGSLCodeGenerator::writeStructDefinition(const StructDefinition& s) {
    const Type& type = s.type();
    this->writeLine("struct " + type.displayName() + " {");
    fIndentation++;
    this->writeFields(SkSpan(type.fields()), type.fPosition);
    fIndentation--;
    this->writeLine("};");
}

void WGSLCodeGenerator::writeFields(SkSpan<const Field> fields,
                                    Position parentPos,
                                    const MemoryLayout*) {
    // TODO(skia:13092): Check alignment against `layout` constraints, if present. A layout
    // constraint will be specified for interface blocks and for structs that appear in a block.
    for (const Field& field : fields) {
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
                this->write("  ");
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

std::string WGSLCodeGenerator::functionDependencyArgs(const FunctionDeclaration& f) {
    FunctionDependencies* deps = fRequirements.dependencies.find(&f);
    std::string args;
    if (deps && *deps != FunctionDependencies::kNone) {
        const char* separator = "";
        if ((*deps & FunctionDependencies::kPipelineInputs) != FunctionDependencies::kNone) {
            args += "_stageIn";
            separator = ", ";
        }
        if ((*deps & FunctionDependencies::kPipelineOutputs) != FunctionDependencies::kNone) {
            args += separator;
            args += "_stageOut";
        }
    }
    return args;
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

}  // namespace SkSL
