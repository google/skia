/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmSourceFile_h
#define cmSourceFile_h

#include "cmSourceFileLocation.h"
#include "cmCustomCommand.h"
#include "cmPropertyMap.h"

class cmake;

/** \class cmSourceFile
 * \brief Represent a class loaded from a makefile.
 *
 * cmSourceFile is represents a class loaded from
 * a makefile.
 */
class cmSourceFile
{
public:
  /**
   * Construct with the makefile storing the source and the initial
   * name referencing it.
   */
  cmSourceFile(cmMakefile* mf, const std::string& name);

  ~cmSourceFile();

  /**
   * Get the list of the custom commands for this source file
   */
  cmCustomCommand* GetCustomCommand();
  cmCustomCommand const* GetCustomCommand() const;
  void SetCustomCommand(cmCustomCommand *cc);

  ///! Set/Get a property of this source file
  void SetProperty(const std::string& prop, const char *value);
  void AppendProperty(const std::string& prop,
                      const char* value,bool asString=false);
  const char *GetProperty(const std::string& prop) const;
  bool GetPropertyAsBool(const std::string& prop) const;

  /** Implement getting a property when called from a CMake language
      command like get_property or get_source_file_property.  */
  const char* GetPropertyForUser(const std::string& prop);

  /**
   * The full path to the file.  The non-const version of this method
   * may attempt to locate the file on disk and finalize its location.
   * The const version of this method may return an empty string if
   * the non-const version has not yet been called (yes this is a
   * horrible interface, but is necessary for backwards
   * compatibility).
   */
  std::string const& GetFullPath(std::string* error = 0);
  std::string const& GetFullPath() const;

  /**
   * Get the information currently known about the source file
   * location without attempting to locate the file as GetFullPath
   * would.  See cmSourceFileLocation documentation.
   */
  cmSourceFileLocation const& GetLocation() const;

  /**
   * Get the file extension of this source file.
   */
  std::string const& GetExtension() const;

  /**
   * Get the language of the compiler to use for this source file.
   */
  std::string GetLanguage();
  std::string GetLanguage() const;

  /**
   * Return the vector that holds the list of dependencies
   */
  const std::vector<std::string> &GetDepends() const {return this->Depends;}
  void AddDepend(const std::string& d) { this->Depends.push_back(d); }

  // Get the properties
  cmPropertyMap &GetProperties() { return this->Properties; }

  /**
   * Check whether the given source file location could refer to this
   * source.
   */
  bool Matches(cmSourceFileLocation const&);

  void SetObjectLibrary(std::string const& objlib);
  std::string GetObjectLibrary() const;

private:
  cmSourceFileLocation Location;
  cmPropertyMap Properties;
  cmCustomCommand* CustomCommand;
  std::string Extension;
  std::string Language;
  std::string FullPath;
  bool FindFullPathFailed;
  std::string ObjectLibrary;
  bool IsUiFile;

  bool FindFullPath(std::string* error);
  bool TryFullPath(const std::string& path, const std::string& ext);
  void CheckExtension();
  void CheckLanguage(std::string const& ext);

  std::vector<std::string> Depends;

  static const std::string propLANGUAGE;
};

// TODO: Factor out into platform information modules.
#define CM_HEADER_REGEX "\\.(h|hh|h\\+\\+|hm|hpp|hxx|in|txx|inl)$"

#endif
