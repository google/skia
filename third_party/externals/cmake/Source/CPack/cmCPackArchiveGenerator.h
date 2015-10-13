/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackArchiveGenerator_h
#define cmCPackArchiveGenerator_h

#include "cmArchiveWrite.h"
#include "cmCPackGenerator.h"


/** \class cmCPackArchiveGenerator
 * \brief A generator base for libarchive generation.
 * The generator itself uses the libarchive wrapper
 * \ref cmArchiveWrite.
 *
 */
class cmCPackArchiveGenerator : public cmCPackGenerator
  {
public:
  cmTypeMacro(cmCPackArchiveGenerator, cmCPackGenerator);

  /**
   * Construct generator
   */
  cmCPackArchiveGenerator(cmArchiveWrite::Compress, std::string const& format);
  virtual ~cmCPackArchiveGenerator();
  // Used to add a header to the archive
  virtual int GenerateHeader(std::ostream* os);
  // component support
  virtual bool SupportsComponentInstallation() const;
protected:
  virtual int InitializeInternal();
  /**
   * Add the files belonging to the specified component
   * to the provided (already opened) archive.
   * @param[in,out] archive the archive object
   * @param[in] component the component whose file will be added to archive
   */
  int addOneComponentToArchive(cmArchiveWrite& archive,
                               cmCPackComponent* component);

  /**
   * The main package file method.
   * If component install was required this
   * method will call either PackageComponents or
   * PackageComponentsAllInOne.
   */
  int PackageFiles();
  /**
   * The method used to package files when component
   * install is used. This will create one
   * archive for each component group.
   */
  int PackageComponents(bool ignoreGroup);
  /**
   * Special case of component install where all
   * components will be put in a single installer.
   */
  int PackageComponentsAllInOne();
  virtual const char* GetOutputExtension() = 0;
  cmArchiveWrite::Compress Compress;
  std::string ArchiveFormat;
  };

#endif
