/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef Window_DEFINED
#define Window_DEFINED

#include "SkTypes.h"
#include "SkRect.h"

class SkCanvas;
class VulkanTestContext;

class Window {
public:
    static Window* CreateNativeWindow(void* platformData);

    virtual ~Window() {};

    virtual void setTitle(const char*) = 0;
    virtual void show() = 0;
    virtual void inval() = 0;

    virtual bool scaleContentToFit() const { return false; }
    virtual bool supportsContentRect() const { return false; }
    virtual SkRect getContentRect() { return SkRect::MakeEmpty(); }

    enum BackEndType {
        kNativeGL_BackendType,
        kVulkan_BackendType
    };

    virtual bool attach(BackEndType attachType, int msaaSampleCount) = 0;
    void detach();

    // input handling
    enum Key {
        kNONE_Key,    //corresponds to android's UNKNOWN

        kLeftSoftKey_Key,
        kRightSoftKey_Key,

        kHome_Key,    //!< the home key - added to match android
        kBack_Key,    //!< (CLR)
        kSend_Key,    //!< the green (talk) key
        kEnd_Key,     //!< the red key

        k0_Key,
        k1_Key,
        k2_Key,
        k3_Key,
        k4_Key,
        k5_Key,
        k6_Key,
        k7_Key,
        k8_Key,
        k9_Key,
        kStar_Key,    //!< the * key
        kHash_Key,    //!< the # key

        kUp_Key,
        kDown_Key,
        kLeft_Key,
        kRight_Key,

        kOK_Key,      //!< the center key

        kVolUp_Key,   //!< volume up    - match android
        kVolDown_Key, //!< volume down  - same
        kPower_Key,   //!< power button - same
        kCamera_Key,  //!< camera       - same

        kLast_Key = kCamera_Key
    };
    static const int kKeyCount = kLast_Key + 1;

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
    typedef bool(*OnCharFunc)(SkUnichar c, uint32_t modifiers, void* userData);
    typedef bool(*OnKeyFunc)(Key key, InputState state, uint32_t modifiers, void* userData);
    typedef bool(*OnMouseFunc)(int x, int y, InputState state, uint32_t modifiers, void* userData);
    typedef void(*OnPaintFunc)(SkCanvas*, void* userData);

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

    void registerPaintFunc(OnPaintFunc func, void* userData) {
        fPaintFunc = func;
        fPaintUserData = userData;
    }

    bool onChar(SkUnichar c, uint32_t modifiers);
    bool onKey(Key key, InputState state, uint32_t modifiers);
    bool onMouse(int x, int y, InputState state, uint32_t modifiers);
    void onPaint();
    void onResize(uint32_t width, uint32_t height);

    uint32_t width() { return fWidth; }
    uint32_t height() { return fHeight;  }

protected:
    Window();

    uint32_t     fWidth;
    uint32_t     fHeight;

    OnCharFunc   fCharFunc;
    void*        fCharUserData;
    OnKeyFunc    fKeyFunc;
    void*        fKeyUserData;
    OnMouseFunc  fMouseFunc;
    void*        fMouseUserData;
    OnPaintFunc  fPaintFunc;
    void*        fPaintUserData;

    VulkanTestContext* fTestContext;
};


#endif
