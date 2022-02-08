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

    int64_t sk_tools::getMaxResidentSetSizeBytes() {
      zx_info_task_stats_t task_stats;
      zx_handle_t process = zx_process_self();
      zx_status_t status = zx_object_get_info(
      process, ZX_INFO_TASK_STATS, &task_stats, sizeof(task_stats), nullptr, nullptr);
      if (status != ZX_OK) {
        return -1;
      }
      return (task_stats.mem_private_bytes + task_stats.mem_shared_bytes);
    }
#elif defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS) || defined(SK_BUILD_FOR_ANDROID)
    #include <sys/resource.h>
    int64_t sk_tools::getMaxResidentSetSizeBytes() {
        struct rusage ru;
        getrusage(RUSAGE_SELF, &ru);
    #if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
        return ru.ru_maxrss;         // Darwin reports bytes.
    #else
        return ru.ru_maxrss * 1024;  // Linux reports kilobytes.
    #endif
    }
#elif defined(SK_BUILD_FOR_WIN)
    #include <windows.h>
    #include <psapi.h>
    int64_t sk_tools::getMaxResidentSetSizeBytes() {
        PROCESS_MEMORY_COUNTERS info;
        GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
        return info.PeakWorkingSetSize;
    }
#else
    int64_t sk_tools::getMaxResidentSetSizeBytes() { return -1; }
#endif

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
    #include <mach/mach.h>
    int64_t sk_tools::getCurrResidentSetSizeBytes() {
        mach_task_basic_info info;
        mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
        if (KERN_SUCCESS !=
                task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &count)) {
            return -1;
        }
        return info.resident_size;
    }
#elif defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_ANDROID)  // N.B. /proc is Linux-only.
    #include <unistd.h>
    #include <stdio.h>
    int64_t sk_tools::getCurrResidentSetSizeBytes() {
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
        return rssPages * pageSize;
    }
#elif defined(SK_BUILD_FOR_WIN)
    int64_t sk_tools::getCurrResidentSetSizeBytes() {
        PROCESS_MEMORY_COUNTERS info;
        GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
        return info.WorkingSetSize;
    }
#else
    int64_t sk_tools::getCurrResidentSetSizeBytes() { return -1; }
#endif

int sk_tools::getMaxResidentSetSizeMB() {
    int64_t bytes = sk_tools::getMaxResidentSetSizeBytes();
    return bytes < 0 ? -1 : static_cast<int>(bytes / 1024 / 1024);
}

int sk_tools::getCurrResidentSetSizeMB() {
    int64_t bytes = sk_tools::getCurrResidentSetSizeBytes();
    return bytes < 0 ? -1 : static_cast<int>(bytes / 1024 / 1024);
}
