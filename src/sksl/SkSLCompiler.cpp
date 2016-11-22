/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLCompiler.h"

#include "ast/SkSLASTPrecision.h"
#include "SkSLCFGGenerator.h"
#include "SkSLIRGenerator.h"
#include "SkSLParser.h"
#include "SkSLSPIRVCodeGenerator.h"
#include "ir/SkSLExpression.h"
#include "ir/SkSLIntLiteral.h"
#include "ir/SkSLModifiersDeclaration.h"
#include "ir/SkSLSymbolTable.h"
#include "ir/SkSLUnresolvedFunction.h"
#include "ir/SkSLVarDeclarations.h"
#include "SkMutex.h"

#define STRINGIFY(x) #x

// include the built-in shader symbols as static strings

static const char* SKSL_INCLUDE =
#include "sksl.include"
;

static const char* SKSL_VERT_INCLUDE =
#include "sksl_vert.include"
;

static const char* SKSL_FRAG_INCLUDE =
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
    types->addWithoutOwnership(SkString("mat2x2"), fContext.fMat2x2_Type.get());
    ADD_TYPE(Mat2x3);
    ADD_TYPE(Mat2x4);
    ADD_TYPE(Mat3x2);
    ADD_TYPE(Mat3x3);
    types->addWithoutOwnership(SkString("mat3x3"), fContext.fMat3x3_Type.get());
    ADD_TYPE(Mat3x4);
    ADD_TYPE(Mat4x2);
    ADD_TYPE(Mat4x3);
    ADD_TYPE(Mat4x4);
    types->addWithoutOwnership(SkString("mat4x4"), fContext.fMat4x4_Type.get());
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
    ADD_TYPE(SamplerExternalOES);
    ADD_TYPE(SamplerCube);
    ADD_TYPE(Sampler2DRect);
    ADD_TYPE(Sampler1DArray);
    ADD_TYPE(Sampler2DArray);
    ADD_TYPE(SamplerCubeArray);
    ADD_TYPE(SamplerBuffer);
    ADD_TYPE(Sampler2DMS);
    ADD_TYPE(Sampler2DMSArray);

    ADD_TYPE(ISampler2D);

    ADD_TYPE(Image2D);
    ADD_TYPE(IImage2D);

    ADD_TYPE(SubpassInput);
    ADD_TYPE(SubpassInputMS);

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

    SkString skCapsName("sk_Caps");
    Variable* skCaps = new Variable(Position(), Modifiers(), skCapsName, 
                                    *fContext.fSkCaps_Type, Variable::kGlobal_Storage);
    fIRGenerator->fSymbolTable->add(skCapsName, std::unique_ptr<Symbol>(skCaps));

    Modifiers::Flag ignored1;
    std::vector<std::unique_ptr<ProgramElement>> ignored2;
    this->internalConvertProgram(SkString(SKSL_INCLUDE), &ignored1, &ignored2);
    fIRGenerator->fSymbolTable->markAllFunctionsBuiltin();
    ASSERT(!fErrorCount);
}

Compiler::~Compiler() {
    delete fIRGenerator;
}

// add the definition created by assigning to the lvalue to the definition set
void Compiler::addDefinition(const Expression* lvalue, const Expression* expr,
                           std::unordered_map<const Variable*, const Expression*>* definitions) {
    switch (lvalue->fKind) {
        case Expression::kVariableReference_Kind: {
            const Variable& var = ((VariableReference*) lvalue)->fVariable;
            if (var.fStorage == Variable::kLocal_Storage) {
                (*definitions)[&var] = expr;
            }
            break;
        }
        case Expression::kSwizzle_Kind:
            // We consider the variable written to as long as at least some of its components have
            // been written to. This will lead to some false negatives (we won't catch it if you
            // write to foo.x and then read foo.y), but being stricter could lead to false positives
            // (we write to foo.x, and then pass foo to a function which happens to only read foo.x,
            // but since we pass foo as a whole it is flagged as an error) unless we perform a much
            // more complicated whole-program analysis. This is probably good enough.
            this->addDefinition(((Swizzle*) lvalue)->fBase.get(),
                                fContext.fDefined_Expression.get(),
                                definitions);
            break;
        case Expression::kIndex_Kind:
            // see comments in Swizzle
            this->addDefinition(((IndexExpression*) lvalue)->fBase.get(),
                                fContext.fDefined_Expression.get(),
                                definitions);
            break;
        case Expression::kFieldAccess_Kind:
            // see comments in Swizzle
            this->addDefinition(((FieldAccess*) lvalue)->fBase.get(),
                                fContext.fDefined_Expression.get(),
                                definitions);
            break;
        default:
            // not an lvalue, can't happen
            ASSERT(false);
    }
}

