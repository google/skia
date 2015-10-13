/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCPackArchiveGenerator.h"

#include "cmake.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmCPackLog.h"
#include <errno.h>

#include <cmsys/SystemTools.hxx>
#include <cmsys/Directory.hxx>
#include <cm_libarchive.h>

//----------------------------------------------------------------------
cmCPackArchiveGenerator::cmCPackArchiveGenerator(cmArchiveWrite::Compress t,
  std::string const& format)
{
  this->Compress = t;
  this->ArchiveFormat = format;
}

//----------------------------------------------------------------------
cmCPackArchiveGenerator::~cmCPackArchiveGenerator()
{
}

//----------------------------------------------------------------------
int cmCPackArchiveGenerator::InitializeInternal()
{
  this->SetOptionIfNotSet("CPACK_INCLUDE_TOPLEVEL_DIRECTORY", "1");
  return this->Superclass::InitializeInternal();
}
//----------------------------------------------------------------------
int cmCPackArchiveGenerator::addOneComponentToArchive(cmArchiveWrite& archive,
                             cmCPackComponent* component)
{
  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "   - packaging component: "
      << component->Name
      << std::endl);
  // Add the files of this component to the archive
  std::string localToplevel(this->GetOption("CPACK_TEMPORARY_DIRECTORY"));
  localToplevel += "/"+ component->Name;
  std::string dir = cmSystemTools::GetCurrentWorkingDirectory();
  // Change to local toplevel
  cmSystemTools::ChangeDirectory(localToplevel);
  std::string filePrefix;
  if (this->IsOn("CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY"))
    {
    filePrefix = this->GetOption("CPACK_PACKAGE_FILE_NAME");
    filePrefix += "/";
    }
  const char* installPrefix =
    this->GetOption("CPACK_PACKAGING_INSTALL_PREFIX");
  if(installPrefix && installPrefix[0] == '/' && installPrefix[1] != 0)
    {
    // add to file prefix and remove the leading '/'
    filePrefix += installPrefix+1;
    filePrefix += "/";
    }
  std::vector<std::string>::const_iterator fileIt;
  for (fileIt = component->Files.begin(); fileIt != component->Files.end();
       ++fileIt )
    {
    std::string rp = filePrefix + *fileIt;
    cmCPackLogger(cmCPackLog::LOG_DEBUG,"Adding file: "
                  << rp << std::endl);
    archive.Add(rp);
    if (!archive)
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR, "ERROR while packaging files: "
            << archive.GetError()
            << std::endl);
      return 0;
      }
    }
  // Go back to previous dir
  cmSystemTools::ChangeDirectory(dir);
  return 1;
}

/*
 * The macro will open/create a file 'filename'
 * an declare and open the associated
 * cmArchiveWrite 'archive' object.
 */
#define DECLARE_AND_OPEN_ARCHIVE(filename,archive) \
cmGeneratedFileStream gf; \
gf.Open(filename.c_str(), false, true); \
if (!GenerateHeader(&gf)) \
  { \
   cmCPackLogger(cmCPackLog::LOG_ERROR, \
    "Problem to generate Header for archive < "     \
            << filename \
            << ">." << std::endl); \
    return 0; \
  } \
cmArchiveWrite archive(gf,this->Compress, this->ArchiveFormat); \
if (!archive) \
  { \
  cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem to create archive < " \
     << filename \
     << ">. ERROR =" \
     << archive.GetError() \
     << std::endl); \
  return 0; \
  }

//----------------------------------------------------------------------
int cmCPackArchiveGenerator::PackageComponents(bool ignoreGroup)
{
  packageFileNames.clear();
  // The default behavior is to have one package by component group
  // unless CPACK_COMPONENTS_IGNORE_GROUP is specified.
  if (!ignoreGroup)
    {
    std::map<std::string, cmCPackComponentGroup>::iterator compGIt;
    for (compGIt=this->ComponentGroups.begin();
        compGIt!=this->ComponentGroups.end(); ++compGIt)
      {
      cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Packaging component group: "
          << compGIt->first
          << std::endl);
      // Begin the archive for this group
      std::string packageFileName= std::string(toplevel);
      packageFileName += "/"+
       GetComponentPackageFileName(this->GetOption("CPACK_PACKAGE_FILE_NAME"),
                                   compGIt->first,
                                   true)
         + this->GetOutputExtension();
      // open a block in order to automatically close archive
      // at the end of the block
      {
        DECLARE_AND_OPEN_ARCHIVE(packageFileName,archive);
        // now iterate over the component of this group
        std::vector<cmCPackComponent*>::iterator compIt;
        for (compIt=(compGIt->second).Components.begin();
            compIt!=(compGIt->second).Components.end();
            ++compIt)
          {
          // Add the files of this component to the archive
          addOneComponentToArchive(archive,*compIt);
          }
      }
      // add the generated package to package file names list
      packageFileNames.push_back(packageFileName);
      }
    // Handle Orphan components (components not belonging to any groups)
    std::map<std::string, cmCPackComponent>::iterator compIt;
    for (compIt=this->Components.begin();
         compIt!=this->Components.end(); ++compIt )
      {
      // Does the component belong to a group?
      if (compIt->second.Group==NULL)
        {
        cmCPackLogger(cmCPackLog::LOG_VERBOSE,
            "Component <"
              << compIt->second.Name
              << "> does not belong to any group, package it separately."
              << std::endl);
        std::string localToplevel(
          this->GetOption("CPACK_TEMPORARY_DIRECTORY")
                                 );
        std::string packageFileName = std::string(toplevel);

        localToplevel += "/"+ compIt->first;
        packageFileName += "/"+
        GetComponentPackageFileName(this->GetOption("CPACK_PACKAGE_FILE_NAME"),
                                    compIt->first,
                                    false)
                              + this->GetOutputExtension();
        {
          DECLARE_AND_OPEN_ARCHIVE(packageFileName,archive);
          // Add the files of this component to the archive
          addOneComponentToArchive(archive,&(compIt->second));
        }
        // add the generated package to package file names list
        packageFileNames.push_back(packageFileName);
        }
      }
    }
  // CPACK_COMPONENTS_IGNORE_GROUPS is set
  // We build 1 package per component
  else
    {
    std::map<std::string, cmCPackComponent>::iterator compIt;
    for (compIt=this->Components.begin();
         compIt!=this->Components.end(); ++compIt )
      {
      std::string localToplevel(this->GetOption("CPACK_TEMPORARY_DIRECTORY"));
      std::string packageFileName = std::string(toplevel);

      localToplevel += "/"+ compIt->first;
      packageFileName += "/"+
       GetComponentPackageFileName(this->GetOption("CPACK_PACKAGE_FILE_NAME"),
                                   compIt->first,
                                   false)
        + this->GetOutputExtension();
      {
        DECLARE_AND_OPEN_ARCHIVE(packageFileName,archive);
        // Add the files of this component to the archive
        addOneComponentToArchive(archive,&(compIt->second));
      }
      // add the generated package to package file names list
      packageFileNames.push_back(packageFileName);
      }
    }
  return 1;
}

