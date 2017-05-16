/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmStandardIncludes.h" // to get CMAKE_USE_ELF_PARSER first
#include "cmELF.h"

#include <cmsys/auto_ptr.hxx>
#include <cmsys/FStream.hxx>

// Need the native byte order of the running CPU.
#define cmsys_CPU_UNKNOWN_OKAY // We can decide at runtime if not known.
#include <cmsys/CPU.h>

// Include the ELF format information system header.
#if defined(__OpenBSD__)
# include <stdint.h>
# include <elf_abi.h>
#elif defined(__HAIKU__)
# include <elf32.h>
# include <elf64.h>
  typedef struct Elf32_Ehdr Elf32_Ehdr;
  typedef struct Elf32_Shdr Elf32_Shdr;
  typedef struct Elf32_Sym Elf32_Sym;
  typedef struct Elf32_Rel Elf32_Rel;
  typedef struct Elf32_Rela Elf32_Rela;
# define ELFMAG0 0x7F
# define ELFMAG1 'E'
# define ELFMAG2 'L'
# define ELFMAG3 'F'
# define ET_NONE 0
# define ET_REL 1
# define ET_EXEC 2
# define ET_DYN 3
# define ET_CORE 4
# define EM_386 3
# define EM_SPARC 2
# define EM_PPC 20
#else
# include <elf.h>
#endif
#if defined(__sun)
# include <sys/link.h> // For dynamic section information
#endif

//----------------------------------------------------------------------------
// Low-level byte swapping implementation.
template <size_t s> struct cmELFByteSwapSize {};
void cmELFByteSwap(char*, cmELFByteSwapSize<1> const&)
{
}
void cmELFByteSwap(char* data, cmELFByteSwapSize<2> const&)
{
  char one_byte;
  one_byte = data[0]; data[0] = data[1]; data[1] = one_byte;
}
void cmELFByteSwap(char* data, cmELFByteSwapSize<4> const&)
{
  char one_byte;
  one_byte = data[0]; data[0] = data[3]; data[3] = one_byte;
  one_byte = data[1]; data[1] = data[2]; data[2] = one_byte;
}
void cmELFByteSwap(char* data, cmELFByteSwapSize<8> const&)
{
  char one_byte;
  one_byte = data[0]; data[0] = data[7]; data[7] = one_byte;
  one_byte = data[1]; data[1] = data[6]; data[6] = one_byte;
  one_byte = data[2]; data[2] = data[5]; data[5] = one_byte;
  one_byte = data[3]; data[3] = data[4]; data[4] = one_byte;
}

// Low-level byte swapping interface.
template <typename T>
void cmELFByteSwap(T& x)
{
  cmELFByteSwap(reinterpret_cast<char*>(&x), cmELFByteSwapSize<sizeof(T)>());
}

//----------------------------------------------------------------------------
class cmELFInternal
{
public:
  typedef cmELF::StringEntry StringEntry;
  enum ByteOrderType { ByteOrderMSB, ByteOrderLSB };

  // Construct and take ownership of the file stream object.
  cmELFInternal(cmELF* external,
                cmsys::auto_ptr<cmsys::ifstream>& fin,
                ByteOrderType order):
    External(external),
    Stream(*fin.release()),
    ByteOrder(order),
    ELFType(cmELF::FileTypeInvalid)
    {
    // In most cases the processor-specific byte order will match that
    // of the target execution environment.  If we choose wrong here
    // it is fixed when the header is read.
#if cmsys_CPU_ENDIAN_ID == cmsys_CPU_ENDIAN_ID_LITTLE
    this->NeedSwap = (this->ByteOrder == ByteOrderMSB);
#elif cmsys_CPU_ENDIAN_ID == cmsys_CPU_ENDIAN_ID_BIG
    this->NeedSwap = (this->ByteOrder == ByteOrderLSB);
#else
    this->NeedSwap = false; // Final decision is at runtime anyway.
#endif

    // We have not yet loaded the section info.
    this->DynamicSectionIndex = -1;
    }

  // Destruct and delete the file stream object.
  virtual ~cmELFInternal()
    {
    delete &this->Stream;
    }

