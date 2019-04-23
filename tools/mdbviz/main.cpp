/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <QtWidgets>

#include "tools/mdbviz/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Google");
    QCoreApplication::setApplicationName("MDB Viz");
    QCoreApplication::setApplicationVersion("0.0");

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addVersionOption();
    parser.process(app);

    MainWindow mainWin;
    mainWin.show();

    return app.exec();
}


