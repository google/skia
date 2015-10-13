#include "gtest/gtest.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

typedef EGLAPI EGLDisplay EGLAPIENTRY EGLGetDisplay(EGLNativeDisplayType display_id);
typedef EGLAPI EGLBoolean EGLAPIENTRY EGLInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor);
typedef EGLAPI EGLContext EGLAPIENTRY EGLGetCurrentContext(void);
typedef EGLAPI EGLSurface EGLAPIENTRY EGLGetCurrentSurface(EGLint readdraw);
typedef EGLAPI EGLBoolean EGLAPIENTRY EGLTerminate(EGLDisplay dpy);

class EGLThreadTest : public testing::Test
{
  public:
    virtual void SetUp() {}
    virtual void TearDown() {}

    EGLGetDisplay *mGetDisplay;
    EGLInitialize *mInitialize;
    EGLGetCurrentContext *mGetCurrentContext;
    EGLGetCurrentSurface *mGetCurrentSurface;

    EGLDisplay mDisplay;

    HMODULE mEGL;
    HMODULE mGLESv2;

    static DWORD WINAPI ThreadingTestEntryPoint(LPVOID thisPointer);

  private:
    void ThreadingTest();
};

DWORD WINAPI EGLThreadTest::ThreadingTestEntryPoint(LPVOID lpParameter)
{
    EGLThreadTest *test = (EGLThreadTest *)lpParameter;
    test->ThreadingTest();
    return 0;
}

void EGLThreadTest::ThreadingTest()
{
    mEGL = LoadLibrary(TEXT("libEGL.dll"));
    mGLESv2 = LoadLibrary(TEXT("libGLESv2.dll"));

    EXPECT_TRUE(mEGL != NULL);
    EXPECT_TRUE(mGLESv2 != NULL);

    mGetDisplay = (EGLGetDisplay *)GetProcAddress(mEGL, "eglGetDisplay");
    mInitialize = (EGLInitialize *)GetProcAddress(mEGL, "eglInitialize");
    mGetCurrentContext = (EGLGetCurrentContext *)GetProcAddress(mEGL, "eglGetCurrentContext");
    mGetCurrentSurface = (EGLGetCurrentSurface *)GetProcAddress(mEGL, "eglGetCurrentSurface");

    EXPECT_TRUE(mGetDisplay != NULL);
    EXPECT_TRUE(mInitialize != NULL);
    EXPECT_TRUE(mGetCurrentContext != NULL);
    EXPECT_TRUE(mGetCurrentSurface != NULL);

    mDisplay = mGetDisplay(EGL_D3D11_ELSE_D3D9_DISPLAY_ANGLE);

    EXPECT_TRUE(mDisplay!= EGL_NO_DISPLAY);

    mInitialize(mDisplay, NULL, NULL);
    mGetCurrentContext();
}

TEST_F(EGLThreadTest, thread_init_crash)
{
    DWORD threadId;
    HANDLE threadHandle = CreateThread(NULL, 0, EGLThreadTest::ThreadingTestEntryPoint, this, 0, &threadId);
    EXPECT_TRUE(threadHandle != NULL);

    // wait for signal from thread
    DWORD waitResult = WaitForSingleObject(threadHandle, 1000);
    EXPECT_EQ(waitResult, WAIT_OBJECT_0);

    // crash, because the TLS value is NULL on main thread
    mGetCurrentSurface(EGL_DRAW);
    mGetCurrentContext();

    auto terminate = (EGLTerminate *)GetProcAddress(mEGL, "eglTerminate");
    terminate(mDisplay);
}
