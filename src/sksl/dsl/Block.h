/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_BLOCK
#define SKSL_DSL_BLOCK

#include "src/sksl/dsl/priv/BlockType.h"

namespace skslcode {

template<class... T>
BlockType<T...> Block() {
    return BlockType<T...>();
}

template<class First, class... Rest>
BlockType<First, Rest...> Block(First first, Rest... rest) {
    return BlockType<First, Rest...>(first, rest...);
}

} // namespace skslcode

#endif
