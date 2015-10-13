//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EXT_blend_func_extended.cpp:
//   Test for EXT_blend_func_extended_test
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"

using testing::Combine;
using testing::Values;
using testing::make_tuple;

namespace
{
const char ESSLVersion100[] = "#version 100\n";
const char ESSLVersion300[] = "#version 300 es\n";
const char ESSLVersion310[] = "#version 310 es\n";
const char EXTBFEPragma[]   = "#extension GL_EXT_blend_func_extended : require\n";

const char ESSL100_SimpleShader1[] =
    "precision mediump float;\n"
    "void main() { \n"
    "    gl_FragColor = vec4(1.0);\n"
    "    gl_SecondaryFragColorEXT = vec4(gl_MaxDualSourceDrawBuffersEXT / 10);\n"
    "}\n";

// Shader that tests only the access to gl_MaxDualSourceDrawBuffersEXT.
const char ESSL100_MaxDualSourceAccessShader[] =
    "precision mediump float;\n"
    "void main() { gl_FragColor = vec4(gl_MaxDualSourceDrawBuffersEXT / 10); }\n";

// Shader that writes to SecondaryFragData.
const char ESSL100_FragDataShader[] =
    "#extension GL_EXT_draw_buffers : require\n"
    "precision mediump float;\n"
    "void main() {\n"
    "    gl_FragData[gl_MaxDrawBuffers - 1] = vec4(1.0);\n"
    "    gl_SecondaryFragDataEXT[gl_MaxDualSourceDrawBuffersEXT - 1] = vec4(0.1);\n"
    "}\n";

// Shader that writes to SecondaryFragColor and SecondaryFragData does not compile.
const char ESSL100_ColorAndDataWriteFailureShader1[] =
    "precision mediump float;\n"
    "void main() {\n"
    "    gl_SecondaryFragColorEXT = vec4(1.0);\n"
    "    gl_SecondaryFragDataEXT[gl_MaxDualSourceDrawBuffersEXT] = vec4(0.1);\n"
    "}\n";

// Shader that writes to FragColor and SecondaryFragData does not compile.
const char ESSL100_ColorAndDataWriteFailureShader2[] =
    "precision mediump float;\n"
    "void main() {\n"
    "    gl_FragColor = vec4(1.0);\n"
    "    gl_SecondaryFragDataEXT[gl_MaxDualSourceDrawBuffersEXT] = vec4(0.1);\n"
    "}\n";

// Shader that writes to FragData and SecondaryFragColor.
const char ESSL100_ColorAndDataWriteFailureShader3[] =
    "#extension GL_EXT_draw_buffers : require\n"
    "precision mediump float;\n"
    "void main() {\n"
    "    gl_SecondaryFragColorEXT = vec4(1.0);\n"
    "    gl_FragData[gl_MaxDrawBuffers] = vec4(0.1);\n"
    "}\n";

// In GLSL version 300 es, the gl_MaxDualSourceDrawBuffersEXT is available.
const char ESSL300_MaxDualSourceAccessShader[] =
    "precision mediump float;\n"
    "layout(location = 0) out mediump vec4 fragColor;"
    "void main() {\n"
    "    fragColor = vec4(gl_MaxDualSourceDrawBuffersEXT / 10);\n"
    "}\n";

// In GLSL version 300 es, the only way to write a correct shader is to require the extension and
// then leave the locations unspecified. The caller will then bind the variables with the extension
// binding functions.
const char ESSL300_LocationAndUnspecifiedOutputShader[] =
    "precision mediump float;\n"
    "layout(location = 0) out mediump vec4 fragColor;"
    "out mediump vec4 secondaryFragColor;"
    "void main() {\n"
    "    fragColor = vec4(1.0);\n"
    "    secondaryFragColor = vec4(1.0);\n"
    "}\n";

const char ESSL300_TwoUnspecifiedLocationOutputsShader[] =
    "precision mediump float;\n"
    "out mediump vec4 fragColor;"
    "out mediump vec4 secondaryFragColor;"
    "void main() {\n"
    "    fragColor = vec4(1.0);\n"
    "    secondaryFragColor = vec4(1.0);\n"
    "}\n";

// Shader that is correct in GLSL ES 3.10 fails when used in version 300 es.
const char ESSL310_LocationIndexShader[] =
    "precision mediump float;\n"
    "layout(location = 0) out mediump vec4 fragColor;"
    "layout(location = 0, index = 1) out mediump vec4 secondaryFragColor;"
    "void main() {\n"
    "   fragColor = vec4(1);\n"
    "   secondaryFragColor = vec4(1);\n"
    "}\n";

// Shader that specifies index layout qualifier but not location fails to compile. Currently fails
// to compile due to version 310 es not being supported.
const char ESSL310_LocationIndexFailureShader[] =
    "precision mediump float;\n"
    "layout(location = 0) out mediump vec4 fragColor;"
    "layout(index = 1) out mediump vec4 secondaryFragColor;"
    "void main() {\n"
    "   fragColor = vec4(1.0);\n"
    "   secondaryFragColor = vec4(1.0);\n"
    "}\n";

class EXTBlendFuncExtendedTest
    : public testing::TestWithParam<testing::tuple<ShShaderSpec, const char *, const char *>>
{
  protected:
    virtual void SetUp()
    {
        ShInitBuiltInResources(&mResources);
        // EXT_draw_buffers is used in some of the shaders for test purposes.
        mResources.EXT_draw_buffers = 1;
        mResources.NV_draw_buffers  = 2;

        mCompiler = NULL;
    }

    virtual void TearDown() { DestroyCompiler(); }
    void DestroyCompiler()
    {
        if (mCompiler)
        {
            ShDestruct(mCompiler);
            mCompiler = NULL;
        }
    }

    void InitializeCompiler()
    {
        DestroyCompiler();
        mCompiler = ShConstructCompiler(GL_FRAGMENT_SHADER, testing::get<0>(GetParam()),
                                        SH_GLSL_OUTPUT, &mResources);
        ASSERT_TRUE(mCompiler != NULL) << "Compiler could not be constructed.";
    }

    testing::AssertionResult TestShaderCompile(const char *pragma)
    {
        return TestShaderCompile(testing::get<1>(GetParam()),  // Version.
                                 pragma,
                                 testing::get<2>(GetParam())  // Shader.
                                 );
    }

    testing::AssertionResult TestShaderCompile(const char *version,
                                               const char *pragma,
                                               const char *shader)
    {
        const char *shaderStrings[] = {version, pragma, shader};
        bool success = ShCompile(mCompiler, shaderStrings, 3, 0);
        if (success)
        {
            return ::testing::AssertionSuccess() << "Compilation success";
        }
        return ::testing::AssertionFailure() << ShGetInfoLog(mCompiler);
    }

  protected:
    ShBuiltInResources mResources;
    ShHandle mCompiler;
};

// Extension flag is required to compile properly. Expect failure when it is
// not present.
TEST_P(EXTBlendFuncExtendedTest, CompileFailsWithoutExtension)
{
    mResources.EXT_blend_func_extended = 0;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(EXTBFEPragma));
}

