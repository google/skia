/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/transform/SkSLTransform.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLIntrinsicMap.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

namespace SkSL {

namespace Transform {

void FindAndDeclareBuiltinVariables(const Context& context,
        ProgramKind programKind, std::vector<const ProgramElement*>& sharedElements) {
    class BuiltinVariableScanner : public ProgramVisitor {
    public:
        BuiltinVariableScanner(const Context& context)
            : fContext(context) {}

        void addDeclaringElement(const String& name) {
            // If this is the *first* time we've seen this builtin, findAndInclude will return
            // the corresponding ProgramElement.
            IntrinsicMap& intrinsics = *fContext.fIntrinsics;
            if (const ProgramElement* decl = intrinsics.findAndInclude(name)) {
                SkASSERT(decl->is<GlobalVarDeclaration>() || decl->is<InterfaceBlock>());
                fNewElements.push_back(decl);
            }
        }

        bool visitProgramElement(const ProgramElement& pe) override {
            if (pe.is<FunctionDefinition>()) {
                const FunctionDefinition& funcDef = pe.as<FunctionDefinition>();
                // We synthesize writes to sk_FragColor if main() returns a color, even if it's
                // otherwise unreferenced. Check main's return type to see if it's half4.
                if (funcDef.declaration().isMain() &&
                    funcDef.declaration().returnType() == *fContext.fTypes.fHalf4) {
                    fPreserveFragColor = true;
                }
            }
            return INHERITED::visitProgramElement(pe);
        }

        bool visitExpression(const Expression& e) override {
            if (e.is<VariableReference>() && e.as<VariableReference>().variable()->isBuiltin()) {
                this->addDeclaringElement(String(e.as<VariableReference>().variable()->name()));
            }
            return INHERITED::visitExpression(e);
        }

        const Context& fContext;
        std::vector<const ProgramElement*> fNewElements;
        bool fPreserveFragColor = false;

        using INHERITED = ProgramVisitor;
        using INHERITED::visitProgramElement;
    };

    BuiltinVariableScanner scanner(context);
    for (auto& e : dsl::DSLWriter::ProgramElements()) {
        scanner.visitProgramElement(*e);
    }

    if (scanner.fPreserveFragColor) {
        // main() returns a half4, so make sure we don't dead-strip sk_FragColor.
        scanner.addDeclaringElement(Compiler::FRAGCOLOR_NAME);
    }

    switch (programKind) {
        case ProgramKind::kFragment:
            // Vulkan requires certain builtin variables be present, even if they're unused. At one
            // time, validation errors would result if sk_Clockwise was missing. Now, it's just
            // (Adreno) driver bugs that drop or corrupt draws if they're missing.
            scanner.addDeclaringElement("sk_Clockwise");
            break;
        default:
            break;
    }

    sharedElements.insert(sharedElements.begin(), scanner.fNewElements.begin(),
            scanner.fNewElements.end());
}

} // namespace Transform

} // namespace SkSL
