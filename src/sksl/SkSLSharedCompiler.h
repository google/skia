/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SHARED_COMPILER
#define SKSL_SHARED_COMPILER

#include "include/private/SkMutex.h"
#include "src/sksl/SkSLUtil.h"

namespace SkSL {

class Compiler;

class SharedCompiler {
public:
    SharedCompiler() : fLock(compiler_mutex()) {
        if (!gImpl) {
            gImpl = new Impl();
        }
    }

    SkSL::Compiler* get() const { return gImpl->fCompiler; }

    SkSL::Compiler* operator->() const { return this->get(); }

private:
    SkAutoMutexExclusive fLock;

    static SkMutex& compiler_mutex() {
        static SkMutex& mutex = *(new SkMutex);
        return mutex;
    }

    struct Impl {
        Impl();

        SkSL::ShaderCapsPointer fCaps;
        SkSL::Compiler*         fCompiler;
    };

    static Impl* gImpl;
};

} // namespace SkSL

#endif
