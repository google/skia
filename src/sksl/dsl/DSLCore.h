/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_CORE
#define SKSL_DSL_CORE

#include "src/sksl/dsl/DSLExpression.h"
#include "src/sksl/dsl/DSLType.h"
#include "src/sksl/dsl/DSLVar.h"

namespace SkSL {

class Compiler;

namespace dsl {

/**
 * Class which is notified in the event of an error.
 */
class ErrorHandler {
public:
    virtual ~ErrorHandler() {}

    virtual void handleError(const char* msg) = 0;
};

#if SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)
/**
 * Starts DSL output on the current thread using the specified compiler. This must be called
 * prior to any other DSL functions.
 */
void Start(SkSL::Compiler* compiler);

/**
 * Signals the end of DSL output. This must be called sometime between a call to Start() and the
 * termination of the thread.
 */
void End();

#endif // SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)

/**
 * Installs an ErrorHandler which will be notified of any errors that occur during DSL calls. If
 * no ErrorHandler is installed, any errors will be fatal.
 */
void SetErrorHandler(ErrorHandler* errorHandler);

} // namespace dsl

} // namespace SkSL

#endif
