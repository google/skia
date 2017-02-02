/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLCompiler.h"

#include "ast/SkSLASTPrecision.h"
#include "SkSLCFGGenerator.h"
#include "SkSLGLSLCodeGenerator.h"
#include "SkSLIRGenerator.h"
#include "SkSLParser.h"
#include "SkSLSPIRVCodeGenerator.h"
#include "ir/SkSLExpression.h"
#include "ir/SkSLExpressionStatement.h"
#include "ir/SkSLIntLiteral.h"
#include "ir/SkSLModifiersDeclaration.h"
#include "ir/SkSLNop.h"
#include "ir/SkSLSymbolTable.h"
#include "ir/SkSLTernaryExpression.h"
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
void Compiler::addDefinition(const Expression* lvalue, std::unique_ptr<Expression>* expr,
                             DefinitionMap* definitions) {
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
                                (std::unique_ptr<Expression>*) &fContext.fDefined_Expression,
                                definitions);
            break;
        case Expression::kIndex_Kind:
            // see comments in Swizzle
            this->addDefinition(((IndexExpression*) lvalue)->fBase.get(),
                                (std::unique_ptr<Expression>*) &fContext.fDefined_Expression,
                                definitions);
            break;
        case Expression::kFieldAccess_Kind:
            // see comments in Swizzle
            this->addDefinition(((FieldAccess*) lvalue)->fBase.get(),
                                (std::unique_ptr<Expression>*) &fContext.fDefined_Expression,
                                definitions);
            break;
        default:
            // not an lvalue, can't happen
            ASSERT(false);
    }
}

