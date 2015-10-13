/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackPackageMakerGenerator_h
#define cmCPackPackageMakerGenerator_h


#include "cmCPackGenerator.h"

class cmCPackComponent;

/** \class cmCPackPackageMakerGenerator
 * \brief A generator for PackageMaker files
 *
 * http://developer.apple.com/documentation/Darwin
 * /Reference/ManPages/man1/packagemaker.1.html
 */
class cmCPackPackageMakerGenerator : public cmCPackGenerator
{
public:
  cmCPackTypeMacro(cmCPackPackageMakerGenerator, cmCPackGenerator);

  /**
   * Construct generator
   */
  cmCPackPackageMakerGenerator();
  virtual ~cmCPackPackageMakerGenerator();

  virtual bool SupportsComponentInstallation() const;

protected:
  int CopyInstallScript(const std::string& resdir,
                        const std::string& script,
                        const std::string& name);
  virtual int InitializeInternal();
  int PackageFiles();
  virtual const char* GetOutputExtension() { return ".dmg"; }
  virtual const char* GetOutputPostfix() { return "darwin"; }

  // Copies or creates the resource file with the given name to the
  // package or package staging directory dirName. The variable
  // CPACK_RESOURCE_FILE_${NAME} (where ${NAME} is the uppercased
  // version of name) specifies the input file to use for this file,
  // which will be configured via ConfigureFile.
  bool CopyCreateResourceFile(const std::string& name,
                              const std::string& dirName);
  bool CopyResourcePlistFile(const std::string& name, const char* outName = 0);

  // Run PackageMaker with the given command line, which will (if
  // successful) produce the given package file. Returns true if
  // PackageMaker succeeds, false otherwise.
  bool RunPackageMaker(const char *command, const char *packageFile);

  // Retrieve the name of package file that will be generated for this
  // component. The name is just the file name with extension, and
  // does not include the subdirectory.
  std::string GetPackageName(const cmCPackComponent& component);

  // Generate a package in the file packageFile for the given
  // component.  All of the files within this component are stored in
  // the directory packageDir. Returns true if successful, false
  // otherwise.
  bool GenerateComponentPackage(const char *packageFile,
                                const char *packageDir,
                                const cmCPackComponent& component);

  // Writes a distribution.dist file, which turns a metapackage into a
  // full-fledged distribution. This file is used to describe
  // inter-component dependencies. metapackageFile is the name of the
  // metapackage for the distribution. Only valid for a
  // component-based install.
  void WriteDistributionFile(const char* metapackageFile);

  // Subroutine of WriteDistributionFile that writes out the
  // dependency attributes for inter-component dependencies.
  void AddDependencyAttributes(const cmCPackComponent& component,
                               std::set<const cmCPackComponent *>& visited,
                               std::ostringstream& out);

  // Subroutine of WriteDistributionFile that writes out the
  // reverse dependency attributes for inter-component dependencies.
  void
  AddReverseDependencyAttributes(const cmCPackComponent& component,
                                 std::set<const cmCPackComponent *>& visited,
                                 std::ostringstream& out);

  // Generates XML that encodes the hierarchy of component groups and
  // their components in a form that can be used by distribution
  // metapackages.
  void CreateChoiceOutline(const cmCPackComponentGroup& group,
                           std::ostringstream& out);

  /// Create the "choice" XML element to describe a component group
  /// for the installer GUI.
  void CreateChoice(const cmCPackComponentGroup& group,
                    std::ostringstream& out);

  /// Create the "choice" XML element to describe a component for the
  /// installer GUI.
  void CreateChoice(const cmCPackComponent& component,
                    std::ostringstream& out);

  // Escape the given string to make it usable as an XML attribute
  // value.
  std::string EscapeForXML(std::string str);

  // The PostFlight component when creating a metapackage
  cmCPackComponent PostFlightComponent;

  double PackageMakerVersion;
  unsigned int PackageCompatibilityVersion;
};

#endif
