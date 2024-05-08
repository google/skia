/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef Window_DEFINED
#define Window_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTDArray.h"
#include "tools/skui/InputState.h"
#include "tools/skui/Key.h"
#include "tools/skui/ModifierKey.h"
#include "tools/window/DisplayParams.h"

#include <functional>

class GrDirectContext;
class SkCanvas;
class SkSurface;
class SkSurfaceProps;
class SkString;

namespace skgpu::graphite {
class Context;
class Recorder;
}

using skwindow::DisplayParams;

namespace skwindow {
class WindowContext;
}

namespace sk_app {

class Window {
public:
    static Window* CreateNativeWindow(void* platformData);

    virtual ~Window();

    virtual void setTitle(const char*) = 0;
    virtual void show() = 0;

    // JSON-formatted UI state for Android. Do nothing by default
    virtual void setUIState(const char*) {}

    // Interface to the system clipboard. Only implemented on UNIX.
    virtual const char* getClipboardText() { return nullptr; }
    virtual void        setClipboardText(const char*) {}

    // Schedules an invalidation event for window if one is not currently pending.
    // Make sure that either onPaint or markInvalReceived is called when the client window consumes
    // the the inval event. They unset fIsContentInvalided which allow future onInval.
    void inval();

    virtual bool scaleContentToFit() const { return false; }

    enum BackendType {
#ifdef SK_GL
        kNativeGL_BackendType,
#endif
#if SK_ANGLE && (defined(SK_BUILD_FOR_WIN) || defined(SK_BUILD_FOR_MAC))
        kANGLE_BackendType,
#endif
#ifdef SK_DAWN
#if defined(SK_GRAPHITE)
        kGraphiteDawn_BackendType,
#endif
#endif
#ifdef SK_VULKAN
        kVulkan_BackendType,
#if defined(SK_GRAPHITE)
        kGraphiteVulkan_BackendType,
#endif
#endif
#ifdef SK_METAL
        kMetal_BackendType,
#if defined(SK_GRAPHITE)
        kGraphiteMetal_BackendType,
#endif
#endif
#ifdef SK_DIRECT3D
        kDirect3D_BackendType,
#endif
        kRaster_BackendType,

        kLast_BackendType = kRaster_BackendType
    };
    enum {
        kBackendTypeCount = kLast_BackendType + 1
    };

    virtual bool attach(BackendType) = 0;
    void detach();

    // input handling

    class Layer {
    public:
        Layer() : fActive(true) {}
        virtual ~Layer() = default;

        bool getActive() { return fActive; }
        void setActive(bool active) { fActive = active; }

        // return value of 'true' means 'I have handled this event'
        virtual void onBackendCreated() {}
        virtual void onAttach(Window* window) {}
        virtual bool onChar(SkUnichar c, skui::ModifierKey) { return false; }
        virtual bool onKey(skui::Key, skui::InputState, skui::ModifierKey) { return false; }
        virtual bool onMouse(int x, int y, skui::InputState, skui::ModifierKey) { return false; }
        virtual bool onMouseWheel(float delta, int x, int y, skui::ModifierKey) { return false; }
        virtual bool onTouch(intptr_t owner, skui::InputState, float x, float y) { return false; }
        // Platform-detected gesture events
        virtual bool onFling(skui::InputState state) { return false; }
        virtual bool onPinch(skui::InputState state, float scale, float x, float y) { return false; }
        virtual void onUIStateChanged(const SkString& stateName, const SkString& stateValue) {}
        virtual void onPrePaint() {}
        virtual void onPaint(SkSurface*) {}
        virtual void onResize(int width, int height) {}

    private:
        friend class Window;
        bool fActive;
    };

    void pushLayer(Layer* layer) {
        layer->onAttach(this);
        fLayers.push_back(layer);
    }

    void onBackendCreated();
    bool onChar(SkUnichar c, skui::ModifierKey modifiers);
    bool onKey(skui::Key key, skui::InputState state, skui::ModifierKey modifiers);
    bool onMouse(int x, int y, skui::InputState state, skui::ModifierKey modifiers);
    bool onMouseWheel(float delta, int x, int y, skui::ModifierKey modifiers);
    bool onTouch(intptr_t owner, skui::InputState state, float x, float y);  // multi-owner = multi-touch
    // Platform-detected gesture events
    bool onFling(skui::InputState state);
    bool onPinch(skui::InputState state, float scale, float x, float y);
    void onUIStateChanged(const SkString& stateName, const SkString& stateValue);
    void onPaint();
    void onResize(int width, int height);
    void onActivate(bool isActive);

    int width() const;
    int height() const;
    virtual float scaleFactor() const { return 1.0f; }

    virtual const DisplayParams& getRequestedDisplayParams() { return fRequestedDisplayParams; }
    virtual void setRequestedDisplayParams(const DisplayParams&, bool allowReattach = true);

    // Actual parameters in effect, obtained from the native window.
    int sampleCount() const;
    int stencilBits() const;

    // Returns null if there is not a GPU backend or if the backend is not yet created.
    GrDirectContext* directContext() const;
    skgpu::graphite::Context* graphiteContext() const;
    skgpu::graphite::Recorder* graphiteRecorder() const;

#if defined(SK_GRAPHITE)
    // Will snap a Recording and submit to the Context if using Graphite
    void snapRecordingAndSubmit();
#endif

protected:
    Window();

    SkTDArray<Layer*>      fLayers;
    DisplayParams          fRequestedDisplayParams;
    bool                   fIsActive = true;

    std::unique_ptr<skwindow::WindowContext> fWindowContext;

    virtual void onInval() = 0;

    // Uncheck fIsContentInvalided to allow future inval/onInval.
    void markInvalProcessed();

    bool fIsContentInvalidated = false;  // use this to avoid duplicate invalidate events

    void visitLayers(const std::function<void(Layer*)>& visitor);
    bool signalLayers(const std::function<bool(Layer*)>& visitor);
};

}   // namespace sk_app
#endif
