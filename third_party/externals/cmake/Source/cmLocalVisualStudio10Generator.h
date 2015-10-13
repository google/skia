/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmLocalVisualStudio10Generator_h
#define cmLocalVisualStudio10Generator_h

#include "cmLocalVisualStudio7Generator.h"


/** \class cmLocalVisualStudio10Generator
 * \brief Write Visual Studio 10 project files.
 *
 * cmLocalVisualStudio10Generator produces a Visual Studio 10 project
 * file for each target in its directory.
 */
class cmLocalVisualStudio10Generator : public cmLocalVisualStudio7Generator
{
public:
  ///! Set cache only and recurse to false by default.
  cmLocalVisualStudio10Generator(cmGlobalGenerator* gg,
                                 cmLocalGenerator* parent,
                                 cmState::Snapshot snapshot);

  virtual ~cmLocalVisualStudio10Generator();


  /**
   * Generate the makefile for this directory.
   */
  virtual void Generate();
  virtual void ReadAndStoreExternalGUID(const std::string& name,
                                        const char* path);

protected:
  virtual const char* ReportErrorLabel() const;
  virtual bool CustomCommandUseLocal() const { return true; }

private:
};
#endif
