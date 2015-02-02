/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkInstCnt_DEFINED
#define SkInstCnt_DEFINED

/* To count all instances of T, including all subclasses of T,
 * add SK_DECLARE_INST_COUNT(T) to T's class definition.
 * If you want to print out counts of leaked instances, set gPrintInstCount to true in main().
 *
 * E.g.
 *   struct Base { SK_DECLARE_INST_COUNT(Base) };
 *   struct A : public Base {};
 *   struct SubBase : public Base { SK_DECLARE_INST_COUNT(SubBase); }
 *   struct B : public SubBase {};
 *
 * If gPrintInstCount is true, at the program exit you will see something like:
 *   Base: <N> leaked instances
 *   SubBase: <M> leaked instances
 * where N >= M.  Leaked instances of A count against Base; leaked instances of B count against
 * both SubBase and Base.
 *
 * If SK_ENABLE_INST_COUNT is not defined or defined to 0, or we're in a shared library build,
 * this entire system is compiled away to a noop.
 */

#include "SkTypes.h"

#if SK_ENABLE_INST_COUNT && !defined(SKIA_DLL) // See skia:2058 for why we noop on shared builds.
    #include "SkThread.h"
    #include <stdlib.h>

    #define SK_DECLARE_INST_COUNT(T)                           \
        static const char* InstCountClassName() { return #T; } \
        SkInstCount<T, T::InstCountClassName> fInstCnt;        \
        static int32_t GetInstanceCount() { return SkInstCount<T, InstCountClassName>::Count(); }

    extern bool gPrintInstCount;

    template <typename T, const char*(Name)()>
    class SkInstCount {
    public:
        SkInstCount()                   { Inc(); }
        SkInstCount(const SkInstCount&) { Inc(); }
        ~SkInstCount()                  { sk_atomic_dec(&gCount); }

        SkInstCount& operator==(const SkInstCount&) { return *this; } // == can't change the count.

        static void Inc() {
            // If it's the first time we go from 0 to 1, register to print leaks at process exit.
            if (0 == sk_atomic_inc(&gCount) && sk_atomic_cas(&gRegistered, 0, 1)) {
                atexit(PrintAtExit);
            }
        }

        static void PrintAtExit() {
            int32_t leaks = Count();
            if (gPrintInstCount && leaks > 0) {
                SkDebugf("Leaked %s: %d\n", Name(), leaks);
            }
        }

        // FIXME: Used publicly by unit tests.  Seems like a bad idea in a DM world.
        static int32_t Count() { return sk_acquire_load(&gCount); }

    private:
        static int32_t gCount, gRegistered;
    };
    // As template values, these will be deduplicated.  (No one-definition rule problems.)
    template <typename T, const char*(Name)()> int32_t SkInstCount<T, Name>::gCount      = 0;
    template <typename T, const char*(Name)()> int32_t SkInstCount<T, Name>::gRegistered = 0;
#else
    #define SK_DECLARE_INST_COUNT(T)
#endif

void SkInstCountPrintLeaksOnExit();

#endif // SkInstCnt_DEFINED
