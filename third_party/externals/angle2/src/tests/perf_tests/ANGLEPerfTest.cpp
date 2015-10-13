//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "ANGLEPerfTest.h"

#include "third_party/perf/perf_test.h"

#include <iostream>
#include <cassert>

ANGLEPerfTest::ANGLEPerfTest(const std::string &name, const std::string &suffix)
    : mName(name),
      mSuffix(suffix),
      mRunning(false),
      mTimer(nullptr),
      mNumFrames(0)
{
    mTimer = CreateTimer();
}

ANGLEPerfTest::~ANGLEPerfTest()
{
    SafeDelete(mTimer);
}

void ANGLEPerfTest::run()
{
    mTimer->start();
    double prevTime = 0.0;

    while (mRunning)
    {
        double elapsedTime = mTimer->getElapsedTime();
        double deltaTime = elapsedTime - prevTime;

        ++mNumFrames;
        step(static_cast<float>(deltaTime), elapsedTime);

        if (!mRunning)
        {
            break;
        }

        prevTime = elapsedTime;
    }
}

void ANGLEPerfTest::printResult(const std::string &trace, double value, const std::string &units, bool important) const
{
    perf_test::PrintResult(mName, mSuffix, trace, value, units, important);
}

void ANGLEPerfTest::printResult(const std::string &trace, size_t value, const std::string &units, bool important) const
{
    perf_test::PrintResult(mName, mSuffix, trace, value, units, important);
}

void ANGLEPerfTest::SetUp()
{
    mRunning = true;
}

void ANGLEPerfTest::TearDown()
{
    printResult("score", static_cast<size_t>(mNumFrames), "score", true);
}

double ANGLEPerfTest::normalizedTime(size_t value) const
{
    return static_cast<double>(value) / static_cast<double>(mNumFrames);
}

std::string RenderTestParams::suffix() const
{
    switch (getRenderer())
    {
        case EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE: return "_d3d11";
        case EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE: return "_d3d9";
        case EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE: return "_gl";
        case EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE: return "_gles";
        case EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE: return "_default";
        default: assert(0); return "_unk";
    }
}

ANGLERenderTest::ANGLERenderTest(const std::string &name, const RenderTestParams &testParams)
    : ANGLEPerfTest(name, testParams.suffix()),
      mTestParams(testParams),
      mDrawIterations(10),
      mRunTimeSeconds(5.0),
      mEGLWindow(nullptr),
      mOSWindow(nullptr)
{
}

ANGLERenderTest::~ANGLERenderTest()
{
    SafeDelete(mOSWindow);
    SafeDelete(mEGLWindow);
}

void ANGLERenderTest::SetUp()
{
    mOSWindow = CreateOSWindow();
    mEGLWindow = new EGLWindow(mTestParams.majorVersion, mTestParams.minorVersion,
                               mTestParams.eglParameters);
    mEGLWindow->setSwapInterval(0);

    if (!mOSWindow->initialize(mName, mTestParams.windowWidth, mTestParams.windowHeight))
    {
        FAIL() << "Failed initializing OSWindow";
        return;
    }

    if (!mEGLWindow->initializeGL(mOSWindow))
    {
        FAIL() << "Failed initializing EGLWindow";
        return;
    }

    initializeBenchmark();

    ANGLEPerfTest::SetUp();
}

void ANGLERenderTest::TearDown()
{
    ANGLEPerfTest::TearDown();

    destroyBenchmark();

    mEGLWindow->destroyGL();
    mOSWindow->destroy();
}

void ANGLERenderTest::step(float dt, double totalTime)
{
    stepBenchmark(dt, totalTime);

    // Clear events that the application did not process from this frame
    Event event;
    while (popEvent(&event))
    {
        // If the application did not catch a close event, close now
        if (event.Type == Event::EVENT_CLOSED)
        {
            mRunning = false;
        }
    }

    if (mRunning)
    {
        draw();
        mEGLWindow->swap();
        mOSWindow->messageLoop();
    }
}

void ANGLERenderTest::draw()
{
    if (mTimer->getElapsedTime() > mRunTimeSeconds)
    {
        mRunning = false;
        return;
    }

    ++mNumFrames;

    beginDrawBenchmark();

    for (unsigned int iteration = 0; iteration < mDrawIterations; ++iteration)
    {
        drawBenchmark();
    }

    endDrawBenchmark();
}

bool ANGLERenderTest::popEvent(Event *event)
{
    return mOSWindow->popEvent(event);
}

OSWindow *ANGLERenderTest::getWindow()
{
    return mOSWindow;
}
