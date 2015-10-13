/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCPackPackageMakerGenerator.h"

#include "cmake.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmCPackComponentGroup.h"
#include "cmCPackLog.h"

#include <cmsys/SystemTools.hxx>
#include <cmsys/Glob.hxx>
#include <cmsys/FStream.hxx>

#include <assert.h>

static inline
unsigned int getVersion(unsigned int major, unsigned int minor)
{
  assert(major < 256 && minor < 256);
  return ((major & 0xFF) << 16 | minor);
}

//----------------------------------------------------------------------
cmCPackPackageMakerGenerator::cmCPackPackageMakerGenerator()
{
  this->PackageMakerVersion = 0.0;
  this->PackageCompatibilityVersion = getVersion(10, 4);
}

//----------------------------------------------------------------------
cmCPackPackageMakerGenerator::~cmCPackPackageMakerGenerator()
{
}

//----------------------------------------------------------------------
bool cmCPackPackageMakerGenerator::SupportsComponentInstallation() const
{
  return this->PackageCompatibilityVersion >= getVersion(10, 4);
}

//----------------------------------------------------------------------
int cmCPackPackageMakerGenerator::CopyInstallScript(const std::string& resdir,
                                                    const std::string& script,
                                                    const std::string& name)
{
  std::string dst = resdir;
  dst += "/";
  dst += name;
  cmSystemTools::CopyFileAlways(script.c_str(), dst.c_str());
  cmSystemTools::SetPermissions(dst.c_str(),0777);
  cmCPackLogger(cmCPackLog::LOG_VERBOSE,
                "copy script : " << script << "\ninto " << dst.c_str() <<
                std::endl);

  return 1;
}

