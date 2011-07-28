
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkTypes.h"

#if defined(SK_BUILD_FOR_WIN)

#include <GL/gl.h>
#include <d3d9.h>
#include <WindowsX.h>
#include "SkWindow.h"
#include "SkCanvas.h"
#include "SkOSMenu.h"
#include "SkTime.h"
#include "SkUtils.h"

#include "SkGraphics.h"

#define INVALIDATE_DELAY_MS 200

static SkOSWindow* gCurrOSWin;
static HWND gEventTarget;

#define WM_EVENT_CALLBACK (WM_USER+0)

void post_skwinevent()
{
    PostMessage(gEventTarget, WM_EVENT_CALLBACK, 0, 0);
}

SkOSWindow::SkOSWindow(void* hWnd) : fHWND(hWnd), 
                                     fHGLRC(NULL),
                                     fGLAttached(false),
                                     fD3D9Device(NULL),
                                     fD3D9Attached(FALSE) {
    gEventTarget = (HWND)hWnd;
}

SkOSWindow::~SkOSWindow() {
    if (NULL != fD3D9Device) {
        ((IDirect3DDevice9*)fD3D9Device)->Release();
    }
    if (NULL != fHGLRC) {
        wglDeleteContext((HGLRC)fHGLRC);
    }
}

static SkKey winToskKey(WPARAM vk) {
    static const struct {
        WPARAM    fVK;
        SkKey    fKey;
    } gPair[] = {
        { VK_BACK,    kBack_SkKey },
        { VK_CLEAR,    kBack_SkKey },
        { VK_RETURN, kOK_SkKey },
        { VK_UP,     kUp_SkKey },
        { VK_DOWN,     kDown_SkKey },
        { VK_LEFT,     kLeft_SkKey },
        { VK_RIGHT,     kRight_SkKey }
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(gPair); i++) {
        if (gPair[i].fVK == vk) {
            return gPair[i].fKey;
        }
    }
    return kNONE_SkKey;
}

bool SkOSWindow::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_KEYDOWN: {
            SkKey key = winToskKey(wParam);
            if (kNONE_SkKey != key) {
                this->handleKey(key);
                return true;
            }
        } break;
        case WM_KEYUP: {
            SkKey key = winToskKey(wParam);
            if (kNONE_SkKey != key) {
                this->handleKeyUp(key);
                return true;
            }
        } break;
        case WM_UNICHAR:
            this->handleChar(wParam);
            return true; 
        case WM_CHAR: {
            this->handleChar(SkUTF8_ToUnichar((char*)&wParam));
            return true;
        } break;
        case WM_SIZE:
            this->resize(lParam & 0xFFFF, lParam >> 16);
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            this->doPaint(hdc);
            EndPaint(hWnd, &ps);
            return true;
            } break;

        case WM_TIMER: {
            RECT* rect = (RECT*)wParam;
            InvalidateRect(hWnd, rect, FALSE);
            KillTimer(hWnd, (UINT_PTR)rect);
            delete rect;
            return true;
        } break;
    
        case WM_LBUTTONDOWN: 
            this->handleClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), Click::kDown_State);
            return true;
                    
        case WM_MOUSEMOVE:
            this->handleClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), Click::kMoved_State);
            return true;

        case WM_LBUTTONUP:
            this->handleClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), Click::kUp_State);
            return true;

        case WM_EVENT_CALLBACK:
            if (SkEvent::ProcessEvent()) {
                post_skwinevent();
            }
            return true;
    }
    return false;
}

