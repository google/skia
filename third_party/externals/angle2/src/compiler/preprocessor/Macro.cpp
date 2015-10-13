//
// Copyright (c) 2011 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "Macro.h"

#include <sstream>

#include "Token.h"

namespace pp
{

bool Macro::equals(const Macro &other) const
{
    return (type == other.type) &&
           (name == other.name) &&
           (parameters == other.parameters) &&
           (replacements == other.replacements);
}

void PredefineMacro(MacroSet *macroSet, const char *name, int value)
{
    std::ostringstream stream;
    stream << value;

    Token token;
    token.type = Token::CONST_INT;
    token.text = stream.str();

    Macro macro;
    macro.predefined = true;
    macro.type       = Macro::kTypeObj;
    macro.name = name;
    macro.replacements.push_back(token);

    (*macroSet)[name] = macro;
}

}  // namespace pp

