/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDebuggerGUI.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include <QListWidgetItem>
#include "PictureRenderer.h"
#include "SkPicturePlayback.h"
#include "SkPictureRecord.h"
#include "SkPictureData.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

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
    , fActionToggleIndexStyle(this)
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
    , fImageWidget(&fDebugger)
    , fMenuBar(this)
    , fMenuFile(this)
    , fMenuNavigate(this)
    , fMenuView(this)
    , fBreakpointsActivated(false)
    , fIndexStyleToggle(false)
    , fDeletesActivated(false)
    , fPause(false)
    , fLoading(false)
{
    setupUi(this);
    fListWidget.setSelectionMode(QAbstractItemView::ExtendedSelection);
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
    connect(&fActionToggleIndexStyle, SIGNAL(triggered()), this, SLOT(actionToggleIndexStyle()));
    connect(&fActionInspector, SIGNAL(triggered()), this, SLOT(actionInspector()));
    connect(&fActionSettings, SIGNAL(triggered()), this, SLOT(actionSettings()));
    connect(&fFilter, SIGNAL(activated(QString)), this, SLOT(toggleFilter(QString)));
    connect(&fActionProfile, SIGNAL(triggered()), this, SLOT(actionProfile()));
    connect(&fActionCancel, SIGNAL(triggered()), this, SLOT(actionCancel()));
    connect(&fActionClearBreakpoints, SIGNAL(triggered()), this, SLOT(actionClearBreakpoints()));
    connect(&fActionClearDeletes, SIGNAL(triggered()), this, SLOT(actionClearDeletes()));
    connect(&fActionClose, SIGNAL(triggered()), this, SLOT(actionClose()));
    connect(&fSettingsWidget, SIGNAL(visibilityFilterChanged()), this, SLOT(actionCommandFilter()));
#if SK_SUPPORT_GPU
    connect(&fSettingsWidget, SIGNAL(glSettingsChanged()), this, SLOT(actionGLWidget()));
#endif
    connect(&fSettingsWidget, SIGNAL(texFilterSettingsChanged()), this, SLOT(actionTextureFilter()));
    connect(fSettingsWidget.getRasterCheckBox(), SIGNAL(toggled(bool)), this, SLOT(actionRasterWidget(bool)));
    connect(fSettingsWidget.getOverdrawVizCheckBox(), SIGNAL(toggled(bool)), this, SLOT(actionOverdrawVizWidget(bool)));
    connect(fSettingsWidget.getMegaVizCheckBox(), SIGNAL(toggled(bool)), this, SLOT(actionMegaVizWidget(bool)));
    connect(fSettingsWidget.getPathOpsCheckBox(), SIGNAL(toggled(bool)), this, SLOT(actionPathOpsWidget(bool)));
    connect(&fActionPause, SIGNAL(toggled(bool)), this, SLOT(pauseDrawing(bool)));
    connect(&fActionCreateBreakpoint, SIGNAL(activated()), this, SLOT(toggleBreakpoint()));
    connect(&fActionShowDeletes, SIGNAL(triggered()), this, SLOT(showDeletes()));
    connect(&fCanvasWidget, SIGNAL(hitChanged(int)), this, SLOT(selectCommand(int)));
    connect(&fCanvasWidget, SIGNAL(hitChanged(int)), &fSettingsWidget, SLOT(updateHit(int)));
    connect(&fCanvasWidget, SIGNAL(scaleFactorChanged(float)), this, SLOT(actionScale(float)));
    connect(&fCanvasWidget, SIGNAL(commandChanged(int)), &fSettingsWidget, SLOT(updateCommand(int)));
    connect(&fActionSaveAs, SIGNAL(triggered()), this, SLOT(actionSaveAs()));
    connect(&fActionSave, SIGNAL(triggered()), this, SLOT(actionSave()));

    fMapper.setMapping(&fActionZoomIn, SkCanvasWidget::kIn_ZoomCommand);
    fMapper.setMapping(&fActionZoomOut, SkCanvasWidget::kOut_ZoomCommand);

    connect(&fActionZoomIn, SIGNAL(triggered()), &fMapper, SLOT(map()));
    connect(&fActionZoomOut, SIGNAL(triggered()), &fMapper, SLOT(map()));
    connect(&fMapper, SIGNAL(mapped(int)), &fCanvasWidget, SLOT(zoom(int)));

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

void SkDebuggerGUI::actionToggleIndexStyle() {
    fIndexStyleToggle = !fIndexStyleToggle;
    SkListWidget* list = (SkListWidget*) fListWidget.itemDelegate();
    list->setIndexStyle(fIndexStyleToggle ? SkListWidget::kIndex_IndexStyle :
                                            SkListWidget::kOffset_IndexStyle);
    fListWidget.update();
}

void SkDebuggerGUI::showDeletes() {
    fDeletesActivated = !fDeletesActivated;
    for (int row = 0; row < fListWidget.count(); row++) {
        QListWidgetItem *item = fListWidget.item(row);
        item->setHidden(fDebugger.isCommandVisible(row) && fDeletesActivated);
    }
}

// The timed picture playback just steps through every operation timing
// each one individually. Note that each picture should be replayed multiple 
// times (via calls to 'draw') before each command's time is accessed via 'time'.
class SkTimedPicturePlayback : public SkPicturePlayback {
public:

    SkTimedPicturePlayback(const SkPicture* picture, const SkTDArray<bool>& deletedCommands)
        : INHERITED(picture)
        , fSkipCommands(deletedCommands)
        , fTot(0.0)
        , fCurCommand(0) {
        fTimes.setCount(deletedCommands.count());
        fTypeTimes.setCount(LAST_DRAWTYPE_ENUM+1);
        this->resetTimes();
    }

    virtual void draw(SkCanvas* canvas, SkDrawPictureCallback* callback) SK_OVERRIDE {
        AutoResetOpID aroi(this);
        SkASSERT(0 == fCurOffset);

        SkReader32 reader(fPictureData->opData()->bytes(), fPictureData->opData()->size());

        // Record this, so we can concat w/ it if we encounter a setMatrix()
        SkMatrix initialMatrix = canvas->getTotalMatrix();

        SkAutoCanvasRestore acr(canvas, false);

        int opIndex = -1;

        while (!reader.eof()) {
            if (callback && callback->abortDrawing()) {
                return;
            }

            fCurOffset = reader.offset();
            uint32_t size;
            DrawType op = ReadOpAndSize(&reader, &size);
            if (NOOP == op) {
                // NOOPs are to be ignored - do not propagate them any further
                reader.setOffset(fCurOffset + size);
                continue;
            }

            opIndex++;

            if (this->preDraw(opIndex, op)) {
                // This operation is disabled in the debugger's GUI
                reader.setOffset(fCurOffset + size);
                continue;
            }

            this->handleOp(&reader, op, size, canvas, initialMatrix);

            this->postDraw(opIndex);
        }
    }

    void resetTimes() {
        for (int i = 0; i < fTimes.count(); ++i) {
            fTimes[i] = 0.0;
        }
        for (int i = 0; i < fTypeTimes.count(); ++i) {
            fTypeTimes[i] = 0.0f;
        }
        fTot = 0.0;
    }

    int count() const { return fTimes.count(); }

    // Return the fraction of the total time consumed by the index-th operation
    double time(int index) const { return fTimes[index] / fTot; }

    const SkTDArray<double>* typeTimes() const { return &fTypeTimes; }

    double totTime() const { return fTot; }

protected:
    SysTimer fTimer;
    SkTDArray<bool> fSkipCommands; // has the command been deleted in the GUI?
    SkTDArray<double> fTimes;   // sum of time consumed for each command
    SkTDArray<double> fTypeTimes; // sum of time consumed for each type of command (e.g., drawPath)
    double fTot;                // total of all times in 'fTimes'

    int fCurType;
    int fCurCommand;            // the current command being executed/timed

    bool preDraw(int opIndex, int type) {
        fCurCommand = opIndex;

        if (fSkipCommands[fCurCommand]) {
            return true;
        }

        fCurType = type;
        // The SkDebugCanvas doesn't recognize these types. This class needs to
        // convert or else we'll wind up with a mismatch between the type counts
        // the debugger displays and the profile times.
        if (DRAW_POS_TEXT_TOP_BOTTOM == type) {
            fCurType = DRAW_POS_TEXT;
        } else if (DRAW_POS_TEXT_H_TOP_BOTTOM == type) {
            fCurType = DRAW_POS_TEXT_H;
        }

#if defined(SK_BUILD_FOR_WIN32)
        // CPU timer doesn't work well on Windows
        fTimer.startWall();
#else
        fTimer.startCpu();
#endif

        return false;
    }

    void postDraw(int opIndex) {
#if defined(SK_BUILD_FOR_WIN32)
        // CPU timer doesn't work well on Windows
        double time = fTimer.endWall();
#else
        double time = fTimer.endCpu();
#endif

        SkASSERT(opIndex == fCurCommand);
        SkASSERT(fCurType <= LAST_DRAWTYPE_ENUM);

        fTimes[fCurCommand] += time;
        fTypeTimes[fCurType] += time;
        fTot += time;
    }

private:
    typedef SkPicturePlayback INHERITED;
};

#if 0
// Wrap SkPicture to allow installation of an SkTimedPicturePlayback object
class SkTimedPicture : public SkPicture {
public:
    static SkTimedPicture* CreateTimedPicture(SkStream* stream,
                                              SkPicture::InstallPixelRefProc proc,
                                              const SkTDArray<bool>& deletedCommands) {
        SkPictInfo info;
        if (!InternalOnly_StreamIsSKP(stream, &info)) {
            return NULL;
        }

        // Check to see if there is a playback to recreate.
        if (stream->readBool()) {
            SkTimedPicturePlayback* playback = SkTimedPicturePlayback::CreateFromStream(
                                                                stream,
                                                                info, proc,
                                                                deletedCommands);
            if (NULL == playback) {
                return NULL;
            }

            return SkNEW_ARGS(SkTimedPicture, (playback, info.fWidth, info.fHeight));
        }

        return NULL;
    }

    void resetTimes() { ((SkTimedPicturePlayback*) fData.get())->resetTimes(); }

    int count() const { return ((SkTimedPicturePlayback*) fData.get())->count(); }

    // return the fraction of the total time this command consumed
    double time(int index) const { return ((SkTimedPicturePlayback*) fData.get())->time(index); }

    const SkTDArray<double>* typeTimes() const { return ((SkTimedPicturePlayback*) fData.get())->typeTimes(); }

    double totTime() const { return ((SkTimedPicturePlayback*) fData.get())->totTime(); }

private:
    // disallow default ctor b.c. we don't have a good way to setup the fData ptr
    SkTimedPicture();
    // Private ctor only used by CreateTimedPicture, which has created the playback.
    SkTimedPicture(SkTimedPicturePlayback* playback, int width, int height)
        : INHERITED(playback, width, height) {}
    // disallow the copy ctor - enabling would require copying code from SkPicture
    SkTimedPicture(const SkTimedPicture& src);

    typedef SkPicture INHERITED;
};
#endif

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

#if 0
    // We throw this away the first batch of times to remove first time effects (such as paging in this program)
    pict->resetTimes();
#endif

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


#if 0

    // For now this #if allows switching between tiled and simple rendering
    // modes. Eventually this will be accomplished via the GUI
#if 0
    // With the current batch of SysTimers, profiling in tiled mode
    // gets swamped by the timing overhead:
    //
    //                       tile mode           simple mode
    // debugger                64.2ms              12.8ms
    // bench_pictures          16.9ms              12.4ms
    //
    // This is b.c. in tiled mode each command is called many more times
    // but typically does less work on each invocation (due to clipping)
    sk_tools::TiledPictureRenderer* renderer = NULL;

    renderer = SkNEW(sk_tools::TiledPictureRenderer);
    renderer->setTileWidth(256);
    renderer->setTileHeight(256);
#else
    sk_tools::SimplePictureRenderer* renderer = NULL;

    renderer = SkNEW(sk_tools::SimplePictureRenderer);

#if SK_SUPPORT_GPU
    if (fSettingsWidget.isGLActive()) {
        renderer->setDeviceType(sk_tools::PictureRenderer::kGPU_DeviceType);
        renderer->setSampleCount(fSettingsWidget.getGLSampleCount());
    }
#endif

#endif

    static const int kNumRepeats = 10;

    run(picture.get(), renderer, kNumRepeats);

    SkASSERT(picture->count() == fListWidget.count());

    // extract the individual command times from the SkTimedPlaybackPicture
    for (int i = 0; i < picture->count(); ++i) {
        double temp = picture->time(i);

        QListWidgetItem* item = fListWidget.item(i);

        item->setData(Qt::UserRole + 4, 100.0*temp);
    }

    setupOverviewText(picture->typeTimes(), picture->totTime(), kNumRepeats);
    setupClipStackText();

#endif
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
    if (fPause) {
        fCanvasWidget.drawTo(fPausedRow);
        fImageWidget.draw();
    } else {
        fCanvasWidget.drawTo(fListWidget.currentRow());
        fImageWidget.draw();
    }
}

