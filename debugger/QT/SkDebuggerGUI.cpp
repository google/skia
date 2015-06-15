/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDebuggerGUI.h"
#include "PictureRenderer.h"
#include "SkPictureData.h"
#include "SkPicturePlayback.h"
#include "SkPictureRecord.h"
#include <QListWidgetItem>
#include <QtGui>
#include "sk_tool_utils.h"

#if defined(SK_BUILD_FOR_WIN32)
    #include "SysTimer_windows.h"
#elif defined(SK_BUILD_FOR_MAC)
    #include "SysTimer_mach.h"
#elif defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_ANDROID)
    #include "SysTimer_posix.h"
#else
    #include "SysTimer_c.h"
#endif


SkDebuggerGUI::SkDebuggerGUI(QWidget *parent) :
        QMainWindow(parent)
    , fCentralSplitter(this)
    , fStatusBar(this)
    , fToolBar(this)
    , fActionOpen(this)
    , fActionBreakpoint(this)
    , fActionProfile(this)
    , fActionCancel(this)
    , fActionClearBreakpoints(this)
    , fActionClearDeletes(this)
    , fActionClose(this)
    , fActionCreateBreakpoint(this)
    , fActionDelete(this)
    , fActionDirectory(this)
    , fActionGoToLine(this)
    , fActionInspector(this)
    , fActionSettings(this)
    , fActionPlay(this)
    , fActionPause(this)
    , fActionRewind(this)
    , fActionSave(this)
    , fActionSaveAs(this)
    , fActionShowDeletes(this)
    , fActionStepBack(this)
    , fActionStepForward(this)
    , fActionZoomIn(this)
    , fActionZoomOut(this)
    , fMapper(this)
    , fListWidget(&fCentralSplitter)
    , fDirectoryWidget(&fCentralSplitter)
    , fCanvasWidget(this, &fDebugger)
    , fDrawCommandGeometryWidget(&fDebugger)
    , fMenuBar(this)
    , fMenuFile(this)
    , fMenuNavigate(this)
    , fMenuView(this)
    , fLoading(false)
{
    setupUi(this);
    fListWidget.setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(&fListWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this,
            SLOT(updateDrawCommandInfo()));
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
    connect(&fActionSettings, SIGNAL(triggered()), this, SLOT(actionSettings()));
    connect(&fFilter, SIGNAL(activated(QString)), this, SLOT(toggleFilter(QString)));
    connect(&fActionProfile, SIGNAL(triggered()), this, SLOT(actionProfile()));
    connect(&fActionCancel, SIGNAL(triggered()), this, SLOT(actionCancel()));
    connect(&fActionClearBreakpoints, SIGNAL(triggered()), this, SLOT(actionClearBreakpoints()));
    connect(&fActionClearDeletes, SIGNAL(triggered()), this, SLOT(actionClearDeletes()));
    connect(&fActionClose, SIGNAL(triggered()), this, SLOT(actionClose()));
#if SK_SUPPORT_GPU
    connect(&fSettingsWidget, SIGNAL(glSettingsChanged()), this, SLOT(actionGLSettingsChanged()));
#endif
    connect(&fSettingsWidget, SIGNAL(rasterSettingsChanged()), this, SLOT(actionRasterSettingsChanged()));
    connect(&fSettingsWidget, SIGNAL(visualizationsChanged()), this, SLOT(actionVisualizationsChanged()));
    connect(&fSettingsWidget, SIGNAL(texFilterSettingsChanged()), this, SLOT(actionTextureFilter()));
    connect(&fActionPause, SIGNAL(toggled(bool)), this, SLOT(pauseDrawing(bool)));
    connect(&fActionCreateBreakpoint, SIGNAL(activated()), this, SLOT(toggleBreakpoint()));
    connect(&fActionShowDeletes, SIGNAL(triggered()), this, SLOT(showDeletes()));
    connect(&fCanvasWidget, SIGNAL(hitChanged(int)), this, SLOT(selectCommand(int)));
    connect(&fCanvasWidget, SIGNAL(hitChanged(int)), this, SLOT(updateHit(int)));
    connect(&fCanvasWidget, SIGNAL(scaleFactorChanged(float)), this, SLOT(actionScale(float)));

    connect(&fActionSaveAs, SIGNAL(triggered()), this, SLOT(actionSaveAs()));
    connect(&fActionSave, SIGNAL(triggered()), this, SLOT(actionSave()));

    fMapper.setMapping(&fActionZoomIn, SkCanvasWidget::kIn_ZoomCommand);
    fMapper.setMapping(&fActionZoomOut, SkCanvasWidget::kOut_ZoomCommand);

    connect(&fActionZoomIn, SIGNAL(triggered()), &fMapper, SLOT(map()));
    connect(&fActionZoomOut, SIGNAL(triggered()), &fMapper, SLOT(map()));
    connect(&fMapper, SIGNAL(mapped(int)), &fCanvasWidget, SLOT(zoom(int)));

    fViewStateFrame.setDisabled(true);
    fInspectorWidget.setDisabled(true);
    fMenuEdit.setDisabled(true);
    fMenuNavigate.setDisabled(true);
    fMenuView.setDisabled(true);
}

