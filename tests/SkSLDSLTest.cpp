/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/DSL.h"
#include "src/sksl/dsl/priv/DSLWriter.h"

#include "tests/Test.h"

using namespace SkSL::dsl;

class AutoDSLContext {
public:
    AutoDSLContext(GrGpu* gpu) {
        SkDebugf("CALLING START\n");
        Start(gpu->shaderCompiler());
        SkDebugf("FINISHED CALLING START\n");
    }

    ~AutoDSLContext() {
        End();
    }
};

DEF_GPUTEST_FOR_ALL_CONTEXTS(DSLStartupMODIFIED, r, ctxInfo) {
    SkDebugf("TEST START\n");
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    SkDebugf("STARTUP FINISHED\n");
    Expression e1 = 1;
    REPORTER_ASSERT(r, e1.release()->description() == "1");
    Expression e2 = 1.0;
    REPORTER_ASSERT(r, e2.release()->description() == "1.0");
    Expression e3 = true;
    REPORTER_ASSERT(r, e3.release()->description() == "true");
    SkDebugf("TEST END\n");
}
