//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "deqp_tests.h"

#include <EGL/eglext.h>

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <sstream>
#include <stdarg.h>
#include <windows.h>

#include "tcuDefs.hpp"
#include "tcuCommandLine.hpp"
#include "tcuPlatform.hpp"
#include "tcuApp.hpp"
#include "tcuResource.hpp"
#include "tcuTestLog.hpp"
#include "tcuTestExecutor.hpp"
#include "deUniquePtr.hpp"
#include "tes3TestPackage.hpp"

#include "win32/tcuWin32EglPlatform.hpp"

// Register the gles3 test cases
static tcu::TestPackage* createTestPackage(tcu::TestContext& testCtx)
{
    return new deqp::gles3::TestPackage(testCtx);
}
tcu::TestPackageDescriptor g_gles3PackageDescriptor("dEQP-GLES3", createTestPackage);

// Create a platform that supports custom display types
class Win32EglCustomDisplayPlatform  : public tcu::Win32EglPlatform
{
public:
    Win32EglCustomDisplayPlatform(EGLNativeDisplayType displayType)
        : mDisplayType(displayType)
    {
    }

    virtual ~Win32EglCustomDisplayPlatform()
    {
    }

    virtual tcu::NativeDisplay* createDefaultDisplay()
    {
        return new tcu::Win32EglDisplay(mDisplayType);
    }

private:
    EGLNativeDisplayType mDisplayType;
};

static std::vector<char> FormatArg(const char* fmt, ...)
{
    va_list vararg;
    va_start(vararg, fmt);
    int len = vsnprintf(NULL, 0, fmt, vararg);
    va_end(vararg);

    std::vector<char> buf(len + 1);

    va_start(vararg, fmt);
    vsnprintf(buf.data(), buf.size(), fmt, vararg);
    va_end(vararg);

    return buf;
}

static std::string GetExecutableDirectory()
{
    std::vector<char> executableFileBuf(MAX_PATH);
    DWORD executablePathLen = GetModuleFileNameA(NULL, executableFileBuf.data(), executableFileBuf.size());
    if (executablePathLen == 0)
    {
        return false;
    }

    std::string executableLocation = executableFileBuf.data();
    size_t lastPathSepLoc = executableLocation.find_last_of("\\/");
    if (lastPathSepLoc != std::string::npos)
    {
        executableLocation = executableLocation.substr(0, lastPathSepLoc);
    }
    else
    {
        executableLocation = "";
    }

    return executableLocation;
}

static DEQPConfig kCurrentConfig = { 256, 256, false, EGL_D3D11_ONLY_DISPLAY_ANGLE };

void SetCurrentConfig(const DEQPConfig& config)
{
    kCurrentConfig = config;
}

const DEQPConfig& GetCurrentConfig()
{
    return kCurrentConfig;
}

void RunDEQPTest(const std::string &testPath, const DEQPConfig& config)
{
    try
    {
        std::vector<char*> args;

        // Empty first argument for the program name
        args.push_back("deqp-gles3");

        std::vector<char> visibilityArg = FormatArg("--deqp-visibility=%s", config.hidden ? "hidden" : "windowed");
        args.push_back(visibilityArg.data());

        std::vector<char> widthArg = FormatArg("--deqp-surface-width=%u", config.width);
        args.push_back(widthArg.data());

        std::vector<char> heightArg = FormatArg("--deqp-surface-height=%u", config.height);
        args.push_back(heightArg.data());

        std::vector<char> testNameArg = FormatArg("--deqp-case=%s", testPath.c_str());
        args.push_back(testNameArg.data());

        // Redirect cout
        std::streambuf* oldCoutStreamBuf = std::cout.rdbuf();
        std::ostringstream strCout;
        std::cout.rdbuf(strCout.rdbuf());

        tcu::CommandLine                cmdLine(args.size(), args.data());
        tcu::DirArchive                 archive(GetExecutableDirectory().c_str());
        tcu::TestLog                    log(cmdLine.getLogFileName(), cmdLine.getLogFlags());
        de::UniquePtr<tcu::Platform>    platform(new Win32EglCustomDisplayPlatform(config.displayType));
        de::UniquePtr<tcu::App>         app(new tcu::App(*platform, archive, log, cmdLine));

        // Main loop.
        for (;;)
        {
            if (!app->iterate())
            {
                break;
            }
        }

        // Restore old cout
        std::cout.rdbuf(oldCoutStreamBuf);

        EXPECT_EQ(0, app->getResult().numFailed) << strCout.str();
    }
    catch (const std::exception& e)
    {
        FAIL() << e.what();
    }
}
