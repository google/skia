//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//
// Definition of the in-memory high-level intermediate representation
// of shaders.  This is a tree that parser creates.
//
// Nodes in the tree are defined as a hierarchy of classes derived from
// TIntermNode. Each is a node in a tree.  There is no preset branching factor;
// each node can have it's own type of list of children.
//

#ifndef COMPILER_TRANSLATOR_INTERMNODE_H_
#define COMPILER_TRANSLATOR_INTERMNODE_H_

#include "GLSLANG/ShaderLang.h"

#include <algorithm>
#include <queue>

#include "common/angleutils.h"
#include "compiler/translator/Common.h"
#include "compiler/translator/ConstantUnion.h"
#include "compiler/translator/Operator.h"
#include "compiler/translator/Types.h"

class TIntermTraverser;
class TIntermAggregate;
class TIntermBinary;
class TIntermUnary;
class TIntermConstantUnion;
class TIntermSelection;
class TIntermSwitch;
class TIntermCase;
class TIntermTyped;
class TIntermSymbol;
class TIntermLoop;
class TInfoSink;
class TInfoSinkBase;
class TIntermRaw;

class TSymbolTable;

// Encapsulate an identifier string and track whether it is coming from the original shader code
// (not internal) or from ANGLE (internal). Usually internal names shouldn't be decorated or hashed.
class TName
{
  public:
    POOL_ALLOCATOR_NEW_DELETE();
    explicit TName(const TString &name) : mName(name), mIsInternal(false) {}
    TName() : mName(), mIsInternal(false) {}
    TName(const TName &) = default;
    TName &operator=(const TName &) = default;

    const TString &getString() const { return mName; }
    void setString(const TString &string) { mName = string; }
    bool isInternal() const { return mIsInternal; }
    void setInternal(bool isInternal) { mIsInternal = isInternal; }

  private:
    TString mName;
    bool mIsInternal;
};

//
// Base class for the tree nodes
//
class TIntermNode : angle::NonCopyable
{
  public:
    POOL_ALLOCATOR_NEW_DELETE();
    TIntermNode()
    {
        // TODO: Move this to TSourceLoc constructor
        // after getting rid of TPublicType.
        mLine.first_file = mLine.last_file = 0;
        mLine.first_line = mLine.last_line = 0;
    }
    virtual ~TIntermNode() { }

    const TSourceLoc &getLine() const { return mLine; }
    void setLine(const TSourceLoc &l) { mLine = l; }

    virtual void traverse(TIntermTraverser *) = 0;
    virtual TIntermTyped *getAsTyped() { return 0; }
    virtual TIntermConstantUnion *getAsConstantUnion() { return 0; }
    virtual TIntermAggregate *getAsAggregate() { return 0; }
    virtual TIntermBinary *getAsBinaryNode() { return 0; }
    virtual TIntermUnary *getAsUnaryNode() { return 0; }
    virtual TIntermSelection *getAsSelectionNode() { return 0; }
    virtual TIntermSwitch *getAsSwitchNode() { return 0; }
    virtual TIntermCase *getAsCaseNode() { return 0; }
    virtual TIntermSymbol *getAsSymbolNode() { return 0; }
    virtual TIntermLoop *getAsLoopNode() { return 0; }
    virtual TIntermRaw *getAsRawNode() { return 0; }

    // Replace a child node. Return true if |original| is a child
    // node and it is replaced; otherwise, return false.
    virtual bool replaceChildNode(
        TIntermNode *original, TIntermNode *replacement) = 0;

  protected:
    TSourceLoc mLine;
};

//
// This is just to help yacc.
//
struct TIntermNodePair
{
    TIntermNode *node1;
    TIntermNode *node2;
};

//
// Intermediate class for nodes that have a type.
//
class TIntermTyped : public TIntermNode
{
  public:
    TIntermTyped(const TType &t) : mType(t)  { }

    virtual TIntermTyped *deepCopy() const = 0;

    TIntermTyped *getAsTyped() override { return this; }

    virtual bool hasSideEffects() const = 0;

