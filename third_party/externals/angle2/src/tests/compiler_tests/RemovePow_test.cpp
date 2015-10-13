//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RemovePow_test.cpp:
//   Tests for removing pow() function calls from the AST.
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"
#include "compiler/translator/NodeSearch.h"
#include "compiler/translator/TranslatorGLSL.h"

class RemovePowTest : public testing::Test
{
  public:
    RemovePowTest() {}

  protected:
    void SetUp() override
    {
        allocator.push();
        SetGlobalPoolAllocator(&allocator);
        ShBuiltInResources resources;
        ShInitBuiltInResources(&resources);
        mTranslatorGLSL = new TranslatorGLSL(GL_FRAGMENT_SHADER, SH_GLES2_SPEC, SH_GLSL_COMPATIBILITY_OUTPUT);
        ASSERT_TRUE(mTranslatorGLSL->Init(resources));
    }

    void TearDown() override
    {
        SafeDelete(mTranslatorGLSL);
        SetGlobalPoolAllocator(nullptr);
        allocator.pop();
    }

    void compile(const std::string& shaderString)
    {
        const char *shaderStrings[] = { shaderString.c_str() };
        mASTRoot = mTranslatorGLSL->compileTreeForTesting(shaderStrings, 1,
                                                          SH_OBJECT_CODE | SH_REMOVE_POW_WITH_CONSTANT_EXPONENT);
        if (!mASTRoot)
        {
            TInfoSink &infoSink = mTranslatorGLSL->getInfoSink();
            FAIL() << "Shader compilation into ESSL failed " << infoSink.info.c_str();
        }
    }

    template <class T>
    bool foundInAST()
    {
        return T::search(mASTRoot);
    }

  private:
    TranslatorGLSL *mTranslatorGLSL;
    TIntermNode *mASTRoot;

    TPoolAllocator allocator;
};

// Check if there's a pow() node anywhere in the tree.
class FindPow : public sh::NodeSearchTraverser<FindPow>
{
  public:
    bool visitBinary(Visit visit, TIntermBinary *node) override
    {
        if (node->getOp() == EOpPow)
        {
            mFound = true;
        }
        return !mFound;
    }
};

// Check if the tree starting at node corresponds to exp2(y * log2(x))
// If the tree matches, set base to the node corresponding to x.
bool IsPowWorkaround(TIntermNode *node, TIntermNode **base)
{
    TIntermUnary *exp = node->getAsUnaryNode();
    if (exp != nullptr && exp->getOp() == EOpExp2)
    {
        TIntermBinary *mul = exp->getOperand()->getAsBinaryNode();
        if (mul != nullptr && mul->isMultiplication())
        {
            TIntermUnary *log = mul->getRight()->getAsUnaryNode();
            if (mul->getLeft()->getAsConstantUnion() && log != nullptr)
            {
                if (log->getOp() == EOpLog2)
                {
                    if (base)
                        *base = log->getOperand();
                    return true;
                }
            }
        }
    }
    return false;
}

// Check if there's a node with the correct workaround to pow anywhere in the tree.
class FindPowWorkaround : public sh::NodeSearchTraverser<FindPowWorkaround>
{
  public:
    bool visitUnary(Visit visit, TIntermUnary *node) override
    {
        mFound = IsPowWorkaround(node, nullptr);
        return !mFound;
    }
};

// Check if there's a node with the correct workaround to pow with another workaround to pow
// nested within it anywhere in the tree.
class FindNestedPowWorkaround : public sh::NodeSearchTraverser<FindNestedPowWorkaround>
{
  public:
    bool visitUnary(Visit visit, TIntermUnary *node) override
    {
        TIntermNode *base = nullptr;
        bool oneFound = IsPowWorkaround(node, &base);
        if (oneFound && base)
            mFound = IsPowWorkaround(base, nullptr);
        return !mFound;
    }
};

TEST_F(RemovePowTest, PowWithConstantExponent)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float u;\n"
        "void main() {\n"
        "   gl_FragColor = pow(vec4(u), vec4(0.5));\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundInAST<FindPow>());
    ASSERT_TRUE(foundInAST<FindPowWorkaround>());
    ASSERT_FALSE(foundInAST<FindNestedPowWorkaround>());
}

TEST_F(RemovePowTest, NestedPowWithConstantExponent)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float u;\n"
        "void main() {\n"
        "   gl_FragColor = pow(pow(vec4(u), vec4(2.0)), vec4(0.5));\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundInAST<FindPow>());
    ASSERT_TRUE(foundInAST<FindNestedPowWorkaround>());
}

