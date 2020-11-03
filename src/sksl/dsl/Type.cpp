/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLConstructor.h"

namespace skslcode {

#define SCALAR(T)                                                                                  \
const SkSL::Type& T() {                                                                            \
    return *DSLWriter::Instance().context().f ## T ## _Type;                                       \
}                                                                                                  \
Expression T(Expression expr) {                                                                    \
    SkSL::ExpressionArray args;                                                                    \
    args.push_back(expr.release());                                                                \
    return Expression(std::make_unique<SkSL::Constructor>(/*offset=*/-1, &T(), std::move(args)));  \
}

SCALAR(Bool)
SCALAR(Float)
SCALAR(Half)
SCALAR(Int)
SCALAR(Short)

#define VECTOR2(T)                                                                                 \
SCALAR(T ## 2)                                                                                     \
Expression T ## 2(Expression x, Expression y) {                                                    \
    SkSL::ExpressionArray args;                                                                    \
    args.push_back(x.release());                                                                   \
    args.push_back(y.release());                                                                   \
    return Expression(std::make_unique<SkSL::Constructor>(/*offset=*/-1, &T ## 2(),                \
                      std::move(args)));                                                           \
}

#define VECTOR3(T)                                                                                 \
SCALAR(T ## 3)                                                                                     \
Expression T ## 3(Expression x, Expression y, Expression z) {                                      \
    SkSL::ExpressionArray args;                                                                    \
    args.push_back(x.release());                                                                   \
    args.push_back(y.release());                                                                   \
    args.push_back(z.release());                                                                   \
    return Expression(std::make_unique<SkSL::Constructor>(/*offset=*/-1, &T ## 3(),                \
                      std::move(args)));                                                           \
}

#define VECTOR4(T)                                                                                 \
SCALAR(T ## 4)                                                                                     \
Expression T ## 4(Expression x, Expression y, Expression z, Expression w) {                        \
    SkSL::ExpressionArray args;                                                                    \
    args.push_back(x.release());                                                                   \
    args.push_back(y.release());                                                                   \
    args.push_back(z.release());                                                                   \
    args.push_back(w.release());                                                                   \
    return Expression(std::make_unique<SkSL::Constructor>(/*offset=*/-1, &T ## 4(),                \
                      std::move(args)));                                                           \
}

#define VECTOR(T)                                                                                  \
VECTOR2(T)                                                                                         \
VECTOR3(T)                                                                                         \
VECTOR4(T)

VECTOR(Bool)
VECTOR(Float)
VECTOR(Half)
VECTOR(Int)
VECTOR(Short)

} // namespace skslcode