void SkDebuggerGUI::actionCommandFilter() {
    fDebugger.highlightCurrentCommand(fSettingsWidget.getVisibilityFilter());
    fCanvasWidget.drawTo(fListWidget.currentRow());
    fImageWidget.draw();
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

    int currentRow = fListWidget.currentRow();

    if (fPause) {
        fCanvasWidget.drawTo(fPausedRow);
        fImageWidget.draw();
    } else {
        fCanvasWidget.drawTo(currentRow);
        fImageWidget.draw();
    }
}

#if SK_SUPPORT_GPU
void SkDebuggerGUI::actionGLWidget() {
    bool isToggled = fSettingsWidget.isGLActive();
    if (isToggled) {
        fCanvasWidget.setGLSampleCount(fSettingsWidget.getGLSampleCount());
    }
    fCanvasWidget.setWidgetVisibility(SkCanvasWidget::kGPU_WidgetType, !isToggled);
}
#endif

void SkDebuggerGUI::actionInspector() {
    if (fInspectorWidget.isHidden()) {
        fInspectorWidget.setHidden(false);
        fImageWidget.setHidden(false);
    } else {
        fInspectorWidget.setHidden(true);
        fImageWidget.setHidden(true);
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

void SkDebuggerGUI::actionOverdrawVizWidget(bool isToggled) {
    fDebugger.setOverdrawViz(isToggled);
    fCanvasWidget.update();
}

void SkDebuggerGUI::actionMegaVizWidget(bool isToggled) {
    fDebugger.setMegaViz(isToggled);
    fCanvasWidget.update();
}

void SkDebuggerGUI::actionPathOpsWidget(bool isToggled) {
    fDebugger.setPathOps(isToggled);
    fCanvasWidget.update();
}

void SkDebuggerGUI::actionTextureFilter() {
    SkPaint::FilterLevel level;
    bool enabled = fSettingsWidget.getFilterOverride(&level);
    fDebugger.setTexFilterOverride(enabled, level);
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
    SkAutoTUnref<SkPicture> copy(fDebugger.copyPicture());

    copy->serialize(&file);
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
    fPause = isPaused;
    fPausedRow = fListWidget.currentRow();
    fCanvasWidget.drawTo(fPausedRow);
    fImageWidget.draw();
}

void SkDebuggerGUI::registerListClick(QListWidgetItem *item) {
    if(!fLoading) {
        int currentRow = fListWidget.currentRow();

        if (currentRow != -1) {
            if (!fPause) {
                fCanvasWidget.drawTo(currentRow);
                fImageWidget.draw();
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
            setupClipStackText();
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

    fActionToggleIndexStyle.setShortcut(QKeySequence(tr("Ctrl+T")));
    fActionToggleIndexStyle.setText("Toggle Index Style");

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

    fImageWidget.setFixedSize(SkImageWidget::kImageWidgetWidth,
                              SkImageWidget::kImageWidgetHeight);

    fInspectorWidget.setSizePolicy(QSizePolicy::Expanding,
            QSizePolicy::Expanding);
    fInspectorWidget.setMaximumHeight(300);

    fSettingsAndImageLayout.setSpacing(6);
    fSettingsAndImageLayout.addWidget(&fSettingsWidget);
    fSettingsAndImageLayout.addWidget(&fImageWidget);

    fSettingsWidget.setSizePolicy(QSizePolicy::Expanding,
            QSizePolicy::Expanding);
    fSettingsWidget.setMaximumWidth(250);

    fLeftColumnSplitter.addWidget(&fListWidget);
    fLeftColumnSplitter.addWidget(&fDirectoryWidget);
    fLeftColumnSplitter.setOrientation(Qt::Vertical);

    fCanvasSettingsAndImageLayout.setSpacing(6);
    fCanvasSettingsAndImageLayout.addWidget(&fCanvasWidget);
    fCanvasSettingsAndImageLayout.addLayout(&fSettingsAndImageLayout);

    fMainAndRightColumnLayout.setSpacing(6);
    fMainAndRightColumnLayout.addLayout(&fCanvasSettingsAndImageLayout);
    fMainAndRightColumnLayout.addWidget(&fInspectorWidget);
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
    fMenuView.addAction(&fActionToggleIndexStyle);
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

    fPause = false;

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
    SkStream* stream = SkNEW_ARGS(SkFILEStream, (fileName.c_str()));

    SkPicture* picture = SkPicture::CreateFromStream(stream);

    if (NULL == picture) {
        QMessageBox::critical(this, "Error loading file", "Couldn't read file, sorry.");
        SkSafeUnref(stream);
        return;
    }

    fCanvasWidget.resetWidgetTransform();
    fDebugger.loadPicture(picture);

    fSkipCommands.setCount(fDebugger.getSize());
    for (int i = 0; i < fSkipCommands.count(); ++i) {
        fSkipCommands[i] = false;
    }

    SkSafeUnref(stream);
    SkSafeUnref(picture);

    // Will this automatically clear out due to nature of refcnt?
    SkTArray<SkString>* commands = fDebugger.getDrawCommandsAsStrings();
    SkTDArray<size_t>* offsets = fDebugger.getDrawCommandOffsets();
    SkASSERT(commands->count() == offsets->count());

    fActionProfile.setDisabled(false);

    /* fDebugCanvas is reinitialized every load picture. Need it to retain value
     * of the visibility filter.
     * TODO(chudy): This should be deprecated since fDebugger is not
     * recreated.
     * */
    fDebugger.highlightCurrentCommand(fSettingsWidget.getVisibilityFilter());

    this->setupListWidget(commands, offsets);
    this->setupComboBox(commands);
    this->setupOverviewText(NULL, 0.0, 1);
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

void SkDebuggerGUI::setupListWidget(SkTArray<SkString>* commands, SkTDArray<size_t>* offsets) {
    SkASSERT(commands->count() == offsets->count());
    fListWidget.clear();
    int counter = 0;
    int indent = 0;
    for (int i = 0; i < commands->count(); i++) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setData(Qt::DisplayRole, (*commands)[i].c_str());
        item->setData(Qt::UserRole + 1, counter++);

        if (0 == strcmp("Restore", (*commands)[i].c_str()) ||
            0 == strcmp("EndCommentGroup", (*commands)[i].c_str()) ||
            0 == strcmp("PopCull", (*commands)[i].c_str())) {
            indent -= 10;
        }

        item->setData(Qt::UserRole + 3, indent);

        if (0 == strcmp("Save", (*commands)[i].c_str()) ||
            0 == strcmp("Save Layer", (*commands)[i].c_str()) ||
            0 == strcmp("BeginCommentGroup", (*commands)[i].c_str()) ||
            0 == strcmp("PushCull", (*commands)[i].c_str())) {
            indent += 10;
        }

        item->setData(Qt::UserRole + 4, -1);
        item->setData(Qt::UserRole + 5, (int)(*offsets)[i]);

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

void SkDebuggerGUI::setupClipStackText() {
    SkString clipStack;
    fDebugger.getClipStackText(&clipStack);
    fInspectorWidget.setText(clipStack.c_str(), SkInspectorWidget::kClipStack_TabType);
}

void SkDebuggerGUI::setupComboBox(SkTArray<SkString>* command) {
    fFilter.clear();
    fFilter.addItem("--Filter By Available Commands--");

    std::map<std::string, int> map;
    for (int i = 0; i < command->count(); i++) {
        map[(*command)[i].c_str()]++;
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
