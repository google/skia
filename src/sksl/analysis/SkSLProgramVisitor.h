/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSLProgramVisitor_DEFINED
#define SkSLProgramVisitor_DEFINED

#include <memory>

namespace SkSL {

struct Program;
class Expression;
class Statement;
class ProgramElement;

/**
 * Utility class to visit every element, statement, and expression in an SkSL program IR.
 * This is intended for simple analysis and accumulation, where custom visitation behavior is only
 * needed for a limited set of expression kinds.
 *
 * Subclasses should override visitExpression/visitStatement/visitProgramElement as needed and
 * intercept elements of interest. They can then invoke the base class's function to visit all
 * sub expressions. They can also choose not to call the base function to arrest recursion, or
 * implement custom recursion.
 *
 * The visit functions return a bool that determines how the default implementation recurses. Once
 * any visit call returns true, the default behavior stops recursing and propagates true up the
 * stack.
 */
template <typename T>
class TProgramVisitor {
public:
    virtual ~TProgramVisitor() = default;

protected:
    virtual bool visitExpression(typename T::Expression& expression);
    virtual bool visitStatement(typename T::Statement& statement);
    virtual bool visitProgramElement(typename T::ProgramElement& programElement);

    virtual bool visitExpressionPtr(typename T::UniquePtrExpression& expr) = 0;
    virtual bool visitStatementPtr(typename T::UniquePtrStatement& stmt) = 0;
};

// ProgramVisitors take const types; ProgramWriters do not.
struct ProgramVisitorTypes {
    using Program = const SkSL::Program;
    using Expression = const SkSL::Expression;
    using Statement = const SkSL::Statement;
    using ProgramElement = const SkSL::ProgramElement;
    using UniquePtrExpression = const std::unique_ptr<SkSL::Expression>;
    using UniquePtrStatement = const std::unique_ptr<SkSL::Statement>;
};

// Squelch bogus Clang warning about template vtables: https://bugs.llvm.org/show_bug.cgi?id=18733
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wweak-template-vtables"
#endif
extern template class TProgramVisitor<ProgramVisitorTypes>;
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

class ProgramVisitor : public TProgramVisitor<ProgramVisitorTypes> {
public:
    bool visit(const Program& program);

private:
    // ProgramVisitors shouldn't need access to unique_ptrs, and marking these as final should help
    // these accessors inline away. Use ProgramWriter if you need the unique_ptrs.
    bool visitExpressionPtr(const std::unique_ptr<Expression>& e) final {
        return this->visitExpression(*e);
    }
    bool visitStatementPtr(const std::unique_ptr<Statement>& s) final {
        return this->visitStatement(*s);
    }
};

} // namespace SkSL

#endif
