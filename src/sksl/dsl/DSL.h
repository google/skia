/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL
#define SKSL_DSL

#include "src/sksl/dsl/Bool.h"
#include "src/sksl/dsl/Declare.h"
#include "src/sksl/dsl/Function.h"
#include "src/sksl/dsl/If.h"
#include "src/sksl/dsl/Int.h"
#include "src/sksl/dsl/Var.h"

#include "src/sksl/dsl/priv/Binary.h"

namespace skslcode {

#define DSL_OP(op, token, sksl, native, result)                                                    \
template<class Left, class Right>                                                                  \
skslcode::Binary<typename std::enable_if<std::is_convertible<Left*, skslcode::sksl*>::value,       \
                                         Left>::type,                                              \
                 typename std::enable_if<std::is_convertible<Right*, skslcode::sksl*>::value,      \
                                         Right>::type,                                             \
                 skslcode::result>                                                                 \
operator op(Left left, Right right) {                                                              \
    return skslcode::Binary<Left, Right, skslcode::result>(std::move(left),                        \
                                                           SkSL::Token::Kind::token,               \
                                                           std::move(right));                      \
}                                                                                                  \
                                                                                                   \
template<class Left>                                                                               \
skslcode::Binary<typename std::enable_if<std::is_convertible<Left*, skslcode::sksl*>::value,       \
                                         Left>::type,                                              \
                 sksl, skslcode::result>                                                           \
operator op(Left left, native right) {                                                             \
    return std::move(left) op sksl(right);                                                         \
}                                                                                                  \
                                                                                                   \
template<class Right>                                                                              \
skslcode::Binary<sksl,                                                                             \
                 typename std::enable_if<std::is_convertible<Right*, skslcode::sksl*>::value,      \
                                         Right>::type,                                             \
                 skslcode::result>                                                                 \
operator op(native left, Right right) {                                                            \
    return sksl(left) op std::move(right);                                                         \
}                                                                                                  \

DSL_OP(>, TK_GT, Int, int, Bool)
DSL_OP(<, TK_LT, Int, int, Bool)
DSL_OP(>=, TK_GTEQ, Int, int, Bool)
DSL_OP(<=, TK_LTEQ, Int, int, Bool)
DSL_OP(==, TK_EQEQ, Int, int, Bool)
DSL_OP(!=, TK_NEQ, Int, int, Bool)
DSL_OP(+, TK_PLUS, Int, int, Int)
DSL_OP(-, TK_MINUS, Int, int, Int)
DSL_OP(*, TK_STAR, Int, int, Int)
DSL_OP(/, TK_SLASH, Int, int, Int)

}
#endif