void SkOSWindow::doPaint(void* ctx) {
    this->update(NULL);

    if (!fGLAttached && !fD3D9Attached)
    {
        HDC hdc = (HDC)ctx;
        const SkBitmap& bitmap = this->getBitmap();

        BITMAPINFO bmi;
        memset(&bmi, 0, sizeof(bmi));
        bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth       = bitmap.width();
        bmi.bmiHeader.biHeight      = -bitmap.height(); // top-down image 
        bmi.bmiHeader.biPlanes      = 1;
        bmi.bmiHeader.biBitCount    = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage   = 0;

        // 
        // Do the SetDIBitsToDevice. 
        // 
        // TODO(wjmaclean):
        //       Fix this call to handle SkBitmaps that have rowBytes != width,
        //       i.e. may have padding at the end of lines. The SkASSERT below
        //       may be ignored by builds, and the only obviously safe option
        //       seems to be to copy the bitmap to a temporary (contiguous)
        //       buffer before passing to SetDIBitsToDevice().
        SkASSERT(bitmap.width() * bitmap.bytesPerPixel() == bitmap.rowBytes());
        bitmap.lockPixels();
        int iRet = SetDIBitsToDevice(hdc,
            0, 0,
            bitmap.width(), bitmap.height(),
            0, 0,
            0, bitmap.height(),
            bitmap.getPixels(),
            &bmi,
            DIB_RGB_COLORS);
        bitmap.unlockPixels();
    }
}

#if 0
void SkOSWindow::updateSize()
{
    RECT    r;
    GetWindowRect((HWND)this->getHWND(), &r);
    this->resize(r.right - r.left, r.bottom - r.top);
}
#endif

void SkOSWindow::onHandleInval(const SkIRect& r) {
    RECT* rect = new RECT;
    rect->left    = r.fLeft;
    rect->top     = r.fTop;
    rect->right   = r.fRight;
    rect->bottom  = r.fBottom;
    SetTimer((HWND)fHWND, (UINT_PTR)rect, INVALIDATE_DELAY_MS, NULL);
}

void SkOSWindow::onAddMenu(const SkOSMenu* sk_menu)
{
}

void SkOSWindow::onSetTitle(const char title[]){
    SetWindowTextA((HWND)fHWND, title);
}

enum {
    SK_MacReturnKey     = 36,
    SK_MacDeleteKey     = 51,
    SK_MacEndKey        = 119,
    SK_MacLeftKey       = 123,
    SK_MacRightKey      = 124,
    SK_MacDownKey       = 125,
    SK_MacUpKey         = 126,
    
    SK_Mac0Key          = 0x52,
    SK_Mac1Key          = 0x53,
    SK_Mac2Key          = 0x54,
    SK_Mac3Key          = 0x55,
    SK_Mac4Key          = 0x56,
    SK_Mac5Key          = 0x57,
    SK_Mac6Key          = 0x58,
    SK_Mac7Key          = 0x59,
    SK_Mac8Key          = 0x5b,
    SK_Mac9Key          = 0x5c
};
    
static SkKey raw2key(uint32_t raw)
{
    static const struct {
        uint32_t  fRaw;
        SkKey   fKey;
    } gKeys[] = {
        { SK_MacUpKey,      kUp_SkKey       },
        { SK_MacDownKey,    kDown_SkKey     },
        { SK_MacLeftKey,    kLeft_SkKey     },
        { SK_MacRightKey,   kRight_SkKey    },
        { SK_MacReturnKey,  kOK_SkKey       },
        { SK_MacDeleteKey,  kBack_SkKey     },
        { SK_MacEndKey,     kEnd_SkKey      },
        { SK_Mac0Key,       k0_SkKey        },
        { SK_Mac1Key,       k1_SkKey        },
        { SK_Mac2Key,       k2_SkKey        },
        { SK_Mac3Key,       k3_SkKey        },
        { SK_Mac4Key,       k4_SkKey        },
        { SK_Mac5Key,       k5_SkKey        },
        { SK_Mac6Key,       k6_SkKey        },
        { SK_Mac7Key,       k7_SkKey        },
        { SK_Mac8Key,       k8_SkKey        },
        { SK_Mac9Key,       k9_SkKey        }
    };
    
    for (unsigned i = 0; i < SK_ARRAY_COUNT(gKeys); i++)
        if (gKeys[i].fRaw == raw)
            return gKeys[i].fKey;
    return kNONE_SkKey;
}

///////////////////////////////////////////////////////////////////////////////////////

void SkEvent::SignalNonEmptyQueue()
{
    post_skwinevent();
    //SkDebugf("signal nonempty\n");
}

