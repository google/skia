//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "shader_utils.h"

#include <vector>
#include <iostream>
#include <fstream>

static std::string ReadFileToString(const std::string &source)
{
    std::ifstream stream(source.c_str());
    if (!stream)
    {
        std::cerr << "Failed to load shader file: " << source;
        return "";
    }

    std::string result;

    stream.seekg(0, std::ios::end);
    result.reserve(static_cast<unsigned int>(stream.tellg()));
    stream.seekg(0, std::ios::beg);

    result.assign((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

    return result;
}

GLuint CompileShader(GLenum type, const std::string &source)
{
    GLuint shader = glCreateShader(type);

    const char *sourceArray[1] = { source.c_str() };
    glShaderSource(shader, 1, sourceArray, NULL);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);

    if (compileResult == 0)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<GLchar> infoLog(infoLogLength);
        glGetShaderInfoLog(shader, static_cast<GLsizei>(infoLog.size()), NULL, &infoLog[0]);

        std::cerr << "shader compilation failed: " << &infoLog[0];

        glDeleteShader(shader);
        shader = 0;
    }

    return shader;
}

GLuint CompileShaderFromFile(GLenum type, const std::string &sourcePath)
{
    std::string source = ReadFileToString(sourcePath);
    if (source.empty())
    {
        return 0;
    }

    return CompileShader(type, source);
}

GLuint CompileProgramWithTransformFeedback(
    const std::string &vsSource,
    const std::string &fsSource,
    const std::vector<std::string> &transformFeedbackVaryings,
    GLenum bufferMode)
{
    GLuint program = glCreateProgram();

    GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSource);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSource);

    if (vs == 0 || fs == 0)
    {
        glDeleteShader(fs);
        glDeleteShader(vs);
        glDeleteProgram(program);
        return 0;
    }

    glAttachShader(program, vs);
    glDeleteShader(vs);

    glAttachShader(program, fs);
    glDeleteShader(fs);

    if (transformFeedbackVaryings.size() > 0)
    {
        std::vector<const char *> constCharTFVaryings;

        for (const std::string &transformFeedbackVarying : transformFeedbackVaryings)
        {
            constCharTFVaryings.push_back(transformFeedbackVarying.c_str());
        }

        glTransformFeedbackVaryings(program, static_cast<GLsizei>(transformFeedbackVaryings.size()),
                                    &constCharTFVaryings[0], bufferMode);
    }

    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if (linkStatus == 0)
    {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<GLchar> infoLog(infoLogLength);
        glGetProgramInfoLog(program, static_cast<GLsizei>(infoLog.size()), NULL, &infoLog[0]);

        std::cerr << "program link failed: " << &infoLog[0];

        glDeleteProgram(program);
        return 0;
    }

    return program;
}

GLuint CompileProgram(const std::string &vsSource, const std::string &fsSource)
{
    std::vector<std::string> emptyVector;
    return CompileProgramWithTransformFeedback(vsSource, fsSource, emptyVector, GL_NONE);
}

GLuint CompileProgramFromFiles(const std::string &vsPath, const std::string &fsPath)
{
    std::string vsSource = ReadFileToString(vsPath);
    std::string fsSource = ReadFileToString(fsPath);
    if (vsSource.empty() || fsSource.empty())
    {
        return 0;
    }

    return CompileProgram(vsSource, fsSource);
}
