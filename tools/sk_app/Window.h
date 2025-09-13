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

#include <functional>
#include <memory>

class GrDirectContext;
class SkCanvas;
class SkRecorder;
class SkString;
class SkSurface;
class SkSurfaceProps;

namespace skgpu::graphite {
class Context;
class Recorder;
}

namespace skwindow {
class DisplayParams;
class WindowContext;
}

namespace sk_app {

class Window {
public:
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
        kNativeGL_BackendType,
        kANGLE_BackendType,
        kGraphiteDawn_BackendType,
        kVulkan_BackendType,
        kGraphiteVulkan_BackendType,
        kMetal_BackendType,
        kGraphiteMetal_BackendType,
        kDirect3D_BackendType,
        kRaster_BackendType,
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

    const skwindow::DisplayParams* getRequestedDisplayParams() {
        return fRequestedDisplayParams.get();
    }
    virtual void setRequestedDisplayParams(std::unique_ptr<const skwindow::DisplayParams>,
                                           bool allowReattach = true);

    // Actual parameters in effect, obtained from the native window.
    int sampleCount() const;
    int stencilBits() const;

    // Returns null if there is not a GPU backend or if the backend is not yet created.
    GrDirectContext* directContext() const;
    skgpu::graphite::Context* graphiteContext() const;
    skgpu::graphite::Recorder* graphiteRecorder() const;
    SkRecorder* baseRecorder() const;

    using GpuTimerCallback = std::function<void(uint64_t ns)>;

    // Does the backend of this window support GPU timers (for use with submitToGpu)?
    bool supportsGpuTimer() const;

    // Will snap a Recording and submit to the Context if using Graphite or flush and submit
    // if using Ganesh.
    void submitToGpu(GpuTimerCallback = {});

protected:
    Window();

    SkTDArray<Layer*> fLayers;
    std::unique_ptr<const skwindow::DisplayParams> fRequestedDisplayParams;
    bool fIsActive = true;

    std::unique_ptr<skwindow::WindowContext> fWindowContext;

    virtual void onInval() = 0;

    // Uncheck fIsContentInvalided to allow future inval/onInval.
    void markInvalProcessed();

    bool fIsContentInvalidated = false;  // use this to avoid duplicate invalidate events

    void visitLayers(const std::function<void(Layer*)>& visitor);
    bool signalLayers(const std::function<bool(Layer*)>& visitor);
};

namespace Windows {
Window* CreateNativeWindow(void* platformData);
}

}   // namespace sk_app
#endif
