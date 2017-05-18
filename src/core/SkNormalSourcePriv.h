/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNormalSourcePriv_DEFINED
#define SkNormalSourcePriv_DEFINED

#if SK_SUPPORT_GPU
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"

/* GLSLFragmentProcessors for NormalSourceImpls must sub-class this class and override onEmitCode,
 * and setNormalData calls, as well as all other calls FPs normally override, except for the 2
 * defined in this superclass.
 * This class exists to intercept emitCode calls and emit <0, 0, 1> if the FP requires a distance
 * vector but the GP doesn't provide it. onSetData calls need to be intercepted too because
 * uniform handlers will be invalid in subclasses where onEmitCode isn't called.
 * We don't need to adjust the key here since the use of a given GP (through its class ID already in
 * the key), will determine what code gets emitted here.
 */
class GLSLNormalFP : public GrGLSLFragmentProcessor {
public:
    GLSLNormalFP()
        : fDidIntercept(false) {}

    void emitCode(EmitArgs& args) final override {
        if (args.fFp.usesDistanceVectorField() && !args.fGpImplementsDistanceVector) {
            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
            fragBuilder->codeAppendf("// GLSLNormalFP intercepted emitCode call, GP does not "
                                             "implement required distance vector feature\n");
            fragBuilder->codeAppendf("%s = vec4(0, 0, 1, 0);", args.fOutputColor);

            fDidIntercept = true;
        } else {
            this->onEmitCode(args);
        }
    }

    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& proc) final override {
        if (!fDidIntercept) {
            this->setNormalData(pdman, proc);
        }
    }

protected:
    virtual void onEmitCode(EmitArgs& args) = 0;
    virtual void setNormalData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) = 0;

private:
    bool fDidIntercept;
};
#endif

#endif
