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

#include <map>
#include <vector>

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
    return eval_template(fDirtyExpressionTemplate, {"${newVar}", "${oldVar}"}, {newVar, oldVar});
}

String UniformCTypeMapper::saveState(const String& newVar, const String& oldVar) const {
    return eval_template(fSaveStateTemplate, {"${newVar}", "${oldVar}"}, {newVar, oldVar});
}

String UniformCTypeMapper::setUniform(const String& pdman, const String& uniform,
                                      const String& var) const {
    String count;
    String finalVar;
    const String* activeTemplate;
    if (fArrayCount != -1) {
        count = to_string(fArrayCount);
        finalVar = var + "[0]";
        activeTemplate = &fUniformArrayTemplate;
    } else {
        count = "1";
        finalVar = std::move(var);
        activeTemplate = &fUniformSingleTemplate;
    }

    return eval_template(*activeTemplate,
                         {"${pdman}", "${uniform}", "${var}", "${count}"},
                         {pdman, uniform, finalVar, count});
}

UniformCTypeMapper::UniformCTypeMapper(
        Layout::CType ctype, const std::vector<String>& skslTypes,
        const String& setUniformSingleFormat, const String& setUniformArrayFormat,
        const String& defaultValue, const String& dirtyExpressionFormat,
        const String& saveStateFormat)
    : fCType(ctype)
    , fSKSLTypes(skslTypes)
    , fUniformSingleTemplate(setUniformSingleFormat)
    , fUniformArrayTemplate(setUniformArrayFormat)
    , fInlineValue(determine_inline_from_template(setUniformSingleFormat) &&
                   determine_inline_from_template(setUniformArrayFormat))
    , fDefaultValue(defaultValue)
    , fDirtyExpressionTemplate(dirtyExpressionFormat)
    , fSaveStateTemplate(saveStateFormat) {}

const UniformCTypeMapper* UniformCTypeMapper::arrayMapper(int count) const {
    static SkMutex& mutex = *(new SkMutex);
    SkAutoMutexExclusive guard(mutex);
    using Key = std::pair<const UniformCTypeMapper*, int>;
    static std::map<Key, UniformCTypeMapper> registered;
    Key key(this, count);
    auto result = registered.find(key);
    if (result == registered.end()) {
        auto [iter, didInsert] = registered.insert({key, *this});
        SkASSERT(didInsert);
        UniformCTypeMapper* inserted = &iter->second;
        inserted->fArrayCount = count;
        return inserted;
    }
    return &result->second;
}


static UniformCTypeMapper register_array(Layout::CType ctype, const std::vector<String>& skslTypes,
                                   const char* singleSet, const char* arraySet,
                                   const char* defaultValue, const char* dirtyExpression) {
    return UniformCTypeMapper(ctype, skslTypes, singleSet, arraySet, defaultValue, dirtyExpression,
                              "${oldVar} = ${newVar}");
}

static UniformCTypeMapper register_array(Layout::CType ctype, const std::vector<String>& skslTypes,
                                         const char* singleSet, const char* arraySet,
                                         const char* defaultValue) {
    return register_array(ctype, skslTypes, singleSet, arraySet, defaultValue,
                              "${oldVar} != ${newVar}");
}

static UniformCTypeMapper register_type(Layout::CType ctype, const std::vector<String>& skslTypes,
                                   const char* uniformFormat, const char* defaultValue,
                                   const char* dirtyExpression) {
    return register_array(ctype, skslTypes, uniformFormat, uniformFormat, defaultValue,
                          dirtyExpression);
}

static UniformCTypeMapper register_type(Layout::CType ctype, const std::vector<String>& skslTypes,
                                   const char* uniformFormat, const char* defaultValue) {
    return register_array(ctype, skslTypes, uniformFormat, uniformFormat, defaultValue);
}

//////////////////////////////
// Currently defined ctypes //
//////////////////////////////