  // Forward to the per-class implementation.
  virtual unsigned int GetNumberOfSections() const = 0;
  virtual unsigned int GetDynamicEntryCount() = 0;
  virtual unsigned long GetDynamicEntryPosition(int j) = 0;
  virtual StringEntry const* GetDynamicSectionString(unsigned int tag) = 0;
  virtual void PrintInfo(std::ostream& os) const = 0;

  bool ReadBytes(unsigned long pos, unsigned long size, char* buf)
    {
    this->Stream.seekg(pos);
    this->Stream.read(buf, size);
    return this->Stream?true:false;
    }

  // Lookup the SONAME in the DYNAMIC section.
  StringEntry const* GetSOName()
    {
    return this->GetDynamicSectionString(DT_SONAME);
    }

  // Lookup the RPATH in the DYNAMIC section.
  StringEntry const* GetRPath()
    {
    return this->GetDynamicSectionString(DT_RPATH);
    }

  // Lookup the RUNPATH in the DYNAMIC section.
  StringEntry const* GetRunPath()
    {
#if defined(DT_RUNPATH)
    return this->GetDynamicSectionString(DT_RUNPATH);
#else
    return 0;
#endif
    }

  // Return the recorded ELF type.
  cmELF::FileType GetFileType() const { return this->ELFType; }
protected:
  // Data common to all ELF class implementations.

  // The external cmELF object.
  cmELF* External;

  // The stream from which to read.
  std::istream& Stream;

  // The byte order of the ELF file.
  ByteOrderType ByteOrder;

  // The ELF file type.
  cmELF::FileType ELFType;

  // Whether we need to byte-swap structures read from the stream.
  bool NeedSwap;

  // The section header index of the DYNAMIC section (-1 if none).
  int DynamicSectionIndex;

  // Helper methods for subclasses.
  void SetErrorMessage(const char* msg)
    {
    this->External->ErrorMessage = msg;
    this->ELFType = cmELF::FileTypeInvalid;
    }

  // Store string table entry states.
  std::map<unsigned int, StringEntry> DynamicSectionStrings;
};

//----------------------------------------------------------------------------
// Configure the implementation template for 32-bit ELF files.
struct cmELFTypes32
{
  typedef Elf32_Ehdr ELF_Ehdr;
  typedef Elf32_Shdr ELF_Shdr;
  typedef Elf32_Dyn  ELF_Dyn;
  typedef Elf32_Half ELF_Half;
  typedef cmIML_INT_uint32_t tagtype;
  static const char* GetName() { return "32-bit"; }
};

// Configure the implementation template for 64-bit ELF files.
struct cmELFTypes64
{
  typedef Elf64_Ehdr ELF_Ehdr;
  typedef Elf64_Shdr ELF_Shdr;
  typedef Elf64_Dyn  ELF_Dyn;
  typedef Elf64_Half ELF_Half;
  typedef cmIML_INT_uint64_t tagtype;
  static const char* GetName() { return "64-bit"; }
};

//----------------------------------------------------------------------------
// Parser implementation template.
template <class Types>
class cmELFInternalImpl: public cmELFInternal
{
public:
  // Copy the ELF file format types from our configuration parameter.
  typedef typename Types::ELF_Ehdr ELF_Ehdr;
  typedef typename Types::ELF_Shdr ELF_Shdr;
  typedef typename Types::ELF_Dyn  ELF_Dyn;
  typedef typename Types::ELF_Half ELF_Half;
  typedef typename Types::tagtype tagtype;

  // Construct with a stream and byte swap indicator.
  cmELFInternalImpl(cmELF* external,
                    cmsys::auto_ptr<cmsys::ifstream>& fin,
                    ByteOrderType order);

  // Return the number of sections as specified by the ELF header.
  virtual unsigned int GetNumberOfSections() const
    {
    return static_cast<unsigned int>(this->ELFHeader.e_shnum);
    }

  // Get the file position and size of a dynamic section entry.
  virtual unsigned int GetDynamicEntryCount();
  virtual unsigned long GetDynamicEntryPosition(int j);

  // Lookup a string from the dynamic section with the given tag.
  virtual StringEntry const* GetDynamicSectionString(unsigned int tag);