    void setType(const TType &t) { mType = t; }
    void setTypePreservePrecision(const TType &t);
    const TType &getType() const { return mType; }
    TType *getTypePointer() { return &mType; }

    TBasicType getBasicType() const { return mType.getBasicType(); }
    TQualifier getQualifier() const { return mType.getQualifier(); }
    TPrecision getPrecision() const { return mType.getPrecision(); }
    int getCols() const { return mType.getCols(); }
    int getRows() const { return mType.getRows(); }
    int getNominalSize() const { return mType.getNominalSize(); }
    int getSecondarySize() const { return mType.getSecondarySize(); }

    bool isInterfaceBlock() const { return mType.isInterfaceBlock(); }
    bool isMatrix() const { return mType.isMatrix(); }
    bool isArray()  const { return mType.isArray(); }
    bool isVector() const { return mType.isVector(); }
    bool isScalar() const { return mType.isScalar(); }
    bool isScalarInt() const { return mType.isScalarInt(); }
    const char *getBasicString() const { return mType.getBasicString(); }
    TString getCompleteString() const { return mType.getCompleteString(); }

    int getArraySize() const { return mType.getArraySize(); }

  protected:
    TType mType;

    TIntermTyped(const TIntermTyped &node);
};

//
// Handle for, do-while, and while loops.
//
enum TLoopType
{
    ELoopFor,
    ELoopWhile,
    ELoopDoWhile
};

class TIntermLoop : public TIntermNode
{
  public:
    TIntermLoop(TLoopType type,
                TIntermNode *init, TIntermTyped *cond, TIntermTyped *expr,
                TIntermNode *body)
        : mType(type),
          mInit(init),
          mCond(cond),
          mExpr(expr),
          mBody(body),
          mUnrollFlag(false) { }

    TIntermLoop *getAsLoopNode() override { return this; }
    void traverse(TIntermTraverser *it) override;
    bool replaceChildNode(TIntermNode *original, TIntermNode *replacement) override;

    TLoopType getType() const { return mType; }
    TIntermNode *getInit() { return mInit; }
    TIntermTyped *getCondition() { return mCond; }
    TIntermTyped *getExpression() { return mExpr; }
    TIntermNode *getBody() { return mBody; }

    void setUnrollFlag(bool flag) { mUnrollFlag = flag; }
    bool getUnrollFlag() const { return mUnrollFlag; }

  protected:
    TLoopType mType;
    TIntermNode *mInit;  // for-loop initialization
    TIntermTyped *mCond; // loop exit condition
    TIntermTyped *mExpr; // for-loop expression
    TIntermNode *mBody;  // loop body

    bool mUnrollFlag; // Whether the loop should be unrolled or not.
};

//
// Handle break, continue, return, and kill.
//
class TIntermBranch : public TIntermNode
{
  public:
    TIntermBranch(TOperator op, TIntermTyped *e)
        : mFlowOp(op),
          mExpression(e) { }

    void traverse(TIntermTraverser *it) override;
    bool replaceChildNode(TIntermNode *original, TIntermNode *replacement) override;

    TOperator getFlowOp() { return mFlowOp; }
    TIntermTyped* getExpression() { return mExpression; }

protected:
    TOperator mFlowOp;
    TIntermTyped *mExpression;  // non-zero except for "return exp;" statements
};

//
// Nodes that correspond to symbols or constants in the source code.
//
class TIntermSymbol : public TIntermTyped
{
  public:
    // if symbol is initialized as symbol(sym), the memory comes from the poolallocator of sym.
    // If sym comes from per process globalpoolallocator, then it causes increased memory usage
    // per compile it is essential to use "symbol = sym" to assign to symbol
    TIntermSymbol(int id, const TString &symbol, const TType &type)
        : TIntermTyped(type), mId(id), mSymbol(symbol)
    {
    }

    TIntermTyped *deepCopy() const override { return new TIntermSymbol(*this); }

    bool hasSideEffects() const override { return false; }

    int getId() const { return mId; }
    const TString &getSymbol() const { return mSymbol.getString(); }
    const TName &getName() const { return mSymbol; }

    void setId(int newId) { mId = newId; }

