/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCPPUniformCTypes.h"
#include "src/sksl/SkSLHCodeGenerator.h"
#include "src/sksl/SkSLStringStream.h"

#include <vector>

namespace SkSL {

/////////////////////////
// Template evaluation //
/////////////////////////

static String eval_template(const String& format, const std::vector<String>& tokens,
                            const std::vector<const String*>& values) {
    StringStream stream;

    int tokenNameStart = -1;
    for (size_t i = 0; i < format.size(); i++) {
        if (tokenNameStart >= 0) {
            // Within a token name so check if it is the end
            if (format[i] == '}') {
                // Skip 2 extra characters at the beginning for the $ and {, which must exist since
                // otherwise tokenNameStart < 0
                String token(format.c_str() + tokenNameStart + 2, i - tokenNameStart - 2);
                // Search for the token in supported list
                bool found = false;
                for (size_t j = 0; j < tokens.size(); j++) {
                    if (token == tokens[j]) {
                        // Found a match so append the value corresponding to j to the output
                        stream.writeText(values[j]->c_str());
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    // Write out original characters as if we didn't consider it to be a token name
                    stream.writeText("${");
                    stream.writeText(token.c_str());
                    stream.writeText("}");
                }

                // And end the token name state
                tokenNameStart = -1;
            }
        } else {
            // Outside of a token name, so check if this character starts a name:
            // i == $ and i+1 == {
            if (i < format.size() - 1 && format[i] == '$' && format[i + 1] == '{') {
                // Begin parsing the token
                tokenNameStart = i;
            } else {
                // Just a character so append it
                stream.write8(format[i]);
            }
        }
    }

    return stream.str();
}

static bool determine_inline_from_template(const String& uniformTemplate) {
    // True if there is at most one instance of the ${var} template matcher in fUniformTemplate.
    int firstMatch = uniformTemplate.find("${var}");

    if (firstMatch < 0) {
        // Template doesn't use the value variable at all, so it can "inlined"
        return true;
    }

    // Check for another occurrence of ${var}, after firstMatch + 6
    int secondMatch = uniformTemplate.find("${var}", firstMatch + strlen("${var}"));
    // If there's no second match, then the value can be inlined in the c++ code
    return secondMatch < 0;
}

///////////////////////////////////////
// UniformCTypeMapper implementation //
///////////////////////////////////////

String UniformCTypeMapper::dirtyExpression(const String& newVar, const String& oldVar) const {
    if (fSupportsTracking) {
        std::vector<String> tokens = { "newVar", "oldVar" };
        std::vector<const String*> values = { &newVar, &oldVar };
        return eval_template(fDirtyExpressionTemplate, tokens, values);
    } else {
        return "";
    }
}

String UniformCTypeMapper::saveState(const String& newVar, const String& oldVar) const {
    if (fSupportsTracking) {
        std::vector<String> tokens = { "newVar", "oldVar" };
        std::vector<const String*> values = { &newVar, &oldVar };
        return eval_template(fSaveStateTemplate, tokens, values);
    } else {
        return "";
    }
}

String UniformCTypeMapper::setUniform(const String& pdman, const String& uniform,
                                      const String& var) const {
    std::vector<String> tokens = { "pdman", "uniform", "var" };
    std::vector<const String*> values = { &pdman, &uniform, &var };
    return eval_template(fUniformTemplate, tokens, values);
}

UniformCTypeMapper::UniformCTypeMapper(
        Layout::CType ctype, const std::vector<String>& skslTypes, const String& setUniformFormat,
        bool enableTracking, const String& defaultValue, const String& dirtyExpressionFormat,
        const String& saveStateFormat)
    : fCType(ctype)
    , fSKSLTypes(skslTypes)
    , fUniformTemplate(setUniformFormat)
    , fInlineValue(determine_inline_from_template(setUniformFormat))
    , fSupportsTracking(enableTracking)
    , fDefaultValue(defaultValue)
    , fDirtyExpressionTemplate(dirtyExpressionFormat)
    , fSaveStateTemplate(saveStateFormat) { }

// NOTE: These would be macros, but C++ initialization lists for the sksl type names do not play
// well with macro parsing.

static UniformCTypeMapper REGISTER(Layout::CType ctype, const std::vector<String>& skslTypes,
                                   const char* uniformFormat, const char* defaultValue,
                                   const char* dirtyExpression) {
    return UniformCTypeMapper(ctype, skslTypes, uniformFormat, defaultValue, dirtyExpression,
                              "${oldVar} = ${newVar}");
}

static UniformCTypeMapper REGISTER(Layout::CType ctype, const std::vector<String>& skslTypes,
                                   const char* uniformFormat, const char* defaultValue) {
    return REGISTER(ctype, skslTypes, uniformFormat, defaultValue,
                    "${oldVar} != ${newVar}");
}

//////////////////////////////
// Currently defined ctypes //
//////////////////////////////

static const std::vector<UniformCTypeMapper>& get_mappers() {
    static const std::vector<UniformCTypeMapper> registeredMappers = {
    REGISTER(Layout::CType::kSkRect, { "half4", "float4", "double4" },
        "${pdman}.set4fv(${uniform}, 1, reinterpret_cast<const float*>(&${var}))", // to gpu
        "SkRect::MakeEmpty()",                                                     // default value
        "${oldVar}.isEmpty() || ${oldVar} != ${newVar}"),                          // dirty check

    REGISTER(Layout::CType::kSkIRect, { "int4", "short4", "byte4" },
        "${pdman}.set4iv(${uniform}, 1, reinterpret_cast<const int*>(&${var}))",   // to gpu
        "SkIRect::MakeEmpty()",                                                    // default value
        "${oldVar}.isEmpty() || ${oldVar} != ${newVar}"),                          // dirty check

    REGISTER(Layout::CType::kSkPMColor4f, { "half4", "float4", "double4" },
        "${pdman}.set4fv(${uniform}, 1, ${var}.vec())",                            // to gpu
        "{SK_FloatNaN, SK_FloatNaN, SK_FloatNaN, SK_FloatNaN}"),                   // default value

    REGISTER(Layout::CType::kSkVector4, { "half4", "float4", "double4" },
        "${pdman}.set4fv(${uniform}, 1, ${var}.fData)",                            // to gpu
        "SkVector4(SK_MScalarNaN, SK_MScalarNaN, SK_MScalarNaN, SK_MScalarNaN)",   // default value
        "${oldVar} != (${newVar})"),                                               // dirty check

    REGISTER(Layout::CType::kSkPoint, { "half2", "float2", "double2" } ,
        "${pdman}.set2f(${uniform}, ${var}.fX, ${var}.fY)",                        // to gpu
        "SkPoint::Make(SK_FloatNaN, SK_FloatNaN)"),                                // default value

    REGISTER(Layout::CType::kSkIPoint, { "int2", "short2", "byte2" },
        "${pdman}.set2i(${uniform}, ${var}.fX, ${var}.fY)",                        // to gpu
        "SkIPoint::Make(SK_NaN32, SK_NaN32)"),                                     // default value

    REGISTER(Layout::CType::kSkMatrix, { "half3x3", "float3x3", "double3x3" },
        "${pdman}.setSkMatrix(${uniform}, ${var})",                                // to gpu
        "SkMatrix::MakeScale(SK_FloatNaN)",                                        // default value
        "!${oldVar}.cheapEqualTo(${newVar})"),                                     // dirty check

    REGISTER(Layout::CType::kSkMatrix44,  { "half4x4", "float4x4", "double4x4" },
        "${pdman}.setSkMatrix44(${uniform}, ${var})",                              // to gpu
        "SkMatrix44(SkMatrix44::kNaN_Constructor)",                                // default value
        "${oldVar} != (${newVar})"),                                               // dirty check

    REGISTER(Layout::CType::kFloat,  { "half", "float", "double" },
        "${pdman}.set1f(${uniform}, ${var})",                                      // to gpu
        "SK_FloatNaN"),                                                            // default value

    REGISTER(Layout::CType::kInt32, { "int", "short", "byte" },
        "${pdman}.set1i(${uniform}, ${var})",                                      // to gpu
        "SK_NaN32"),                                                               // default value
    };

    return registeredMappers;
}

/////

// Greedy search through registered handlers for one that has a matching
// ctype and supports the sksl type of the variable.
const UniformCTypeMapper* UniformCTypeMapper::Get(const Context& context, const Type& type,
                                                  const Layout& layout) {
    const std::vector<UniformCTypeMapper>& registeredMappers = get_mappers();

    Layout::CType ctype = layout.fCType;
    // If there's no custom ctype declared in the layout, use the default type mapping
    if (ctype == Layout::CType::kDefault) {
        ctype = HCodeGenerator::ParameterCType(context, type, layout);
    }

    const String& skslType = type.name();

    for (size_t i = 0; i < registeredMappers.size(); i++) {
        if (registeredMappers[i].ctype() == ctype) {
            // Check for sksl support, since some c types (e.g. SkMatrix) can be used in multiple
            // uniform types and send data to the gpu differently in those conditions
            const std::vector<String> supportedSKSL = registeredMappers[i].supportedTypeNames();
            for (size_t j = 0; j < supportedSKSL.size(); j++) {
                if (supportedSKSL[j] == skslType) {
                    // Found a match, so return it or an explicitly untracked version if tracking is
                    // disabled in the layout
                    return &registeredMappers[i];
                }
            }
        }
    }

    // Didn't find a match
    return nullptr;
}

} // namespace