void SkDebuggerGUI::actionBreakpoints() {
    bool breakpointsActivated = fActionBreakpoint.isChecked();
    for (int row = 0; row < fListWidget.count(); row++) {
        QListWidgetItem *item = fListWidget.item(row);
        item->setHidden(item->checkState() == Qt::Unchecked && breakpointsActivated);
    }
}

void SkDebuggerGUI::showDeletes() {
    bool deletesActivated = fActionShowDeletes.isChecked();
    for (int row = 0; row < fListWidget.count(); row++) {
        QListWidgetItem *item = fListWidget.item(row);
        item->setHidden(fDebugger.isCommandVisible(row) && deletesActivated);
    }
}
// This is a simplification of PictureBenchmark's run with the addition of
// clearing of the times after the first pass (in resetTimes)
void SkDebuggerGUI::run(const SkPicture* pict,
                        sk_tools::PictureRenderer* renderer,
                        int repeats) {
    SkASSERT(pict);
    if (NULL == pict) {
        return;
    }

    SkASSERT(renderer != NULL);
    if (NULL == renderer) {
        return;
    }

    renderer->init(pict, NULL, NULL, NULL, false, false);

    renderer->setup();
    renderer->render();
    renderer->resetState(true);    // flush, swapBuffers and Finish

    for (int i = 0; i < repeats; ++i) {
        renderer->setup();
        renderer->render();
        renderer->resetState(false);  // flush & swapBuffers, but don't Finish
    }
    renderer->resetState(true);    // flush, swapBuffers and Finish

    renderer->end();
}

void SkDebuggerGUI::actionProfile() {
    // In order to profile we pass the command offsets (that were read-in
    // in loadPicture by the SkOffsetPicture) to an SkTimedPlaybackPicture.
    // The SkTimedPlaybackPicture in turn passes the offsets to an
    // SkTimedPicturePlayback object which uses them to track the performance
    // of individual commands.
    if (fFileName.isEmpty()) {
        return;
    }

    SkFILEStream inputStream;

    inputStream.setPath(fFileName.c_str());
    if (!inputStream.isValid()) {
        return;
    }

    SkAutoTUnref<SkPicture> picture(SkPicture::CreateFromStream(&inputStream,
                                        &SkImageDecoder::DecodeMemory)); // , fSkipCommands));
    if (NULL == picture.get()) {
        return;
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
                QPixmap(":/blank.png"));
    }
}