static UINT_PTR gTimer;

VOID CALLBACK sk_timer_proc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    SkEvent::ServiceQueueTimer();
    //SkDebugf("timer task fired\n");
}

void SkEvent::SignalQueueTimer(SkMSec delay)
{
    if (gTimer)
    {
        KillTimer(NULL, gTimer);
        gTimer = NULL;
    }
    if (delay)
    {     
        gTimer = SetTimer(NULL, 0, delay, sk_timer_proc);
        //SkDebugf("SetTimer of %d returned %d\n", delay, gTimer);
    }
}

#if defined(UNICODE)
    #define STR_LIT(X) L## #X
#else
    #define STR_LIT(X) #X
#endif

static HWND create_dummy()
{
    HMODULE module = GetModuleHandle(NULL);
    HWND dummy;
    RECT windowRect;
    windowRect.left = 0;
    windowRect.right = 8;
    windowRect.top = 0;
    windowRect.bottom = 8;

    WNDCLASS wc;

    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC) DefWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = module;
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = STR_LIT("DummyClass");

    if(!RegisterClass(&wc))
    {
        return 0;
    }

    DWORD style, exStyle;
    exStyle = WS_EX_CLIENTEDGE;
    style = WS_SYSMENU;

    AdjustWindowRectEx(&windowRect, style, false, exStyle);
    if(!(dummy = CreateWindowEx(exStyle,
        STR_LIT("DummyClass"),
        STR_LIT("DummyWindow"),
        WS_CLIPSIBLINGS | WS_CLIPCHILDREN | style,
        0, 0,
        windowRect.right-windowRect.left,
        windowRect.bottom-windowRect.top,
        NULL, NULL,
        module,
        NULL)))
    {
        UnregisterClass(STR_LIT("DummyClass"), module);
        return NULL;
    }
    ShowWindow(dummy, SW_HIDE);

    return dummy;
}

void kill_dummy(HWND dummy) {
    DestroyWindow(dummy);
    HMODULE module = GetModuleHandle(NULL);
    UnregisterClass(STR_LIT("DummyClass"), module);
}

// WGL_ARB_pixel_format
#define WGL_DRAW_TO_WINDOW_ARB      0x2001
#define WGL_ACCELERATION_ARB        0x2003
#define WGL_SUPPORT_OPENGL_ARB      0x2010
#define WGL_DOUBLE_BUFFER_ARB       0x2011
#define WGL_COLOR_BITS_ARB          0x2014
#define WGL_ALPHA_BITS_ARB          0x201B
#define WGL_STENCIL_BITS_ARB        0x2023
#define WGL_FULL_ACCELERATION_ARB   0x2027

// WGL_ARB_multisample
#define WGL_SAMPLE_BUFFERS_ARB      0x2041
#define WGL_SAMPLES_ARB             0x2042

#define USE_MSAA 0

