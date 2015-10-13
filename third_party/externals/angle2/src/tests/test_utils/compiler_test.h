//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// compiler_test.h:
//     utilities for compiler unit tests.

#ifndef TESTS_TEST_UTILS_COMPILER_TEST_H_
#define TESTS_TEST_UTILS_COMPILER_TEST_H_

#include "GLSLANG/ShaderLang.h"

bool compileTestShader(sh::GLenum type,
                       ShShaderSpec spec,
                       ShShaderOutput output,
                       const std::string &shaderString,
                       ShBuiltInResources *resources,
                       int compileOptions,
                       std::string *translatedCode,
                       std::string *infoLog);

bool compileTestShader(sh::GLenum type,
                       ShShaderSpec spec,
                       ShShaderOutput output,
                       const std::string &shaderString,
                       ShBuiltInResources *resources,
                       std::string *translatedCode,
                       std::string *infoLog);

bool compileTestShader(sh::GLenum type,
                       ShShaderSpec spec,
                       ShShaderOutput output,
                       const std::string &shaderString,
                       int compileOptions,
                       std::string *translatedCode,
                       std::string *infoLog);

bool compileTestShader(sh::GLenum type,
                       ShShaderSpec spec,
                       ShShaderOutput output,
                       const std::string &shaderString,
                       std::string *translatedCode,
                       std::string *infoLog);

#endif // TESTS_TEST_UTILS_COMPILER_TEST_H_
