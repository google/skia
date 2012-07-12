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
    , fActionClose(this)
    , fActionDelete(this)
    , fActionDirectory(this)
    , fActionGoToLine(this)
    , fActionInspector(this)
    , fActionPlay(this)
    , fActionReload(this)
    , fActionRewind(this)
    , fActionSettings(this)
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
{
    setupUi(this);
    connect(&fListWidget, SIGNAL(currentItemChanged(QListWidgetItem*,
                    QListWidgetItem*)), this,
            SLOT(registerListClick(QListWidgetItem *)));
    connect(&fActionOpen, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(&fActionDirectory, SIGNAL(triggered()), this,
            SLOT(toggleDirectory()));
    connect(&fDirectoryWidget, SIGNAL(currentItemChanged(QListWidgetItem*,
                    QListWidgetItem*)), this,
            SLOT(loadFile(QListWidgetItem *)));
    connect(&fActionDelete, SIGNAL(triggered()), this, SLOT(actionDelete()));
    connect(&fActionReload, SIGNAL(triggered()), this, SLOT(actionReload()));
    connect(&fListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this,
            SLOT(toggleBreakpoint()));
    connect(&fActionRewind, SIGNAL(triggered()), this, SLOT(actionRewind()));
    connect(&fActionPlay, SIGNAL(triggered()), this, SLOT(actionPlay()));
    connect(&fActionStepBack, SIGNAL(triggered()), this, SLOT(actionStepBack()));
    connect(&fActionStepForward, SIGNAL(triggered()), this,
            SLOT(actionStepForward()));
    connect(&fActionBreakpoint, SIGNAL(triggered()), this,
            SLOT(actionBreakpoints()));
    connect(&fActionInspector, SIGNAL(triggered()), this,
            SLOT(actionInspector()));
    connect(&fFilter, SIGNAL(activated(QString)), this,
            SLOT(toggleFilter(QString)));
    connect(&fActionCancel, SIGNAL(triggered()), this, SLOT(actionCancel()));
    connect(&fActionClose, SIGNAL(triggered()), this, SLOT(actionClose()));
    connect(&fActionSettings, SIGNAL(triggered()), this, SLOT(actionSettings()));
    connect(fSettingsWidget.getVisibilityButton(), SIGNAL(toggled(bool)), this,
            SLOT(actionCommandFilter()));
    connect(&fCanvasWidget, SIGNAL(scaleFactorChanged(float)), this,
            SLOT(actionScale(float)));
    connect(fSettingsWidget.getCommandCheckBox(), SIGNAL(toggled(bool)),
            this, SLOT(pauseDrawing(bool)));
    connect(&fCanvasWidget, SIGNAL(commandChanged(int)), &fSettingsWidget,
            SLOT(updateCommand(int)));
    connect(&fCanvasWidget, SIGNAL(hitChanged(int)), this, SLOT(selectCommand(int)));
    connect(&fCanvasWidget, SIGNAL(hitChanged(int)), &fSettingsWidget,
            SLOT(updateHit(int)));
}

SkDebuggerGUI::~SkDebuggerGUI() {
}

void SkDebuggerGUI::actionBreakpoints() {
    if (!fBreakpointsActivated) {
        fBreakpointsActivated = true;
    } else {
        fBreakpointsActivated = false;
    }

    for (int row = 0; row < fListWidget.count(); row++) {
        QListWidgetItem *item = fListWidget.item(row);

        if (item->checkState() == Qt::Unchecked && fBreakpointsActivated) {
            item->setHidden(true);
        } else {
            item->setHidden(false);
        }
    }
}

void SkDebuggerGUI::actionCancel() {
    for (int row = 0; row < fListWidget.count(); row++) {
        fListWidget.item(row)->setHidden(false);
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
    QListWidgetItem* item = fListWidget.currentItem();
    if (item->data(Qt::UserRole + 2) == true) {
        item->setData(Qt::UserRole + 2, false);
        item->setData(Qt::UserRole + 3, QPixmap(":/images/Icons/delete.png"));
    } else {
        item->setData(Qt::UserRole + 2, true);
        item->setData(Qt::UserRole + 3, QPixmap(":/images/Icons/blank.png"));
    }
    int currentRow = fListWidget.currentRow();
    // NOTE(chudy): Forces a redraw up to current selected command.
    fCanvasWidget.toggleCommand(currentRow);
    fCanvasWidget.drawTo(fPausedRow);
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

void SkDebuggerGUI::actionReload() {
    for (int row = 0; row < fListWidget.count(); row++) {
        QListWidgetItem* item = fListWidget.item(row);
        item->setData(Qt::UserRole + 2, true);
        item->setData(Qt::DecorationRole, QPixmap(":/images/Icons/blank.png"));
        fCanvasWidget.toggleCommand(row, true);
    }
    fCanvasWidget.drawTo(fListWidget.currentRow());
}

void SkDebuggerGUI::actionRewind() {
    /* NOTE(chudy): Hack. All skps opened so far start with save and concat
     * commands that don't clear or reset the canvas. */
    fListWidget.setCurrentRow(2);
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
    int currentRow = fListWidget.currentRow();
    if (!fPause) {
        fCanvasWidget.drawTo(currentRow);
    }
    std::vector<std::string> *v = fCanvasWidget.getCurrentCommandInfo(
            currentRow);

    /* TODO(chudy): Add command type before parameters. Rename v
     * to something more informative. */
    if (v) {
        std::vector<std::string>::iterator it;

        QString info;
        info.append("<b>Parameters: </b><br/>");
        for (it = v->begin(); it != v->end(); ++it) {
            info.append(QString((*it).c_str()));
            info.append("<br/>");
        }
        fInspectorWidget.setDetailText(info);
        fInspectorWidget.setDisabled(false);
        fInspectorWidget.setMatrix(fCanvasWidget.getCurrentMatrix());
        fInspectorWidget.setClip(fCanvasWidget.getCurrentClip());
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

    QIcon open;
    open.addFile(QString::fromUtf8(":/images/Icons/package-br32.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionOpen.setIcon(open);
    fActionOpen.setText("Open");

    QIcon breakpoint;
    breakpoint.addFile(QString::fromUtf8(":/images/Icons/breakpoint.png"),
            QSize(), QIcon::Normal, QIcon::Off);
    fActionBreakpoint.setIcon(breakpoint);
    fActionBreakpoint.setText("Show Breakpoints");

    QIcon cancel;
    cancel.addFile(QString::fromUtf8(":/images/Icons/reset.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionCancel.setIcon(cancel);
    fActionCancel.setText("Clear Filter");

    fActionClose.setText("Exit");

    QIcon deleteIcon;
    deleteIcon.addFile(QString::fromUtf8(":/images/Icons/delete.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionDelete.setIcon(deleteIcon);
    fActionDelete.setText("Delete Command");

    QIcon directory;
    directory.addFile(QString::fromUtf8(":/images/Icons/drawer-open-icon.png"),
            QSize(), QIcon::Normal, QIcon::Off);
    fActionDirectory.setIcon(directory);
    fActionDirectory.setText("Toggle Directory");

    QIcon inspector;
    inspector.addFile(QString::fromUtf8(":/images/Icons/inspector.png"),
            QSize(), QIcon::Normal, QIcon::Off);
    fActionInspector.setIcon(inspector);
    fActionInspector.setText("Toggle Inspector");

    QIcon play;
    play.addFile(QString::fromUtf8(":/images/Icons/play.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionPlay.setIcon(play);
    fActionPlay.setText("Play");

    QIcon reload;
    reload.addFile(QString::fromUtf8(":/images/Icons/reload.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionReload.setIcon(reload);
    fActionReload.setText("Reset Picture");

    QIcon rewind;
    rewind.addFile(QString::fromUtf8(":/images/Icons/rewind.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionRewind.setIcon(rewind);
    fActionRewind.setText("Rewind");

    QIcon settings;
    settings.addFile(QString::fromUtf8(":/images/Icons/settings.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionSettings.setIcon(settings);
    fActionSettings.setText("Settings");

    QIcon stepBack;
    stepBack.addFile(QString::fromUtf8(":/images/Icons/back.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionStepBack.setIcon(stepBack);
    fActionStepBack.setText("Step Back");

    QIcon stepForward;
    stepForward.addFile(QString::fromUtf8(":/images/Icons/go-next.png"),
            QSize(), QIcon::Normal, QIcon::Off);
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
    fSettingsWidget.setHidden(true);

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

    fToolBar.setIconSize(QSize(24, 24));
    fToolBar.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    SkDebuggerGUI->addToolBar(Qt::TopToolBarArea, &fToolBar);

    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    fToolBar.addAction(&fActionRewind);
    fToolBar.addAction(&fActionStepBack);
    fToolBar.addAction(&fActionStepForward);
    fToolBar.addAction(&fActionPlay);
    fToolBar.addSeparator();
    fToolBar.addAction(&fActionBreakpoint);
    fToolBar.addSeparator();
    fToolBar.addAction(&fActionDelete);
    fToolBar.addAction(&fActionReload);
    fToolBar.addSeparator();
    fToolBar.addAction(&fActionSettings);
    fToolBar.addWidget(spacer);
    fToolBar.addWidget(&fFilter);
    fToolBar.addAction(&fActionCancel);

    // TODO(chudy): Remove static call.
    fDirectoryWidgetActive = false;
    fPath = "/usr/local/google/home/chudy/trunk-linux/debugger/skp";
    setupDirectoryWidget();
    fDirectoryWidgetActive = true;

    // Menu Bar
    fMenuFile.setTitle("File");
    fMenuFile.addAction(&fActionOpen);
    fMenuFile.addAction(&fActionClose);
    fMenuNavigate.setTitle("Navigate");
    fMenuNavigate.addAction(&fActionGoToLine);
    fMenuView.setTitle("View");
    fMenuView.addAction(&fActionInspector);
    fMenuView.addAction(&fActionDirectory);

    fActionGoToLine.setText("Go to Line...");
    fActionGoToLine.setDisabled(true);
    fMenuBar.addAction(fMenuFile.menuAction());
    fMenuBar.addAction(fMenuView.menuAction());
    fMenuBar.addAction(fMenuNavigate.menuAction());

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
    fCanvasWidget.loadPicture(fileName);
    std::vector<std::string> *cv = fCanvasWidget.getDrawCommands();
    /* fDebugCanvas is reinitialized every load picture. Need it to retain value
     * of the visibility filter. */
    fCanvasWidget.toggleCurrentCommandFilter(
            fSettingsWidget.getVisibilityButton()->isChecked());
    setupListWidget(cv);
    setupComboBox(cv);
    fSettingsWidget.setDisabled(false);
}

void SkDebuggerGUI::setupListWidget(std::vector<std::string>* cv) {
    fListWidget.clear();
    int counter = 0;
    for (unsigned int i = 0; i < cv->size(); i++) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setData(Qt::DisplayRole, (*cv)[i].c_str());
        item->setData(Qt::UserRole + 1, counter++);
        item->setData(Qt::UserRole + 2, true);
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
