/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/gpu/vk/GrVkVulkan.h"

#include "tools/sk_app/win/Window_win.h"

#include <tchar.h>
#include <windows.h>
#include <windowsx.h>

#include "src/core/SkUtils.h"
#include "tools/sk_app/WindowContext.h"
#include "tools/sk_app/win/WindowContextFactory_win.h"
#ifdef SK_VULKAN
#include "tools/sk_app/VulkanWindowContext.h"
#endif

namespace sk_app {

static int gWindowX = CW_USEDEFAULT;
static int gWindowY = 0;
static int gWindowWidth = CW_USEDEFAULT;
static int gWindowHeight = 0;

Window* Window::CreateNativeWindow(void* platformData) {
    HINSTANCE hInstance = (HINSTANCE)platformData;

    Window_win* window = new Window_win();
    if (!window->init(hInstance)) {
        delete window;
        return nullptr;
    }

    return window;
}

void Window_win::closeWindow() {
    RECT r;
    if (GetWindowRect(fHWnd, &r)) {
        gWindowX = r.left;
        gWindowY = r.top;
        gWindowWidth = r.right - r.left;
        gWindowHeight = r.bottom - r.top;
    }
    DestroyWindow(fHWnd);
}

Window_win::~Window_win() {
    this->closeWindow();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


bool Window_win::init(HINSTANCE hInstance) {
    fHInstance = hInstance ? hInstance : GetModuleHandle(nullptr);

    // The main window class name
    static const TCHAR gSZWindowClass[] = _T("SkiaApp");

    static WNDCLASSEX wcex;
    static bool wcexInit = false;
    if (!wcexInit) {
        wcex.cbSize = sizeof(WNDCLASSEX);

        wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = fHInstance;
        wcex.hIcon = LoadIcon(fHInstance, (LPCTSTR)IDI_WINLOGO);
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = gSZWindowClass;
        wcex.hIconSm = LoadIcon(fHInstance, (LPCTSTR)IDI_WINLOGO);

        if (!RegisterClassEx(&wcex)) {
            return false;
        }
        wcexInit = true;
    }

   /*
    if (fullscreen)
    {
        DEVMODE dmScreenSettings;
        // If full screen set the screen to maximum size of the users desktop and 32bit.
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = (unsigned long)width;
        dmScreenSettings.dmPelsHeight = (unsigned long)height;
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        // Change the display settings to full screen.
        ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

        // Set the position of the window to the top left corner.
        posX = posY = 0;
    }
    */
 //   gIsFullscreen = fullscreen;

    fHWnd = CreateWindow(gSZWindowClass, nullptr, WS_OVERLAPPEDWINDOW,
                         gWindowX, gWindowY, gWindowWidth, gWindowHeight,
                         nullptr, nullptr, fHInstance, nullptr);
    if (!fHWnd)
    {
        return false;
    }

    SetWindowLongPtr(fHWnd, GWLP_USERDATA, (LONG_PTR)this);
    RegisterTouchWindow(fHWnd, 0);

    return true;
}

static Window::Key get_key(WPARAM vk) {
    static const struct {
        WPARAM      fVK;
        Window::Key fKey;
    } gPair[] = {
        { VK_BACK, Window::Key::kBack },
        { VK_CLEAR, Window::Key::kBack },
        { VK_RETURN, Window::Key::kOK },
        { VK_UP, Window::Key::kUp },
        { VK_DOWN, Window::Key::kDown },
        { VK_LEFT, Window::Key::kLeft },
        { VK_RIGHT, Window::Key::kRight },
        { VK_TAB, Window::Key::kTab },
        { VK_PRIOR, Window::Key::kPageUp },
        { VK_NEXT, Window::Key::kPageDown },
        { VK_HOME, Window::Key::kHome },
        { VK_END, Window::Key::kEnd },
        { VK_DELETE, Window::Key::kDelete },
        { VK_ESCAPE, Window::Key::kEscape },
        { VK_SHIFT, Window::Key::kShift },
        { VK_CONTROL, Window::Key::kCtrl },
        { VK_MENU, Window::Key::kOption },
        { 'A', Window::Key::kA },
        { 'C', Window::Key::kC },
        { 'V', Window::Key::kV },
        { 'X', Window::Key::kX },
        { 'Y', Window::Key::kY },
        { 'Z', Window::Key::kZ },
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(gPair); i++) {
        if (gPair[i].fVK == vk) {
            return gPair[i].fKey;
        }
    }
    return Window::Key::kNONE;
}

static uint32_t get_modifiers(UINT message, WPARAM wParam, LPARAM lParam) {
    uint32_t modifiers = 0;

    switch (message) {
        case WM_UNICHAR:
        case WM_CHAR:
            if (0 == (lParam & (1 << 30))) {
                modifiers |= Window::kFirstPress_ModifierKey;
            }
            if (lParam & (1 << 29)) {
                modifiers |= Window::kOption_ModifierKey;
            }
            break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            if (0 == (lParam & (1 << 30))) {
                modifiers |= Window::kFirstPress_ModifierKey;
            }
            if (lParam & (1 << 29)) {
                modifiers |= Window::kOption_ModifierKey;
            }
            break;

        case WM_KEYUP:
        case WM_SYSKEYUP:
            if (lParam & (1 << 29)) {
                modifiers |= Window::kOption_ModifierKey;
            }
            break;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MOUSEMOVE:
        case WM_MOUSEWHEEL:
            if (wParam & MK_CONTROL) {
                modifiers |= Window::kControl_ModifierKey;
            }
            if (wParam & MK_SHIFT) {
                modifiers |= Window::kShift_ModifierKey;
            }
            break;
    }

    return modifiers;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    Window_win* window = (Window_win*) GetWindowLongPtr(hWnd, GWLP_USERDATA);

    bool eventHandled = false;

    switch (message) {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            window->onPaint();
            EndPaint(hWnd, &ps);
            eventHandled = true;
            break;

        case WM_CLOSE:
            PostQuitMessage(0);
            eventHandled = true;
            break;

        case WM_ACTIVATE:
            // disable/enable rendering here, depending on wParam != WA_INACTIVE
            break;

        case WM_SIZE:
            window->onResize(LOWORD(lParam), HIWORD(lParam));
            eventHandled = true;
            break;

        case WM_UNICHAR:
            eventHandled = window->onChar((SkUnichar)wParam,
                                          get_modifiers(message, wParam, lParam));
            break;

        case WM_CHAR: {
            const uint16_t* c = reinterpret_cast<uint16_t*>(&wParam);
            eventHandled = window->onChar(SkUTF16_NextUnichar(&c),
                                          get_modifiers(message, wParam, lParam));
        } break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            eventHandled = window->onKey(get_key(wParam), Window::kDown_InputState,
                                         get_modifiers(message, wParam, lParam));
            break;

        case WM_KEYUP:
        case WM_SYSKEYUP:
            eventHandled = window->onKey(get_key(wParam), Window::kUp_InputState,
                                         get_modifiers(message, wParam, lParam));
            break;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP: {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);

            //if (!gIsFullscreen)
            //{
            //    RECT rc = { 0, 0, 640, 480 };
            //    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
            //    xPos -= rc.left;
            //    yPos -= rc.top;
            //}

            Window::InputState istate = ((wParam & MK_LBUTTON) != 0) ? Window::kDown_InputState
                                                                     : Window::kUp_InputState;

            eventHandled = window->onMouse(xPos, yPos, istate,
                                            get_modifiers(message, wParam, lParam));
        } break;

        case WM_MOUSEMOVE: {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);

            //if (!gIsFullscreen)
            //{
            //    RECT rc = { 0, 0, 640, 480 };
            //    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
            //    xPos -= rc.left;
            //    yPos -= rc.top;
            //}

            eventHandled = window->onMouse(xPos, yPos, Window::kMove_InputState,
                                           get_modifiers(message, wParam, lParam));
        } break;

        case WM_MOUSEWHEEL:
            eventHandled = window->onMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f,
                                                get_modifiers(message, wParam, lParam));
            break;

