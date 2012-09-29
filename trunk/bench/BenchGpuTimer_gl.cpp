
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "BenchGpuTimer_gl.h"
#include "gl/SkGLContext.h"
#include "gl/GrGLUtil.h"

BenchGpuTimer::BenchGpuTimer(const SkGLContext* glctx) {
    fContext = glctx;
    glctx->ref();
    glctx->makeCurrent();
    fStarted = false;
    fSupported = GrGLGetVersion(glctx->gl()) > GR_GL_VER(3,3) ||
                 GrGLHasExtension(glctx->gl(), "GL_ARB_timer_query") ||
                 GrGLHasExtension(glctx->gl(), "GL_EXT_timer_query");

    if (fSupported) {
        SK_GL(*glctx, GenQueries(1, &fQuery));
    }
}

BenchGpuTimer::~BenchGpuTimer() {
    if (fSupported) {
        fContext->makeCurrent();
        SK_GL(*fContext, DeleteQueries(1, &fQuery));
    }
    fContext->unref();
}

void BenchGpuTimer::startGpu() {
    if (fSupported) {
        fContext->makeCurrent();
        fStarted = true;
        SK_GL(*fContext, BeginQuery(GR_GL_TIME_ELAPSED, fQuery));
    }
}

/**
 * It is important to stop the cpu clocks first,
 * as this will cpu wait for the gpu to finish.
 */
double BenchGpuTimer::endGpu() {
    if (fSupported) {
        fStarted = false;
        fContext->makeCurrent();
        SK_GL(*fContext, EndQuery(GR_GL_TIME_ELAPSED));

        GrGLint available = 0;
        while (!available) {
            SK_GL(*fContext, GetQueryObjectiv(fQuery,
                                             GR_GL_QUERY_RESULT_AVAILABLE,
                                             &available));
        }
        GrGLuint64 totalGPUTimeElapsed = 0;
        SK_GL(*fContext, GetQueryObjectui64v(fQuery,
                                             GR_GL_QUERY_RESULT,
                                             &totalGPUTimeElapsed));

        return totalGPUTimeElapsed / 1000000.0;
    } else {
        return 0;
    }
}
