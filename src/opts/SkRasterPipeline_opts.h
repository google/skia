/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterPipeline_opts_DEFINED
#define SkRasterPipeline_opts_DEFINED

#include <string.h>

namespace SK_OPTS_NS {

    // First some helper functions used all over the place.
    template <typename T>
    static T unaligned_load(const void* ptr) {
        T v;
        memcpy(&v, ptr, sizeof(v));
        return v;
    }

    template <typename T>
    static void unaligned_store(void* ptr, T v) {
        memcpy(ptr, &v, sizeof(v));
    }

    template <typename Dst, typename Src>
    static Dst bit_cast(const Src& src) {
        static_assert(sizeof(Dst) == sizeof(Src), "");
        return unaligned_load<Dst>(&src);
    }

    template <typename Dst, typename Src>
    static Dst widen_cast(const Src& src) {
        static_assert(sizeof(Dst) > sizeof(Src), "");
        Dst dst;
        memcpy(&dst, &src, sizeof(Src));
        return dst;
    }

    // Our program is an array of void*, interlacing stage function pointers and their contexts.
    // load_and_inc() steps the program forward by one pointer.
    static void* load_and_inc(void**& program) {
        // TODO: platform specific implementatiosn like lodsq on x86-64.
        return *program++;
    }

    // Lazily resolved on first cast.  Does nothing if cast to Ctx::None.
    struct Ctx {
        struct None {};

        void*   ptr;
        void**& program;

        explicit Ctx(void**& p) : ptr(nullptr), program(p) {}

        template <typename T>
        operator T*() {
            if (!ptr) { ptr = load_and_inc(program); }
            return (T*)ptr;
        }
        operator None() { return None{}; }
    };


    static void start_pipeline(size_t x, size_t y, size_t w, size_t h, void** program) {
        // TODO
    }

    static void start_pipeline_lowp(size_t x, size_t y, size_t w, size_t h, void** program) {
        // TODO
    }

    static void(*just_return     )() = nullptr;
    static void(*just_return_lowp)() = nullptr;

    #define M(st) static void(*st)() = nullptr; static void(*st##_lowp)() = nullptr;
        SK_RASTER_PIPELINE_STAGES(M)
    #undef M
}

#endif//SkRasterPipeline_opts_DEFINED

