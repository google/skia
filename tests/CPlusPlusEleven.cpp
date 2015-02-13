/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"

namespace {
template <class T> T&& Move(T& o) { return static_cast<T&&>(o); }

class Moveable {
public:
    Moveable() {}
    Moveable(Moveable&&) {}
    Moveable& operator=(Moveable&&) { return *this; }
private:
    Moveable(const Moveable&);
    Moveable& operator=(const Moveable&);
};
} // namespace

DEF_TEST(CPlusPlusEleven_RvalueAndMove, r) {
    Moveable src1; Moveable dst1(Move(src1));
    Moveable src2, dst2; dst2 = Move(src2);
}
