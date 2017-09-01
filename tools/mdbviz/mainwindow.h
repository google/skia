/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef MainWindow_DEFINED
#define MainWindow_DEFINED

#include <QMainWindow>

class QLabel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow();

private slots:
    void openFile();

private:
    void loadFile(const QString &fileName);

    void createActions();
    void createStatusBar();
    void readSettings();
    void writeSettings();

    QImage  fImage;
    QLabel* fImageLabel;
};

#endif
