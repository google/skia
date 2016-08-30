/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_ERRORREPORTER
#define SKSL_ERRORREPORTER

#include "SkSLPosition.h"

namespace SkSL {

/**
 * Interface for the compiler to report errors.
 */
class ErrorReporter {
public:
    virtual ~ErrorReporter() {}

    virtual void error(Position position, std::string msg) = 0;
};

} // namespace

#endif
