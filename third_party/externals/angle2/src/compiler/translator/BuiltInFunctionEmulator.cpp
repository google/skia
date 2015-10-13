//
// Copyright (c) 2002-2011 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "angle_gl.h"
#include "compiler/translator/BuiltInFunctionEmulator.h"
#include "compiler/translator/SymbolTable.h"

class BuiltInFunctionEmulator::BuiltInFunctionEmulationMarker : public TIntermTraverser
{
  public:
    BuiltInFunctionEmulationMarker(BuiltInFunctionEmulator &emulator)
        : TIntermTraverser(true, false, false),
          mEmulator(emulator)
    {
    }

    bool visitUnary(Visit visit, TIntermUnary *node) override
    {
        if (visit == PreVisit)
        {
            bool needToEmulate = mEmulator.SetFunctionCalled(node->getOp(), node->getOperand()->getType());
            if (needToEmulate)
                node->setUseEmulatedFunction();
        }
        return true;
    }

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        if (visit == PreVisit)
        {
            // Here we handle all the built-in functions instead of the ones we
            // currently identified as problematic.
            switch (node->getOp())
            {
                case EOpLessThan:
                case EOpGreaterThan:
                case EOpLessThanEqual:
                case EOpGreaterThanEqual:
                case EOpVectorEqual:
                case EOpVectorNotEqual:
                case EOpMod:
                case EOpPow:
                case EOpAtan:
                case EOpMin:
                case EOpMax:
                case EOpClamp:
                case EOpMix:
                case EOpStep:
                case EOpSmoothStep:
                case EOpDistance:
                case EOpDot:
                case EOpCross:
                case EOpFaceForward:
                case EOpReflect:
                case EOpRefract:
                case EOpOuterProduct:
                case EOpMul:
                    break;
                default:
                    return true;
            }
            const TIntermSequence &sequence = *(node->getSequence());
            bool needToEmulate = false;
            // Right now we only handle built-in functions with two or three parameters.
            if (sequence.size() == 2)
            {
                TIntermTyped *param1 = sequence[0]->getAsTyped();
                TIntermTyped *param2 = sequence[1]->getAsTyped();
                if (!param1 || !param2)
                    return true;
                needToEmulate = mEmulator.SetFunctionCalled(
                    node->getOp(), param1->getType(), param2->getType());
            }
            else if (sequence.size() == 3)
            {
                TIntermTyped *param1 = sequence[0]->getAsTyped();
                TIntermTyped *param2 = sequence[1]->getAsTyped();
                TIntermTyped *param3 = sequence[2]->getAsTyped();
                if (!param1 || !param2 || !param3)
                    return true;
                needToEmulate = mEmulator.SetFunctionCalled(
                    node->getOp(), param1->getType(), param2->getType(), param3->getType());
            }
            else
            {
                return true;
            }

            if (needToEmulate)
                node->setUseEmulatedFunction();
        }
        return true;
    }

  private:
    BuiltInFunctionEmulator &mEmulator;
};

BuiltInFunctionEmulator::BuiltInFunctionEmulator()
{}

void BuiltInFunctionEmulator::addEmulatedFunction(TOperator op, const TType *param,
                                                  const char *emulatedFunctionDefinition)
{
    mEmulatedFunctions[FunctionId(op, param)] = std::string(emulatedFunctionDefinition);
}

void BuiltInFunctionEmulator::addEmulatedFunction(TOperator op, const TType *param1, const TType *param2,
                                                  const char *emulatedFunctionDefinition)
{
    mEmulatedFunctions[FunctionId(op, param1, param2)] = std::string(emulatedFunctionDefinition);
}

void BuiltInFunctionEmulator::addEmulatedFunction(TOperator op, const TType *param1, const TType *param2,
                                                  const TType *param3, const char *emulatedFunctionDefinition)
{
    mEmulatedFunctions[FunctionId(op, param1, param2, param3)] = std::string(emulatedFunctionDefinition);
}

