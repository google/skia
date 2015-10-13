//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#include <sstream>
#include <string>
#include <vector>
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"

#define SHADER(Src) #Src

class ExpressionLimitTest : public testing::Test {
protected:
    static const int kMaxExpressionComplexity = 16;
    static const int kMaxCallStackDepth = 16;
    static const char* kExpressionTooComplex;
    static const char* kCallStackTooDeep;
    static const char* kHasRecursion;

    virtual void SetUp()
    {
        memset(&resources, 0, sizeof(resources));

        GenerateResources(&resources);
    }

    // Set up the per compile resources
    static void GenerateResources(ShBuiltInResources *res)
    {
        ShInitBuiltInResources(res);

        res->MaxVertexAttribs             = 8;
        res->MaxVertexUniformVectors      = 128;
        res->MaxVaryingVectors            = 8;
        res->MaxVertexTextureImageUnits   = 0;
        res->MaxCombinedTextureImageUnits = 8;
        res->MaxTextureImageUnits         = 8;
        res->MaxFragmentUniformVectors    = 16;
        res->MaxDrawBuffers               = 1;

        res->OES_standard_derivatives = 0;
        res->OES_EGL_image_external   = 0;

        res->MaxExpressionComplexity = kMaxExpressionComplexity;
        res->MaxCallStackDepth       = kMaxCallStackDepth;
    }

    void GenerateLongExpression(int length, std::stringstream* ss)
    {
        for (int ii = 0; ii < length; ++ii) {
          *ss << "+ vec4(" << ii << ")";
        }
    }

    std::string GenerateShaderWithLongExpression(int length)
    {
        static const char* shaderStart = SHADER(
            precision mediump float;
            uniform vec4 u_color;
            void main()
            {
               gl_FragColor = u_color
        );

        std::stringstream ss;
        ss << shaderStart;
        GenerateLongExpression(length, &ss);
        ss << "; }";

        return ss.str();
    }

    std::string GenerateShaderWithUnusedLongExpression(int length)
    {
        static const char* shaderStart = SHADER(
            precision mediump float;
            uniform vec4 u_color;
            void main()
            {
               gl_FragColor = u_color;
            }
            vec4 someFunction() {
              return u_color
        );

        std::stringstream ss;

        ss << shaderStart;
        GenerateLongExpression(length, &ss);
        ss << "; }";

        return ss.str();
    }

    void GenerateDeepFunctionStack(int length, std::stringstream* ss)
    {
        static const char* shaderStart = SHADER(
            precision mediump float;
            uniform vec4 u_color;
            vec4 function0()  {
              return u_color;
            }
        );

        *ss << shaderStart;
        for (int ii = 0; ii < length; ++ii) {
          *ss << "vec4 function" << (ii + 1) << "() {\n"
              << "  return function" << ii << "();\n"
              << "}\n";
        }
    }

    std::string GenerateShaderWithDeepFunctionStack(int length)
    {
        std::stringstream ss;

        GenerateDeepFunctionStack(length, &ss);

        ss << "void main() {\n"
           << "  gl_FragColor = function" << length << "();\n"
           << "}";

        return ss.str();
    }

    std::string GenerateShaderWithUnusedDeepFunctionStack(int length)
    {
        std::stringstream ss;

        GenerateDeepFunctionStack(length, &ss);

        ss << "void main() {\n"
           << "  gl_FragColor = vec4(0,0,0,0);\n"
           << "}";


        return ss.str();
    }

    // Compiles a shader and if there's an error checks for a specific
    // substring in the error log. This way we know the error is specific
    // to the issue we are testing.
    bool CheckShaderCompilation(ShHandle compiler,
                                const char *source,
                                int compileOptions,
                                const char *expected_error)
    {
        bool success = ShCompile(compiler, &source, 1, compileOptions) != 0;
        if (success)
        {
            success = !expected_error;
        }
        else
        {
            std::string log = ShGetInfoLog(compiler);
            if (expected_error)
                success = log.find(expected_error) != std::string::npos;

            EXPECT_TRUE(success) << log << "\n----shader----\n" << source;
        }
        return success;
    }

    ShBuiltInResources resources;
};

const char* ExpressionLimitTest::kExpressionTooComplex =
    "Expression too complex";
const char* ExpressionLimitTest::kCallStackTooDeep =
    "Call stack too deep";
const char* ExpressionLimitTest::kHasRecursion =
    "Function recursion detected";

TEST_F(ExpressionLimitTest, ExpressionComplexity)
{
    ShShaderSpec spec = SH_WEBGL_SPEC;
    ShShaderOutput output = SH_ESSL_OUTPUT;
    ShHandle vertexCompiler = ShConstructCompiler(
        GL_FRAGMENT_SHADER, spec, output, &resources);
    int compileOptions = SH_LIMIT_EXPRESSION_COMPLEXITY;

    // Test expression under the limit passes.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler,
        GenerateShaderWithLongExpression(
            kMaxExpressionComplexity - 10).c_str(),
        compileOptions, NULL));
    // Test expression over the limit fails.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler,
        GenerateShaderWithLongExpression(
            kMaxExpressionComplexity + 10).c_str(),
        compileOptions, kExpressionTooComplex));
    // Test expression over the limit without a limit does not fail.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler,
        GenerateShaderWithLongExpression(
            kMaxExpressionComplexity + 10).c_str(),
        compileOptions & ~SH_LIMIT_EXPRESSION_COMPLEXITY, NULL));
    ShDestruct(vertexCompiler);
}

