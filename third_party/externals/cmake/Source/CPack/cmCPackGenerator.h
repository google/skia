/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackGenerator_h
#define cmCPackGenerator_h

#include "cmObject.h"
#include "cmSystemTools.h"
#include <map>
#include <vector>

#include "cmCPackComponentGroup.h" // cmCPackComponent and friends
  // Forward declarations are insufficient since we use them in
  // std::map data members below...

#define cmCPackTypeMacro(klass, superclass) \
  cmTypeMacro(klass, superclass); \
  static cmCPackGenerator* CreateGenerator() { return new klass; } \
  class cmCPackTypeMacro_UseTrailingSemicolon

#define cmCPackLogger(logType, msg) \
  do { \
    std::ostringstream cmCPackLog_msg; \
    cmCPackLog_msg << msg; \
    this->Logger->Log(logType, __FILE__, __LINE__,\
                      cmCPackLog_msg.str().c_str());\
  } while ( 0 )

#ifdef cerr
#  undef cerr
#endif
#define cerr no_cerr_use_cmCPack_Log

#ifdef cout
#  undef cout
#endif
#define cout no_cout_use_cmCPack_Log

class cmMakefile;
class cmCPackLog;
class cmInstalledFile;

/** \class cmCPackGenerator
 * \brief A superclass of all CPack Generators
 *
 */
class cmCPackGenerator : public cmObject
{
public:
  cmTypeMacro(cmCPackGenerator, cmObject);
  /**
   * If verbose then more information is printed out
   */
  void SetVerbose(bool val)
    { this->GeneratorVerbose = val ?
      cmSystemTools::OUTPUT_MERGE : cmSystemTools::OUTPUT_NONE; }

  /**
   * Returns true if the generator may work on this system.
   * Rational:
   * Some CPack generator may run on some host and may not on others
   * (with the same system) because some tools are missing. If the tool
   * is missing then CPack won't activate (in the CPackGeneratorFactory)
   * this particular generator.
   */
  static bool CanGenerate() { return true; }

  /**
   * Do the actual whole package processing.
   * Subclass may redefine it but its usually enough
   * to redefine @ref PackageFiles, because in fact
   * this method do call:
   *     - PrepareName
   *     - clean-up temp dirs
   *     - InstallProject (with the appropriate method)
   *     - prepare list of files and/or components to be package
   *     - PackageFiles
   *     - Copy produced packages at the expected place
   * @return 0 if error.
   */
  virtual int DoPackage();

  /**
   * Initialize generator
   */
  int Initialize(const std::string& name, cmMakefile* mf);

  /**
   * Construct generator
   */
  cmCPackGenerator();
  virtual ~cmCPackGenerator();

  //! Set and get the options
  void SetOption(const std::string& op, const char* value);
  void SetOptionIfNotSet(const std::string& op, const char* value);
  const char* GetOption(const std::string& op) const;
  std::vector<std::string> GetOptions() const;
  bool IsSet(const std::string& name) const;
  bool IsOn(const std::string& name) const;

  //! Set the logger
  void SetLogger(cmCPackLog* log) { this->Logger = log; }

  //! Display verbose information via logger
  void DisplayVerboseOutput(const char* msg, float progress);

  bool ReadListFile(const char* moduleName);

protected:
  /**
   * Prepare common used names by inspecting
   * several CPACK_xxx var values.
   */
  int PrepareNames();

  /**
   * Install the project using appropriate method.
   */
  int InstallProject();

  int CleanTemporaryDirectory();

  cmInstalledFile const* GetInstalledFile(std::string const& name) const;

  virtual const char* GetOutputExtension() { return ".cpack"; }
  virtual const char* GetOutputPostfix() { return 0; }

  /**
   * Prepare requested grouping kind from CPACK_xxx vars
   * CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE
   * CPACK_COMPONENTS_IGNORE_GROUPS
   * or
   * CPACK_COMPONENTS_ONE_PACKAGE_PER_GROUP
   * @return 1 on success 0 on failure.
   */
  virtual int PrepareGroupingKind();

  /**
   * Some CPack generators may prefer to have
   * CPack install all components belonging to the same
   * [component] group to be install in the same directory.
   * The default behavior is to install each component in
   * a separate directory.
   * @param[in] componentName the name of the component to be installed
   * @return the name suffix the generator wants for the specified component
   *         default is "componentName"
   */
  virtual std::string GetComponentInstallDirNameSuffix(
      const std::string& componentName);

  /**
   * CPack specific generator may mangle CPACK_PACKAGE_FILE_NAME
   * with CPACK_COMPONENT_xxxx_<NAME>_DISPLAY_NAME if
   * CPACK_<GEN>_USE_DISPLAY_NAME_IN_FILENAME is ON.
   * @param[in] initialPackageFileName the initial package name to be mangled
   * @param[in] groupOrComponentName the name of the group/component
   * @param[in] isGroupName true if previous name refers to a group,
   *            false otherwise
   */
  virtual std::string GetComponentPackageFileName(
      const std::string& initialPackageFileName,
      const std::string& groupOrComponentName,
      bool isGroupName);

