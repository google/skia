//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WindowSurfaceCGL.cpp: CGL implementation of egl::Surface for windows

#include "libANGLE/renderer/gl/cgl/WindowSurfaceCGL.h"

#import <Cocoa/Cocoa.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <OpenGL/OpenGL.h>
#import <QuartzCore/QuartzCore.h>

#include "common/debug.h"
#include "libANGLE/renderer/gl/cgl/DisplayCGL.h"
#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/RendererGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"

#define GL_TEXTURE_RECTANGLE_ARB 0x84F5

#include <iostream>

namespace
{

// All time computations in the file are done in nanoseconds but both the
// current time and the screen's present time are given as "mach time".
// The first thing we will do after receiving a mach time is convert it.
uint64_t MachTimeToNanoseconds(uint64_t machTime)
{
    static mach_timebase_info_data_t timebaseInfo;

    if (timebaseInfo.denom == 0)
    {
        mach_timebase_info(&timebaseInfo);
    }

    return (machTime * timebaseInfo.numer) / timebaseInfo.denom;
}

uint64_t NowInNanoseconds()
{
    return MachTimeToNanoseconds(mach_absolute_time());
}

}

namespace rx
{

// A wrapper around CoreVideo's DisplayLink. It polls the screen vsync time,
// and allows computing when is the next present time after a given time point.
class DisplayLink
{
  public:
    DisplayLink() : mDisplayLink(nullptr), mHostTimeRemainderNanos(0), mPresentIntervalNanos(0) {}

    ~DisplayLink()
    {
        if (mDisplayLink != nullptr)
        {
            stop();
            CVDisplayLinkRelease(mDisplayLink);
            mDisplayLink = nullptr;
        }
    }

    void start()
    {
        CVDisplayLinkCreateWithActiveCGDisplays(&mDisplayLink);
        ASSERT(mDisplayLink != nullptr);
        CVDisplayLinkSetOutputCallback(mDisplayLink, &Callback, this);
        CVDisplayLinkStart(mDisplayLink);
    }

    void stop()
    {
        ASSERT(mDisplayLink != nullptr);
        CVDisplayLinkStop(mDisplayLink);
    }

    uint64_t nextPresentAfter(uint64_t time)
    {
        // Load the two variables locally to avoid them changing in the middle of the computation.
        uint64_t presentInterval = mPresentIntervalNanos;
        uint64_t remainder       = mHostTimeRemainderNanos;

        if (presentInterval == 0)
        {
            return time;
        }

        uint64_t nextSwapNumber = (time - remainder) / presentInterval + 1;
        return nextSwapNumber * presentInterval + remainder;
    }

    uint64_t getPresentInterval() const { return mPresentIntervalNanos; }

  private:
    static CVReturn Callback(CVDisplayLinkRef displayLink,
                             const CVTimeStamp *now,
                             const CVTimeStamp *outputTime,
                             CVOptionFlags flagsIn,
                             CVOptionFlags *flagsOut,
                             void *userData)
    {
        DisplayLink *link = reinterpret_cast<DisplayLink *>(userData);
        link->tick(*outputTime);
        return kCVReturnSuccess;
    }

    void tick(const CVTimeStamp &outputTime)
    {
        // This is called from a special high priority thread so by setting
        // both member variables here we have a data race. However if we assume
        // the present interval doesn't change, the data race disappears after
        // the first tick.
        // In addition computations using these member variables are made to
        // produce a sensible result if one of the two members has the default
        // value of 0.
        uint64_t hostTimeNanos = MachTimeToNanoseconds(outputTime.hostTime);

        uint64_t numerator            = outputTime.videoRefreshPeriod;
        uint64_t denominator          = outputTime.videoTimeScale;
        uint64_t presentIntervalNanos = (1000000000 * numerator) / denominator;

        mHostTimeRemainderNanos = hostTimeNanos % presentIntervalNanos;
        mPresentIntervalNanos   = presentIntervalNanos;
    }

