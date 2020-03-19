/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/gl/GLTestContext.h"

#include "include/gpu/GrContext.h"
#include "src/gpu/gl/GrGLUtil.h"
#include "tools/gpu/GpuTimer.h"

namespace {

class GLGpuTimer : public sk_gpu_test::GpuTimer {
public:
    static std::unique_ptr<GLGpuTimer> MakeIfSupported(const sk_gpu_test::GLTestContext*);

    QueryStatus checkQueryStatus(sk_gpu_test::PlatformTimerQuery) override;
    std::chrono::nanoseconds getTimeElapsed(sk_gpu_test::PlatformTimerQuery) override;
    void deleteQuery(sk_gpu_test::PlatformTimerQuery) override;

private:
    GLGpuTimer(bool disjointSupport, const sk_gpu_test::GLTestContext*, const char* ext = "");

    bool validate() const;

    sk_gpu_test::PlatformTimerQuery onQueueTimerStart() const override;
    void onQueueTimerStop(sk_gpu_test::PlatformTimerQuery) const override;

    static constexpr GrGLenum GL_QUERY_RESULT            = 0x8866;
    static constexpr GrGLenum GL_QUERY_RESULT_AVAILABLE  = 0x8867;
    static constexpr GrGLenum GL_TIME_ELAPSED            = 0x88bf;
    static constexpr GrGLenum GL_GPU_DISJOINT            = 0x8fbb;

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


    typedef sk_gpu_test::GpuTimer INHERITED;
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
    SkASSERT(nullptr == fGL.get());
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
    fGL = std::move(gl);
    fFenceSupport = fence_is_supported(this);
    fGpuTimer = GLGpuTimer::MakeIfSupported(this);
#ifndef SK_GL
    fWasInitialized = true;
#endif
}

void GLTestContext::teardown() {
    fGL.reset(nullptr);
    INHERITED::teardown();
}

void GLTestContext::testAbandon() {
    INHERITED::testAbandon();
#ifdef SK_GL
    if (fGL) {
        fGL->abandon();
    }
#endif
}

void GLTestContext::finish() {
#ifdef SK_GL
    if (fGL) {
        GR_GL_CALL(fGL.get(), Finish());
    }
#endif
}

GrGLuint GLTestContext::createTextureRectangle(int width, int height, GrGLenum internalFormat,
                                               GrGLenum externalFormat, GrGLenum externalType,
                                               GrGLvoid* data) {
#ifdef SK_GL
    // Should match GrGLCaps check for fRectangleTextureSupport.
    if (kGL_GrGLStandard != fGL->fStandard ||
        (GrGLGetVersion(fGL.get()) < GR_GL_VER(3, 1) &&
         !fGL->fExtensions.has("GL_ARB_texture_rectangle") &&
         !fGL->fExtensions.has("GL_ANGLE_texture_rectangle"))) {
        return 0;
    }

    if  (GrGLGetGLSLVersion(fGL.get()) < GR_GLSL_VER(1, 40)) {
        return 0;
    }

    GrGLuint id;
    GR_GL_CALL(fGL.get(), GenTextures(1, &id));
    GR_GL_CALL(fGL.get(), BindTexture(GR_GL_TEXTURE_RECTANGLE, id));
    GR_GL_CALL(fGL.get(), TexParameteri(GR_GL_TEXTURE_RECTANGLE, GR_GL_TEXTURE_MAG_FILTER,
                                        GR_GL_NEAREST));
    GR_GL_CALL(fGL.get(), TexParameteri(GR_GL_TEXTURE_RECTANGLE, GR_GL_TEXTURE_MIN_FILTER,
                                        GR_GL_NEAREST));
    GR_GL_CALL(fGL.get(), TexParameteri(GR_GL_TEXTURE_RECTANGLE, GR_GL_TEXTURE_WRAP_S,
                                        GR_GL_CLAMP_TO_EDGE));
    GR_GL_CALL(fGL.get(), TexParameteri(GR_GL_TEXTURE_RECTANGLE, GR_GL_TEXTURE_WRAP_T,
                                        GR_GL_CLAMP_TO_EDGE));
    GR_GL_CALL(fGL.get(), TexImage2D(GR_GL_TEXTURE_RECTANGLE, 0, internalFormat, width, height, 0,
                                     externalFormat, externalType, data));
    return id;
#else
    return 0;
#endif
}

sk_sp<GrContext> GLTestContext::makeGrContext(const GrContextOptions& options) {
#ifdef SK_GL
    return GrContext::MakeGL(fGL, options);
#else
    return nullptr;
#endif
}

}  // namespace sk_gpu_test