//----------------------------------------------------------------------
int cmCPackPackageMakerGenerator::PackageFiles()
{
  // TODO: Use toplevel
  //       It is used! Is this an obsolete comment?

  std::string resDir; // Where this package's resources will go.
  std::string packageDirFileName
    = this->GetOption("CPACK_TEMPORARY_DIRECTORY");
  if (this->Components.empty())
    {
    packageDirFileName += ".pkg";
    resDir = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
    resDir += "/Resources";
    }
  else
    {
    packageDirFileName += ".mpkg";
    if ( !cmsys::SystemTools::MakeDirectory(packageDirFileName.c_str()))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "unable to create package directory "
                    << packageDirFileName << std::endl);
        return 0;
      }

    resDir = packageDirFileName;
    resDir += "/Contents";
    if ( !cmsys::SystemTools::MakeDirectory(resDir.c_str()))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "unable to create package subdirectory " << resDir
                    << std::endl);
        return 0;
      }

    resDir += "/Resources";
    if ( !cmsys::SystemTools::MakeDirectory(resDir.c_str()))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "unable to create package subdirectory " << resDir
                    << std::endl);
        return 0;
      }

    resDir += "/en.lproj";
    }

  const char* preflight = this->GetOption("CPACK_PREFLIGHT_SCRIPT");
  const char* postflight = this->GetOption("CPACK_POSTFLIGHT_SCRIPT");
  const char* postupgrade = this->GetOption("CPACK_POSTUPGRADE_SCRIPT");

  if(this->Components.empty())
    {
    // Create directory structure
    std::string preflightDirName = resDir + "/PreFlight";
    std::string postflightDirName = resDir + "/PostFlight";
    // if preflight or postflight scripts not there create directories
    // of the same name, I think this makes it work
    if(!preflight)
      {
      if ( !cmsys::SystemTools::MakeDirectory(preflightDirName.c_str()))
        {
        cmCPackLogger(cmCPackLog::LOG_ERROR,
                      "Problem creating installer directory: "
                      << preflightDirName.c_str() << std::endl);
        return 0;
        }
      }
    if(!postflight)
      {
      if ( !cmsys::SystemTools::MakeDirectory(postflightDirName.c_str()))
        {
        cmCPackLogger(cmCPackLog::LOG_ERROR,
                      "Problem creating installer directory: "
                      << postflightDirName.c_str() << std::endl);
        return 0;
        }
      }
    // if preflight, postflight, or postupgrade are set
    // then copy them into the resource directory and make
    // them executable
    if(preflight)
      {
      this->CopyInstallScript(resDir.c_str(),
                              preflight,
                              "preflight");
      }
    if(postflight)
      {
      this->CopyInstallScript(resDir.c_str(),
                              postflight,
                              "postflight");
      }
    if(postupgrade)
      {
      this->CopyInstallScript(resDir.c_str(),
                              postupgrade,
                              "postupgrade");
      }
    }
  else if(postflight)
    {
    // create a postflight component to house the script
    this->PostFlightComponent.Name = "PostFlight";
    this->PostFlightComponent.DisplayName = "PostFlight";
    this->PostFlightComponent.Description = "PostFlight";
    this->PostFlightComponent.IsHidden = true;

    // empty directory for pkg contents
    std::string packageDir = toplevel + "/" + PostFlightComponent.Name;
    if (!cmsys::SystemTools::MakeDirectory(packageDir.c_str()))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Problem creating component packages directory: "
                    << packageDir.c_str() << std::endl);
      return 0;
      }

    // create package
    std::string packageFileDir = packageDirFileName + "/Contents/Packages/";
    if (!cmsys::SystemTools::MakeDirectory(packageFileDir.c_str()))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                   "Problem creating component PostFlight Packages directory: "
                    << packageFileDir.c_str() << std::endl);
      return 0;
      }
    std::string packageFile = packageFileDir +
      this->GetPackageName(PostFlightComponent);
    if (!this->GenerateComponentPackage(packageFile.c_str(),
                                        packageDir.c_str(),
                                        PostFlightComponent))
      {
      return 0;
      }

    // copy postflight script into resource directory of .pkg
    std::string resourceDir = packageFile + "/Contents/Resources";
    this->CopyInstallScript(resourceDir.c_str(),
                            postflight,
                            "postflight");
    }

  if (!this->Components.empty())
    {
    // Create the directory where component packages will be built.
    std::string basePackageDir = packageDirFileName;
    basePackageDir += "/Contents/Packages";
    if (!cmsys::SystemTools::MakeDirectory(basePackageDir.c_str()))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Problem creating component packages directory: "
                    << basePackageDir.c_str() << std::endl);
      return 0;
      }

    // Create the directory where downloaded component packages will
    // be placed.
    const char* userUploadDirectory =
      this->GetOption("CPACK_UPLOAD_DIRECTORY");
    std::string uploadDirectory;
    if (userUploadDirectory && *userUploadDirectory)
      {
      uploadDirectory = userUploadDirectory;
      }
    else
      {
      uploadDirectory= this->GetOption("CPACK_PACKAGE_DIRECTORY");
      uploadDirectory += "/CPackUploads";
      }

    // Create packages for each component
    bool warnedAboutDownloadCompatibility = false;

    std::map<std::string, cmCPackComponent>::iterator compIt;
    for (compIt = this->Components.begin(); compIt != this->Components.end();
         ++compIt)
      {
      std::string packageFile;
      if (compIt->second.IsDownloaded)
        {
        if (this->PackageCompatibilityVersion >= getVersion(10, 5) &&
            this->PackageMakerVersion >= 3.0)
          {
          // Build this package within the upload directory.
          packageFile = uploadDirectory;

          if(!cmSystemTools::FileExists(uploadDirectory.c_str()))
            {
            if (!cmSystemTools::MakeDirectory(uploadDirectory.c_str()))
              {
              cmCPackLogger(cmCPackLog::LOG_ERROR,
                            "Unable to create package upload directory "
                            << uploadDirectory << std::endl);
              return 0;
              }
            }
          }
        else if (!warnedAboutDownloadCompatibility)
          {
            if (this->PackageCompatibilityVersion < getVersion(10, 5))
            {
            cmCPackLogger(
              cmCPackLog::LOG_WARNING,
              "CPack warning: please set CPACK_OSX_PACKAGE_VERSION to 10.5 "
              "or greater enable downloaded packages. CPack will build a "
              "non-downloaded package."
              << std::endl);
            }

          if (this->PackageMakerVersion < 3)
            {
            cmCPackLogger(cmCPackLog::LOG_WARNING,
                        "CPack warning: unable to build downloaded "
                          "packages with PackageMaker versions prior "
                          "to 3.0. CPack will build a non-downloaded package."
                          << std::endl);
            }

          warnedAboutDownloadCompatibility = true;
          }
        }

      if (packageFile.empty())
        {
        // Build this package within the overall distribution
        // metapackage.
        packageFile = basePackageDir;

        // We're not downloading this component, even if the user
        // requested it.
        compIt->second.IsDownloaded = false;
        }

      packageFile += '/';
      packageFile += GetPackageName(compIt->second);

      std::string packageDir = toplevel;
      packageDir += '/';
      packageDir += compIt->first;
      if (!this->GenerateComponentPackage(packageFile.c_str(),
                                          packageDir.c_str(),
                                          compIt->second))
        {
        return 0;
        }
      }
    }
  this->SetOption("CPACK_MODULE_VERSION_SUFFIX", "");

  // Copy or create all of the resource files we need.
  if ( !this->CopyCreateResourceFile("License", resDir.c_str())
       || !this->CopyCreateResourceFile("ReadMe", resDir.c_str())
       || !this->CopyCreateResourceFile("Welcome", resDir.c_str())
       || !this->CopyResourcePlistFile("Info.plist")
       || !this->CopyResourcePlistFile("Description.plist") )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem copying the resource files"
      << std::endl);
    return 0;
    }

  if (this->Components.empty())
    {
    // Use PackageMaker to build the package.
    std::ostringstream pkgCmd;
    pkgCmd << "\"" << this->GetOption("CPACK_INSTALLER_PROGRAM")
           << "\" -build -p \"" << packageDirFileName << "\"";
    if (this->Components.empty())
      {
      pkgCmd << " -f \"" << this->GetOption("CPACK_TEMPORARY_DIRECTORY");
      }
    else
      {
      pkgCmd << " -mi \"" << this->GetOption("CPACK_TEMPORARY_DIRECTORY")
             << "/packages/";
      }
    pkgCmd << "\" -r \"" << this->GetOption("CPACK_TOPLEVEL_DIRECTORY")
           << "/Resources\" -i \""
           << this->GetOption("CPACK_TOPLEVEL_DIRECTORY")
           << "/Info.plist\" -d \""
           << this->GetOption("CPACK_TOPLEVEL_DIRECTORY")
           << "/Description.plist\"";
    if ( this->PackageMakerVersion > 2.0 )
      {
      pkgCmd << " -v";
      }
    if (!RunPackageMaker(pkgCmd.str().c_str(), packageDirFileName.c_str()))
      return 0;
    }
  else
    {
    // We have built the package in place. Generate the
    // distribution.dist file to describe it for the installer.
    WriteDistributionFile(packageDirFileName.c_str());
    }

  std::string tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  tmpFile += "/hdiutilOutput.log";
  std::ostringstream dmgCmd;
  dmgCmd << "\"" << this->GetOption("CPACK_INSTALLER_PROGRAM_DISK_IMAGE")
    << "\" create -ov -format UDZO -srcfolder \"" << packageDirFileName
    << "\" \"" << packageFileNames[0] << "\"";
  std::string output;
  int retVal = 1;
  int numTries = 10;
  bool res = false;
  while(numTries > 0)
    {
    res = cmSystemTools::RunSingleCommand(
      dmgCmd.str().c_str(), &output, &output,
      &retVal, 0, this->GeneratorVerbose, 0);
    if ( res && !retVal )
      {
      numTries = -1;
      break;
      }
    cmSystemTools::Delay(500);
    numTries--;
    }
  if ( !res || retVal )
    {
    cmGeneratedFileStream ofs(tmpFile.c_str());
    ofs << "# Run command: " << dmgCmd.str().c_str() << std::endl
      << "# Output:" << std::endl
      << output.c_str() << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem running hdiutil command: "
      << dmgCmd.str().c_str() << std::endl
      << "Please check " << tmpFile.c_str() << " for errors" << std::endl);
    return 0;
    }

  return 1;
}

