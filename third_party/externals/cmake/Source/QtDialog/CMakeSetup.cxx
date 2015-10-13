/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "QCMake.h"  // include to disable MS warnings
#include <QApplication>
#include <QDir>
#include <QTranslator>
#include <QLocale>
#include <QTextCodec>
#include "CMakeSetupDialog.h"
#include "cmDocumentation.h"
#include "cmake.h"
#include "cmVersion.h"
#include "cmAlgorithms.h"
#include <cmsys/CommandLineArguments.hxx>
#include <cmsys/SystemTools.hxx>
#include <cmsys/Encoding.hxx>

//----------------------------------------------------------------------------
static const char * cmDocumentationName[][2] =
{
  {0,
   "  cmake-gui - CMake GUI."},
  {0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationUsage[][2] =
{
  {0,
   "  cmake-gui [options]\n"
   "  cmake-gui [options] <path-to-source>\n"
   "  cmake-gui [options] <path-to-existing-build>"},
  {0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationOptions[][2] =
{
  {0,0}
};

#if defined(Q_OS_MAC)
static int cmOSXInstall(std::string dir);
#endif

int main(int argc, char** argv)
{
  cmsys::Encoding::CommandLineArguments encoding_args =
    cmsys::Encoding::CommandLineArguments::Main(argc, argv);
  int argc2 = encoding_args.argc();
  char const* const* argv2 = encoding_args.argv();

  cmSystemTools::FindCMakeResources(argv2[0]);
  // check docs first so that X is not need to get docs
  // do docs, if args were given
  cmDocumentation doc;
  doc.addCMakeStandardDocSections();
  if(argc2 >1 && doc.CheckOptions(argc2, argv2))
    {
    // Construct and print requested documentation.
    cmake hcm;
    hcm.SetHomeDirectory("");
    hcm.SetHomeOutputDirectory("");
    hcm.AddCMakePaths();

    std::vector<cmDocumentationEntry> generators;
    hcm.GetGeneratorDocumentation(generators);
    doc.SetName("cmake");
    doc.SetSection("Name",cmDocumentationName);
    doc.SetSection("Usage",cmDocumentationUsage);
    doc.AppendSection("Generators",generators);
    doc.PrependSection("Options",cmDocumentationOptions);

    return (doc.PrintRequestedDocumentation(std::cout)? 0:1);
    }

#if defined(Q_OS_MAC)
  if (argc2 == 2 && strcmp(argv2[1], "--install") == 0)
    {
    return cmOSXInstall("/usr/local/bin");
    }
  if (argc2 == 2 && cmHasLiteralPrefix(argv2[1], "--install="))
    {
    return cmOSXInstall(argv2[1]+10);
    }
#endif

  QApplication app(argc, argv);

#if defined(CMAKE_ENCODING_UTF8)
  QTextCodec* utf8_codec = QTextCodec::codecForName("UTF-8");
  QTextCodec::setCodecForLocale(utf8_codec);
#endif

  // clean out standard Qt paths for plugins, which we don't use anyway
  // when creating Mac bundles, it potentially causes problems
  foreach(QString p, QApplication::libraryPaths())
    {
    QApplication::removeLibraryPath(p);
    }

  // tell the cmake library where cmake is
  QDir cmExecDir(QApplication::applicationDirPath());
#if defined(Q_OS_MAC)
  cmExecDir.cd("../../../");
#endif

  // pick up translation files if they exists in the data directory
  QDir translationsDir = cmExecDir;
  translationsDir.cd(QString::fromLocal8Bit(".." CMAKE_DATA_DIR));
  translationsDir.cd("i18n");
  QTranslator translator;
  QString transfile = QString("cmake_%1").arg(QLocale::system().name());
  translator.load(transfile, translationsDir.path());
  app.installTranslator(&translator);

  // app setup
  app.setApplicationName("CMakeSetup");
  app.setOrganizationName("Kitware");
  QIcon appIcon;
  appIcon.addFile(":/Icons/CMakeSetup32.png");
  appIcon.addFile(":/Icons/CMakeSetup128.png");
  app.setWindowIcon(appIcon);

  CMakeSetupDialog dialog;
  dialog.show();

  cmsys::CommandLineArguments arg;
  arg.Initialize(argc2, argv2);
  std::string binaryDirectory;
  std::string sourceDirectory;
  typedef cmsys::CommandLineArguments argT;
  arg.AddArgument("-B", argT::CONCAT_ARGUMENT,
                  &binaryDirectory, "Binary Directory");
  arg.AddArgument("-H", argT::CONCAT_ARGUMENT,
                  &sourceDirectory, "Source Directory");
  // do not complain about unknown options
  arg.StoreUnusedArguments(true);
  arg.Parse();
  if(!sourceDirectory.empty() && !binaryDirectory.empty())
    {
    dialog.setSourceDirectory(QString::fromLocal8Bit(sourceDirectory.c_str()));
    dialog.setBinaryDirectory(QString::fromLocal8Bit(binaryDirectory.c_str()));
    }
  else
    {
    QStringList args = app.arguments();
    if(args.count() == 2)
      {
      std::string filePath = cmSystemTools::CollapseFullPath(args[1].toLocal8Bit().data());

      // check if argument is a directory containing CMakeCache.txt
      std::string buildFilePath =
        cmSystemTools::CollapseFullPath("CMakeCache.txt", filePath.c_str());

      // check if argument is a CMakeCache.txt file
      if(cmSystemTools::GetFilenameName(filePath) == "CMakeCache.txt" &&
         cmSystemTools::FileExists(filePath.c_str()))
        {
        buildFilePath = filePath;
        }

      // check if argument is a directory containing CMakeLists.txt
      std::string srcFilePath =
        cmSystemTools::CollapseFullPath("CMakeLists.txt", filePath.c_str());

      if(cmSystemTools::FileExists(buildFilePath.c_str()))
        {
        dialog.setBinaryDirectory(
          QString::fromLocal8Bit(
            cmSystemTools::GetFilenamePath(buildFilePath).c_str()
            )
          );
        }
      else if(cmSystemTools::FileExists(srcFilePath.c_str()))
        {
        dialog.setSourceDirectory(QString::fromLocal8Bit(filePath.c_str()));
        dialog.setBinaryDirectory(
          QString::fromLocal8Bit(cmSystemTools::CollapseFullPath(".").c_str())
          );
        }
      }
    }

  return app.exec();
}

#if defined(Q_OS_MAC)
# include <errno.h>
# include <string.h>
# include <sys/stat.h>
# include <unistd.h>
static bool cmOSXInstall(std::string const& dir, std::string const& tool)
{
  if (tool.empty())
    {
    return true;
    }
  std::string link = dir + cmSystemTools::GetFilenameName(tool);
  struct stat st;
  if (lstat(link.c_str(), &st) == 0 && S_ISLNK(st.st_mode))
    {
    char buf[4096];
    ssize_t s = readlink(link.c_str(), buf, sizeof(buf)-1);
    if (s >= 0 && std::string(buf, s) == tool)
      {
      std::cerr << "Exists: '" << link << "' -> '" << tool << "'\n";
      return true;
      }
    }
  if (symlink(tool.c_str(), link.c_str()) == 0)
    {
    std::cerr << "Linked: '" << link << "' -> '" << tool << "'\n";
    return true;
    }
  else
    {
    int err = errno;
    std::cerr << "Failed: '" << link << "' -> '" << tool << "': "
              << strerror(err) << "\n";
    return false;
    }
}
static int cmOSXInstall(std::string dir)
{
  if (!cmHasLiteralSuffix(dir, "/"))
    {
    dir += "/";
    }
  return (
    cmOSXInstall(dir, cmSystemTools::GetCMakeCommand()) &&
    cmOSXInstall(dir, cmSystemTools::GetCTestCommand()) &&
    cmOSXInstall(dir, cmSystemTools::GetCPackCommand()) &&
    cmOSXInstall(dir, cmSystemTools::GetCMakeGUICommand()) &&
    cmOSXInstall(dir, cmSystemTools::GetCMakeCursesCommand())
    ) ? 0 : 1;
}
#endif
