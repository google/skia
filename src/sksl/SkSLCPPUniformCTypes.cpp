/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCPPUniformCTypes.h"

#include "include/private/SkMutex.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/codegen/SkSLHCodeGenerator.h"

#include <unordered_map>

#if defined(SKSL_STANDALONE) || GR_TEST_UTILS

namespace SkSL {

/////////////////////////
// Template evaluation //
/////////////////////////

static String eval_template(const String& format,
                            std::initializer_list<String> tokens,
                            std::initializer_list<String> replacements) {
    SkASSERT(tokens.size() == replacements.size());
    String str = format;

    // Replace every token with its replacement.
    auto tokenIter = tokens.begin();
    auto replacementIter = replacements.begin();
    for (; tokenIter != tokens.end(); ++tokenIter, ++replacementIter) {
        size_t position = 0;
        for (;;) {
            // Replace one instance of the current token with the requested replacement.
            position = str.find(*tokenIter, position);
            if (position == String::npos) {
                break;
            }
            str.replace(position, tokenIter->size(), *replacementIter);
            position += replacementIter->size();
        }
    }

    return str;
}

///////////////////////////////////////
// UniformCTypeMapper implementation //
///////////////////////////////////////

String UniformCTypeMapper::setUniform(const String& pdman, const String& uniform,
                                      const String& var) const {
    return eval_template(fSetUniformTemplate,
                         {"${pdman}", "${uniform}", "${var}"},
                         {pdman, uniform, var});
}

const UniformCTypeMapper* UniformCTypeMapper::Get(const Context& context, const Type& type,
                                                  const Layout& layout) {
    SkASSERT(!type.isArray());

    static const auto* kRegisteredMappers =
            new std::unordered_map<Layout::CType, UniformCTypeMapper>{
                    {Layout::CType::kSkRect,
                     "${pdman}.set4fv(${uniform}, 1, reinterpret_cast<const float*>(&${var}))"},
                    {Layout::CType::kSkIRect,
                     "${pdman}.set4iv(${uniform}, 1, reinterpret_cast<const int*>(&${var}))"},
                    {Layout::CType::kSkPMColor4f, "${pdman}.set4fv(${uniform}, 1, ${var}.vec())"},
                    {Layout::CType::kSkV4, "${pdman}.set4fv(${uniform}, 1, ${var}.ptr())"},
                    {Layout::CType::kSkPoint, "${pdman}.set2f(${uniform}, ${var}.fX, ${var}.fY)"},
                    {Layout::CType::kSkIPoint, "${pdman}.set2i(${uniform}, ${var}.fX, ${var}.fY)"},
                    {Layout::CType::kSkMatrix, "${pdman}.setSkMatrix(${uniform}, ${var})"},
                    {Layout::CType::kSkM44, "${pdman}.setSkM44(${uniform}, ${var})"},
                    {Layout::CType::kFloat, "${pdman}.set1f(${uniform}, ${var})"},
                    {Layout::CType::kInt32, "${pdman}.set1i(${uniform}, ${var})"},
            };

    Layout::CType ctype = layout.fCType;
    // If there's no custom ctype declared in the layout, use the default type mapping
    if (ctype == Layout::CType::kDefault) {
        ctype = HCodeGenerator::ParameterCType(context, type, layout);
    }

    auto it = kRegisteredMappers->find(ctype);
    return it == kRegisteredMappers->end() ? nullptr : &it->second;
}

}  // namespace SkSL

#endif // defined(SKSL_STANDALONE) || GR_TEST_UTILS
