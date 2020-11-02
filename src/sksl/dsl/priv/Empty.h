/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_EMPTY
#define SKSL_DSL_EMPTY

namespace skslcode {

class Empty : public Statement {
public:
    std::unique_ptr<SkSL::Statement> statement() const {
        return nullptr;
    }
};

} // namespace skslcode

#endif
