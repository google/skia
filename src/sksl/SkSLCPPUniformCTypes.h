/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSLUniformCTypes_DEFINED
#define SkSLUniformCTypes_DEFINED

#include "include/private/SkSLString.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariable.h"

#if defined(SKSL_STANDALONE) || GR_TEST_UTILS

namespace SkSL {

// This uses templates to define setUniform(). The template can reference token names formatted
// ${name} that are replaced with the actual values passed into the function.
//
// setUniform() supports these tokens:
//  - ${pdman} replaced with value of pdmanName (1st argument)
//  - ${uniform} replaced with value of uniformHandleName (2nd argument)
//  - ${var} replaced with value of valueVarName (3rd argument)
//
// All templates and C++ snippets should produce valid expressions, but do not need to include
// semicolons or newlines, which will be handled by the code generation itself.
class UniformCTypeMapper {
public:
    UniformCTypeMapper(const char* setUniformTemplate) : fSetUniformTemplate(setUniformTemplate) {}

    // Returns nullptr if the type and layout are not supported; the returned pointer's ownership
    // is not transfered to the caller.
    //
    // The returned mapper can support tracking even if tracking is disabled based on the flags in
    // the layout.
    static const UniformCTypeMapper* Get(const Context& context, const Type& type,
                                         const Layout& layout);

    static const UniformCTypeMapper* Get(const Context& context, const Variable& variable) {
        return Get(context, variable.type(), variable.modifiers().fLayout);
    }

    // Return a statement that invokes the appropriate setX method on the GrGLSLProgramDataManager
    // specified by pdmanName, where the uniform is provided by the expression stored in
    // uniformHandleName, and valueVarName is the variable name pointing to the ctype instance
    // holding the new value.
    //
    // The returned snippet will be a valid expression.
    String setUniform(const String& pdmanName, const String& uniformHandleName,
                      const String& valueVarName) const;

private:
    String fSetUniformTemplate;
};

}  // namespace SkSL

#endif // defined(SKSL_STANDALONE) || GR_TEST_UTILS

#endif // SkSLUniformCTypes_DEFINED
