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
 * Returns the absolute value of x. If x is a vector, operates componentwise.
 */
DSLExpression abs(DSLExpression x);

/**
 * Returns true if all of the components of boolean vector x are true.
 */
DSLExpression all(DSLExpression x);

/**
 * Returns true if any of the components of boolean vector x are true.
 */
DSLExpression any(DSLExpression x);

/**
 * Returns x rounded towards positive infinity. If x is a vector, operates componentwise.
 */
DSLExpression ceil(DSLExpression x);

/**
 * Returns x clamped to between min and max. If x is a vector, operates componentwise.
 */
DSLExpression clamp(DSLExpression x, DSLExpression min, DSLExpression max);

/**
 * Returns the cosine of x. If x is a vector, operates componentwise.
 */
DSLExpression cos(DSLExpression x);

/**
 * Returns the cross product of x and y.
 */
DSLExpression cross(DSLExpression x, DSLExpression y);

/**
 * Returns x converted from radians to degrees. If x is a vector, operates componentwise.
 */
DSLExpression degrees(DSLExpression x);

/**
 * Returns the distance between x and y.
 */
DSLExpression distance(DSLExpression x, DSLExpression y);

/**
 * Returns the dot product of x and y.
 */
DSLExpression dot(DSLExpression x, DSLExpression y);

/**
 * Returns a boolean vector indicating whether components of x are equal to the corresponding
 * components of y.
 */
DSLExpression equal(DSLExpression x, DSLExpression y);

/**
 * Returns e^x. If x is a vector, operates componentwise.
 */
DSLExpression exp(DSLExpression x);

/**
 * Returns 2^x. If x is a vector, operates componentwise.
 */
DSLExpression exp2(DSLExpression x);

/**
 * If dot(i, nref) >= 0, returns n, otherwise returns -n.
 */
DSLExpression faceforward(DSLExpression n, DSLExpression i, DSLExpression nref);

/**
 * Returns x rounded towards negative infinity. If x is a vector, operates componentwise.
 */
DSLExpression floor(DSLExpression x);

/**
 * Returns the fractional part of x. If x is a vector, operates componentwise.
 */
DSLExpression fract(DSLExpression x);

/**
 * Returns a boolean vector indicating whether components of x are greater than the corresponding
 * components of y.
 */
DSLExpression greaterThan(DSLExpression x, DSLExpression y);

/**
 * Returns a boolean vector indicating whether components of x are greater than or equal to the
 * corresponding components of y.
 */
DSLExpression greaterThanEqual(DSLExpression x, DSLExpression y);

/**
 * Returns the 1/sqrt(x). If x is a vector, operates componentwise.
 */
DSLExpression inversesqrt(DSLExpression x);

/**
 * Returns the inverse of the matrix x.
 */
DSLExpression inverse(DSLExpression x);

/**
 * Returns the length of the vector x.
 */
DSLExpression length(DSLExpression x);

/**
 * Returns a boolean vector indicating whether components of x are less than the corresponding
 * components of y.
 */
DSLExpression lessThan(DSLExpression x, DSLExpression y);

/**
 * Returns a boolean vector indicating whether components of x are less than or equal to the
 * corresponding components of y.
 */
DSLExpression lessThanEqual(DSLExpression x, DSLExpression y);

/**
 * Returns the log base e of x. If x is a vector, operates componentwise.
 */
DSLExpression log(DSLExpression x);

/**
 * Returns the log base 2 of x. If x is a vector, operates componentwise.
 */
DSLExpression log2(DSLExpression x);

/**
 * Returns the larger (closer to positive infinity) of x and y. If x is a vector, operates
 * componentwise. y may be either a vector of the same dimensions as x, or a scalar.
 */
DSLExpression max(DSLExpression x, DSLExpression y);

/**
 * Returns the smaller (closer to negative infinity) of x and y. If x is a vector, operates
 * componentwise. y may be either a vector of the same dimensions as x, or a scalar.
 */
DSLExpression min(DSLExpression x, DSLExpression y);

/**
 * Returns a linear intepolation between x and y at position a, where a=0 results in x and a=1
 * results in y. If x and y are vectors, operates componentwise. a may be either a vector of the
 * same dimensions as x and y, or a scalar.
 */
DSLExpression mix(DSLExpression x, DSLExpression y, DSLExpression a);

/**
 * Returns x modulo y. If x is a vector, operates componentwise. y may be either a vector of the
 * same dimensions as x, or a scalar.
 */
DSLExpression mod(DSLExpression x, DSLExpression y);

/**
 * Returns the vector x normalized to a length of 1.
 */
DSLExpression normalize(DSLExpression x);

/**
 * Returns a boolean vector indicating whether components of x are not equal to the corresponding
 * components of y.
 */
DSLExpression notEqual(DSLExpression x, DSLExpression y);

/**
 * Returns x raised to the power y. If x is a vector, operates componentwise. y may be either a
 * vector of the same dimensions as x, or a scalar.
 */
DSLExpression pow(DSLExpression x, DSLExpression y);

/**
 * Returns x converted from degrees to radians. If x is a vector, operates componentwise.
 */
DSLExpression radians(DSLExpression x);

/**
 * Returns i reflected from a surface with normal n.
 */
DSLExpression reflect(DSLExpression i, DSLExpression n);

/**
 * Returns i refracted across a surface with normal n and ratio of indices of refraction eta.
 */
DSLExpression refract(DSLExpression i, DSLExpression n, DSLExpression eta);

/**
 * Returns x clamped to the range [0, 1]. If x is a vector, operates componentwise.
 */
DSLExpression saturate(DSLExpression x);

/**
 * Returns -1, 0, or 1 depending on whether x is negative, zero, or positive, respectively. If x is
 * a vector, operates componentwise.
 */
DSLExpression sign(DSLExpression x);

/**
 * Returns the sine of x. If x is a vector, operates componentwise.
 */
DSLExpression sin(DSLExpression x);

/**
 * Returns a smooth interpolation between 0 (at x=edge1) and 1 (at x=edge2). If x is a vector,
 * operates componentwise. edge1 and edge2 may either be both vectors of the same dimensions as x or
 * scalars.
 */
DSLExpression smoothstep(DSLExpression edge1, DSLExpression edge2, DSLExpression x);

/**
 * Returns the square root of x. If x is a vector, operates componentwise.
 */
DSLExpression sqrt(DSLExpression x);

/**
 * Returns 0 if x < edge or 1 if x >= edge. If x is a vector, operates componentwise. edge may be
 * either a vector of the same dimensions as x, or a scalar.
 */
DSLExpression step(DSLExpression edge, DSLExpression x);

/**
 * Returns the tangent of x. If x is a vector, operates componentwise.
 */
DSLExpression tan(DSLExpression x);

/**
 * Returns x converted from premultipled to unpremultiplied alpha.
 */
DSLExpression unpremul(DSLExpression x);

} // namespace dsl

} // namespace SkSL

#endif
