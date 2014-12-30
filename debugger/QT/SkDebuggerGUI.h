
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKDEBUGGERUI_H
#define SKDEBUGGERUI_H


#include "SkCanvas.h"
#include "SkCanvasWidget.h"
#include "SkDebugger.h"
#include "SkGLWidget.h"
#include "SkListWidget.h"
#include "SkInspectorWidget.h"
#include "SkRasterWidget.h"
#include "SkImageWidget.h"
#include "SkSettingsWidget.h"
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QListView>
#include <QtGui/QListWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QSplitter>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <vector>

class SkTimedPicture;
namespace sk_tools {
    class PictureRenderer;
}

/** \class SkDebuggerGUI

    Container for the UI and it's functions.
 */
class SkDebuggerGUI : public QMainWindow {
    Q_OBJECT

public:
    /**
        Constructs the view of the application.
        @param parent  The parent container of this widget.
     */
    SkDebuggerGUI(QWidget *parent = 0);

    ~SkDebuggerGUI();

    /**
        Updates the directory widget with the latest directory path stored in
        the global class variable fPath.
     */
    void setupDirectoryWidget(const QString& path);

    /**
        Loads the specified file.
    */
    void openFile(const QString& filename);

signals:
    void commandChanged(int command);

private slots:
    /**
        Toggles breakpoint view in the list widget.
     */
    void actionBreakpoints();

    /**
        Toggles between count and offset style of command indexing in GUI
     */
    void actionToggleIndexStyle();

    /**
        Profile the commands
     */
    void actionProfile();

    /**
        Cancels the command filter in the list widget.
     */
    void actionCancel();

    /**
        Clears the breakpoint state off of all commands marked as breakpoints.
     */
    void actionClearBreakpoints();

    /**
        Clears the deleted state off of all commands marked as deleted.
     */
    void actionClearDeletes();

    /**
        Applies a visible filter to all drawing commands other than the previous.
     */
    void actionCommandFilter();

    /**
        Closes the application.
     */
    void actionClose();

    /**
        Deletes the command in question.
     */
    void actionDelete();

#if SK_SUPPORT_GPU
    /**
        Updates the visibility of the GL canvas widget and sample count of the GL surface.
     */
    void actionGLWidget();
#endif

    /**
        Toggles the visibility of the inspector widget.
     */
    void actionInspector();

    /**
        Plays from the current step to the next breakpoint if it exists, otherwise
        executes all remaining draw commands.
     */
    void actionPlay();

    /**
        Toggles the visibility of the raster canvas widget.
     */
    void actionRasterWidget(bool isToggled);

    /**
        Toggles the the overdraw visualization on and off
     */
    void actionOverdrawVizWidget(bool isToggled);

    /**
        Toggles the the mega visualization on and off
     */
    void actionMegaVizWidget(bool isToggled);

    /**
        Toggles using path ops to simplify the clip stack
     */
    void actionPathOpsWidget(bool );

    /**
        Applies the new texture filter override
     */
    void actionTextureFilter();

    /**
        Rewinds from the current step back to the start of the commands.
     */
    void actionRewind();

    /**
        Saves the current SKP with all modifications.
     */
    void actionSave();

    /**
        Saves the current SKP under a different name and/or location.
     */
    void actionSaveAs();

    /**
        Sends the scale factor information to the settings widget.
     */
    void actionScale(float scaleFactor);

    /**
        Toggles the settings widget visibility.
     */
    void actionSettings();

    /**
        Steps forward to the next draw command.
     */
    void actionStepBack();

    /**
        Steps backwards to the next draw command.
     */
    void actionStepForward();

    /**
        Called when the canvas is done being drawn to by SkCanvasWidget.
     */
    void drawComplete();

    /**
        Loads an skpicture selected from the directory.
     */
    void loadFile(QListWidgetItem *item);

    /**
        Toggles a dialog with a file browser for navigating to a skpicture. Loads
        the selected file.
     */
    void openFile();

