//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CallDAG.h: Implements a call graph DAG of functions to be re-used accross
// analyses, allows to efficiently traverse the functions in topological
// order.

#include "compiler/translator/CallDAG.h"
#include "compiler/translator/InfoSink.h"

// The CallDAGCreator does all the processing required to create the CallDAG
// structure so that the latter contains only the necessary variables.
class CallDAG::CallDAGCreator : public TIntermTraverser
{
  public:
    CallDAGCreator(TInfoSinkBase *info)
        : TIntermTraverser(true, false, true),
          mCreationInfo(info),
          mCurrentFunction(nullptr),
          mCurrentIndex(0)
    {
    }

    InitResult assignIndices()
    {
        int skipped = 0;
        for (auto &it : mFunctions)
        {
            // Skip unimplemented functions
            if (it.second.node)
            {
                InitResult result = assignIndicesInternal(&it.second);
                if (result != INITDAG_SUCCESS)
                {
                    return result;
                }
            }
            else
            {
                skipped++;
            }
        }
        ASSERT(mFunctions.size() == mCurrentIndex + skipped);
        return INITDAG_SUCCESS;
    }

    void fillDataStructures(std::vector<Record> *records, std::map<int, int> *idToIndex)
    {
        ASSERT(records->empty());
        ASSERT(idToIndex->empty());

        records->resize(mCurrentIndex);

        for (auto &it : mFunctions)
        {
            CreatorFunctionData &data = it.second;
            // Skip unimplemented functions
            if (!data.node)
            {
                continue;
            }
            ASSERT(data.index < records->size());
            Record &record = (*records)[data.index];

            record.name = data.name.data();
            record.node = data.node;

            record.callees.reserve(data.callees.size());
            for (auto &callee : data.callees)
            {
                record.callees.push_back(static_cast<int>(callee->index));
            }

            (*idToIndex)[data.node->getFunctionId()] = static_cast<int>(data.index);
        }
    }

  private:

    struct CreatorFunctionData
    {
        CreatorFunctionData()
            : node(nullptr),
              index(0),
              indexAssigned(false),
              visiting(false)
        {
        }

        std::set<CreatorFunctionData*> callees;
        TIntermAggregate *node;
        TString name;
        size_t index;
        bool indexAssigned;
        bool visiting;
    };

    // Aggregates the AST node for each function as well as the name of the functions called by it
    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        switch (node->getOp())
        {
          case EOpPrototype:
            if (visit == PreVisit)
            {
                // Function declaration, create an empty record.
                mFunctions[node->getName()];
            }
            break;
          case EOpFunction:
            {
                // Function definition, create the record if need be and remember the node.
                if (visit == PreVisit)
                {
                    auto it = mFunctions.find(node->getName());

                    if (it == mFunctions.end())
                    {
                        mCurrentFunction = &mFunctions[node->getName()];
                    }
                    else
                    {
                        mCurrentFunction = &it->second;
                    }

                    mCurrentFunction->node = node;
                    mCurrentFunction->name = node->getName();

                }
                else if (visit == PostVisit)
                {
                    mCurrentFunction = nullptr;
                }
                break;
            }
          case EOpFunctionCall:
            {
                // Function call, add the callees
                if (visit == PreVisit)
                {
                    // Do not handle calls to builtin functions
                    if (node->isUserDefined())
                    {
                        auto it = mFunctions.find(node->getName());
                        ASSERT(it != mFunctions.end());

                        // We might be in a top-level function call to set a global variable
                        if (mCurrentFunction)
                        {
                            mCurrentFunction->callees.insert(&it->second);
                        }
                    }
                }
                break;
            }
          default:
            break;
        }
        return true;
    }

    // Recursively assigns indices to a sub DAG
    InitResult assignIndicesInternal(CreatorFunctionData *function)
    {
        ASSERT(function);

        if (!function->node)
        {
            *mCreationInfo << "Undefined function: " << function->name;
            return INITDAG_UNDEFINED;
        }

        if (function->indexAssigned)
        {
            return INITDAG_SUCCESS;
        }

        if (function->visiting)
        {
            if (mCreationInfo)
            {
                *mCreationInfo << "Recursive function call in the following call chain: " << function->name;
            }
            return INITDAG_RECURSION;
        }
        function->visiting = true;

        for (auto &callee : function->callees)
        {
            InitResult result = assignIndicesInternal(callee);
            if (result == INITDAG_RECURSION)
            {
                // We know that there is a recursive function call chain in the AST,
                // print the link of the chain we were processing.
                if (mCreationInfo)
                {
                    *mCreationInfo << " <- " << function->name;
                }
                return INITDAG_RECURSION;
            }
            else if (result == INITDAG_UNDEFINED)
            {
                return INITDAG_UNDEFINED;
            }
        }

        function->index = mCurrentIndex++;
        function->indexAssigned = true;

        function->visiting = false;
        return INITDAG_SUCCESS;
    }

    TInfoSinkBase *mCreationInfo;

    std::map<TString, CreatorFunctionData> mFunctions;
    CreatorFunctionData *mCurrentFunction;
    size_t mCurrentIndex;
};

// CallDAG

CallDAG::CallDAG()
{
}

CallDAG::~CallDAG()
{
}

const size_t CallDAG::InvalidIndex = std::numeric_limits<size_t>::max();

size_t CallDAG::findIndex(const TIntermAggregate *function) const
{
    TOperator op = function->getOp();
    ASSERT(op == EOpPrototype || op == EOpFunction || op == EOpFunctionCall);
    UNUSED_ASSERTION_VARIABLE(op);

    auto it = mFunctionIdToIndex.find(function->getFunctionId());

    if (it == mFunctionIdToIndex.end())
    {
        return InvalidIndex;
    }
    else
    {
        return it->second;
    }
}

const CallDAG::Record &CallDAG::getRecordFromIndex(size_t index) const
{
    ASSERT(index != InvalidIndex && index < mRecords.size());
    return mRecords[index];
}

const CallDAG::Record &CallDAG::getRecord(const TIntermAggregate *function) const
{
    size_t index = findIndex(function);
    ASSERT(index != InvalidIndex && index < mRecords.size());
    return mRecords[index];
}

size_t CallDAG::size() const
{
    return mRecords.size();
}

void CallDAG::clear()
{
    mRecords.clear();
    mFunctionIdToIndex.clear();
}

CallDAG::InitResult CallDAG::init(TIntermNode *root, TInfoSinkBase *info)
{
    CallDAGCreator creator(info);

    // Creates the mapping of functions to callees
    root->traverse(&creator);

    // Does the topological sort and detects recursions
    InitResult result = creator.assignIndices();
    if (result != INITDAG_SUCCESS)
    {
        return result;
    }

    creator.fillDataStructures(&mRecords, &mFunctionIdToIndex);
    return INITDAG_SUCCESS;
}
