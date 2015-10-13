/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmVariableWatch_h
#define cmVariableWatch_h

#include "cmStandardIncludes.h"

class cmMakefile;

/** \class cmVariableWatch
 * \brief Helper class for watching of variable accesses.
 *
 * Calls function when variable is accessed
 */
class cmVariableWatch
{
public:
  typedef void (*WatchMethod)(const std::string& variable, int access_type,
    void* client_data, const char* newValue, const cmMakefile* mf);
  typedef void (*DeleteData)(void* client_data);

  cmVariableWatch();
  ~cmVariableWatch();

  /**
   * Add watch to the variable
   */
  bool AddWatch(const std::string& variable, WatchMethod method,
                void* client_data=0, DeleteData delete_data=0);
  void RemoveWatch(const std::string& variable, WatchMethod method,
                   void* client_data=0);

  /**
   * This method is called when variable is accessed
   */
  void VariableAccessed(const std::string& variable, int access_type,
    const char* newValue, const cmMakefile* mf) const;

  /**
   * Different access types.
   */
  enum
    {
    VARIABLE_READ_ACCESS = 0,
    UNKNOWN_VARIABLE_READ_ACCESS,
    UNKNOWN_VARIABLE_DEFINED_ACCESS,
    VARIABLE_MODIFIED_ACCESS,
    VARIABLE_REMOVED_ACCESS,
    NO_ACCESS
    };

  /**
   * Return the access as string
   */
  static const char* GetAccessAsString(int access_type);

protected:
  struct Pair
  {
    WatchMethod Method;
    void*        ClientData;
    DeleteData   DeleteDataCall;
    Pair() : Method(0), ClientData(0), DeleteDataCall(0) {}
    ~Pair()
      {
      if (this->DeleteDataCall && this->ClientData)
        {
        this->DeleteDataCall(this->ClientData);
        }
      }
  };

  typedef std::vector< Pair* > VectorOfPairs;
  typedef std::map<std::string, VectorOfPairs > StringToVectorOfPairs;

  StringToVectorOfPairs WatchMap;
};


#endif
