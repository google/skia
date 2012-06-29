
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDebuggerGUI.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    SkDebuggerGUI w;
    w.show();
    return a.exec();
}