  // Print information about the ELF file.
  virtual void PrintInfo(std::ostream& os) const
    {
    os << "ELF " << Types::GetName();
    if(this->ByteOrder == ByteOrderMSB)
      {
      os << " MSB";
      }
    else if(this->ByteOrder == ByteOrderLSB)
      {
      os << " LSB";
      }
    switch(this->ELFType)
      {
      case cmELF::FileTypeInvalid:
        os << " invalid file";
        break;
      case cmELF::FileTypeRelocatableObject:
        os << " relocatable object";
        break;
      case cmELF::FileTypeExecutable:
        os << " executable";
        break;
      case cmELF::FileTypeSharedLibrary:
        os << " shared library";
        break;
      case cmELF::FileTypeCore:
        os << " core file";
        break;
      case cmELF::FileTypeSpecificOS:
        os << " os-specific type";
        break;
      case cmELF::FileTypeSpecificProc:
        os << " processor-specific type";
        break;
      }
    os << "\n";
    }

private:
  void ByteSwap(ELF_Ehdr& elf_header)
    {
    cmELFByteSwap(elf_header.e_type);
    cmELFByteSwap(elf_header.e_machine);
    cmELFByteSwap(elf_header.e_version);
    cmELFByteSwap(elf_header.e_entry);
    cmELFByteSwap(elf_header.e_phoff);
    cmELFByteSwap(elf_header.e_shoff);
    cmELFByteSwap(elf_header.e_flags);
    cmELFByteSwap(elf_header.e_ehsize);
    cmELFByteSwap(elf_header.e_phentsize);
    cmELFByteSwap(elf_header.e_phnum);
    cmELFByteSwap(elf_header.e_shentsize);
    cmELFByteSwap(elf_header.e_shnum);
    cmELFByteSwap(elf_header.e_shstrndx);
    }

  void ByteSwap(ELF_Shdr& sec_header)
    {
    cmELFByteSwap(sec_header.sh_name);
    cmELFByteSwap(sec_header.sh_type);
    cmELFByteSwap(sec_header.sh_flags);
    cmELFByteSwap(sec_header.sh_addr);
    cmELFByteSwap(sec_header.sh_offset);
    cmELFByteSwap(sec_header.sh_size);
    cmELFByteSwap(sec_header.sh_link);
    cmELFByteSwap(sec_header.sh_info);
    cmELFByteSwap(sec_header.sh_addralign);
    cmELFByteSwap(sec_header.sh_entsize);
    }

  void ByteSwap(ELF_Dyn& dyn)
    {
    cmELFByteSwap(dyn.d_tag);
    switch (dyn.d_tag)
      {
      case DT_NULL:     /* dyn.d_un ignored */         break;
      case DT_NEEDED:   cmELFByteSwap(dyn.d_un.d_val); break;
      case DT_PLTRELSZ: cmELFByteSwap(dyn.d_un.d_val); break;
      case DT_PLTGOT:   cmELFByteSwap(dyn.d_un.d_ptr); break;
      case DT_HASH:     cmELFByteSwap(dyn.d_un.d_ptr); break;
      case DT_STRTAB:   cmELFByteSwap(dyn.d_un.d_ptr); break;
      case DT_SYMTAB:   cmELFByteSwap(dyn.d_un.d_ptr); break;
      case DT_RELA:     cmELFByteSwap(dyn.d_un.d_ptr); break;
      case DT_RELASZ:   cmELFByteSwap(dyn.d_un.d_val); break;
      case DT_RELAENT:  cmELFByteSwap(dyn.d_un.d_val); break;
      case DT_STRSZ:    cmELFByteSwap(dyn.d_un.d_val); break;
      case DT_SYMENT:   cmELFByteSwap(dyn.d_un.d_val); break;
      case DT_INIT:     cmELFByteSwap(dyn.d_un.d_ptr); break;
      case DT_FINI:     cmELFByteSwap(dyn.d_un.d_ptr); break;
      case DT_SONAME:   cmELFByteSwap(dyn.d_un.d_val); break;
      case DT_RPATH:    cmELFByteSwap(dyn.d_un.d_val); break;
      case DT_SYMBOLIC: /* dyn.d_un ignored */         break;
      case DT_REL:      cmELFByteSwap(dyn.d_un.d_ptr); break;
      case DT_RELSZ:    cmELFByteSwap(dyn.d_un.d_val); break;
      case DT_RELENT:   cmELFByteSwap(dyn.d_un.d_val); break;
      case DT_PLTREL:   cmELFByteSwap(dyn.d_un.d_val); break;
      case DT_DEBUG:    cmELFByteSwap(dyn.d_un.d_ptr); break;
      case DT_TEXTREL:  /* dyn.d_un ignored */         break;
      case DT_JMPREL:   cmELFByteSwap(dyn.d_un.d_ptr); break;
#ifdef T_BIND_NOW
      case T_BIND_NOW:         /* dyn.d_un ignored */         break;
#endif
#ifdef DT_INIT_ARRAY
      case DT_INIT_ARRAY:      cmELFByteSwap(dyn.d_un.d_ptr); break;
#endif
#ifdef DT_FINI_ARRAY
      case DT_FINI_ARRAY:      cmELFByteSwap(dyn.d_un.d_ptr); break;
#endif
#ifdef DT_INIT_ARRAYSZ
      case DT_INIT_ARRAYSZ:    cmELFByteSwap(dyn.d_un.d_val); break;
#endif
#ifdef DT_FINI_ARRAYSZ
      case DT_FINI_ARRAYSZ:    cmELFByteSwap(dyn.d_un.d_val); break;
#endif
#ifdef DT_RUNPATH
      case DT_RUNPATH:         cmELFByteSwap(dyn.d_un.d_val); break;
#endif
#ifdef DT_FLAGS
      case DT_FLAGS:           cmELFByteSwap(dyn.d_un.d_val); break;
#endif
#ifdef DT_PREINIT_ARRAY
      case DT_PREINIT_ARRAY:   cmELFByteSwap(dyn.d_un.d_ptr); break;
#endif
#ifdef DT_PREINIT_ARRAYSZ
      case DT_PREINIT_ARRAYSZ: cmELFByteSwap(dyn.d_un.d_val); break;
#endif
      }
    }

