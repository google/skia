/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXTERNALVALUESYMBOL
#define SKSL_EXTERNALVALUESYMBOL

#include "src/sksl/SkSLExternalValue.h"

namespace SkSL {

class String;
class Type;

class ExternalValueSymbol : public Symbol {
public:
    ExternalValueSymbol(IRGenerator* irGenerator, ExternalValue* ev)
        : INHERITED(irGenerator, -1, kExternal_Kind, ev->fName)
        , fValue(*ev) {}

    ExternalValue& fValue;

private:
    typedef Symbol INHERITED;
};

} // namespace

#endif
