
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef RasterWindowContext_android_DEFINED
#define RasterWindowContext_android_DEFINED

#include <android/native_window_jni.h>

#include "../RasterWindowContext.h"

namespace sk_app {

class RasterWindowContext_android : public RasterWindowContext {
public:
    friend RasterWindowContext* RasterWindowContext::Create(
            void* platformData, const DisplayParams&);

    sk_sp<SkSurface> getBackbufferSurface() override;
    void swapBuffers() override;

    bool isValid() override { return SkToBool(fNativeWindow); }
    void resize(uint32_t w, uint32_t h) override;
    void setDisplayParams(const DisplayParams& params) override;

private:
    RasterWindowContext_android(void* platformData, const DisplayParams& params);
    void setBuffersGeometry();
    sk_sp<SkSurface> fBackbufferSurface = nullptr;
    ANativeWindow* fNativeWindow = nullptr;
    ANativeWindow_Buffer fBuffer;
    ARect fBounds;
};

}   // namespace sk_app

#endif
