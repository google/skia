/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef QCMake_h
#define QCMake_h
#ifdef _MSC_VER
#pragma warning ( disable : 4127 )
#pragma warning ( disable : 4512 )
#endif

#include <QObject>
#include <QString>
#include <QVariant>
#include <QList>
#include <QStringList>
#include <QMetaType>
#include <QAtomicInt>

class cmake;

/// struct to represent cmake properties in Qt
/// Value is of type String or Bool
struct QCMakeProperty
{
  enum PropertyType { BOOL, PATH, FILEPATH, STRING };
  QString Key;
  QVariant Value;
  QStringList Strings;
  QString Help;
  PropertyType Type;
  bool Advanced;
  bool operator==(const QCMakeProperty& other) const
    {
    return this->Key == other.Key;
    }
  bool operator<(const QCMakeProperty& other) const
    {
    return this->Key < other.Key;
    }
};

// list of properties
typedef QList<QCMakeProperty> QCMakePropertyList;

// allow QVariant to be a property or list of properties
Q_DECLARE_METATYPE(QCMakeProperty)
Q_DECLARE_METATYPE(QCMakePropertyList)

/// Qt API for CMake library.
/// Wrapper like class allows for easier integration with
/// Qt features such as, signal/slot connections, multi-threading, etc..
class QCMake : public QObject
{
  Q_OBJECT
public:
  QCMake(QObject* p=0);
  ~QCMake();
public slots:
  /// load the cache file in a directory
  void loadCache(const QString& dir);
  /// set the source directory containing the source
  void setSourceDirectory(const QString& dir);
  /// set the binary directory to build in
  void setBinaryDirectory(const QString& dir);
  /// set the desired generator to use
  void setGenerator(const QString& generator);
  /// do the configure step
  void configure();
  /// generate the files
  void generate();
  /// set the property values
  void setProperties(const QCMakePropertyList&);
  /// interrupt the configure or generate process (if connecting, make a direct connection)
  void interrupt();
  /// delete the cache in binary directory
  void deleteCache();
  /// reload the cache in binary directory
  void reloadCache();
  /// set whether to do debug output
  void setDebugOutput(bool);
  /// set whether to do suppress dev warnings
  void setSuppressDevWarnings(bool value);
  /// set whether to run cmake with warnings about uninitialized variables
  void setWarnUninitializedMode(bool value);
  /// set whether to run cmake with warnings about unused variables
  void setWarnUnusedMode(bool value);

public:
  /// get the list of cache properties
  QCMakePropertyList properties() const;
  /// get the current binary directory
  QString binaryDirectory() const;
  /// get the current source directory
  QString sourceDirectory() const;
  /// get the current generator
  QString generator() const;
  /// get the available generators
  QStringList availableGenerators() const;
  /// get whether to do debug output
  bool getDebugOutput() const;

signals:
  /// signal when properties change (during read from disk or configure process)
  void propertiesChanged(const QCMakePropertyList& vars);
  /// signal when the generator changes
  void generatorChanged(const QString& gen);
  /// signal when the source directory changes (binary directory already
  /// containing a CMakeCache.txt file)
  void sourceDirChanged(const QString& dir);
  /// signal when the binary directory changes
  void binaryDirChanged(const QString& dir);
  /// signal for progress events
  void progressChanged(const QString& msg, float percent);
  /// signal when configure is done
  void configureDone(int error);
  /// signal when generate is done
  void generateDone(int error);
  /// signal when there is an output message
  void outputMessage(const QString& msg);
  /// signal when there is an error message
  void errorMessage(const QString& msg);
  /// signal when debug output changes
  void debugOutputChanged(bool);

protected:
  cmake* CMakeInstance;

  static bool interruptCallback(void*);
  static void progressCallback(const char* msg, float percent, void* cd);
  static void messageCallback(const char* msg, const char* title,
                              bool&, void* cd);
  static void stdoutCallback(const char* msg, size_t len, void* cd);
  static void stderrCallback(const char* msg, size_t len, void* cd);
  bool SuppressDevWarnings;
  bool WarnUninitializedMode;
  bool WarnUnusedMode;
  bool WarnUnusedAllMode;
  QString SourceDirectory;
  QString BinaryDirectory;
  QString Generator;
  QStringList AvailableGenerators;
  QString CMakeExecutable;
  QAtomicInt InterruptFlag;
};

#endif // QCMake_h