HGLRC create_gl(HWND hwnd) {
    HDC hdc;    
    HDC prevHDC;
    HGLRC prevGLRC, glrc;
    PIXELFORMATDESCRIPTOR pfd;

    prevGLRC = wglGetCurrentContext();
    prevHDC  = wglGetCurrentDC();

    int format = 0;

    typedef BOOL (WINAPI *WGLChoosePixelFormatFunc)(HDC hdc,
                                                    const int *,
                                                    const FLOAT *,
                                                    UINT nMaxFormats,
                                                    int *,
                                                    UINT *);

    static WGLChoosePixelFormatFunc wglChoosePixelFormatARB;

    // This is horrible but true: wglGetProcAddress cannot be called before a
    // context is created. SetPixelFormat also needs to be called before the
    // context is created. But SetPixelFormat is only allowed to succeed once per
    // window. So we need to create a dummy window in order to call
    // wglGetProcAddress to get wglChoosePixelFormatARB.
    if (NULL == wglChoosePixelFormatARB) {
        ZeroMemory(&pfd, sizeof(pfd));
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits  = 32;
        pfd.cDepthBits  = 0;
        pfd.cStencilBits = 8;
        pfd.iLayerType = PFD_MAIN_PLANE;
        HWND dummy = create_dummy();
        SkASSERT(NULL != dummy);
        hdc = GetDC(dummy);
        format = ChoosePixelFormat(hdc, &pfd);
        SetPixelFormat(hdc, format, &pfd);
        glrc = wglCreateContext(hdc);
        SkASSERT(glrc);
        wglMakeCurrent(hdc, glrc);

        wglChoosePixelFormatARB = (WGLChoosePixelFormatFunc) wglGetProcAddress("wglChoosePixelFormatARB");

        wglMakeCurrent(hdc, NULL);
        wglDeleteContext(glrc);
        glrc = 0;
        kill_dummy(dummy);

        if (NULL == wglChoosePixelFormatARB) {
            return 0;
        }
    }

    hdc = GetDC(hwnd);
    format = 0;

    GLint iattrs[] = {
        WGL_DRAW_TO_WINDOW_ARB, TRUE,
        WGL_DOUBLE_BUFFER_ARB, TRUE,
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
        WGL_SUPPORT_OPENGL_ARB, TRUE,
        WGL_COLOR_BITS_ARB, 24,
        WGL_ALPHA_BITS_ARB, 8,
        WGL_STENCIL_BITS_ARB, 8,
#if USE_MSAA
        WGL_SAMPLE_BUFFERS_ARB, TRUE,
        WGL_SAMPLES_ARB, 0,
#else
        0, 0, 0, 0,
#endif
        0,0
    };
    for (int samples = 16; samples > 1; --samples) {
        iattrs[15] = samples;
        GLfloat fattrs[] = {0,0};
        GLuint num;
        int formats[64];
        wglChoosePixelFormatARB(hdc, iattrs, fattrs, 64, formats, &num);
        num = min(num,64);
        for (GLuint i = 0; i < num; ++i) {            
            DescribePixelFormat(hdc, formats[i], sizeof(pfd), &pfd);
            if (SetPixelFormat(hdc, formats[i], &pfd)) {
                format = formats[i];
                break;
            }
        }
    }
    if (0 == format) {
        iattrs[12] = iattrs[13] = 0;
        GLfloat fattrs[] = {0,0};
        GLuint num;
        wglChoosePixelFormatARB(hdc, iattrs, fattrs, 1, &format, &num);
        DescribePixelFormat(hdc, format, sizeof(pfd), &pfd);
        BOOL set = SetPixelFormat(hdc, format, &pfd);
        SkASSERT(TRUE == set);
    }
    
    glrc = wglCreateContext(hdc);
    SkASSERT(glrc);    

    wglMakeCurrent(prevHDC, prevGLRC);
    return glrc;
}

bool SkOSWindow::attachGL() {
    if (NULL == fHGLRC) {
        fHGLRC = create_gl((HWND)fHWND);
        if (NULL == fHGLRC) {
            return false;
        }
        glClearStencil(0);
        glClearColor(0, 0, 0, 0);
        glStencilMask(0xffffffff);
        glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    }
    if (wglMakeCurrent(GetDC((HWND)fHWND), (HGLRC)fHGLRC)) {
        glViewport(0, 0, SkScalarRound(this->width()),
                   SkScalarRound(this->height()));
        fGLAttached = true;
        return true;
    }
    return false;
}

void SkOSWindow::detachGL() {
    wglMakeCurrent(GetDC((HWND)fHWND), 0);
    fGLAttached = false;
}

void SkOSWindow::presentGL() {
    glFlush();
    SwapBuffers(GetDC((HWND)fHWND));
}

