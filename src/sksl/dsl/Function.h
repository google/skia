/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_FUNCTION
#define SKSL_DSL_FUNCTION

#include "src/sksl/dsl/Block.h"

namespace skslcode {

template<class... Types>
class Parameters {
public:
    Parameters() {}
};

template<class First, class... Rest>
class Parameters<First, Rest...> : Parameters<Rest...> {
public:
    Parameters(First first, Rest... rest)
        : Parameters<Rest...>(rest...)
        , fFirst(first) {}

private:
    First fFirst;
};

template<class Result, class... ParameterTypes>
class Function {
public:
    Function(const char* name, ParameterTypes... parameters)
        : fParameters(parameters...) {}

    template<class... Statements>
    void define(Statements&&... stmts) {
    }

    template<class First, class... Rest>
    inline void define(First&& first, Rest&&... rest) {
        printf("%s\n", first.statement()->description().c_str());
        this->define(rest...);
    }
private:
    const char* fName;
    Parameters<ParameterTypes...> fParameters;
};

} // namespace skslcode

#endif
