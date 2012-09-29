
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

#endif // SkApplication_DEFINED