static const std::vector<UniformCTypeMapper>& get_mappers() {
    static const auto& kRegisteredMappers = *new std::vector<UniformCTypeMapper>{
    register_type(Layout::CType::kSkRect, { "half4", "float4", "double4" },
        "${pdman}.set4fv(${uniform}, ${count}, reinterpret_cast<const float*>(&${var}))", // to gpu
        "SkRect::MakeEmpty()",                                                     // default value
        "${oldVar}.isEmpty() || ${oldVar} != ${newVar}"),                          // dirty check

    register_type(Layout::CType::kSkIRect, { "int4", "short4", "byte4" },
        "${pdman}.set4iv(${uniform}, ${count}, reinterpret_cast<const int*>(&${var}))", // to gpu
        "SkIRect::MakeEmpty()",                                                    // default value
        "${oldVar}.isEmpty() || ${oldVar} != ${newVar}"),                          // dirty check

    register_type(Layout::CType::kSkPMColor4f, { "half4", "float4", "double4" },
        "${pdman}.set4fv(${uniform}, ${count}, ${var}.vec())",                     // to gpu
        "{SK_FloatNaN, SK_FloatNaN, SK_FloatNaN, SK_FloatNaN}"),                   // default value

    register_type(Layout::CType::kSkV4, { "half4", "float4", "double4" },
        "${pdman}.set4fv(${uniform}, ${count}, ${var}.ptr())",                     // to gpu
        "SkV4{SK_FloatNaN, SK_FloatNaN, SK_FloatNaN, SK_FloatNaN}",                // default value
        "${oldVar} != (${newVar})"),                                               // dirty check

    register_array(Layout::CType::kSkPoint, { "half2", "float2", "double2" } ,
        "${pdman}.set2f(${uniform}, ${var}.fX, ${var}.fY)",                        // single
        "${pdman}.set2fv(${uniform}, ${count}, &${var}.fX)",                       // array
        "SkPoint::Make(SK_FloatNaN, SK_FloatNaN)"),                                // default value

    register_array(Layout::CType::kSkIPoint, { "int2", "short2", "byte2" },
        "${pdman}.set2i(${uniform}, ${var}.fX, ${var}.fY)",                        // single
        "${pdman}.set2iv(${uniform}, ${count}, ${var}.fX, ${var}.fY)",             // array
        "SkIPoint::Make(SK_NaN32, SK_NaN32)"),                                     // default value

    register_type(Layout::CType::kSkMatrix, { "half3x3", "float3x3", "double3x3" },
        "static_assert(${count} == 1); ${pdman}.setSkMatrix(${uniform}, ${var})",  // to gpu
        "SkMatrix::Scale(SK_FloatNaN, SK_FloatNaN)",                               // default value
        "!${oldVar}.cheapEqualTo(${newVar})"),                                     // dirty check

    register_type(Layout::CType::kSkM44, { "half4x4", "float4x4", "double4x4" },
        "static_assert(${count} == 1); ${pdman}.setSkM44(${uniform}, ${var})",     // to gpu
        "SkM44(SkM44::kNaN_Constructor)",                                          // default value
        "${oldVar} != (${newVar})"),                                               // dirty check

    register_array(Layout::CType::kFloat, { "half", "float", "double" },
        "${pdman}.set1f(${uniform}, ${var})",                                      // single
        "${pdman}.set1fv(${uniform}, ${count}, &${var})",                          // array
        "SK_FloatNaN"),                                                            // default value

    register_array(Layout::CType::kInt32, { "int", "short", "byte" },
        "${pdman}.set1i(${uniform}, ${var})",                                      // single
        "${pdman}.set1iv(${uniform}, ${count}, &${var})",                          // array
        "SK_NaN32"),                                                               // default value
    };

    return kRegisteredMappers;
}

/////

// Greedy search through registered handlers for one that has a matching
// ctype and supports the sksl type of the variable.
const UniformCTypeMapper* UniformCTypeMapper::Get(const Context& context, const Type& type,
                                                  const Layout& layout) {
    if (type.isArray()) {
        const UniformCTypeMapper* base = Get(context, type.componentType(), layout);
        return base ? base->arrayMapper(type.columns()) : nullptr;
    }
    const std::vector<UniformCTypeMapper>& registeredMappers = get_mappers();

    Layout::CType ctype = layout.fCType;
    // If there's no custom ctype declared in the layout, use the default type mapping
    if (ctype == Layout::CType::kDefault) {
        ctype = HCodeGenerator::ParameterCType(context, type, layout);
    }

    for (const UniformCTypeMapper& mapper : registeredMappers) {
        if (mapper.ctype() == ctype) {
            // Check for SkSL support, since some C types (e.g. SkMatrix) can be used in multiple
            // uniform types and send data to the GPU differently depending on the uniform type.
            for (const String& mapperSupportedType : mapper.supportedTypeNames()) {
                if (mapperSupportedType == type.name()) {
                    // Return the match that we found.
                    return &mapper;
                }
            }
        }
    }

    // Didn't find a match.
    return nullptr;
}

}  // namespace SkSL

#endif // defined(SKSL_STANDALONE) || GR_TEST_UTILS
