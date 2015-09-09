
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDebuggerGUI.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include <QApplication>

__SK_FORCE_IMAGE_DECODER_LINKING;


static void usage(const char * argv0) {
    SkDebugf("%s <input> \n", argv0);
    SkDebugf("    [--help|-h]: show this help message\n");
    SkDebugf("\n\n");
    SkDebugf("     input:     Either a directory or a single .skp file.\n");
}

int main(int argc, char *argv[]) {
#ifndef SK_BUILD_FOR_WIN32
    // Set numeric formatting to default. Otherwise shaders will have numbers with wrong comma.
    // QApplication documentation recommends setlocale("LC_NUMERIC", "C") after QApplication
    // constuction.  However, the components Qt calls (X11 libs, ..) will override that.
    setenv("LC_NUMERIC", "C", 1);
#endif
    SkGraphics::Init();
    QApplication a(argc, argv);
    QStringList argList = a.arguments();

    if (argList.count() <= 0) {
        return -1;  // should at least have command name
    }

    SkString input;

    QStringList::const_iterator iter = argList.begin();

    SkString commandName(iter->toAscii().data());
    ++iter; // skip the command name

    for ( ; iter != argList.end(); ++iter) {
        if (0 == iter->compare("--help") || 0 == iter->compare("-h")) {
            usage(commandName.c_str());
            return -1;
        } else if (input.isEmpty()) {
            input = SkString(iter->toAscii().data());
        } else {
            usage(commandName.c_str());
            return -1;
        }
    }

    SkDebuggerGUI w;

    if (!input.isEmpty()) {
        if (SkStrEndsWith(input.c_str(), ".skp")) {
            w.openFile(input.c_str());
        } else {
            w.setupDirectoryWidget(input.c_str());
        }
    }

    w.show();
    int result = a.exec();
    return result;
}