// add local variables defined by this node to the set
void Compiler::addDefinitions(const BasicBlock::Node& node,
                              std::unordered_map<const Variable*, const Expression*>* definitions) {
    switch (node.fKind) {
        case BasicBlock::Node::kExpression_Kind: {
            const Expression* expr = (Expression*) node.fNode;
            if (expr->fKind == Expression::kBinary_Kind) {
                const BinaryExpression* b = (BinaryExpression*) expr;
                if (b->fOperator == Token::EQ) {
                    this->addDefinition(b->fLeft.get(), b->fRight.get(), definitions);
                }
            }
            break;
        }
        case BasicBlock::Node::kStatement_Kind: {
            const Statement* stmt = (Statement*) node.fNode;
            if (stmt->fKind == Statement::kVarDeclarations_Kind) {
                const VarDeclarationsStatement* vd = (VarDeclarationsStatement*) stmt;
                for (const VarDeclaration& decl : vd->fDeclaration->fVars) {
                    if (decl.fValue) {
                        (*definitions)[decl.fVar] = decl.fValue.get();
                    }
                }
            }
            break;
        }
    }
}

void Compiler::scanCFG(CFG* cfg, BlockId blockId, std::set<BlockId>* workList) {
    BasicBlock& block = cfg->fBlocks[blockId];

    // compute definitions after this block
    std::unordered_map<const Variable*, const Expression*> after = block.fBefore;
    for (const BasicBlock::Node& n : block.fNodes) {
        this->addDefinitions(n, &after);
    }

    // propagate definitions to exits
    for (BlockId exitId : block.fExits) {
        BasicBlock& exit = cfg->fBlocks[exitId];
        for (const auto& pair : after) {
            const Expression* e1 = pair.second;
            if (exit.fBefore.find(pair.first) == exit.fBefore.end()) {
                exit.fBefore[pair.first] = e1;
            } else {
                const Expression* e2 = exit.fBefore[pair.first];
                if (e1 != e2) {
                    // definition has changed, merge and add exit block to worklist
                    workList->insert(exitId);
                    if (!e1 || !e2) {
                        exit.fBefore[pair.first] = nullptr;
                    } else {
                        exit.fBefore[pair.first] = fContext.fDefined_Expression.get();
                    }
                }
            }
        }
    }
}

// returns a map which maps all local variables in the function to null, indicating that their value
// is initially unknown
static std::unordered_map<const Variable*, const Expression*> compute_start_state(const CFG& cfg) {
    std::unordered_map<const Variable*, const Expression*> result;
    for (const auto& block : cfg.fBlocks) {
        for (const auto& node : block.fNodes) {
            if (node.fKind == BasicBlock::Node::kStatement_Kind) {
                const Statement* s = (Statement*) node.fNode;
                if (s->fKind == Statement::kVarDeclarations_Kind) {
                    const VarDeclarationsStatement* vd = (const VarDeclarationsStatement*) s;
                    for (const VarDeclaration& decl : vd->fDeclaration->fVars) {
                        result[decl.fVar] = nullptr;
                    }
                }
            }
        }
    }
    return result;
}

void Compiler::scanCFG(const FunctionDefinition& f) {
    CFG cfg = CFGGenerator().getCFG(f);

    // compute the data flow
    cfg.fBlocks[cfg.fStart].fBefore = compute_start_state(cfg);
    std::set<BlockId> workList;
    for (BlockId i = 0; i < cfg.fBlocks.size(); i++) {
        workList.insert(i);
    }
    while (workList.size()) {
        BlockId next = *workList.begin();
        workList.erase(workList.begin());
        this->scanCFG(&cfg, next, &workList);
    }

    // check for unreachable code
    for (size_t i = 0; i < cfg.fBlocks.size(); i++) {
        if (i != cfg.fStart && !cfg.fBlocks[i].fEntrances.size() &&
            cfg.fBlocks[i].fNodes.size()) {
            this->error(cfg.fBlocks[i].fNodes[0].fNode->fPosition, SkString("unreachable"));
        }
    }
    if (fErrorCount) {
        return;
    }

    // check for undefined variables
    for (const BasicBlock& b : cfg.fBlocks) {
        std::unordered_map<const Variable*, const Expression*> definitions = b.fBefore;
        for (const BasicBlock::Node& n : b.fNodes) {
            if (n.fKind == BasicBlock::Node::kExpression_Kind) {
                const Expression* expr = (const Expression*) n.fNode;
                if (expr->fKind == Expression::kVariableReference_Kind) {
                    const Variable& var = ((VariableReference*) expr)->fVariable;
                    if (var.fStorage == Variable::kLocal_Storage &&
                        !definitions[&var]) {
                        this->error(expr->fPosition,
                                    "'" + var.fName + "' has not been assigned");
                    }
                }
            }
            this->addDefinitions(n, &definitions);
        }
    }

    // check for missing return
    if (f.fDeclaration.fReturnType != *fContext.fVoid_Type) {
        if (cfg.fBlocks[cfg.fExit].fEntrances.size()) {
            this->error(f.fPosition, SkString("function can exit without returning a value"));
        }
    }
}

