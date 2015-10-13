//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_OUTPUTESSL_H_
#define COMPILER_TRANSLATOR_OUTPUTESSL_H_

#include "compiler/translator/OutputGLSLBase.h"

class TOutputESSL : public TOutputGLSLBase
{
public:
    TOutputESSL(TInfoSinkBase& objSink,
                ShArrayIndexClampingStrategy clampingStrategy,
                ShHashFunction64 hashFunction,
                NameMap& nameMap,
                TSymbolTable& symbolTable,
                int shaderVersion,
                bool forceHighp);

protected:
  bool writeVariablePrecision(TPrecision precision) override;

private:
    bool mForceHighp;
};

#endif  // COMPILER_TRANSLATOR_OUTPUTESSL_H_
