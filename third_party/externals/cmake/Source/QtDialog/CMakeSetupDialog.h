/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef CMakeSetupDialog_h
#define CMakeSetupDialog_h

#include "QCMake.h"
#include <QMainWindow>
#include <QThread>
#include <QEventLoop>
#include "ui_CMakeSetupDialog.h"

class QCMakeThread;
class CMakeCacheModel;
class QProgressBar;
class QToolButton;

/// Qt user interface for CMake
class CMakeSetupDialog : public QMainWindow, public Ui::CMakeSetupDialog
{
  Q_OBJECT
public:
  CMakeSetupDialog();
  ~CMakeSetupDialog();

public slots:
  void setBinaryDirectory(const QString& dir);
  void setSourceDirectory(const QString& dir);

protected slots:
  void initialize();
  void doConfigure();
  void doGenerate();
  void doInstallForCommandLine();
  void doHelp();
  void doAbout();
  void doInterrupt();
  void error(const QString& message);
  void message(const QString& message);

  void doSourceBrowse();
  void doBinaryBrowse();
  void doReloadCache();
  void doDeleteCache();
  void updateSourceDirectory(const QString& dir);
  void updateBinaryDirectory(const QString& dir);
  void showProgress(const QString& msg, float percent);
  void setEnabledState(bool);
  bool setupFirstConfigure();
  void updateGeneratorLabel(const QString& gen);
  void setExitAfterGenerate(bool);
  void addBinaryPath(const QString&);
  QStringList loadBuildPaths();
  void saveBuildPaths(const QStringList&);
  void onBinaryDirectoryChanged(const QString& dir);
  void onSourceDirectoryChanged(const QString& dir);
  void setCacheModified();
  void removeSelectedCacheEntries();
  void selectionChanged();
  void addCacheEntry();
  void startSearch();
  void setDebugOutput(bool);
  void setAdvancedView(bool);
  void setGroupedView(bool);
  void showUserChanges();
  void setSearchFilter(const QString& str);
  bool prepareConfigure();
  bool doConfigureInternal();
  bool doGenerateInternal();
  void exitLoop(int);
  void doOutputContextMenu(const QPoint &);
  void doOutputFindDialog();
  void doOutputFindNext(bool directionForward = true);
  void doOutputFindPrev();
  void doOutputErrorNext();

protected:

  enum State { Interrupting, ReadyConfigure, ReadyGenerate, Configuring, Generating };
  void enterState(State s);

  void closeEvent(QCloseEvent*);
  void dragEnterEvent(QDragEnterEvent*);
  void dropEvent(QDropEvent*);

  QCMakeThread* CMakeThread;
  bool ExitAfterGenerate;
  bool CacheModified;
  bool ConfigureNeeded;
  QAction* ReloadCacheAction;
  QAction* DeleteCacheAction;
  QAction* ExitAction;
  QAction* ConfigureAction;
  QAction* GenerateAction;
  QAction* SuppressDevWarningsAction;
  QAction* WarnUninitializedAction;
  QAction* WarnUnusedAction;
  QAction* InstallForCommandLineAction;
  State CurrentState;

  QTextCharFormat ErrorFormat;
  QTextCharFormat MessageFormat;

  QStringList AddVariableNames;
  QStringList AddVariableTypes;
  QStringList FindHistory;

  QEventLoop LocalLoop;

  float ProgressOffset;
  float ProgressFactor;
};

// QCMake instance on a thread
class QCMakeThread : public QThread
{
  Q_OBJECT
public:
  QCMakeThread(QObject* p);
  QCMake* cmakeInstance() const;

signals:
  void cmakeInitialized();

protected:
  virtual void run();
  QCMake* CMakeInstance;
};

#endif // CMakeSetupDialog_h
