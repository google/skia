//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RemoveDynamicIndexing is an AST traverser to remove dynamic indexing of vectors and matrices,
// replacing them with calls to functions that choose which component to return or write.
//

#include "compiler/translator/RemoveDynamicIndexing.h"

#include "compiler/translator/InfoSink.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/SymbolTable.h"

namespace
{

TName GetIndexFunctionName(const TType &type, bool write)
{
    TInfoSinkBase nameSink;
    nameSink << "dyn_index_";
    if (write)
    {
        nameSink << "write_";
    }
    if (type.isMatrix())
    {
        nameSink << "mat" << type.getCols() << "x" << type.getRows();
    }
    else
    {
        switch (type.getBasicType())
        {
            case EbtInt:
                nameSink << "ivec";
                break;
            case EbtBool:
                nameSink << "bvec";
                break;
            case EbtUInt:
                nameSink << "uvec";
                break;
            case EbtFloat:
                nameSink << "vec";
                break;
            default:
                UNREACHABLE();
        }
        nameSink << type.getNominalSize();
    }
    TString nameString = TFunction::mangleName(nameSink.c_str());
    TName name(nameString);
    name.setInternal(true);
    return name;
}

TIntermSymbol *CreateBaseSymbol(const TType &type)
{
    TIntermSymbol *symbol = new TIntermSymbol(0, "base", type);
    symbol->setInternal(true);
    return symbol;
}

TIntermSymbol *CreateIndexSymbol()
{
    TIntermSymbol *symbol = new TIntermSymbol(0, "index", TType(EbtInt, EbpHigh));
    symbol->setInternal(true);
    return symbol;
}

TIntermSymbol *CreateValueSymbol(const TType &type)
{
    TIntermSymbol *symbol = new TIntermSymbol(0, "value", type);
    symbol->setInternal(true);
    return symbol;
}

TIntermConstantUnion *CreateIntConstantNode(int i)
{
    TConstantUnion *constant = new TConstantUnion();
    constant->setIConst(i);
    return new TIntermConstantUnion(constant, TType(EbtInt, EbpHigh));
}

TIntermBinary *CreateIndexDirectBaseSymbolNode(const TType &indexedType,
                                               const TType &fieldType,
                                               const int index)
{
    TIntermBinary *indexNode = new TIntermBinary(EOpIndexDirect);
    indexNode->setType(fieldType);
    indexNode->setLeft(CreateBaseSymbol(indexedType));
    indexNode->setRight(CreateIntConstantNode(index));
    return indexNode;
}

TIntermBinary *CreateAssignValueSymbolNode(TIntermTyped *targetNode, const TType &assignedValueType)
{
    TIntermBinary *assignNode = new TIntermBinary(EOpAssign);
    assignNode->setType(assignedValueType);
    assignNode->setLeft(targetNode);
    assignNode->setRight(CreateValueSymbol(assignedValueType));
    return assignNode;
}

TIntermTyped *EnsureSignedInt(TIntermTyped *node)
{
    if (node->getBasicType() == EbtInt)
        return node;

    TIntermAggregate *convertedNode = new TIntermAggregate(EOpConstructInt);
    convertedNode->setType(TType(EbtInt));
    convertedNode->getSequence()->push_back(node);
    convertedNode->setPrecisionFromChildren();
    return convertedNode;
}

TType GetFieldType(const TType &indexedType)
{
    if (indexedType.isMatrix())
    {
        TType fieldType = TType(indexedType.getBasicType(), indexedType.getPrecision());
        fieldType.setPrimarySize(static_cast<unsigned char>(indexedType.getRows()));
        return fieldType;
    }
    else
    {
        return TType(indexedType.getBasicType(), indexedType.getPrecision());
    }
}

// Generate a read or write function for one field in a vector/matrix.
// Out-of-range indices are clamped. This is consistent with how ANGLE handles out-of-range
// indices in other places.
// Note that indices can be either int or uint. We create only int versions of the functions,
// and convert uint indices to int at the call site.
// read function example:
// float dyn_index_vec2(in vec2 base, in int index)
// {
//    switch(index)
//    {
//      case (0):
//        return base[0];
//      case (1):
//        return base[1];
//      default:
//        break;
//    }
//    if (index < 0)
//      return base[0];
//    return base[1];
// }
// write function example:
// void dyn_index_write_vec2(inout vec2 base, in int index, in float value)
// {
//    switch(index)
//    {
//      case (0):
//        base[0] = value;
//        return;
//      case (1):
//        base[1] = value;
//        return;
//      default:
//        break;
//    }
//    if (index < 0)
//    {
//      base[0] = value;
//      return;
//    }
//    base[1] = value;
// }
// Note that else is not used in above functions to avoid the RewriteElseBlocks transformation.
TIntermAggregate *GetIndexFunctionDefinition(TType type, bool write)
{
    ASSERT(!type.isArray());
    // Conservatively use highp here, even if the indexed type is not highp. That way the code can't
    // end up using mediump version of an indexing function for a highp value, if both mediump and
    // highp values are being indexed in the shader. For HLSL precision doesn't matter, but in
    // principle this code could be used with multiple backends.
    type.setPrecision(EbpHigh);
    TIntermAggregate *indexingFunction = new TIntermAggregate(EOpFunction);
    indexingFunction->setNameObj(GetIndexFunctionName(type, write));

    TType fieldType = GetFieldType(type);
    int numCases = 0;
    if (type.isMatrix())
    {
        numCases = type.getCols();
    }
    else
    {
        numCases = type.getNominalSize();
    }
    if (write)
    {
        indexingFunction->setType(TType(EbtVoid));
    }
    else
    {
        indexingFunction->setType(fieldType);
    }

    TIntermAggregate *paramsNode = new TIntermAggregate(EOpParameters);
    TIntermSymbol *baseParam = CreateBaseSymbol(type);
    if (write)
        baseParam->getTypePointer()->setQualifier(EvqInOut);
    else
        baseParam->getTypePointer()->setQualifier(EvqIn);
    paramsNode->getSequence()->push_back(baseParam);
    TIntermSymbol *indexParam = CreateIndexSymbol();
    indexParam->getTypePointer()->setQualifier(EvqIn);
    paramsNode->getSequence()->push_back(indexParam);
    if (write)
    {
        TIntermSymbol *valueParam = CreateValueSymbol(fieldType);
        valueParam->getTypePointer()->setQualifier(EvqIn);
        paramsNode->getSequence()->push_back(valueParam);
    }
    indexingFunction->getSequence()->push_back(paramsNode);

    TIntermAggregate *statementList = new TIntermAggregate(EOpSequence);
    for (int i = 0; i < numCases; ++i)
    {
        TIntermCase *caseNode = new TIntermCase(CreateIntConstantNode(i));
        statementList->getSequence()->push_back(caseNode);

        TIntermBinary *indexNode = CreateIndexDirectBaseSymbolNode(type, fieldType, i);
        if (write)
        {
            TIntermBinary *assignNode = CreateAssignValueSymbolNode(indexNode, fieldType);
            statementList->getSequence()->push_back(assignNode);
            TIntermBranch *returnNode = new TIntermBranch(EOpReturn, nullptr);
            statementList->getSequence()->push_back(returnNode);
        }
        else
        {
            TIntermBranch *returnNode = new TIntermBranch(EOpReturn, indexNode);
            statementList->getSequence()->push_back(returnNode);
        }
    }

    // Default case
    TIntermCase *defaultNode = new TIntermCase(nullptr);
    statementList->getSequence()->push_back(defaultNode);
    TIntermBranch *breakNode = new TIntermBranch(EOpBreak, nullptr);
    statementList->getSequence()->push_back(breakNode);

    TIntermSwitch *switchNode = new TIntermSwitch(CreateIndexSymbol(), statementList);

    TIntermAggregate *bodyNode = new TIntermAggregate(EOpSequence);
    bodyNode->getSequence()->push_back(switchNode);

    TIntermBinary *cond = new TIntermBinary(EOpLessThan);
    cond->setType(TType(EbtBool, EbpUndefined));
    cond->setLeft(CreateIndexSymbol());
    cond->setRight(CreateIntConstantNode(0));

    // Two blocks: one accesses (either reads or writes) the first element and returns,
    // the other accesses the last element.
    TIntermAggregate *useFirstBlock = new TIntermAggregate(EOpSequence);
    TIntermAggregate *useLastBlock  = new TIntermAggregate(EOpSequence);
    TIntermBinary *indexFirstNode   = CreateIndexDirectBaseSymbolNode(type, fieldType, 0);
    TIntermBinary *indexLastNode = CreateIndexDirectBaseSymbolNode(type, fieldType, numCases - 1);
    if (write)
    {
        TIntermBinary *assignFirstNode = CreateAssignValueSymbolNode(indexFirstNode, fieldType);
        useFirstBlock->getSequence()->push_back(assignFirstNode);
        TIntermBranch *returnNode = new TIntermBranch(EOpReturn, nullptr);
        useFirstBlock->getSequence()->push_back(returnNode);

        TIntermBinary *assignLastNode = CreateAssignValueSymbolNode(indexLastNode, fieldType);
        useLastBlock->getSequence()->push_back(assignLastNode);
    }
    else
    {
        TIntermBranch *returnFirstNode = new TIntermBranch(EOpReturn, indexFirstNode);
        useFirstBlock->getSequence()->push_back(returnFirstNode);

        TIntermBranch *returnLastNode = new TIntermBranch(EOpReturn, indexLastNode);
        useLastBlock->getSequence()->push_back(returnLastNode);
    }
    TIntermSelection *ifNode = new TIntermSelection(cond, useFirstBlock, nullptr);
    bodyNode->getSequence()->push_back(ifNode);
    bodyNode->getSequence()->push_back(useLastBlock);

    indexingFunction->getSequence()->push_back(bodyNode);

    return indexingFunction;
}

class RemoveDynamicIndexingTraverser : public TLValueTrackingTraverser
{
  public:
    RemoveDynamicIndexingTraverser(const TSymbolTable &symbolTable, int shaderVersion);