    void setInternal(bool internal) { mSymbol.setInternal(internal); }

    void traverse(TIntermTraverser *it) override;
    TIntermSymbol *getAsSymbolNode() override { return this; }
    bool replaceChildNode(TIntermNode *, TIntermNode *) override { return false; }

  protected:
    int mId;
    TName mSymbol;

  private:
    TIntermSymbol(const TIntermSymbol &) = default;  // Note: not deleted, just private!
};

// A Raw node stores raw code, that the translator will insert verbatim
// into the output stream. Useful for transformation operations that make
// complex code that might not fit naturally into the GLSL model.
class TIntermRaw : public TIntermTyped
{
  public:
    TIntermRaw(const TType &type, const TString &rawText)
        : TIntermTyped(type),
          mRawText(rawText) { }
    TIntermRaw(const TIntermRaw &) = delete;

    TIntermTyped *deepCopy() const override
    {
        UNREACHABLE();
        return nullptr;
    }

    bool hasSideEffects() const override { return false; }

    TString getRawText() const { return mRawText; }

    void traverse(TIntermTraverser *it) override;

    TIntermRaw *getAsRawNode() override { return this; }
    bool replaceChildNode(TIntermNode *, TIntermNode *) override { return false; }

  protected:
    TString mRawText;
};

class TIntermConstantUnion : public TIntermTyped
{
  public:
    TIntermConstantUnion(TConstantUnion *unionPointer, const TType &type)
        : TIntermTyped(type),
          mUnionArrayPointer(unionPointer) { }

    TIntermTyped *deepCopy() const override { return new TIntermConstantUnion(*this); }

    bool hasSideEffects() const override { return false; }

    const TConstantUnion *getUnionArrayPointer() const { return mUnionArrayPointer; }
    TConstantUnion *getUnionArrayPointer() { return mUnionArrayPointer; }

    int getIConst(size_t index) const
    {
        return mUnionArrayPointer ? mUnionArrayPointer[index].getIConst() : 0;
    }
    unsigned int getUConst(size_t index) const
    {
        return mUnionArrayPointer ? mUnionArrayPointer[index].getUConst() : 0;
    }
    float getFConst(size_t index) const
    {
        return mUnionArrayPointer ? mUnionArrayPointer[index].getFConst() : 0.0f;
    }
    bool getBConst(size_t index) const
    {
        return mUnionArrayPointer ? mUnionArrayPointer[index].getBConst() : false;
    }

    void replaceConstantUnion(TConstantUnion *safeConstantUnion)
    {
        // Previous union pointer freed on pool deallocation.
        mUnionArrayPointer = safeConstantUnion;
    }

    TIntermConstantUnion *getAsConstantUnion() override { return this; }
    void traverse(TIntermTraverser *it) override;
    bool replaceChildNode(TIntermNode *, TIntermNode *) override { return false; }

    TConstantUnion *foldBinary(TOperator op, TIntermConstantUnion *rightNode, TInfoSink &infoSink);
    TConstantUnion *foldUnaryWithDifferentReturnType(TOperator op, TInfoSink &infoSink);
    TConstantUnion *foldUnaryWithSameReturnType(TOperator op, TInfoSink &infoSink);

    static TConstantUnion *FoldAggregateBuiltIn(TIntermAggregate *aggregate, TInfoSink &infoSink);

  protected:
    TConstantUnion *mUnionArrayPointer;

  private:
    typedef float(*FloatTypeUnaryFunc) (float);
    bool foldFloatTypeUnary(const TConstantUnion &parameter, FloatTypeUnaryFunc builtinFunc, TInfoSink &infoSink, TConstantUnion *result) const;

    TIntermConstantUnion(const TIntermConstantUnion &node);  // Note: not deleted, just private!
};

//
// Intermediate class for node types that hold operators.
//
class TIntermOperator : public TIntermTyped
{
  public:
    TOperator getOp() const { return mOp; }
    void setOp(TOperator op) { mOp = op; }

    bool isAssignment() const;
    bool isMultiplication() const;
    bool isConstructor() const;

    bool hasSideEffects() const override { return isAssignment(); }

