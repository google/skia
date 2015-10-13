//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef ANGLE_TEST_CONFIGS_H_
#define ANGLE_TEST_CONFIGS_H_

// On Linux EGL/egl.h includes X.h which does defines for some very common
// names that are used by gtest (like None and Bool) and causes a lot of
// compilation errors. To work around this, even if this file doesn't use it,
// we include gtest before EGL so that it compiles fine in other files that
// want to use gtest.
#include <gtest/gtest.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "angle_test_instantiate.h"
#include "EGLWindow.h"

namespace angle
{

struct PlatformParameters
{
    PlatformParameters();
    PlatformParameters(EGLint majorVersion, EGLint minorVersion,
                       const EGLPlatformParameters &eglPlatformParameters);

    EGLint getRenderer() const;

    EGLint majorVersion;
    EGLint minorVersion;
    EGLPlatformParameters eglParameters;
};

bool operator<(const PlatformParameters &a, const PlatformParameters &b);
bool operator==(const PlatformParameters &a, const PlatformParameters &b);
std::ostream &operator<<(std::ostream& stream, const PlatformParameters &pp);

// EGL platforms
namespace egl_platform
{

EGLPlatformParameters DEFAULT();
EGLPlatformParameters DEFAULT_NULL();

EGLPlatformParameters D3D9();
EGLPlatformParameters D3D9_NULL();
EGLPlatformParameters D3D9_REFERENCE();

EGLPlatformParameters D3D11();
EGLPlatformParameters D3D11_FL11_1();
EGLPlatformParameters D3D11_FL11_0();
EGLPlatformParameters D3D11_FL10_1();
EGLPlatformParameters D3D11_FL10_0();
EGLPlatformParameters D3D11_FL9_3();

EGLPlatformParameters D3D11_NULL();

EGLPlatformParameters D3D11_WARP();
EGLPlatformParameters D3D11_FL11_1_WARP();
EGLPlatformParameters D3D11_FL11_0_WARP();
EGLPlatformParameters D3D11_FL10_1_WARP();
EGLPlatformParameters D3D11_FL10_0_WARP();
EGLPlatformParameters D3D11_FL9_3_WARP();

EGLPlatformParameters D3D11_REFERENCE();
EGLPlatformParameters D3D11_FL11_1_REFERENCE();
EGLPlatformParameters D3D11_FL11_0_REFERENCE();
EGLPlatformParameters D3D11_FL10_1_REFERENCE();
EGLPlatformParameters D3D11_FL10_0_REFERENCE();
EGLPlatformParameters D3D11_FL9_3_REFERENCE();

EGLPlatformParameters OPENGL();
EGLPlatformParameters OPENGL(EGLint major, EGLint minor);
EGLPlatformParameters OPENGL_NULL();

EGLPlatformParameters OPENGLES();

} // namespace egl_platform

// ANGLE tests platforms
PlatformParameters ES2_D3D9();
PlatformParameters ES2_D3D9_REFERENCE();

PlatformParameters ES2_D3D11();
PlatformParameters ES2_D3D11_FL11_0();
PlatformParameters ES2_D3D11_FL10_1();
PlatformParameters ES2_D3D11_FL10_0();
PlatformParameters ES2_D3D11_FL9_3();

PlatformParameters ES2_D3D11_WARP();
PlatformParameters ES2_D3D11_FL11_0_WARP();
PlatformParameters ES2_D3D11_FL10_1_WARP();
PlatformParameters ES2_D3D11_FL10_0_WARP();
PlatformParameters ES2_D3D11_FL9_3_WARP();

PlatformParameters ES2_D3D11_REFERENCE();
PlatformParameters ES2_D3D11_FL11_0_REFERENCE();
PlatformParameters ES2_D3D11_FL10_1_REFERENCE();
PlatformParameters ES2_D3D11_FL10_0_REFERENCE();
PlatformParameters ES2_D3D11_FL9_3_REFERENCE();

PlatformParameters ES3_D3D11();
PlatformParameters ES3_D3D11_FL11_1();
PlatformParameters ES3_D3D11_FL11_0();
PlatformParameters ES3_D3D11_FL10_1();
PlatformParameters ES3_D3D11_FL10_0();

PlatformParameters ES3_D3D11_WARP();
PlatformParameters ES3_D3D11_FL11_1_WARP();
PlatformParameters ES3_D3D11_FL11_0_WARP();
PlatformParameters ES3_D3D11_FL10_1_WARP();
PlatformParameters ES3_D3D11_FL10_0_WARP();

PlatformParameters ES3_D3D11_REFERENCE();
PlatformParameters ES3_D3D11_FL11_1_REFERENCE();
PlatformParameters ES3_D3D11_FL11_0_REFERENCE();
PlatformParameters ES3_D3D11_FL10_1_REFERENCE();
PlatformParameters ES3_D3D11_FL10_0_REFERENCE();

PlatformParameters ES2_OPENGL();
PlatformParameters ES2_OPENGL(EGLint major, EGLint minor);
PlatformParameters ES3_OPENGL();
PlatformParameters ES3_OPENGL(EGLint major, EGLint minor);

PlatformParameters ES2_OPENGLES();
PlatformParameters ES3_OPENGLES();

} // namespace angle

#endif // ANGLE_TEST_CONFIGS_H_