TEST_F(ExpressionLimitTest, UnusedExpressionComplexity)
{
    ShShaderSpec spec = SH_WEBGL_SPEC;
    ShShaderOutput output = SH_ESSL_OUTPUT;
    ShHandle vertexCompiler = ShConstructCompiler(
        GL_FRAGMENT_SHADER, spec, output, &resources);
    int compileOptions = SH_LIMIT_EXPRESSION_COMPLEXITY;

    // Test expression under the limit passes.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler,
        GenerateShaderWithUnusedLongExpression(
            kMaxExpressionComplexity - 10).c_str(),
        compileOptions, NULL));
    // Test expression over the limit fails.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler,
        GenerateShaderWithUnusedLongExpression(
            kMaxExpressionComplexity + 10).c_str(),
        compileOptions, kExpressionTooComplex));
    // Test expression over the limit without a limit does not fail.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler,
        GenerateShaderWithUnusedLongExpression(
            kMaxExpressionComplexity + 10).c_str(),
        compileOptions & ~SH_LIMIT_EXPRESSION_COMPLEXITY, NULL));
    ShDestruct(vertexCompiler);
}

TEST_F(ExpressionLimitTest, CallStackDepth)
{
    ShShaderSpec spec = SH_WEBGL_SPEC;
    ShShaderOutput output = SH_ESSL_OUTPUT;
    ShHandle vertexCompiler = ShConstructCompiler(
        GL_FRAGMENT_SHADER, spec, output, &resources);
    int compileOptions = SH_LIMIT_CALL_STACK_DEPTH;

    // Test call stack under the limit passes.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler,
        GenerateShaderWithDeepFunctionStack(
            kMaxCallStackDepth - 10).c_str(),
        compileOptions, NULL));
    // Test call stack over the limit fails.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler,
        GenerateShaderWithDeepFunctionStack(
            kMaxCallStackDepth + 10).c_str(),
        compileOptions, kCallStackTooDeep));
    // Test call stack over the limit without limit does not fail.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler,
        GenerateShaderWithDeepFunctionStack(
            kMaxCallStackDepth + 10).c_str(),
        compileOptions & ~SH_LIMIT_CALL_STACK_DEPTH, NULL));
    ShDestruct(vertexCompiler);
}

TEST_F(ExpressionLimitTest, UnusedCallStackDepth)
{
    ShShaderSpec spec = SH_WEBGL_SPEC;
    ShShaderOutput output = SH_ESSL_OUTPUT;
    ShHandle vertexCompiler = ShConstructCompiler(
        GL_FRAGMENT_SHADER, spec, output, &resources);
    int compileOptions = SH_LIMIT_CALL_STACK_DEPTH;

    // Test call stack under the limit passes.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler,
        GenerateShaderWithUnusedDeepFunctionStack(
            kMaxCallStackDepth - 10).c_str(),
        compileOptions, NULL));
    // Test call stack over the limit fails.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler,
        GenerateShaderWithUnusedDeepFunctionStack(
            kMaxCallStackDepth + 10).c_str(),
        compileOptions, kCallStackTooDeep));
    // Test call stack over the limit without limit does not fail.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler,
        GenerateShaderWithUnusedDeepFunctionStack(
            kMaxCallStackDepth + 10).c_str(),
        compileOptions & ~SH_LIMIT_CALL_STACK_DEPTH, NULL));
    ShDestruct(vertexCompiler);
}