// Extension directive is required to compile properly. Expect failure when
// it is not present.
TEST_P(EXTBlendFuncExtendedTest, CompileFailsWithExtensionWithoutPragma)
{
    mResources.EXT_blend_func_extended  = 1;
    mResources.MaxDualSourceDrawBuffers = 1;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(""));
}

// With extension flag and extension directive, compiling succeeds.
// Also test that the extension directive state is reset correctly.
TEST_P(EXTBlendFuncExtendedTest, CompileSucceedsWithExtensionAndPragma)
{
    mResources.EXT_blend_func_extended  = 1;
    mResources.MaxDualSourceDrawBuffers = 1;
    InitializeCompiler();
    EXPECT_TRUE(TestShaderCompile(EXTBFEPragma));
    // Test reset functionality.
    EXPECT_FALSE(TestShaderCompile(""));
    EXPECT_TRUE(TestShaderCompile(EXTBFEPragma));
}

// The SL #version 100 shaders that are correct work similarly
// in both GL2 and GL3, with and without the version string.
INSTANTIATE_TEST_CASE_P(CorrectESSL100Shaders,
                        EXTBlendFuncExtendedTest,
                        Combine(Values(SH_GLES2_SPEC, SH_GLES3_SPEC),
                                Values("", ESSLVersion100),
                                Values(ESSL100_SimpleShader1,
                                       ESSL100_MaxDualSourceAccessShader,
                                       ESSL100_FragDataShader)));

