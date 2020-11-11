/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_TYPE
#define SKSL_DSL_TYPE

namespace skslcode {

#define SCALAR(T)                                                                                  \
const SkSL::Type& T();                                                                             \
Expression T(Expression expr);

SCALAR(Bool)
SCALAR(Float)
SCALAR(Half)
SCALAR(Int)
SCALAR(Short)

#define VECTOR2(T)                                                                                 \
SCALAR(T ## 2)                                                                                     \
Expression T ## 2(Expression x, Expression y);

#define VECTOR3(T)                                                                                 \
SCALAR(T ## 3)                                                                                     \
Expression T ## 3(Expression x, Expression y, Expression z);

#define VECTOR4(T)                                                                                 \
SCALAR(T ## 4)                                                                                     \
Expression T ## 4(Expression x, Expression y, Expression z, Expression w);

#define VECTOR(T)                                                                                  \
VECTOR2(T)                                                                                         \
VECTOR3(T)                                                                                         \
VECTOR4(T)

VECTOR(Bool)
VECTOR(Float)
VECTOR(Half)
VECTOR(Int)
VECTOR(Short)

const SkSL::Type& Array(const SkSL::Type& base, int count);

} // namespace skslcode

#endif
