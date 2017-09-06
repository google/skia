/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <QtWidgets>

#include "MainWindow.h"

MainWindow::MainWindow() {
    this->createActions();
    this->createStatusBar();
    this->createDockWindows();

    this->setWindowTitle("MDB Viz");

    this->readSettings();
    this->setUnifiedTitleAndToolBarOnMac(true);
}

void MainWindow::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
        this->loadFile(fileName);
    }
}

void MainWindow::setupOpListWidget() {
    fOpListWidget->clear();

    QTreeWidgetItem* item = nullptr;
    SkTDArray<QTreeWidgetItem*> parents;

    for (int i = 0; i < fModel.numOps(); i++) {
        item = new QTreeWidgetItem();

        item->setText(0, QString::number(i));
        item->setData(0, Qt::UserRole, i);
        item->setText(1, fModel.getOpName(i));

        if (fModel.isHierarchyPop(i)) {
            parents.pop();
        }

        if (parents.isEmpty()) {
            fOpListWidget->addTopLevelItem(item);
        } else {
            parents.top()->addChild(item);
        }

        if (fModel.isHierarchyPush(i)) {
            *parents.push() = item;
        }
    }

    fOpListWidget->setCurrentItem(item);
    fOpListWidget->expandToDepth(100);
}

void MainWindow::presentCurrentRenderState() {
    fImage = QImage((uchar*)fModel.getPixels(), fModel.width(), fModel.height(),
                    QImage::Format_RGBA8888);
    fImageLabel->setPixmap(QPixmap::fromImage(fImage));
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

    Model::ErrorCode err = fModel.load(str.c_str());
    if (Model::ErrorCode::kOK != err) {
        this->statusBar()->showMessage(Model::ErrorString(err));
        return;
    }

    this->setupOpListWidget();
    this->presentCurrentRenderState();

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
}


void MainWindow::about() {
   QMessageBox::about(this, "About MDB Viz", "Visualize MDB");
}

void MainWindow::createActions() {

    // File menu
    QMenu* fileMenu = this->menuBar()->addMenu(tr("&File"));
    QToolBar* fileToolBar = this->addToolBar(tr("File"));

    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
    QAction* openAct = new QAction(openIcon, tr("&Open..."), this);
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

    // View menu
    fViewMenu = this->menuBar()->addMenu(tr("&View"));

    // Help menu
    this->menuBar()->addSeparator();

    QMenu* helpMenu = this->menuBar()->addMenu(tr("&Help"));

    QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    aboutAct->setStatusTip(tr("Show the application's About box"));
}

void MainWindow::onCurrentItemChanged(QTreeWidgetItem* cur, QTreeWidgetItem* /* prev */) {
    int currentRow = cur->data(0, Qt::UserRole).toInt();
    fModel.setCurOp(currentRow);
    this->presentCurrentRenderState();
}

void MainWindow::createStatusBar() {
    this->statusBar()->showMessage(tr("Ready"));
}

void MainWindow::createDockWindows() {

    // Op List Window
    {
        QDockWidget* opListDock = new QDockWidget("Ops", this);
        opListDock->setAllowedAreas(Qt::LeftDockWidgetArea);

        fOpListWidget = new QTreeWidget(opListDock);

        QTreeWidgetItem* headerItem = new QTreeWidgetItem;
        headerItem->setText(0, "Index");
        headerItem->setText(1, "Op Name");
        fOpListWidget->setHeaderItem(headerItem);

        fOpListWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        fOpListWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

        opListDock->setWidget(fOpListWidget);
        this->addDockWidget(Qt::LeftDockWidgetArea, opListDock);

        fViewMenu->addAction(opListDock->toggleViewAction());

        connect(fOpListWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
                this, SLOT(onCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
    }

    // Main canvas Window
    {
        QDockWidget* mainCanvasDock = new QDockWidget("Main Canvas", this);
        mainCanvasDock->setAllowedAreas(Qt::RightDockWidgetArea);

        fImageLabel = new QLabel(mainCanvasDock);

        fImage = QImage(1024, 1024, QImage::Format_RGBA8888);
        fImage.fill(0);
        fImageLabel->setPixmap(QPixmap::fromImage(fImage));

        mainCanvasDock->setWidget(fImageLabel);
        this->addDockWidget(Qt::RightDockWidgetArea, mainCanvasDock);

        fViewMenu->addAction(mainCanvasDock->toggleViewAction());
    }
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
