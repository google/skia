/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "iOSShell.h"

#include "Resources.h"
#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
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

void tool_main(int argc, char *argv[]);

bool set_cmd_line_args(int argc, char *argv[], const char* resourceDir) {
    for (int index = 0; index < argc; ++index) {
        if (!strcmp("--test", argv[index])) {
            SetResourcePath(resourceDir);
            tool_main(argc - 1, argv);
            return true;
        }
    }
    return false;
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
    SkGraphics::Term();
}
