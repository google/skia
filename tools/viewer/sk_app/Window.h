/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef Window_DEFINED
#define Window_DEFINED

#include "DisplayParams.h"
#include "SkRect.h"
#include "SkTouchGesture.h"
#include "SkTypes.h"
#include "SkJSONCPP.h"

class GrContext;
class SkCanvas;
class SkSurface;

namespace sk_app {

class WindowContext;

class Window {
public:
    static Window* CreateNativeWindow(void* platformData);

    virtual ~Window() { this->detach(); }

    virtual void setTitle(const char*) = 0;
    virtual void show() = 0;
    virtual void setUIState(const Json::Value& state) {}  // do nothing in default

    // Shedules an invalidation event for window if one is not currently pending.
    // Make sure that either onPaint or markInvalReceived is called when the client window consumes
    // the the inval event. They unset fIsContentInvalided which allow future onInval.
    void inval();

    virtual bool scaleContentToFit() const { return false; }

    enum BackendType {
        kNativeGL_BackendType,
#ifdef SK_VULKAN
        kVulkan_BackendType,
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
    enum class Key {
        kNONE,    //corresponds to android's UNKNOWN

        kLeftSoftKey,
        kRightSoftKey,

        kHome,    //!< the home key - added to match android
        kBack,    //!< (CLR)
        kSend,    //!< the green (talk) key
        kEnd,     //!< the red key

        k0,
        k1,
        k2,
        k3,
        k4,
        k5,
        k6,
        k7,
        k8,
        k9,
        kStar,    //!< the * key
        kHash,    //!< the # key

        kUp,
        kDown,
        kLeft,
        kRight,

        // Keys needed by ImGui
        kTab,
        kPageUp,
        kPageDown,
        kDelete,
        kEscape,
        kShift,
        kCtrl,
        kOption, // AKA Alt
        kA,
        kC,
        kV,
        kX,
        kY,
        kZ,

        kOK,      //!< the center key

        kVolUp,   //!< volume up    - match android
        kVolDown, //!< volume down  - same
        kPower,   //!< power button - same
        kCamera,  //!< camera       - same

        kLast = kCamera
    };
    static const int kKeyCount = static_cast<int>(Key::kLast) + 1;

    enum ModifierKeys {
        kShift_ModifierKey = 1 << 0,
        kControl_ModifierKey = 1 << 1,
        kOption_ModifierKey = 1 << 2,   // same as ALT
        kCommand_ModifierKey = 1 << 3,
        kFirstPress_ModifierKey = 1 << 4,
    };

    enum InputState {
        kDown_InputState,
        kUp_InputState,
        kMove_InputState   // only valid for mouse
    };

    // return value of 'true' means 'I have handled this event'
    typedef void(*OnBackendCreatedFunc)(void* userData);
    typedef bool(*OnCharFunc)(SkUnichar c, uint32_t modifiers, void* userData);
    typedef bool(*OnKeyFunc)(Key key, InputState state, uint32_t modifiers, void* userData);
    typedef bool(*OnMouseFunc)(int x, int y, InputState state, uint32_t modifiers, void* userData);
    typedef bool(*OnMouseWheelFunc)(float delta, uint32_t modifiers, void* userData);
    typedef bool(*OnTouchFunc)(intptr_t owner, InputState state, float x, float y, void* userData);
    typedef void(*OnUIStateChangedFunc)(
            const SkString& stateName, const SkString& stateValue, void* userData);
    typedef void(*OnPaintFunc)(SkCanvas*, void* userData);

    void registerBackendCreatedFunc(OnBackendCreatedFunc func, void* userData) {
        fBackendCreatedFunc = func;
        fBackendCreatedUserData = userData;
    }

    void registerCharFunc(OnCharFunc func, void* userData) {
        fCharFunc = func;
        fCharUserData = userData;
    }

    void registerKeyFunc(OnKeyFunc func, void* userData) {
        fKeyFunc = func;
        fKeyUserData = userData;
    }

    void registerMouseFunc(OnMouseFunc func, void* userData) {
        fMouseFunc = func;
        fMouseUserData = userData;
    }

    void registerMouseWheelFunc(OnMouseWheelFunc func, void* userData) {
        fMouseWheelFunc = func;
        fMouseWheelUserData = userData;
    }

    void registerPaintFunc(OnPaintFunc func, void* userData) {
        fPaintFunc = func;
        fPaintUserData = userData;
    }

    void registerTouchFunc(OnTouchFunc func, void* userData) {
        fTouchFunc = func;
        fTouchUserData = userData;
    }

    void registerUIStateChangedFunc(OnUIStateChangedFunc func, void* userData) {
        fUIStateChangedFunc = func;
        fUIStateChangedUserData = userData;
    }

    void onBackendCreated();
    bool onChar(SkUnichar c, uint32_t modifiers);
    bool onKey(Key key, InputState state, uint32_t modifiers);
    bool onMouse(int x, int y, InputState state, uint32_t modifiers);
    bool onMouseWheel(float delta, uint32_t modifiers);
    bool onTouch(intptr_t owner, InputState state, float x, float y);  // multi-owner = multi-touch
    void onUIStateChanged(const SkString& stateName, const SkString& stateValue);
    void onPaint();
    void onResize(int width, int height);

    int width();
    int height();

    virtual const DisplayParams& getRequestedDisplayParams() { return fRequestedDisplayParams; }
    virtual void setRequestedDisplayParams(const DisplayParams&, bool allowReattach = true);

    // Actual parameters in effect, obtained from the native window.
    int sampleCount() const;
    int stencilBits() const;

    // Returns null if there is not a GPU backend or if the backend is not yet created.
    const GrContext* getGrContext() const;

protected:
    Window();

    OnBackendCreatedFunc   fBackendCreatedFunc;
    void*                  fBackendCreatedUserData;
    OnCharFunc             fCharFunc;
    void*                  fCharUserData;
    OnKeyFunc              fKeyFunc;
    void*                  fKeyUserData;
    OnMouseFunc            fMouseFunc;
    void*                  fMouseUserData;
    OnMouseWheelFunc       fMouseWheelFunc;
    void*                  fMouseWheelUserData;
    OnTouchFunc            fTouchFunc;
    void*                  fTouchUserData;
    OnUIStateChangedFunc   fUIStateChangedFunc;
    void*                  fUIStateChangedUserData;
    OnPaintFunc            fPaintFunc;
    void*                  fPaintUserData;
    DisplayParams          fRequestedDisplayParams;

    WindowContext* fWindowContext = nullptr;

    virtual void onInval() = 0;

    // Uncheck fIsContentInvalided to allow future inval/onInval.
    void markInvalProcessed();

    bool fIsContentInvalidated = false;  // use this to avoid duplicate invalidate events
};

}   // namespace sk_app
#endif