  bool FileTypeValid(ELF_Half et)
    {
    unsigned int eti = static_cast<unsigned int>(et);
    if(eti == ET_NONE || eti == ET_REL || eti == ET_EXEC ||
       eti == ET_DYN || eti == ET_CORE)
      {
      return true;
      }
#if defined(ET_LOOS) && defined(ET_HIOS)
    if(eti >= ET_LOOS && eti <= ET_HIOS)
      {
      return true;
      }
#endif
#if defined(ET_LOPROC) && defined(ET_HIPROC)
    if(eti >= ET_LOPROC && eti <= ET_HIPROC)
      {
      return true;
      }
#endif
    return false;
    }

  bool Read(ELF_Ehdr& x)
    {
    // Read the header from the file.
    if(!this->Stream.read(reinterpret_cast<char*>(&x), sizeof(x)))
      {
      return false;
      }

    // The byte order of ELF header fields may not match that of the
    // processor-specific data.  The header fields are ordered to
    // match the target execution environment, so we may need to
    // memorize the order of all platforms based on the e_machine
    // value.  As a heuristic, if the type is invalid but its
    // swapped value is okay then flip our swap mode.
    ELF_Half et = x.e_type;
    if(this->NeedSwap)
      {
      cmELFByteSwap(et);
      }
    if(!this->FileTypeValid(et))
      {
      cmELFByteSwap(et);
      if(this->FileTypeValid(et))
        {
        // The previous byte order guess was wrong.  Flip it.
        this->NeedSwap = !this->NeedSwap;
        }
      }

    // Fix the byte order of the header.
    if(this->NeedSwap)
      {
      ByteSwap(x);
      }
    return true;
    }
  bool Read(ELF_Shdr& x)
    {
    if(this->Stream.read(reinterpret_cast<char*>(&x), sizeof(x)) &&
       this->NeedSwap)
      {
      ByteSwap(x);
      }
    return this->Stream? true:false;
    }
  bool Read(ELF_Dyn& x)
    {
    if(this->Stream.read(reinterpret_cast<char*>(&x), sizeof(x)) &&
       this->NeedSwap)
      {
      ByteSwap(x);
      }
    return this->Stream? true:false;
    }

