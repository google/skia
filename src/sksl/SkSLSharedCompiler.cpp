/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLSharedCompiler.h"

#ifdef SK_ENABLE_SKSL

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLUtil.h"

#include <memory>

namespace SkSL {
struct SharedCompiler::Impl {
    Impl() {
        // These caps are configured to apply *no* workarounds. This avoids changes that are
        // unnecessary (GLSL intrinsic rewrites), or possibly even incorrect.
        // We may apply other "neutral" transformations to the user's SkSL, including inlining.
        // Anything determined by the device caps is deferred to the GPU backend. The processor
        // set produces the final program (including our re-emitted SkSL), and the backend's
        // compiler resolves any necessary workarounds.
        fCaps = ShaderCapsFactory::Standalone();
        fCaps->fBuiltinFMASupport = true;
        fCaps->fBuiltinDeterminantSupport = true;
        fCompiler = new SkSL::Compiler(fCaps.get());
    }

    std::unique_ptr<SkSL::ShaderCaps> fCaps;
    SkSL::Compiler*                   fCompiler;
};

SharedCompiler::Impl* SharedCompiler::gImpl = nullptr;

SharedCompiler::SharedCompiler() : fLock(compiler_mutex()) {
    if (!gImpl) {
        gImpl = new Impl();
    }
}

SkSL::Compiler* SharedCompiler::operator->() const { return gImpl->fCompiler; }

SkMutex& SharedCompiler::compiler_mutex() {
    static SkMutex& mutex = *(new SkMutex);
    return mutex;
}

}  // namespace SkSL

#endif
