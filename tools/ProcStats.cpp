/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ProcStats.h"

#if defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_ANDROID)
    #include <sys/resource.h>
    int sk_tools::getMaxResidentSetSizeMB() {
        struct rusage ru;
        getrusage(RUSAGE_SELF, &ru);
    #if defined(SK_BUILD_FOR_MAC)
        return static_cast<int>(ru.ru_maxrss / 1024 / 1024);  // Darwin reports bytes.
    #else
        return static_cast<int>(ru.ru_maxrss / 1024);  // Linux reports kilobytes.
    #endif
    }
#elif defined(SK_BUILD_FOR_WIN32)
    #include <windows.h>
    #include <psapi.h>
    int sk_tools::getMaxResidentSetSizeMB() {
        PROCESS_MEMORY_COUNTERS info;
        GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
        return static_cast<int>(info.PeakWorkingSetSize / 1024 / 1024);  // Windows reports bytes.
    }
#else
    int sk_tools::getMaxResidentSetSizeMB() { return -1; }
#endif

#if defined(SK_BUILD_FOR_MAC)
    #include <mach/mach.h>
    int sk_tools::getCurrResidentSetSizeMB() {
        mach_task_basic_info info;
        mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
        if (KERN_SUCCESS !=
                task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &count)) {
            return -1;
        }
        return info.resident_size / 1024 / 1024;  // Darwin reports bytes.
    }
#elif defined(SK_BUILD_FOR_WIN32)
    int sk_tools::getCurrResidentSetSizeMB() {
        PROCESS_MEMORY_COUNTERS info;
        GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
        return static_cast<int>(info.WorkingSetSize / 1024 / 1024);  // Windows reports bytes.
    }
#else
    int sk_tools::getCurrResidentSetSizeMB() { return -1; }
#endif
