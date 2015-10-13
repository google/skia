/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCPackRPMGenerator.h"
#include "cmCPackLog.h"
#include "cmSystemTools.h"

//----------------------------------------------------------------------
cmCPackRPMGenerator::cmCPackRPMGenerator()
{
}

//----------------------------------------------------------------------
cmCPackRPMGenerator::~cmCPackRPMGenerator()
{
}

//----------------------------------------------------------------------
int cmCPackRPMGenerator::InitializeInternal()
{
  this->SetOptionIfNotSet("CPACK_PACKAGING_INSTALL_PREFIX", "/usr");
  if (cmSystemTools::IsOff(this->GetOption("CPACK_SET_DESTDIR")))
    {
    this->SetOption("CPACK_SET_DESTDIR", "I_ON");
    }
  /* Replace space in CPACK_PACKAGE_NAME in order to avoid
   * rpmbuild scream on unwanted space in filename issue
   * Moreover RPM file do not usually embed space in filename
   */
  if (this->GetOption("CPACK_PACKAGE_NAME")) {
    std::string packageName=this->GetOption("CPACK_PACKAGE_NAME");
    cmSystemTools::ReplaceString(packageName," ","-");
    this->SetOption("CPACK_PACKAGE_NAME",packageName.c_str());
  }
  /* same for CPACK_PACKAGE_FILE_NAME */
  if (this->GetOption("CPACK_PACKAGE_FILE_NAME")) {
    std::string packageName=this->GetOption("CPACK_PACKAGE_FILE_NAME");
    cmSystemTools::ReplaceString(packageName," ","-");
    this->SetOption("CPACK_PACKAGE_FILE_NAME",packageName.c_str());
  }
  return this->Superclass::InitializeInternal();
}

//----------------------------------------------------------------------
int cmCPackRPMGenerator::PackageOnePack(std::string initialToplevel,
                                        std::string packageName)
{
  int retval = 1;
  // Begin the archive for this pack
  std::string localToplevel(initialToplevel);
  std::string packageFileName(
      cmSystemTools::GetParentDirectory(toplevel)
                             );
  std::string outputFileName(
   GetComponentPackageFileName(this->GetOption("CPACK_PACKAGE_FILE_NAME"),
                               packageName,
                               true)
                               + this->GetOutputExtension()
                            );

  localToplevel += "/"+ packageName;
  /* replace the TEMP DIRECTORY with the component one */
  this->SetOption("CPACK_TEMPORARY_DIRECTORY",localToplevel.c_str());
  packageFileName += "/"+ outputFileName;
  /* replace proposed CPACK_OUTPUT_FILE_NAME */
  this->SetOption("CPACK_OUTPUT_FILE_NAME",outputFileName.c_str());
  /* replace the TEMPORARY package file name */
  this->SetOption("CPACK_TEMPORARY_PACKAGE_FILE_NAME",
                  packageFileName.c_str());
  // Tell CPackRPM.cmake the name of the component NAME.
  this->SetOption("CPACK_RPM_PACKAGE_COMPONENT",packageName.c_str());
  // Tell CPackRPM.cmake the path where the component is.
  std::string component_path = "/";
  component_path += packageName;
  this->SetOption("CPACK_RPM_PACKAGE_COMPONENT_PART_PATH",
                  component_path.c_str());
  if (!this->ReadListFile("CPackRPM.cmake"))
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Error while execution CPackRPM.cmake" << std::endl);
    retval = 0;
    }
  // add the generated package to package file names list
  packageFileNames.push_back(packageFileName);
  return retval;
}

//----------------------------------------------------------------------
int cmCPackRPMGenerator::PackageComponents(bool ignoreGroup)
{
  int retval = 1;
  /* Reset package file name list it will be populated during the
   * component packaging run*/
  packageFileNames.clear();
  std::string initialTopLevel(this->GetOption("CPACK_TEMPORARY_DIRECTORY"));

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
      retval &= PackageOnePack(initialTopLevel,compGIt->first);
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
        retval &= PackageOnePack(initialTopLevel,compIt->first);
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
      retval &= PackageOnePack(initialTopLevel,compIt->first);
      }
    }
  return retval;
}