//----------------------------------------------------------------------
int cmCPackPackageMakerGenerator::InitializeInternal()
{
  cmCPackLogger(cmCPackLog::LOG_DEBUG,
    "cmCPackPackageMakerGenerator::Initialize()" << std::endl);
  this->SetOptionIfNotSet("CPACK_PACKAGING_INSTALL_PREFIX", "/usr");

  // Starting with Xcode 4.3, PackageMaker is a separate app, and you
  // can put it anywhere you want. So... use a variable for its location.
  // People who put it in unexpected places can use the variable to tell
  // us where it is.
  //
  // Use the following locations, in "most recent installation" order,
  // to search for the PackageMaker app. Assume people who copy it into
  // the new Xcode 4.3 app in "/Applications" will copy it into the nested
  // Applications folder inside the Xcode bundle itself. Or directly in
  // the "/Applications" directory.
  //
  // If found, save result in the CPACK_INSTALLER_PROGRAM variable.

  std::vector<std::string> paths;
  paths.push_back(
    "/Applications/Xcode.app/Contents/Applications"
    "/PackageMaker.app/Contents/MacOS");
  paths.push_back(
    "/Applications/Utilities"
    "/PackageMaker.app/Contents/MacOS");
  paths.push_back(
    "/Applications"
    "/PackageMaker.app/Contents/MacOS");
  paths.push_back(
    "/Developer/Applications/Utilities"
    "/PackageMaker.app/Contents/MacOS");
  paths.push_back(
    "/Developer/Applications"
    "/PackageMaker.app/Contents/MacOS");

  std::string pkgPath;
  const char *inst_program = this->GetOption("CPACK_INSTALLER_PROGRAM");
  if (inst_program && *inst_program)
    {
    pkgPath = inst_program;
    }
  else
    {
    pkgPath = cmSystemTools::FindProgram("PackageMaker", paths, false);
    if ( pkgPath.empty() )
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find PackageMaker compiler"
        << std::endl);
      return 0;
      }
    this->SetOptionIfNotSet("CPACK_INSTALLER_PROGRAM", pkgPath.c_str());
    }

  // Get path to the real PackageMaker, not a symlink:
  pkgPath = cmSystemTools::GetRealPath(pkgPath.c_str());
  // Up from there to find the version.plist file in the "Contents" dir:
  std::string contents_dir;
  contents_dir = cmSystemTools::GetFilenamePath(pkgPath);
  contents_dir = cmSystemTools::GetFilenamePath(contents_dir);

  std::string versionFile = contents_dir + "/version.plist";

  if ( !cmSystemTools::FileExists(versionFile.c_str()) )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Cannot find PackageMaker compiler version file: "
      << versionFile.c_str()
      << std::endl);
    return 0;
    }

  cmsys::ifstream ifs(versionFile.c_str());
  if ( !ifs )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Cannot open PackageMaker compiler version file" << std::endl);
    return 0;
    }

  // Check the PackageMaker version
  cmsys::RegularExpression rexKey("<key>CFBundleShortVersionString</key>");
  cmsys::RegularExpression rexVersion("<string>([0-9]+.[0-9.]+)</string>");
  std::string line;
  bool foundKey = false;
  while ( cmSystemTools::GetLineFromStream(ifs, line) )
    {
    if ( rexKey.find(line) )
      {
      foundKey = true;
      break;
      }
    }
  if ( !foundKey )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Cannot find CFBundleShortVersionString in the PackageMaker compiler "
      "version file" << std::endl);
    return 0;
    }
  if ( !cmSystemTools::GetLineFromStream(ifs, line) ||
    !rexVersion.find(line) )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Problem reading the PackageMaker compiler version file: "
      << versionFile.c_str() << std::endl);
    return 0;
    }
  this->PackageMakerVersion = atof(rexVersion.match(1).c_str());
  if ( this->PackageMakerVersion < 1.0 )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Require PackageMaker 1.0 or higher"
      << std::endl);
    return 0;
    }
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "PackageMaker version is: "
    << this->PackageMakerVersion << std::endl);

  // Determine the package compatibility version. If it wasn't
  // specified by the user, we define it based on which features the
  // user requested.
  const char *packageCompat = this->GetOption("CPACK_OSX_PACKAGE_VERSION");
  if (packageCompat && *packageCompat)
    {
    unsigned int majorVersion = 10;
    unsigned int minorVersion = 5;
    int res = sscanf(packageCompat, "%u.%u", &majorVersion, &minorVersion);
    if (res == 2)
      {
      this->PackageCompatibilityVersion =
        getVersion(majorVersion, minorVersion);
      }
    }
  else if (this->GetOption("CPACK_DOWNLOAD_SITE"))
    {
    this->SetOption("CPACK_OSX_PACKAGE_VERSION", "10.5");
    this->PackageCompatibilityVersion = getVersion(10, 5);
    }
  else if (this->GetOption("CPACK_COMPONENTS_ALL"))
    {
    this->SetOption("CPACK_OSX_PACKAGE_VERSION", "10.4");
    this->PackageCompatibilityVersion = getVersion(10, 4);
    }
  else
    {
    this->SetOption("CPACK_OSX_PACKAGE_VERSION", "10.3");
    this->PackageCompatibilityVersion = getVersion(10, 3);
    }

  std::vector<std::string> no_paths;
  pkgPath = cmSystemTools::FindProgram("hdiutil", no_paths, false);
  if ( pkgPath.empty() )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find hdiutil compiler"
      << std::endl);
    return 0;
    }
  this->SetOptionIfNotSet("CPACK_INSTALLER_PROGRAM_DISK_IMAGE",
                          pkgPath.c_str());

  return this->Superclass::InitializeInternal();
}

