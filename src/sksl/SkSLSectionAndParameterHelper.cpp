/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLSectionAndParameterHelper.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"

namespace SkSL {

SectionAndParameterHelper::SectionAndParameterHelper(const Program* program, ErrorReporter& errors)
    : fProgram(*program) {
    for (const ProgramElement* p : fProgram.elements()) {
        switch (p->kind()) {
            case ProgramElement::Kind::kGlobalVar: {
                const VarDeclaration& decl =
                                  p->as<GlobalVarDeclaration>().declaration()->as<VarDeclaration>();
                if (IsParameter(decl.var())) {
                    fParameters.push_back(&decl.var());
                }
                break;
            }
            case ProgramElement::Kind::kSection: {
                const Section& s = p->as<Section>();
                const String& name = s.name();
                const String& arg = s.argument();
                if (IsSupportedSection(name.c_str())) {
                    if (SectionRequiresArgument(name.c_str()) && !arg.size()) {
                        errors.error(s.fOffset,
                                     ("section '@" + name +
                                      "' requires one parameter").c_str());
                    }
                    if (!SectionAcceptsArgument(name.c_str()) && arg.size()) {
                        errors.error(s.fOffset,
                                     ("section '@" + name + "' has no parameters").c_str());
                    }
                } else {
                    errors.error(s.fOffset,
                                 ("unsupported section '@" + name + "'").c_str());
                }
                if (!SectionPermitsDuplicates(name.c_str()) &&
                        fSections.find(name) != fSections.end()) {
                    errors.error(s.fOffset,
                                 ("duplicate section '@" + name + "'").c_str());
                }
                fSections[name].push_back(&s);
                break;
            }
            default:
                break;
        }
    }
}

}  // namespace SkSL
