/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_SWIZZLETYPE
#define SKSL_DSL_SWIZZLETYPE

#include "src/sksl/dsl/SwizzleComponent.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLSwizzle.h"

namespace skslcode {

static SkSL::String swizzle_component(SwizzleComponent c) {
    switch (c) {
        case R:
            return "r";
        case G:
            return "g";
        case B:
            return "b";
        case A:
            return "a";
        case X:
            return "x";
        case Y:
            return "y";
        case Z:
            return "z";
        case W:
            return "w";
        case ZERO:
            return "0";
        case ONE:
            return "1";
    }
}

template<class Base>
class Swizzle1 {
public:
    Swizzle1(Base base, SwizzleComponent a)
        : fBase(base)
        , fMask(swizzle_component(a)) {}

    std::unique_ptr<SkSL::Expression> expression() const {
        return DSLWriter::Instance().irGenerator().convertSwizzle(fBase.expression(),
                                                                  fMask);
    }

    std::unique_ptr<SkSL::Statement> statement() const {
        return std::make_unique<SkSL::ExpressionStatement>(this->expression());
    }

private:
    Base fBase;
    SkSL::String fMask;
};

template<class Base>
class Swizzle2 {
public:
    Swizzle2(Base base, SwizzleComponent a, SwizzleComponent b)
        : fBase(base)
        , fMask(swizzle_component(a) + swizzle_component(b)) {}

    std::unique_ptr<SkSL::Expression> expression() const {
        return DSLWriter::Instance().irGenerator().convertSwizzle(fBase.expression(),
                                                                  fMask);
    }

    std::unique_ptr<SkSL::Expression> lvalue() const {
        return DSLWriter::Instance().irGenerator().convertSwizzle(
                                                               fBase.lvalue(),
                                                               SkSL::StringFragment(fMask.c_str()));
    }

    std::unique_ptr<SkSL::Statement> statement() const {
        return std::make_unique<SkSL::ExpressionStatement>(this->expression());
    }

private:
    Base fBase;
    SkSL::String fMask;
};

template<class Base>
class Swizzle3 {
public:
    Swizzle3(Base base, SwizzleComponent a, SwizzleComponent b, SwizzleComponent c)
        : fBase(base)
        , fMask(swizzle_component(a) + swizzle_component(b) + swizzle_component(c)) {}

    Swizzle3(const Swizzle3&) = default;

    template<class Right>
    Assignment<Swizzle3, Right, typename Base::Type>
    operator=(Right&& value) {
        return Assignment<Swizzle3, Right, typename Base::Type>(Swizzle3(*this), std::move(value));
    }

    std::unique_ptr<SkSL::Expression> expression() const {
        return DSLWriter::Instance().irGenerator().convertSwizzle(fBase.expression(),
                                                                  fMask);
    }

    std::unique_ptr<SkSL::Expression> lvalue() const {
        return DSLWriter::Instance().irGenerator().convertSwizzle(
                                                               fBase.lvalue(),
                                                               SkSL::StringFragment(fMask.c_str()));
    }

    std::unique_ptr<SkSL::Statement> statement() const {
        return std::make_unique<SkSL::ExpressionStatement>(this->expression());
    }

private:
    Base fBase;
    SkSL::String fMask;
};

template<class Base>
class Swizzle4 {
public:
    Swizzle4(Base base, SwizzleComponent a, SwizzleComponent b, SwizzleComponent c,
             SwizzleComponent d)
        : fBase(base)
        , fMask(swizzle_component(a) + swizzle_component(b) + swizzle_component(c) +
                swizzle_component(d)) {}

    Swizzle4(const Swizzle4&) = default;

    template<class Right>
    Assignment<Swizzle4, Right, typename Base::Type>
    operator=(Right&& value) {
        return Assignment<Swizzle4, Right, typename Base::Type>(Swizzle4(*this), std::move(value));
    }

    std::unique_ptr<SkSL::Expression> expression() const {
        return DSLWriter::Instance().irGenerator().convertSwizzle(
                                                               fBase.expression(),
                                                               SkSL::StringFragment(fMask.c_str()));
    }

    std::unique_ptr<SkSL::Expression> lvalue() const {
        return DSLWriter::Instance().irGenerator().convertSwizzle(
                                                               fBase.lvalue(),
                                                               SkSL::StringFragment(fMask.c_str()));
    }

    std::unique_ptr<SkSL::Statement> statement() const {
        return std::make_unique<SkSL::ExpressionStatement>(this->expression());
    }

private:
    Base fBase;
    SkSL::String fMask;
};

} // namespace skslcode

#endif
