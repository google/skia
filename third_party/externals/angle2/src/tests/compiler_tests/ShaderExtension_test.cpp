//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Extension_test.cpp
//   Test that shaders need various extensions to be compiled.
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"

static const char *FragDepthExtShdr[] =
{
    // First string of the shader. The required pragma declaration.
    "#extension GL_EXT_frag_depth : enable\n",

    // Second string of the shader. The rest of the shader body.
    "void main() { gl_FragDepthEXT = 1.0; }\n"
};

static const char *StandDerivExtShdr[] =
{
    // First string of the shader. The required pragma declaration.
    "#extension GL_OES_standard_derivatives : enable\n",

    // Second string of the shader. The rest of the shader body.
    "precision mediump float;\n"
    "varying vec2 texCoord;\n"
    "void main() { gl_FragColor = vec4(dFdx(texCoord.x), dFdy(texCoord.y), fwidth(texCoord.x), 1.0); }\n"
};

static const char *TextureLODShdr[] =
{
    // First string of the shader. The required pragma declaration.
    "#extension GL_EXT_shader_texture_lod : enable\n",

    // Second string of the shader. The rest of the shader body.
    "precision mediump float;\n"
    "varying vec2 texCoord0v;\n"
    "uniform float lod;\n"
    "uniform sampler2D tex;\n"
    "void main() { vec4 color = texture2DLodEXT(tex, texCoord0v, lod); }\n"
};

class ShaderExtensionTest : public testing::Test
{
  public:
    ShaderExtensionTest() {}

  protected:
    virtual void SetUp()
    {
        ShInitBuiltInResources(&mResources);
        mCompiler = NULL;
    }

    virtual void TearDown()
    {
        DestroyCompiler();
    }

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
        mCompiler = ShConstructCompiler(GL_FRAGMENT_SHADER, SH_WEBGL_SPEC, SH_GLSL_OUTPUT, &mResources);
        ASSERT_TRUE(mCompiler != NULL) << "Compiler could not be constructed.";
    }

    void TestShaderExtension(const char **shaderStrings, int stringCount, bool expectation)
    {
        bool success = ShCompile(mCompiler, shaderStrings, stringCount, 0);
        const std::string& compileLog = ShGetInfoLog(mCompiler);
        EXPECT_EQ(expectation, success) << compileLog;
    }

  protected:
    ShBuiltInResources mResources;
    ShHandle mCompiler;
};

TEST_F(ShaderExtensionTest, FragDepthExtRequiresExt)
{
    // Extension is required to compile properly.
    mResources.EXT_frag_depth = 0;
    InitializeCompiler();
    TestShaderExtension(FragDepthExtShdr, 2, false);
}

TEST_F(ShaderExtensionTest, FragDepthExtRequiresPragma)
{
    // Pragma is required to compile properly.
    mResources.EXT_frag_depth = 1;
    InitializeCompiler();
    TestShaderExtension(&FragDepthExtShdr[1], 1, false);
}

TEST_F(ShaderExtensionTest, FragDepthExtCompiles)
{
    // Test that it compiles properly with extension enabled.
    mResources.EXT_frag_depth = 1;
    InitializeCompiler();
    TestShaderExtension(FragDepthExtShdr, 2, true);
}

TEST_F(ShaderExtensionTest, FragDepthExtResetsInternalStates)
{
    // Test that extension internal states are reset properly between compiles.
    mResources.EXT_frag_depth = 1;
    InitializeCompiler();

    TestShaderExtension(FragDepthExtShdr, 2, true);
    TestShaderExtension(&FragDepthExtShdr[1], 1, false);
    TestShaderExtension(FragDepthExtShdr, 2, true);
}

TEST_F(ShaderExtensionTest, StandDerivExtRequiresExt)
{
    // Extension is required to compile properly.
    mResources.OES_standard_derivatives = 0;
    InitializeCompiler();
    TestShaderExtension(StandDerivExtShdr, 2, false);
}

TEST_F(ShaderExtensionTest, StandDerivExtRequiresPragma)
{
    // Pragma is required to compile properly.
    mResources.OES_standard_derivatives = 1;
    InitializeCompiler();
    TestShaderExtension(&StandDerivExtShdr[1], 1, false);
}

TEST_F(ShaderExtensionTest, StandDerivExtCompiles)
{
    // Test that it compiles properly with extension enabled.
    mResources.OES_standard_derivatives = 1;
    InitializeCompiler();
    TestShaderExtension(StandDerivExtShdr, 2, true);
}

TEST_F(ShaderExtensionTest, StandDerivExtResetsInternalStates)
{
    // Test that extension internal states are reset properly between compiles.
    mResources.OES_standard_derivatives = 1;
    InitializeCompiler();

    TestShaderExtension(StandDerivExtShdr, 2, true);
    TestShaderExtension(&StandDerivExtShdr[1], 1, false);
    TestShaderExtension(StandDerivExtShdr, 2, true);
    TestShaderExtension(&StandDerivExtShdr[1], 1, false);
}

TEST_F(ShaderExtensionTest, TextureLODExtRequiresExt)
{
    // Extension is required to compile properly.
    mResources.EXT_shader_texture_lod = 0;
    InitializeCompiler();
    TestShaderExtension(TextureLODShdr, 2, false);
}

TEST_F(ShaderExtensionTest, TextureLODExtRequiresPragma)
{
    // Pragma is required to compile properly.
    mResources.EXT_shader_texture_lod = 1;
    InitializeCompiler();
    TestShaderExtension(&TextureLODShdr[1], 1, false);
}

TEST_F(ShaderExtensionTest, TextureLODExtCompiles)
{
    // Test that it compiles properly with extension enabled.
    mResources.EXT_shader_texture_lod = 1;
    InitializeCompiler();
    TestShaderExtension(TextureLODShdr, 2, true);
}

TEST_F(ShaderExtensionTest, TextureLODExtResetsInternalStates)
{
    // Test that extension internal states are reset properly between compiles.
    mResources.EXT_shader_texture_lod = 1;
    InitializeCompiler();

    TestShaderExtension(&TextureLODShdr[1], 1, false);
    TestShaderExtension(TextureLODShdr, 2, true);
    TestShaderExtension(&TextureLODShdr[1], 1, false);
    TestShaderExtension(TextureLODShdr, 2, true);
}

// Test a bug where we could modify the value of a builtin variable.
TEST_F(ShaderExtensionTest, BuiltinRewritingBug)
{
    mResources.MaxDrawBuffers = 4;
    mResources.EXT_draw_buffers = 1;
    InitializeCompiler();

    const std::string &shaderString =
        "#extension GL_EXT_draw_buffers : require\n"
        "precision mediump float;\n"
        "void main() {\n"
        "    gl_FragData[gl_MaxDrawBuffers] = vec4(0.0);\n"
        "}";

    const char *shaderStrings[] = { shaderString.c_str() };

    TestShaderExtension(shaderStrings, 1, false);
    TestShaderExtension(shaderStrings, 1, false);
}
