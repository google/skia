/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VARDECLARATIONS
#define SKSL_VARDECLARATIONS

#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLVariable.h"

namespace SkSL {

/**
 * A single variable declaration within a var declaration statement. For instance, the statement
 * 'int x = 2, y[3];' is a VarDeclarations statement containing two individual VarDeclaration
 * instances.
 */
struct VarDeclaration : public Statement {
    VarDeclaration(IRGenerator* irGenerator, IRNode::ID var, std::vector<IRNode::ID> sizes,
                   IRNode::ID value)
    : INHERITED(irGenerator, var.node().fOffset, Statement::kVarDeclaration_Kind)
    , fVar(var)
    , fSizes(std::move(sizes))
    , fValue(value) {}

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new VarDeclaration(fIRGenerator, fVar, fSizes, fValue));
    }

    String description() const override {
        String result = ((const Variable&) fVar.node()).fName;
        for (const auto& size : fSizes) {
            if (size) {
                result += "[" + size.node().description() + "]";
            } else {
                result += "[]";
            }
        }
        if (fValue) {
            result += " = " + fValue.node().description();
        }
        return result;
    }

    IRNode::ID fVar;
    std::vector<IRNode::ID> fSizes;
    IRNode::ID fValue;

    typedef Statement INHERITED;
};

/**
 * A variable declaration statement, which may consist of one or more individual variables.
 */
struct VarDeclarations : public ProgramElement {
    VarDeclarations(IRGenerator* irGenerator, int offset, IRNode::ID baseType,
                    std::vector<IRNode::ID> vars)
    : INHERITED(irGenerator, offset, kVar_Kind)
    , fBaseType(baseType)
    , fVars(std::move(vars)) {}

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new VarDeclarations(fIRGenerator, fOffset, fBaseType,
                                                            fVars));
    }

    String description() const override {
        if (!fVars.size()) {
            return String();
        }
        VarDeclaration& decl = (VarDeclaration&) fVars[0].node();
        String result = ((Variable&) decl.fVar.node()).fModifiers.description() +
                        fBaseType.node().description() + " ";
        String separator;
        for (const auto& var : fVars) {
            result += separator;
            separator = ", ";
            result += var.node().description();
        }
        return result;
    }

    IRNode::ID fBaseType;
    std::vector<IRNode::ID> fVars;

    typedef ProgramElement INHERITED;
};

} // namespace

#endif
