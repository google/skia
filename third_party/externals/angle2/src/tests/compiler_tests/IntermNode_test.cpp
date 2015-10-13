//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// IntermNode_test.cpp:
//   Unit tests for the AST node classes.
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "compiler/translator/InfoSink.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/PoolAlloc.h"

class IntermNodeTest : public testing::Test
{
  public:
    IntermNodeTest() : mUniqueIndex(0) {}

  protected:
    void SetUp() override
    {
        allocator.push();
        SetGlobalPoolAllocator(&allocator);
    }

    void TearDown() override
    {
        SetGlobalPoolAllocator(nullptr);
        allocator.pop();
    }

    TIntermSymbol *createTestSymbol(const TType &type)
    {
        TInfoSinkBase symbolNameOut;
        symbolNameOut << "test" << mUniqueIndex;
        TString symbolName = symbolNameOut.c_str();
        ++mUniqueIndex;

        TIntermSymbol *node = new TIntermSymbol(0, symbolName, type);
        node->setLine(createUniqueSourceLoc());
        node->setInternal(true);
        node->getTypePointer()->setQualifier(EvqTemporary);
        return node;
    }

    TIntermSymbol *createTestSymbol()
    {
        TType type(EbtFloat, EbpHigh);
        return createTestSymbol(type);
    }

    void checkTypeEqualWithQualifiers(const TType &original, const TType &copy)
    {
        ASSERT_EQ(original, copy);
        ASSERT_EQ(original.getPrecision(), copy.getPrecision());
        ASSERT_EQ(original.getQualifier(), copy.getQualifier());
    }

    void checkSymbolCopy(TIntermNode *aOriginal, TIntermNode *aCopy)
    {
        ASSERT_NE(aOriginal, aCopy);
        TIntermSymbol *copy     = aCopy->getAsSymbolNode();
        TIntermSymbol *original = aOriginal->getAsSymbolNode();
        ASSERT_NE(nullptr, copy);
        ASSERT_NE(nullptr, original);
        ASSERT_NE(original, copy);
        ASSERT_EQ(original->getId(), copy->getId());
        ASSERT_EQ(original->getName().getString(), copy->getName().getString());
        ASSERT_EQ(original->getName().isInternal(), copy->getName().isInternal());
        checkTypeEqualWithQualifiers(original->getType(), copy->getType());
        ASSERT_EQ(original->getLine().first_file, copy->getLine().first_file);
        ASSERT_EQ(original->getLine().first_line, copy->getLine().first_line);
        ASSERT_EQ(original->getLine().last_file, copy->getLine().last_file);
        ASSERT_EQ(original->getLine().last_line, copy->getLine().last_line);
    }

    TSourceLoc createUniqueSourceLoc()
    {
        TSourceLoc loc;
        loc.first_file = mUniqueIndex;
        loc.first_line = mUniqueIndex + 1;
        loc.last_file  = mUniqueIndex + 2;
        loc.last_line  = mUniqueIndex + 3;
        ++mUniqueIndex;
        return loc;
    }

    static TSourceLoc getTestSourceLoc()
    {
        TSourceLoc loc;
        loc.first_file = 1;
        loc.first_line = 2;
        loc.last_file  = 3;
        loc.last_line  = 4;
        return loc;
    }

    static void checkTestSourceLoc(const TSourceLoc &loc)
    {
        ASSERT_EQ(1, loc.first_file);
        ASSERT_EQ(2, loc.first_line);
        ASSERT_EQ(3, loc.last_file);
        ASSERT_EQ(4, loc.last_line);
    }

  private:
    TPoolAllocator allocator;
    int mUniqueIndex;
};

// Check that the deep copy of a symbol node is an actual copy with the same attributes as the
// original.
TEST_F(IntermNodeTest, DeepCopySymbolNode)
{
    TType type(EbtInt, EbpHigh);
    TIntermSymbol *original = new TIntermSymbol(0, TString("name"), type);
    original->setLine(getTestSourceLoc());
    original->setInternal(true);
    TIntermTyped *copy = original->deepCopy();
    checkSymbolCopy(original, copy);
    checkTestSourceLoc(copy->getLine());
}

// Check that the deep copy of a constant union node is an actual copy with the same attributes as
// the original.
TEST_F(IntermNodeTest, DeepCopyConstantUnionNode)
{
    TType type(EbtInt, EbpHigh);
    TConstantUnion *constValue = new TConstantUnion[1];
    constValue[0].setIConst(101);
    TIntermConstantUnion *original = new TIntermConstantUnion(constValue, type);
    original->setLine(getTestSourceLoc());
    TIntermTyped *copyTyped    = original->deepCopy();
    TIntermConstantUnion *copy = copyTyped->getAsConstantUnion();
    ASSERT_NE(nullptr, copy);
    ASSERT_NE(original, copy);
    checkTestSourceLoc(copy->getLine());
    checkTypeEqualWithQualifiers(original->getType(), copy->getType());
    ASSERT_EQ(101, copy->getIConst(0));
}