  protected:
    TIntermOperator(TOperator op)
        : TIntermTyped(TType(EbtFloat, EbpUndefined)),
          mOp(op) {}
    TIntermOperator(TOperator op, const TType &type)
        : TIntermTyped(type),
          mOp(op) {}

    TIntermOperator(const TIntermOperator &) = default;

    TOperator mOp;
};

//
// Nodes for all the basic binary math operators.
//
class TIntermBinary : public TIntermOperator
{
  public:
    TIntermBinary(TOperator op)
        : TIntermOperator(op),
          mAddIndexClamp(false) {}

    TIntermTyped *deepCopy() const override { return new TIntermBinary(*this); }

    TIntermBinary *getAsBinaryNode() override { return this; };
    void traverse(TIntermTraverser *it) override;
    bool replaceChildNode(TIntermNode *original, TIntermNode *replacement) override;

    bool hasSideEffects() const override
    {
        return isAssignment() || mLeft->hasSideEffects() || mRight->hasSideEffects();
    }

    void setLeft(TIntermTyped *node) { mLeft = node; }
    void setRight(TIntermTyped *node) { mRight = node; }
    TIntermTyped *getLeft() const { return mLeft; }
    TIntermTyped *getRight() const { return mRight; }
    bool promote(TInfoSink &);
    TIntermTyped *fold(TInfoSink &infoSink);

    void setAddIndexClamp() { mAddIndexClamp = true; }
    bool getAddIndexClamp() { return mAddIndexClamp; }

  protected:
    TIntermTyped* mLeft;
    TIntermTyped* mRight;

    // If set to true, wrap any EOpIndexIndirect with a clamp to bounds.
    bool mAddIndexClamp;

  private:
    TIntermBinary(const TIntermBinary &node);  // Note: not deleted, just private!
};

//
// Nodes for unary math operators.
//
class TIntermUnary : public TIntermOperator
{
  public:
    TIntermUnary(TOperator op, const TType &type)
        : TIntermOperator(op, type),
          mOperand(NULL),
          mUseEmulatedFunction(false) {}
    TIntermUnary(TOperator op)
        : TIntermOperator(op),
          mOperand(NULL),
          mUseEmulatedFunction(false) {}

    TIntermTyped *deepCopy() const override { return new TIntermUnary(*this); }

    void traverse(TIntermTraverser *it) override;
    TIntermUnary *getAsUnaryNode() override { return this; }
    bool replaceChildNode(TIntermNode *original, TIntermNode *replacement) override;

    bool hasSideEffects() const override { return isAssignment() || mOperand->hasSideEffects(); }

    void setOperand(TIntermTyped *operand) { mOperand = operand; }
    TIntermTyped *getOperand() { return mOperand; }
    void promote(const TType *funcReturnType);
    TIntermTyped *fold(TInfoSink &infoSink);

    void setUseEmulatedFunction() { mUseEmulatedFunction = true; }
    bool getUseEmulatedFunction() { return mUseEmulatedFunction; }

  protected:
    TIntermTyped *mOperand;

    // If set to true, replace the built-in function call with an emulated one
    // to work around driver bugs.
    bool mUseEmulatedFunction;

  private:
    TIntermUnary(const TIntermUnary &node);  // note: not deleted, just private!
};

typedef TVector<TIntermNode *> TIntermSequence;
typedef TVector<int> TQualifierList;

//
// Nodes that operate on an arbitrary sized set of children.
//
class TIntermAggregate : public TIntermOperator
{
  public:
    TIntermAggregate()
        : TIntermOperator(EOpNull),
          mUserDefined(false),
          mUseEmulatedFunction(false),
          mGotPrecisionFromChildren(false)
    {
    }
    TIntermAggregate(TOperator op)
        : TIntermOperator(op),
          mUseEmulatedFunction(false),
          mGotPrecisionFromChildren(false)
    {
    }
    ~TIntermAggregate() { }

    // Note: only supported for nodes that can be a part of an expression.
    TIntermTyped *deepCopy() const override { return new TIntermAggregate(*this); }

