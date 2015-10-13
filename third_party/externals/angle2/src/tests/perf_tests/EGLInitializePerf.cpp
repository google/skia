//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLInitializePerfTest:
//   Performance test for device creation.
//

#include "ANGLEPerfTest.h"
#include "Timer.h"
#include "test_utils/angle_test_configs.h"
#include "test_utils/angle_test_instantiate.h"
#include "platform/Platform.h"

using namespace testing;

namespace
{

// Only applies to D3D11
class CapturePlatform : public angle::Platform
{
  public:
    CapturePlatform()
        : mTimer(CreateTimer()),
          mLoadDLLsMS(0),
          mCreateDeviceMS(0),
          mInitResourcesMS(0)
    {
        mTimer->start();
    }

    double currentTime() override;
    void histogramCustomCounts(
        const char *name, int sample, int min, int max, int bucketCount) override;

    size_t getLoadDLLsMS() const { return mLoadDLLsMS; }
    size_t getCreateDeviceMS() const { return mCreateDeviceMS; }
    size_t getInitResourcesMS() const { return mInitResourcesMS; }

  private:
    Timer *mTimer;
    size_t mLoadDLLsMS;
    size_t mCreateDeviceMS;
    size_t mInitResourcesMS;
};

double CapturePlatform::currentTime()
{
    return mTimer->getElapsedTime();
}

void CapturePlatform::histogramCustomCounts(
    const char *name, int sample, int /*min*/, int /*max*/, int /*bucketCount*/)
{
    // These must match the names of the histograms.
    if (strcmp(name, "GPU.ANGLE.Renderer11InitializeDLLsMS") == 0)
    {
        mLoadDLLsMS += static_cast<size_t>(sample);
    }
    // Note: not captured in debug, due to creating a debug device
    else if (strcmp(name, "GPU.ANGLE.D3D11CreateDeviceMS") == 0)
    {
        mCreateDeviceMS += static_cast<size_t>(sample);
    }
    else if (strcmp(name, "GPU.ANGLE.Renderer11InitializeDeviceMS") == 0)
    {
        mInitResourcesMS += static_cast<size_t>(sample);
    }
}

class EGLInitializePerfTest : public ANGLEPerfTest,
                              public WithParamInterface<angle::PlatformParameters>
{
  public:
    EGLInitializePerfTest();
    ~EGLInitializePerfTest();

    void step(float dt, double totalTime) override;
    void TearDown() override;

  private:
    OSWindow *mOSWindow;
    EGLDisplay mDisplay;
    CapturePlatform mCapturePlatform;
};

EGLInitializePerfTest::EGLInitializePerfTest()
    : ANGLEPerfTest("EGLInitialize", "_run"),
      mOSWindow(nullptr),
      mDisplay(EGL_NO_DISPLAY)
{
    auto platform = GetParam().eglParameters;

    std::vector<EGLint> displayAttributes;
    displayAttributes.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
    displayAttributes.push_back(platform.renderer);
    displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE);
    displayAttributes.push_back(platform.majorVersion);
    displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE);
    displayAttributes.push_back(platform.minorVersion);

    if (platform.renderer == EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE ||
        platform.renderer == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE);
        displayAttributes.push_back(platform.deviceType);
    }
    displayAttributes.push_back(EGL_NONE);

    mOSWindow = CreateOSWindow();
    mOSWindow->initialize("EGLInitialize Test", 64, 64);

    auto eglGetPlatformDisplayEXT =
        reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
    if (eglGetPlatformDisplayEXT == nullptr)
    {
        std::cerr << "Error getting platform display!" << std::endl;
        return;
    }

    mDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE,
                                        mOSWindow->getNativeDisplay(),
                                        &displayAttributes[0]);

    ANGLEPlatformInitialize(&mCapturePlatform);
}

EGLInitializePerfTest::~EGLInitializePerfTest()
{
    SafeDelete(mOSWindow);
}

void EGLInitializePerfTest::step(float dt, double totalTime)
{
    ASSERT_TRUE(mDisplay != EGL_NO_DISPLAY);

    EGLint majorVersion, minorVersion;
    ASSERT_TRUE(eglInitialize(mDisplay, &majorVersion, &minorVersion) == EGL_TRUE);
    ASSERT_TRUE(eglTerminate(mDisplay) == EGL_TRUE);

    if (mTimer->getElapsedTime() >= 5.0)
    {
        mRunning = false;
    }
}

void EGLInitializePerfTest::TearDown()
{
    ANGLEPerfTest::TearDown();
    printResult("LoadDLLs", normalizedTime(mCapturePlatform.getLoadDLLsMS()), "ms", true);
    printResult("D3D11CreateDevice", normalizedTime(mCapturePlatform.getCreateDeviceMS()), "ms", true);
    printResult("InitResources", normalizedTime(mCapturePlatform.getInitResourcesMS()), "ms", true);

    ANGLEPlatformShutdown();
}

TEST_P(EGLInitializePerfTest, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(EGLInitializePerfTest, angle::ES2_D3D11());

} // namespace