// Check that the deep copy of a binary node is an actual copy with the same attributes as the
// original. Child nodes also need to be copies with the same attributes as the original children.
TEST_F(IntermNodeTest, DeepCopyBinaryNode)
{
    TType type(EbtFloat, EbpHigh);

    TIntermBinary *original = new TIntermBinary(EOpAdd);
    original->setLine(getTestSourceLoc());
    original->setLeft(createTestSymbol());
    original->setRight(createTestSymbol());
    TIntermTyped *copyTyped = original->deepCopy();
    TIntermBinary *copy = copyTyped->getAsBinaryNode();
    ASSERT_NE(nullptr, copy);
    ASSERT_NE(original, copy);
    checkTestSourceLoc(copy->getLine());
    checkTypeEqualWithQualifiers(original->getType(), copy->getType());

    checkSymbolCopy(original->getLeft(), copy->getLeft());
    checkSymbolCopy(original->getRight(), copy->getRight());
}

// Check that the deep copy of a unary node is an actual copy with the same attributes as the
// original. The child node also needs to be a copy with the same attributes as the original child.
TEST_F(IntermNodeTest, DeepCopyUnaryNode)
{
    TType type(EbtFloat, EbpHigh);

    TIntermUnary *original = new TIntermUnary(EOpPreIncrement);
    original->setLine(getTestSourceLoc());
    original->setOperand(createTestSymbol());
    TIntermTyped *copyTyped = original->deepCopy();
    TIntermUnary *copy = copyTyped->getAsUnaryNode();
    ASSERT_NE(nullptr, copy);
    ASSERT_NE(original, copy);
    checkTestSourceLoc(copy->getLine());
    checkTypeEqualWithQualifiers(original->getType(), copy->getType());

    checkSymbolCopy(original->getOperand(), copy->getOperand());
}

// Check that the deep copy of an aggregate node is an actual copy with the same attributes as the
// original. Child nodes also need to be copies with the same attributes as the original children.
TEST_F(IntermNodeTest, DeepCopyAggregateNode)
{
    TType type(EbtFloat, EbpHigh);

    TIntermAggregate *original = new TIntermAggregate(EOpMix);
    original->setLine(getTestSourceLoc());
    TIntermSequence *originalSeq = original->getSequence();
    originalSeq->push_back(createTestSymbol());
    originalSeq->push_back(createTestSymbol());
    originalSeq->push_back(createTestSymbol());
    TIntermTyped *copyTyped = original->deepCopy();
    TIntermAggregate *copy = copyTyped->getAsAggregate();
    ASSERT_NE(nullptr, copy);
    ASSERT_NE(original, copy);
    checkTestSourceLoc(copy->getLine());
    checkTypeEqualWithQualifiers(original->getType(), copy->getType());

    ASSERT_EQ(original->getSequence()->size(), copy->getSequence()->size());
    TIntermSequence::size_type i = 0;
    for (auto *copyChild : *copy->getSequence())
    {
        TIntermNode *originalChild = originalSeq->at(i);
        checkSymbolCopy(originalChild, copyChild);
        ++i;
    }
}

// Check that the deep copy of a selection node is an actual copy with the same attributes as the
// original. Child nodes also need to be copies with the same attributes as the original children.
TEST_F(IntermNodeTest, DeepCopySelectionNode)
{
    TType type(EbtFloat, EbpHigh);

    TIntermSelection *original = new TIntermSelection(
        createTestSymbol(TType(EbtBool, EbpUndefined)), createTestSymbol(), createTestSymbol());
    original->setLine(getTestSourceLoc());
    TIntermTyped *copyTyped = original->deepCopy();
    TIntermSelection *copy = copyTyped->getAsSelectionNode();
    ASSERT_NE(nullptr, copy);
    ASSERT_NE(original, copy);
    checkTestSourceLoc(copy->getLine());
    checkTypeEqualWithQualifiers(original->getType(), copy->getType());

    checkSymbolCopy(original->getCondition(), copy->getCondition());
    checkSymbolCopy(original->getTrueBlock(), copy->getTrueBlock());
    checkSymbolCopy(original->getFalseBlock(), copy->getFalseBlock());
}