//----------------------------------------------------------------------
bool cmCPackPackageMakerGenerator::CopyCreateResourceFile(
                                            const std::string& name,
                                            const std::string& dirName)
{
  std::string uname = cmSystemTools::UpperCase(name);
  std::string cpackVar = "CPACK_RESOURCE_FILE_" + uname;
  const char* inFileName = this->GetOption(cpackVar.c_str());
  if ( !inFileName )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "CPack option: " << cpackVar.c_str()
                  << " not specified. It should point to "
                  << (!name.empty() ? name : "<empty>")
                  << ".rtf, " << name
                  << ".html, or " << name << ".txt file" << std::endl);
    return false;
    }
  if ( !cmSystemTools::FileExists(inFileName) )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find "
                  << (!name.empty() ? name : "<empty>")
                  << " resource file: " << inFileName << std::endl);
    return false;
    }
  std::string ext = cmSystemTools::GetFilenameLastExtension(inFileName);
  if ( ext != ".rtfd" && ext != ".rtf" && ext != ".html" && ext != ".txt" )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Bad file extension specified: "
      << ext << ". Currently only .rtfd, .rtf, .html, and .txt files allowed."
      << std::endl);
    return false;
    }

  std::string destFileName = dirName;
  destFileName += '/';
  destFileName += name + ext;

  // Set this so that distribution.dist gets the right name (without
  // the path).
  this->SetOption(("CPACK_RESOURCE_FILE_" + uname + "_NOPATH").c_str(),
                  (name + ext).c_str());

  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Configure file: "
                << (inFileName ? inFileName : "(NULL)")
                << " to " << destFileName.c_str() << std::endl);
  this->ConfigureFile(inFileName, destFileName.c_str());
  return true;
}

