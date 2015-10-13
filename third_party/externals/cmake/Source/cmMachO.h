/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmMachO_h
#define cmMachO_h

#if !defined(CMAKE_USE_MACH_PARSER)
# error "This file may be included only if CMAKE_USE_MACH_PARSER is enabled."
#endif

class cmMachOInternal;

/** \class cmMachO
 * \brief Executable and Link Format (Mach-O) parser.
 */
class cmMachO
{
public:
  /** Construct with the name of the Mach-O input file to parse.  */
  cmMachO(const char* fname);

  /** Destruct.   */
  ~cmMachO();

  /** Get the error message if any.  */
  std::string const& GetErrorMessage() const;

  /** Boolean conversion.  True if the Mach-O file is valid.  */
  operator bool() const { return this->Valid(); }

  /** Get Install name from binary **/
  bool GetInstallName(std::string& install_name);

  /** Print human-readable information about the Mach-O file.  */
  void PrintInfo(std::ostream& os) const;

private:
  friend class cmMachOInternal;
  bool Valid() const;
  cmMachOInternal* Internal;
};

#endif
