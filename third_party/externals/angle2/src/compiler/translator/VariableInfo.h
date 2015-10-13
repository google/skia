//
// Copyright (c) 2002-2011 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_VARIABLEINFO_H_
#define COMPILER_TRANSLATOR_VARIABLEINFO_H_

#include <GLSLANG/ShaderLang.h>

#include "compiler/translator/IntermNode.h"

class TSymbolTable;

namespace sh
{

// Traverses intermediate tree to collect all attributes, uniforms, varyings.
class CollectVariables : public TIntermTraverser
{
  public:
    CollectVariables(std::vector<Attribute> *attribs,
                     std::vector<OutputVariable> *outputVariables,
                     std::vector<Uniform> *uniforms,
                     std::vector<Varying> *varyings,
                     std::vector<InterfaceBlock> *interfaceBlocks,
                     ShHashFunction64 hashFunction,
                     const TSymbolTable &symbolTable);

    void visitSymbol(TIntermSymbol *symbol) override;
    bool visitAggregate(Visit, TIntermAggregate *node) override;
    bool visitBinary(Visit visit, TIntermBinary *binaryNode) override;

  private:
    template <typename VarT>
    void visitVariable(const TIntermSymbol *variable, std::vector<VarT> *infoList) const;

    template <typename VarT>
    void visitInfoList(const TIntermSequence &sequence, std::vector<VarT> *infoList) const;

    std::vector<Attribute> *mAttribs;
    std::vector<OutputVariable> *mOutputVariables;
    std::vector<Uniform> *mUniforms;
    std::vector<Varying> *mVaryings;
    std::vector<InterfaceBlock> *mInterfaceBlocks;

    std::map<std::string, InterfaceBlockField *> mInterfaceBlockFields;

    bool mDepthRangeAdded;
    bool mPointCoordAdded;
    bool mFrontFacingAdded;
    bool mFragCoordAdded;

    bool mInstanceIDAdded;
    bool mPositionAdded;
    bool mPointSizeAdded;
    bool mLastFragDataAdded;
    bool mFragColorAdded;
    bool mFragDataAdded;
    bool mFragDepthAdded;
    bool mSecondaryFragColorEXTAdded;
    bool mSecondaryFragDataEXTAdded;

    ShHashFunction64 mHashFunction;

    const TSymbolTable &mSymbolTable;
};

// Expand struct uniforms to flattened lists of split variables
void ExpandUniforms(const std::vector<Uniform> &compact,
                    std::vector<ShaderVariable> *expanded);

}

#endif  // COMPILER_TRANSLATOR_VARIABLEINFO_H_
