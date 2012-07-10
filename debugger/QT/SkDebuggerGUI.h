
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKDEBUGGERUI_H
#define SKDEBUGGERUI_H


#include "SkCanvas.h"
#include "SkDebugCanvas.h"
#include "SkListWidget.h"
#include "SkInspectorWidget.h"
#include "SkCanvasWidget.h"
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
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <vector>

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

signals:
    void commandChanged(int command);

private slots:
    /**
        Toggles breakpoint view in the list widget.
     */
    void actionBreakpoints();

    /**
        Cancels the command filter in the list widget.
     */
    void actionCancel();

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
        Resets all deleted commands.
     */
    void actionReload();

    /**
        Rewinds from the current step back to the start of the commands.
     */
    void actionRewind();

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
        Loads an skpicture selected from the directory.
     */
    void loadFile(QListWidgetItem *item);

    /**
        Toggles a dialog with a file browser for navigating to a skpicture. Loads
        the seleced file.
     */
    void openFile();

    /**
        Toggles whether drawing to a new command requires a double click
        or simple focus.
     */
    void pauseDrawing(bool isPaused);

    /**
        Executes draw commands up to the selected command
     */
    void registerListClick(QListWidgetItem *item);

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
    QAction fActionOpen;
    QAction fActionBreakpoint;
    QAction fActionCancel;
    QAction fActionClose;
    QAction fActionDelete;
    QAction fActionDirectory;
    QAction fActionGoToLine;
    QAction fActionInspector;
    QAction fActionPlay;
    QAction fActionReload;
    QAction fActionRewind;
    QAction fActionSettings;
    QAction fActionStepBack;
    QAction fActionStepForward;
    QWidget fCentralWidget;

    QComboBox fFilter;

    QVBoxLayout fLeftColumnLayout;
    QVBoxLayout fMainAndRightColumnLayout;
    QHBoxLayout fContainerLayout;
    QHBoxLayout fCanvasAndSettingsLayout;

    QListWidget fListWidget;
    QListWidget fDirectoryWidget;

    SkCanvasWidget fCanvasWidget;
    SkInspectorWidget fInspectorWidget;
    SkSettingsWidget fSettingsWidget;
    QStatusBar fStatusBar;

    QString fPath;
    bool fDirectoryWidgetActive;

    QMenuBar fMenuBar;
    QMenu fMenuFile;
    QMenu fMenuNavigate;
    QMenu fMenuView;
    QToolBar fToolBar;

    bool fBreakpointsActivated;
    bool fPause;

    /**
        Creates the entire UI.
     */
    void setupUi(QMainWindow *SkDebuggerGUI);

    /**
        Pipes a QString in with the location of the filename, proceeds to updating
        the listwidget, combowidget and inspectorwidget.
     */
    void loadPicture(QString fileName);

    /**
        Populates the list widget with the vector of strings passed in.
     */
    void setupListWidget(std::vector<std::string>* cv);

    /**
        Populates the combo box widget with the vector of strings passed in.
     */
    void setupComboBox(std::vector<std::string>* cv);

    /**
        Updates the directory widget with the latest directory path stored in
        the global class variable fPath.
     */
    void setupDirectoryWidget();
};

#endif // SKDEBUGGERUI_H