  /**
   * Package the list of files and/or components which
   * has been prepared by the beginning of DoPackage.
   * @pre the @ref toplevel has been filled-in
   * @pre the list of file @ref files has been populated
   * @pre packageFileNames contains at least 1 entry
   * @post packageFileNames may have been updated and contains
   *       the list of packages generated by the specific generator.
   */
  virtual int PackageFiles();
  virtual const char* GetInstallPath();
  virtual const char* GetPackagingInstallPrefix();

  virtual std::string FindTemplate(const char* name);
  virtual bool ConfigureFile(const char* inName, const char* outName,
    bool copyOnly = false);
  virtual bool ConfigureString(const std::string& input, std::string& output);
  virtual int InitializeInternal();


  //! Run install commands if specified
  virtual int InstallProjectViaInstallCommands(
    bool setDestDir, const std::string& tempInstallDirectory);
  virtual int InstallProjectViaInstallScript(
    bool setDestDir, const std::string& tempInstallDirectory);
  virtual int InstallProjectViaInstalledDirectories(
    bool setDestDir, const std::string& tempInstallDirectory);
  virtual int InstallProjectViaInstallCMakeProjects(
    bool setDestDir, const std::string& tempInstallDirectory);

  /**
   * The various level of support of
   * CPACK_SET_DESTDIR used by the generator.
   */
  enum CPackSetDestdirSupport {
    /* the generator works with or without it */
    SETDESTDIR_SUPPORTED,
    /* the generator works best if automatically handled */
    SETDESTDIR_INTERNALLY_SUPPORTED,
    /* no official support, use at your own risk */
    SETDESTDIR_SHOULD_NOT_BE_USED,
    /* officially NOT supported */
    SETDESTDIR_UNSUPPORTED
  };

  /**
   * Does the CPack generator support CPACK_SET_DESTDIR?
   * The default legacy value is 'SETDESTDIR_SUPPORTED' generator
   * have to override it in order change this.
   * @return CPackSetDestdirSupport
   */
  virtual enum CPackSetDestdirSupport SupportsSetDestdir() const;

  /**
   * Does the CPack generator support absolute path
   * in INSTALL DESTINATION?
   * The default legacy value is 'true' generator
   * have to override it in order change this.
   * @return true if supported false otherwise
   */
  virtual bool SupportsAbsoluteDestination() const;

  /**
   * Does the CPack generator support component installation?.
   * Some Generators requires the user to set
   * CPACK_<GENNAME>_COMPONENT_INSTALL in order to make this
   * method return true.
   * @return true if supported, false otherwise
   */
  virtual bool SupportsComponentInstallation() const;
  /**
   * Does the currently running generator want a component installation.
   * The generator may support component installation but he may
   * be requiring monolithic install using CPACK_MONOLITHIC_INSTALL.
   * @return true if component installation is supported and wanted.
   */
  virtual bool WantsComponentInstallation() const;
  virtual cmCPackInstallationType* GetInstallationType(
                                                const std::string& projectName,
                                                const std::string& name);
  virtual cmCPackComponent* GetComponent(const std::string& projectName,
                                         const std::string& name);
  virtual cmCPackComponentGroup* GetComponentGroup(
                                                const std::string& projectName,
                                                const std::string& name);

  cmSystemTools::OutputOption GeneratorVerbose;
  std::string Name;

  std::string InstallPath;

  /**
   * The list of package file names.
   * At beginning of DoPackage the (generic) generator will populate
   * the list of desired package file names then it will
   * call the redefined method PackageFiles which is may
   * either use this set of names (usually on entry there should be
   * only a single name) or update the vector with the list
   * of created package file names.
   */
  std::vector<std::string> packageFileNames;

  /**
   * The directory where all the files to be packaged reside.
   * If the installer support components there will be one
   * sub-directory for each component. In those directories
   * one will find the file belonging to the specified component.
   */
  std::string toplevel;

  /**
   * The complete list of files to be packaged.
   * This list will be populated by DoPackage before
   * PackageFiles is called.
   */
  std::vector<std::string> files;

  std::map<std::string, cmCPackInstallationType> InstallationTypes;
  /**
   * The set of components.
   * If component installation is supported then this map
   * contains the component specified in CPACK_COMPONENTS_ALL
   */
  std::map<std::string, cmCPackComponent> Components;
  std::map<std::string, cmCPackComponentGroup> ComponentGroups;

  /**
   * If components are enabled, this enum represents the different
   * ways of mapping components to package files.
   */
  enum ComponentPackageMethod
  {
    /* one package for all components */
    ONE_PACKAGE,
    /* one package for each component */
    ONE_PACKAGE_PER_COMPONENT,
    /* one package for each group,
     * with left over components in their own package */
    ONE_PACKAGE_PER_GROUP,
    UNKNOWN_COMPONENT_PACKAGE_METHOD
  };

  /**
   * The component package method
   * The default is ONE_PACKAGE_PER_GROUP,
   * and generators may override the default
   * before PrepareGroupingKind() is called.
   */
  ComponentPackageMethod componentPackageMethod;

  cmCPackLog* Logger;
private:
  cmMakefile* MakefileMap;
};

#endif
