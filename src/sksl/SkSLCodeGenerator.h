/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_CODEGENERATOR
#define SKSL_CODEGENERATOR

#include "ir/SkSLProgram.h"
#include <vector>
#include <iostream>

namespace SkSL {

/**
 * Abstract superclass of all code generators, which take a Program as input and produce code as
 * output.
 */
class CodeGenerator {
public:
    virtual ~CodeGenerator() {}
    
    virtual void generateCode(const Program& program, std::ostream& out) = 0;
};

} // namespace

#endif
