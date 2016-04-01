
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "NullGLTestContext.h"
#include "gl/GrGLTestInterface.h"
#include "gl/GrGLDefines.h"
#include "gl/GrGLInterface.h"
#include "gl/GrGLTypes.h"
#include "SkMutex.h"
#include "SkTDArray.h"

namespace {
class NullGLContext : public sk_gpu_test::GLTestContext {
public:
    NullGLContext() { this->init(GrGLCreateNullInterface()); }
   ~NullGLContext() override { this->teardown(); }

private:    
    void onPlatformMakeCurrent() const override {};
    void onPlatformSwapBuffers() const override {}
    GrGLFuncPtr onPlatformGetProcAddress(const char*) const override { return nullptr; }
};

}  // anonymous namespace

namespace sk_gpu_test {
GLTestContext* CreateNullGLTestContext() {
    GLTestContext* ctx = new NullGLContext();
    if (ctx->isValid()) {
        return ctx;
    }
    delete ctx;
    return nullptr;
}
}  // namespace sk_gpu_test

