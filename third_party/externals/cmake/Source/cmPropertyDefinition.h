/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmPropertyDefinition_h
#define cmPropertyDefinition_h

#include "cmProperty.h"

/** \class cmPropertyDefinition
 * \brief Property meta-information
 *
 * This class contains the following meta-information about property:
 * - Name;
 * - Various documentation strings;
 * - The scope of the property;
 * - If the property is chained.
 */
class cmPropertyDefinition
{
public:
  /// Define this property
  void DefineProperty(const std::string& name, cmProperty::ScopeType scope,
                      const char *ShortDescription,
                      const char *FullDescription,
                      bool chained);

  /// Default constructor
  cmPropertyDefinition() { this->Chained = false; }

  /// Is the property chained?
  bool IsChained() const { return this->Chained; }

  /// Get the scope
  cmProperty::ScopeType GetScope() const {
    return this->Scope; }

  /// Get the documentation (short version)
  const std::string &GetShortDescription() const {
    return this->ShortDescription; }

  /// Get the documentation (full version)
  const std::string &GetFullDescription() const {
    return this->FullDescription; }

protected:
  std::string Name;
  std::string ShortDescription;
  std::string FullDescription;
  cmProperty::ScopeType Scope;
  bool Chained;
};

#endif
