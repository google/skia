/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkTemplates.h"
#include "SkScopeExit.h"
#include <utility>

namespace {
class Moveable {
public:
    Moveable() {}
    Moveable(Moveable&&) {}
    Moveable& operator=(Moveable&&) { return *this; }
private:
    Moveable(const Moveable&);
    Moveable& operator=(const Moveable&);
};
template <typename T> void deleter(T*) { }
template <typename T> struct Deleter {
    void operator()(T* t) { delete static_cast<const Moveable*>(t); }
};
} // namespace

DEF_TEST(CPlusPlusEleven_RvalueAndMove, r) {
    Moveable src1; Moveable dst1(std::move(src1));
    Moveable src2, dst2; dst2 = std::move(src2);
}

DEF_TEST(CPlusPlusEleven_constexpr, r) {
    static constexpr int x = Sk32ToBool(50);
    REPORTER_ASSERT(r, x == 1);
    static constexpr int y = SkTPin<int>(100, 0, 10);
    REPORTER_ASSERT(r, y == 10);
}

namespace {
struct MoveableCopyable {
    bool fCopied;
    MoveableCopyable() : fCopied(false) {}
    MoveableCopyable(const MoveableCopyable &o) : fCopied(true) {}
    MoveableCopyable(MoveableCopyable &&o) : fCopied(o.fCopied) {}
};
struct TestClass {
    MoveableCopyable fFoo;
};
}  // namespace

DEF_TEST(CPlusPlusEleven_default_move, r) {
    TestClass a;
    TestClass b(a);
    TestClass c(std::move(a));
    REPORTER_ASSERT(r, b.fFoo.fCopied);
    REPORTER_ASSERT(r, !c.fFoo.fCopied);
}

DEF_TEST(SkAtScopeExit, r) {
    int x = 5;
    {
        SK_AT_SCOPE_EXIT(x--);
        REPORTER_ASSERT(r, x == 5);
    }
    REPORTER_ASSERT(r, x == 4);
}