bool cmCPackPackageMakerGenerator::CopyResourcePlistFile(
                                                const std::string& name,
                                                const char* outName)
{
  if (!outName)
    {
    outName = name.c_str();
    }

  std::string inFName = "CPack.";
  inFName += name;
  inFName += ".in";
  std::string inFileName = this->FindTemplate(inFName.c_str());
  if ( inFileName.empty() )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find input file: "
      << inFName << std::endl);
    return false;
    }

  std::string destFileName = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  destFileName += "/";
  destFileName += outName;

  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Configure file: "
    << inFileName.c_str() << " to " << destFileName.c_str() << std::endl);
  this->ConfigureFile(inFileName.c_str(), destFileName.c_str());
  return true;
}

//----------------------------------------------------------------------
bool cmCPackPackageMakerGenerator::RunPackageMaker(const char *command,
                                                   const char *packageFile)
{
  std::string tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  tmpFile += "/PackageMakerOutput.log";

  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Execute: " << command << std::endl);
  std::string output;
  int retVal = 1;
  bool res = cmSystemTools::RunSingleCommand(
    command, &output, &output,
    &retVal, 0, this->GeneratorVerbose, 0);
  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Done running package maker"
    << std::endl);
  if ( !res || retVal )
    {
    cmGeneratedFileStream ofs(tmpFile.c_str());
    ofs << "# Run command: " << command << std::endl
      << "# Output:" << std::endl
      << output.c_str() << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Problem running PackageMaker command: " << command
      << std::endl << "Please check " << tmpFile.c_str() << " for errors"
      << std::endl);
    return false;
    }
  // sometimes the command finishes but the directory is not yet
  // created, so try 10 times to see if it shows up
  int tries = 10;
  while(tries > 0 &&
        !cmSystemTools::FileExists(packageFile))
    {
    cmSystemTools::Delay(500);
    tries--;
    }
  if(!cmSystemTools::FileExists(packageFile))
    {
    cmCPackLogger(
      cmCPackLog::LOG_ERROR,
      "Problem running PackageMaker command: " << command
      << std::endl << "Package not created: " << packageFile
      << std::endl);
    return false;
    }

  return true;
}

