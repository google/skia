//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_TRANSLATORHLSL_H_
#define COMPILER_TRANSLATOR_TRANSLATORHLSL_H_

#include "compiler/translator/Compiler.h"

class TranslatorHLSL : public TCompiler
{
  public:
    TranslatorHLSL(sh::GLenum type, ShShaderSpec spec, ShShaderOutput output);
    TranslatorHLSL *getAsTranslatorHLSL() override { return this; }

    bool hasInterfaceBlock(const std::string &interfaceBlockName) const;
    unsigned int getInterfaceBlockRegister(const std::string &interfaceBlockName) const;

    bool hasUniform(const std::string &uniformName) const;
    unsigned int getUniformRegister(const std::string &uniformName) const;

  protected:
    void translate(TIntermNode *root, int compileOptions) override;

    // collectVariables needs to be run always so registers can be assigned.
    bool shouldCollectVariables(int compileOptions) override { return true; }

    std::map<std::string, unsigned int> mInterfaceBlockRegisterMap;
    std::map<std::string, unsigned int> mUniformRegisterMap;
};

#endif  // COMPILER_TRANSLATOR_TRANSLATORHLSL_H_