IDirect3DDevice9* create_d3d9_device(HWND hwnd) {
    HRESULT hr;

    IDirect3D9* d3d9;
    d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
    if (NULL == d3d9) {
        return NULL;
    }
    D3DDEVTYPE devType = D3DDEVTYPE_HAL;
    //D3DDEVTYPE devType = D3DDEVTYPE_REF;
    DWORD qLevels;
    DWORD qLevelsDepth;
    D3DMULTISAMPLE_TYPE type;
    for (type = D3DMULTISAMPLE_16_SAMPLES; 
         type >= D3DMULTISAMPLE_NONMASKABLE; --(*(DWORD*)&type)) {
        hr = d3d9->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, 
                                              devType, D3DFMT_D24S8, TRUE,
                                              type, &qLevels);
        qLevels = (hr == D3D_OK) ? qLevels : 0;
        hr = d3d9->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, 
                                              devType, D3DFMT_A8R8G8B8, TRUE,
                                              type, &qLevelsDepth);
        qLevelsDepth = (hr == D3D_OK) ? qLevelsDepth : 0;
        qLevels = min(qLevels,qLevelsDepth);
        if (qLevels > 0) {
            break;
        }
    }
    qLevels = 0;
    IDirect3DDevice9* d3d9Device;
    D3DPRESENT_PARAMETERS pres;
    memset(&pres, 0, sizeof(pres));
    pres.EnableAutoDepthStencil = TRUE;
    pres.AutoDepthStencilFormat = D3DFMT_D24S8;
    pres.BackBufferCount = 2;
    pres.BackBufferFormat = D3DFMT_A8R8G8B8;
    pres.BackBufferHeight = 0;
    pres.BackBufferWidth = 0;
    if (qLevels > 0) {
        pres.MultiSampleType = type;
        pres.MultiSampleQuality = qLevels-1;
    } else {
        pres.MultiSampleType = D3DMULTISAMPLE_NONE;
        pres.MultiSampleQuality = 0;
    }
    pres.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pres.Windowed = TRUE;
    pres.hDeviceWindow = hwnd;
    pres.PresentationInterval = 1;
    pres.Flags = 0;
    hr = d3d9->CreateDevice(D3DADAPTER_DEFAULT,
                            devType,
                            hwnd, 
                            D3DCREATE_HARDWARE_VERTEXPROCESSING, 
                            &pres, 
                            &d3d9Device);    
    D3DERR_INVALIDCALL;
    if (SUCCEEDED(hr)) {
        d3d9Device->Clear(0, NULL, D3DCLEAR_TARGET, 0xFFFFFFFF, 0, 0);
        return d3d9Device;
    }
    return NULL;
}

// This needs some improvement. D3D doesn't have the same notion of attach/detach
// as GL. However, just allowing GDI to write to the window after creating the 
// D3D device seems to work. 
// We need to handle resizing. On XP and earlier Reset() will trash all our textures
// so we would need to inform the SkGpu/caches or just recreate them. On Vista+ we
// could use an IDirect3DDevice9Ex and call ResetEx() to resize without trashing
// everything. Currently we do nothing and the D3D9 image gets stretched/compressed
// when resized.

bool SkOSWindow::attachD3D9() {
    if (NULL == fD3D9Device) {
        fD3D9Device = (void*) create_d3d9_device((HWND)fHWND);
    }
    if (NULL != fD3D9Device) {
        ((IDirect3DDevice9*)fD3D9Device)->BeginScene();
        fD3D9Attached = true;
    }
    return fD3D9Attached;
}

void SkOSWindow::detachD3D9() {
    if (NULL != fD3D9Device) {
        ((IDirect3DDevice9*)fD3D9Device)->EndScene();
    }
    fD3D9Attached = false;
}

void SkOSWindow::presentD3D9() {
    if (NULL != fD3D9Device) {
        HRESULT hr;
        hr = ((IDirect3DDevice9*)fD3D9Device)->EndScene();
        SkASSERT(SUCCEEDED(hr));
        hr = ((IDirect3DDevice9*)d3d9Device())->Present(NULL, NULL, NULL, NULL);
        SkASSERT(SUCCEEDED(hr));
        hr = ((IDirect3DDevice9*)fD3D9Device)->Clear(0,NULL,D3DCLEAR_TARGET | 
                                                     D3DCLEAR_STENCIL, 0x0, 0, 
                                                     0);
        SkASSERT(SUCCEEDED(hr));
        hr = ((IDirect3DDevice9*)fD3D9Device)->BeginScene();
        SkASSERT(SUCCEEDED(hr));
    }
}


#endif