    TIntermAggregate *getAsAggregate() override { return this; }
    void traverse(TIntermTraverser *it) override;
    bool replaceChildNode(TIntermNode *original, TIntermNode *replacement) override;
    bool replaceChildNodeWithMultiple(TIntermNode *original, TIntermSequence replacements);
    bool insertChildNodes(TIntermSequence::size_type position, TIntermSequence insertions);
    // Conservatively assume function calls and other aggregate operators have side-effects
    bool hasSideEffects() const override { return true; }
    TIntermTyped *fold(TInfoSink &infoSink);

    TIntermSequence *getSequence() { return &mSequence; }

    void setNameObj(const TName &name) { mName = name; }
    const TName &getNameObj() const { return mName; }

    void setName(const TString &name) { mName.setString(name); }
    const TString &getName() const { return mName.getString(); }

    void setUserDefined() { mUserDefined = true; }
    bool isUserDefined() const { return mUserDefined; }

    void setFunctionId(int functionId) { mFunctionId = functionId; }
    int getFunctionId() const { return mFunctionId; }

    void setUseEmulatedFunction() { mUseEmulatedFunction = true; }
    bool getUseEmulatedFunction() { return mUseEmulatedFunction; }

    void setPrecisionFromChildren();
    void setBuiltInFunctionPrecision();

    // Returns true if changing parameter precision may affect the return value.
    bool gotPrecisionFromChildren() const { return mGotPrecisionFromChildren; }

  protected:
    TIntermSequence mSequence;
    TName mName;
    bool mUserDefined; // used for user defined function names
    int mFunctionId;

    // If set to true, replace the built-in function call with an emulated one
    // to work around driver bugs.
    bool mUseEmulatedFunction;

    bool mGotPrecisionFromChildren;

  private:
    TIntermAggregate(const TIntermAggregate &node);  // note: not deleted, just private!
};

//
// For if tests.
//
class TIntermSelection : public TIntermTyped
{
  public:
    TIntermSelection(TIntermTyped *cond, TIntermNode *trueB, TIntermNode *falseB)
        : TIntermTyped(TType(EbtVoid, EbpUndefined)),
          mCondition(cond),
          mTrueBlock(trueB),
          mFalseBlock(falseB) {}
    TIntermSelection(TIntermTyped *cond, TIntermNode *trueB, TIntermNode *falseB,
                     const TType &type)
        : TIntermTyped(type),
          mCondition(cond),
          mTrueBlock(trueB),
          mFalseBlock(falseB) {}

    // Note: only supported for ternary operator nodes.
    TIntermTyped *deepCopy() const override { return new TIntermSelection(*this); }

    void traverse(TIntermTraverser *it) override;
    bool replaceChildNode(TIntermNode *original, TIntermNode *replacement) override;

    // Conservatively assume selections have side-effects
    bool hasSideEffects() const override { return true; }

    bool usesTernaryOperator() const { return getBasicType() != EbtVoid; }
    TIntermNode *getCondition() const { return mCondition; }
    TIntermNode *getTrueBlock() const { return mTrueBlock; }
    TIntermNode *getFalseBlock() const { return mFalseBlock; }
    TIntermSelection *getAsSelectionNode() override { return this; }

  protected:
    TIntermTyped *mCondition;
    TIntermNode *mTrueBlock;
    TIntermNode *mFalseBlock;

  private:
    TIntermSelection(const TIntermSelection &node);  // Note: not deleted, just private!
};

//
// Switch statement.
//
class TIntermSwitch : public TIntermNode
{
  public:
    TIntermSwitch(TIntermTyped *init, TIntermAggregate *statementList)
        : TIntermNode(),
          mInit(init),
          mStatementList(statementList)
    {
    }

    void traverse(TIntermTraverser *it) override;
    bool replaceChildNode(
        TIntermNode *original, TIntermNode *replacement) override;

    TIntermSwitch *getAsSwitchNode() override { return this; }

    TIntermTyped *getInit() { return mInit; }
    TIntermAggregate *getStatementList() { return mStatementList; }
    void setStatementList(TIntermAggregate *statementList) { mStatementList = statementList; }