//----------------------------------------------------------------------
std::string
cmCPackPackageMakerGenerator::GetPackageName(const cmCPackComponent& component)
{
  if (component.ArchiveFile.empty())
    {
    std::string packagesDir = this->GetOption("CPACK_TEMPORARY_DIRECTORY");
    packagesDir += ".dummy";
    std::ostringstream out;
    out << cmSystemTools::GetFilenameWithoutLastExtension(packagesDir)
        << "-" << component.Name << ".pkg";
    return out.str();
    }
  else
    {
    return component.ArchiveFile + ".pkg";
    }
}

//----------------------------------------------------------------------
bool
cmCPackPackageMakerGenerator::
GenerateComponentPackage(const char *packageFile,
                         const char *packageDir,
                         const cmCPackComponent& component)
{
  cmCPackLogger(cmCPackLog::LOG_OUTPUT,
                "-   Building component package: " <<
                packageFile << std::endl);

  // The command that will be used to run PackageMaker
  std::ostringstream pkgCmd;

  if (this->PackageCompatibilityVersion < getVersion(10, 5) ||
      this->PackageMakerVersion < 3.0)
    {
    // Create Description.plist and Info.plist files for normal Mac OS
    // X packages, which work on Mac OS X 10.3 and newer.
    std::string descriptionFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
    descriptionFile += '/' + component.Name + "-Description.plist";
    cmsys::ofstream out(descriptionFile.c_str());
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl
        << "<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\""
        << "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">" << std::endl
        << "<plist version=\"1.4\">" << std::endl
        << "<dict>" << std::endl
        << "  <key>IFPkgDescriptionTitle</key>" << std::endl
        << "  <string>" << component.DisplayName << "</string>" << std::endl
        << "  <key>IFPkgDescriptionVersion</key>" << std::endl
        << "  <string>" << this->GetOption("CPACK_PACKAGE_VERSION")
        << "</string>" << std::endl
        << "  <key>IFPkgDescriptionDescription</key>" << std::endl
        << "  <string>" + this->EscapeForXML(component.Description)
        << "</string>" << std::endl
        << "</dict>" << std::endl
        << "</plist>" << std::endl;
    out.close();

    // Create the Info.plist file for this component
    std::string moduleVersionSuffix = ".";
    moduleVersionSuffix += component.Name;
    this->SetOption("CPACK_MODULE_VERSION_SUFFIX",
                    moduleVersionSuffix.c_str());
    std::string infoFileName = component.Name;
    infoFileName += "-Info.plist";
    if (!this->CopyResourcePlistFile("Info.plist", infoFileName.c_str()))
      {
      return false;
      }

    pkgCmd << "\"" << this->GetOption("CPACK_INSTALLER_PROGRAM")
           << "\" -build -p \"" << packageFile << "\""
           << " -f \"" << packageDir << "\""
           << " -i \"" << this->GetOption("CPACK_TOPLEVEL_DIRECTORY")
           << "/" << infoFileName << "\""
           << " -d \"" << descriptionFile << "\"";
    }
  else
    {
    // Create a "flat" package on Mac OS X 10.5 and newer. Flat
    // packages are stored in a single file, rather than a directory
    // like normal packages, and can be downloaded by the installer
    // on-the-fly in Mac OS X 10.5 or newer. Thus, we need to create
    // flat packages when the packages will be downloaded on the fly.
    std::string pkgId = "com.";
    pkgId += this->GetOption("CPACK_PACKAGE_VENDOR");
    pkgId += '.';
    pkgId += this->GetOption("CPACK_PACKAGE_NAME");
    pkgId += '.';
    pkgId += component.Name;

    pkgCmd << "\"" << this->GetOption("CPACK_INSTALLER_PROGRAM")
           << "\" --root \"" << packageDir << "\""
           << " --id " << pkgId
           << " --target " << this->GetOption("CPACK_OSX_PACKAGE_VERSION")
           << " --out \"" << packageFile << "\"";
    }

  // Run PackageMaker
  return RunPackageMaker(pkgCmd.str().c_str(), packageFile);
}

