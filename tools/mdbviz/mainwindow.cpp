/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <QtWidgets>

#include "MainWindow.h"

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkPicture.h"
#include "SkStream.h"

MainWindow::MainWindow()
        : fImageLabel(new QLabel) {
    this->setCentralWidget(fImageLabel);

    this->createActions();
    this->createStatusBar();
    this->readSettings();
    this->setUnifiedTitleAndToolBarOnMac(true);
}

void MainWindow::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
        this->loadFile(fileName);
    }
}

void MainWindow::loadFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("MDB Viz"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QTextStream in(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif

    std::string str = file.fileName().toLocal8Bit().constData();

    std::unique_ptr<SkStream> stream = SkStream::MakeFromFile(str.c_str());
    if (!stream) {
        this->statusBar()->showMessage(tr("Couldn't read file"));
        return;
    }
    sk_sp<SkPicture> pic(SkPicture::MakeFromStream(stream.get()));
    if (!pic) {
        this->statusBar()->showMessage(tr("Couldn't decode picture"));
        return;
    }

    SkBitmap bm;

    SkImageInfo ii = SkImageInfo::MakeN32Premul(1024, 1024);
    bm.allocPixels(ii, 0);

    SkCanvas canvas(bm);

    canvas.drawPicture(pic);

    fImage = QImage((uchar*)bm.getPixels(), bm.width(), bm.height(), QImage::Format_RGBA8888);
    fImageLabel->setPixmap(QPixmap::fromImage(fImage));

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
}

void MainWindow::createActions() {

    QMenu *fileMenu = this->menuBar()->addMenu(tr("&File"));
    QToolBar *fileToolBar = this->addToolBar(tr("File"));

    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
    QAction *openAct = new QAction(openIcon, tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::openFile);
    fileMenu->addAction(openAct);
    fileToolBar->addAction(openAct);

    fileMenu->addSeparator();

    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    QAction *exitAct = fileMenu->addAction(exitIcon, tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
}

void MainWindow::createStatusBar() {
    this->statusBar()->showMessage(tr("Ready"));
}

void MainWindow::readSettings() {
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        this->restoreGeometry(geometry);
    }
}

void MainWindow::writeSettings() {
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", this->saveGeometry());
}
