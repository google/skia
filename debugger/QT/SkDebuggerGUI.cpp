/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <iostream>
#include "SkDebuggerGUI.h"
#include <QListWidgetItem>

SkDebuggerGUI::SkDebuggerGUI(QWidget *parent) :
        QMainWindow(parent)
    , fActionOpen(this)
    , fActionBreakpoint(this)
    , fActionCancel(this)
    , fActionClearBreakpoints(this)
    , fActionClearDeletes(this)
    , fActionClose(this)
    , fActionCreateBreakpoint(this)
    , fActionDelete(this)
    , fActionDirectory(this)
    , fActionGoToLine(this)
    , fActionInspector(this)
    , fActionPlay(this)
    , fActionPause(this)
    , fActionRewind(this)
    , fActionShowDeletes(this)
    , fActionStepBack(this)
    , fActionStepForward(this)
    , fCentralWidget(this)
    , fFilter(&fCentralWidget)
    , fContainerLayout(&fCentralWidget)
    , fListWidget(&fCentralWidget)
    , fDirectoryWidget(&fCentralWidget)
    , fCanvasWidget(&fCentralWidget)
    , fSettingsWidget(&fCentralWidget)
    , fStatusBar(this)
    , fMenuBar(this)
    , fMenuFile(this)
    , fMenuNavigate(this)
    , fMenuView(this)
    , fToolBar(this)
    , fBreakpointsActivated(false)
    , fDeletesActivated(false)
    , fPause(false)
    , fLoading(false)
{
    setupUi(this);
    connect(&fListWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(registerListClick(QListWidgetItem *)));
    connect(&fActionOpen, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(&fActionDirectory, SIGNAL(triggered()), this, SLOT(toggleDirectory()));
    connect(&fDirectoryWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(loadFile(QListWidgetItem *)));
    connect(&fActionDelete, SIGNAL(triggered()), this, SLOT(actionDelete()));
    connect(&fListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(toggleBreakpoint()));
    connect(&fActionRewind, SIGNAL(triggered()), this, SLOT(actionRewind()));
    connect(&fActionPlay, SIGNAL(triggered()), this, SLOT(actionPlay()));
    connect(&fActionStepBack, SIGNAL(triggered()), this, SLOT(actionStepBack()));
    connect(&fActionStepForward, SIGNAL(triggered()), this, SLOT(actionStepForward()));
    connect(&fActionBreakpoint, SIGNAL(triggered()), this, SLOT(actionBreakpoints()));
    connect(&fActionInspector, SIGNAL(triggered()), this, SLOT(actionInspector()));
    connect(&fActionInspector, SIGNAL(triggered()), this, SLOT(actionSettings()));
    connect(&fFilter, SIGNAL(activated(QString)), this, SLOT(toggleFilter(QString)));
    connect(&fActionCancel, SIGNAL(triggered()), this, SLOT(actionCancel()));
    connect(&fActionClearBreakpoints, SIGNAL(triggered()), this, SLOT(actionClearBreakpoints()));
    connect(&fActionClearDeletes, SIGNAL(triggered()), this, SLOT(actionClearDeletes()));
    connect(&fActionClose, SIGNAL(triggered()), this, SLOT(actionClose()));
    connect(fSettingsWidget.getVisibilityButton(), SIGNAL(toggled(bool)), this, SLOT(actionCommandFilter()));
    connect(fSettingsWidget.getGLCheckBox(), SIGNAL(toggled(bool)), this, SLOT(actionGLWidget(bool)));
    connect(fSettingsWidget.getRasterCheckBox(), SIGNAL(toggled(bool)), this, SLOT(actionRasterWidget(bool)));
    connect(&fActionPause, SIGNAL(toggled(bool)), this, SLOT(pauseDrawing(bool)));
    connect(&fActionCreateBreakpoint, SIGNAL(activated()), this, SLOT(toggleBreakpoint()));
    connect(&fActionShowDeletes, SIGNAL(triggered()), this, SLOT(showDeletes()));
    connect(&fCanvasWidget, SIGNAL(hitChanged(int)), this, SLOT(selectCommand(int)));
    connect(&fCanvasWidget, SIGNAL(hitChanged(int)), &fSettingsWidget, SLOT(updateHit(int)));
    connect(&fCanvasWidget, SIGNAL(scaleFactorChanged(float)), this, SLOT(actionScale(float)));
    connect(&fCanvasWidget, SIGNAL(commandChanged(int)), &fSettingsWidget, SLOT(updateCommand(int)));

    fInspectorWidget.setDisabled(true);
    fMenuEdit.setDisabled(true);
    fMenuNavigate.setDisabled(true);
    fMenuView.setDisabled(true);

}

SkDebuggerGUI::~SkDebuggerGUI() {
}

void SkDebuggerGUI::actionBreakpoints() {
    fBreakpointsActivated = !fBreakpointsActivated;
    for (int row = 0; row < fListWidget.count(); row++) {
        QListWidgetItem *item = fListWidget.item(row);
        item->setHidden(item->checkState() == Qt::Unchecked && fBreakpointsActivated);
    }
}

void SkDebuggerGUI::showDeletes() {
    fDeletesActivated = !fDeletesActivated;
    for (int row = 0; row < fListWidget.count(); row++) {
        QListWidgetItem *item = fListWidget.item(row);
        bool isVisible = fCanvasWidget.commandIsVisibleAtIndex(row);
        item->setHidden(isVisible && fDeletesActivated);
    }
}

void SkDebuggerGUI::actionCancel() {
    for (int row = 0; row < fListWidget.count(); row++) {
        fListWidget.item(row)->setHidden(false);
    }
}

void SkDebuggerGUI::actionClearBreakpoints() {
    for (int row = 0; row < fListWidget.count(); row++) {
        QListWidgetItem* item = fListWidget.item(row);
        item->setCheckState(Qt::Unchecked);
        item->setData(Qt::DecorationRole,
                QPixmap(":/images/Icons/blank.png"));
    }
}

void SkDebuggerGUI::actionClearDeletes() {
    for (int row = 0; row < fListWidget.count(); row++) {
        QListWidgetItem* item = fListWidget.item(row);
        item->setData(Qt::UserRole + 2, QPixmap(":/images/Icons/blank.png"));
        fCanvasWidget.setCommandVisibliltyAtIndex(row, true);
    }
    if (fPause) {
        fCanvasWidget.drawTo(fPausedRow);
    } else {
        fCanvasWidget.drawTo(fListWidget.currentRow());
    }
}

void SkDebuggerGUI::actionCommandFilter() {
    fCanvasWidget.toggleCurrentCommandFilter(
            fSettingsWidget.getVisibilityButton()->isChecked());
    fCanvasWidget.drawTo(fListWidget.currentRow());
}

void SkDebuggerGUI::actionClose() {
    this->close();
}

void SkDebuggerGUI::actionDelete() {
    int currentRow = fListWidget.currentRow();
    QListWidgetItem* item = fListWidget.currentItem();

    if (fCanvasWidget.commandIsVisibleAtIndex(currentRow)) {
        item->setData(Qt::UserRole + 2, QPixmap(":/images/Icons/delete.png"));
        fCanvasWidget.setCommandVisibliltyAtIndex(currentRow, false);
    } else {
        item->setData(Qt::UserRole + 2, QPixmap(":/images/Icons/blank.png"));
        fCanvasWidget.setCommandVisibliltyAtIndex(currentRow, true);
    }

    if (fPause) {
        fCanvasWidget.drawTo(fPausedRow);
    } else {
        fCanvasWidget.drawTo(currentRow);
    }
}

void SkDebuggerGUI::actionGLWidget(bool isToggled) {
    fCanvasWidget.setWidgetVisibility(SkCanvasWidget::kGPU_WidgetType, !isToggled);
}

void SkDebuggerGUI::actionInspector() {
    if (fInspectorWidget.isHidden()) {
        fInspectorWidget.setHidden(false);
    } else {
        fInspectorWidget.setHidden(true);
    }
}

void SkDebuggerGUI::actionPlay() {
    for (int row = fListWidget.currentRow() + 1; row < fListWidget.count();
            row++) {
        QListWidgetItem *item = fListWidget.item(row);
        if (item->checkState() == Qt::Checked) {
            fListWidget.setCurrentItem(item);
            return;
        }
    }
    fListWidget.setCurrentRow(fListWidget.count() - 1);
}

void SkDebuggerGUI::actionRasterWidget(bool isToggled) {
    fCanvasWidget.setWidgetVisibility(SkCanvasWidget::kRaster_8888_WidgetType, !isToggled);
}

void SkDebuggerGUI::actionRewind() {
    fListWidget.setCurrentRow(0);
}

void SkDebuggerGUI::actionScale(float scaleFactor) {
    fSettingsWidget.setZoomText(scaleFactor);
}

void SkDebuggerGUI::actionSettings() {
    if (fSettingsWidget.isHidden()) {
        fSettingsWidget.setHidden(false);
    } else {
        fSettingsWidget.setHidden(true);
    }
}

void SkDebuggerGUI::actionStepBack() {
    int currentRow = fListWidget.currentRow();
    if (currentRow != 0) {
        fListWidget.setCurrentRow(currentRow - 1);
    }
}

void SkDebuggerGUI::actionStepForward() {
    int currentRow = fListWidget.currentRow();
    QString curRow = QString::number(currentRow);
    QString curCount = QString::number(fListWidget.count());
    if (currentRow < fListWidget.count() - 1) {
        fListWidget.setCurrentRow(currentRow + 1);
    }
}

void SkDebuggerGUI::loadFile(QListWidgetItem *item) {
    if (fDirectoryWidgetActive) {
        QString fileName;
        fileName.append(fPath);
        fileName.append("/");
        fileName.append(item->text());
        loadPicture(fileName);
    }
}

void SkDebuggerGUI::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "",
            tr("Files (*.*)"));
    fDirectoryWidgetActive = false;
    if (!fileName.isNull()) {
        QFileInfo pathInfo(fileName);
        fPath = pathInfo.path();
        loadPicture(fileName);
        setupDirectoryWidget();
    }
    fDirectoryWidgetActive = true;
}

