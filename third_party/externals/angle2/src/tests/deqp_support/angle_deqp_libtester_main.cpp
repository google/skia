//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// deqp_libtester_main.cpp: Entry point for tester DLL.

#include <cstdio>
#include <iostream>

#include "angle_deqp_libtester.h"
#include "common/angleutils.h"
#include "deMath.h"
#include "deUniquePtr.hpp"
#include "tcuApp.hpp"
#include "tcuCommandLine.hpp"
#include "tcuDefs.hpp"
#include "tcuPlatform.hpp"
#include "tcuRandomOrderExecutor.h"
#include "tcuResource.hpp"
#include "tcuTestLog.hpp"

#if (DE_OS == DE_OS_WIN32)
#include <Windows.h>
#elif (DE_OS == DE_OS_UNIX) || (DE_OS == DE_OS_OSX)
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

tcu::Platform *createPlatform();

namespace
{

tcu::Platform *g_platform = nullptr;
tcu::CommandLine *g_cmdLine = nullptr;
tcu::DirArchive *g_archive = nullptr;
tcu::TestLog *g_log = nullptr;
tcu::TestContext *g_testCtx = nullptr;
tcu::TestPackageRoot *g_root = nullptr;
tcu::RandomOrderExecutor *g_executor = nullptr;

const char *g_dEQPDataSearchDirs[] =
{
    "data",
    "third_party/deqp/data",
    "../third_party/deqp/src/data",
    "deqp_support/data",
    "third_party/deqp/src/data",
    "../../third_party/deqp/src/data"
};

// TODO(jmadill): upstream to dEQP?
#if (DE_OS == DE_OS_WIN32)
deBool deIsDir(const char *filename)
{
    WIN32_FILE_ATTRIBUTE_DATA fileInformation;

    BOOL result = GetFileAttributesExA(filename, GetFileExInfoStandard, &fileInformation);
    if (result)
    {
        DWORD attribs = fileInformation.dwFileAttributes;
        return (attribs != INVALID_FILE_ATTRIBUTES) && ((attribs & FILE_ATTRIBUTE_DIRECTORY) > 0);
    }

    return false;
}
#elif (DE_OS == DE_OS_UNIX) || (DE_OS == DE_OS_OSX)
deBool deIsDir(const char *filename)
{
    struct stat st;
    int result = stat(filename, &st);
    return result == 0 && ((st.st_mode & S_IFDIR) == S_IFDIR);
}
#else
#error TODO(jmadill): support other platforms
#endif

const char *FindDataDir()
{
    for (size_t dirIndex = 0; dirIndex < ArraySize(g_dEQPDataSearchDirs); ++dirIndex)
    {
        if (deIsDir(g_dEQPDataSearchDirs[dirIndex]))
        {
            return g_dEQPDataSearchDirs[dirIndex];
        }
    }

    return nullptr;
}

bool InitPlatform(int argc, const char *argv[])
{
    try
    {
#if (DE_OS != DE_OS_WIN32)
        // Set stdout to line-buffered mode (will be fully buffered by default if stdout is pipe).
        setvbuf(stdout, DE_NULL, _IOLBF, 4 * 1024);
#endif
        g_platform = createPlatform();

        if (!deSetRoundingMode(DE_ROUNDINGMODE_TO_NEAREST))
        {
            std::cout << "Failed to set floating point rounding mode." << std::endl;
            return false;
        }

        const char *deqpDataDir = FindDataDir();

        if (deqpDataDir == nullptr)
        {
            std::cout << "Failed to find dEQP data directory." << std::endl;
            return false;
        }

        g_cmdLine = new tcu::CommandLine(argc, argv);
        g_archive = new tcu::DirArchive(deqpDataDir);
        g_log = new tcu::TestLog(g_cmdLine->getLogFileName(), g_cmdLine->getLogFlags());
        g_testCtx = new tcu::TestContext(*g_platform, *g_archive, *g_log, *g_cmdLine, DE_NULL);
        g_root = new tcu::TestPackageRoot(*g_testCtx, tcu::TestPackageRegistry::getSingleton());
        g_executor = new tcu::RandomOrderExecutor(*g_root, *g_testCtx);
    }
    catch (const std::exception& e)
    {
        tcu::die("%s", e.what());
        return false;
    }

    return true;
}

} // anonymous namespace

// Exported to the tester app.
ANGLE_LIBTESTER_EXPORT int deqp_libtester_main(int argc, const char *argv[])
{
    if (!InitPlatform(argc, argv))
    {
        tcu::die("Could not initialize the dEQP platform");
    }

    try
    {
        de::UniquePtr<tcu::App> app(new tcu::App(*g_platform, *g_archive, *g_log, *g_cmdLine));

        // Main loop.
        for (;;)
        {
            if (!app->iterate())
                break;
        }
    }
    catch (const std::exception &e)
    {
        deqp_libtester_shutdown_platform();
        tcu::die("%s", e.what());
    }

    deqp_libtester_shutdown_platform();
    return 0;
}

ANGLE_LIBTESTER_EXPORT void deqp_libtester_shutdown_platform()
{
    delete g_executor;
    delete g_root;
    delete g_testCtx;
    delete g_log;
    delete g_archive;
    delete g_cmdLine;
    delete g_platform;
}

ANGLE_LIBTESTER_EXPORT bool deqp_libtester_run(const char *caseName)
{
    const char *emptyString = "";
    if (g_platform == nullptr && !InitPlatform(1, &emptyString))
    {
        tcu::die("Failed to initialize platform.");
    }

    try
    {
        // Poll platform events
        const bool platformOk = g_platform->processEvents();

        if (platformOk)
        {
            const tcu::TestStatus &result = g_executor->execute(caseName);
            switch (result.getCode())
            {
                case QP_TEST_RESULT_PASS:
                case QP_TEST_RESULT_NOT_SUPPORTED:
                    return true;
                case QP_TEST_RESULT_QUALITY_WARNING:
                    std::cout << "Quality warning! " << result.getDescription() << std::endl;
                    return true;
                case QP_TEST_RESULT_COMPATIBILITY_WARNING:
                    std::cout << "Compatiblity warning! " << result.getDescription() << std::endl;
                    return true;
                default:
                    return false;
            }
        }
        else
        {
            std::cout << "Aborted test!" << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Exception running test: " << e.what() << std::endl;
    }

    return false;
}
