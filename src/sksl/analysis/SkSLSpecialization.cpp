/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/analysis/SkSLSpecialization.h"

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkSpan_impl.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#include <algorithm>
#include <memory>
#include <vector>

using namespace skia_private;

namespace SkSL::Analysis {

static bool parameter_mappings_are_equal(const SpecializedParameters& left,
                                         const SpecializedParameters& right) {
    if (left.count() != right.count()) {
        return false;
    }
    for (const auto& [key, leftExpr] : left) {
        const Expression** rightExpr = right.find(key);
        if (!rightExpr) {
            return false;
        }
        if (!Analysis::IsSameExpressionTree(*leftExpr, **rightExpr)) {
            return false;
        }
    }
    return true;
}

void FindFunctionsToSpecialize(const Program& program,
                               SpecializationInfo* info,
                               const ParameterMatchesFn& parameterMatchesFn) {
    class Searcher : public ProgramVisitor {
    public:
        using ProgramVisitor::visitProgramElement;
        using INHERITED = ProgramVisitor;

        Searcher(SpecializationInfo& info, const ParameterMatchesFn& parameterMatchesFn)
                : fSpecializationMap(info.fSpecializationMap)
                , fSpecializedCallMap(info.fSpecializedCallMap)
                , fParameterMatchesFn(parameterMatchesFn) {}

        bool visitExpression(const Expression& expr) override {
            if (expr.is<FunctionCall>()) {
                const FunctionCall& call = expr.as<FunctionCall>();
                const FunctionDeclaration& decl = call.function();

                if (!decl.isIntrinsic()) {
                    SpecializedParameters specialization;

                    const int numParams = decl.parameters().size();
                    SkASSERT(call.arguments().size() == numParams);

                    for (int i = 0; i < numParams; i++) {
                        const Expression& arg = *call.arguments()[i];

                        // Specializations can only be made on arguments that are not complex
                        // expressions but only a variable reference since this reference will
                        // be inlined in the generated specialized functions.
                        if (!arg.is<VariableReference>()) {
                            continue;
                        }

                        const Variable* var = arg.as<VariableReference>().variable();
                        const Variable* param = decl.parameters()[i];

                        // Check that this parameter fits the criteria to create a specialization.
                        if (!fParameterMatchesFn(*param)) {
                            continue;
                        }

                        if (var->storage() == Variable::Storage::kGlobal) {
                            specialization[param] = &arg;
                        } else if (var->storage() == Variable::Storage::kParameter) {
                            const Expression** uniformExpr = fInheritedSpecializations.find(var);
                            SkASSERT(uniformExpr);

                            specialization[param] = *uniformExpr;
                        } else {
                            // TODO(b/353532475): Report an error instead of aborting.
                            SK_ABORT("Specialization requires a uniform or parameter variable");
                        }
                    }

                    // Only create a specialization for this function if there are
                    // variables to specialize on.
                    if (specialization.count() > 0) {
                        Specializations& specializations = fSpecializationMap[&decl];
                        SpecializedCallKey callKey{call.stableID(), fInheritedSpecializationIndex};

                        for (int i = 0; i < specializations.size(); i++) {
                            const SpecializedParameters& entry = specializations[i];
                            if (parameter_mappings_are_equal(specialization, entry)) {
                                // This specialization has already been tracked.
                                fSpecializedCallMap[callKey] = i;
                                return INHERITED::visitExpression(expr);
                            }
                        }

                        // Set the index to the corresponding specialization this function call
                        // requires, also tracking the inherited specialization this function
                        // call is in so the right specialized function can be called.
                        SpecializationIndex specializationIndex = specializations.size();
                        fSpecializedCallMap[callKey] = specializationIndex;
                        specializations.push_back(specialization);

                        // We swap so we don't lose when our last inherited specializations were
                        // once we are done traversing this specific specialization.
                        fInheritedSpecializations.swap(specialization);
                        std::swap(fInheritedSpecializationIndex, specializationIndex);

                        this->visitProgramElement(*decl.definition());

                        std::swap(fInheritedSpecializationIndex, specializationIndex);
                        fInheritedSpecializations.swap(specialization);
                    }
                }
            }
            return INHERITED::visitExpression(expr);
        }

    private:
        SpecializationMap& fSpecializationMap;
        SpecializedCallMap& fSpecializedCallMap;
        const ParameterMatchesFn& fParameterMatchesFn;

        SpecializedParameters fInheritedSpecializations;
        SpecializationIndex fInheritedSpecializationIndex = kUnspecialized;
    };

    for (const std::unique_ptr<ProgramElement>& elem : program.fOwnedElements) {
        if (elem->is<FunctionDefinition>() &&
            elem->as<FunctionDefinition>().declaration().isMain()) {
            // Visit through the program call stack and aggregates any necessary
            // function specializations.
            Searcher(*info, parameterMatchesFn).visitProgramElement(*elem);
            break;
        }
    }
}

SpecializationIndex FindSpecializationIndexForCall(const FunctionCall& call,
                                                   const SpecializationInfo& info,
                                                   SpecializationIndex parentSpecializationIndex) {
    SpecializedCallKey callKey{call.stableID(), parentSpecializationIndex};
    SpecializationIndex* foundIndex = info.fSpecializedCallMap.find(callKey);
    return foundIndex ? *foundIndex : kUnspecialized;
}

SkBitSet FindSpecializedParametersForFunction(const FunctionDeclaration& func,
                                              const SpecializationInfo& info) {
    SkBitSet result(func.parameters().size());
    if (const Specializations* specializations = info.fSpecializationMap.find(&func)) {
        const Analysis::SpecializedParameters& specializedParams = specializations->front();
        const SkSpan<Variable* const> funcParams = func.parameters();

        for (size_t index = 0; index < funcParams.size(); ++index) {
            if (specializedParams.find(funcParams[index])) {
                result.set(index);
            }
        }
    }

    return result;
}

void GetParameterMappingsForFunction(const FunctionDeclaration& func,
                                     const SpecializationInfo& info,
                                     SpecializationIndex specializationIndex,
                                     const ParameterMappingCallback& callback) {
    if (specializationIndex != Analysis::kUnspecialized) {
        if (const Specializations* specializations = info.fSpecializationMap.find(&func)) {
            const Analysis::SpecializedParameters& specializedParams =
                    specializations->at(specializationIndex);
            const SkSpan<Variable* const> funcParams = func.parameters();

            for (size_t index = 0; index < funcParams.size(); ++index) {
                const Variable* param = funcParams[index];
                if (const Expression** expr = specializedParams.find(param)) {
                    callback(index, param, *expr);
                }
            }
        }
    }
}

}  // namespace SkSL::Analysis
