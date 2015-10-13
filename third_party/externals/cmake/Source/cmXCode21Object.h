/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmXCode21Object_h
#define cmXCode21Object_h

#include "cmXCodeObject.h"

class cmXCode21Object : public cmXCodeObject
{
public:
  cmXCode21Object(PBXType ptype, Type type);
  virtual void PrintComment(std::ostream&);
  static void PrintList(std::vector<cmXCodeObject*> const&,
                        std::ostream& out,
                        PBXType t);
  static void PrintList(std::vector<cmXCodeObject*> const&,
                        std::ostream& out);
};
#endif
