/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/gl/GLTestContext.h"

#include "include/gpu/GrDirectContext.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"
#include "tools/gpu/GpuTimer.h"

namespace {

class GLGpuTimer : public sk_gpu_test::GpuTimer {
public:
    static std::unique_ptr<GLGpuTimer> MakeIfSupported(const sk_gpu_test::GLTestContext*);

    QueryStatus checkQueryStatus(sk_gpu_test::PlatformTimerQuery) override;
    std::chrono::nanoseconds getTimeElapsed(sk_gpu_test::PlatformTimerQuery) override;
    void deleteQuery(sk_gpu_test::PlatformTimerQuery) override;

private:
#ifdef SK_GL
    GLGpuTimer(bool disjointSupport, const sk_gpu_test::GLTestContext*, const char* ext = "");
    bool validate() const;
#endif

    sk_gpu_test::PlatformTimerQuery onQueueTimerStart() const override;
    void onQueueTimerStop(sk_gpu_test::PlatformTimerQuery) const override;

    inline static constexpr GrGLenum GL_QUERY_RESULT            = 0x8866;
    inline static constexpr GrGLenum GL_QUERY_RESULT_AVAILABLE  = 0x8867;
    inline static constexpr GrGLenum GL_TIME_ELAPSED            = 0x88bf;
    inline static constexpr GrGLenum GL_GPU_DISJOINT            = 0x8fbb;

    typedef void (GR_GL_FUNCTION_TYPE* GLGetIntegervProc) (GrGLenum, GrGLint*);
    typedef void (GR_GL_FUNCTION_TYPE* GLGenQueriesProc) (GrGLsizei, GrGLuint*);
    typedef void (GR_GL_FUNCTION_TYPE* GLDeleteQueriesProc) (GrGLsizei, const GrGLuint*);
    typedef void (GR_GL_FUNCTION_TYPE* GLBeginQueryProc) (GrGLenum, GrGLuint);
    typedef void (GR_GL_FUNCTION_TYPE* GLEndQueryProc) (GrGLenum);
    typedef void (GR_GL_FUNCTION_TYPE* GLGetQueryObjectuivProc) (GrGLuint, GrGLenum, GrGLuint*);
    typedef void (GR_GL_FUNCTION_TYPE* GLGetQueryObjectui64vProc) (GrGLuint, GrGLenum, GrGLuint64*);

    GLGetIntegervProc           fGLGetIntegerv;
    GLGenQueriesProc            fGLGenQueries;
    GLDeleteQueriesProc         fGLDeleteQueries;
    GLBeginQueryProc            fGLBeginQuery;
    GLEndQueryProc              fGLEndQuery;
    GLGetQueryObjectuivProc     fGLGetQueryObjectuiv;
    GLGetQueryObjectui64vProc   fGLGetQueryObjectui64v;


    using INHERITED = sk_gpu_test::GpuTimer;
};

std::unique_ptr<GLGpuTimer> GLGpuTimer::MakeIfSupported(const sk_gpu_test::GLTestContext* ctx) {
#ifdef SK_GL
    std::unique_ptr<GLGpuTimer> ret;
    const GrGLInterface* gl = ctx->gl();
    if (gl->fExtensions.has("GL_EXT_disjoint_timer_query")) {
        ret.reset(new GLGpuTimer(true, ctx, "EXT"));
    } else if (kGL_GrGLStandard == gl->fStandard &&
               (GrGLGetVersion(gl) > GR_GL_VER(3,3) || gl->fExtensions.has("GL_ARB_timer_query"))) {
        ret.reset(new GLGpuTimer(false, ctx));
    } else if (gl->fExtensions.has("GL_EXT_timer_query")) {
        ret.reset(new GLGpuTimer(false, ctx, "EXT"));
    }
    if (ret && !ret->validate()) {
        ret = nullptr;
    }
    return ret;
#else
    return nullptr;
#endif
}

#ifdef SK_GL
GLGpuTimer::GLGpuTimer(bool disjointSupport, const sk_gpu_test::GLTestContext* ctx, const char* ext)
    : INHERITED(disjointSupport) {
    ctx->getGLProcAddress(&fGLGetIntegerv, "glGetIntegerv");
    ctx->getGLProcAddress(&fGLGenQueries, "glGenQueries", ext);
    ctx->getGLProcAddress(&fGLDeleteQueries, "glDeleteQueries", ext);
    ctx->getGLProcAddress(&fGLBeginQuery, "glBeginQuery", ext);
    ctx->getGLProcAddress(&fGLEndQuery, "glEndQuery", ext);
    ctx->getGLProcAddress(&fGLGetQueryObjectuiv, "glGetQueryObjectuiv", ext);
    ctx->getGLProcAddress(&fGLGetQueryObjectui64v, "glGetQueryObjectui64v", ext);
}

bool GLGpuTimer::validate() const {
    return fGLGetIntegerv && fGLGenQueries && fGLDeleteQueries && fGLBeginQuery && fGLEndQuery &&
           fGLGetQueryObjectuiv && fGLGetQueryObjectui64v;
}
#endif

sk_gpu_test::PlatformTimerQuery GLGpuTimer::onQueueTimerStart() const {
    GrGLuint queryID;
    fGLGenQueries(1, &queryID);
    if (!queryID) {
        return sk_gpu_test::kInvalidTimerQuery;
    }
    if (this->disjointSupport()) {
        // Clear the disjoint flag.
        GrGLint disjoint;
        fGLGetIntegerv(GL_GPU_DISJOINT, &disjoint);
    }
    fGLBeginQuery(GL_TIME_ELAPSED, queryID);
    return static_cast<sk_gpu_test::PlatformTimerQuery>(queryID);
}

void GLGpuTimer::onQueueTimerStop(sk_gpu_test::PlatformTimerQuery platformTimer) const {
    if (sk_gpu_test::kInvalidTimerQuery == platformTimer) {
        return;
    }
    fGLEndQuery(GL_TIME_ELAPSED);
}

sk_gpu_test::GpuTimer::QueryStatus
GLGpuTimer::checkQueryStatus(sk_gpu_test::PlatformTimerQuery platformTimer) {
    const GrGLuint queryID = static_cast<GrGLuint>(platformTimer);
    if (!queryID) {
        return QueryStatus::kInvalid;
    }
    GrGLuint available = 0;
    fGLGetQueryObjectuiv(queryID, GL_QUERY_RESULT_AVAILABLE, &available);
    if (!available) {
        return QueryStatus::kPending;
    }
    if (this->disjointSupport()) {
        GrGLint disjoint = 1;
        fGLGetIntegerv(GL_GPU_DISJOINT, &disjoint);
        if (disjoint) {
            return QueryStatus::kDisjoint;
        }
    }
    return QueryStatus::kAccurate;
}

std::chrono::nanoseconds GLGpuTimer::getTimeElapsed(sk_gpu_test::PlatformTimerQuery platformTimer) {
    SkASSERT(this->checkQueryStatus(platformTimer) >= QueryStatus::kDisjoint);
    const GrGLuint queryID = static_cast<GrGLuint>(platformTimer);
    GrGLuint64 nanoseconds;
    fGLGetQueryObjectui64v(queryID, GL_QUERY_RESULT, &nanoseconds);
    return std::chrono::nanoseconds(nanoseconds);
}

void GLGpuTimer::deleteQuery(sk_gpu_test::PlatformTimerQuery platformTimer) {
    const GrGLuint queryID = static_cast<GrGLuint>(platformTimer);
    fGLDeleteQueries(1, &queryID);
}

static_assert(sizeof(GrGLuint) <= sizeof(sk_gpu_test::PlatformTimerQuery));

}  // anonymous namespace

