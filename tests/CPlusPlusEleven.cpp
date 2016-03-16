/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkTemplates.h"
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