//----------------------------------------------------------------------
int cmCPackRPMGenerator::PackageComponentsAllInOne()
{
  int retval = 1;
  std::string compInstDirName;
  /* Reset package file name list it will be populated during the
   * component packaging run*/
  packageFileNames.clear();
  std::string initialTopLevel(this->GetOption("CPACK_TEMPORARY_DIRECTORY"));

  compInstDirName = "ALL_COMPONENTS_IN_ONE";

  cmCPackLogger(cmCPackLog::LOG_VERBOSE,
                "Packaging all groups in one package..."
                "(CPACK_COMPONENTS_ALL_[GROUPS_]IN_ONE_PACKAGE is set)"
      << std::endl);

  // The ALL GROUPS in ONE package case
  std::string localToplevel(initialTopLevel);
  std::string packageFileName(
      cmSystemTools::GetParentDirectory(toplevel)
                             );
  std::string outputFileName(
            std::string(this->GetOption("CPACK_PACKAGE_FILE_NAME"))
            + this->GetOutputExtension()
                            );
  // all GROUP in one vs all COMPONENT in one
  localToplevel += "/"+compInstDirName;

  /* replace the TEMP DIRECTORY with the component one */
  this->SetOption("CPACK_TEMPORARY_DIRECTORY",localToplevel.c_str());
  packageFileName += "/"+ outputFileName;
  /* replace proposed CPACK_OUTPUT_FILE_NAME */
  this->SetOption("CPACK_OUTPUT_FILE_NAME",outputFileName.c_str());
  /* replace the TEMPORARY package file name */
  this->SetOption("CPACK_TEMPORARY_PACKAGE_FILE_NAME",
      packageFileName.c_str());
  // Tell CPackRPM.cmake the path where the component is.
  std::string component_path = "/";
  component_path += compInstDirName;
  this->SetOption("CPACK_RPM_PACKAGE_COMPONENT_PART_PATH",
                  component_path.c_str());
  if (!this->ReadListFile("CPackRPM.cmake"))
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Error while execution CPackRPM.cmake" << std::endl);
    retval = 0;
    }
  // add the generated package to package file names list
  packageFileNames.push_back(packageFileName);

  return retval;
}

//----------------------------------------------------------------------
int cmCPackRPMGenerator::PackageFiles()
{
  int retval = 1;

  cmCPackLogger(cmCPackLog::LOG_DEBUG, "Toplevel: "
                  << toplevel << std::endl);

  /* Are we in the component packaging case */
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
  else
    {
    if (!this->ReadListFile("CPackRPM.cmake"))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Error while execution CPackRPM.cmake" << std::endl);
      retval = 0;
      }
    }

  if (!this->IsSet("RPMBUILD_EXECUTABLE"))
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find rpmbuild" << std::endl);
    retval = 0;
    }
  return retval;
}

bool cmCPackRPMGenerator::SupportsComponentInstallation() const
  {
  if (IsOn("CPACK_RPM_COMPONENT_INSTALL"))
    {
      return true;
    }
  else
    {
      return false;
    }
  }

std::string cmCPackRPMGenerator::GetComponentInstallDirNameSuffix(
    const std::string& componentName)
  {
  if (componentPackageMethod == ONE_PACKAGE_PER_COMPONENT) {
    return componentName;
  }

  if (componentPackageMethod == ONE_PACKAGE) {
    return std::string("ALL_COMPONENTS_IN_ONE");
  }
  // We have to find the name of the COMPONENT GROUP
  // the current COMPONENT belongs to.
  std::string groupVar = "CPACK_COMPONENT_" +
        cmSystemTools::UpperCase(componentName) + "_GROUP";
    if (NULL != GetOption(groupVar))
      {
      return std::string(GetOption(groupVar));
      }
    else
      {
      return componentName;
      }
  }