namespace sk_gpu_test {

GLTestContext::GLTestContext() : TestContext() {}

GLTestContext::~GLTestContext() {
    SkASSERT(!fGLInterface);
    SkASSERT(!fOriginalGLInterface);
}

bool GLTestContext::isValid() const {
#ifdef SK_GL
    return SkToBool(this->gl());
#else
    return fWasInitialized;
#endif
}

static bool fence_is_supported(const GLTestContext* ctx) {
#ifdef SK_GL
    if (kGL_GrGLStandard == ctx->gl()->fStandard) {
        if (GrGLGetVersion(ctx->gl()) < GR_GL_VER(3, 2) &&
            !ctx->gl()->hasExtension("GL_ARB_sync")) {
            return false;
        }
        return true;
    } else {
        if (ctx->gl()->hasExtension("GL_APPLE_sync")) {
            return true;
        } else if (ctx->gl()->hasExtension("GL_NV_fence")) {
            return true;
        } else if (GrGLGetVersion(ctx->gl()) >= GR_GL_VER(3, 0)) {
            return true;
        } else {
            return false;
        }
    }
#else
    return false;
#endif
}

void GLTestContext::init(sk_sp<const GrGLInterface> gl) {
    fGLInterface = std::move(gl);
    fOriginalGLInterface = fGLInterface;
    fFenceSupport = fence_is_supported(this);
    fGpuTimer = GLGpuTimer::MakeIfSupported(this);
#ifndef SK_GL
    fWasInitialized = true;
#endif
}

void GLTestContext::teardown() {
    fGLInterface.reset();
    fOriginalGLInterface.reset();
    INHERITED::teardown();
}

void GLTestContext::testAbandon() {
    INHERITED::testAbandon();
#if defined(SK_GL) && defined(GR_TEST_UTILS)
    if (fGLInterface) {
        fGLInterface->abandon();
        fOriginalGLInterface->abandon();
    }
#endif
}

void GLTestContext::finish() {
#ifdef SK_GL
    if (fGLInterface) {
        GR_GL_CALL(fGLInterface.get(), Finish());
    }
#endif
}

void GLTestContext::overrideVersion(const char* version, const char* shadingLanguageVersion) {
#ifdef SK_GL
    // GrGLFunction has both a limited capture size and doesn't call a destructor when it is
    // initialized with a lambda. So here we're trusting fOriginalGLInterface will be kept alive.
    auto getString = [wrapped = &fOriginalGLInterface->fFunctions.fGetString,
                      version,
                      shadingLanguageVersion](GrGLenum name) {
        if (name == GR_GL_VERSION) {
            return reinterpret_cast<const GrGLubyte*>(version);
        } else if (name == GR_GL_SHADING_LANGUAGE_VERSION) {
            return reinterpret_cast<const GrGLubyte*>(shadingLanguageVersion);
        }
        return (*wrapped)(name);
    };
    auto newInterface = sk_make_sp<GrGLInterface>(*fOriginalGLInterface);
    newInterface->fFunctions.fGetString = getString;
    fGLInterface = std::move(newInterface);
#endif
}

sk_sp<GrDirectContext> GLTestContext::makeContext(const GrContextOptions& options) {
#ifdef SK_GL
    return GrDirectContext::MakeGL(fGLInterface, options);
#else
    return nullptr;
#endif
}

}  // namespace sk_gpu_test
