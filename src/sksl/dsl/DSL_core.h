/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_CORE
#define SKSL_DSL_CORE

#include "src/sksl/dsl/DSLExpression.h"
#include "src/sksl/dsl/DSLFunction.h"
#include "src/sksl/dsl/DSLStatement.h"
#include "src/sksl/dsl/DSLType.h"
#include "src/sksl/dsl/DSLVar.h"

namespace SkSL {

namespace dsl {

/**
 * Represents the fragment color, equivalent to gl_FragColor.
 */
DSLVar sk_FragColor();

/**
 * Represents the fragment coordinates, equivalent to gl_FragCoord.
 */
DSLVar sk_FragCoord();

/**
 * (Fragment processors only) Represents args.fSampleCoords.
 */
DSLVar sk_SampleCoord();

/**
 * (Fragment processors only) Represents args.fOutputColor.
 */
DSLVar sk_OutColor();

/**
 * (Fragment processors only) Represents args.fInputColor.
 */
DSLVar sk_InColor();

/**
 * Class which is notified in the event of an error.
 */
class ErrorHandler {
public:
    virtual ~ErrorHandler() {}

    virtual void handleError(const char* msg) = 0;
};

/**
 * Installs an ErrorHandler which will be notified of any errors that occur during DSL calls. If no
 * ErrorHandler is installed, any errors will be fatal.
 */
void SetErrorHandler(ErrorHandler* errorHandler);

/**
 * Creates a variable declaration statement with an initial value.
 */
DSLStatement Declare(DSLVar& var, DSLExpression initialValue = DSLExpression());

/**
 * do stmt; while (test);
 */
DSLStatement Do(DSLStatement stmt, DSLExpression test);

/**
 * for (initializer; test; next) stmt;
 */
DSLStatement For(DSLStatement initializer, DSLExpression test, DSLExpression next,
                 DSLStatement stmt);

/**
 * if (test) ifTrue; [else ifFalse;]
 */
DSLStatement If(DSLExpression test, DSLStatement ifTrue, DSLStatement ifFalse = DSLStatement());

/**
 * test ? ifTrue : ifFalse
 */
DSLExpression Ternary(DSLExpression test, DSLExpression ifTrue, DSLExpression ifFalse);

/**
 * while (test) stmt;
 */
DSLStatement While(DSLExpression test, DSLStatement stmt);

/**
 * Returns x rounded towards positive infinity.
 */
DSLExpression ceil(DSLExpression x);

/**
 * Returns x clamped to between min and max.
 */
DSLExpression clamp(DSLExpression x, DSLExpression min, DSLExpression max);

/**
 * Returns the dot product of x and y.
 */
DSLExpression dot(DSLExpression x, DSLExpression y);

/**
 * Returns x rounded towards negative infinity.
 */
DSLExpression floor(DSLExpression x);

/**
 * Returns x clamped to the range [0, 1].
 */
DSLExpression saturate(DSLExpression x);

/**
 * Returns x converted from premultipled to unpremultiplied alpha.
 */
DSLExpression unpremul(DSLExpression x);

} // namespace dsl

} // namespace SkSL

#endif
