/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "Window_win.h"

#include <tchar.h>
#include <windows.h>
#include <windowsx.h>

#include "SkUtils.h"
#include "VulkanWindowContext_win.h"

namespace sk_app {

Window* Window::CreateNativeWindow(void* platformData) {
    HINSTANCE hInstance = (HINSTANCE)platformData;

    Window_win* window = new Window_win();
    if (!window->init(hInstance)) {
        delete window;
        return nullptr;
    }

    return window;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

bool Window_win::init(HINSTANCE hInstance) {
    fHInstance = hInstance ? hInstance : GetModuleHandle(nullptr);

    WNDCLASSEX wcex;
    // The main window class name
    static const TCHAR gSZWindowClass[] = _T("SkiaApp");

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = fHInstance;
    wcex.hIcon = LoadIcon(fHInstance, (LPCTSTR)IDI_WINLOGO);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);;
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = gSZWindowClass;
    wcex.hIconSm = LoadIcon(fHInstance, (LPCTSTR)IDI_WINLOGO);;

    if (!RegisterClassEx(&wcex)) {
        return false;
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
                         CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, fHInstance, nullptr);
    if (!fHWnd)
    {
        return false;
    }

    SetWindowLongPtr(fHWnd, GWLP_USERDATA, (LONG_PTR)this);

    return true;
}

static Window::Key get_key(WPARAM vk) {
    static const struct {
        WPARAM      fVK;
        Window::Key fKey;
    } gPair[] = {
        { VK_BACK, Window::kBack_Key },
        { VK_CLEAR, Window::kBack_Key },
        { VK_RETURN, Window::kOK_Key },
        { VK_UP, Window::kUp_Key },
        { VK_DOWN, Window::kDown_Key },
        { VK_LEFT, Window::kLeft_Key },
        { VK_RIGHT, Window::kRight_Key }
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(gPair); i++) {
        if (gPair[i].fVK == vk) {
            return gPair[i].fKey;
        }
    }
    return Window::kNONE_Key;
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
            if (wParam & MK_CONTROL) {
                modifiers |= Window::kControl_ModifierKey;
            }
            if (wParam & MK_SHIFT) {
                modifiers |= Window::kShift_ModifierKey;
            }
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
        case WM_DESTROY:
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

        case WM_MOUSEMOVE: 
            // only track if left button is down
            if ((wParam & MK_LBUTTON) != 0) {
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
            }
            break;

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


bool Window_win::attach(BackEndType attachType, int msaaSampleCount) {
    if (kVulkan_BackendType != attachType) {
        return false;
    }

    ContextPlatformData_win platformData;
    platformData.fHInstance = fHInstance;
    platformData.fHWnd = fHWnd;

    fWindowContext = VulkanWindowContext::Create((void*)&platformData, msaaSampleCount);

    return (SkToBool(fWindowContext));
}

void Window_win::inval() {
    InvalidateRect(fHWnd, nullptr, false);
}

}   // namespace sk_app
