/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_VEC
#define SKSL_DSL_VEC

#include "src/sksl/SkSLContext.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/dsl/priv/Type.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLIntLiteral.h"

namespace SkSL {

class Expression;
class Statement;

} // namespace SkSL

namespace skslcode {

template<class T, class X, class Y>
class Vec2 : Type {
public:
    Vec2() = default;

    template<class X1 = X>
    Vec2(typename std::enable_if<std::is_convertible<X*, T*>::value &&
                                 std::is_convertible<Y*, T*>::value, X1>::type x, Y y)
        : fX(x)
        , fY(y) {}

    const SkSL::Type& type() const {
        return T::Type.type().toCompound(DSLWriter::Instance().context(), 2, 1);
    }

    std::unique_ptr<SkSL::Expression> expression() const {
        SkSL::ExpressionArray args;
        args.push_back(fX.expression());
        args.push_back(fY.expression());
        return std::make_unique<SkSL::Constructor>(/*offset=*/-1, &this->type(), std::move(args));
    }

    std::unique_ptr<SkSL::Statement> statement() const {
        return std::make_unique<SkSL::ExpressionStatement>(this->expression());
    }

private:
    X fX;
    Y fY;
};

template<class T, class X, class Y, class Z>
class Vec3 : Type {
public:
    Vec3() = default;

    template<class X1 = X>
    Vec3(typename std::enable_if<std::is_convertible<X*, T*>::value &&
                                 std::is_convertible<Y*, T*>::value &&
                                 std::is_convertible<Z*, T*>::value, X1>::type x, Y y, Z z)
        : fX(x)
        , fY(y)
        , fZ(z) {}

    const SkSL::Type& type() const {
        return T().type().toCompound(DSLWriter::Instance().context(), 3, 1);
    }

    std::unique_ptr<SkSL::Expression> expression() const {
        SkSL::ExpressionArray args;
        args.push_back(fX.expression());
        args.push_back(fY.expression());
        args.push_back(fZ.expression());
        return std::make_unique<SkSL::Constructor>(/*offset=*/-1, &this->type(), std::move(args));
    }

    std::unique_ptr<SkSL::Statement> statement() const {
        return std::make_unique<SkSL::ExpressionStatement>(this->expression());
    }

private:
    X fX;
    Y fY;
    Z fZ;
};

template<class T, class X, class Y, class Z, class W>
class Vec4 : Type {
public:
    Vec4() = default;

    template<class X1 = X>
    Vec4(typename std::enable_if<std::is_convertible<X*, T*>::value &&
                                 std::is_convertible<Y*, T*>::value &&
                                 std::is_convertible<Z*, T*>::value &&
                                 std::is_convertible<W*, T*>::value, X1>::type x, Y y, Z z, W w)
        : fX(x)
        , fY(y)
        , fZ(z)
        , fW(w) {}

    const SkSL::Type& type() const {
        return T().type().toCompound(DSLWriter::Instance().context(), 4, 1);
    }

    std::unique_ptr<SkSL::Expression> expression() const {
        SkSL::ExpressionArray args;
        args.push_back(fX.expression());
        args.push_back(fY.expression());
        args.push_back(fZ.expression());
        args.push_back(fW.expression());
        return std::make_unique<SkSL::Constructor>(/*offset=*/-1, &this->type(), std::move(args));
    }

    std::unique_ptr<SkSL::Statement> statement() const {
        return std::make_unique<SkSL::ExpressionStatement>(this->expression());
    }

private:
    X fX;
    Y fY;
    Z fZ;
    W fW;
};

} //namespace skslcode

#endif
