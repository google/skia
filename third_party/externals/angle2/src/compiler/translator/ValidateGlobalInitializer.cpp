//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/ValidateGlobalInitializer.h"

#include "compiler/translator/ParseContext.h"

namespace
{

class ValidateGlobalInitializerTraverser : public TIntermTraverser
{
  public:
    ValidateGlobalInitializerTraverser(const TParseContext *context);

    void visitSymbol(TIntermSymbol *node) override;
    bool visitAggregate(Visit visit, TIntermAggregate *node) override;
    bool visitBinary(Visit visit, TIntermBinary *node) override;
    bool visitUnary(Visit visit, TIntermUnary *node) override;

    bool isValid() const { return mIsValid; }
    bool issueWarning() const { return mIssueWarning; }

  private:
    const TParseContext *mContext;
    bool mIsValid;
    bool mIssueWarning;
};

void ValidateGlobalInitializerTraverser::visitSymbol(TIntermSymbol *node)
{
    const TSymbol *sym = mContext->symbolTable.find(node->getSymbol(), mContext->getShaderVersion());
    if (sym->isVariable())
    {
        // ESSL 1.00 section 4.3 (or ESSL 3.00 section 4.3):
        // Global initializers must be constant expressions.
        const TVariable *var = static_cast<const TVariable *>(sym);
        switch (var->getType().getQualifier())
        {
          case EvqConst:
            break;
          case EvqGlobal:
          case EvqTemporary:
          case EvqUniform:
            // We allow these cases to be compatible with legacy ESSL 1.00 content.
            // Implement stricter rules for ESSL 3.00 since there's no legacy content to deal with.
            if (mContext->getShaderVersion() >= 300)
            {
                mIsValid = false;
            }
            else
            {
                mIssueWarning = true;
            }
            break;
          default:
            mIsValid = false;
        }
    }
}

bool ValidateGlobalInitializerTraverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    // Disallow calls to user-defined functions and texture lookup functions in global variable initializers.
    // This is done simply by disabling all function calls - built-in math functions don't use EOpFunctionCall.
    if (node->getOp() == EOpFunctionCall)
    {
        mIsValid = false;
    }
    return true;
}

bool ValidateGlobalInitializerTraverser::visitBinary(Visit visit, TIntermBinary *node)
{
    if (node->isAssignment())
    {
        mIsValid = false;
    }
    return true;
}

bool ValidateGlobalInitializerTraverser::visitUnary(Visit visit, TIntermUnary *node)
{
    if (node->isAssignment())
    {
        mIsValid = false;
    }
    return true;
}

ValidateGlobalInitializerTraverser::ValidateGlobalInitializerTraverser(const TParseContext *context)
    : TIntermTraverser(true, false, false),
      mContext(context),
      mIsValid(true),
      mIssueWarning(false)
{
}

} // namespace

bool ValidateGlobalInitializer(TIntermTyped *initializer, const TParseContext *context, bool *warning)
{
    ValidateGlobalInitializerTraverser validate(context);
    initializer->traverse(&validate);
    ASSERT(warning != nullptr);
    *warning = validate.issueWarning();
    return validate.isValid();
}

