
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
#include "SkWGL.h"
#include "SkWindow.h"
#include "SkCanvas.h"
#include "SkOSMenu.h"
#include "SkTime.h"
#include "SkUtils.h"

#include "SkGraphics.h"

#if SK_ANGLE
#include "gl/GrGLInterface.h"

#include "GLES2/gl2.h"
#endif

#define INVALIDATE_DELAY_MS 200

static SkOSWindow* gCurrOSWin;
static HWND gEventTarget;

#define WM_EVENT_CALLBACK (WM_USER+0)

void post_skwinevent()
{
    PostMessage(gEventTarget, WM_EVENT_CALLBACK, 0, 0);
}

SkOSWindow::SkOSWindow(void* hWnd) 
    : fHWND(hWnd) 
#if SK_ANGLE
    , fDisplay(EGL_NO_DISPLAY)
    , fContext(EGL_NO_CONTEXT)
    , fSurface(EGL_NO_SURFACE)
#endif
    , fHGLRC(NULL)
    , fD3D9Device(NULL)
    , fAttached(kNone_BackEndType) {
    gEventTarget = (HWND)hWnd;
}

SkOSWindow::~SkOSWindow() {
    if (NULL != fD3D9Device) {
        ((IDirect3DDevice9*)fD3D9Device)->Release();
    }
    if (NULL != fHGLRC) {
        wglDeleteContext((HGLRC)fHGLRC);
    }
#if SK_ANGLE
    if (EGL_NO_CONTEXT != fContext) {
        eglDestroyContext(fDisplay, fContext);
        fContext = EGL_NO_CONTEXT;
    }

    if (EGL_NO_SURFACE != fSurface) {
        eglDestroySurface(fDisplay, fSurface);
        fSurface = EGL_NO_SURFACE;
    }

    if (EGL_NO_DISPLAY != fDisplay) {
        eglTerminate(fDisplay);
        fDisplay = EGL_NO_DISPLAY;
    }
#endif
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

    if (kNone_BackEndType == fAttached)
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


#define USE_MSAA 0

HGLRC create_gl(HWND hwnd, int msaaSampleCount) {

    HDC dc = GetDC(hwnd);

    SkWGLExtensions extensions;
    if (!extensions.hasExtension(dc, "WGL_ARB_pixel_format")) {
        return NULL;
    }

    HDC prevDC = wglGetCurrentDC();
    HGLRC prevGLRC = wglGetCurrentContext();
    PIXELFORMATDESCRIPTOR pfd;

    int format = 0;

    static const GLint iAttrs[] = {
        SK_WGL_DRAW_TO_WINDOW, TRUE,
        SK_WGL_DOUBLE_BUFFER, TRUE,
        SK_WGL_ACCELERATION, SK_WGL_FULL_ACCELERATION,
        SK_WGL_SUPPORT_OPENGL, TRUE,
        SK_WGL_COLOR_BITS, 24,
        SK_WGL_ALPHA_BITS, 8,
        SK_WGL_STENCIL_BITS, 8,
        0, 0
    };

    GLfloat fAttrs[] = {0, 0};

    if (msaaSampleCount > 0 &&
        extensions.hasExtension(dc, "WGL_ARB_multisample")) {
        static const int kIAttrsCount = SK_ARRAY_COUNT(iAttrs);
        GLint msaaIAttrs[kIAttrsCount + 4];
        memcpy(msaaIAttrs, iAttrs, sizeof(GLint) * kIAttrsCount);
        SkASSERT(0 == msaaIAttrs[kIAttrsCount - 2] &&
                 0 == msaaIAttrs[kIAttrsCount - 1]);
        msaaIAttrs[kIAttrsCount - 2] = SK_WGL_SAMPLE_BUFFERS;
        msaaIAttrs[kIAttrsCount - 1] = TRUE;
        msaaIAttrs[kIAttrsCount + 0] = SK_WGL_SAMPLES;
        msaaIAttrs[kIAttrsCount + 1] = msaaSampleCount;
        msaaIAttrs[kIAttrsCount + 2] = 0;
        msaaIAttrs[kIAttrsCount + 3] = 0;
        GLuint num;
        int formats[64];
        extensions.choosePixelFormat(dc, msaaIAttrs, fAttrs, 64, formats, &num);
        num = min(num,64);
        for (GLuint i = 0; i < num; ++i) {
            DescribePixelFormat(dc, formats[i], sizeof(pfd), &pfd);
            if (SetPixelFormat(dc, formats[i], &pfd)) {
                format = formats[i];
                break;
            }
        }
    }

    if (0 == format) {
        GLuint num;
        extensions.choosePixelFormat(dc, iAttrs, fAttrs, 1, &format, &num);
        DescribePixelFormat(dc, format, sizeof(pfd), &pfd);
        BOOL set = SetPixelFormat(dc, format, &pfd);
        SkASSERT(TRUE == set);
    }
    
    HGLRC glrc = wglCreateContext(dc);
    SkASSERT(glrc);

    wglMakeCurrent(prevDC, prevGLRC);
    return glrc;
}

bool SkOSWindow::attachGL(int msaaSampleCount) {
    if (NULL == fHGLRC) {
        fHGLRC = create_gl((HWND)fHWND, msaaSampleCount);
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
        return true;
    }
    return false;
}

void SkOSWindow::detachGL() {
    wglMakeCurrent(GetDC((HWND)fHWND), 0);
    wglDeleteContext((HGLRC)fHGLRC);
    fHGLRC = NULL;
}

void SkOSWindow::presentGL() {
    glFlush();
    SwapBuffers(GetDC((HWND)fHWND));
}

#if SK_ANGLE
bool create_ANGLE(EGLNativeWindowType hWnd,
                  int msaaSampleCount,
                  angle::EGLDisplay* eglDisplay,
                  angle::EGLContext* eglContext,
                  angle::EGLSurface* eglSurface) {
    static EGLint contextAttribs[] = { 
        EGL_NONE, EGL_NONE 
    };
    static EGLint configAttribList[] = {
        EGL_RED_SIZE,       8,
        EGL_GREEN_SIZE,     8,
        EGL_BLUE_SIZE,      8,
        EGL_ALPHA_SIZE,     8,
        EGL_DEPTH_SIZE,     8,
        EGL_STENCIL_SIZE,   8,
        EGL_NONE
    };
    static EGLint surfaceAttribList[] = {
        EGL_NONE, EGL_NONE
    };

    EGLDisplay display = eglGetDisplay(GetDC(hWnd));
    if (display == EGL_NO_DISPLAY ) {
       return false;
    }

    // Initialize EGL
    EGLint majorVersion, minorVersion;
    if (!eglInitialize(display, &majorVersion, &minorVersion)) {
       return false;
    }

    EGLint numConfigs;
    if (!eglGetConfigs(display, NULL, 0, &numConfigs)) {
       return false;
    }

    // Choose config
    EGLConfig config;
    bool foundConfig = false;
    if (msaaSampleCount) {
        static int kConfigAttribListCnt = SK_ARRAY_COUNT(configAttribList);
        EGLint msaaConfigAttribList[kConfigAttribListCnt + 4];
        memcpy(msaaConfigAttribList,
               configAttribList,
               sizeof(configAttribList));
        SkASSERT(EGL_NONE == msaaConfigAttribList[kConfigAttribListCnt - 1]);
        msaaConfigAttribList[kConfigAttribListCnt - 1] = EGL_SAMPLE_BUFFERS;
        msaaConfigAttribList[kConfigAttribListCnt + 0] = 1;
        msaaConfigAttribList[kConfigAttribListCnt + 1] = EGL_SAMPLES;
        msaaConfigAttribList[kConfigAttribListCnt + 2] = msaaCount;
        msaaConfigAttribList[kConfigAttribListCnt + 3] = EGL_NONE;
        if (eglChooseConfig(display, configAttribList,
                                   &config, 1, &numConfigs)) {
            if (numConfigs > 0) {
                foundConfig = true;
            }
        }
    }
    if (!foundConfig) {
        if (!eglChooseConfig(display, configAttribList, 
                                    &config, 1, &numConfigs)) {
           return false;
        }
    }

    // Create a surface
    EGLSurface surface = eglCreateWindowSurface(display, config, 
                                                (EGLNativeWindowType)hWnd, 
                                                surfaceAttribList);
    if (surface == EGL_NO_SURFACE) {
       return false;
    }

    // Create a GL context
    EGLContext context = eglCreateContext(display, config, 
                                          EGL_NO_CONTEXT,
                                          contextAttribs );
    if (context == EGL_NO_CONTEXT ) {
       return false;
    }   
    
    // Make the context current
    if (!eglMakeCurrent(display, surface, surface, context)) {
       return false;
    }
    
    *eglDisplay = display;
    *eglContext = context;
    *eglSurface = surface;
    return true;
}

bool SkOSWindow::attachANGLE(int msaaSampleCount) {
    if (EGL_NO_DISPLAY == fDisplay) {
        bool bResult = create_ANGLE((HWND)fHWND, &fDisplay, &fContext, &fSurface);
        if (false == bResult) {
            return false;
        }
        const GrGLInterface* intf = GrGLCreateANGLEInterface();

        if (intf) {
            GR_GL_CALL(intf, ClearStencil(0));
            GR_GL_CALL(intf, ClearColor(0, 0, 0, 0));
            GR_GL_CALL(intf, StencilMask(0xffffffff));
            GR_GL_CALL(intf, Clear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
        }
    }
    if (eglMakeCurrent(fDisplay, fSurface, fSurface, fContext)) {
        const GrGLInterface* intf = GrGLCreateANGLEInterface();

        if (intf ) {
            GR_GL_CALL(intf, Viewport(0, 0, SkScalarRound(this->width()),
                                      SkScalarRound(this->height())));
        }
        return true;
    }
    return false;
}

void SkOSWindow::detachANGLE() {
    eglMakeCurrent(fDisplay, EGL_NO_SURFACE , EGL_NO_SURFACE , EGL_NO_CONTEXT);

    eglDestroyContext(fDisplay, fContext);
    fContext = EGL_NO_CONTEXT;

    eglDestroySurface(fDisplay, fSurface);
    fSurface = EGL_NO_SURFACE;

    eglTerminate(fDisplay);
    fDisplay = EGL_NO_DISPLAY;
}

void SkOSWindow::presentANGLE() {
    const GrGLInterface* intf = GrGLCreateANGLEInterface();

    if (intf) {
        GR_GL_CALL(intf, Flush());
    }

    eglSwapBuffers(fDisplay, fSurface);
}
#endif

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
        return true;
    }
    return false;
}

