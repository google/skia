/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#import "gl_context_helper.h"

#import <AvailabilityMacros.h>
#import <OpenGL/OpenGL.h>
#import <dlfcn.h>
#import <cstdio>

    // cribbed from https://skia.googlesource.com/skia/+/78f0b8a7eda92e59943164caaaa00e01404643b9/tools/gpu/gl/mac/CreatePlatformGLTestContext_mac.cpp#46
bool initialize_gl_mac() {
    CGLPixelFormatAttribute attributes[] = {
        // base parameters
        kCGLPFAOpenGLProfile,
        (CGLPixelFormatAttribute) kCGLOGLPVersion_3_2_Core,
        kCGLPFADoubleBuffer,
        (CGLPixelFormatAttribute)NULL
    };

    CGLPixelFormatObj pixFormat;
    GLint npix;
    CGLChoosePixelFormat(attributes, &pixFormat, &npix);
    if (nullptr == pixFormat) {
        printf("CGLChoosePixelFormat failed.");
        return false;
    }

    CGLContextObj context;
    CGLCreateContext(pixFormat, nullptr, &context);
    CGLReleasePixelFormat(pixFormat);

    if (!context) {
        printf("CGLCreateContext failed.");
        return false;
    }

    CGLSetCurrentContext(context);
    return true;
}
