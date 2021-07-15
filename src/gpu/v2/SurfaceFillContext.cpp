/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/v2/SurfaceFillContext_v2.h"

#include "include/core/SkTypes.h"

//
//            SurfaceContext
//                   |
//          SurfaceFillContext
//           /       |       \
//     v1::SFC       |       v2::SFC
//        |          |           |
//        |  SurfaceDrawContext  |
//        |  /                \  |
//     v1::SDC               v2::SDC
//


class A {
public:
    A() {}
    virtual ~A() {}

    virtual void callA() = 0;
};

class A1 : virtual public A {
public:
    ~A1() override {}

    void callA() override { SkDebugf("A1"); }
};

class A2 : virtual public A {
public:
    ~A2() override {}

    void callA() override { SkDebugf("A2"); }
};

class B: virtual public A {
public:
    ~B() override {}

    virtual void callB() = 0;
};

class B1 : public A1, public B {
public:
    ~B1() override {}

    void callB() override { SkDebugf("B1"); }
};

class B2 : public A2, public B {
public:
    ~B2() override {}

    void callB() override { SkDebugf("B2"); }
};

void cpp_test() {
    A1 a1;
    A2 a2;
    B1 b1;
    B2 b2;

    a1.callA();
    a2.callA();

    b1.callA();
    b1.callB();

    b2.callA();
    b2.callB();
}
