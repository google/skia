/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLSharedCompiler.h"

#include "src/sksl/SkSLCompiler.h"

namespace SkSL {

SharedCompiler::Impl* SharedCompiler::gImpl;

SharedCompiler::Impl::Impl() {
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

    fCompiler = new SkSL::Compiler(fCaps.get());
}

}
