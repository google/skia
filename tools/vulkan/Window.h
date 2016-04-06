/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef Window_DEFINED
#define Window_DEFINED

class SkCanvas;
class VulkanTestContext;

class Window {
public:
    static Window* CreateNativeWindow(void* platformData);

    virtual ~Window() {};

    virtual void setTitle(const char*) = 0;
    virtual void show() = 0;

    struct AttachmentInfo {
        int fSampleCount;
        int fStencilBits;
    };

    enum BackEndTypes {
        kNativeGL_BackendType,
        kVulkan_BackendType
    };

    virtual bool attach(BackEndTypes attachType, int msaaSampleCount, AttachmentInfo*) = 0;
    void detach();

    // input handling
    typedef bool(*OnKeyFunc)(int key, bool down, void* userData);
    typedef bool(*OnMouseFunc)(int x, int y, bool down, void* userData);
    typedef void(*OnPaintFunc)(SkCanvas*, void* userData);

    void registerKeyFunc(OnKeyFunc func, void* userData) {
        fKeyFunc = func;
        fKeyUserData = userData;
    }

    void registerMouseFunc(OnMouseFunc func, void* userData) {
        fMouseFunc = func;
        fMouseUserData = userData;
    }

    void registerPaintFunc(OnPaintFunc func, void* userData) {
        fPaintFunc = func;
        fPaintUserData = userData;
    }

    void onPaint();
    void onSize();

protected:
    Window();

    OnKeyFunc    fKeyFunc;
    void*        fKeyUserData;
    OnMouseFunc  fMouseFunc;
    void*        fMouseUserData;
    OnPaintFunc  fPaintFunc;
    void*        fPaintUserData;

    VulkanTestContext* fTestContext;
};


#endif