  protected:
    TIntermTyped *mInit;
    TIntermAggregate *mStatementList;
};

//
// Case label.
//
class TIntermCase : public TIntermNode
{
  public:
    TIntermCase(TIntermTyped *condition)
        : TIntermNode(),
          mCondition(condition)
    {
    }

    void traverse(TIntermTraverser *it) override;
    bool replaceChildNode(
        TIntermNode *original, TIntermNode *replacement) override;

    TIntermCase *getAsCaseNode() override { return this; }

    bool hasCondition() const { return mCondition != nullptr; }
    TIntermTyped *getCondition() const { return mCondition; }

  protected:
    TIntermTyped *mCondition;
};

enum Visit
{
    PreVisit,
    InVisit,
    PostVisit
};

//
// For traversing the tree.  User should derive from this class overriding the visit functions,
// and then pass an object of the subclass to a traverse method of a node.
//
// The traverse*() functions may also be overridden do other bookkeeping on the tree to provide
// contextual information to the visit functions, such as whether the node is the target of an
// assignment.
//
// When using this, just fill in the methods for nodes you want visited.
// Return false from a pre-visit to skip visiting that node's subtree.
//
class TIntermTraverser : angle::NonCopyable
{
  public:
    POOL_ALLOCATOR_NEW_DELETE();
    TIntermTraverser(bool preVisit, bool inVisit, bool postVisit)
        : preVisit(preVisit),
          inVisit(inVisit),
          postVisit(postVisit),
          mDepth(0),
          mMaxDepth(0),
          mTemporaryIndex(nullptr)
    {
    }
    virtual ~TIntermTraverser() {}

    virtual void visitSymbol(TIntermSymbol *node) {}
    virtual void visitRaw(TIntermRaw *node) {}
    virtual void visitConstantUnion(TIntermConstantUnion *node) {}
    virtual bool visitBinary(Visit visit, TIntermBinary *node) { return true; }
    virtual bool visitUnary(Visit visit, TIntermUnary *node) { return true; }
    virtual bool visitSelection(Visit visit, TIntermSelection *node) { return true; }
    virtual bool visitSwitch(Visit visit, TIntermSwitch *node) { return true; }
    virtual bool visitCase(Visit visit, TIntermCase *node) { return true; }
    virtual bool visitAggregate(Visit visit, TIntermAggregate *node) { return true; }
    virtual bool visitLoop(Visit visit, TIntermLoop *node) { return true; }
    virtual bool visitBranch(Visit visit, TIntermBranch *node) { return true; }

    // The traverse functions contain logic for iterating over the children of the node
    // and calling the visit functions in the appropriate places. They also track some
    // context that may be used by the visit functions.
    virtual void traverseSymbol(TIntermSymbol *node);
    virtual void traverseRaw(TIntermRaw *node);
    virtual void traverseConstantUnion(TIntermConstantUnion *node);
    virtual void traverseBinary(TIntermBinary *node);
    virtual void traverseUnary(TIntermUnary *node);
    virtual void traverseSelection(TIntermSelection *node);
    virtual void traverseSwitch(TIntermSwitch *node);
    virtual void traverseCase(TIntermCase *node);
    virtual void traverseAggregate(TIntermAggregate *node);
    virtual void traverseLoop(TIntermLoop *node);
    virtual void traverseBranch(TIntermBranch *node);

    int getMaxDepth() const { return mMaxDepth; }

    // Return the original name if hash function pointer is NULL;
    // otherwise return the hashed name.
    static TString hash(const TString &name, ShHashFunction64 hashFunction);

    // If traversers need to replace nodes, they can add the replacements in
    // mReplacements/mMultiReplacements during traversal and the user of the traverser should call
    // this function after traversal to perform them.
    void updateTree();

    // Start creating temporary symbols from the given temporary symbol index + 1.
    void useTemporaryIndex(unsigned int *temporaryIndex);

  protected:
    void incrementDepth(TIntermNode *current)
    {
        mDepth++;
        mMaxDepth = std::max(mMaxDepth, mDepth);
        mPath.push_back(current);
    }

