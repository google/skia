/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"

#if defined(SK_BUILD_FOR_MAC)
#include <ApplicationServices/ApplicationServices.h>
#elif defined(SK_BUILD_FOR_WIN)
#include <windows.h>
#endif

int main(int argc, char** argv) {
#if defined(SK_BUILD_FOR_MAC)
    CGColorSpaceRef cs = CGDisplayCopyColorSpace(CGMainDisplayID());
    CFDataRef dataRef = CGColorSpaceCopyICCProfile(cs);
    const uint8_t* data = CFDataGetBytePtr(dataRef);
    size_t size = CFDataGetLength(dataRef);

    SkFILEWStream file("monitor_0.icc");
    file.write(data, size);

    CFRelease(cs);
    CFRelease(dataRef);
    return 0;
#elif defined(SK_BUILD_FOR_WIN)
    DISPLAY_DEVICE dd = { sizeof(DISPLAY_DEVICE) };
    SkString outputFilename;

    // Chrome's code for this currently just gets the primary monitor's profile. This code iterates
    // over all attached monitors, so it's "better" in that sense. Making intelligent use of this
    // information (via things like MonitorFromWindow or MonitorFromRect to pick the correct
    // profile for a particular window or region of a window), is an exercise left to the reader.
    for (int i = 0; EnumDisplayDevices(NULL, i, &dd, 0); ++i) {
        if (dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) {
            // There are other helpful things in dd at this point:
            // dd.DeviceString has a longer name for the adapter
            // dd.StateFlags indicates primary display, mirroring, etc...
            HDC dc = CreateDC(NULL, dd.DeviceName, NULL, NULL);
            if (dc) {
                char icmPath[MAX_PATH + 1];
                DWORD pathLength = MAX_PATH;
                if (GetICMProfile(dc, &pathLength, icmPath)) {
                    // GetICMProfile just returns the path to the installed profile (not the data)
                    outputFilename = SkStringPrintf("monitor_%d.icc", i);
                    CopyFile(icmPath, outputFilename.c_str(), FALSE);
                }
                DeleteDC(dc);
            }
        }
    }

    return 0;
#else
    SkDebugf("ERROR: Unsupported platform\n");
    return 1;
#endif
}
