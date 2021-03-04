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

// This uses templates to define dirtyExpression(), saveState() and setUniform(). Each template can
// reference token names formatted ${name} that are replaced with the actual values passed into the
// functions.
//
// dirtyExpression() and saveState() support the following tokens:
//  - ${newVar} replaced with value of newValueVarName (1st argument)
//  - ${oldVar} replaced with value of oldValueVarName (2nd argument)
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
    UniformCTypeMapper(Layout::CType ctype, const std::vector<String>& skslTypes,
            const String& setUniformSingleFormat, const String& setUniformArrayFormat,
            const String& defaultValue = "", const String& dirtyExpressionFormat = "",
            const String& saveStateFormat = "")
        : UniformCTypeMapper(ctype, skslTypes, setUniformSingleFormat, setUniformArrayFormat,
                true, defaultValue, dirtyExpressionFormat, saveStateFormat) { }

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

    // The C++ type name that this mapper applies to
    Layout::CType ctype() const {
        return fCType;
    }

    // The sksl type names that the mapper's ctype can be mapped to
    const std::vector<String>& supportedTypeNames() const {
        return fSKSLTypes;
    }

    // Whether or not this handler knows how to write state tracking code
    // for the uniform variables
    bool supportsTracking() const {
        return fSupportsTracking;
    }

    // What the C++ class fields are initialized to in the GLSLFragmentProcessor The empty string
    // implies the no-arg constructor is suitable. This is not used if supportsTracking() returns
    // false.
    //
    // The returned snippet will be a valid as the lhs of an assignment.
    const String& defaultValue() const {
        return fDefaultValue;
    }

    // Return a boolean expression that returns true if the variables specified by newValueVarName
    // and oldValueVarName have different values. This is ignored if supportsTracking() returns
    // false.
    //
    // The returned snippet will be a valid expression to be inserted into the condition of an 'if'
    // statement.
    String dirtyExpression(const String& newValueVarName, const String& oldValueVarName) const;

    // Return a statement that stores the value of newValueVarName into the variable specified by
    // oldValueVarName. This is ignored if supportsTracking() returns false.
    //
    // The returned snippet will be a valid expression.
    String saveState(const String& newValueVarName, const String& oldValueVarName) const;

    // Return a statement that invokes the appropriate setX method on the GrGLSLProgramDataManager
    // specified by pdmanName, where the uniform is provided by the expression stored in
    // uniformHandleName, and valueVarName is the variable name pointing to the ctype instance
    // holding the new value.
    //
    // The returned snippet will be a valid expression.
    String setUniform(const String& pdmanName, const String& uniformHandleName,
                      const String& valueVarName) const;

    // True if the setUniform() template only uses the value variable once in its expression. The
    // variable does not necessarily get inlined if this returns true, since a local variable may be
    // needed if state tracking is employed for a particular uniform.
    bool canInlineUniformValue() const {
        return fInlineValue;
    }

private:
    UniformCTypeMapper(Layout::CType ctype, const std::vector<String>& skslTypes,
            const String& setUniformSingleFormat, const String& setUniformArrayFormat,
            bool enableTracking, const String& defaultValue, const String& dirtyExpressionFormat,
            const String& saveStateFormat);

    const UniformCTypeMapper* arrayMapper(int arrayCount) const;

    Layout::CType fCType;
    int fArrayCount = -1;
    std::vector<String> fSKSLTypes;
    String fUniformSingleTemplate;
    String fUniformArrayTemplate;
    bool fInlineValue; // Cached value calculated from fUniformTemplate

    bool fSupportsTracking;
    String fDefaultValue;
    String fDirtyExpressionTemplate;
    String fSaveStateTemplate;
};

}  // namespace SkSL

#endif // defined(SKSL_STANDALONE) || GR_TEST_UTILS

#endif // SkSLUniformCTypes_DEFINED
