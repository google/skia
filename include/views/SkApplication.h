
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkApplication_DEFINED
#define SkApplication_DEFINED

class SkOSWindow;

extern SkOSWindow* create_sk_window(void* hwnd, int argc, char** argv);
extern void application_init();
extern void application_term();

#ifdef SK_BUILD_FOR_IOS
enum IOS_launch_type {
    kError_iOSLaunchType = -1,
    kTool_iOSLaunchType = 0,
    kApplication__iOSLaunchType = 1
};

extern IOS_launch_type set_cmd_line_args(int argc, char *argv[],
                                         const char* resourceDir);
#endif

#endif // SkApplication_DEFINED