    void decrementDepth()
    {
        mDepth--;
        mPath.pop_back();
    }

    TIntermNode *getParentNode()
    {
        return mPath.size() == 0 ? NULL : mPath.back();
    }

    void pushParentBlock(TIntermAggregate *node);
    void incrementParentBlockPos();
    void popParentBlock();

    bool parentNodeIsBlock()
    {
        return !mParentBlockStack.empty() && getParentNode() == mParentBlockStack.back().node;
    }

    const bool preVisit;
    const bool inVisit;
    const bool postVisit;

    int mDepth;
    int mMaxDepth;

    // All the nodes from root to the current node's parent during traversing.
    TVector<TIntermNode *> mPath;

    // To replace a single node with another on the parent node
    struct NodeUpdateEntry
    {
        NodeUpdateEntry(TIntermNode *_parent,
                        TIntermNode *_original,
                        TIntermNode *_replacement,
                        bool _originalBecomesChildOfReplacement)
            : parent(_parent),
              original(_original),
              replacement(_replacement),
              originalBecomesChildOfReplacement(_originalBecomesChildOfReplacement) {}

        TIntermNode *parent;
        TIntermNode *original;
        TIntermNode *replacement;
        bool originalBecomesChildOfReplacement;
    };

    // To replace a single node with multiple nodes on the parent aggregate node
    struct NodeReplaceWithMultipleEntry
    {
        NodeReplaceWithMultipleEntry(TIntermAggregate *_parent, TIntermNode *_original, TIntermSequence _replacements)
            : parent(_parent),
              original(_original),
              replacements(_replacements)
        {
        }

        TIntermAggregate *parent;
        TIntermNode *original;
        TIntermSequence replacements;
    };

    // To insert multiple nodes on the parent aggregate node
    struct NodeInsertMultipleEntry
    {
        NodeInsertMultipleEntry(TIntermAggregate *_parent,
                                TIntermSequence::size_type _position,
                                TIntermSequence _insertionsBefore,
                                TIntermSequence _insertionsAfter)
            : parent(_parent),
              position(_position),
              insertionsBefore(_insertionsBefore),
              insertionsAfter(_insertionsAfter)
        {
        }

        TIntermAggregate *parent;
        TIntermSequence::size_type position;
        TIntermSequence insertionsBefore;
        TIntermSequence insertionsAfter;
    };

    // During traversing, save all the changes that need to happen into
    // mReplacements/mMultiReplacements, then do them by calling updateTree().
    // Multi replacements are processed after single replacements.
    std::vector<NodeUpdateEntry> mReplacements;
    std::vector<NodeReplaceWithMultipleEntry> mMultiReplacements;
    std::vector<NodeInsertMultipleEntry> mInsertions;

    // Helper to insert statements in the parent block (sequence) of the node currently being traversed.
    // The statements will be inserted before the node being traversed once updateTree is called.
    // Should only be called during PreVisit or PostVisit from sequence nodes.
    // Note that inserting more than one set of nodes to the same parent node on a single updateTree call is not
    // supported.
    void insertStatementsInParentBlock(const TIntermSequence &insertions);

    // Same as above, but supports simultaneous insertion of statements before and after the node
    // currently being traversed.
    void insertStatementsInParentBlock(const TIntermSequence &insertionsBefore,
                                       const TIntermSequence &insertionsAfter);

    // Helper to create a temporary symbol node with the given qualifier.
    TIntermSymbol *createTempSymbol(const TType &type, TQualifier qualifier);
    // Helper to create a temporary symbol node.
    TIntermSymbol *createTempSymbol(const TType &type);
    // Create a node that declares but doesn't initialize a temporary symbol.
    TIntermAggregate *createTempDeclaration(const TType &type);
    // Create a node that initializes the current temporary symbol with initializer having the given qualifier.
    TIntermAggregate *createTempInitDeclaration(TIntermTyped *initializer, TQualifier qualifier);
    // Create a node that initializes the current temporary symbol with initializer.
    TIntermAggregate *createTempInitDeclaration(TIntermTyped *initializer);
    // Create a node that assigns rightNode to the current temporary symbol.
    TIntermBinary *createTempAssignment(TIntermTyped *rightNode);
    // Increment temporary symbol index.
    void nextTemporaryIndex();

