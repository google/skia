/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmState_h
#define cmState_h

#include "cmStandardIncludes.h"
#include "cmPropertyDefinitionMap.h"
#include "cmPropertyMap.h"

class cmake;
class cmCommand;

class cmState
{
  typedef std::vector<std::string>::size_type PositionType;
  friend class Snapshot;
public:
  cmState(cmake* cm);
  ~cmState();

  class Snapshot {
  public:
    Snapshot(cmState* state = 0, PositionType position = 0);

    const char* GetCurrentSourceDirectory() const;
    void SetCurrentSourceDirectory(std::string const& dir);
    const char* GetCurrentBinaryDirectory() const;
    void SetCurrentBinaryDirectory(std::string const& dir);

    std::vector<std::string> const& GetCurrentSourceDirectoryComponents();
    std::vector<std::string> const& GetCurrentBinaryDirectoryComponents();

    const char* GetRelativePathTopSource() const;
    const char* GetRelativePathTopBinary() const;
    void SetRelativePathTopSource(const char* dir);
    void SetRelativePathTopBinary(const char* dir);

    bool IsValid() const;
    Snapshot GetParent() const;

  private:
    void ComputeRelativePathTopSource();
    void ComputeRelativePathTopBinary();

  private:
    friend class cmState;
    cmState* State;
    cmState::PositionType Position;
  };

  Snapshot CreateSnapshot(Snapshot originSnapshot);

  enum CacheEntryType{ BOOL=0, PATH, FILEPATH, STRING, INTERNAL,STATIC,
                       UNINITIALIZED };
  static CacheEntryType StringToCacheEntryType(const char*);
  static const char* CacheEntryTypeToString(CacheEntryType);
  static bool IsCacheEntryType(std::string const& key);

  std::vector<std::string> GetCacheEntryKeys() const;
  const char* GetCacheEntryValue(std::string const& key) const;
  const char* GetInitializedCacheValue(std::string const& key) const;
  CacheEntryType GetCacheEntryType(std::string const& key) const;
  void SetCacheEntryValue(std::string const& key, std::string const& value);
  void SetCacheValue(std::string const& key, std::string const& value);

  void AddCacheEntry(const std::string& key, const char* value,
                     const char* helpString, CacheEntryType type);
  void RemoveCacheEntry(std::string const& key);

  void SetCacheEntryProperty(std::string const& key,
                             std::string const& propertyName,
                             std::string const& value);
  void SetCacheEntryBoolProperty(std::string const& key,
                                 std::string const& propertyName,
                                 bool value);
  const char* GetCacheEntryProperty(std::string const& key,
                                    std::string const& propertyName);
  bool GetCacheEntryPropertyAsBool(std::string const& key,
                                   std::string const& propertyName);
  void AppendCacheEntryProperty(std::string const& key,
                                const std::string& property,
                                const std::string& value,
                                bool asString = false);
  void RemoveCacheEntryProperty(std::string const& key,
                                std::string const& propertyName);

  void Reset();
  // Define a property
  void DefineProperty(const std::string& name, cmProperty::ScopeType scope,
                      const char *ShortDescription,
                      const char *FullDescription,
                      bool chain = false);

  // get property definition
  cmPropertyDefinition *GetPropertyDefinition
  (const std::string& name, cmProperty::ScopeType scope);

  // Is a property defined?
  bool IsPropertyDefined(const std::string& name, cmProperty::ScopeType scope);
  bool IsPropertyChained(const std::string& name, cmProperty::ScopeType scope);

  void SetLanguageEnabled(std::string const& l);
  bool GetLanguageEnabled(std::string const& l) const;
  std::vector<std::string> GetEnabledLanguages() const;
  void SetEnabledLanguages(std::vector<std::string> const& langs);
  void ClearEnabledLanguages();

  bool GetIsInTryCompile() const;
  void SetIsInTryCompile(bool b);

  cmCommand* GetCommand(std::string const& name) const;
  void AddCommand(cmCommand* command);
  void RemoveUnscriptableCommands();
  void RenameCommand(std::string const& oldName, std::string const& newName);
  void RemoveUserDefinedCommands();
  std::vector<std::string> GetCommandNames() const;

  void SetGlobalProperty(const std::string& prop, const char *value);
  void AppendGlobalProperty(const std::string& prop,
                      const char *value,bool asString=false);
  const char *GetGlobalProperty(const std::string& prop);
  bool GetGlobalPropertyAsBool(const std::string& prop);

  const char* GetSourceDirectory() const;
  void SetSourceDirectory(std::string const& sourceDirectory);
  const char* GetBinaryDirectory() const;
  void SetBinaryDirectory(std::string const& binaryDirectory);

  std::vector<std::string> const& GetSourceDirectoryComponents() const;
  std::vector<std::string> const& GetBinaryDirectoryComponents() const;

  void SetWindowsShell(bool windowsShell);
  bool UseWindowsShell() const;
  void SetWindowsVSIDE(bool windowsVSIDE);
  bool UseWindowsVSIDE() const;
  void SetWatcomWMake(bool watcomWMake);
  bool UseWatcomWMake() const;
  void SetMinGWMake(bool minGWMake);
  bool UseMinGWMake() const;
  void SetNMake(bool nMake);
  bool UseNMake() const;
  void SetMSYSShell(bool mSYSShell);
  bool UseMSYSShell() const;

private:
  std::map<cmProperty::ScopeType, cmPropertyDefinitionMap> PropertyDefinitions;
  std::vector<std::string> EnabledLanguages;
  std::map<std::string, cmCommand*> Commands;
  cmPropertyMap GlobalProperties;
  cmake* CMakeInstance;
  std::vector<std::string> Locations;
  std::vector<std::string> OutputLocations;
  std::vector<PositionType> ParentPositions;

  std::vector<std::vector<std::string> > CurrentSourceDirectoryComponents;
  std::vector<std::vector<std::string> > CurrentBinaryDirectoryComponents;
  // The top-most directories for relative path conversion.  Both the
  // source and destination location of a relative path conversion
  // must be underneath one of these directories (both under source or
  // both under binary) in order for the relative path to be evaluated
  // safely by the build tools.
  std::vector<std::string> RelativePathTopSource;
  std::vector<std::string> RelativePathTopBinary;

  std::vector<std::string> SourceDirectoryComponents;
  std::vector<std::string> BinaryDirectoryComponents;
  std::string SourceDirectory;
  std::string BinaryDirectory;
  bool IsInTryCompile;
  bool WindowsShell;
  bool WindowsVSIDE;
  bool WatcomWMake;
  bool MinGWMake;
  bool NMake;
  bool MSYSShell;
};

#endif
