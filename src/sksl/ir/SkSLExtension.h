/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXTENSION
#define SKSL_EXTENSION

namespace SkSL {

/**
 * An extension declaration.
 */
struct Extension : public IRNode {
    static constexpr Kind kIRNodeKind = kExtension_Kind;

    Extension(int offset, String name)
    : INHERITED(offset, kIRNodeKind)
    , fName(std::move(name)) {}

    std::unique_ptr<IRNode> clone() const override {
        return std::unique_ptr<IRNode>(new Extension(fOffset, fName));
    }

    String description() const override {
        return "#extension " + fName + " : enable";
    }

    const String fName;

    typedef IRNode INHERITED;
};

}  // namespace SkSL

#endif
