//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef ANGLE_ENABLE_D3D9
#define ANGLE_ENABLE_D3D9
#endif

#ifndef ANGLE_ENABLE_D3D11
#define ANGLE_ENABLE_D3D11
#endif

#include "test_utils/ANGLETest.h"
#include "com_utils.h"

using namespace angle;

class QueryDisplayAttributeTest : public ANGLETest
{
  protected:
    QueryDisplayAttributeTest()
    {
        mQueryDisplayAttribEXT = nullptr;
        mQueryDeviceAttribEXT = nullptr;
        mQueryDeviceStringEXT = nullptr;
    }

    void SetUp() override
    {
        ANGLETest::SetUp();

        const char *extensionString = static_cast<const char*>(eglQueryString(getEGLWindow()->getDisplay(), EGL_EXTENSIONS));
        if (strstr(extensionString, "EGL_EXT_device_query"))
        {
            mQueryDisplayAttribEXT = (PFNEGLQUERYDISPLAYATTRIBEXTPROC)eglGetProcAddress("eglQueryDisplayAttribEXT");
            mQueryDeviceAttribEXT = (PFNEGLQUERYDEVICEATTRIBEXTPROC)eglGetProcAddress("eglQueryDeviceAttribEXT");
            mQueryDeviceStringEXT = (PFNEGLQUERYDEVICESTRINGEXTPROC)eglGetProcAddress("eglQueryDeviceStringEXT");
        }

        if (!mQueryDeviceStringEXT)
        {
            FAIL() << "ANGLE extension EGL_EXT_device_query export eglQueryDeviceStringEXT was not found";
        }

        if (!mQueryDisplayAttribEXT)
        {
            FAIL() << "ANGLE extension EGL_EXT_device_query export eglQueryDisplayAttribEXT was not found";
        }

        if (!mQueryDeviceAttribEXT)
        {
            FAIL() << "ANGLE extension EGL_EXT_device_query export eglQueryDeviceAttribEXT was not found";
        }

        EGLAttrib angleDevice = 0;
        EXPECT_EQ(EGL_TRUE, mQueryDisplayAttribEXT(getEGLWindow()->getDisplay(), EGL_DEVICE_EXT, &angleDevice));
        extensionString = static_cast<const char*>(mQueryDeviceStringEXT(reinterpret_cast<EGLDeviceEXT>(angleDevice), EGL_EXTENSIONS));
        if (strstr(extensionString, "EGL_ANGLE_device_d3d") == NULL)
        {
            FAIL() << "ANGLE extension EGL_ANGLE_device_d3d was not found";
        }
    }

    void TearDown() override
    {
        ANGLETest::TearDown();
    }

    PFNEGLQUERYDISPLAYATTRIBEXTPROC mQueryDisplayAttribEXT;
    PFNEGLQUERYDEVICEATTRIBEXTPROC mQueryDeviceAttribEXT;
    PFNEGLQUERYDEVICESTRINGEXTPROC mQueryDeviceStringEXT;
};

// This test attempts to obtain a D3D11 device and a D3D9 device using the eglQueryDeviceAttribEXT function.
// If the test is configured to use D3D11 then it should succeed to obtain a D3D11 device.
// If the test is confitured to use D3D9, then it should succeed to obtain a D3D9 device.
TEST_P(QueryDisplayAttributeTest, QueryDevice)
{
    EGLAttrib device = 0;
    EGLAttrib angleDevice = 0;
    if (getPlatformRenderer() == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        EXPECT_EQ(EGL_TRUE, mQueryDisplayAttribEXT(getEGLWindow()->getDisplay(), EGL_DEVICE_EXT, &angleDevice));
        EXPECT_EQ(EGL_TRUE, mQueryDeviceAttribEXT(reinterpret_cast<EGLDeviceEXT>(angleDevice), EGL_D3D11_DEVICE_ANGLE, &device));
        ID3D11Device *d3d11Device = reinterpret_cast<ID3D11Device*>(device);
        IDXGIDevice *dxgiDevice = DynamicCastComObject<IDXGIDevice>(d3d11Device);
        EXPECT_TRUE(dxgiDevice != nullptr);
        SafeRelease(dxgiDevice);
    }

    if (getPlatformRenderer() == EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE)
    {
        EXPECT_EQ(EGL_TRUE, mQueryDisplayAttribEXT(getEGLWindow()->getDisplay(), EGL_DEVICE_EXT, &angleDevice));
        EXPECT_EQ(EGL_TRUE, mQueryDeviceAttribEXT(reinterpret_cast<EGLDeviceEXT>(angleDevice), EGL_D3D9_DEVICE_ANGLE, &device));
        IDirect3DDevice9 *d3d9Device = reinterpret_cast<IDirect3DDevice9*>(device);
        IDirect3D9 *d3d9 = nullptr;
        EXPECT_EQ(S_OK, d3d9Device->GetDirect3D(&d3d9));
        EXPECT_TRUE(d3d9 != nullptr);
        SafeRelease(d3d9);
    }
}

// This test attempts to obtain a D3D11 device from a D3D9 configured system and a D3D9 device from
// a D3D11 configured system using the eglQueryDeviceAttribEXT function.
// If the test is configured to use D3D11 then it should fail to obtain a D3D11 device.
// If the test is confitured to use D3D9, then it should fail to obtain a D3D9 device.
TEST_P(QueryDisplayAttributeTest, QueryDeviceBadAttrbiute)
{
    EGLAttrib device = 0;
    EGLAttrib angleDevice = 0;
    if (getPlatformRenderer() == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        EXPECT_EQ(EGL_TRUE, mQueryDisplayAttribEXT(getEGLWindow()->getDisplay(), EGL_DEVICE_EXT, &angleDevice));
        EXPECT_EQ(EGL_FALSE, mQueryDeviceAttribEXT(reinterpret_cast<EGLDeviceEXT>(angleDevice), EGL_D3D9_DEVICE_ANGLE, &device));
    }

    if (getPlatformRenderer() == EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE)
    {
        EXPECT_EQ(EGL_TRUE, mQueryDisplayAttribEXT(getEGLWindow()->getDisplay(), EGL_DEVICE_EXT, &angleDevice));
        EXPECT_EQ(EGL_FALSE, mQueryDeviceAttribEXT(reinterpret_cast<EGLDeviceEXT>(angleDevice), EGL_D3D11_DEVICE_ANGLE, &device));
    }
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_INSTANTIATE_TEST(QueryDisplayAttributeTest, ES2_D3D9(), ES2_D3D11());
