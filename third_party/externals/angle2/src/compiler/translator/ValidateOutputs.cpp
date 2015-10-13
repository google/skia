//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/ValidateOutputs.h"
#include "compiler/translator/InfoSink.h"
#include "compiler/translator/InitializeParseContext.h"
#include "compiler/translator/ParseContext.h"

namespace
{
void error(int *errorCount, TInfoSinkBase &sink, const TIntermSymbol &symbol, const char *reason)
{
    sink.prefix(EPrefixError);
    sink.location(symbol.getLine());
    sink << "'" << symbol.getSymbol() << "' : " << reason << "\n";
    (*errorCount)++;
}

}  // namespace

ValidateOutputs::ValidateOutputs(const TExtensionBehavior &extBehavior, int maxDrawBuffers)
    : TIntermTraverser(true, false, false),
      mMaxDrawBuffers(maxDrawBuffers),
      mAllowUnspecifiedOutputLocationResolution(
          IsExtensionEnabled(extBehavior, "GL_EXT_blend_func_extended"))
{
}

void ValidateOutputs::visitSymbol(TIntermSymbol *symbol)
{
    TString name = symbol->getSymbol();
    TQualifier qualifier = symbol->getQualifier();

    if (mVisitedSymbols.count(name) == 1)
        return;

    mVisitedSymbols.insert(name);

    if (qualifier == EvqFragmentOut)
    {
        if (symbol->getType().getLayoutQualifier().location == -1)
        {
            mUnspecifiedLocationOutputs.push_back(symbol);
        }
        else
        {
            mOutputs.push_back(symbol);
        }
    }
}

int ValidateOutputs::validateAndCountErrors(TInfoSinkBase &sink) const
{
    OutputVector validOutputs(mMaxDrawBuffers);
    int errorCount = 0;

    for (const auto &symbol : mOutputs)
    {
        const TType &type         = symbol->getType();
        const size_t elementCount = static_cast<size_t>(type.isArray() ? type.getArraySize() : 1);
        const size_t location     = static_cast<size_t>(type.getLayoutQualifier().location);

        ASSERT(type.getLayoutQualifier().location != -1);

        if (location + elementCount <= validOutputs.size())
        {
            for (size_t elementIndex = 0; elementIndex < elementCount; elementIndex++)
            {
                const size_t offsetLocation = location + elementIndex;
                if (validOutputs[offsetLocation])
                {
                    std::stringstream strstr;
                    strstr << "conflicting output locations with previously defined output '"
                           << validOutputs[offsetLocation]->getSymbol() << "'";
                    error(&errorCount, sink, *symbol, strstr.str().c_str());
                }
                else
                {
                    validOutputs[offsetLocation] = symbol;
                }
            }
        }
        else
        {
            if (elementCount > 0)
            {
                error(&errorCount, sink, *symbol,
                      elementCount > 1 ? "output array locations would exceed MAX_DRAW_BUFFERS"
                                       : "output location must be < MAX_DRAW_BUFFERS");
            }
        }
    }

    if (!mAllowUnspecifiedOutputLocationResolution &&
        ((!mOutputs.empty() && !mUnspecifiedLocationOutputs.empty()) ||
         mUnspecifiedLocationOutputs.size() > 1))
    {
        for (const auto &symbol : mUnspecifiedLocationOutputs)
        {
            error(&errorCount, sink, *symbol,
                  "must explicitly specify all locations when using multiple fragment outputs");
        }
    }
    return errorCount;
}