  private:
    struct ParentBlock
    {
        ParentBlock(TIntermAggregate *nodeIn, TIntermSequence::size_type posIn)
            : node(nodeIn),
              pos(posIn)
        {
        }

        TIntermAggregate *node;
        TIntermSequence::size_type pos;
    };
    // All the code blocks from the root to the current node's parent during traversal.
    std::vector<ParentBlock> mParentBlockStack;

    unsigned int *mTemporaryIndex;
};

// Traverser parent class that tracks where a node is a destination of a write operation and so is
// required to be an l-value.
class TLValueTrackingTraverser : public TIntermTraverser
{
  public:
    TLValueTrackingTraverser(bool preVisit,
                             bool inVisit,
                             bool postVisit,
                             const TSymbolTable &symbolTable,
                             int shaderVersion)
        : TIntermTraverser(preVisit, inVisit, postVisit),
          mOperatorRequiresLValue(false),
          mInFunctionCallOutParameter(false),
          mSymbolTable(symbolTable),
          mShaderVersion(shaderVersion)
    {
    }
    virtual ~TLValueTrackingTraverser() {}

    void traverseBinary(TIntermBinary *node) override;
    void traverseUnary(TIntermUnary *node) override;
    void traverseAggregate(TIntermAggregate *node) override;

  protected:
    bool isLValueRequiredHere() const
    {
        return mOperatorRequiresLValue || mInFunctionCallOutParameter;
    }

    // Return true if the prototype or definition of the function being called has been encountered
    // during traversal.
    bool isInFunctionMap(const TIntermAggregate *callNode) const;

  private:
    // Track whether an l-value is required in the node that is currently being traversed by the
    // surrounding operator.
    // Use isLValueRequiredHere to check all conditions which require an l-value.
    void setOperatorRequiresLValue(bool lValueRequired)
    {
        mOperatorRequiresLValue = lValueRequired;
    }
    bool operatorRequiresLValue() const { return mOperatorRequiresLValue; }

    // Add a function encountered during traversal to the function map.
    void addToFunctionMap(const TString &name, TIntermSequence *paramSequence);

    // Return the parameters sequence from the function definition or prototype.
    TIntermSequence *getFunctionParameters(const TIntermAggregate *callNode);

    // Track whether an l-value is required inside a function call.
    void setInFunctionCallOutParameter(bool inOutParameter);
    bool isInFunctionCallOutParameter() const;

    bool mOperatorRequiresLValue;
    bool mInFunctionCallOutParameter;

    struct TStringComparator
    {
        bool operator()(const TString &a, const TString &b) const { return a.compare(b) < 0; }
    };

    // Map from mangled function names to their parameter sequences
    TMap<TString, TIntermSequence *, TStringComparator> mFunctionMap;

    const TSymbolTable &mSymbolTable;
    const int mShaderVersion;
};

//
// For traversing the tree, and computing max depth.
// Takes a maximum depth limit to prevent stack overflow.
//
class TMaxDepthTraverser : public TIntermTraverser
{
  public:
    POOL_ALLOCATOR_NEW_DELETE();
    TMaxDepthTraverser(int depthLimit)
        : TIntermTraverser(true, true, false),
          mDepthLimit(depthLimit) { }

    bool visitBinary(Visit, TIntermBinary *) override { return depthCheck(); }
    bool visitUnary(Visit, TIntermUnary *) override { return depthCheck(); }
    bool visitSelection(Visit, TIntermSelection *) override { return depthCheck(); }
    bool visitAggregate(Visit, TIntermAggregate *) override { return depthCheck(); }
    bool visitLoop(Visit, TIntermLoop *) override { return depthCheck(); }
    bool visitBranch(Visit, TIntermBranch *) override { return depthCheck(); }

  protected:
    bool depthCheck() const { return mMaxDepth < mDepthLimit; }

    int mDepthLimit;
};

#endif  // COMPILER_TRANSLATOR_INTERMNODE_H_