void SkDebuggerGUI::actionClearDeletes() {
    for (int row = 0; row < fListWidget.count(); row++) {
        QListWidgetItem* item = fListWidget.item(row);
        item->setData(Qt::UserRole + 2, QPixmap(":/blank.png"));
        fDebugger.setCommandVisible(row, true);
        fSkipCommands[row] = false;
    }
    this->updateImage();
}

void SkDebuggerGUI::actionClose() {
    this->close();
}

void SkDebuggerGUI::actionDelete() {

    for (int row = 0; row < fListWidget.count(); ++row) {
        QListWidgetItem* item = fListWidget.item(row);

        if (!item->isSelected()) {
            continue;
        }

        if (fDebugger.isCommandVisible(row)) {
            item->setData(Qt::UserRole + 2, QPixmap(":/delete.png"));
            fDebugger.setCommandVisible(row, false);
            fSkipCommands[row] = true;
        } else {
            item->setData(Qt::UserRole + 2, QPixmap(":/blank.png"));
            fDebugger.setCommandVisible(row, true);
            fSkipCommands[row] = false;
        }
    }

    this->updateImage();
}

#if SK_SUPPORT_GPU
void SkDebuggerGUI::actionGLSettingsChanged() {
    bool isToggled = fSettingsWidget.isGLActive();
    if (isToggled) {
        fCanvasWidget.setGLSampleCount(fSettingsWidget.getGLSampleCount());
    }
    fCanvasWidget.setWidgetVisibility(SkCanvasWidget::kGPU_WidgetType, !isToggled);
}
#endif

