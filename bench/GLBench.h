/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GLBench_DEFINED
#define GLBench_DEFINED

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkImageEncoder.h"

#if SK_SUPPORT_GPU
#include "gl/GrGLFunctions.h"

class GrGLContext;
struct GrGLInterface;

/*
 * A virtual base class for microbenches which want to specifically test the performance of GL
 */

class GLBench : public Benchmark {
public:
    GLBench() {}

protected:
    const GrGLContext* getGLContext(SkCanvas*);
    virtual const GrGLContext* onGetGLContext(const GrGLContext* ctx) { return ctx; }
    void onPreDraw(SkCanvas*) override;
    virtual void setup(const GrGLContext*)=0;
    void onPostDraw(SkCanvas* canvas) override;
    virtual void teardown(const GrGLInterface*)=0;
    void onDraw(int loops, SkCanvas*) override;
    virtual void glDraw(int loops, const GrGLContext*)=0;
    static GrGLuint CompileShader(const GrGLContext*, const char* shaderSrc, GrGLenum type);
    static GrGLuint CreateProgram(const GrGLContext*, const char* vshader, const char* fshader);
    static GrGLuint SetupFramebuffer(const GrGLInterface*, int screenWidth, int screenHeight);
    static void DumpImage(const GrGLInterface* gl, uint32_t screenWidth, uint32_t screenHeight,
                          const char* filename);


private:
    typedef Benchmark INHERITED;
};


#endif
#endif