TEST_F(ExpressionLimitTest, Recursion)
{
    ShShaderSpec spec = SH_WEBGL_SPEC;
    ShShaderOutput output = SH_ESSL_OUTPUT;
    ShHandle vertexCompiler = ShConstructCompiler(
        GL_FRAGMENT_SHADER, spec, output, &resources);
    int compileOptions = 0;

    static const char* shaderWithRecursion0 = SHADER(
        precision mediump float;
        uniform vec4 u_color;
        vec4 someFunc()  {
            return someFunc();
        }

        void main() {
            gl_FragColor = u_color * someFunc();
        }
    );

    static const char* shaderWithRecursion1 = SHADER(
        precision mediump float;
        uniform vec4 u_color;

        vec4 someFunc();

        vec4 someFunc1()  {
            return someFunc();
        }

        vec4 someFunc()  {
            return someFunc1();
        }

        void main() {
            gl_FragColor = u_color * someFunc();
        }
    );

    static const char* shaderWithRecursion2 = SHADER(
        precision mediump float;
        uniform vec4 u_color;
        vec4 someFunc()  {
            if (u_color.x > 0.5) {
                return someFunc();
            } else {
                return vec4(1);
            }
        }

        void main() {
            gl_FragColor = someFunc();
        }
    );

    static const char* shaderWithRecursion3 = SHADER(
        precision mediump float;
        uniform vec4 u_color;
        vec4 someFunc()  {
            if (u_color.x > 0.5) {
                return vec4(1);
            } else {
                return someFunc();
            }
        }

        void main() {
            gl_FragColor = someFunc();
        }
    );

    static const char* shaderWithRecursion4 = SHADER(
        precision mediump float;
        uniform vec4 u_color;
        vec4 someFunc()  {
            return (u_color.x > 0.5) ? vec4(1) : someFunc();
        }

        void main() {
            gl_FragColor = someFunc();
        }
    );

    static const char* shaderWithRecursion5 = SHADER(
        precision mediump float;
        uniform vec4 u_color;
        vec4 someFunc()  {
            return (u_color.x > 0.5) ? someFunc() : vec4(1);
        }

        void main() {
            gl_FragColor = someFunc();
        }
    );

    static const char* shaderWithRecursion6 = SHADER(
        precision mediump float;
        uniform vec4 u_color;
        vec4 someFunc()  {
            return someFunc();
        }

        void main() {
            gl_FragColor = u_color;
        }
    );

    static const char* shaderWithNoRecursion = SHADER(
        precision mediump float;
        uniform vec4 u_color;

        vec3 rgb(int r, int g, int b) {
            return vec3(float(r) / 255.0, float(g) / 255.0, float(b) / 255.0);
        }

        void main() {
            vec3 hairColor0 = rgb(151, 200, 234);
            vec3 faceColor2 = rgb(183, 148, 133);
            gl_FragColor = u_color + vec4(hairColor0 + faceColor2, 0);
        }
    );

    static const char* shaderWithRecursion7 = SHADER(
        precision mediump float;
        uniform vec4 u_color;

        vec4 function2() {
            return u_color;
        }

        vec4 function1() {
            vec4 a = function2();
            vec4 b = function1();
            return a + b;
        }

        void main() {
            gl_FragColor = function1();
        }
    );

    static const char* shaderWithRecursion8 = SHADER(
        precision mediump float;
        uniform vec4 u_color;

        vec4 function1();

        vec4 function3() {
            return function1();
        }

        vec4 function2() {
            return function3();
        }

        vec4 function1() {
            return function2();
        }

        void main() {
            gl_FragColor = function1();
        }
    );

    // Check simple recursions fails.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler, shaderWithRecursion0,
        compileOptions, kHasRecursion));
    // Check simple recursions fails.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler, shaderWithRecursion1,
        compileOptions, kHasRecursion));
    // Check if recursions fails.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler, shaderWithRecursion2,
        compileOptions, kHasRecursion));
    // Check if recursions fails.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler, shaderWithRecursion3,
        compileOptions, kHasRecursion));
    // Check ternary recursions fails.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler, shaderWithRecursion4,
        compileOptions, kHasRecursion));
    // Check ternary recursions fails.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler, shaderWithRecursion5,
        compileOptions, kHasRecursion));

    // Check some more forms of recursion
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler, shaderWithRecursion6,
        compileOptions, kHasRecursion));
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler, shaderWithRecursion7,
        compileOptions, kHasRecursion));
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler, shaderWithRecursion8,
        compileOptions, kHasRecursion));
    // Check unused recursions fails if limiting call stack
    // since we check all paths.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler, shaderWithRecursion6,
        compileOptions | SH_LIMIT_CALL_STACK_DEPTH, kHasRecursion));

    // Check unused recursions passes.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler, shaderWithNoRecursion,
        compileOptions, NULL));
    // Check unused recursions passes if limiting call stack.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler, shaderWithNoRecursion,
        compileOptions | SH_LIMIT_CALL_STACK_DEPTH, NULL));
    ShDestruct(vertexCompiler);
}