//----------------------------------------------------------------------
void
cmCPackPackageMakerGenerator::
WriteDistributionFile(const char* metapackageFile)
{
  std::string distributionTemplate
    = this->FindTemplate("CPack.distribution.dist.in");
  if ( distributionTemplate.empty() )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find input file: "
      << distributionTemplate << std::endl);
    return;
    }

  std::string distributionFile = metapackageFile;
  distributionFile += "/Contents/distribution.dist";

  // Create the choice outline, which provides a tree-based view of
  // the components in their groups.
  std::ostringstream choiceOut;
  choiceOut << "<choices-outline>" << std::endl;

  // Emit the outline for the groups
  std::map<std::string, cmCPackComponentGroup>::iterator groupIt;
  for (groupIt = this->ComponentGroups.begin();
       groupIt != this->ComponentGroups.end();
       ++groupIt)
    {
    if (groupIt->second.ParentGroup == 0)
      {
      CreateChoiceOutline(groupIt->second, choiceOut);
      }
    }

  // Emit the outline for the non-grouped components
  std::map<std::string, cmCPackComponent>::iterator compIt;
  for (compIt = this->Components.begin(); compIt != this->Components.end();
       ++compIt)
    {
    if (!compIt->second.Group)
      {
      choiceOut << "<line choice=\"" << compIt->first << "Choice\"></line>"
                << std::endl;
      }
    }
  if(!this->PostFlightComponent.Name.empty())
    {
      choiceOut << "<line choice=\"" << PostFlightComponent.Name
                << "Choice\"></line>" << std::endl;
    }
  choiceOut << "</choices-outline>" << std::endl;

  // Create the actual choices
  for (groupIt = this->ComponentGroups.begin();
       groupIt != this->ComponentGroups.end();
       ++groupIt)
    {
    CreateChoice(groupIt->second, choiceOut);
    }
  for (compIt = this->Components.begin(); compIt != this->Components.end();
       ++compIt)
    {
    CreateChoice(compIt->second, choiceOut);
    }

  if(!this->PostFlightComponent.Name.empty())
    {
    CreateChoice(PostFlightComponent, choiceOut);
    }

  this->SetOption("CPACK_PACKAGEMAKER_CHOICES", choiceOut.str().c_str());

  // Create the distribution.dist file in the metapackage to turn it
  // into a distribution package.
  this->ConfigureFile(distributionTemplate.c_str(),
                      distributionFile.c_str());
}

//----------------------------------------------------------------------
void
cmCPackPackageMakerGenerator::
CreateChoiceOutline(const cmCPackComponentGroup& group,
                    std::ostringstream& out)
{
  out << "<line choice=\"" << group.Name << "Choice\">" << std::endl;
  std::vector<cmCPackComponentGroup*>::const_iterator groupIt;
  for (groupIt = group.Subgroups.begin(); groupIt != group.Subgroups.end();
       ++groupIt)
    {
    CreateChoiceOutline(**groupIt, out);
    }

  std::vector<cmCPackComponent*>::const_iterator compIt;
  for (compIt = group.Components.begin(); compIt != group.Components.end();
       ++compIt)
    {
    out << "  <line choice=\"" << (*compIt)->Name << "Choice\"></line>"
        << std::endl;
    }
  out << "</line>" << std::endl;
}

//----------------------------------------------------------------------
void
cmCPackPackageMakerGenerator::CreateChoice(const cmCPackComponentGroup& group,
                                           std::ostringstream& out)
{
  out << "<choice id=\"" << group.Name << "Choice\" "
      << "title=\"" << group.DisplayName << "\" "
      << "start_selected=\"true\" "
      << "start_enabled=\"true\" "
      << "start_visible=\"true\" ";
  if (!group.Description.empty())
    {
    out << "description=\"" << EscapeForXML(group.Description)
        << "\"";
    }
  out << "></choice>" << std::endl;
}

