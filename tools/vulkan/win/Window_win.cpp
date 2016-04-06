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

#include "VulkanTestContext_win.h"

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


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    Window_win* window = (Window_win*) GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        window->onPaint();
        EndPaint(hWnd, &ps);
        break;

    case WM_CLOSE:
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_ACTIVATE:
        // disable/enable rendering here, depending on wParam != WA_INACTIVE
        break;

    case WM_SIZE:
        window->onSize();
        break;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        {
            DWORD dwMask = (1 << 29);
            bool bAltDown = ((lParam & dwMask) != 0);
            UINT theChar = MapVirtualKey((UINT)wParam, 2);
            // Handle Extended ASCII only
            if (theChar < 256) {
                return window->onKeyboard(theChar, true, bAltDown);
            }
        }
        break;

    case WM_KEYUP:
    case WM_SYSKEYUP:
        {
            DWORD dwMask = (1 << 29);
            bool bAltDown = ((lParam & dwMask) != 0);
            UINT theChar = MapVirtualKey((UINT)wParam, 2);
            // Handle Extended ASCII only
            if (theChar < 256) {
                return window->onKeyboard(theChar, false, bAltDown);
            }
        }
        break;

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
        {
            bool bLeftDown = ((wParam & MK_LBUTTON) != 0);
            bool bRightDown = ((wParam & MK_RBUTTON) != 0);
            bool bMiddleDown = ((wParam & MK_MBUTTON) != 0);

            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            //if (!gIsFullscreen)
            //{
            //    RECT rc = { 0, 0, 640, 480 };
            //    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
            //    xPos -= rc.left;
            //    yPos -= rc.top;
            //}

            return window->onMouse(bLeftDown, bRightDown, bMiddleDown, false, false, 0, xPos, yPos);
        }
    break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

bool Window_win::onKeyboard(UINT nChar, bool bKeyDown, bool bAltDown) {
    return fKeyFunc(nChar, bKeyDown, fKeyUserData);
}

bool Window_win::onMouse(bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
                         bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
                         int xPos, int yPos) {
    return fMouseFunc(xPos, yPos, bLeftButtonDown, fMouseUserData);
}

void Window_win::setTitle(const char* title) {
    SetWindowTextA(fHWnd, title);
}

void Window_win::show() {
    ShowWindow(fHWnd, SW_SHOW);
}


bool Window_win::attach(BackEndTypes attachType, int msaaSampleCount, AttachmentInfo*) {
    if (kVulkan_BackendType != attachType) {
        return false;
    }

    ContextPlatformData_win platformData;
    platformData.fHInstance = fHInstance;
    platformData.fHWnd = fHWnd;

    fTestContext = VulkanTestContext::Create((void*)&platformData, msaaSampleCount);

    return (SkToBool(fTestContext));
}