// add local variables defined by this node to the set
void Compiler::addDefinitions(const BasicBlock::Node& node,
                              DefinitionMap* definitions) {
    switch (node.fKind) {
        case BasicBlock::Node::kExpression_Kind: {
            ASSERT(node.fExpression);
            const Expression* expr = (Expression*) node.fExpression->get();
            switch (expr->fKind) {
                case Expression::kBinary_Kind: {
                    BinaryExpression* b = (BinaryExpression*) expr;
                    if (b->fOperator == Token::EQ) {
                        this->addDefinition(b->fLeft.get(), &b->fRight, definitions);
                    } else if (Token::IsAssignment(b->fOperator)) {
                        this->addDefinition(
                                       b->fLeft.get(),
                                       (std::unique_ptr<Expression>*) &fContext.fDefined_Expression,
                                       definitions);

                    }
                    break;
                }
                case Expression::kPrefix_Kind: {
                    const PrefixExpression* p = (PrefixExpression*) expr;
                    if (p->fOperator == Token::MINUSMINUS || p->fOperator == Token::PLUSPLUS) {
                        this->addDefinition(
                                       p->fOperand.get(),
                                       (std::unique_ptr<Expression>*) &fContext.fDefined_Expression,
                                       definitions);
                    }
                    break;
                }
                case Expression::kPostfix_Kind: {
                    const PostfixExpression* p = (PostfixExpression*) expr;
                    if (p->fOperator == Token::MINUSMINUS || p->fOperator == Token::PLUSPLUS) {
                        this->addDefinition(
                                       p->fOperand.get(),
                                       (std::unique_ptr<Expression>*) &fContext.fDefined_Expression,
                                       definitions);
                    }
                    break;
                }
                case Expression::kVariableReference_Kind: {
                    const VariableReference* v = (VariableReference*) expr;
                    if (v->fRefKind != VariableReference::kRead_RefKind) {
                        this->addDefinition(
                                       v,
                                       (std::unique_ptr<Expression>*) &fContext.fDefined_Expression,
                                       definitions);
                    }
                }
                default:
                    break;
            }
            break;
        }
        case BasicBlock::Node::kStatement_Kind: {
            const Statement* stmt = (Statement*) node.fStatement->get();
            if (stmt->fKind == Statement::kVarDeclarations_Kind) {
                VarDeclarationsStatement* vd = (VarDeclarationsStatement*) stmt;
                for (const auto& decl : vd->fDeclaration->fVars) {
                    if (decl->fValue) {
                        (*definitions)[decl->fVar] = &decl->fValue;
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
    DefinitionMap after = block.fBefore;
    for (const BasicBlock::Node& n : block.fNodes) {
        this->addDefinitions(n, &after);
    }

    // propagate definitions to exits
    for (BlockId exitId : block.fExits) {
        BasicBlock& exit = cfg->fBlocks[exitId];
        for (const auto& pair : after) {
            std::unique_ptr<Expression>* e1 = pair.second;
            auto found = exit.fBefore.find(pair.first);
            if (found == exit.fBefore.end()) {
                // exit has no definition for it, just copy it
                workList->insert(exitId);
                exit.fBefore[pair.first] = e1;
            } else {
                // exit has a (possibly different) value already defined
                std::unique_ptr<Expression>* e2 = exit.fBefore[pair.first];
                if (e1 != e2) {
                    // definition has changed, merge and add exit block to worklist
                    workList->insert(exitId);
                    exit.fBefore[pair.first] =
                                       (std::unique_ptr<Expression>*) &fContext.fDefined_Expression;
                }
            }
        }
    }
}

// returns a map which maps all local variables in the function to null, indicating that their value
// is initially unknown
static DefinitionMap compute_start_state(const CFG& cfg) {
    DefinitionMap result;
    for (const auto& block : cfg.fBlocks) {
        for (const auto& node : block.fNodes) {
            if (node.fKind == BasicBlock::Node::kStatement_Kind) {
                ASSERT(node.fStatement);
                const Statement* s = node.fStatement->get();
                if (s->fKind == Statement::kVarDeclarations_Kind) {
                    const VarDeclarationsStatement* vd = (const VarDeclarationsStatement*) s;
                    for (const auto& decl : vd->fDeclaration->fVars) {
                        result[decl->fVar] = nullptr;
                    }
                }
            }
        }
    }
    return result;
}

/**
 * Returns true if assigning to this lvalue has no effect.
 */
static bool is_dead(const Expression& lvalue) {
    switch (lvalue.fKind) {
        case Expression::kVariableReference_Kind:
            return ((VariableReference&) lvalue).fVariable.dead();
        case Expression::kSwizzle_Kind:
            return is_dead(*((Swizzle&) lvalue).fBase);
        case Expression::kFieldAccess_Kind:
            return is_dead(*((FieldAccess&) lvalue).fBase);
        case Expression::kIndex_Kind: {
            const IndexExpression& idx = (IndexExpression&) lvalue;
            return is_dead(*idx.fBase) && !idx.fIndex->hasSideEffects();
        }
        default:
            SkDebugf("invalid lvalue: %s\n", lvalue.description().c_str());
            ASSERT(false);
            return false;
    }
}

/**
 * Returns true if this is an assignment which can be collapsed down to just the right hand side due
 * to a dead target and lack of side effects on the left hand side.
 */
static bool dead_assignment(const BinaryExpression& b) {
    if (!Token::IsAssignment(b.fOperator)) {
        return false;
    }
    return is_dead(*b.fLeft);
}

void Compiler::computeDataFlow(CFG* cfg) {
    cfg->fBlocks[cfg->fStart].fBefore = compute_start_state(*cfg);
    std::set<BlockId> workList;
    for (BlockId i = 0; i < cfg->fBlocks.size(); i++) {
        workList.insert(i);
    }
    while (workList.size()) {
        BlockId next = *workList.begin();
        workList.erase(workList.begin());
        this->scanCFG(cfg, next, &workList);
    }
}


/**
 * Attempts to replace the expression pointed to by iter with a new one (in both the CFG and the
 * IR). If the expression can be cleanly removed, returns true and updates the iterator to point to
 * the newly-inserted element. Otherwise updates only the IR and returns false (and the CFG will
 * need to be regenerated).
 */
bool SK_WARN_UNUSED_RESULT try_replace_expression(BasicBlock* b,
                                                  std::vector<BasicBlock::Node>::iterator* iter,
                                                  std::unique_ptr<Expression> newExpression) {
    std::unique_ptr<Expression>* target = (*iter)->fExpression;
    if (!b->tryRemoveExpression(iter)) {
        *target = std::move(newExpression);
        return false;
    }
    *target = std::move(newExpression);
    return b->tryInsertExpression( iter, target);
}

void Compiler::scanCFG(FunctionDefinition& f) {
    CFG cfg = CFGGenerator().getCFG(f);
    this->computeDataFlow(&cfg);

    // check for unreachable code
    for (size_t i = 0; i < cfg.fBlocks.size(); i++) {
        if (i != cfg.fStart && !cfg.fBlocks[i].fEntrances.size() &&
            cfg.fBlocks[i].fNodes.size()) {
            Position p;
            switch (cfg.fBlocks[i].fNodes[0].fKind) {
                case BasicBlock::Node::kStatement_Kind:
                    p = (*cfg.fBlocks[i].fNodes[0].fStatement)->fPosition;
                    break;
                case BasicBlock::Node::kExpression_Kind:
                    p = (*cfg.fBlocks[i].fNodes[0].fExpression)->fPosition;
                    break;
            }
            this->error(p, SkString("unreachable"));
        }
    }
    if (fErrorCount) {
        return;
    }

    // check for dead code & undefined variables, perform constant propagation
    std::unordered_set<const Variable*> undefinedVariables;
    bool updated;
    bool needsRescan = false;
    do {
        if (needsRescan) {
            cfg = CFGGenerator().getCFG(f);
            this->computeDataFlow(&cfg);
            needsRescan = false;
        }

        updated = false;
        for (BasicBlock& b : cfg.fBlocks) {
            DefinitionMap definitions = b.fBefore;

            for (auto iter = b.fNodes.begin(); iter != b.fNodes.end() && !needsRescan; ++iter) {
                if (iter->fKind == BasicBlock::Node::kExpression_Kind) {
                    Expression* expr = iter->fExpression->get();
                    ASSERT(expr);
                    if (iter->fConstantPropagation) {
                        std::unique_ptr<Expression> optimized = expr->constantPropagate(
                                                                                      *fIRGenerator,
                                                                                      definitions);
                        if (optimized) {
                            if (!try_replace_expression(&b, &iter, std::move(optimized))) {
                                needsRescan = true;
                            }
                            ASSERT(iter->fKind == BasicBlock::Node::kExpression_Kind);
                            expr = iter->fExpression->get();
                            updated = true;
                        }
                    }
                    switch (expr->fKind) {
                        case Expression::kVariableReference_Kind: {
                            const Variable& var = ((VariableReference*) expr)->fVariable;
                            if (var.fStorage == Variable::kLocal_Storage &&
                                !definitions[&var] &&
                                undefinedVariables.find(&var) == undefinedVariables.end()) {
                                undefinedVariables.insert(&var);
                                this->error(expr->fPosition,
                                            "'" + var.fName + "' has not been assigned");
                            }
                            break;
                        }
                        case Expression::kTernary_Kind: {
                            TernaryExpression* t = (TernaryExpression*) expr;
                            if (t->fTest->fKind == Expression::kBoolLiteral_Kind) {
                                // ternary has a constant test, replace it with either the true or
                                // false branch
                                if (((BoolLiteral&) *t->fTest).fValue) {
                                    *iter->fExpression = std::move(t->fIfTrue);
                                } else {
                                    *iter->fExpression = std::move(t->fIfFalse);
                                }
                                updated = true;
                                needsRescan = true;
                            }
                            break;
                        }
                        default:
                            break;
                    }
                } else {
                    ASSERT(iter->fKind == BasicBlock::Node::kStatement_Kind);
                    Statement* stmt = iter->fStatement->get();
                    switch (stmt->fKind) {
                        case Statement::kVarDeclarations_Kind: {
                            VarDeclarations& vd = *((VarDeclarationsStatement&) *stmt).fDeclaration;
                            for (auto varIter = vd.fVars.begin(); varIter != vd.fVars.end(); ) {
                                const auto& varDecl = **varIter;
                                if (varDecl.fVar->dead() &&
                                    (!varDecl.fValue ||
                                     !varDecl.fValue->hasSideEffects())) {
                                    if (varDecl.fValue) {
                                        ASSERT(iter->fKind == BasicBlock::Node::kStatement_Kind &&
                                               iter->fStatement->get() == stmt);
                                        if (!b.tryRemoveExpressionBefore(
                                                                        &iter,
                                                                        varDecl.fValue.get())) {
                                            needsRescan = true;
                                        }
                                    }
                                    varIter = vd.fVars.erase(varIter);
                                    updated = true;
                                } else {
                                    ++varIter;
                                }
                            }
                            if (vd.fVars.size() == 0) {
                                iter->fStatement->reset(new Nop());
                            }
                            break;
                        }
                        case Statement::kIf_Kind: {
                            IfStatement& i = (IfStatement&) *stmt;
                            if (i.fIfFalse && i.fIfFalse->isEmpty()) {
                                // else block doesn't do anything, remove it
                                i.fIfFalse.reset();
                                updated = true;
                                needsRescan = true;
                            }
                            if (!i.fIfFalse && i.fIfTrue->isEmpty()) {
                                // if block doesn't do anything, no else block
                                if (i.fTest->hasSideEffects()) {
                                    // test has side effects, keep it
                                    iter->fStatement->reset(new ExpressionStatement(
                                                                               std::move(i.fTest)));
                                } else {
                                    // no if, no else, no test side effects, kill the whole if
                                    // statement
                                    iter->fStatement->reset(new Nop());
                                }
                                updated = true;
                                needsRescan = true;
                            }
                            break;
                        }
                        case Statement::kExpression_Kind: {
                            ExpressionStatement& e = (ExpressionStatement&) *stmt;
                            ASSERT(iter->fStatement->get() == &e);
                            if (e.fExpression->fKind == Expression::kBinary_Kind) {
                                BinaryExpression& bin = (BinaryExpression&) *e.fExpression;
                                if (dead_assignment(bin)) {
                                    if (!b.tryRemoveExpressionBefore(&iter, &bin)) {
                                        needsRescan = true;
                                    }
                                    if (bin.fRight->hasSideEffects()) {
                                        // still have to evaluate the right due to side effects,
                                        // replace the binary expression with just the right side
                                        e.fExpression = std::move(bin.fRight);
                                        if (!b.tryInsertExpression(&iter, &e.fExpression)) {
                                            needsRescan = true;
                                        }
                                    } else {
                                        // no side effects, kill the whole statement
                                        ASSERT(iter->fKind == BasicBlock::Node::kStatement_Kind &&
                                                              iter->fStatement->get() == stmt);
                                        iter->fStatement->reset(new Nop());
                                    }
                                    updated = true;
                                    break;
                                }
                            }
                            if (!e.fExpression->hasSideEffects()) {
                                // Expression statement with no side effects, kill it
                                if (!b.tryRemoveExpressionBefore(&iter, e.fExpression.get())) {
                                    needsRescan = true;
                                }
                                ASSERT(iter->fKind == BasicBlock::Node::kStatement_Kind &&
                                       iter->fStatement->get() == stmt);
                                iter->fStatement->reset(new Nop());
                                updated = true;
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
                this->addDefinitions(*iter, &definitions);
            }
        }
    } while (updated);
    ASSERT(!needsRescan);

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
                                                  const Program::Settings& settings) {
    fErrorText = "";
    fErrorCount = 0;
    fIRGenerator->start(&settings);
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
    auto result = std::unique_ptr<Program>(new Program(kind, settings, defaultPrecision, &fContext,
                                                       std::move(elements),
                                                       fIRGenerator->fSymbolTable,
                                                       fIRGenerator->fInputs));
    fIRGenerator->finish();
    this->writeErrorCount();
    if (fErrorCount) {
        return nullptr;
    }
    return result;
}


bool Compiler::toSPIRV(const Program& program, SkWStream& out) {
    SPIRVCodeGenerator cg(&fContext, &program, this, &out);
    bool result = cg.generateCode();
    this->writeErrorCount();
    return result;
}

bool Compiler::toSPIRV(const Program& program, SkString* out) {
    SkDynamicMemoryWStream buffer;
    bool result = this->toSPIRV(program, buffer);
    if (result) {
        sk_sp<SkData> data(buffer.detachAsData());
        *out = SkString((const char*) data->data(), data->size());
    }
    return result;
}

bool Compiler::toGLSL(const Program& program, SkWStream& out) {
    GLSLCodeGenerator cg(&fContext, &program, this, &out);
    bool result = cg.generateCode();
    this->writeErrorCount();
    return result;
}

bool Compiler::toGLSL(const Program& program, SkString* out) {
    SkDynamicMemoryWStream buffer;
    bool result = this->toGLSL(program, buffer);
    if (result) {
        sk_sp<SkData> data(buffer.detachAsData());
        *out = SkString((const char*) data->data(), data->size());
    }
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

} // namespace
