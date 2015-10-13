//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "SampleApplication.h"
#include "EGLWindow.h"

SampleApplication::SampleApplication(const std::string &name,
                                     size_t width,
                                     size_t height,
                                     EGLint glesMajorVersion,
                                     EGLint glesMinorVersion,
                                     EGLint requestedRenderer)
    : mName(name), mWidth(width), mHeight(height), mRunning(false)
{
    mEGLWindow.reset(new EGLWindow(glesMajorVersion, glesMinorVersion,
                                   EGLPlatformParameters(requestedRenderer)));
    mTimer.reset(CreateTimer());
    mOSWindow.reset(CreateOSWindow());

    mEGLWindow->setConfigRedBits(8);
    mEGLWindow->setConfigGreenBits(8);
    mEGLWindow->setConfigBlueBits(8);
    mEGLWindow->setConfigAlphaBits(8);
    mEGLWindow->setConfigDepthBits(24);
    mEGLWindow->setConfigStencilBits(8);

    // Disable vsync
    mEGLWindow->setSwapInterval(0);
}

SampleApplication::~SampleApplication()
{
}

bool SampleApplication::initialize()
{
    return true;
}

void SampleApplication::destroy()
{
}

void SampleApplication::step(float dt, double totalTime)
{
}

void SampleApplication::draw()
{
}

void SampleApplication::swap()
{
    mEGLWindow->swap();
}

OSWindow *SampleApplication::getWindow() const
{
    return mOSWindow.get();
}

EGLConfig SampleApplication::getConfig() const
{
    return mEGLWindow->getConfig();
}

EGLDisplay SampleApplication::getDisplay() const
{
    return mEGLWindow->getDisplay();
}

EGLSurface SampleApplication::getSurface() const
{
    return mEGLWindow->getSurface();
}

EGLContext SampleApplication::getContext() const
{
    return mEGLWindow->getContext();
}

int SampleApplication::run()
{
    if (!mOSWindow->initialize(mName, mWidth, mHeight))
    {
        return -1;
    }

    mOSWindow->setVisible(true);

    if (!mEGLWindow->initializeGL(mOSWindow.get()))
    {
        return -1;
    }

    mRunning = true;
    int result = 0;

    if (!initialize())
    {
        mRunning = false;
        result = -1;
    }

    mTimer->start();
    double prevTime = 0.0;

    while (mRunning)
    {
        double elapsedTime = mTimer->getElapsedTime();
        double deltaTime = elapsedTime - prevTime;

        step(static_cast<float>(deltaTime), elapsedTime);

        // Clear events that the application did not process from this frame
        Event event;
        while (popEvent(&event))
        {
            // If the application did not catch a close event, close now
            if (event.Type == Event::EVENT_CLOSED)
            {
                exit();
            }
        }

        if (!mRunning)
        {
            break;
        }

        draw();
        swap();

        mOSWindow->messageLoop();

        prevTime = elapsedTime;
    }

    destroy();
    mEGLWindow->destroyGL();
    mOSWindow->destroy();

    return result;
}

void SampleApplication::exit()
{
    mRunning = false;
}

bool SampleApplication::popEvent(Event *event)
{
    return mOSWindow->popEvent(event);
}