    bool visitBinary(Visit visit, TIntermBinary *node) override;

    void insertHelperDefinitions(TIntermNode *root);

    void nextIteration();

    bool usedTreeInsertion() const { return mUsedTreeInsertion; }

  protected:
    // Sets of types that are indexed. Note that these can not store multiple variants
    // of the same type with different precisions - only one precision gets stored.
    std::set<TType> mIndexedVecAndMatrixTypes;
    std::set<TType> mWrittenVecAndMatrixTypes;

    bool mUsedTreeInsertion;

    // When true, the traverser will remove side effects from any indexing expression.
    // This is done so that in code like
    //   V[j++][i]++.
    // where V is an array of vectors, j++ will only be evaluated once.
    bool mRemoveIndexSideEffectsInSubtree;
};

RemoveDynamicIndexingTraverser::RemoveDynamicIndexingTraverser(const TSymbolTable &symbolTable,
                                                               int shaderVersion)
    : TLValueTrackingTraverser(true, false, false, symbolTable, shaderVersion),
      mUsedTreeInsertion(false),
      mRemoveIndexSideEffectsInSubtree(false)
{
}

void RemoveDynamicIndexingTraverser::insertHelperDefinitions(TIntermNode *root)
{
    TIntermAggregate *rootAgg = root->getAsAggregate();
    ASSERT(rootAgg != nullptr && rootAgg->getOp() == EOpSequence);
    TIntermSequence insertions;
    for (TType type : mIndexedVecAndMatrixTypes)
    {
        insertions.push_back(GetIndexFunctionDefinition(type, false));
    }
    for (TType type : mWrittenVecAndMatrixTypes)
    {
        insertions.push_back(GetIndexFunctionDefinition(type, true));
    }
    mInsertions.push_back(NodeInsertMultipleEntry(rootAgg, 0, insertions, TIntermSequence()));
}

// Create a call to dyn_index_*() based on an indirect indexing op node
TIntermAggregate *CreateIndexFunctionCall(TIntermBinary *node,
                                          TIntermTyped *indexedNode,
                                          TIntermTyped *index)
{
    ASSERT(node->getOp() == EOpIndexIndirect);
    TIntermAggregate *indexingCall = new TIntermAggregate(EOpFunctionCall);
    indexingCall->setLine(node->getLine());
    indexingCall->setUserDefined();
    indexingCall->setNameObj(GetIndexFunctionName(indexedNode->getType(), false));
    indexingCall->getSequence()->push_back(indexedNode);
    indexingCall->getSequence()->push_back(index);

    TType fieldType = GetFieldType(indexedNode->getType());
    indexingCall->setType(fieldType);
    return indexingCall;
}

TIntermAggregate *CreateIndexedWriteFunctionCall(TIntermBinary *node,
                                                 TIntermTyped *index,
                                                 TIntermTyped *writtenValue)
{
    // Deep copy the left node so that two pointers to the same node don't end up in the tree.
    TIntermNode *leftCopy = node->getLeft()->deepCopy();
    ASSERT(leftCopy != nullptr && leftCopy->getAsTyped() != nullptr);
    TIntermAggregate *indexedWriteCall =
        CreateIndexFunctionCall(node, leftCopy->getAsTyped(), index);
    indexedWriteCall->setNameObj(GetIndexFunctionName(node->getLeft()->getType(), true));
    indexedWriteCall->setType(TType(EbtVoid));
    indexedWriteCall->getSequence()->push_back(writtenValue);
    return indexedWriteCall;
}

bool RemoveDynamicIndexingTraverser::visitBinary(Visit visit, TIntermBinary *node)
{
    if (mUsedTreeInsertion)
        return false;

    if (node->getOp() == EOpIndexIndirect)
    {
        if (mRemoveIndexSideEffectsInSubtree)
        {
            ASSERT(node->getRight()->hasSideEffects());
            // In case we're just removing index side effects, convert
            //   v_expr[index_expr]
            // to this:
            //   int s0 = index_expr; v_expr[s0];
            // Now v_expr[s0] can be safely executed several times without unintended side effects.

            // Init the temp variable holding the index
            TIntermAggregate *initIndex = createTempInitDeclaration(node->getRight());
            TIntermSequence insertions;
            insertions.push_back(initIndex);
            insertStatementsInParentBlock(insertions);
            mUsedTreeInsertion = true;

            // Replace the index with the temp variable
            TIntermSymbol *tempIndex = createTempSymbol(node->getRight()->getType());
            NodeUpdateEntry replaceIndex(node, node->getRight(), tempIndex, false);
            mReplacements.push_back(replaceIndex);
        }
        else if (!node->getLeft()->isArray() && node->getLeft()->getBasicType() != EbtStruct)
        {
            bool write = isLValueRequiredHere();

            TType type = node->getLeft()->getType();
            mIndexedVecAndMatrixTypes.insert(type);

            if (write)
            {
                // Convert:
                //   v_expr[index_expr]++;
                // to this:
                //   int s0 = index_expr; float s1 = dyn_index(v_expr, s0); s1++;
                //   dyn_index_write(v_expr, s0, s1);
                // This works even if index_expr has some side effects.
                if (node->getLeft()->hasSideEffects())
                {
                    // If v_expr has side effects, those need to be removed before proceeding.
                    // Otherwise the side effects of v_expr would be evaluated twice.
                    // The only case where an l-value can have side effects is when it is
                    // indexing. For example, it can be V[j++] where V is an array of vectors.
                    mRemoveIndexSideEffectsInSubtree = true;
                    return true;
                }
                // TODO(oetuaho@nvidia.com): This is not optimal if the expression using the value
                // only writes it and doesn't need the previous value. http://anglebug.com/1116

                mWrittenVecAndMatrixTypes.insert(type);
                TType fieldType = GetFieldType(type);

                TIntermSequence insertionsBefore;
                TIntermSequence insertionsAfter;

                // Store the index in a temporary signed int variable.
                TIntermTyped *indexInitializer = EnsureSignedInt(node->getRight());
                TIntermAggregate *initIndex = createTempInitDeclaration(indexInitializer);
                initIndex->setLine(node->getLine());
                insertionsBefore.push_back(initIndex);

                TIntermAggregate *indexingCall = CreateIndexFunctionCall(
                    node, node->getLeft(), createTempSymbol(indexInitializer->getType()));

                // Create a node for referring to the index after the nextTemporaryIndex() call
                // below.
                TIntermSymbol *tempIndex = createTempSymbol(indexInitializer->getType());

                nextTemporaryIndex();  // From now on, creating temporary symbols that refer to the
                                       // field value.
                insertionsBefore.push_back(createTempInitDeclaration(indexingCall));

                TIntermAggregate *indexedWriteCall =
                    CreateIndexedWriteFunctionCall(node, tempIndex, createTempSymbol(fieldType));
                insertionsAfter.push_back(indexedWriteCall);
                insertStatementsInParentBlock(insertionsBefore, insertionsAfter);
                NodeUpdateEntry replaceIndex(getParentNode(), node, createTempSymbol(fieldType),
                                             false);
                mReplacements.push_back(replaceIndex);
                mUsedTreeInsertion = true;
            }
            else
            {
                // The indexed value is not being written, so we can simply convert
                //   v_expr[index_expr]
                // into
                //   dyn_index(v_expr, index_expr)
                // If the index_expr is unsigned, we'll convert it to signed.
                ASSERT(!mRemoveIndexSideEffectsInSubtree);
                TIntermAggregate *indexingCall = CreateIndexFunctionCall(
                    node, node->getLeft(), EnsureSignedInt(node->getRight()));
                NodeUpdateEntry replaceIndex(getParentNode(), node, indexingCall, false);
                mReplacements.push_back(replaceIndex);
            }
        }
    }
    return !mUsedTreeInsertion;
}

void RemoveDynamicIndexingTraverser::nextIteration()
{
    mUsedTreeInsertion               = false;
    mRemoveIndexSideEffectsInSubtree = false;
    nextTemporaryIndex();
}

}  // namespace

void RemoveDynamicIndexing(TIntermNode *root,
                           unsigned int *temporaryIndex,
                           const TSymbolTable &symbolTable,
                           int shaderVersion)
{
    RemoveDynamicIndexingTraverser traverser(symbolTable, shaderVersion);
    ASSERT(temporaryIndex != nullptr);
    traverser.useTemporaryIndex(temporaryIndex);
    do
    {
        traverser.nextIteration();
        root->traverse(&traverser);
        traverser.updateTree();
    } while (traverser.usedTreeInsertion());
    traverser.insertHelperDefinitions(root);
    traverser.updateTree();
}
