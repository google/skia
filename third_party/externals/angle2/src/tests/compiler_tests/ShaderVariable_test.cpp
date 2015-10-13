//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CollectVariables_test.cpp:
//   Some tests for shader inspection
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"

namespace sh
{

TEST(ShaderVariableTest, FindInfoByMappedName)
{
    // struct A {
    //   float x[2];
    //   vec3 y;
    // };
    // struct B {
    //   A a[3];
    // };
    // B uni[2];
    ShaderVariable uni;
    uni.arraySize = 2;
    uni.name = "uni";
    uni.mappedName = "m_uni";
    uni.structName = "B";
    {
        ShaderVariable a;
        a.arraySize = 3;
        a.name = "a";
        a.mappedName = "m_a";
        a.structName = "A";
        {
            ShaderVariable x(GL_FLOAT, 2);
            x.name = "x";
            x.mappedName = "m_x";
            a.fields.push_back(x);

            ShaderVariable y(GL_FLOAT_VEC3, 0);
            y.name = "y";
            y.mappedName = "m_y";
            a.fields.push_back(y);
        }
        uni.fields.push_back(a);
    }

    const ShaderVariable *leafVar = NULL;
    std::string originalFullName;

    std::string mappedFullName = "wrongName";
    EXPECT_FALSE(uni.findInfoByMappedName(
        mappedFullName, &leafVar, &originalFullName));

    mappedFullName = "m_uni";
    EXPECT_TRUE(uni.findInfoByMappedName(
        mappedFullName, &leafVar, &originalFullName));
    EXPECT_EQ(&uni, leafVar);
    EXPECT_STREQ("uni", originalFullName.c_str());

    mappedFullName = "m_uni[0].m_a[1].wrongName";
    EXPECT_FALSE(uni.findInfoByMappedName(
        mappedFullName, &leafVar, &originalFullName));

    mappedFullName = "m_uni[0].m_a[1].m_x";
    EXPECT_TRUE(uni.findInfoByMappedName(
        mappedFullName, &leafVar, &originalFullName));
    EXPECT_EQ(&(uni.fields[0].fields[0]), leafVar);
    EXPECT_STREQ("uni[0].a[1].x", originalFullName.c_str());

    mappedFullName = "m_uni[0].m_a[1].m_x[0]";
    EXPECT_TRUE(uni.findInfoByMappedName(
        mappedFullName, &leafVar, &originalFullName));
    EXPECT_EQ(&(uni.fields[0].fields[0]), leafVar);
    EXPECT_STREQ("uni[0].a[1].x[0]", originalFullName.c_str());

    mappedFullName = "m_uni[0].m_a[1].m_y";
    EXPECT_TRUE(uni.findInfoByMappedName(
        mappedFullName, &leafVar, &originalFullName));
    EXPECT_EQ(&(uni.fields[0].fields[1]), leafVar);
    EXPECT_STREQ("uni[0].a[1].y", originalFullName.c_str());
}

TEST(ShaderVariableTest, IsSameUniformWithDifferentFieldOrder)
{
    // struct A {
    //   float x;
    //   float y;
    // };
    // uniform A uni;
    Uniform vx_a;
    vx_a.arraySize = 0;
    vx_a.name = "uni";
    vx_a.mappedName = "m_uni";
    vx_a.structName = "A";
    {
        ShaderVariable x(GL_FLOAT, 0);
        x.name = "x";
        x.mappedName = "m_x";
        vx_a.fields.push_back(x);

        ShaderVariable y(GL_FLOAT, 0);
        y.name = "y";
        y.mappedName = "m_y";
        vx_a.fields.push_back(y);
    }

    // struct A {
    //   float y;
    //   float x;
    // };
    // uniform A uni;
    Uniform fx_a;
    fx_a.arraySize = 0;
    fx_a.name = "uni";
    fx_a.mappedName = "m_uni";
    fx_a.structName = "A";
    {
        ShaderVariable y(GL_FLOAT, 0);
        y.name = "y";
        y.mappedName = "m_y";
        fx_a.fields.push_back(y);

        ShaderVariable x(GL_FLOAT, 0);
        x.name = "x";
        x.mappedName = "m_x";
        fx_a.fields.push_back(x);
    }

    EXPECT_FALSE(vx_a.isSameUniformAtLinkTime(fx_a));
}

TEST(ShaderVariableTest, IsSameUniformWithDifferentStructNames)
{
    // struct A {
    //   float x;
    //   float y;
    // };
    // uniform A uni;
    Uniform vx_a;
    vx_a.arraySize = 0;
    vx_a.name = "uni";
    vx_a.mappedName = "m_uni";
    vx_a.structName = "A";
    {
        ShaderVariable x(GL_FLOAT, 0);
        x.name = "x";
        x.mappedName = "m_x";
        vx_a.fields.push_back(x);

        ShaderVariable y(GL_FLOAT, 0);
        y.name = "y";
        y.mappedName = "m_y";
        vx_a.fields.push_back(y);
    }

    // struct B {
    //   float x;
    //   float y;
    // };
    // uniform B uni;
    Uniform fx_a;
    fx_a.arraySize = 0;
    fx_a.name = "uni";
    fx_a.mappedName = "m_uni";
    {
        ShaderVariable x(GL_FLOAT, 0);
        x.name = "x";
        x.mappedName = "m_x";
        fx_a.fields.push_back(x);

        ShaderVariable y(GL_FLOAT, 0);
        y.name = "y";
        y.mappedName = "m_y";
        fx_a.fields.push_back(y);
    }

    fx_a.structName = "B";
    EXPECT_FALSE(vx_a.isSameUniformAtLinkTime(fx_a));

    fx_a.structName = "A";
    EXPECT_TRUE(vx_a.isSameUniformAtLinkTime(fx_a));

    fx_a.structName = "";
    EXPECT_FALSE(vx_a.isSameUniformAtLinkTime(fx_a));
}

TEST(ShaderVariableTest, IsSameVaryingWithDifferentInvariance)
{
    // invariant varying float vary;
    Varying vx;
    vx.type = GL_FLOAT;
    vx.arraySize = 0;
    vx.precision = GL_MEDIUM_FLOAT;
    vx.name = "vary";
    vx.mappedName = "m_vary";
    vx.staticUse = true;
    vx.isInvariant = true;

    // varying float vary;
    Varying fx;
    fx.type = GL_FLOAT;
    fx.arraySize = 0;
    fx.precision = GL_MEDIUM_FLOAT;
    fx.name = "vary";
    fx.mappedName = "m_vary";
    fx.staticUse = true;
    fx.isInvariant = false;

    // Default to ESSL1 behavior: invariance must match
    EXPECT_FALSE(vx.isSameVaryingAtLinkTime(fx));
    EXPECT_FALSE(vx.isSameVaryingAtLinkTime(fx, 100));
    // ESSL3 behavior: invariance doesn't need to match
    EXPECT_TRUE(vx.isSameVaryingAtLinkTime(fx, 300));

    // invariant varying float vary;
    fx.isInvariant = true;
    EXPECT_TRUE(vx.isSameVaryingAtLinkTime(fx));
    EXPECT_TRUE(vx.isSameVaryingAtLinkTime(fx, 100));
    EXPECT_TRUE(vx.isSameVaryingAtLinkTime(fx, 300));
}

// Test that using invariant varyings doesn't trigger a double delete.
TEST(ShaderVariableTest, InvariantDoubleDeleteBug)
{
    ShBuiltInResources resources;
    ShInitBuiltInResources(&resources);

    ShHandle compiler = ShConstructCompiler(GL_VERTEX_SHADER, SH_GLES2_SPEC, SH_GLSL_OUTPUT, &resources);
    EXPECT_NE(static_cast<ShHandle>(0), compiler);

    const char *program[] =
    {
        "attribute vec4 position;\n"
        "varying float v;\n"
        "invariant v;\n"
        "void main() {\n"
        "  v = 1.0;\n"
        "  gl_Position = position;\n"
        "}"
    };

    EXPECT_TRUE(ShCompile(compiler, program, 1, SH_OBJECT_CODE));
    EXPECT_TRUE(ShCompile(compiler, program, 1, SH_OBJECT_CODE));
    ShDestruct(compiler);
}

}  // namespace sh
