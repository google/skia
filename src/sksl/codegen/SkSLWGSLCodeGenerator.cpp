/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/codegen/SkSLWGSLCodeGenerator.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

// TODO(skia:13092): This is a temporary debug feature. Remove when the implementation is
// complete and this is no longer needed.
#define DUMP_SRC_IR 0

namespace SkSL {
namespace {

std::string toScalarType(const Type& type) {
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
    return std::string(type.name());
}

// Convert a SkSL type to a WGSL type. Handles all plain types except structure types
// (see https://www.w3.org/TR/WGSL/#plain-types-section).
std::string toWGSLType(const Type& type) {
    // TODO(skia:13092): Handle array, matrix, sampler types.
    switch (type.typeKind()) {
        case Type::TypeKind::kScalar:
            return toScalarType(type);
        case Type::TypeKind::kVector:
            return "vec" + std::to_string(type.columns()) + "<" +
                   toScalarType(type.componentType()) + ">";
        default:
            break;
    }
    return std::string(type.name());
}

std::string toWGSLBuiltinName(WGSLCodeGenerator::Builtin kind) {
    using Builtin = WGSLCodeGenerator::Builtin;
    switch (kind) {
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

// Map a SkSL builtin flag to a WGSL builtin kind. Returns std::nullopt if `builtin` is not
// not supported for WGSL.
//
// Also see //src/sksl/sksl_vert.sksl and //src/sksl/sksl_frag.sksl for supported built-ins.
std::optional<WGSLCodeGenerator::Builtin> builtinFromSkSLName(int builtin) {
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

}  // namespace

bool WGSLCodeGenerator::generateCode() {
    // The resources of a WGSL program are structured in the following way:
    // - Vertex and fragment stage attribute inputs and outputs are bundled
    //   inside synthetic structs called VSIn/VSOut/FSIn/FSOut.
    // - All uniform and storage type resources are declared in global scope.
    StringStream header;
    {
        AutoOutputStream outputToHeader(this, &header, &fIndentation);
        // TODO(skia:13092): Implement the following:
        // - struct definitions
        // - global uniform/storage resource declarations, including interface blocks.
        this->writeStageInputStruct();
        this->writeStageOutputStruct();
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
    fContext.fErrors->reportPendingErrors(Position());
    return fContext.fErrors->errorCount() == 0;
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

void WGSLCodeGenerator::writePipelineIODeclaration(Modifiers modifiers,
                                                   const Type& type,
                                                   std::string_view name) {
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
        this->writeUserDefinedVariableDecl(type, name, location);
    } else if (modifiers.fLayout.fBuiltin >= 0) {
        auto builtin = builtinFromSkSLName(modifiers.fLayout.fBuiltin);
        if (builtin.has_value()) {
            this->writeBuiltinVariableDecl(type, name, *builtin);
        }
    }
}

void WGSLCodeGenerator::writeUserDefinedVariableDecl(const Type& type,
                                                     std::string_view name,
                                                     int location) {
    this->write("@location(" + std::to_string(location) + ") ");
    this->writeName(name);
    this->write(": " + toWGSLType(type));
    this->writeLine(";");
}

void WGSLCodeGenerator::writeBuiltinVariableDecl(const Type& type,
                                                 std::string_view name,
                                                 Builtin kind) {
    this->write("@builtin(" + toWGSLBuiltinName(kind) + ") ");
    this->writeName(name);
    this->write(": " + toWGSLType(type));
    this->writeLine(";");
}

void WGSLCodeGenerator::writeProgramElement(const ProgramElement& e) {
    // TODO(skia:13092): implement
}

void WGSLCodeGenerator::writeStageInputStruct() {
    std::string structNamePrefix;
    if (fProgram.fConfig->fKind == ProgramKind::kVertex) {
        structNamePrefix = "VS";
    } else if (fProgram.fConfig->fKind == ProgramKind::kFragment) {
        structNamePrefix = "FS";
    } else {
        // There's no need to declare pipeline stage outputs.
        return;
    }

    this->write("struct ");
    this->write(structNamePrefix);
    this->writeLine("In {");
    fIndentation++;

    // TODO(skia:13092): Remember all variables that are added to the input struct here so they
    // can be referenced correctly when handling variable references.
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const Variable& v =
                    e->as<GlobalVarDeclaration>().declaration()->as<VarDeclaration>().var();
            if (v.modifiers().fFlags & Modifiers::kIn_Flag) {
                this->writePipelineIODeclaration(v.modifiers(), v.type(), v.name());
            }
        } else if (e->is<InterfaceBlock>()) {
            const Variable& v = e->as<InterfaceBlock>().variable();
            // Merge all the members of `in` interface blocks to the input struct, which are
            // specified as either "builtin" or with a "layout(location=".
            //
            // TODO(armansito): Is it legal to have an interface block without a storage qualifier
            // but with members that have individual storage qualifiers?
            if (v.modifiers().fFlags & Modifiers::kIn_Flag) {
                for (const auto& f : v.type().fields()) {
                    this->writePipelineIODeclaration(f.fModifiers, *f.fType, f.fName);
                }
            }
        }
    }

    fIndentation--;
    this->writeLine("};");
}

void WGSLCodeGenerator::writeStageOutputStruct() {
    std::string structNamePrefix;
    if (fProgram.fConfig->fKind == ProgramKind::kVertex) {
        structNamePrefix = "VS";
    } else if (fProgram.fConfig->fKind == ProgramKind::kFragment) {
        structNamePrefix = "FS";
    } else {
        // There's no need to declare pipeline stage outputs.
        return;
    }

    this->write("struct ");
    this->write(structNamePrefix);
    this->writeLine("Out {");
    fIndentation++;

    // TODO(skia:13092): Remember all variables that are added to the output struct here so they
    // can be referenced correctly when handling variable references.
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const Variable& v =
                    e->as<GlobalVarDeclaration>().declaration()->as<VarDeclaration>().var();
            if (v.modifiers().fFlags & Modifiers::kOut_Flag) {
                this->writePipelineIODeclaration(v.modifiers(), v.type(), v.name());
            }
        } else if (e->is<InterfaceBlock>()) {
            const Variable& v = e->as<InterfaceBlock>().variable();
            // Merge all the members of `out` interface blocks to the output struct, which are
            // specified as either "builtin" or with a "layout(location=".
            //
            // TODO(armansito): Is it legal to have an interface block without a storage qualifier
            // but with members that have individual storage qualifiers?
            if (v.modifiers().fFlags & Modifiers::kOut_Flag) {
                for (const auto& f : v.type().fields()) {
                    this->writePipelineIODeclaration(f.fModifiers, *f.fType, f.fName);
                }
            }
        }
    }

    fIndentation--;
    this->writeLine("};");
}

}  // namespace SkSL
