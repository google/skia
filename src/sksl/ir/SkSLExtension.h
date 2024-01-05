/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXTENSION
#define SKSL_EXTENSION

#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLProgramElement.h"

#include <memory>
#include <string>
#include <string_view>

namespace SkSL {

class Context;

/**
 * #extension <name> : enable
 */
class Extension final : public ProgramElement {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kExtension;

    Extension(Position pos, std::string_view name)
            : INHERITED(pos, kIRNodeKind)
            , fName(name) {}

    std::string_view name() const {
        return fName;
    }

    // Reports errors via ErrorReporter. This may return null even if no error occurred;
    // in particular, if the behavior text is `disabled`, no ProgramElement is necessary.
    static std::unique_ptr<Extension> Convert(const Context& context,
                                              Position pos,
                                              std::string_view name,
                                              std::string_view behaviorText);

    // Asserts if an error is detected.
    static std::unique_ptr<Extension> Make(const Context& context,
                                           Position pos,
                                           std::string_view name);

    std::string description() const override {
        return "#extension " + std::string(this->name()) + " : enable";
    }

private:
    std::string_view fName;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