bool BuiltInFunctionEmulator::IsOutputEmpty() const
{
    return (mFunctions.size() == 0);
}

void BuiltInFunctionEmulator::OutputEmulatedFunctions(TInfoSinkBase &out) const
{
    for (size_t i = 0; i < mFunctions.size(); ++i)
    {
        out << mEmulatedFunctions.find(mFunctions[i])->second << "\n\n";
    }
}

bool BuiltInFunctionEmulator::SetFunctionCalled(TOperator op, const TType &param)
{
    return SetFunctionCalled(FunctionId(op, &param));
}

bool BuiltInFunctionEmulator::SetFunctionCalled(TOperator op, const TType &param1, const TType &param2)
{
    return SetFunctionCalled(FunctionId(op, &param1, &param2));
}

bool BuiltInFunctionEmulator::SetFunctionCalled(TOperator op,
                                                const TType &param1, const TType &param2, const TType &param3)
{
    return SetFunctionCalled(FunctionId(op, &param1, &param2, &param3));
}

bool BuiltInFunctionEmulator::SetFunctionCalled(const FunctionId &functionId)
{
    if (mEmulatedFunctions.find(functionId) != mEmulatedFunctions.end())
    {
        for (size_t i = 0; i < mFunctions.size(); ++i)
        {
            if (mFunctions[i] == functionId)
                return true;
        }
        // Copy the functionId if it needs to be stored, to make sure that the TType pointers inside
        // remain valid and constant.
        mFunctions.push_back(functionId.getCopy());
        return true;
    }
    return false;
}

void BuiltInFunctionEmulator::MarkBuiltInFunctionsForEmulation(TIntermNode *root)
{
    ASSERT(root);

    if (mEmulatedFunctions.empty())
        return;

    BuiltInFunctionEmulationMarker marker(*this);
    root->traverse(&marker);
}

void BuiltInFunctionEmulator::Cleanup()
{
    mFunctions.clear();
}

//static
TString BuiltInFunctionEmulator::GetEmulatedFunctionName(
    const TString &name)
{
    ASSERT(name[name.length() - 1] == '(');
    return "webgl_" + name.substr(0, name.length() - 1) + "_emu(";
}

BuiltInFunctionEmulator::FunctionId::FunctionId(TOperator op, const TType *param)
    : mOp(op),
      mParam1(param),
      mParam2(new TType(EbtVoid)),
      mParam3(new TType(EbtVoid))
{
}

BuiltInFunctionEmulator::FunctionId::FunctionId(TOperator op, const TType *param1, const TType *param2)
    : mOp(op),
      mParam1(param1),
      mParam2(param2),
      mParam3(new TType(EbtVoid))
{
}

BuiltInFunctionEmulator::FunctionId::FunctionId(TOperator op,
                                                const TType *param1, const TType *param2, const TType *param3)
    : mOp(op),
      mParam1(param1),
      mParam2(param2),
      mParam3(param3)
{
}

bool BuiltInFunctionEmulator::FunctionId::operator==(const BuiltInFunctionEmulator::FunctionId &other) const
{
    return (mOp == other.mOp &&
            *mParam1 == *other.mParam1 &&
            *mParam2 == *other.mParam2 &&
            *mParam3 == *other.mParam3);
}

bool BuiltInFunctionEmulator::FunctionId::operator<(const BuiltInFunctionEmulator::FunctionId &other) const
{
    if (mOp != other.mOp)
        return mOp < other.mOp;
    if (*mParam1 != *other.mParam1)
        return *mParam1 < *other.mParam1;
    if (*mParam2 != *other.mParam2)
        return *mParam2 < *other.mParam2;
    if (*mParam3 != *other.mParam3)
       return *mParam3 < *other.mParam3;
    return false; // all fields are equal
}

BuiltInFunctionEmulator::FunctionId BuiltInFunctionEmulator::FunctionId::getCopy() const
{
    return FunctionId(mOp, new TType(*mParam1), new TType(*mParam2), new TType(*mParam3));
}
