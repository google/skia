//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// compiler_test.cpp:
//     utilities for compiler unit tests.

#include "compiler/translator/Compiler.h"

bool compileTestShader(sh::GLenum type,
                       ShShaderSpec spec,
                       ShShaderOutput output,
                       const std::string &shaderString,
                       ShBuiltInResources *resources,
                       int compileOptions,
                       std::string *translatedCode,
                       std::string *infoLog)
{
    if (spec == SH_GLES3_SPEC || spec == SH_WEBGL2_SPEC)
    {
        resources->FragmentPrecisionHigh = 1;
    }

    TCompiler *translator = ConstructCompiler(type, spec, output);
    if (!translator->Init(*resources))
    {
        SafeDelete(translator);
        return false;
    }

    const char *shaderStrings[] = { shaderString.c_str() };

    bool compilationSuccess = translator->compile(shaderStrings, 1, SH_OBJECT_CODE | compileOptions);
    TInfoSink &infoSink = translator->getInfoSink();
    if (translatedCode)
        *translatedCode = infoSink.obj.c_str();
    if (infoLog)
        *infoLog = infoSink.info.c_str();
    SafeDelete(translator);
    return compilationSuccess;
}

bool compileTestShader(sh::GLenum type,
                       ShShaderSpec spec,
                       ShShaderOutput output,
                       const std::string &shaderString,
                       ShBuiltInResources *resources,
                       std::string *translatedCode,
                       std::string *infoLog)
{
    return compileTestShader(type, spec, output, shaderString, resources, 0, translatedCode, infoLog);
}

bool compileTestShader(sh::GLenum type,
                       ShShaderSpec spec,
                       ShShaderOutput output,
                       const std::string &shaderString,
                       int compileOptions,
                       std::string *translatedCode,
                       std::string *infoLog)
{
    ShBuiltInResources resources;
    ShInitBuiltInResources(&resources);
    return compileTestShader(type, spec, output, shaderString, &resources, compileOptions, translatedCode, infoLog);
}

bool compileTestShader(sh::GLenum type,
                       ShShaderSpec spec,
                       ShShaderOutput output,
                       const std::string &shaderString,
                       std::string *translatedCode,
                       std::string *infoLog)
{
    return compileTestShader(type, spec, output, shaderString, 0, translatedCode, infoLog);
}
