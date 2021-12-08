/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLSharedCompiler.h"

#ifdef SK_ENABLE_SKSL

namespace SkSL {
struct SharedCompiler::Impl {
    Impl() {
        // These caps are configured to apply *no* workarounds. This avoids changes that are
        // unnecessary (GLSL intrinsic rewrites), or possibly incorrect (adding do-while loops).
        // We may apply other "neutral" transformations to the user's SkSL, including inlining.
        // Anything determined by the device caps is deferred to the GPU backend. The processor
        // set produces the final program (including our re-emitted SkSL), and the backend's
        // compiler resolves any necessary workarounds.
        fCaps = ShaderCapsFactory::Standalone();
        fCaps->fBuiltinFMASupport = true;
        fCaps->fBuiltinDeterminantSupport = true;
        // Don't inline if it would require a do loop, some devices don't support them.
        fCaps->fCanUseDoLoops = false;

        // SkSL created by the GPU backend is typically parsed, converted to a backend format,
        // and the IR is immediately discarded. In that situation, it makes sense to use node
        // pools to accelerate the IR allocations. Here, SkRuntimeEffect instances are often
        // long-lived (especially those created internally for runtime FPs). In this situation,
        // we're willing to pay for a slightly longer compile so that we don't waste huge
        // amounts of memory.
        fCaps->fUseNodePools = false;

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
