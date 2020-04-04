/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "tools/ProcStats.h"

#if defined(__Fuchsia__)
    #include <zircon/process.h>
    #include <zircon/syscalls.h>
    #include <zircon/syscalls/object.h>
    #include <zircon/types.h>

    int sk_tools::getMaxResidentSetSizeMB() {
      zx_info_task_stats_t task_stats;
      zx_handle_t process = zx_process_self();
      zx_status_t status = zx_object_get_info(
      process, ZX_INFO_TASK_STATS, &task_stats, sizeof(task_stats), NULL, NULL);
      if (status != ZX_OK) {
        return -1;
      }
      return (task_stats.mem_private_bytes + task_stats.mem_shared_bytes) / (1 << 20);
    }
#elif defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS) || defined(SK_BUILD_FOR_ANDROID)
    #include <sys/resource.h>
    int sk_tools::getMaxResidentSetSizeMB() {
        struct rusage ru;
        getrusage(RUSAGE_SELF, &ru);
    #if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
        return static_cast<int>(ru.ru_maxrss / 1024 / 1024);  // Darwin reports bytes.
    #else
        return static_cast<int>(ru.ru_maxrss / 1024);  // Linux reports kilobytes.
    #endif
    }
#elif defined(SK_BUILD_FOR_WIN)
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

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
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
#elif defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_ANDROID)  // N.B. /proc is Linux-only.
    #include <unistd.h>
    #include <stdio.h>
    int sk_tools::getCurrResidentSetSizeMB() {
        const long pageSize = sysconf(_SC_PAGESIZE);
        long long rssPages = 0;
        if (FILE* statm = fopen("/proc/self/statm", "r")) {
            // statm contains: program-size rss shared text lib data dirty, all in page counts.
            int rc = fscanf(statm, "%*d %lld", &rssPages);
            fclose(statm);
            if (rc != 1) {
                return -1;
            }
        }
        return rssPages * pageSize / 1024 / 1024;
    }

#elif defined(SK_BUILD_FOR_WIN)
    int sk_tools::getCurrResidentSetSizeMB() {
        PROCESS_MEMORY_COUNTERS info;
        GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
        return static_cast<int>(info.WorkingSetSize / 1024 / 1024);  // Windows reports bytes.
    }
#else
    int sk_tools::getCurrResidentSetSizeMB() { return -1; }
#endif