INSTANTIATE_TEST_CASE_P(CorrectESSL300Shaders,
                        EXTBlendFuncExtendedTest,
                        Combine(Values(SH_GLES3_SPEC),
                                Values(ESSLVersion300),
                                Values(ESSL300_MaxDualSourceAccessShader,
                                       ESSL300_LocationAndUnspecifiedOutputShader,
                                       ESSL300_TwoUnspecifiedLocationOutputsShader)));

class EXTBlendFuncExtendedCompileFailureTest : public EXTBlendFuncExtendedTest
{
};

TEST_P(EXTBlendFuncExtendedCompileFailureTest, CompileFails)
{
    // Expect compile failure due to shader error, with shader having correct pragma.
    mResources.EXT_blend_func_extended  = 1;
    mResources.MaxDualSourceDrawBuffers = 1;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(EXTBFEPragma));
}

// Incorrect #version 100 shaders fail.
INSTANTIATE_TEST_CASE_P(IncorrectESSL100Shaders,
                        EXTBlendFuncExtendedCompileFailureTest,
                        Combine(Values(SH_GLES2_SPEC),
                                Values(ESSLVersion100),
                                Values(ESSL100_ColorAndDataWriteFailureShader1,
                                       ESSL100_ColorAndDataWriteFailureShader2,
                                       ESSL100_ColorAndDataWriteFailureShader3)));

// Correct #version 300 es shaders fail in GLES2 context, regardless of version string.
INSTANTIATE_TEST_CASE_P(CorrectESSL300Shaders,
                        EXTBlendFuncExtendedCompileFailureTest,
                        Combine(Values(SH_GLES2_SPEC),
                                Values("", ESSLVersion100, ESSLVersion300),
                                Values(ESSL300_LocationAndUnspecifiedOutputShader,
                                       ESSL300_TwoUnspecifiedLocationOutputsShader)));

// Correct #version 100 shaders fail when used with #version 300 es.
INSTANTIATE_TEST_CASE_P(CorrectESSL100Shaders,
                        EXTBlendFuncExtendedCompileFailureTest,
                        Combine(Values(SH_GLES3_SPEC),
                                Values(ESSLVersion300),
                                Values(ESSL100_SimpleShader1, ESSL100_FragDataShader)));

// Incorrect #version 310 es always fails.
INSTANTIATE_TEST_CASE_P(IncorrectESSL310Shaders,
                        EXTBlendFuncExtendedCompileFailureTest,
                        Combine(Values(SH_GLES3_SPEC),
                                Values(ESSLVersion300, ESSLVersion310),
                                Values(ESSL310_LocationIndexFailureShader)));

// Correct #version 310 es fails in #version 300 es.
INSTANTIATE_TEST_CASE_P(
    CorrectESSL310ShadersInESSL300,
    EXTBlendFuncExtendedCompileFailureTest,
    Values(make_tuple(SH_GLES3_SPEC, &ESSLVersion300[0], &ESSL310_LocationIndexShader[0])));

// Correct #version 310 es fails in #version 310 es, due to 3.1 not being supported.
INSTANTIATE_TEST_CASE_P(
    CorrectESSL310Shaders,
    EXTBlendFuncExtendedCompileFailureTest,
    Values(make_tuple(SH_GLES3_SPEC, &ESSLVersion310[0], &ESSL310_LocationIndexShader[0])));

}  // namespace
