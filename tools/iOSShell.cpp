/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "iOSShell.h"

#include "Resources.h"
#include "SkApplication.h"
#include "SkCanvas.h"
#include "SkCommonFlags.h"
#include "SkGraphics.h"
#include "SkWindow.h"
#include "sk_tool_utils.h"

//////////////////////////////////////////////////////////////////////////////

static SkView* curr_view(SkWindow* wind) {
    SkView::F2BIter iter(wind);
    return iter.next();
}

ShellWindow::ShellWindow(void* hwnd, int argc, char** argv)
    : INHERITED(hwnd) {
    SkCommandLineFlags::Parse(argc, argv);
}

ShellWindow::~ShellWindow() {
}

///////////////////////////////////////////////////////////////////////////////

bool ShellWindow::onDispatchClick(int x, int y, Click::State state,
        void* owner, unsigned modi) {
    int w = SkScalarRoundToInt(this->width());
    int h = SkScalarRoundToInt(this->height());

    // check for the resize-box
    if (w - x < 16 && h - y < 16) {
        return false;   // let the OS handle the click
    } else {
        return this->INHERITED::onDispatchClick(x, y, state, owner, modi);
    }
}

void ShellWindow::onSizeChange() {
    this->INHERITED::onSizeChange();

    SkView::F2BIter iter(this);
    SkView* view = iter.next();
    view->setSize(this->width(), this->height());
}

DEFINE_bool(dm, false, "run dm");
DEFINE_bool(nanobench, false, "run nanobench");

int nanobench_main();
int dm_main();

IOS_launch_type set_cmd_line_args(int argc, char *argv[], const char* resourceDir) {
    SkCommandLineFlags::Parse(argc, argv);
    if (FLAGS_nanobench) {
        return nanobench_main() ? kError_iOSLaunchType : kTool_iOSLaunchType;
    }
    if (FLAGS_dm) {
        return dm_main() ? kError_iOSLaunchType : kTool_iOSLaunchType;
    }
    return kError_iOSLaunchType;
}

// FIXME: this should be in a header
SkOSWindow* create_sk_window(void* hwnd, int argc, char** argv);
SkOSWindow* create_sk_window(void* hwnd, int argc, char** argv) {
    return new ShellWindow(hwnd, argc, argv);
}

// FIXME: this should be in a header
void get_preferred_size(int* x, int* y, int* width, int* height);
void get_preferred_size(int* x, int* y, int* width, int* height) {
    *x = 10;
    *y = 50;
    *width = 640;
    *height = 480;
}

// FIXME: this should be in a header
void application_init();
void application_init() {
    SkGraphics::Init();
    SkEvent::Init();
}

// FIXME: this should be in a header
void application_term();
void application_term() {
    SkEvent::Term();
}
