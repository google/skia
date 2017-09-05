/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef MainWindow_DEFINED
#define MainWindow_DEFINED

#include <memory>
#include <QMainWindow>

#include "SkDebugCanvas.h"

class QLabel;
class QListWidget;
class QMenu;


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow();

private slots:
    void openFile();
    void about();

private:
    void loadFile(const QString &fileName);
    void setupOpListWidget();

    void createActions();
    void createStatusBar();
    void createDockWindows();

    void readSettings();
    void writeSettings();

    QImage  fImage;
    QLabel* fImageLabel;

    QListWidget* fOpListWidget;

    QMenu* fViewMenu;

    std::unique_ptr<SkDebugCanvas> fDebugCanvas;
};

#endif