void SkDebuggerGUI::actionInspector() {
    bool newState = !fInspectorWidget.isHidden();

    fInspectorWidget.setHidden(newState);
    fViewStateFrame.setHidden(newState);
    fDrawCommandGeometryWidget.setHidden(newState);
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

void SkDebuggerGUI::actionRasterSettingsChanged() {
    fCanvasWidget.setWidgetVisibility(SkCanvasWidget::kRaster_8888_WidgetType,
                                      !fSettingsWidget.isRasterEnabled());
    fDebugger.setOverdrawViz(fSettingsWidget.isOverdrawVizEnabled());
    this->updateImage();
}

void SkDebuggerGUI::actionVisualizationsChanged() {
    fDebugger.setMegaViz(fSettingsWidget.isMegaVizEnabled());
    fDebugger.setPathOps(fSettingsWidget.isPathOpsEnabled());
    fDebugger.highlightCurrentCommand(fSettingsWidget.isVisibilityFilterEnabled());
    this->updateImage();
}

void SkDebuggerGUI::actionTextureFilter() {
    SkFilterQuality quality;
    bool enabled = fSettingsWidget.getFilterOverride(&quality);
    fDebugger.setTexFilterOverride(enabled, quality);
    fCanvasWidget.update();
}

void SkDebuggerGUI::actionRewind() {
    fListWidget.setCurrentRow(0);
}

void SkDebuggerGUI::actionSave() {
    fFileName = fPath.toAscii().data();
    fFileName.append("/");
    fFileName.append(fDirectoryWidget.currentItem()->text().toAscii().data());
    saveToFile(fFileName);
}

void SkDebuggerGUI::actionSaveAs() {
    QString filename = QFileDialog::getSaveFileName(this, "Save File", "",
            "Skia Picture (*skp)");
    if (!filename.endsWith(".skp", Qt::CaseInsensitive)) {
        filename.append(".skp");
    }
    saveToFile(SkString(filename.toAscii().data()));
}

void SkDebuggerGUI::actionScale(float scaleFactor) {
    fZoomBox.setText(QString::number(scaleFactor * 100, 'f', 0).append("%"));
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

void SkDebuggerGUI::drawComplete() {
    SkString clipStack;
    fDebugger.getClipStackText(&clipStack);
    fInspectorWidget.setText(clipStack.c_str(), SkInspectorWidget::kClipStack_TabType);

    fInspectorWidget.setMatrix(fDebugger.getCurrentMatrix());
    fInspectorWidget.setClip(fDebugger.getCurrentClip());
}

void SkDebuggerGUI::saveToFile(const SkString& filename) {
    SkFILEWStream file(filename.c_str());
    SkAutoTUnref<SkPicture> copy(fDebugger.copyPicture());

    sk_tool_utils::PngPixelSerializer serializer;
    copy->serialize(&file, &serializer);
}

void SkDebuggerGUI::loadFile(QListWidgetItem *item) {
    if (fDirectoryWidgetActive) {
        fFileName = fPath.toAscii().data();
        // don't add a '/' to files in the local directory
        if (fFileName.size() > 0) {
            fFileName.append("/");
        }
        fFileName.append(item->text().toAscii().data());
        loadPicture(fFileName);
    }
}

void SkDebuggerGUI::openFile() {
    QString temp = QFileDialog::getOpenFileName(this, tr("Open File"), "",
            tr("Files (*.*)"));
    openFile(temp);
}

void SkDebuggerGUI::openFile(const QString &filename) {
    fDirectoryWidgetActive = false;
    if (!filename.isEmpty()) {
        QFileInfo pathInfo(filename);
        loadPicture(SkString(filename.toAscii().data()));
        setupDirectoryWidget(pathInfo.path());
    }
    fDirectoryWidgetActive = true;
}

void SkDebuggerGUI::pauseDrawing(bool isPaused) {
    fPausedRow = fListWidget.currentRow();
    this->updateDrawCommandInfo();
}

void SkDebuggerGUI::updateDrawCommandInfo() {
    int currentRow = -1;
    if (!fLoading) {
        currentRow = fListWidget.currentRow();
    }
    if (currentRow == -1) {
        fInspectorWidget.setText("", SkInspectorWidget::kDetail_TabType);
        fInspectorWidget.setText("", SkInspectorWidget::kClipStack_TabType);
        fCurrentCommandBox.setText("");
        fDrawCommandGeometryWidget.setDrawCommandIndex(-1);
    } else {
        this->updateImage();

        const SkTDArray<SkString*> *currInfo = fDebugger.getCommandInfo(currentRow);

        /* TODO(chudy): Add command type before parameters. Rename v
         * to something more informative. */
        if (currInfo) {
            QString info;
            info.append("<b>Parameters: </b><br/>");
            for (int i = 0; i < currInfo->count(); i++) {
                info.append(QString((*currInfo)[i]->c_str()));
                info.append("<br/>");
            }
            fInspectorWidget.setText(info, SkInspectorWidget::kDetail_TabType);
        }

        fCurrentCommandBox.setText(QString::number(currentRow));

        fDrawCommandGeometryWidget.setDrawCommandIndex(currentRow);

        fInspectorWidget.setDisabled(false);
        fViewStateFrame.setDisabled(false);
    }
}

void SkDebuggerGUI::selectCommand(int command) {
    if (this->isPaused()) {
        fListWidget.setCurrentRow(command);
    }
}

void SkDebuggerGUI::toggleBreakpoint() {
    QListWidgetItem* item = fListWidget.currentItem();
    if (item->checkState() == Qt::Unchecked) {
        item->setCheckState(Qt::Checked);
        item->setData(Qt::DecorationRole,
                QPixmap(":/breakpoint_16x16.png"));
    } else {
        item->setCheckState(Qt::Unchecked);
        item->setData(Qt::DecorationRole,
                QPixmap(":/blank.png"));
    }
}

void SkDebuggerGUI::toggleDirectory() {
    fDirectoryWidget.setHidden(!fDirectoryWidget.isHidden());
}

void SkDebuggerGUI::toggleFilter(QString string) {
    for (int row = 0; row < fListWidget.count(); row++) {
        QListWidgetItem *item = fListWidget.item(row);
        item->setHidden(item->text() != string);
    }
}

void SkDebuggerGUI::setupUi(QMainWindow *SkDebuggerGUI) {
    QIcon windowIcon;
    windowIcon.addFile(QString::fromUtf8(":/skia.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    SkDebuggerGUI->setObjectName(QString::fromUtf8("SkDebuggerGUI"));
    SkDebuggerGUI->resize(1200, 1000);
    SkDebuggerGUI->setWindowIcon(windowIcon);
    SkDebuggerGUI->setWindowTitle("Skia Debugger");

    fActionOpen.setShortcuts(QKeySequence::Open);
    fActionOpen.setText("Open");

    QIcon breakpoint;
    breakpoint.addFile(QString::fromUtf8(":/breakpoint.png"),
            QSize(), QIcon::Normal, QIcon::Off);
    fActionBreakpoint.setShortcut(QKeySequence(tr("Ctrl+B")));
    fActionBreakpoint.setIcon(breakpoint);
    fActionBreakpoint.setText("Breakpoints");
    fActionBreakpoint.setCheckable(true);

    QIcon cancel;
    cancel.addFile(QString::fromUtf8(":/reload.png"), QSize(),
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

    QIcon profile;
    profile.addFile(QString::fromUtf8(":/profile.png"), QSize(),
                    QIcon::Normal, QIcon::Off);
    fActionProfile.setIcon(profile);
    fActionProfile.setText("Profile");
    fActionProfile.setDisabled(true);

    QIcon inspector;
    inspector.addFile(QString::fromUtf8(":/inspector.png"),
            QSize(), QIcon::Normal, QIcon::Off);
    fActionInspector.setShortcut(QKeySequence(tr("Ctrl+I")));
    fActionInspector.setIcon(inspector);
    fActionInspector.setText("Inspector");

    QIcon settings;
    settings.addFile(QString::fromUtf8(":/inspector.png"),
            QSize(), QIcon::Normal, QIcon::Off);
    fActionSettings.setShortcut(QKeySequence(tr("Ctrl+G")));
    fActionSettings.setIcon(settings);
    fActionSettings.setText("Settings");

    QIcon play;
    play.addFile(QString::fromUtf8(":/play.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionPlay.setShortcut(QKeySequence(tr("Ctrl+P")));
    fActionPlay.setIcon(play);
    fActionPlay.setText("Play");

    QIcon pause;
    pause.addFile(QString::fromUtf8(":/pause.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionPause.setShortcut(QKeySequence(tr("Space")));
    fActionPause.setCheckable(true);
    fActionPause.setIcon(pause);
    fActionPause.setText("Pause");

    QIcon rewind;
    rewind.addFile(QString::fromUtf8(":/rewind.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionRewind.setShortcut(QKeySequence(tr("Ctrl+R")));
    fActionRewind.setIcon(rewind);
    fActionRewind.setText("Rewind");

    fActionSave.setShortcut(QKeySequence::Save);
    fActionSave.setText("Save");
    fActionSave.setDisabled(true);
    fActionSaveAs.setShortcut(QKeySequence::SaveAs);
    fActionSaveAs.setText("Save As");
    fActionSaveAs.setDisabled(true);

    fActionShowDeletes.setShortcut(QKeySequence(tr("Ctrl+X")));
    fActionShowDeletes.setText("Deleted Commands");
    fActionShowDeletes.setCheckable(true);

    QIcon stepBack;
    stepBack.addFile(QString::fromUtf8(":/previous.png"), QSize(),
            QIcon::Normal, QIcon::Off);
    fActionStepBack.setShortcut(QKeySequence(tr("[")));
    fActionStepBack.setIcon(stepBack);
    fActionStepBack.setText("Step Back");

    QIcon stepForward;
    stepForward.addFile(QString::fromUtf8(":/next.png"),
            QSize(), QIcon::Normal, QIcon::Off);
    fActionStepForward.setShortcut(QKeySequence(tr("]")));
    fActionStepForward.setIcon(stepForward);
    fActionStepForward.setText("Step Forward");

    fActionZoomIn.setShortcut(QKeySequence(tr("Ctrl+=")));
    fActionZoomIn.setText("Zoom In");
    fActionZoomOut.setShortcut(QKeySequence(tr("Ctrl+-")));
    fActionZoomOut.setText("Zoom Out");

    fListWidget.setItemDelegate(new SkListWidget(&fListWidget));
    fListWidget.setObjectName(QString::fromUtf8("listWidget"));
    fListWidget.setMinimumWidth(250);

    fFilter.addItem("--Filter By Available Commands--");

    fDirectoryWidget.setMinimumWidth(250);
    fDirectoryWidget.setStyleSheet("QListWidget::Item {padding: 5px;}");

    fCanvasWidget.setSizePolicy(QSizePolicy::Expanding,
            QSizePolicy::Expanding);

    fDrawCommandGeometryWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    fSettingsAndImageLayout.addWidget(&fSettingsWidget);

    // View state group, part of inspector.
    fViewStateFrame.setFrameStyle(QFrame::Panel);
    fViewStateFrame.setLayout(&fViewStateFrameLayout);
    fViewStateFrameLayout.addWidget(&fViewStateGroup);
    fViewStateGroup.setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    fViewStateGroup.setTitle("View");
    fViewStateLayout.addRow("Zoom Level", &fZoomBox);
    fZoomBox.setText("100%");
    fZoomBox.setMinimumSize(QSize(50,25));
    fZoomBox.setMaximumSize(QSize(50,25));
    fZoomBox.setAlignment(Qt::AlignRight);
    fZoomBox.setReadOnly(true);
    fViewStateLayout.addRow("Command HitBox", &fCommandHitBox);
    fCommandHitBox.setText("0");
    fCommandHitBox.setMinimumSize(QSize(50,25));
    fCommandHitBox.setMaximumSize(QSize(50,25));
    fCommandHitBox.setAlignment(Qt::AlignRight);
    fCommandHitBox.setReadOnly(true);
    fViewStateLayout.addRow("Current Command", &fCurrentCommandBox);
    fCurrentCommandBox.setText("0");
    fCurrentCommandBox.setMinimumSize(QSize(50,25));
    fCurrentCommandBox.setMaximumSize(QSize(50,25));
    fCurrentCommandBox.setAlignment(Qt::AlignRight);
    fCurrentCommandBox.setReadOnly(true);
    fViewStateGroup.setLayout(&fViewStateLayout);
    fSettingsAndImageLayout.addWidget(&fViewStateFrame);

    fDrawCommandGeometryWidget.setToolTip("Current Command Geometry");
    fSettingsAndImageLayout.addWidget(&fDrawCommandGeometryWidget);

    fLeftColumnSplitter.addWidget(&fListWidget);
    fLeftColumnSplitter.addWidget(&fDirectoryWidget);
    fLeftColumnSplitter.setOrientation(Qt::Vertical);

    fCanvasSettingsAndImageLayout.setSpacing(6);
    fCanvasSettingsAndImageLayout.addWidget(&fCanvasWidget, 1);
    fCanvasSettingsAndImageLayout.addLayout(&fSettingsAndImageLayout, 0);

    fMainAndRightColumnLayout.setSpacing(6);
    fMainAndRightColumnLayout.addLayout(&fCanvasSettingsAndImageLayout, 1);
    fMainAndRightColumnLayout.addWidget(&fInspectorWidget, 0);
    fMainAndRightColumnWidget.setLayout(&fMainAndRightColumnLayout);

    fCentralSplitter.addWidget(&fLeftColumnSplitter);
    fCentralSplitter.addWidget(&fMainAndRightColumnWidget);
    fCentralSplitter.setStretchFactor(0, 0);
    fCentralSplitter.setStretchFactor(1, 1);

    SkDebuggerGUI->setCentralWidget(&fCentralSplitter);
    SkDebuggerGUI->setStatusBar(&fStatusBar);

    fToolBar.setIconSize(QSize(32, 32));
    fToolBar.setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    SkDebuggerGUI->addToolBar(Qt::TopToolBarArea, &fToolBar);

    fSpacer.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    fToolBar.addAction(&fActionRewind);
    fToolBar.addAction(&fActionStepBack);
    fToolBar.addAction(&fActionPause);
    fToolBar.addAction(&fActionStepForward);
    fToolBar.addAction(&fActionPlay);
    fToolBar.addSeparator();
    fToolBar.addAction(&fActionInspector);
    fToolBar.addAction(&fActionSettings);
    fToolBar.addSeparator();
    fToolBar.addAction(&fActionProfile);

    fToolBar.addSeparator();
    fToolBar.addWidget(&fSpacer);
    fToolBar.addWidget(&fFilter);
    fToolBar.addAction(&fActionCancel);

    // TODO(chudy): Remove static call.
    fDirectoryWidgetActive = false;
    fFileName = "";
    setupDirectoryWidget("");
    fDirectoryWidgetActive = true;

    // Menu Bar
    fMenuFile.setTitle("File");
    fMenuFile.addAction(&fActionOpen);
    fMenuFile.addAction(&fActionSave);
    fMenuFile.addAction(&fActionSaveAs);
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
    fMenuView.addAction(&fActionZoomIn);
    fMenuView.addAction(&fActionZoomOut);

    fMenuWindows.setTitle("Window");
    fMenuWindows.addAction(&fActionInspector);
    fMenuWindows.addAction(&fActionSettings);
    fMenuWindows.addAction(&fActionDirectory);

    fActionGoToLine.setText("Go to Line...");
    fActionGoToLine.setDisabled(true);
    fMenuBar.addAction(fMenuFile.menuAction());
    fMenuBar.addAction(fMenuEdit.menuAction());
    fMenuBar.addAction(fMenuView.menuAction());
    fMenuBar.addAction(fMenuNavigate.menuAction());
    fMenuBar.addAction(fMenuWindows.menuAction());

    SkDebuggerGUI->setMenuBar(&fMenuBar);
    QMetaObject::connectSlotsByName(SkDebuggerGUI);
}

void SkDebuggerGUI::setupDirectoryWidget(const QString& path) {
    fPath = path;
    QDir dir(path);
    QRegExp r(".skp");
    fDirectoryWidget.clear();
    const QStringList files = dir.entryList();
    foreach (QString f, files) {
        if (f.contains(r))
            fDirectoryWidget.addItem(f);
    }
}

void SkDebuggerGUI::loadPicture(const SkString& fileName) {
    fFileName = fileName;
    fLoading = true;
    SkAutoTDelete<SkStream> stream(SkNEW_ARGS(SkFILEStream, (fileName.c_str())));

    SkPicture* picture = SkPicture::CreateFromStream(stream);

    if (NULL == picture) {
        QMessageBox::critical(this, "Error loading file", "Couldn't read file, sorry.");
        return;
    }

    fCanvasWidget.resetWidgetTransform();
    fDebugger.loadPicture(picture);

    fSkipCommands.setCount(fDebugger.getSize());
    for (int i = 0; i < fSkipCommands.count(); ++i) {
        fSkipCommands[i] = false;
    }

    SkSafeUnref(picture);

    fActionProfile.setDisabled(false);

    /* fDebugCanvas is reinitialized every load picture. Need it to retain value
     * of the visibility filter.
     * TODO(chudy): This should be deprecated since fDebugger is not
     * recreated.
     * */
    fDebugger.highlightCurrentCommand(fSettingsWidget.isVisibilityFilterEnabled());

    this->setupListWidget();
    this->setupComboBox();
    this->setupOverviewText(NULL, 0.0, 1);
    fInspectorWidget.setDisabled(false);
    fViewStateFrame.setDisabled(false);
    fSettingsWidget.setDisabled(false);
    fMenuEdit.setDisabled(false);
    fMenuNavigate.setDisabled(false);
    fMenuView.setDisabled(false);
    fActionSave.setDisabled(false);
    fActionSaveAs.setDisabled(false);
    fActionPause.setChecked(false);
    fDrawCommandGeometryWidget.setDrawCommandIndex(-1);

    fLoading = false;
    actionPlay();
}

void SkDebuggerGUI::setupListWidget() {

    SkASSERT(!strcmp("Save",
                     SkDrawCommand::GetCommandString(SkDrawCommand::kSave_OpType)));
    SkASSERT(!strcmp("SaveLayer",
                     SkDrawCommand::GetCommandString(SkDrawCommand::kSaveLayer_OpType)));
    SkASSERT(!strcmp("Restore",
                     SkDrawCommand::GetCommandString(SkDrawCommand::kRestore_OpType)));
    SkASSERT(!strcmp("BeginDrawPicture",
                     SkDrawCommand::GetCommandString(SkDrawCommand::kBeginDrawPicture_OpType)));
    SkASSERT(!strcmp("EndDrawPicture",
                     SkDrawCommand::GetCommandString(SkDrawCommand::kEndDrawPicture_OpType)));

    fListWidget.clear();
    int counter = 0;
    int indent = 0;
    for (int i = 0; i < fDebugger.getSize(); i++) {
        QListWidgetItem *item = new QListWidgetItem();
        SkDrawCommand* command = fDebugger.getDrawCommandAt(i);
        SkString commandString = command->toString();
        item->setData(Qt::DisplayRole, commandString.c_str());
        item->setData(Qt::UserRole + 1, counter++);

        if (0 == strcmp("Restore", commandString.c_str()) ||
            0 == strcmp("EndDrawPicture", commandString.c_str())) {
            indent -= 10;
        }

        item->setData(Qt::UserRole + 3, indent);

        if (0 == strcmp("Save", commandString.c_str()) ||
            0 == strcmp("SaveLayer", commandString.c_str()) ||
            0 == strcmp("BeginDrawPicture", commandString.c_str())) {
            indent += 10;
        }

        item->setData(Qt::UserRole + 4, -1);

        fListWidget.addItem(item);
    }
}

void SkDebuggerGUI::setupOverviewText(const SkTDArray<double>* typeTimes,
                                      double totTime,
                                      int numRuns) {
    SkString overview;
    fDebugger.getOverviewText(typeTimes, totTime, &overview, numRuns);
    fInspectorWidget.setText(overview.c_str(), SkInspectorWidget::kOverview_TabType);
}


void SkDebuggerGUI::setupComboBox() {
    fFilter.clear();
    fFilter.addItem("--Filter By Available Commands--");

    std::map<std::string, int> map;
    for (int i = 0; i < fDebugger.getSize(); i++) {
        map[fDebugger.getDrawCommandAt(i)->toString().c_str()]++;
    }

    for (std::map<std::string, int>::iterator it = map.begin(); it != map.end();
         ++it) {
        fFilter.addItem((it->first).c_str());
    }

    // NOTE(chudy): Makes first item unselectable.
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(
            fFilter.model());
    QModelIndex firstIndex = model->index(0, fFilter.modelColumn(),
            fFilter.rootModelIndex());
    QStandardItem* firstItem = model->itemFromIndex(firstIndex);
    firstItem->setSelectable(false);
}

void SkDebuggerGUI::updateImage() {
    if (this->isPaused()) {
        fCanvasWidget.drawTo(fPausedRow);
    } else {
        fCanvasWidget.drawTo(fListWidget.currentRow());
    }
}

void SkDebuggerGUI::updateHit(int newHit) {
    fCommandHitBox.setText(QString::number(newHit));
}