    /**
        Toggles whether drawing to a new command requires a double click
        or simple focus.
     */
    void pauseDrawing(bool isPaused = true);

    /**
        Executes draw commands up to the selected command
     */
    void registerListClick(QListWidgetItem *item);

    /**
        Sets the command to active in the list widget.
     */
    void selectCommand(int command);

    /**
        Toggles the exclusive listing of commands set as deleted.
     */
    void showDeletes();

    /**
        Toggles a breakpoint on the current step in the list widget.
     */
    void toggleBreakpoint();

    /**
        Toggles the visibility of the directory widget.
     */
    void toggleDirectory();

    /**
        Filters the list widgets command visibility based on the currently
        active selection.
     */
    void toggleFilter(QString string);

private:
    QSplitter fCentralSplitter;
    QStatusBar fStatusBar;
    QToolBar fToolBar;

    QAction fActionOpen;
    QAction fActionBreakpoint;
    QAction fActionToggleIndexStyle;
    QAction fActionProfile;
    QAction fActionCancel;
    QAction fActionClearBreakpoints;
    QAction fActionClearDeletes;
    QAction fActionClose;
    QAction fActionCreateBreakpoint;
    QAction fActionDelete;
    QAction fActionDirectory;
    QAction fActionGoToLine;
    QAction fActionInspector;
    QAction fActionSettings;
    QAction fActionPlay;
    QAction fActionPause;
    QAction fActionRewind;
    QAction fActionSave;
    QAction fActionSaveAs;
    QAction fActionShowDeletes;
    QAction fActionStepBack;
    QAction fActionStepForward;
    QAction fActionZoomIn;
    QAction fActionZoomOut;
    QSignalMapper fMapper;

    QWidget fSpacer;
    QComboBox fFilter;

    QSplitter fLeftColumnSplitter;
    QWidget fMainAndRightColumnWidget;
    QVBoxLayout fMainAndRightColumnLayout;
    QHBoxLayout fCanvasSettingsAndImageLayout;
    QVBoxLayout fSettingsAndImageLayout;

    QListWidget fListWidget;
    QListWidget fDirectoryWidget;

    SkDebugger fDebugger;
    SkCanvasWidget fCanvasWidget;
    SkImageWidget fImageWidget;
    SkInspectorWidget fInspectorWidget;
    SkSettingsWidget fSettingsWidget;

    QString fPath;
    SkString fFileName;
    SkTDArray<bool> fSkipCommands; // has a specific command been deleted?
    bool fDirectoryWidgetActive;

    QMenuBar fMenuBar;
    QMenu fMenuFile;
    QMenu fMenuEdit;
    QMenu fMenuNavigate;
    QMenu fMenuView;
    QMenu fMenuWindows;

    bool fBreakpointsActivated;
    bool fIndexStyleToggle;
    bool fDeletesActivated;
    bool fPause;
    bool fLoading;
    int fPausedRow;

    /**
        Creates the entire UI.
     */
    void setupUi(QMainWindow *SkDebuggerGUI);

    /**
        Pipes a QString in with the location of the filename, proceeds to updating
        the listwidget, combowidget and inspectorwidget.
     */
    void loadPicture(const SkString& fileName);

    /**
        Creates a picture of the current canvas.
     */
    void saveToFile(const SkString& filename);

    /**
        Populates the list widget with the debugger draw command info.
     */
    void setupListWidget();

    /**
        Populates the combo box widget with with the debugger draw command info.
     */
    void setupComboBox();

    /**
        Fills in the overview pane with text
     */
    void setupOverviewText(const SkTDArray<double>* typeTimes, double totTime, int numRuns);

    /**
        Fills in the clip stack pane with text
     */
    void setupClipStackText();

    /**
        Render the supplied picture several times tracking the time consumed
        by each command.
     */
    void run(const SkPicture* pict,
             sk_tools::PictureRenderer* renderer,
             int repeats);
};

#endif // SKDEBUGGERUI_H
