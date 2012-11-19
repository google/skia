/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDebuggerGUI.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include <QListWidgetItem>
#include "PictureBenchmark.h"
#include "PictureRenderer.h"
#include "SkBenchLogger.h"
#include "SkPictureRecord.h"
#include "SkPicturePlayback.h"
#include "BenchTimer.h"

SkDebuggerGUI::SkDebuggerGUI(QWidget *parent) :
        QMainWindow(parent)
    , fCentralWidget(this)
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
    , fListWidget(&fCentralWidget)
    , fDirectoryWidget(&fCentralWidget)
    , fCanvasWidget(this, &fDebugger)
    , fMenuBar(this)
    , fMenuFile(this)
    , fMenuNavigate(this)
    , fMenuView(this)
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
    connect(&fActionProfile, SIGNAL(triggered()), this, SLOT(actionProfile()));
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
    connect(&fActionSaveAs, SIGNAL(triggered()), this, SLOT(actionSaveAs()));
    connect(&fActionSave, SIGNAL(triggered()), this, SLOT(actionSave()));

    fMapper.setMapping(&fActionZoomIn, 1);
    fMapper.setMapping(&fActionZoomOut, -1);

    connect(&fActionZoomIn, SIGNAL(triggered()), &fMapper, SLOT(map()));
    connect(&fActionZoomOut, SIGNAL(triggered()), &fMapper, SLOT(map()));
    connect(&fMapper, SIGNAL(mapped(int)), &fCanvasWidget, SLOT(keyZoom(int)));

    fInspectorWidget.setDisabled(true);
    fMenuEdit.setDisabled(true);
    fMenuNavigate.setDisabled(true);
    fMenuView.setDisabled(true);

    SkGraphics::Init();
}

SkDebuggerGUI::~SkDebuggerGUI() {
    SkGraphics::Term();
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
        item->setHidden(fDebugger.isCommandVisible(row)
                && fDeletesActivated);
    }
}

// The timed picture playback uses the SkPicturePlayback's profiling stubs
// to time individual commands. The offsets are needed to map SkPicture
// offsets to individual commands.
class SkTimedPicturePlayback : public SkPicturePlayback {
public:
    SkTimedPicturePlayback(SkStream* stream, const SkPictInfo& info, bool* isValid,
                           SkSerializationHelpers::DecodeBitmap decoder,
                           const SkTDArray<size_t>& offsets)
        : INHERITED(stream, info, isValid, decoder)
        , fTot(0.0)
        , fCurCommand(0)
        , fOffsets(offsets) {
        fTimes.setCount(fOffsets.count());
        for (int i = 0; i < fOffsets.count(); ++i) {
            fTimes[i] = 0;
        }
    }

    int count() const { return fTimes.count(); }

    double time(int index) const { return fTimes[index] / fTot; }

protected:
    BenchTimer fTimer;
    SkTDArray<size_t> fOffsets; // offset in the SkPicture for each command
    SkTDArray<double> fTimes;   // sum of time consumed for each command
    double fTot;                // total of all times in 'fTimes'
    size_t fCurOffset;
    int fCurCommand;            // the current command being executed/timed

    virtual void preDraw(size_t offset, int type) {
        // This search isn't as bad as it seems. In normal playback mode, the
        // base class steps through the commands in order and can only skip ahead
        // a bit on a clip. This class is only used during profiling so we
        // don't have to worry about forward/backward scrubbing through commands.
        for (int i = 0; offset != fOffsets[fCurCommand]; ++i) {
            fCurCommand = (fCurCommand+1) % fOffsets.count();
            SkASSERT(i <= fOffsets.count()); // should always find the offset in the list
        }

        fCurOffset = offset;

        fTimer.start();
    }

    virtual void postDraw(size_t offset) {
        fTimer.end();

        SkASSERT(offset == fCurOffset);

#if defined(SK_BUILD_FOR_WIN32)
        // CPU timer doesn't work well on Windows
        fTimes[fCurCommand] += fTimer.fWall;
        fTot += fTimer.fWall;
#else
        fTimes[fCurCommand] += fTimer.fCpu;
        fTot += fTimer.fCpu;
#endif
    }

private:
    typedef SkPicturePlayback INHERITED;
};

