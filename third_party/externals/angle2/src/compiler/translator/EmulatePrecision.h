//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_EMULATE_PRECISION_H_
#define COMPILER_TRANSLATOR_EMULATE_PRECISION_H_

#include "common/angleutils.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/InfoSink.h"
#include "compiler/translator/IntermNode.h"
#include "GLSLANG/ShaderLang.h"

// This class gathers all compound assignments from the AST and can then write
// the functions required for their precision emulation. This way there is no
// need to write a huge number of variations of the emulated compound assignment
// to every translated shader with emulation enabled.

class EmulatePrecision : public TLValueTrackingTraverser
{
  public:
    EmulatePrecision(const TSymbolTable &symbolTable, int shaderVersion);

    void visitSymbol(TIntermSymbol *node) override;
    bool visitBinary(Visit visit, TIntermBinary *node) override;
    bool visitUnary(Visit visit, TIntermUnary *node) override;
    bool visitAggregate(Visit visit, TIntermAggregate *node) override;

    void writeEmulationHelpers(TInfoSinkBase& sink, ShShaderOutput outputLanguage);

  private:
    struct TypePair
    {
        TypePair(const char *l, const char *r)
            : lType(l), rType(r) { }

        const char *lType;
        const char *rType;
    };

    struct TypePairComparator
    {
        bool operator() (const TypePair& l, const TypePair& r) const
        {
            if (l.lType == r.lType)
                return l.rType < r.rType;
            return l.lType < r.lType;
        }
    };

    typedef std::set<TypePair, TypePairComparator> EmulationSet;
    EmulationSet mEmulateCompoundAdd;
    EmulationSet mEmulateCompoundSub;
    EmulationSet mEmulateCompoundMul;
    EmulationSet mEmulateCompoundDiv;

    bool mDeclaringVariables;
};

#endif  // COMPILER_TRANSLATOR_EMULATE_PRECISION_H_
