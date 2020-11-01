/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_VAR
#define SKSL_DSL_VAR

#include "src/sksl/dsl/Bool.h"
#include "src/sksl/dsl/Int.h"
#include "src/sksl/dsl/priv/Assignment.h"
#include "src/sksl/dsl/priv/LValue.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#include <type_traits>

namespace skslcode {

template<class T>
class Var : public T {
public:
    Var(const char* name)
        : fName(name)
        , fIndex(writer().declare(std::make_unique<SkSL::Variable>(
                                                              /*offset=*/-1,
                                                              writer().modifiers(SkSL::Modifiers()),
                                                              fName,
                                                              &this->type(),
                                                              /*builtin=*/false,
                                                              SkSL::Variable::Storage::kLocal))) {}

    Var(const Var& other)
        : fName(other.fName)
        , fIndex(other.fIndex) {}

    Assignment<Var, Var, T> operator=(const Var& other) {
        return Assignment<Var, Var, T>(Var(*this), Var(other));
    }

    const char* name() const {
        return fName;
    }

    const SkSL::Variable* variable() const {
        return writer().var(fIndex);
    }

    std::unique_ptr<SkSL::Expression> expression() const {
        return std::make_unique<SkSL::VariableReference>(/*offset=*/-1, this->variable(),
                                                         SkSL::VariableReference::RefKind::kRead);
    }

    std::unique_ptr<SkSL::Expression> lvalue() const {
        return std::make_unique<SkSL::VariableReference>(/*offset=*/-1, this->variable(),
                                                         SkSL::VariableReference::RefKind::kWrite);
    }

    template<class Right>
    Assignment<Var, typename std::enable_if<std::is_convertible<Right*, T>::value, Right*>::type, T>
    operator=(Right&& value) {
        return Assignment<Var, Right, T>(Var(*this), std::move(value));
    }

    // enable assignment to Int variables from literal ints
    template<class Test = T>
    Assignment<Var, typename std::enable_if<std::is_same<T, Int>::value, Test>::type, T>
    operator=(int value) {
        return Assignment<Var, T, T>(Var(*this), Int(value));
    }

    // enable assignment to Bool variables from literal bools
    template<class Test = T>
    Assignment<Var, typename std::enable_if<std::is_same<T, Bool>::value, Test>::type, T>
    operator=(bool value) {
        return Assignment<Var, T, T>(Var(*this), Bool(value));
    }

private:
	const char* fName;
    const int fIndex;

    friend class DSLWriter;
};

} // namespace skslcode

#endif