//----------------------------------------------------------------------
int cmCPackArchiveGenerator::PackageComponentsAllInOne()
{
  // reset the package file names
  packageFileNames.clear();
  packageFileNames.push_back(std::string(toplevel));
  packageFileNames[0] += "/"
    +std::string(this->GetOption("CPACK_PACKAGE_FILE_NAME"))
    + this->GetOutputExtension();
  cmCPackLogger(cmCPackLog::LOG_VERBOSE,
                "Packaging all groups in one package..."
                "(CPACK_COMPONENTS_ALL_GROUPS_IN_ONE_PACKAGE is set)"
      << std::endl);
  DECLARE_AND_OPEN_ARCHIVE(packageFileNames[0],archive);

  // The ALL COMPONENTS in ONE package case
  std::map<std::string, cmCPackComponent>::iterator compIt;
  for (compIt=this->Components.begin();compIt!=this->Components.end();
      ++compIt )
    {
    // Add the files of this component to the archive
    addOneComponentToArchive(archive,&(compIt->second));
    }

  // archive goes out of scope so it will finalized and closed.
  return 1;
}

//----------------------------------------------------------------------
int cmCPackArchiveGenerator::PackageFiles()
{
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "Toplevel: "
                << toplevel << std::endl);

  if (WantsComponentInstallation()) {
    // CASE 1 : COMPONENT ALL-IN-ONE package
    // If ALL COMPONENTS in ONE package has been requested
    // then the package file is unique and should be open here.
    if (componentPackageMethod == ONE_PACKAGE)
      {
      return PackageComponentsAllInOne();
      }
    // CASE 2 : COMPONENT CLASSICAL package(s) (i.e. not all-in-one)
    // There will be 1 package for each component group
    // however one may require to ignore component group and
    // in this case you'll get 1 package for each component.
    else
      {
      return PackageComponents(componentPackageMethod ==
                               ONE_PACKAGE_PER_COMPONENT);
      }
  }

  // CASE 3 : NON COMPONENT package.
  DECLARE_AND_OPEN_ARCHIVE(packageFileNames[0],archive);
  std::vector<std::string>::const_iterator fileIt;
  std::string dir = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(toplevel);
  for ( fileIt = files.begin(); fileIt != files.end(); ++ fileIt )
    {
    // Get the relative path to the file
    std::string rp = cmSystemTools::RelativePath(toplevel.c_str(),
                                                 fileIt->c_str());
    archive.Add(rp);
    if(!archive)
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem while adding file< "
          << *fileIt
          << "> to archive <"
          << packageFileNames[0] << "> .ERROR ="
          << archive.GetError()
          << std::endl);
      return 0;
      }
    }
  cmSystemTools::ChangeDirectory(dir);
  // The destructor of cmArchiveWrite will close and finish the write
  return 1;
}

//----------------------------------------------------------------------
int cmCPackArchiveGenerator::GenerateHeader(std::ostream*)
{
  return 1;
}

bool cmCPackArchiveGenerator::SupportsComponentInstallation() const {
  // The Component installation support should only
  // be activated if explicitly requested by the user
  // (for backward compatibility reason)
  if (IsOn("CPACK_ARCHIVE_COMPONENT_INSTALL"))
    {
    return true;
    }
  else
    {
    return false;
    }
}
