/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLModule.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/transform/SkSLProgramWriter.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

using namespace skia_private;

namespace SkSL {

void Transform::ReplaceConstVarsWithLiterals(Module& module, ProgramUsage* usage) {
    class ConstVarReplacer : public ProgramWriter {
    public:
        ConstVarReplacer(ProgramUsage* usage) : fUsage(usage) {}

        using ProgramWriter::visitProgramElement;

        bool visitExpressionPtr(std::unique_ptr<Expression>& expr) override {
            // If this is a variable...
            if (expr->is<VariableReference>()) {
                VariableReference& var = expr->as<VariableReference>();
                // ... and it's a candidate for size reduction...
                if (fCandidates.contains(var.variable())) {
                    // ... get its constant value...
                    if (const Expression* value = ConstantFolder::GetConstantValueOrNull(var)) {
                        // ... and replace it with that value.
                        fUsage->remove(expr.get());
                        expr = value->clone();
                        fUsage->add(expr.get());
                        return false;
                    }
                }
            }
            return INHERITED::visitExpressionPtr(expr);
        }

        ProgramUsage* fUsage;
        THashSet<const Variable*> fCandidates;

        using INHERITED = ProgramWriter;
    };

    ConstVarReplacer visitor{usage};

    for (const auto& [var, count] : usage->fVariableCounts) {
        // We can only replace const variables that still exist, and that have initial values.
        if (!count.fVarExists || count.fWrite != 1) {
            continue;
        }
        if (!var->modifierFlags().isConst()) {
            continue;
        }
        if (!var->initialValue()) {
            continue;
        }
        // The current size is:
        //   strlen("const type varname=initialvalue;`") + count*strlen("varname").
        size_t initialvalueSize = ConstantFolder::GetConstantValueForVariable(*var->initialValue())
                                          ->description()
                                          .size();
        size_t totalOldSize = var->description().size() +        // const type varname
                              1 +                                // =
                              initialvalueSize +                 // initialvalue
                              1 +                                // ;
                              count.fRead * var->name().size();  // count * varname
        // If we replace varname with initialvalue everywhere, the new size would be:
        //   count*strlen("initialvalue")
        size_t totalNewSize = count.fRead * initialvalueSize;    // count * initialvalue

        if (totalNewSize <= totalOldSize) {
            visitor.fCandidates.add(var);
        }
    }

    if (!visitor.fCandidates.empty()) {
        for (std::unique_ptr<ProgramElement>& pe : module.fElements) {
            if (pe->is<FunctionDefinition>()) {
                visitor.visitProgramElement(*pe);
            }
        }
    }
}

}  // namespace SkSL