        case WM_TOUCH: {
            uint16_t numInputs = LOWORD(wParam);
            std::unique_ptr<TOUCHINPUT[]> inputs(new TOUCHINPUT[numInputs]);
            if (GetTouchInputInfo((HTOUCHINPUT)lParam, numInputs, inputs.get(),
                                  sizeof(TOUCHINPUT))) {
                POINT topLeft = {0, 0};
                ClientToScreen(hWnd, &topLeft);
                for (uint16_t i = 0; i < numInputs; ++i) {
                    TOUCHINPUT ti = inputs[i];
                    Window::InputState state;
                    if (ti.dwFlags & TOUCHEVENTF_DOWN) {
                        state = Window::kDown_InputState;
                    } else if (ti.dwFlags & TOUCHEVENTF_MOVE) {
                        state = Window::kMove_InputState;
                    } else if (ti.dwFlags & TOUCHEVENTF_UP) {
                        state = Window::kUp_InputState;
                    } else {
                        continue;
                    }
                    // TOUCHINPUT coordinates are in 100ths of pixels
                    // Adjust for that, and make them window relative
                    LONG tx = (ti.x / 100) - topLeft.x;
                    LONG ty = (ti.y / 100) - topLeft.y;
                    eventHandled = window->onTouch(ti.dwID, state, tx, ty) || eventHandled;
                }
            }
        } break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return eventHandled ? 0 : 1;
}

void Window_win::setTitle(const char* title) {
    SetWindowTextA(fHWnd, title);
}

void Window_win::show() {
    ShowWindow(fHWnd, SW_SHOW);
}


bool Window_win::attach(BackendType attachType) {
    fBackend = attachType;

    switch (attachType) {
        case kNativeGL_BackendType:
            fWindowContext = window_context_factory::NewGLForWin(fHWnd, fRequestedDisplayParams);
            break;
#if SK_ANGLE
        case kANGLE_BackendType:
            fWindowContext = window_context_factory::NewANGLEForWin(fHWnd, fRequestedDisplayParams);
            break;
#endif
        case kRaster_BackendType:
            fWindowContext = window_context_factory::NewRasterForWin(fHWnd,
                                                                     fRequestedDisplayParams);
            break;
#ifdef SK_VULKAN
        case kVulkan_BackendType:
            fWindowContext = window_context_factory::NewVulkanForWin(fHWnd,
                                                                     fRequestedDisplayParams);
            break;
#endif
    }
    this->onBackendCreated();

    return (SkToBool(fWindowContext));
}

void Window_win::onInval() {
    InvalidateRect(fHWnd, nullptr, false);
}

void Window_win::setRequestedDisplayParams(const DisplayParams& params, bool allowReattach) {
    // GL on Windows doesn't let us change MSAA after the window is created
    if (params.fMSAASampleCount != this->getRequestedDisplayParams().fMSAASampleCount
            && allowReattach) {
        // Need to change these early, so attach() creates the window context correctly
        fRequestedDisplayParams = params;

        delete fWindowContext;
        this->closeWindow();
        this->init(fHInstance);
        this->attach(fBackend);
    }

    INHERITED::setRequestedDisplayParams(params, allowReattach);
}

}   // namespace sk_app
