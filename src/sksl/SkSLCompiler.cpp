/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#include "SkSLCompiler.h"

#include <fstream>
#include <streambuf>

#include "SkSLIRGenerator.h"
#include "SkSLParser.h"
#include "SkSLSPIRVCodeGenerator.h"
#include "ir/SkSLExpression.h"
#include "ir/SkSLIntLiteral.h"
#include "ir/SkSLSymbolTable.h"
#include "ir/SkSLVarDeclaration.h"
#include "SkMutex.h"

#define STRINGIFY(x) #x

// include the built-in shader symbols as static strings

static std::string SKSL_INCLUDE = 
#include "sksl.include"
;

static std::string SKSL_VERT_INCLUDE = 
#include "sksl_vert.include"
;

static std::string SKSL_FRAG_INCLUDE = 
#include "sksl_frag.include"
;

namespace SkSL {

Compiler::Compiler() 
: fErrorCount(0) {
    auto types = std::shared_ptr<SymbolTable>(new SymbolTable(*this));
    auto symbols = std::shared_ptr<SymbolTable>(new SymbolTable(types, *this));
    fIRGenerator = new IRGenerator(&fContext, symbols, *this);
    fTypes = types;
    #define ADD_TYPE(t) types->addWithoutOwnership(fContext.f ## t ## _Type->fName, \
                                                   fContext.f ## t ## _Type.get())
    ADD_TYPE(Void);
    ADD_TYPE(Float);
    ADD_TYPE(Vec2);
    ADD_TYPE(Vec3);
    ADD_TYPE(Vec4);
    ADD_TYPE(Double);
    ADD_TYPE(DVec2);
    ADD_TYPE(DVec3);
    ADD_TYPE(DVec4);
    ADD_TYPE(Int);
    ADD_TYPE(IVec2);
    ADD_TYPE(IVec3);
    ADD_TYPE(IVec4);
    ADD_TYPE(UInt);
    ADD_TYPE(UVec2);
    ADD_TYPE(UVec3);
    ADD_TYPE(UVec4);
    ADD_TYPE(Bool);
    ADD_TYPE(BVec2);
    ADD_TYPE(BVec3);
    ADD_TYPE(BVec4);
    ADD_TYPE(Mat2x2);
    types->addWithoutOwnership("mat2x2", fContext.fMat2x2_Type.get());
    ADD_TYPE(Mat2x3);
    ADD_TYPE(Mat2x4);
    ADD_TYPE(Mat3x2);
    ADD_TYPE(Mat3x3);
    types->addWithoutOwnership("mat3x3", fContext.fMat3x3_Type.get());
    ADD_TYPE(Mat3x4);
    ADD_TYPE(Mat4x2);
    ADD_TYPE(Mat4x3);
    ADD_TYPE(Mat4x4);
    types->addWithoutOwnership("mat4x4", fContext.fMat4x4_Type.get());
    ADD_TYPE(GenType);
    ADD_TYPE(GenDType);
    ADD_TYPE(GenIType);
    ADD_TYPE(GenUType);
    ADD_TYPE(GenBType);
    ADD_TYPE(Mat);
    ADD_TYPE(Vec);
    ADD_TYPE(GVec);
    ADD_TYPE(GVec2);
    ADD_TYPE(GVec3);
    ADD_TYPE(GVec4);
    ADD_TYPE(DVec);
    ADD_TYPE(IVec);
    ADD_TYPE(UVec);
    ADD_TYPE(BVec);

    ADD_TYPE(Sampler1D);
    ADD_TYPE(Sampler2D);
    ADD_TYPE(Sampler3D);
    ADD_TYPE(SamplerCube);
    ADD_TYPE(Sampler2DRect);
    ADD_TYPE(Sampler1DArray);
    ADD_TYPE(Sampler2DArray);
    ADD_TYPE(SamplerCubeArray);
    ADD_TYPE(SamplerBuffer);
    ADD_TYPE(Sampler2DMS);
    ADD_TYPE(Sampler2DMSArray);

    ADD_TYPE(GSampler1D);
    ADD_TYPE(GSampler2D);
    ADD_TYPE(GSampler3D);
    ADD_TYPE(GSamplerCube);
    ADD_TYPE(GSampler2DRect);
    ADD_TYPE(GSampler1DArray);
    ADD_TYPE(GSampler2DArray);
    ADD_TYPE(GSamplerCubeArray);
    ADD_TYPE(GSamplerBuffer);
    ADD_TYPE(GSampler2DMS);
    ADD_TYPE(GSampler2DMSArray);

    ADD_TYPE(Sampler1DShadow);
    ADD_TYPE(Sampler2DShadow);
    ADD_TYPE(SamplerCubeShadow);
    ADD_TYPE(Sampler2DRectShadow);
    ADD_TYPE(Sampler1DArrayShadow);
    ADD_TYPE(Sampler2DArrayShadow);
    ADD_TYPE(SamplerCubeArrayShadow);
    ADD_TYPE(GSampler2DArrayShadow);
    ADD_TYPE(GSamplerCubeArrayShadow);

    std::vector<std::unique_ptr<ProgramElement>> ignored;
    this->internalConvertProgram(SKSL_INCLUDE, &ignored);
    ASSERT(!fErrorCount);
}

Compiler::~Compiler() {
    delete fIRGenerator;
}

void Compiler::internalConvertProgram(std::string text,
                                      std::vector<std::unique_ptr<ProgramElement>>* result) {
    Parser parser(text, *fTypes, *this);
    std::vector<std::unique_ptr<ASTDeclaration>> parsed = parser.file();
    if (fErrorCount) {
        return;
    }
    for (size_t i = 0; i < parsed.size(); i++) {
        ASTDeclaration& decl = *parsed[i];
        switch (decl.fKind) {
            case ASTDeclaration::kVar_Kind: {
                std::unique_ptr<VarDeclarations> s = fIRGenerator->convertVarDeclarations(
                                                                         (ASTVarDeclarations&) decl, 
                                                                         Variable::kGlobal_Storage);
                if (s) {
                    result->push_back(std::move(s));
                }
                break;
            }
            case ASTDeclaration::kFunction_Kind: {
                std::unique_ptr<FunctionDefinition> f = fIRGenerator->convertFunction(
                                                                               (ASTFunction&) decl);
                if (f) {
                    result->push_back(std::move(f));
                }
                break;
            }
            case ASTDeclaration::kInterfaceBlock_Kind: {
                std::unique_ptr<InterfaceBlock> i = fIRGenerator->convertInterfaceBlock(
                                                                         (ASTInterfaceBlock&) decl);
                if (i) {
                    result->push_back(std::move(i));
                }
                break;
            }
            case ASTDeclaration::kExtension_Kind: {
                std::unique_ptr<Extension> e = fIRGenerator->convertExtension((ASTExtension&) decl);
                if (e) {
                    result->push_back(std::move(e));
                }
                break;
            }
            default:
                ABORT("unsupported declaration: %s\n", decl.description().c_str());
        }
    }
}

std::unique_ptr<Program> Compiler::convertProgram(Program::Kind kind, std::string text) {
    fErrorText = "";
    fErrorCount = 0;
    fIRGenerator->pushSymbolTable();
    std::vector<std::unique_ptr<ProgramElement>> elements;
    switch (kind) {
        case Program::kVertex_Kind:
            this->internalConvertProgram(SKSL_VERT_INCLUDE, &elements);
            break;
        case Program::kFragment_Kind:
            this->internalConvertProgram(SKSL_FRAG_INCLUDE, &elements);
            break;
    }
    this->internalConvertProgram(text, &elements);
    auto result = std::unique_ptr<Program>(new Program(kind, std::move(elements), 
                                                       fIRGenerator->fSymbolTable));;
    fIRGenerator->popSymbolTable();
    this->writeErrorCount();
    return result;
}

void Compiler::error(Position position, std::string msg) {
    fErrorCount++;
    fErrorText += "error: " + position.description() + ": " + msg.c_str() + "\n";
}

std::string Compiler::errorText() {
    std::string result = fErrorText;
    return result;
}

void Compiler::writeErrorCount() {
    if (fErrorCount) {
        fErrorText += to_string(fErrorCount) + " error";
        if (fErrorCount > 1) {
            fErrorText += "s";
        }
        fErrorText += "\n";
    }
}

bool Compiler::toSPIRV(Program::Kind kind, const std::string& text, std::ostream& out) {
    auto program = this->convertProgram(kind, text);
    if (fErrorCount == 0) {
        SkSL::SPIRVCodeGenerator cg(&fContext);
        cg.generateCode(*program.get(), out);
        ASSERT(!out.rdstate());
    }
    return fErrorCount == 0;
}

bool Compiler::toSPIRV(Program::Kind kind, const std::string& text, std::string* out) {
    std::stringstream buffer;
    bool result = this->toSPIRV(kind, text, buffer);
    if (result) {
        *out = buffer.str();
    }
    return result;
}

bool Compiler::toGLSL(Program::Kind kind, const std::string& text, GLCaps caps, 
                      std::ostream& out) {
    auto program = this->convertProgram(kind, text);
    if (fErrorCount == 0) {
        SkSL::GLSLCodeGenerator cg(&fContext, caps);
        cg.generateCode(*program.get(), out);
        ASSERT(!out.rdstate());
    }
    return fErrorCount == 0;
}

bool Compiler::toGLSL(Program::Kind kind, const std::string& text, GLCaps caps, 
                      std::string* out) {
    std::stringstream buffer;
    bool result = this->toGLSL(kind, text, caps, buffer);
    if (result) {
        *out = buffer.str();
    }
    return result;
}

} // namespace
