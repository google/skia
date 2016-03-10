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

#define TOO_BIG "The unique_ptr was bigger than expected."
#define WEIRD_SIZE "The unique_ptr was a different size than expected."

DEF_TEST(CPlusPlusEleven_UniquePtr, r) {
    struct SmallUniquePtr {
        Moveable* p;
    };
    struct BigUniquePtr {
        void(*d)(Moveable*);
        Moveable* p;
    };

    static_assert(sizeof(skstd::unique_ptr<Moveable>) == sizeof(SmallUniquePtr), TOO_BIG);
    static_assert(sizeof(skstd::unique_ptr<Moveable[]>) == sizeof(SmallUniquePtr), TOO_BIG);

    using proc = void(*)(Moveable*);
    static_assert(sizeof(skstd::unique_ptr<Moveable, proc>) == sizeof(BigUniquePtr), WEIRD_SIZE);
    static_assert(sizeof(skstd::unique_ptr<Moveable[], proc>) == sizeof(BigUniquePtr), WEIRD_SIZE);

    {
        skstd::unique_ptr<Moveable, void(*)(Moveable*)> u(nullptr, deleter<Moveable>);
        static_assert(sizeof(u) == sizeof(BigUniquePtr), WEIRD_SIZE);

        auto u2 = std::move(u);
        static_assert(sizeof(u2) == sizeof(BigUniquePtr), WEIRD_SIZE);
    }

    {
        skstd::unique_ptr<Moveable, void(*)(Moveable*)> u(nullptr, [](Moveable* m){ deleter(m); });
        static_assert(sizeof(u) == sizeof(BigUniquePtr), WEIRD_SIZE);

        auto u2 = std::move(u);
        static_assert(sizeof(u2) == sizeof(BigUniquePtr), WEIRD_SIZE);
    }

    {
        auto d = [](Moveable* m){ deleter(m); };
        skstd::unique_ptr<Moveable, decltype(d)> u(nullptr, d);
        static_assert(sizeof(u) == sizeof(SmallUniquePtr), TOO_BIG);

        auto u2 = std::move(u);
        static_assert(sizeof(u2) == sizeof(SmallUniquePtr), TOO_BIG);
    }

    {
        skstd::unique_ptr<Moveable, Deleter<Moveable>> u(nullptr, Deleter<Moveable>());
        static_assert(sizeof(u) == sizeof(SmallUniquePtr), TOO_BIG);

        auto u2 = std::move(u);
        static_assert(sizeof(u2) == sizeof(SmallUniquePtr), TOO_BIG);
    }

    {
        skstd::unique_ptr<Moveable, Deleter<Moveable>> u(new Moveable(), Deleter<Moveable>());
        static_assert(sizeof(u) == sizeof(SmallUniquePtr), TOO_BIG);

        auto u2 = std::move(u);
        static_assert(sizeof(u2) == sizeof(SmallUniquePtr), TOO_BIG);
    }

    {
        skstd::unique_ptr<const void, Deleter<const void>> u(new Moveable(), Deleter<const void>());
        static_assert(sizeof(u) == sizeof(SmallUniquePtr), TOO_BIG);

        auto u2 = std::move(u);
        static_assert(sizeof(u2) == sizeof(SmallUniquePtr), TOO_BIG);
    }
}