  bool LoadSectionHeader(ELF_Half i)
    {
    // Read the section header from the file.
    this->Stream.seekg(this->ELFHeader.e_shoff +
                       this->ELFHeader.e_shentsize * i);
    if(!this->Read(this->SectionHeaders[i]))
      {
      return false;
      }

    // Identify some important sections.
    if(this->SectionHeaders[i].sh_type == SHT_DYNAMIC)
      {
      this->DynamicSectionIndex = i;
      }
    return true;
    }

  bool LoadDynamicSection();

  // Store the main ELF header.
  ELF_Ehdr ELFHeader;

  // Store all the section headers.
  std::vector<ELF_Shdr> SectionHeaders;

  // Store all entries of the DYNAMIC section.
  std::vector<ELF_Dyn> DynamicSectionEntries;
};

//----------------------------------------------------------------------------
template <class Types>
cmELFInternalImpl<Types>
::cmELFInternalImpl(cmELF* external,
                    cmsys::auto_ptr<cmsys::ifstream>& fin,
                    ByteOrderType order):
  cmELFInternal(external, fin, order)
{
  // Read the main header.
  if(!this->Read(this->ELFHeader))
    {
    this->SetErrorMessage("Failed to read main ELF header.");
    return;
    }

  // Determine the ELF file type.
  switch(this->ELFHeader.e_type)
    {
    case ET_NONE:
      this->SetErrorMessage("ELF file type is NONE.");
      return;
    case ET_REL:
      this->ELFType = cmELF::FileTypeRelocatableObject;
      break;
    case ET_EXEC:
      this->ELFType = cmELF::FileTypeExecutable;
      break;
    case ET_DYN:
      this->ELFType = cmELF::FileTypeSharedLibrary;
      break;
    case ET_CORE:
      this->ELFType = cmELF::FileTypeCore;
      break;
    default:
      {
      unsigned int eti = static_cast<unsigned int>(this->ELFHeader.e_type);
#if defined(ET_LOOS) && defined(ET_HIOS)
      if(eti >= ET_LOOS && eti <= ET_HIOS)
        {
        this->ELFType = cmELF::FileTypeSpecificOS;
        break;
        }
#endif
#if defined(ET_LOPROC) && defined(ET_HIPROC)
      if(eti >= ET_LOPROC && eti <= ET_HIPROC)
        {
        this->ELFType = cmELF::FileTypeSpecificProc;
        break;
        }
#endif
      std::ostringstream e;
      e << "Unknown ELF file type " << eti;
      this->SetErrorMessage(e.str().c_str());
      return;
      }
    }

  // Load the section headers.
  this->SectionHeaders.resize(this->ELFHeader.e_shnum);
  for(ELF_Half i=0; i < this->ELFHeader.e_shnum; ++i)
    {
    if(!this->LoadSectionHeader(i))
      {
      this->SetErrorMessage("Failed to load section headers.");
      return;
      }
    }
}

