/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmObject_h
#define cmObject_h

#include "cmStandardIncludes.h"

/** \class cmObject
 * \brief Superclass for all commands and other classes in CMake.
 *
 * cmObject is the base class for all classes in CMake. It defines some
 * methods such as GetNameOfClass, IsA, SafeDownCast.
 */
class cmObject
{
public:
  /**
   * Need virtual destructor to destroy real command type.
   */
  virtual ~cmObject() {}

  /**
   * The class name of the command.
   */
  virtual const char* GetNameOfClass() = 0;

  /**
   * Returns true if this class is the given class, or a subclass of it.
   */
  static bool IsTypeOf(const char *type)
    { return !strcmp("cmObject", type); }

  /**
   * Returns true if this object is an instance of the given class or
   * a subclass of it.
   */
  virtual bool IsA(const char *type)
    { return cmObject::IsTypeOf(type); }
};

#endif

