/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmELF_h
#define cmELF_h

#if !defined(CMAKE_USE_ELF_PARSER)
# error "This file may be included only if CMAKE_USE_ELF_PARSER is enabled."
#endif

class cmELFInternal;

/** \class cmELF
 * \brief Executable and Link Format (ELF) parser.
 */
class cmELF
{
public:
  /** Construct with the name of the ELF input file to parse.  */
  cmELF(const char* fname);

  /** Destruct.   */
  ~cmELF();

  /** Get the error message if any.  */
  std::string const& GetErrorMessage() const
    {
    return this->ErrorMessage;
    }

  /** Boolean conversion.  True if the ELF file is valid.  */
  operator bool() const { return this->Valid(); }

  /** Enumeration of ELF file types.  */
  enum FileType
  {
    FileTypeInvalid,
    FileTypeRelocatableObject,
    FileTypeExecutable,
    FileTypeSharedLibrary,
    FileTypeCore,
    FileTypeSpecificOS,
    FileTypeSpecificProc
  };

  /** Represent string table entries.  */
  struct StringEntry
  {
    // The string value itself.
    std::string Value;

    // The position in the file at which the string appears.
    unsigned long Position;

    // The size of the string table entry.  This includes the space
    // allocated for one or more null terminators.
    unsigned long Size;

    // The index of the section entry referencing the string.
    int IndexInSection;
  };

  /** Get the type of the file opened.  */
  FileType GetFileType() const;

  /** Get the number of ELF sections present.  */
  unsigned int GetNumberOfSections() const;

  /** Get the number of DYNAMIC section entries before the first
      DT_NULL.  Returns zero on error.  */
  unsigned int GetDynamicEntryCount() const;

  /** Get the position of a DYNAMIC section header entry.  Returns
      zero on error.  */
  unsigned long GetDynamicEntryPosition(int index) const;

  /** Read bytes from the file.  */
  bool ReadBytes(unsigned long pos, unsigned long size, char* buf) const;

  /** Get the SONAME field if any.  */
  bool GetSOName(std::string& soname);
  StringEntry const* GetSOName();

  /** Get the RPATH field if any.  */
  StringEntry const* GetRPath();

  /** Get the RUNPATH field if any.  */
  StringEntry const* GetRunPath();

  /** Print human-readable information about the ELF file.  */
  void PrintInfo(std::ostream& os) const;

private:
  friend class cmELFInternal;
  bool Valid() const;
  cmELFInternal* Internal;
  std::string ErrorMessage;
};

#endif