void SkDebuggerGUI::pauseDrawing(bool isPaused) {
    // Qt uses 0 for unchecked, 1 for partially enabled and 2 for checked.
    if (isPaused) {
        fPause = true;
        fPausedRow = fListWidget.currentRow();
    } else {
        fPause = false;
        fCanvasWidget.drawTo(fListWidget.currentRow());
    }
}

void SkDebuggerGUI::registerListClick(QListWidgetItem *item) {
    if(!fLoading) {
        int currentRow = fListWidget.currentRow();

        if (currentRow != -1) {
            if (!fPause) {
                fCanvasWidget.drawTo(currentRow);
            }
            std::vector<std::string> *cuffInfo = fCanvasWidget.getCurrentCommandInfo(
                    currentRow);

            /* TODO(chudy): Add command type before parameters. Rename v
             * to something more informative. */
            if (cuffInfo) {
                std::vector<std::string>::iterator it;

                QString info;
                info.append("<b>Parameters: </b><br/>");
                for (it = cuffInfo->begin(); it != cuffInfo->end(); ++it) {
                    info.append(QString((*it).c_str()));
                    info.append("<br/>");
                }
                fInspectorWidget.setDetailText(info);
                fInspectorWidget.setDisabled(false);
                fInspectorWidget.setMatrix(fCanvasWidget.getCurrentMatrix());
                fInspectorWidget.setClip(fCanvasWidget.getCurrentClip());
            }
        }

    }
}