// Wrap SkPicture to allow installation of an SkTimedPicturePlayback object
class SkTimedPicture : public SkPicture {
public:
    explicit SkTimedPicture(SkStream* stream,
                            bool* success,
                            SkSerializationHelpers::DecodeBitmap decoder,
                            const SkTDArray<size_t>& offsets) {
        if (success) {
            *success = false;
        }
        fRecord = NULL;
        fPlayback = NULL;
        fWidth = fHeight = 0;

        SkPictInfo info;

        if (!stream->read(&info, sizeof(info))) {
            return;
        }
        if (SkPicture::PICTURE_VERSION != info.fVersion) {
            return;
        }

        if (stream->readBool()) {
            bool isValid = false;
            fPlayback = SkNEW_ARGS(SkTimedPicturePlayback,
                                   (stream, info, &isValid, decoder, offsets));
            if (!isValid) {
                SkDELETE(fPlayback);
                fPlayback = NULL;
                return;
            }
        }

        // do this at the end, so that they will be zero if we hit an error.
        fWidth = info.fWidth;
        fHeight = info.fHeight;
        if (success) {
            *success = true;
        }
    }

    int count() const { return ((SkTimedPicturePlayback*) fPlayback)->count(); }

    // return the fraction of the total time this command consumed
    double time(int index) const { return ((SkTimedPicturePlayback*) fPlayback)->time(index); }

private:
    typedef SkPicture INHERITED;
};

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

    bool success = false;
    SkTimedPicture picture(&inputStream, &success, &SkImageDecoder::DecodeStream, fOffsets);
    if (!success) {
        return;
    }

    sk_tools::PictureBenchmark benchmark;

    sk_tools::TiledPictureRenderer* renderer = NULL;

    renderer = SkNEW(sk_tools::TiledPictureRenderer);
    renderer->setTileWidth(256);
    renderer->setTileHeight(256);


    benchmark.setRepeats(2);
    benchmark.setRenderer(renderer);
    benchmark.setTimersToShow(true, false, true, false, false);

    SkBenchLogger logger;

    benchmark.setLogger(&logger);

    benchmark.run(&picture);

    SkASSERT(picture.count() == fListWidget.count());

    // extract the individual command times from the SkTimedPlaybackPicture
    for (int i = 0; i < picture.count(); ++i) {
        double temp = picture.time(i);

        QListWidgetItem* item = fListWidget.item(i);

        item->setData(Qt::UserRole + 4, 100.0*temp);
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
    }
    if (fPause) {
        fCanvasWidget.drawTo(fPausedRow);
    } else {
        fCanvasWidget.drawTo(fListWidget.currentRow());
    }
}

void SkDebuggerGUI::actionCommandFilter() {
    fDebugger.highlightCurrentCommand(
            fSettingsWidget.getVisibilityButton()->isChecked());
    fCanvasWidget.drawTo(fListWidget.currentRow());
}

void SkDebuggerGUI::actionClose() {
    this->close();
}

