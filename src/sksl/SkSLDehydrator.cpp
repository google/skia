/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLDehydrator.h"

#include "src/sksl/SkSLRehydrator.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBreakStatement.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLContinueStatement.h"
#include "src/sksl/ir/SkSLDiscardStatement.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLEnum.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLField.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLIntLiteral.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLNullLiteral.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSetting.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSwitchCase.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLSymbol.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLUnresolvedFunction.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVarDeclarationsStatement.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLWhileStatement.h"

#ifdef SKSL_STANDALONE

namespace SkSL {

static constexpr int HEADER_SIZE = 2;

class AutoDehydratorSymbolTable {
public:
    AutoDehydratorSymbolTable(Dehydrator* dehydrator, const std::shared_ptr<SymbolTable>& symbols)
        : fDehydrator(dehydrator) {
        dehydrator->fSymbolMap.emplace_back();
        if (symbols) {
            dehydrator->write(*symbols);
        } else {
            dehydrator->writeU8(Rehydrator::kVoid_Command);
        }
    }

    ~AutoDehydratorSymbolTable() {
        fDehydrator->fSymbolMap.pop_back();
    }

private:
    Dehydrator* fDehydrator;
};

void Dehydrator::write(Layout l) {
    if (l == Layout()) {
        this->writeU8(Rehydrator::kDefaultLayout_Command);
    } else if (l == Layout::builtin(l.fBuiltin)) {
        this->writeS8(Rehydrator::kBuiltinLayout_Command);
        this->writeS16(l.fBuiltin);
    } else {
        this->writeS8(Rehydrator::kLayout_Command);
        fBody.write32(l.fFlags);
        this->writeS8(l.fLocation);
        this->writeS8(l.fOffset);
        this->writeS8(l.fBinding);
        this->writeS8(l.fIndex);
        this->writeS8(l.fSet);
        this->writeS16(l.fBuiltin);
        this->writeS8(l.fInputAttachmentIndex);
        this->writeS8((int) l.fFormat);
        this->writeS8(l.fPrimitive);
        this->writeS8(l.fMaxVertices);
        this->writeS8(l.fInvocations);
        this->write(l.fMarker);
        this->write(l.fWhen);
        this->writeS8(l.fKey);
        this->writeS8((int) l.fCType);
    }
}

void Dehydrator::write(Modifiers m) {
    if (m == Modifiers()) {
        this->writeU8(Rehydrator::kDefaultModifiers_Command);
    } else {
        if (m.fFlags <= 255) {
            this->writeU8(Rehydrator::kModifiers8Bit_Command);
            this->write(m.fLayout);
            this->writeU8(m.fFlags);
        } else {
            this->writeU8(Rehydrator::kModifiers_Command);
            this->write(m.fLayout);
            this->writeS32(m.fFlags);
        }
    }
}

void Dehydrator::write(StringFragment s) {
    this->write(String(s));
}

void Dehydrator::write(String s) {
    auto found = fStrings.find(s);
    int offset;
    if (found == fStrings.end()) {
        offset = fStringBuffer.str().length() + HEADER_SIZE;
        fStrings.insert({ s, offset });
        SkASSERT(s.length() <= 255);
        fStringBuffer.write8(s.length());
        fStringBuffer.writeString(s);
    } else {
        offset = found->second;
    }
    this->writeU16(offset);
}

void Dehydrator::write(const Symbol& s) {
    uint16_t id = this->symbolId(&s, false);
    if (id) {
        this->writeU8(Rehydrator::kSymbolRef_Command);
        this->writeU16(id);
        return;
    }
    switch (s.fKind) {
        case Symbol::kFunctionDeclaration_Kind: {
            const FunctionDeclaration& f = s.as<FunctionDeclaration>();
            this->writeU8(Rehydrator::kFunctionDeclaration_Command);
            this->writeId(&f);
            this->write(f.fModifiers);
            this->write(f.fName);
            this->writeU8(f.fParameters.size());
            for (const Variable* p : f.fParameters) {
                this->writeU16(this->symbolId(p));
            }
            this->write(f.fReturnType);
            break;
        }
        case Symbol::kUnresolvedFunction_Kind: {
            const UnresolvedFunction& f = s.as<UnresolvedFunction>();
            this->writeU8(Rehydrator::kUnresolvedFunction_Command);
            this->writeId(&f);
            this->writeU8(f.fFunctions.size());
            for (const FunctionDeclaration* f : f.fFunctions) {
                this->write(*f);
            }
            break;
        }
        case Symbol::kType_Kind: {
            const Type& t = s.as<Type>();
            switch (t.kind()) {
                case Type::kArray_Kind:
                    this->writeU8(Rehydrator::kArrayType_Command);
                    this->writeId(&t);
                    this->write(t.componentType());
                    this->writeU8(t.columns());
                    break;
                case Type::kEnum_Kind:
                    this->writeU8(Rehydrator::kEnumType_Command);
                    this->writeId(&t);
                    this->write(t.fName);
                    break;
                case Type::kNullable_Kind:
                    this->writeU8(Rehydrator::kNullableType_Command);
                    this->writeId(&t);
                    this->write(t.componentType());
                    break;
                case Type::kStruct_Kind:
                    this->writeU8(Rehydrator::kStructType_Command);
                    this->writeId(&t);
                    this->write(t.fName);
                    this->writeU8(t.fields().size());
                    for (const Type::Field& f : t.fields()) {
                        this->write(f.fModifiers);
                        this->write(f.fName);
                        this->write(*f.fType);
                    }
                    break;
                default:
                    this->writeU8(Rehydrator::kSystemType_Command);
                    this->writeId(&t);
                    this->write(t.fName);
            }
            break;
        }
        case Symbol::kVariable_Kind: {
            const Variable& v = s.as<Variable>();
            this->writeU8(Rehydrator::kVariable_Command);
            this->writeId(&v);
            this->write(v.fModifiers);
            this->write(v.fName);
            this->write(v.fType);
            this->writeU8(v.fStorage);
            break;
        }
        case Symbol::kField_Kind: {
            const Field& f = s.as<Field>();
            this->writeU8(Rehydrator::kField_Command);
            this->writeU16(this->symbolId(&f.fOwner));
            this->writeU8(f.fFieldIndex);
            break;
        }
        case Symbol::kExternal_Kind:
            SkASSERT(false);
            break;
    }
}

void Dehydrator::write(const SymbolTable& symbols) {
    this->writeU8(Rehydrator::kSymbolTable_Command);
    this->writeU16(symbols.fOwnedSymbols.size());
    for (const std::unique_ptr<const Symbol>& s : symbols.fOwnedSymbols) {
        this->write(*s);
    }
    this->writeU16(symbols.fSymbols.size());
    std::map<StringFragment, const Symbol*> ordered;
    for (std::pair<StringFragment, const Symbol*> p : symbols.fSymbols) {
        ordered.insert(p);
    }
    for (std::pair<StringFragment, const Symbol*> p : ordered) {
        this->write(p.first);
        bool found = false;
        for (size_t i = 0; i < symbols.fOwnedSymbols.size(); ++i) {
            if (symbols.fOwnedSymbols[i].get() == p.second) {
                this->writeU16(i);
                found = true;
                break;
            }
        }
        SkASSERT(found);
    }
}

void Dehydrator::write(const Expression* e) {
    if (e) {
        switch (e->fKind) {
            case Expression::kBinary_Kind: {
                const BinaryExpression& b = e->as<BinaryExpression>();
                this->writeU8(Rehydrator::kBinary_Command);
                this->write(b.fLeft.get());
                this->writeU8((int) b.fOperator);
                this->write(b.fRight.get());
                this->write(b.fType);
                break;
            }
            case Expression::kBoolLiteral_Kind: {
                const BoolLiteral& b = e->as<BoolLiteral>();
                this->writeU8(Rehydrator::kBoolLiteral_Command);
                this->writeU8(b.fValue);
                break;
            }
            case Expression::kConstructor_Kind: {
                const Constructor& c = e->as<Constructor>();
                this->writeU8(Rehydrator::kConstructor_Command);
                this->write(c.fType);
                this->writeU8(c.fArguments.size());
                for (const auto& a : c.fArguments) {
                    this->write(a.get());
                }
                break;
            }
            case Expression::kExternalFunctionCall_Kind:
            case Expression::kExternalValue_Kind:
                // not implemented; doesn't seem like we'll ever need them from within an include
                // file
                SkASSERT(false);
                break;
            case Expression::kFieldAccess_Kind: {
                const FieldAccess& f = e->as<FieldAccess>();
                this->writeU8(Rehydrator::kFieldAccess_Command);
                this->write(f.fBase.get());
                this->writeU8(f.fFieldIndex);
                this->writeU8(f.fOwnerKind);
                break;
            }
            case Expression::kFloatLiteral_Kind: {
                const FloatLiteral& f = e->as<FloatLiteral>();
                this->writeU8(Rehydrator::kFloatLiteral_Command);
                FloatIntUnion u;
                u.fFloat = f.fValue;
                this->writeS32(u.fInt);
                break;
            }
            case Expression::kFunctionCall_Kind: {
                const FunctionCall& f = e->as<FunctionCall>();
                this->writeU8(Rehydrator::kFunctionCall_Command);
                this->write(f.fType);
                this->writeId(&f.fFunction);
                this->writeU8(f.fArguments.size());
                for (const auto& a : f.fArguments) {
                    this->write(a.get());
                }
                break;
            }
            case Expression::kIndex_Kind: {
                const IndexExpression& i = e->as<IndexExpression>();
                this->writeU8(Rehydrator::kIndex_Command);
                this->write(i.fBase.get());
                this->write(i.fIndex.get());
                break;
            }
            case Expression::kIntLiteral_Kind: {
                const IntLiteral& i = e->as<IntLiteral>();
                this->writeU8(Rehydrator::kIntLiteral_Command);
                this->writeS32(i.fValue);
                break;
            }
            case Expression::kNullLiteral_Kind:
                this->writeU8(Rehydrator::kNullLiteral_Command);
                break;
            case Expression::kPostfix_Kind: {
                const PostfixExpression& p = e->as<PostfixExpression>();
                this->writeU8(Rehydrator::kPostfix_Command);
                this->writeU8((int) p.fOperator);
                this->write(p.fOperand.get());
                break;
            }
            case Expression::kPrefix_Kind: {
                const PrefixExpression& p = e->as<PrefixExpression>();
                this->writeU8(Rehydrator::kPrefix_Command);
                this->writeU8((int) p.fOperator);
                this->write(p.fOperand.get());
                break;
            }
            case Expression::kSetting_Kind: {
                const Setting& s = e->as<Setting>();
                this->writeU8(Rehydrator::kSetting_Command);
                this->write(s.fName);
                this->write(s.fValue.get());
                break;
            }
            case Expression::kSwizzle_Kind: {
                const Swizzle& s = e->as<Swizzle>();
                this->writeU8(Rehydrator::kSwizzle_Command);
                this->write(s.fBase.get());
                this->writeU8(s.fComponents.size());
                for (int c : s.fComponents) {
                    this->writeU8(c);
                }
                break;
            }
            case Expression::kTernary_Kind: {
                const TernaryExpression& t = e->as<TernaryExpression>();
                this->writeU8(Rehydrator::kTernary_Command);
                this->write(t.fTest.get());
                this->write(t.fIfTrue.get());
                this->write(t.fIfFalse.get());
                break;
            }
            case Expression::kVariableReference_Kind: {
                const VariableReference& v = e->as<VariableReference>();
                this->writeU8(Rehydrator::kVariableReference_Command);
                this->writeId(&v.fVariable);
                this->writeU8(v.fRefKind);
                break;
            }
            case Expression::kFunctionReference_Kind:
            case Expression::kTypeReference_Kind:
            case Expression::kDefined_Kind:
                // shouldn't appear in finished code
                SkASSERT(false);
                break;
        }
    } else {
        this->writeU8(Rehydrator::kVoid_Command);
    }
}

void Dehydrator::write(const Statement* s) {
    if (s) {
        switch (s->fKind) {
            case Statement::kBlock_Kind: {
                const Block& b = s->as<Block>();
                this->writeU8(Rehydrator::kBlock_Command);
                AutoDehydratorSymbolTable symbols(this, b.fSymbols);
                this->writeU8(b.fStatements.size());
                for (const auto& s : b.fStatements) {
                    this->write(s.get());
                }
                this->writeU8(b.fIsScope);
                break;
            }
            case Statement::kBreak_Kind:
                this->writeU8(Rehydrator::kBreak_Command);
                break;
            case Statement::kContinue_Kind:
                this->writeU8(Rehydrator::kContinue_Command);
                break;
            case Statement::kDiscard_Kind:
                this->writeU8(Rehydrator::kDiscard_Command);
                break;
            case Statement::kDo_Kind: {
                const DoStatement& d = s->as<DoStatement>();
                this->writeU8(Rehydrator::kDo_Command);
                this->write(d.fStatement.get());
                this->write(d.fTest.get());
                break;
            }
            case Statement::kExpression_Kind: {
                const ExpressionStatement& e = s->as<ExpressionStatement>();
                this->writeU8(Rehydrator::kExpressionStatement_Command);
                this->write(e.fExpression.get());
                break;
            }
            case Statement::kFor_Kind: {
                const ForStatement& f = s->as<ForStatement>();
                this->writeU8(Rehydrator::kFor_Command);
                this->write(f.fInitializer.get());
                this->write(f.fTest.get());
                this->write(f.fNext.get());
                this->write(f.fStatement.get());
                this->write(f.fSymbols);
                break;
            }
            case Statement::kIf_Kind: {
                const IfStatement& i = s->as<IfStatement>();
                this->writeU8(Rehydrator::kIf_Command);
                this->writeU8(i.fIsStatic);
                this->write(i.fTest.get());
                this->write(i.fIfTrue.get());
                this->write(i.fIfFalse.get());
                break;
            }
            case Statement::kNop_Kind:
                SkASSERT(false);
                break;
            case Statement::kReturn_Kind: {
                const ReturnStatement& r = s->as<ReturnStatement>();
                this->writeU8(Rehydrator::kReturn_Command);
                this->write(r.fExpression.get());
                break;
            }
            case Statement::kSwitch_Kind: {
                const SwitchStatement& ss = s->as<SwitchStatement>();
                this->writeU8(Rehydrator::kSwitch_Command);
                this->writeU8(ss.fIsStatic);
                AutoDehydratorSymbolTable symbols(this, ss.fSymbols);
                this->write(ss.fValue.get());
                this->writeU8(ss.fCases.size());
                for (const auto& sc : ss.fCases) {
                    this->write(sc->fValue.get());
                    this->writeU8(sc->fStatements.size());
                    for (const auto& stmt : sc->fStatements) {
                        this->write(stmt.get());
                    }
                }
                break;
            }
            case Statement::kVarDeclaration_Kind: {
                const VarDeclaration& v = s->as<VarDeclaration>();
                this->writeU8(Rehydrator::kVarDeclaration_Command);
                this->writeU16(this->symbolId(v.fVar));
                this->writeU8(v.fSizes.size());
                for (const auto& s : v.fSizes) {
                    this->write(s.get());
                }
                this->write(v.fValue.get());
                break;
            }
            case Statement::kVarDeclarations_Kind: {
                const VarDeclarationsStatement& v = s->as<VarDeclarationsStatement>();
                this->write(*v.fDeclaration);
                break;
            }
            case Statement::kWhile_Kind: {
                const WhileStatement& w = s->as<WhileStatement>();
                this->writeU8(Rehydrator::kWhile_Command);
                this->write(w.fTest.get());
                this->write(w.fStatement.get());
                break;
            }
        }
    } else {
        this->writeU8(Rehydrator::kVoid_Command);
    }
}

void Dehydrator::write(const ProgramElement& e) {
    switch (e.fKind) {
        case ProgramElement::kEnum_Kind: {
            const Enum& en = e.as<Enum>();
            this->writeU8(Rehydrator::kEnum_Command);
            this->write(en.fTypeName);
            AutoDehydratorSymbolTable symbols(this, en.fSymbols);
            for (const auto& s : en.fSymbols->fOwnedSymbols) {
                SkASSERT(s->fKind == Symbol::kVariable_Kind);
                Variable& v = (Variable&) *s;
                SkASSERT(v.fInitialValue);
                const IntLiteral& i = v.fInitialValue->as<IntLiteral>();
                this->writeS32(i.fValue);
            }
            break;
        }
        case ProgramElement::kExtension_Kind:
            SkASSERT(false);
            break;
        case ProgramElement::kFunction_Kind: {
            const FunctionDefinition& f = e.as<FunctionDefinition>();
            this->writeU8(Rehydrator::kFunctionDefinition_Command);
            this->writeU16(this->symbolId(&f.fDeclaration));
            this->write(f.fBody.get());
            this->writeU8(f.fReferencedIntrinsics.size());
            std::set<uint16_t> ordered;
            for (const FunctionDeclaration* ref : f.fReferencedIntrinsics) {
                ordered.insert(this->symbolId(ref));
            }
            for (uint16_t ref : ordered) {
                this->writeU16(ref);
            }
            break;
        }
        case ProgramElement::kInterfaceBlock_Kind: {
            const InterfaceBlock& i = e.as<InterfaceBlock>();
            this->writeU8(Rehydrator::kInterfaceBlock_Command);
            this->write(i.fVariable);
            this->write(i.fTypeName);
            this->write(i.fInstanceName);
            this->writeU8(i.fSizes.size());
            for (const auto& s : i.fSizes) {
                this->write(s.get());
            }
            break;
        }
        case ProgramElement::kModifiers_Kind:
            SkASSERT(false);
            break;
        case ProgramElement::kSection_Kind:
            SkASSERT(false);
            break;
        case ProgramElement::kVar_Kind: {
            const VarDeclarations& v = e.as<VarDeclarations>();
            this->writeU8(Rehydrator::kVarDeclarations_Command);
            this->write(v.fBaseType);
            this->writeU8(v.fVars.size());
            for (const auto& v : v.fVars) {
                this->write(v.get());
            }
            break;
        }
    }
}

void Dehydrator::write(const std::vector<std::unique_ptr<ProgramElement>>& elements) {
    this->writeU8(Rehydrator::kElements_Command);
    this->writeU8(elements.size());
    for (const auto& e : elements) {
        this->write(*e);
    }
}

void Dehydrator::finish(OutputStream& out) {
    out.write16(fStringBuffer.str().size());
    out.writeString(fStringBuffer.str());
    out.writeString(fBody.str());
}

} // namespace

#endif
