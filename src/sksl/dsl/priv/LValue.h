/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_LVALUE
#define SKSL_DSL_LVALUE

namespace skslcode {

template<class T>
class LValue : public T {
};

} // namespace skslcode

#endif
