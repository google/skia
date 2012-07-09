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
        QMainWindow(parent) {

    setupUi(this);
    connect(fListWidget, SIGNAL(currentItemChanged(QListWidgetItem*,
                    QListWidgetItem*)), this,
            SLOT(registerListClick(QListWidgetItem *)));
    connect(fActionOpen, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(fActionDirectory, SIGNAL(triggered()), this,
            SLOT(toggleDirectory()));
    connect(fDirectoryWidget, SIGNAL(currentItemChanged(QListWidgetItem*,
                    QListWidgetItem*)), this,
            SLOT(loadFile(QListWidgetItem *)));
    connect(fActionDelete, SIGNAL(triggered()), this, SLOT(actionDelete()));
    connect(fActionReload, SIGNAL(triggered()), this, SLOT(actionReload()));
    connect(fListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this,
            SLOT(toggleBreakpoint()));
    connect(fActionRewind, SIGNAL(triggered()), this, SLOT(actionRewind()));
    connect(fActionPlay, SIGNAL(triggered()), this, SLOT(actionPlay()));
    connect(fActionStepBack, SIGNAL(triggered()), this, SLOT(actionStepBack()));
    connect(fActionStepForward, SIGNAL(triggered()), this,
            SLOT(actionStepForward()));
    connect(fActionBreakpoint, SIGNAL(triggered()), this,
            SLOT(actionBreakpoints()));
    connect(fActionInspector, SIGNAL(triggered()), this,
            SLOT(actionInspector()));
    connect(fFilter, SIGNAL(activated(QString)), this,
            SLOT(toggleFilter(QString)));
    connect(fActionCancel, SIGNAL(triggered()), this, SLOT(actionCancel()));
    connect(fActionClose, SIGNAL(triggered()), this, SLOT(actionClose()));
    connect(fActionSettings, SIGNAL(triggered()), this, SLOT(actionSettings()));
    connect(fSettingsWidget->getVisibilityButton(), SIGNAL(toggled(bool)), this,
            SLOT(actionCommandFilter()));
    connect(fCanvasWidget, SIGNAL(scaleFactorChanged(float)), this,
            SLOT(actionScale(float)));
    connect(fSettingsWidget->getCommandCheckBox(), SIGNAL(stateChanged(int)),
            this, SLOT(pauseDrawing(int)));
    connect(fCanvasWidget, SIGNAL(commandChanged(int)), fSettingsWidget,
            SLOT(updateCommand(int)));
}

SkDebuggerGUI::~SkDebuggerGUI() {
}

void SkDebuggerGUI::actionBreakpoints() {
    if (!fBreakpointsActivated) {
        fBreakpointsActivated = true;
    } else {
        fBreakpointsActivated = false;
    }

    for (int row = 0; row < fListWidget->count(); row++) {
        QListWidgetItem *item = fListWidget->item(row);

        if (item->checkState() == Qt::Unchecked && fBreakpointsActivated) {
            item->setHidden(true);
        } else {
            item->setHidden(false);
        }
    }
}

void SkDebuggerGUI::actionCancel() {
    for (int row = 0; row < fListWidget->count(); row++) {
        fListWidget->item(row)->setHidden(false);
    }
}

void SkDebuggerGUI::actionCommandFilter() {
    fCanvasWidget->toggleCurrentCommandFilter(fSettingsWidget->getVisibilityButton()->isChecked());
    fCanvasWidget->drawTo(fListWidget->currentRow());
}

void SkDebuggerGUI::actionClose() {
    this->close();
}

void SkDebuggerGUI::actionDelete() {
    QListWidgetItem* item = fListWidget->currentItem();
    if (item->data(Qt::UserRole + 2) == true) {
        item->setData(Qt::UserRole + 2, false);
        item->setData(Qt::DecorationRole, QPixmap(":/images/Icons/delete.png"));
    } else {
        item->setData(Qt::UserRole + 2, true);
        if (item->checkState() == Qt::Unchecked) {
            item->setData(Qt::DecorationRole,
                    QPixmap(":/images/Icons/blank.png"));
        } else {
            item->setData(Qt::DecorationRole,
                    QPixmap(":/images/Icons/breakpoint_16x16.png"));
        }
    }
    int currentRow = fListWidget->currentRow();
    // NOTE(chudy): Forces a redraw up to current selected command.
    if (fCanvasWidget) {
        fCanvasWidget->toggleCommand(currentRow);
        fCanvasWidget->drawTo(currentRow);
    }
}

void SkDebuggerGUI::actionInspector() {
    if (fInspectorWidget->isHidden()) {
        fInspectorWidget->setHidden(false);
    } else {
        fInspectorWidget->setHidden(true);
    }
}

void SkDebuggerGUI::actionPlay() {
    for (int row = fListWidget->currentRow() + 1; row < fListWidget->count();
            row++) {
        QListWidgetItem *item = fListWidget->item(row);
        if (item->checkState() == Qt::Checked) {
            fListWidget->setCurrentItem(item);
            return;
        }
    }
    fListWidget->setCurrentRow(fListWidget->count() - 1);
}

void SkDebuggerGUI::actionReload() {
    for (int row = 0; row < fListWidget->count(); row++) {
        QListWidgetItem* item = fListWidget->item(row);
        item->setData(Qt::UserRole + 2, true);
        item->setData(Qt::DecorationRole, QPixmap(":/images/Icons/blank.png"));
        fCanvasWidget->toggleCommand(row, true);
    }
    fCanvasWidget->drawTo(fListWidget->currentRow());
}

void SkDebuggerGUI::actionRewind() {
    /* NOTE(chudy): Hack. All skps opened so far start with save and concat
     * commands that don't clear or reset the canvas. */
    fListWidget->setCurrentRow(2);
}

void SkDebuggerGUI::actionScale(float scaleFactor) {
    fSettingsWidget->setZoomText(scaleFactor);
}

void SkDebuggerGUI::actionSettings() {
    if (fSettingsWidget->isHidden()) {
        fSettingsWidget->setHidden(false);
    } else {
        fSettingsWidget->setHidden(true);
    }
}

void SkDebuggerGUI::actionStepBack() {
    int currentRow = fListWidget->currentRow();
    if (currentRow != 0) {
        fListWidget->setCurrentRow(currentRow - 1);
    }
}

void SkDebuggerGUI::actionStepForward() {
    int currentRow = fListWidget->currentRow();
    QString curRow = QString::number(currentRow);
    QString curCount = QString::number(fListWidget->count());
    if (currentRow < fListWidget->count() - 1) {
        fListWidget->setCurrentRow(currentRow + 1);
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

void SkDebuggerGUI::pauseDrawing(int state) {
    // Qt uses 0 for unchecked, 1 for partially enabled and 2 for checked.
    if (state == 2) {
        fPause = true;
    } else {
        fPause = false;
        fCanvasWidget->drawTo(fListWidget->currentRow());
    }
}

void SkDebuggerGUI::registerListClick(QListWidgetItem *item) {
    int currentRow = fListWidget->currentRow();
    if (!fPause) {
        fCanvasWidget->drawTo(currentRow);
    }
    std::vector<std::string> *v = fCanvasWidget->getCurrentCommandInfo(
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
        fInspectorWidget->setDetailText(info);
        fInspectorWidget->setDisabled(false);
        fInspectorWidget->setMatrix(fCanvasWidget->getCurrentMatrix());
        fInspectorWidget->setClip(fCanvasWidget->getCurrentClip());
    }
}

void SkDebuggerGUI::toggleBreakpoint() {
    QListWidgetItem* item = fListWidget->currentItem();
    if (item->checkState() == Qt::Unchecked) {
        item->setCheckState(Qt::Checked);

        /* NOTE(chudy): If the command is toggled as hidden that takes
         * precendence over the breakpoint icon.
         */
        if (item->data(Qt::UserRole + 2) == false) {
            item->setData(Qt::DecorationRole,
                    QPixmap(":/images/Icons/delete.png"));
        } else {
            item->setData(Qt::DecorationRole,
                    QPixmap(":/images/Icons/breakpoint_16x16.png"));
        }
    } else {

        /* NOTE(chudy): When untoggling as a breakpoint if the command
         * is hidden then the portraying icon should remain the delete icon.
         */
        item->setCheckState(Qt::Unchecked);

        if (item->data(Qt::UserRole + 2) == false) {
            item->setData(Qt::DecorationRole,
                    QPixmap(":/images/Icons/delete.png"));
        } else {
            item->setData(Qt::DecorationRole,
                    QPixmap(":/images/Icons/blank.png"));
        }
    }
}

void SkDebuggerGUI::toggleDirectory() {
    if (fDirectoryWidget->isHidden()) {
        fDirectoryWidget->setHidden(false);
    } else {
        fDirectoryWidget->setHidden(true);
    }
}

void SkDebuggerGUI::toggleFilter(QString string) {
    for (int row = 0; row < fListWidget->count(); row++) {
        QListWidgetItem *item = fListWidget->item(row);
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

    QIcon open;
    open.addFile(QString::fromUtf8(":/images/Icons/package-br32.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionOpen = new QAction(SkDebuggerGUI);
    fActionOpen->setObjectName(QString::fromUtf8("actionOpen"));
    fActionOpen->setIcon(open);

    QIcon directory;
    directory.addFile(QString::fromUtf8(":/images/Icons/drawer-open-icon.png"),
            QSize(), QIcon::Normal, QIcon::Off);
    fActionDirectory = new QAction(SkDebuggerGUI);
    fActionDirectory->setObjectName(QString::fromUtf8("actionDirectory"));
    fActionDirectory->setIcon(directory);
    fActionDirectory->setText("Toggle Directory");

    QIcon rewind;
    rewind.addFile(QString::fromUtf8(":/images/Icons/rewind.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionRewind = new QAction(SkDebuggerGUI);
    fActionRewind->setObjectName(QString::fromUtf8("actionRewind"));
    fActionRewind->setIcon(rewind);
    fActionRewind->setText("Rewind");

    QIcon stepBack;
    stepBack.addFile(QString::fromUtf8(":/images/Icons/back.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionStepBack = new QAction(SkDebuggerGUI);
    fActionStepBack->setObjectName(QString::fromUtf8("actionStepBack"));
    fActionStepBack->setIcon(stepBack);
    fActionStepBack->setText("Step Back");

    QIcon stepForward;
    stepForward.addFile(QString::fromUtf8(":/images/Icons/go-next.png"),
            QSize(), QIcon::Normal, QIcon::Off);
    fActionStepForward = new QAction(SkDebuggerGUI);
    fActionStepForward->setObjectName(QString::fromUtf8("actionStepBack"));
    fActionStepForward->setIcon(stepForward);
    fActionStepForward->setText("Step Forward");

    QIcon play;
    play.addFile(QString::fromUtf8(":/images/Icons/play.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionPlay = new QAction(SkDebuggerGUI);
    fActionPlay->setObjectName(QString::fromUtf8("actionPlay"));
    fActionPlay->setIcon(play);
    fActionPlay->setText("Play");

    QIcon breakpoint;
    breakpoint.addFile(QString::fromUtf8(":/images/Icons/breakpoint.png"),
            QSize(), QIcon::Normal, QIcon::Off);
    fActionBreakpoint = new QAction(SkDebuggerGUI);
    fActionBreakpoint->setObjectName(QString::fromUtf8("actionBreakpoint"));
    fActionBreakpoint->setIcon(breakpoint);
    fActionBreakpoint->setText("Show Breakpoints");

    QIcon inspector;
    inspector.addFile(QString::fromUtf8(":/images/Icons/inspector.png"),
            QSize(), QIcon::Normal, QIcon::Off);
    fActionInspector = new QAction(SkDebuggerGUI);
    fActionInspector->setObjectName(QString::fromUtf8("actionInspector"));
    fActionInspector->setIcon(inspector);
    fActionInspector->setText("Inspector");

    QIcon deleteIcon;
    deleteIcon.addFile(QString::fromUtf8(":/images/Icons/delete.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionDelete = new QAction(SkDebuggerGUI);
    fActionDelete->setObjectName(QString::fromUtf8("actionDelete"));
    fActionDelete->setIcon(deleteIcon);
    fActionDelete->setText("Delete Command");

    QIcon reload;
    reload.addFile(QString::fromUtf8(":/images/Icons/reload.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionReload = new QAction(SkDebuggerGUI);
    fActionReload->setObjectName(QString::fromUtf8("actionReload"));
    fActionReload->setIcon(reload);
    fActionReload->setText("Reset Picture");

    QIcon settings;
    settings.addFile(QString::fromUtf8(":/images/Icons/settings.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionSettings = new QAction(SkDebuggerGUI);
    fActionSettings->setObjectName(QString::fromUtf8("actionSettings"));
    fActionSettings->setIcon(settings);
    fActionSettings->setText("Settings");

    QIcon cancel;
    cancel.addFile(QString::fromUtf8(":/images/Icons/reset.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionCancel = new QAction(SkDebuggerGUI);
    fActionCancel->setObjectName(QString::fromUtf8("actionCancel"));
    fActionCancel->setIcon(cancel);
    fActionCancel->setText("Clear Filter");

    fCentralWidget = new QWidget(SkDebuggerGUI);
    fCentralWidget->setObjectName(QString::fromUtf8("centralWidget"));

    fHorizontalLayout = new QHBoxLayout(fCentralWidget);
    fHorizontalLayout->setSpacing(6);
    fHorizontalLayout->setContentsMargins(11, 11, 11, 11);
    fHorizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));

    fVerticalLayout = new QVBoxLayout();
    fVerticalLayout->setSpacing(6);
    fVerticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));

    fVerticalLayout_2 = new QVBoxLayout();
    fVerticalLayout_2->setSpacing(6);
    fVerticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));

    fListWidget = new QListWidget(fCentralWidget);
    fListWidget->setItemDelegate(new SkListWidget(fListWidget));
    fListWidget->setObjectName(QString::fromUtf8("listWidget"));
    fListWidget->setMaximumWidth(250);

    fInspectorWidget = new SkInspectorWidget();
    fInspectorWidget->setObjectName(QString::fromUtf8("inspectorWidget"));
    fInspectorWidget->setSizePolicy(QSizePolicy::Expanding,
            QSizePolicy::Expanding);
    fInspectorWidget->setMaximumHeight(300);

    fFilter = new QComboBox(fCentralWidget);
    fFilter->setObjectName(QString::fromUtf8("comboBox"));
    fFilter->addItem("--Filter By Available Commands--");

    fDirectoryWidget = new QListWidget(fCentralWidget);
    fDirectoryWidget->setObjectName(QString::fromUtf8("listWidget_2"));
    fDirectoryWidget->setMaximumWidth(250);
    fDirectoryWidget->setStyleSheet("QListWidget::Item {padding: 5px;}");

    fVerticalLayout_2->addWidget(fListWidget);
    fVerticalLayout_2->addWidget(fDirectoryWidget);

    fCanvasWidget = new SkCanvasWidget(fCentralWidget);
    fCanvasWidget->setSizePolicy(QSizePolicy::Expanding,
            QSizePolicy::Expanding);

    fSettingsWidget = new SkSettingsWidget(fCentralWidget);
    fSettingsWidget->setSizePolicy(QSizePolicy::Expanding,
            QSizePolicy::Expanding);
    fSettingsWidget->setMaximumWidth(250);
    fSettingsWidget->setHidden(true);

    fHorizontalLayout_2 = new QHBoxLayout();
    fHorizontalLayout_2->setSpacing(6);

    fHorizontalLayout_2->addWidget(fCanvasWidget);
    fHorizontalLayout_2->addWidget(fSettingsWidget);

    fVerticalLayout->addLayout(fHorizontalLayout_2);
    fVerticalLayout->addWidget(fInspectorWidget);

    fHorizontalLayout->addLayout(fVerticalLayout_2);
    fHorizontalLayout->addLayout(fVerticalLayout);

    SkDebuggerGUI->setCentralWidget(fCentralWidget);
    fStatusBar = new QStatusBar(SkDebuggerGUI);
    fStatusBar->setObjectName(QString::fromUtf8("statusBar"));
    SkDebuggerGUI->setStatusBar(fStatusBar);
    fToolBar = new QToolBar(SkDebuggerGUI);
    fToolBar->setObjectName(QString::fromUtf8("toolBar"));
    fToolBar->setIconSize(QSize(24, 24));
    //fToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    SkDebuggerGUI->addToolBar(Qt::TopToolBarArea, fToolBar);

    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    fToolBar->addAction(fActionOpen);
    fToolBar->addSeparator();
    fToolBar->addAction(fActionDirectory);
    fToolBar->addSeparator();
    fToolBar->addAction(fActionRewind);
    fToolBar->addAction(fActionStepBack);
    fToolBar->addAction(fActionStepForward);
    fToolBar->addAction(fActionPlay);
    fToolBar->addSeparator();
    fToolBar->addAction(fActionBreakpoint);
    fToolBar->addAction(fActionInspector);
    fToolBar->addSeparator();
    fToolBar->addAction(fActionDelete);
    fToolBar->addAction(fActionReload);
    fToolBar->addSeparator();
    fToolBar->addAction(fActionSettings);
    fToolBar->addWidget(spacer);
    fToolBar->addWidget(fFilter);
    fToolBar->addAction(fActionCancel);

    // TODO(chudy): Remove static call.
    fDirectoryWidgetActive = false;
    fPath = "/usr/local/google/home/chudy/trunk-linux/debugger/skp";
    setupDirectoryWidget();
    fDirectoryWidgetActive = true;

    fMenuBar = new QMenuBar(SkDebuggerGUI);

    // File
    fMenuFile = new QMenu(SkDebuggerGUI);
    fMenuFile->setTitle("File");

    fActionClose = new QAction(SkDebuggerGUI);
    fActionClose->setText("Close");

    fMenuFile->addAction(fActionOpen);
    fMenuFile->addAction(fActionClose);

    // Navigate
    fMenuNavigate = new QMenu(SkDebuggerGUI);
    fMenuNavigate->setTitle("Navigate");

    fActionGoToLine = new QAction(SkDebuggerGUI);
    fActionGoToLine->setText("Go to Line...");
    fActionGoToLine->setDisabled(true);

    fMenuNavigate->addAction(fActionGoToLine);

    // Menu Bar
    fMenuBar->addAction(fMenuFile->menuAction());
    fMenuBar->addAction(fMenuNavigate->menuAction());

    fPause = false;

    SkDebuggerGUI->setMenuBar(fMenuBar);

    retranslateUi(SkDebuggerGUI);
    QMetaObject::connectSlotsByName(SkDebuggerGUI);
}

void SkDebuggerGUI::setupDirectoryWidget() {
    fDir = new QDir(fPath);
    QRegExp r(".skp");
    fDirectoryWidget->clear();
    const QStringList files = fDir->entryList();
    foreach (QString f, files) {
        if (f.contains(r))
            fDirectoryWidget->addItem(f);
    }
}

// TODO(chudy): Is this necessary?
void SkDebuggerGUI::retranslateUi(QMainWindow *SkDebuggerGUI) {
    SkDebuggerGUI->setWindowTitle(
            QApplication::translate("SkDebuggerGUI", "SkDebuggerGUI", 0,
                    QApplication::UnicodeUTF8));
    fActionOpen->setText(
            QApplication::translate("SkDebuggerGUI", "Open", 0,
                    QApplication::UnicodeUTF8));
    fToolBar->setWindowTitle(
            QApplication::translate("SkDebuggerGUI", "toolBar", 0,
                    QApplication::UnicodeUTF8));
}

void SkDebuggerGUI::loadPicture(QString fileName) {
    fCanvasWidget->loadPicture(fileName);
    std::vector<std::string> *cv = fCanvasWidget->getDrawCommands();
    /* fDebugCanvas is reinitialized every load picture. Need it to retain value
     * of the visibility filter. */
    actionCommandFilter();

    fCanvasWidget->toggleCurrentCommandFilter(fSettingsWidget->getVisibilityButton()->isChecked());




    setupListWidget(cv);
    setupComboBox(cv);
}

void SkDebuggerGUI::setupListWidget(std::vector<std::string>* cv) {
    fListWidget->clear();
    int counter = 0;
    for (unsigned int i = 0; i < cv->size(); i++) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setData(Qt::DisplayRole, (*cv)[i].c_str());
        item->setData(Qt::UserRole + 1, counter++);
        item->setData(Qt::UserRole + 2, true);
        fListWidget->addItem(item);
    }
}

void SkDebuggerGUI::setupComboBox(std::vector<std::string>* cv) {
    fFilter->clear();
    fFilter->addItem("--Filter By Available Commands--");

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
        fFilter->addItem((it->first).c_str());
    }
    QString total;
    total.append("Total Draw Commands: ");
    total.append(QString::number(counter));
    total.append("<br/>");
    overview.insert(0, total);

    overview.append("<br/>");
    overview.append("SkBitmap Width: ");
    // NOTE(chudy): This is where we can pull out the SkPictures width.
    overview.append(QString::number(fCanvasWidget->getBitmapWidth()));
    overview.append("px<br/>");
    overview.append("SkBitmap Height: ");
    overview.append(QString::number(fCanvasWidget->getBitmapHeight()));
    overview.append("px");
    fInspectorWidget->setOverviewText(overview);

    // NOTE(chudy): Makes first item unselectable.
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(
            fFilter->model());
    QModelIndex firstIndex = model->index(0, fFilter->modelColumn(),
            fFilter->rootModelIndex());
    QStandardItem* firstItem = model->itemFromIndex(firstIndex);
    firstItem->setSelectable(false);
}