//----------------------------------------------------------------------------
template <class Types>
bool cmELFInternalImpl<Types>::LoadDynamicSection()
{
  // If there is no dynamic section we are done.
  if(this->DynamicSectionIndex < 0)
    {
    return false;
    }

  // If the section was already loaded we are done.
  if(!this->DynamicSectionEntries.empty())
    {
    return true;
    }

  // Allocate the dynamic section entries.
  ELF_Shdr const& sec = this->SectionHeaders[this->DynamicSectionIndex];
  int n = static_cast<int>(sec.sh_size / sec.sh_entsize);
  this->DynamicSectionEntries.resize(n);

  // Read each entry.
  for(int j=0; j < n; ++j)
    {
    // Seek to the beginning of the section entry.
    this->Stream.seekg(sec.sh_offset + sec.sh_entsize*j);
    ELF_Dyn& dyn = this->DynamicSectionEntries[j];

    // Try reading the entry.
    if(!this->Read(dyn))
      {
      this->SetErrorMessage("Error reading entry from DYNAMIC section.");
      this->DynamicSectionIndex = -1;
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
template <class Types>
unsigned int cmELFInternalImpl<Types>::GetDynamicEntryCount()
{
  if(!this->LoadDynamicSection())
    {
    return 0;
    }
  for(unsigned int i = 0; i < this->DynamicSectionEntries.size(); ++i)
    {
    if(this->DynamicSectionEntries[i].d_tag == DT_NULL)
      {
      return i;
      }
    }
  return static_cast<unsigned int>(this->DynamicSectionEntries.size());
}

//----------------------------------------------------------------------------
template <class Types>
unsigned long cmELFInternalImpl<Types>::GetDynamicEntryPosition(int j)
{
  if(!this->LoadDynamicSection())
    {
    return 0;
    }
  if(j < 0 || j >= static_cast<int>(this->DynamicSectionEntries.size()))
    {
    return 0;
    }
  ELF_Shdr const& sec = this->SectionHeaders[this->DynamicSectionIndex];
  return static_cast<unsigned long>(sec.sh_offset + sec.sh_entsize*j);
}

//----------------------------------------------------------------------------
template <class Types>
cmELF::StringEntry const*
cmELFInternalImpl<Types>::GetDynamicSectionString(unsigned int tag)
{
  // Short-circuit if already checked.
  std::map<unsigned int, StringEntry>::iterator dssi =
    this->DynamicSectionStrings.find(tag);
  if(dssi != this->DynamicSectionStrings.end())
    {
    if(dssi->second.Position > 0)
      {
      return &dssi->second;
      }
    return 0;
    }

  // Create an entry for this tag.  Assume it is missing until found.
  StringEntry& se = this->DynamicSectionStrings[tag];
  se.Position = 0;
  se.Size = 0;
  se.IndexInSection = -1;

  // Try reading the dynamic section.
  if(!this->LoadDynamicSection())
    {
    return 0;
    }

  // Get the string table referenced by the DYNAMIC section.
  ELF_Shdr const& sec = this->SectionHeaders[this->DynamicSectionIndex];
  if(sec.sh_link >= this->SectionHeaders.size())
    {
    this->SetErrorMessage("Section DYNAMIC has invalid string table index.");
    return 0;
    }
  ELF_Shdr const& strtab = this->SectionHeaders[sec.sh_link];

  // Look for the requested entry.
  for(typename std::vector<ELF_Dyn>::iterator
        di = this->DynamicSectionEntries.begin();
      di != this->DynamicSectionEntries.end(); ++di)
    {
    ELF_Dyn& dyn = *di;
    if(static_cast<tagtype>(dyn.d_tag) == static_cast<tagtype>(tag))
      {
      // We found the tag requested.
      // Make sure the position given is within the string section.
      if(dyn.d_un.d_val >= strtab.sh_size)
        {
        this->SetErrorMessage("Section DYNAMIC references string beyond "
                              "the end of its string section.");
        return 0;
        }

      // Seek to the position reported by the entry.
      unsigned long first = static_cast<unsigned long>(dyn.d_un.d_val);
      unsigned long last = first;
      unsigned long end = static_cast<unsigned long>(strtab.sh_size);
      this->Stream.seekg(strtab.sh_offset + first);

      // Read the string.  It may be followed by more than one NULL
      // terminator.  Count the total size of the region allocated to
      // the string.  This assumes that the next string in the table
      // is non-empty, but the "chrpath" tool makes the same
      // assumption.
      bool terminated = false;
      char c;
      while(last != end && this->Stream.get(c) && !(terminated && c))
        {
        ++last;
        if(c)
          {
          se.Value += c;
          }
        else
          {
          terminated = true;
          }
        }

      // Make sure the whole value was read.
      if(!this->Stream)
        {
        this->SetErrorMessage("Dynamic section specifies unreadable RPATH.");
        se.Value = "";
        return 0;
        }

      // The value has been read successfully.  Report it.
      se.Position = static_cast<unsigned long>(strtab.sh_offset + first);
      se.Size = last - first;
      se.IndexInSection =
        static_cast<int>(di - this->DynamicSectionEntries.begin());
      return &se;
      }
    }
  return 0;
}

//============================================================================
// External class implementation.

//----------------------------------------------------------------------------
cmELF::cmELF(const char* fname): Internal(0)
{
  // Try to open the file.
  cmsys::auto_ptr<cmsys::ifstream> fin(new cmsys::ifstream(fname));

  // Quit now if the file could not be opened.
  if(!fin.get() || !*fin)
    {
    this->ErrorMessage = "Error opening input file.";
    return;
    }

  // Read the ELF identification block.
  char ident[EI_NIDENT];
  if(!fin->read(ident, EI_NIDENT))
    {
    this->ErrorMessage = "Error reading ELF identification.";
    return;
    }
  if(!fin->seekg(0))
    {
    this->ErrorMessage = "Error seeking to beginning of file.";
    return;
    }

  // Verify the ELF identification.
  if(!(ident[EI_MAG0] == ELFMAG0 &&
       ident[EI_MAG1] == ELFMAG1 &&
       ident[EI_MAG2] == ELFMAG2 &&
       ident[EI_MAG3] == ELFMAG3))
    {
    this->ErrorMessage = "File does not have a valid ELF identification.";
    return;
    }

  // Check the byte order in which the rest of the file is encoded.
  cmELFInternal::ByteOrderType order;
  if(ident[EI_DATA] == ELFDATA2LSB)
    {
    // File is LSB.
    order =  cmELFInternal::ByteOrderLSB;
    }
  else if(ident[EI_DATA] == ELFDATA2MSB)
    {
    // File is MSB.
    order =  cmELFInternal::ByteOrderMSB;
    }
  else
    {
    this->ErrorMessage = "ELF file is not LSB or MSB encoded.";
    return;
    }

  // Check the class of the file and construct the corresponding
  // parser implementation.
  if(ident[EI_CLASS] == ELFCLASS32)
    {
    // 32-bit ELF
    this->Internal = new cmELFInternalImpl<cmELFTypes32>(this, fin, order);
    }
  else if(ident[EI_CLASS] == ELFCLASS64)
    {
    // 64-bit ELF
    this->Internal = new cmELFInternalImpl<cmELFTypes64>(this, fin, order);
    }
  else
    {
    this->ErrorMessage = "ELF file class is not 32-bit or 64-bit.";
    return;
    }
}

//----------------------------------------------------------------------------
cmELF::~cmELF()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
bool cmELF::Valid() const
{
  return this->Internal && this->Internal->GetFileType() != FileTypeInvalid;
}

//----------------------------------------------------------------------------
cmELF::FileType cmELF::GetFileType() const
{
  if(this->Valid())
    {
    return this->Internal->GetFileType();
    }
  else
    {
    return FileTypeInvalid;
    }
}

//----------------------------------------------------------------------------
unsigned int cmELF::GetNumberOfSections() const
{
  if(this->Valid())
    {
    return this->Internal->GetNumberOfSections();
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
unsigned int cmELF::GetDynamicEntryCount() const
{
  if(this->Valid())
    {
    return this->Internal->GetDynamicEntryCount();
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
unsigned long cmELF::GetDynamicEntryPosition(int index) const
{
  if(this->Valid())
    {
    return this->Internal->GetDynamicEntryPosition(index);
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
bool cmELF::ReadBytes(unsigned long pos, unsigned long size, char* buf) const
{
  if(this->Valid())
    {
    return this->Internal->ReadBytes(pos, size, buf);
    }
  else
    {
    return false;
    }
}

//----------------------------------------------------------------------------
bool cmELF::GetSOName(std::string& soname)
{
  if(StringEntry const* se = this->GetSOName())
    {
    soname = se->Value;
    return true;
    }
  else
    {
    return false;
    }
}

//----------------------------------------------------------------------------
cmELF::StringEntry const* cmELF::GetSOName()
{
  if(this->Valid() &&
     this->Internal->GetFileType() == cmELF::FileTypeSharedLibrary)
    {
    return this->Internal->GetSOName();
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
cmELF::StringEntry const* cmELF::GetRPath()
{
  if(this->Valid() &&
     (this->Internal->GetFileType() == cmELF::FileTypeExecutable ||
      this->Internal->GetFileType() == cmELF::FileTypeSharedLibrary))
    {
    return this->Internal->GetRPath();
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
cmELF::StringEntry const* cmELF::GetRunPath()
{
  if(this->Valid() &&
     (this->Internal->GetFileType() == cmELF::FileTypeExecutable ||
      this->Internal->GetFileType() == cmELF::FileTypeSharedLibrary))
    {
    return this->Internal->GetRunPath();
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
void cmELF::PrintInfo(std::ostream& os) const
{
  if(this->Valid())
    {
    this->Internal->PrintInfo(os);
    }
  else
    {
    os << "Not a valid ELF file.\n";
    }
}