void SkOSWindow::detachD3D9() {
    if (NULL != fD3D9Device) {
        ((IDirect3DDevice9*)fD3D9Device)->EndScene();
    }
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

// return true on success
bool SkOSWindow::attach(SkBackEndTypes attachType, int msaaSampleCount) {

    // attach doubles as "windowResize" so we need to allo
    // already bound states to pass through again
    // TODO: split out the resize functionality
//    SkASSERT(kNone_BackEndType == fAttached);
    bool result = true;

    switch (attachType) {
    case kNone_BackEndType:
        // nothing to do
        break; 
    case kNativeGL_BackEndType:
        result = attachGL(msaaSampleCount);
        break;
#if SK_ANGLE
    case kANGLE_BackEndType:
        result = attachANGLE(msaaSampleCount);
        break;
#endif
    case kD3D9_BackEndType:
        result = attachD3D9();
        break;
    default:
        SkASSERT(false);
        result = false;
        break;
    }

    if (result) {
        fAttached = attachType;
    }

    return result;
}

void SkOSWindow::detach() {
    switch (fAttached) {
    case kNone_BackEndType:
        // nothing to do
        break; 
    case kNativeGL_BackEndType:
        detachGL();
        break;
#if SK_ANGLE
    case kANGLE_BackEndType:
        detachANGLE();
        break;
#endif
    case kD3D9_BackEndType:
        detachD3D9();
        break;
    default:
        SkASSERT(false);
        break;
    }
    fAttached = kNone_BackEndType;
}

void SkOSWindow::present() {
    switch (fAttached) {
    case kNone_BackEndType:
        // nothing to do
        return; 
    case kNativeGL_BackEndType:
        presentGL();
        break;
#if SK_ANGLE
    case kANGLE_BackEndType:
        presentANGLE();
        break;
#endif
    case kD3D9_BackEndType:
        presentD3D9();
        break;
    default:
        SkASSERT(false);
        break;
    }
}

#endif