void SkDebuggerGUI::selectCommand(int command) {
    if (fPause) {
        fListWidget.setCurrentRow(command);
    }
}

void SkDebuggerGUI::toggleBreakpoint() {
    QListWidgetItem* item = fListWidget.currentItem();
    if (item->checkState() == Qt::Unchecked) {
        item->setCheckState(Qt::Checked);
        item->setData(Qt::DecorationRole,
                QPixmap(":/images/Icons/breakpoint_16x16.png"));
    } else {
        item->setCheckState(Qt::Unchecked);
        item->setData(Qt::DecorationRole,
                QPixmap(":/images/Icons/blank.png"));
    }
}

void SkDebuggerGUI::toggleDirectory() {
    if (fDirectoryWidget.isHidden()) {
        fDirectoryWidget.setHidden(false);
    } else {
        fDirectoryWidget.setHidden(true);
    }
}

void SkDebuggerGUI::toggleFilter(QString string) {
    for (int row = 0; row < fListWidget.count(); row++) {
        QListWidgetItem *item = fListWidget.item(row);
        if (item->text() == string) {
            item->setHidden(false);
        } else {
            item->setHidden(true);
        }
    }
}

void SkDebuggerGUI::setupUi(QMainWindow *SkDebuggerGUI) {
    QIcon windowIcon;
    windowIcon.addFile(QString::fromUtf8(":/images/Icons/skia.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    SkDebuggerGUI->setObjectName(QString::fromUtf8("SkDebuggerGUI"));
    SkDebuggerGUI->resize(1200, 1000);
    SkDebuggerGUI->setWindowIcon(windowIcon);
    SkDebuggerGUI->setWindowTitle("Skia Debugger");

    fActionOpen.setShortcuts(QKeySequence::Open);
    fActionOpen.setText("Open");

    QIcon breakpoint;
    breakpoint.addFile(QString::fromUtf8(":/images/Icons/breakpoint.png"),
            QSize(), QIcon::Normal, QIcon::Off);
    fActionBreakpoint.setShortcut(QKeySequence(tr("Ctrl+B")));
    fActionBreakpoint.setIcon(breakpoint);
    fActionBreakpoint.setText("Breakpoints");

    QIcon cancel;
    cancel.addFile(QString::fromUtf8(":/images/Ico/reload.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionCancel.setIcon(cancel);
    fActionCancel.setText("Clear Filter");

    fActionClearBreakpoints.setShortcut(QKeySequence(tr("Alt+B")));
    fActionClearBreakpoints.setText("Clear Breakpoints");

    fActionClearDeletes.setShortcut(QKeySequence(tr("Alt+X")));
    fActionClearDeletes.setText("Clear Deletes");

    fActionClose.setShortcuts(QKeySequence::Quit);
    fActionClose.setText("Exit");

    fActionCreateBreakpoint.setShortcut(QKeySequence(tr("B")));
    fActionCreateBreakpoint.setText("Set Breakpoint");

    fActionDelete.setShortcut(QKeySequence(tr("X")));
    fActionDelete.setText("Delete Command");

    fActionDirectory.setShortcut(QKeySequence(tr("Ctrl+D")));
    fActionDirectory.setText("Directory");

    QIcon inspector;
    inspector.addFile(QString::fromUtf8(":/images/Ico/inspector.png"),
            QSize(), QIcon::Normal, QIcon::Off);
    fActionInspector.setShortcut(QKeySequence(tr("Ctrl+I")));
    fActionInspector.setIcon(inspector);
    fActionInspector.setText("Inspector");

    QIcon play;
    play.addFile(QString::fromUtf8(":/images/Ico/play.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionPlay.setShortcut(QKeySequence(tr("Ctrl+P")));
    fActionPlay.setIcon(play);
    fActionPlay.setText("Play");

    QIcon pause;
    pause.addFile(QString::fromUtf8(":/images/Ico/pause.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionPause.setShortcut(QKeySequence(tr("Space")));
    fActionPause.setCheckable(true);
    fActionPause.setIcon(pause);
    fActionPause.setText("Pause");

    QIcon rewind;
    rewind.addFile(QString::fromUtf8(":/images/Ico/rewind.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionRewind.setShortcut(QKeySequence(tr("Ctrl+R")));
    fActionRewind.setIcon(rewind);
    fActionRewind.setText("Rewind");

    fActionShowDeletes.setShortcut(QKeySequence(tr("Ctrl+X")));
    fActionShowDeletes.setText("Deleted Commands");

    QIcon stepBack;
    stepBack.addFile(QString::fromUtf8(":/images/Ico/previous.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionStepBack.setShortcut(QKeySequence(tr("[")));
    fActionStepBack.setIcon(stepBack);
    fActionStepBack.setText("Step Back");

    QIcon stepForward;
    stepForward.addFile(QString::fromUtf8(":/images/Ico/next.png"),
            QSize(), QIcon::Normal, QIcon::Off);
    fActionStepForward.setShortcut(QKeySequence(tr("]")));
    fActionStepForward.setIcon(stepForward);
    fActionStepForward.setText("Step Forward");

    fListWidget.setItemDelegate(new SkListWidget(&fListWidget));
    fListWidget.setObjectName(QString::fromUtf8("listWidget"));
    fListWidget.setMaximumWidth(250);

    fFilter.addItem("--Filter By Available Commands--");

    fDirectoryWidget.setMaximumWidth(250);
    fDirectoryWidget.setStyleSheet("QListWidget::Item {padding: 5px;}");

    fCanvasWidget.setSizePolicy(QSizePolicy::Expanding,
            QSizePolicy::Expanding);

    fInspectorWidget.setSizePolicy(QSizePolicy::Expanding,
            QSizePolicy::Expanding);
    fInspectorWidget.setMaximumHeight(300);

    fSettingsWidget.setSizePolicy(QSizePolicy::Expanding,
            QSizePolicy::Expanding);
    fSettingsWidget.setMaximumWidth(250);

    fLeftColumnLayout.setSpacing(6);
    fLeftColumnLayout.addWidget(&fListWidget);
    fLeftColumnLayout.addWidget(&fDirectoryWidget);

    fCanvasAndSettingsLayout.setSpacing(6);
    fCanvasAndSettingsLayout.addWidget(&fCanvasWidget);
    fCanvasAndSettingsLayout.addWidget(&fSettingsWidget);

    fMainAndRightColumnLayout.setSpacing(6);
    fMainAndRightColumnLayout.addLayout(&fCanvasAndSettingsLayout);
    fMainAndRightColumnLayout.addWidget(&fInspectorWidget);

    fContainerLayout.setSpacing(6);
    fContainerLayout.setContentsMargins(11, 11, 11, 11);
    fContainerLayout.addLayout(&fLeftColumnLayout);
    fContainerLayout.addLayout(&fMainAndRightColumnLayout);

    SkDebuggerGUI->setCentralWidget(&fCentralWidget);
    SkDebuggerGUI->setStatusBar(&fStatusBar);

    fToolBar.setIconSize(QSize(32, 32));
    fToolBar.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    SkDebuggerGUI->addToolBar(Qt::TopToolBarArea, &fToolBar);

    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    fToolBar.addAction(&fActionRewind);
    fToolBar.addAction(&fActionStepBack);
    fToolBar.addAction(&fActionPause);
    fToolBar.addAction(&fActionStepForward);
    fToolBar.addAction(&fActionPlay);
    fToolBar.addSeparator();
    fToolBar.addAction(&fActionInspector);
    fToolBar.addSeparator();
    fToolBar.addWidget(spacer);
    fToolBar.addWidget(&fFilter);
    fToolBar.addAction(&fActionCancel);

    // TODO(chudy): Remove static call.
    fDirectoryWidgetActive = false;
    fPath = "/usr/local/google/home/chudy/trunk-git/trunk/skp";
    setupDirectoryWidget();
    fDirectoryWidgetActive = true;

    // Menu Bar
    fMenuFile.setTitle("File");
    fMenuFile.addAction(&fActionOpen);
    fMenuFile.addAction(&fActionClose);

    fMenuEdit.setTitle("Edit");
    fMenuEdit.addAction(&fActionDelete);
    fMenuEdit.addAction(&fActionClearDeletes);
    fMenuEdit.addSeparator();
    fMenuEdit.addAction(&fActionCreateBreakpoint);
    fMenuEdit.addAction(&fActionClearBreakpoints);

    fMenuNavigate.setTitle("Navigate");
    fMenuNavigate.addAction(&fActionRewind);
    fMenuNavigate.addAction(&fActionStepBack);
    fMenuNavigate.addAction(&fActionStepForward);
    fMenuNavigate.addAction(&fActionPlay);
    fMenuNavigate.addAction(&fActionPause);
    fMenuNavigate.addAction(&fActionGoToLine);

    fMenuView.setTitle("View");
    fMenuView.addAction(&fActionBreakpoint);
    fMenuView.addAction(&fActionShowDeletes);

    fMenuWindows.setTitle("Window");
    fMenuWindows.addAction(&fActionInspector);
    fMenuWindows.addAction(&fActionDirectory);

    fActionGoToLine.setText("Go to Line...");
    fActionGoToLine.setDisabled(true);
    fMenuBar.addAction(fMenuFile.menuAction());
    fMenuBar.addAction(fMenuEdit.menuAction());
    fMenuBar.addAction(fMenuView.menuAction());
    fMenuBar.addAction(fMenuNavigate.menuAction());
    fMenuBar.addAction(fMenuWindows.menuAction());

    fPause = false;

    SkDebuggerGUI->setMenuBar(&fMenuBar);
    QMetaObject::connectSlotsByName(SkDebuggerGUI);
}

void SkDebuggerGUI::setupDirectoryWidget() {
    QDir dir(fPath);
    QRegExp r(".skp");
    fDirectoryWidget.clear();
    const QStringList files = dir.entryList();
    foreach (QString f, files) {
        if (f.contains(r))
            fDirectoryWidget.addItem(f);
    }
}

void SkDebuggerGUI::loadPicture(QString fileName) {
    fLoading = true;
    fCanvasWidget.loadPicture(fileName);
    std::vector<std::string> *cv = fCanvasWidget.getDrawCommands();
    /* fDebugCanvas is reinitialized every load picture. Need it to retain value
     * of the visibility filter. */
    fCanvasWidget.toggleCurrentCommandFilter(
            fSettingsWidget.getVisibilityButton()->isChecked());
    setupListWidget(cv);
    setupComboBox(cv);
    fInspectorWidget.setDisabled(false);
    fSettingsWidget.setDisabled(false);
    fMenuEdit.setDisabled(false);
    fMenuNavigate.setDisabled(false);
    fMenuView.setDisabled(false);
    fLoading = false;
    actionPlay();
}

void SkDebuggerGUI::setupListWidget(std::vector<std::string>* cv) {
    fListWidget.clear();
    int counter = 0;
    for (unsigned int i = 0; i < cv->size(); i++) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setData(Qt::DisplayRole, (*cv)[i].c_str());
        item->setData(Qt::UserRole + 1, counter++);
        fListWidget.addItem(item);
    }
}

void SkDebuggerGUI::setupComboBox(std::vector<std::string>* cv) {
    fFilter.clear();
    fFilter.addItem("--Filter By Available Commands--");

    std::map<std::string, int> map;
    for (unsigned int i = 0; i < cv->size(); i++) {
        map[(*cv)[i]]++;
    }

    QString overview;
    int counter;
    for (std::map<std::string, int>::iterator it = map.begin(); it != map.end();
            ++it) {
        overview.append((it->first).c_str());
        overview.append(": ");
        overview.append(QString::number(it->second));
        overview.append("<br/>");
        counter += it->second;
        fFilter.addItem((it->first).c_str());
    }
    QString total;
    total.append("Total Draw Commands: ");
    total.append(QString::number(counter));
    total.append("<br/>");
    overview.insert(0, total);

    overview.append("<br/>");
    overview.append("SkBitmap Width: ");
    // NOTE(chudy): This is where we can pull out the SkPictures width.
    overview.append(QString::number(fCanvasWidget.getBitmapWidth()));
    overview.append("px<br/>");
    overview.append("SkBitmap Height: ");
    overview.append(QString::number(fCanvasWidget.getBitmapHeight()));
    overview.append("px");
    fInspectorWidget.setOverviewText(overview);

    // NOTE(chudy): Makes first item unselectable.
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(
            fFilter.model());
    QModelIndex firstIndex = model->index(0, fFilter.modelColumn(),
            fFilter.rootModelIndex());
    QStandardItem* firstItem = model->itemFromIndex(firstIndex);
    firstItem->setSelectable(false);
}