void SkDebuggerGUI::actionDelete() {
    int currentRow = fListWidget.currentRow();
    QListWidgetItem* item = fListWidget.currentItem();

    if (fDebugger.isCommandVisible(currentRow)) {
        item->setData(Qt::UserRole + 2, QPixmap(":/delete.png"));
        fDebugger.setCommandVisible(currentRow, false);
    } else {
        item->setData(Qt::UserRole + 2, QPixmap(":/blank.png"));
        fDebugger.setCommandVisible(currentRow, true);
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

void SkDebuggerGUI::actionSave() {
    fFileName = fPath.toAscii();
    fFileName.append("/");
    fFileName.append(fDirectoryWidget.currentItem()->text().toAscii());
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

void SkDebuggerGUI::drawComplete() {
    fInspectorWidget.setMatrix(fDebugger.getCurrentMatrix());
    fInspectorWidget.setClip(fDebugger.getCurrentClip());
}

void SkDebuggerGUI::saveToFile(const SkString& filename) {
    SkFILEWStream file(filename.c_str());
    fDebugger.makePicture()->serialize(&file);
}

void SkDebuggerGUI::loadFile(QListWidgetItem *item) {
    if (fDirectoryWidgetActive) {
        fFileName = fPath.toAscii();
        fFileName.append("/");
        fFileName.append(item->text().toAscii());
        loadPicture(fFileName);
    }
}

void SkDebuggerGUI::openFile() {
    QString temp = QFileDialog::getOpenFileName(this, tr("Open File"), "",
            tr("Files (*.*)"));
    fDirectoryWidgetActive = false;
    if (!temp.isEmpty()) {
        QFileInfo pathInfo(temp);
        fPath = pathInfo.path();
        loadPicture(SkString(temp.toAscii().data()));
        setupDirectoryWidget();
    }
    fDirectoryWidgetActive = true;
}

void SkDebuggerGUI::pauseDrawing(bool isPaused) {
    fPause = isPaused;
    fPausedRow = fListWidget.currentRow();
    fCanvasWidget.drawTo(fPausedRow);
}

void SkDebuggerGUI::registerListClick(QListWidgetItem *item) {
    if(!fLoading) {
        int currentRow = fListWidget.currentRow();

        if (currentRow != -1) {
            if (!fPause) {
                fCanvasWidget.drawTo(currentRow);
            }
            SkTDArray<SkString*> *currInfo = fDebugger.getCommandInfo(
                    currentRow);

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
                fInspectorWidget.setDisabled(false);
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
    profile.addFile(QString::fromUtf8(":/images/Ico/reload.png"), QSize(),
                    QIcon::Normal, QIcon::Off);
    fActionProfile.setIcon(profile);
    fActionProfile.setText("Profile");
    fActionProfile.setDisabled(true);

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

    fCentralWidget.setLayout(&fContainerLayout);
    fContainerLayout.setSpacing(6);
    fContainerLayout.setContentsMargins(11, 11, 11, 11);
    fContainerLayout.addLayout(&fLeftColumnLayout);
    fContainerLayout.addLayout(&fMainAndRightColumnLayout);

    SkDebuggerGUI->setCentralWidget(&fCentralWidget);
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
    fToolBar.addSeparator();
    fToolBar.addAction(&fActionProfile);

    fToolBar.addSeparator();
    fToolBar.addWidget(&fSpacer);
    fToolBar.addWidget(&fFilter);
    fToolBar.addAction(&fActionCancel);

    // TODO(chudy): Remove static call.
    fDirectoryWidgetActive = false;
    fPath = "";
    fFileName = "";
    setupDirectoryWidget();
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

// SkOffsetPicturePlayback records the offset of each command in the picture.
// These are needed by the profiling system.
class SkOffsetPicturePlayback : public SkPicturePlayback {
public:
    SkOffsetPicturePlayback(SkStream* stream, const SkPictInfo& info, bool* isValid,
                            SkSerializationHelpers::DecodeBitmap decoder)
        : INHERITED(stream, info, isValid, decoder) {
    }

    const SkTDArray<size_t>& offsets() const { return fOffsets; }

protected:
    SkTDArray<size_t> fOffsets;

    virtual void preDraw(size_t offset, int type) {
        *fOffsets.append() = offset;
    }

private:
    typedef SkPicturePlayback INHERITED;
};

// Picture to wrap an SkOffsetPicturePlayback.
class SkOffsetPicture : public SkPicture {
public:
    SkOffsetPicture(SkStream* stream,
                    bool* success,
                    SkSerializationHelpers::DecodeBitmap decoder) {
        if (success) {
            *success = false;
        }
        fRecord = NULL;
        fPlayback = NULL;
        fWidth = fHeight = 0;

        SkPictInfo info;

        if (!stream->read(&info, sizeof(info))) {
            return;
        }
        if (PICTURE_VERSION != info.fVersion) {
            return;
        }

        if (stream->readBool()) {
            bool isValid = false;
            fPlayback = SkNEW_ARGS(SkOffsetPicturePlayback, (stream, info, &isValid, decoder));
            if (!isValid) {
                SkDELETE(fPlayback);
                fPlayback = NULL;
                return;
            }
        }

        // do this at the end, so that they will be zero if we hit an error.
        fWidth = info.fWidth;
        fHeight = info.fHeight;
        if (success) {
            *success = true;
        }
    }

    const SkTDArray<size_t>& offsets() const {
        return ((SkOffsetPicturePlayback*) fPlayback)->offsets();
    }

private:
    typedef SkPicture INHERITED;
};



void SkDebuggerGUI::loadPicture(const SkString& fileName) {
    fFileName = fileName;
    fLoading = true;
    SkStream* stream = SkNEW_ARGS(SkFILEStream, (fileName.c_str()));
    SkOffsetPicture* picture = SkNEW_ARGS(SkOffsetPicture, (stream, NULL, &SkImageDecoder::DecodeStream));

    fCanvasWidget.resetWidgetTransform();
    fDebugger.loadPicture(picture);

    fOffsets = picture->offsets();

    SkSafeUnref(stream);
    SkSafeUnref(picture);

    // Will this automatically clear out due to nature of refcnt?
    SkTDArray<SkString*>* commands = fDebugger.getDrawCommands();

    // If SkPicturePlayback is compiled w/o SK_PICTURE_PROFILING_STUBS
    // the offset count will always be zero
    SkASSERT(0 == fOffsets.count() || commands->count() == fOffsets.count());
    if (commands->count() == fOffsets.count()) {
        fActionProfile.setDisabled(false);
    }

    /* fDebugCanvas is reinitialized every load picture. Need it to retain value
     * of the visibility filter.
     * TODO(chudy): This should be deprecated since fDebugger is not
     * recreated.
     * */
    fDebugger.highlightCurrentCommand(fSettingsWidget.getVisibilityButton()->isChecked());

    setupListWidget(commands);
    setupComboBox(commands);
    fInspectorWidget.setDisabled(false);
    fSettingsWidget.setDisabled(false);
    fMenuEdit.setDisabled(false);
    fMenuNavigate.setDisabled(false);
    fMenuView.setDisabled(false);
    fActionSave.setDisabled(false);
    fActionSaveAs.setDisabled(false);
    fLoading = false;
    actionPlay();
}

void SkDebuggerGUI::setupListWidget(SkTDArray<SkString*>* command) {
    fListWidget.clear();
    int counter = 0;
    int indent = 0;
    for (int i = 0; i < command->count(); i++) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setData(Qt::DisplayRole, (*command)[i]->c_str());
        item->setData(Qt::UserRole + 1, counter++);

        if (0 == strcmp("Restore", (*command)[i]->c_str())) {
            indent -= 10;
        }

        item->setData(Qt::UserRole + 3, indent);

        if (0 == strcmp("Save", (*command)[i]->c_str()) ||
            0 == strcmp("Save Layer", (*command)[i]->c_str())) {
            indent += 10;
        }

        item->setData(Qt::UserRole + 4, -1.0);

        fListWidget.addItem(item);
    }
}

void SkDebuggerGUI::setupComboBox(SkTDArray<SkString*>* command) {
    fFilter.clear();
    fFilter.addItem("--Filter By Available Commands--");

    std::map<std::string, int> map;
    for (int i = 0; i < command->count(); i++) {
        map[(*command)[i]->c_str()]++;
    }

    QString overview;
    int counter = 0;
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
    overview.append("SkPicture Width: ");
    // NOTE(chudy): This is where we can pull out the SkPictures width.
    overview.append(QString::number(fDebugger.pictureWidth()));
    overview.append("px<br/>");
    overview.append("SkPicture Height: ");
    overview.append(QString::number(fDebugger.pictureHeight()));
    overview.append("px");
    fInspectorWidget.setText(overview, SkInspectorWidget::kOverview_TabType);

    // NOTE(chudy): Makes first item unselectable.
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(
            fFilter.model());
    QModelIndex firstIndex = model->index(0, fFilter.modelColumn(),
            fFilter.rootModelIndex());
    QStandardItem* firstItem = model->itemFromIndex(firstIndex);
    firstItem->setSelectable(false);
}