void Compiler::internalConvertProgram(SkString text,
                                      Modifiers::Flag* defaultPrecision,
                                      std::vector<std::unique_ptr<ProgramElement>>* result) {
    Parser parser(text, *fTypes, *this);
    std::vector<std::unique_ptr<ASTDeclaration>> parsed = parser.file();
    if (fErrorCount) {
        return;
    }
    *defaultPrecision = Modifiers::kHighp_Flag;
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
                if (!fErrorCount && f) {
                    this->scanCFG(*f);
                    result->push_back(std::move(f));
                }
                break;
            }
            case ASTDeclaration::kModifiers_Kind: {
                std::unique_ptr<ModifiersDeclaration> f = fIRGenerator->convertModifiersDeclaration(
                                                                   (ASTModifiersDeclaration&) decl);
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
            case ASTDeclaration::kPrecision_Kind: {
                *defaultPrecision = ((ASTPrecision&) decl).fPrecision;
                break;
            }
            default:
                ABORT("unsupported declaration: %s\n", decl.description().c_str());
        }
    }
}

std::unique_ptr<Program> Compiler::convertProgram(Program::Kind kind, SkString text,
                                                  std::unordered_map<SkString, CapValue> caps) {
    fErrorText = "";
    fErrorCount = 0;
    fIRGenerator->start(&caps);
    std::vector<std::unique_ptr<ProgramElement>> elements;
    Modifiers::Flag ignored;
    switch (kind) {
        case Program::kVertex_Kind:
            this->internalConvertProgram(SkString(SKSL_VERT_INCLUDE), &ignored, &elements);
            break;
        case Program::kFragment_Kind:
            this->internalConvertProgram(SkString(SKSL_FRAG_INCLUDE), &ignored, &elements);
            break;
    }
    fIRGenerator->fSymbolTable->markAllFunctionsBuiltin();
    Modifiers::Flag defaultPrecision;
    this->internalConvertProgram(text, &defaultPrecision, &elements);
    auto result = std::unique_ptr<Program>(new Program(kind, defaultPrecision, std::move(elements),
                                                       fIRGenerator->fSymbolTable));
    fIRGenerator->finish();
    this->writeErrorCount();
    return result;
}

void Compiler::error(Position position, SkString msg) {
    fErrorCount++;
    fErrorText += "error: " + position.description() + ": " + msg.c_str() + "\n";
}

SkString Compiler::errorText() {
    SkString result = fErrorText;
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

bool Compiler::toSPIRV(Program::Kind kind, const SkString& text, SkWStream& out) {
    std::unordered_map<SkString, CapValue> capsMap;
    auto program = this->convertProgram(kind, text, capsMap);
    if (fErrorCount == 0) {
        SkSL::SPIRVCodeGenerator cg(&fContext);
        cg.generateCode(*program.get(), out);
    }
    return fErrorCount == 0;
}

bool Compiler::toSPIRV(Program::Kind kind, const SkString& text, SkString* out) {
    SkDynamicMemoryWStream buffer;
    bool result = this->toSPIRV(kind, text, buffer);
    if (result) {
        sk_sp<SkData> data(buffer.detachAsData());
        *out = SkString((const char*) data->data(), data->size());
    }
    return result;
}

static void fill_caps(const GrGLSLCaps& caps, std::unordered_map<SkString, CapValue>* capsMap) {
#define CAP(name) capsMap->insert(std::make_pair(SkString(#name), CapValue(caps.name())));
    CAP(fbFetchSupport);
    CAP(fbFetchNeedsCustomOutput);
    CAP(bindlessTextureSupport);
    CAP(dropsTileOnZeroDivide);
    CAP(flatInterpolationSupport);
    CAP(noperspectiveInterpolationSupport);
    CAP(multisampleInterpolationSupport);
    CAP(sampleVariablesSupport);
    CAP(sampleMaskOverrideCoverageSupport);
    CAP(externalTextureSupport);
    CAP(texelFetchSupport);
    CAP(imageLoadStoreSupport);
    CAP(mustEnableAdvBlendEqs);
    CAP(mustEnableSpecificAdvBlendEqs);
    CAP(mustDeclareFragmentShaderOutput);
    CAP(canUseAnyFunctionInShader);
#undef CAP
}

bool Compiler::toGLSL(Program::Kind kind, const SkString& text, const GrGLSLCaps& caps,
                      SkWStream& out) {
    std::unordered_map<SkString, CapValue> capsMap;
    fill_caps(caps, &capsMap);
    auto program = this->convertProgram(kind, text, capsMap);
    if (fErrorCount == 0) {
        SkSL::GLSLCodeGenerator cg(&fContext, &caps);
        cg.generateCode(*program.get(), out);
    }
    return fErrorCount == 0;
}

bool Compiler::toGLSL(Program::Kind kind, const SkString& text, const GrGLSLCaps& caps,
                      SkString* out) {
    SkDynamicMemoryWStream buffer;
    bool result = this->toGLSL(kind, text, caps, buffer);
    if (result) {
        sk_sp<SkData> data(buffer.detachAsData());
        *out = SkString((const char*) data->data(), data->size());
    }
    return result;
}

} // namespace