    CVDisplayLinkRef mDisplayLink;

    uint64_t mHostTimeRemainderNanos;
    uint64_t mPresentIntervalNanos;
};

WindowSurfaceCGL::WindowSurfaceCGL(RendererGL *renderer,
                                   CALayer *layer,
                                   const FunctionsGL *functions)
    : SurfaceGL(renderer),
      mLayer(layer),
      mFunctions(functions),
      mStateManager(renderer->getStateManager()),
      mWorkarounds(renderer->getWorkarounds()),
      mDisplayLink(nullptr),
      mCurrentSurface(0),
      mFramebuffer(0),
      mDSRenderbuffer(0)
{
    for (auto &surface : mSurfaces)
    {
        surface.texture   = 0;
        surface.ioSurface = nil;
    }
}

WindowSurfaceCGL::~WindowSurfaceCGL()
{
    if (mFramebuffer != 0)
    {
        mFunctions->deleteFramebuffers(1, &mFramebuffer);
        mFramebuffer = 0;
    }

    if (mDSRenderbuffer != 0)
    {
        mFunctions->deleteRenderbuffers(1, &mDSRenderbuffer);
        mDSRenderbuffer = 0;
    }

    for (auto &surface : mSurfaces)
    {
        freeSurfaceData(&surface);
    }

    if (mDisplayLink != nullptr)
    {
        SafeDelete(mDisplayLink);
    }
}

egl::Error WindowSurfaceCGL::initialize()
{
    unsigned width  = getWidth();
    unsigned height = getHeight();

    for (auto &surface : mSurfaces)
    {
        surface.lastPresentNanos = 0;
        initializeSurfaceData(&surface, width, height);
    }

    mFunctions->genRenderbuffers(1, &mDSRenderbuffer);
    mStateManager->bindRenderbuffer(GL_RENDERBUFFER, mDSRenderbuffer);
    mFunctions->renderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

    mFunctions->genFramebuffers(1, &mFramebuffer);
    mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    mFunctions->framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE_ARB,
                                     mSurfaces[0].texture, 0);
    mFunctions->framebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                        mDSRenderbuffer);

    mDisplayLink = new DisplayLink;
    mDisplayLink->start();

    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceCGL::makeCurrent()
{
    // TODO(cwallez) if it is the first makeCurrent set the viewport and scissor?
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceCGL::swap()
{
    // Because we are rendering to IOSurfaces we have to implement swapBuffers ourselves
    // (contrary to GLX, WGL or EGL) and have to send the IOSurfaces for presentation at the
    // right time to avoid causing tearing or flickering. Likewise we must make sure we do
    // not render to an IOSurface that is still being presented.
    // ANGLE standalone cannot post function to be executed at a later time so we can only
    // implement basic synchronization with the present time by sleeping.
    uint64_t presentInterval = mDisplayLink->getPresentInterval();
    uint64_t now             = NowInNanoseconds();

    {
        Surface &surface = mSurfaces[mCurrentSurface];
        // A flush is needed for the IOSurface to get the result of the GL operations
        // as specified in the documentation of CGLTexImageIOSurface2D
        mFunctions->flush();

        // We can only send the IOSurface to the CALayer during a present window
        // that is in the middle of two vsyncs, otherwise flickering can happen.
        {
            // The present window was determined empirically.
            static const double kPresentWindowMin = 0.3;
            static const double kPresentWindowMax = 0.7;

            uint64_t nextPresent      = mDisplayLink->nextPresentAfter(now);
            uint64_t presentWindowMin = nextPresent - presentInterval * (1.0 - kPresentWindowMin);
            uint64_t presentWindowMax = nextPresent - presentInterval * (1.0 - kPresentWindowMax);
            if (now <= presentWindowMin)
            {
                usleep((presentWindowMin - now) / 1000);
            }
            else if (now >= presentWindowMax)
            {
                uint64_t presentTarget = nextPresent + presentInterval * kPresentWindowMin;
                usleep((presentTarget - now) / 1000);
            }
        }

        // Put the IOSurface as the content of the layer
        [CATransaction begin];
        [mLayer setContents:(id)surface.ioSurface];
        [CATransaction commit];

        surface.lastPresentNanos = NowInNanoseconds();
    }

    mCurrentSurface = (mCurrentSurface + 1) % ArraySize(mSurfaces);

    {
        Surface &surface = mSurfaces[mCurrentSurface];

        {
            // We need to wait a bit after a swap before rendering to the IOSurface again
            // otherwise with a small desync between clocks could cause a flickering.
            static const double kDrawWindowStart = 0.05;

            uint64_t timePresentFinishes = mDisplayLink->nextPresentAfter(surface.lastPresentNanos);
            timePresentFinishes += presentInterval * kDrawWindowStart;

            if (now < timePresentFinishes)
            {
                usleep((timePresentFinishes - now) / 1000);
            }
        }

        mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
        mFunctions->framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                         GL_TEXTURE_RECTANGLE_ARB, surface.texture, 0);
    }

    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceCGL::postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceCGL::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceCGL::bindTexImage(EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceCGL::releaseTexImage(EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

void WindowSurfaceCGL::setSwapInterval(EGLint interval)
{
    // TODO(cwallez) investigate the need for swapIntervals other than 1
}

EGLint WindowSurfaceCGL::getWidth() const
{
    return CGRectGetWidth([mLayer frame]);
}

EGLint WindowSurfaceCGL::getHeight() const
{
    return CGRectGetHeight([mLayer frame]);
}

EGLint WindowSurfaceCGL::isPostSubBufferSupported() const
{
    UNIMPLEMENTED();
    return EGL_FALSE;
}

EGLint WindowSurfaceCGL::getSwapBehavior() const
{
    return EGL_BUFFER_DESTROYED;
}

FramebufferImpl *WindowSurfaceCGL::createDefaultFramebuffer(const gl::Framebuffer::Data &data)
{
    // TODO(cwallez) assert it happens only once?
    return new FramebufferGL(mFramebuffer, data, mFunctions, mWorkarounds, mStateManager);
}

void WindowSurfaceCGL::freeSurfaceData(Surface *surface)
{
    if (surface->texture != 0)
    {
        mFunctions->deleteTextures(1, &surface->texture);
        surface->texture = 0;
    }
    if (surface->ioSurface != nil)
    {
        CFRelease(surface->ioSurface);
        surface->ioSurface = nil;
    }
}

egl::Error WindowSurfaceCGL::initializeSurfaceData(Surface *surface, int width, int height)
{
    CFDictionaryRef ioSurfaceOptions = nil;
    {
        unsigned pixelFormat            = 'BGRA';
        const unsigned kBytesPerElement = 4;

        NSDictionary *options = @{
            (id) kIOSurfaceWidth : @(width),
            (id) kIOSurfaceHeight : @(height),
            (id) kIOSurfacePixelFormat : @(pixelFormat),
            (id) kIOSurfaceBytesPerElement : @(kBytesPerElement),
        };
        ioSurfaceOptions = reinterpret_cast<CFDictionaryRef>(options);
    }

    surface->ioSurface = IOSurfaceCreate(ioSurfaceOptions);

    mFunctions->genTextures(1, &surface->texture);
    mFunctions->bindTexture(GL_TEXTURE_RECTANGLE_ARB, surface->texture);

    CGLError error =
        CGLTexImageIOSurface2D(CGLGetCurrentContext(), GL_TEXTURE_RECTANGLE_ARB, GL_RGBA, width,
                               height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, surface->ioSurface, 0);

    if (error != kCGLNoError)
    {
        std::string errorMessage =
            "Could not create the IOSurfaces: " + std::string(CGLErrorString(error));
        return egl::Error(EGL_BAD_NATIVE_WINDOW, errorMessage.c_str());
    }

    return egl::Error(EGL_SUCCESS);
}

}
