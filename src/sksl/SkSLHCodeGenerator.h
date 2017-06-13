/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_HCODEGENERATOR
#define SKSL_HCODEGENERATOR

#include "SkSLCodeGenerator.h"
#include "ir/SkSLType.h"
#include "ir/SkSLVariable.h"

namespace SkSL {

#define CLASS_SECTION              "class"
#define CPP_SECTION                "cpp"
#define HEADER_SECTION             "header"
#define CONSTRUCTOR_PARAMS_SECTION "constructorParams"
#define CONSTRUCTOR_SECTION        "constructor"
#define CONSTRUCTOR_CODE_SECTION   "constructorCode"
#define INITIALIZERS_SECTION       "initializers"
#define EMIT_CODE_SECTION          "emitCode"
#define FIELDS_SECTION             "fields"
#define MAKE_SECTION               "make"
#define OPTIMIZATION_FLAGS_SECTION "optimizationFlags"
#define SET_DATA_SECTION           "setData"
#define TEST_CODE_SECTION          "test"

class HCodeGenerator : public CodeGenerator {
public:
    HCodeGenerator(const Program* program, ErrorReporter* errors, String name, OutputStream* out);

    bool generateCode() override;

    static String ParameterType(const Type& type);

    static String FieldType(const Type& type);

    static bool IsSupportedSection(const char* name);

    static bool SectionAcceptsArgument(const char* name);

private:
    void writef(const char* s, va_list va) SKSL_PRINTF_LIKE(2, 0);

    void writef(const char* s, ...) SKSL_PRINTF_LIKE(2, 3);

    bool writeSection(const char* name, const char* prefix = "");

    // given a @constructorParams section of e.g. 'int x, float y', writes out "<separator>x, y".
    // Writes nothing (not even the separator) if there is no @constructorParams section.
    void writeExtraConstructorParams(const char* separator);

    void writeMake();

    void writeConstructor();

    void writeFields();

    String fName;
    String fFullName;
    std::vector<const Variable*> fParameters;
    std::unordered_map<String, String> fSections;

    typedef CodeGenerator INHERITED;
};

}

#endif
