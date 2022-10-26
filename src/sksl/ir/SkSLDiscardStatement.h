/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DISCARDSTATEMENT
#define SKSL_DISCARDSTATEMENT

#include "include/private/SkSLIRNode.h"
#include "include/private/SkSLStatement.h"
#include "include/sksl/SkSLPosition.h"

#include <memory>
#include <string>

namespace SkSL {

class Context;

/**
 * A 'discard' statement.
 */
class DiscardStatement final : public Statement {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kDiscard;

    DiscardStatement(Position pos) : INHERITED(pos, kIRNodeKind) {}

    // Creates a discard-statement; reports errors via ErrorReporter.
    static std::unique_ptr<Statement> Convert(const Context& context, Position pos);

    // Creates a discard-statement; reports errors via SkASSERT.
    static std::unique_ptr<Statement> Make(const Context& context, Position pos);

    std::unique_ptr<Statement> clone() const override;

    std::string description() const override {
        return "discard;";
    }

private:
    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