//----------------------------------------------------------------------
void
cmCPackPackageMakerGenerator::CreateChoice(const cmCPackComponent& component,
                                           std::ostringstream& out)
{
  std::string packageId = "com.";
  packageId += this->GetOption("CPACK_PACKAGE_VENDOR");
  packageId += '.';
  packageId += this->GetOption("CPACK_PACKAGE_NAME");
  packageId += '.';
  packageId += component.Name;

  out << "<choice id=\"" << component.Name << "Choice\" "
      << "title=\"" << component.DisplayName << "\" "
      << "start_selected=\""
      << (component.IsDisabledByDefault &&
          !component.IsRequired? "false" : "true")
      << "\" "
      << "start_enabled=\""
      << (component.IsRequired? "false" : "true")
      << "\" "
      << "start_visible=\"" << (component.IsHidden? "false" : "true") << "\" ";
  if (!component.Description.empty())
    {
    out << "description=\"" << EscapeForXML(component.Description)
        << "\" ";
    }
  if (!component.Dependencies.empty() ||
      !component.ReverseDependencies.empty())
    {
    // The "selected" expression is evaluated each time any choice is
    // selected, for all choices *except* the one that the user
    // selected. A component is marked selected if it has been
    // selected (my.choice.selected in Javascript) and all of the
    // components it depends on have been selected (transitively) or
    // if any of the components that depend on it have been selected
    // (transitively). Assume that we have components A, B, C, D, and
    // E, where each component depends on the previous component (B
    // depends on A, C depends on B, D depends on C, and E depends on
    // D). The expression we build for the component C will be
    //   my.choice.selected && B && A || D || E
    // This way, selecting C will automatically select everything it depends
    // on (B and A), while selecting something that depends on C--either D
    // or E--will automatically cause C to get selected.
    out << "selected=\"my.choice.selected";
    std::set<const cmCPackComponent *> visited;
    AddDependencyAttributes(component, visited, out);
    visited.clear();
    AddReverseDependencyAttributes(component, visited, out);
    out << "\"";
    }
  out << ">" << std::endl;
  out << "  <pkg-ref id=\"" << packageId << "\"></pkg-ref>" << std::endl;
  out << "</choice>" << std::endl;

  // Create a description of the package associated with this
  // component.
  std::string relativePackageLocation = "Contents/Packages/";
  relativePackageLocation += this->GetPackageName(component);

  // Determine the installed size of the package.
  std::string dirName = this->GetOption("CPACK_TEMPORARY_DIRECTORY");
  dirName += '/';
  dirName += component.Name;
  dirName += this->GetOption("CPACK_PACKAGING_INSTALL_PREFIX");
  unsigned long installedSize
    = component.GetInstalledSizeInKbytes(dirName.c_str());

  out << "<pkg-ref id=\"" << packageId << "\" "
      << "version=\"" << this->GetOption("CPACK_PACKAGE_VERSION") << "\" "
      << "installKBytes=\"" << installedSize << "\" "
      << "auth=\"Admin\" onConclusion=\"None\">";
  if (component.IsDownloaded)
    {
    out << this->GetOption("CPACK_DOWNLOAD_SITE")
        << this->GetPackageName(component);
    }
  else
    {
    out << "file:./" << relativePackageLocation;
    }
  out << "</pkg-ref>" << std::endl;
}

//----------------------------------------------------------------------
void
cmCPackPackageMakerGenerator::
AddDependencyAttributes(const cmCPackComponent& component,
                        std::set<const cmCPackComponent *>& visited,
                        std::ostringstream& out)
{
  if (visited.find(&component) != visited.end())
    {
    return;
    }
  visited.insert(&component);

  std::vector<cmCPackComponent *>::const_iterator dependIt;
  for (dependIt = component.Dependencies.begin();
       dependIt != component.Dependencies.end();
       ++dependIt)
    {
    out << " &amp;&amp; choices['" <<
      (*dependIt)->Name << "Choice'].selected";
    AddDependencyAttributes(**dependIt, visited, out);
    }
}

//----------------------------------------------------------------------
void
cmCPackPackageMakerGenerator::
AddReverseDependencyAttributes(const cmCPackComponent& component,
                               std::set<const cmCPackComponent *>& visited,
                               std::ostringstream& out)
{
  if (visited.find(&component) != visited.end())
    {
    return;
    }
  visited.insert(&component);

  std::vector<cmCPackComponent *>::const_iterator dependIt;
  for (dependIt = component.ReverseDependencies.begin();
       dependIt != component.ReverseDependencies.end();
       ++dependIt)
    {
    out << " || choices['" << (*dependIt)->Name << "Choice'].selected";
    AddReverseDependencyAttributes(**dependIt, visited, out);
    }
}

//----------------------------------------------------------------------
std::string cmCPackPackageMakerGenerator::EscapeForXML(std::string str)
{
  cmSystemTools::ReplaceString(str, "&", "&amp;");
  cmSystemTools::ReplaceString(str, "<", "&lt;");
  cmSystemTools::ReplaceString(str, ">", "&gt;");
  cmSystemTools::ReplaceString(str, "\"", "&quot;");
  return str;
}
