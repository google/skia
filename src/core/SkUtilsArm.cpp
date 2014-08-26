
/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkUtilsArm.h"

#if SK_ARM_NEON_IS_DYNAMIC

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#if SK_BUILD_FOR_ANDROID
#  include <cpu-features.h>
#endif

// A function used to determine at runtime if the target CPU supports
// the ARM NEON instruction set. This implementation is Linux-specific.
static bool sk_cpu_arm_check_neon(void) {
    // If we fail any of the following, assume we don't have NEON instructions
    // This allows us to return immediately in case of error.
    bool result = false;

// Use the Android NDK's cpu-features helper library to detect NEON at runtime.
// See http://crbug.com/164154 to see why this is needed in Chromium for Android.
#ifdef SK_BUILD_FOR_ANDROID

  result = (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0;

#else  // SK_BUILD_FOR_ANDROID

    // There is no user-accessible CPUID instruction on ARM that we can use.
    // Instead, we must parse /proc/cpuinfo and look for the 'neon' feature.
    // For example, here's a typical output (Nexus S running ICS 4.0.3):
    /*
    Processor       : ARMv7 Processor rev 2 (v7l)
    BogoMIPS        : 994.65
    Features        : swp half thumb fastmult vfp edsp thumbee neon vfpv3
    CPU implementer : 0x41
    CPU architecture: 7
    CPU variant     : 0x2
    CPU part        : 0xc08
    CPU revision    : 2

    Hardware        : herring
    Revision        : 000b
    Serial          : 3833c77d6dc000ec
    */
    char   buffer[4096];

    do {
        // open /proc/cpuinfo
        int fd = TEMP_FAILURE_RETRY(open("/proc/cpuinfo", O_RDONLY));
        if (fd < 0) {
            SkDebugf("Could not open /proc/cpuinfo: %s\n", strerror(errno));
            break;
        }

        // Read the file. To simplify our search, we're going to place two
        // sentinel '\n' characters: one at the start of the buffer, and one at
        // the end. This means we reserve the first and last buffer bytes.
        buffer[0] = '\n';
        int size = TEMP_FAILURE_RETRY(read(fd, buffer+1, sizeof(buffer)-2));
        close(fd);

        if (size < 0) {  // should not happen
            SkDebugf("Could not read /proc/cpuinfo: %s\n", strerror(errno));
            break;
        }

        SkDebugf("START /proc/cpuinfo:\n%.*s\nEND /proc/cpuinfo\n",
                 size, buffer+1);

        // Compute buffer limit, and place final sentinel
        char* buffer_end = buffer + 1 + size;
        buffer_end[0] = '\n';

        // Now, find a line that starts with "Features", i.e. look for
        // '\nFeatures ' in our buffer.
        const char features[] = "\nFeatures\t";
        const size_t features_len = sizeof(features)-1;

        char*  line = (char*) memmem(buffer, buffer_end - buffer,
                                     features, features_len);
        if (line == NULL) {  // Weird, no Features line, bad kernel?
            SkDebugf("Could not find a line starting with 'Features'"
              "in /proc/cpuinfo ?\n");
            break;
        }

        line += features_len;  // Skip the "\nFeatures\t" prefix

        // Find the end of the current line
        char* line_end = (char*) memchr(line, '\n', buffer_end - line);
        if (line_end == NULL)
            line_end = buffer_end;

        // Now find an instance of 'neon' in the flags list. We want to
        // ensure it's only 'neon' and not something fancy like 'noneon'
        // so check that it follows a space.
        const char neon[] = " neon";
        const size_t neon_len = sizeof(neon)-1;
        const char* flag = (const char*) memmem(line, line_end - line,
                                                neon, neon_len);
        if (flag == NULL)
            break;

        // Ensure it is followed by a space or a newline.
        if (flag[neon_len] != ' ' && flag[neon_len] != '\n')
            break;

        // Fine, we support Arm NEON !
        result = true;

    } while (0);

#endif  // SK_BUILD_FOR_ANDROID

    if (result) {
        SkDEBUGF(("Device supports ARM NEON instructions!\n"));
    } else {
        SkDEBUGF(("Device does NOT support ARM NEON instructions!\n"));
    }
    return result;
}

static pthread_once_t  sOnce;
static bool            sHasArmNeon;

// called through pthread_once()
void sk_cpu_arm_probe_features(void) {
    sHasArmNeon = sk_cpu_arm_check_neon();
}

bool sk_cpu_arm_has_neon(void) {
    pthread_once(&sOnce, sk_cpu_arm_probe_features);
    return sHasArmNeon;
}

#endif // SK_ARM_NEON_IS_DYNAMIC
