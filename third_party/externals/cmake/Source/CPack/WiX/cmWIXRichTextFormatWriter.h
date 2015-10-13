/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2012 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmWIXRichTextFormatWriter_h
#define cmWIXRichTextFormatWriter_h

#include <cmsys/FStream.hxx>

/** \class cmWIXRichtTextFormatWriter
 * \brief Helper class to generate Rich Text Format (RTF) documents
 * from plain text (e.g. for license and welcome text)
 */
class cmWIXRichTextFormatWriter
{
public:
  cmWIXRichTextFormatWriter(std::string const& filename);
  ~cmWIXRichTextFormatWriter();

  void AddText(std::string const& text);

private:
  void WriteHeader();
  void WriteFontTable();
  void WriteColorTable();
  void WriteGenerator();

  void WriteDocumentPrefix();

  void ControlWord(std::string const& keyword);
  void NewControlWord(std::string const& keyword);

  void StartGroup();
  void EndGroup();

  void EmitUnicodeCodepoint(int c);
  void EmitUnicodeSurrogate(int c);

  void EmitInvalidCodepoint(int c);

  cmsys::ofstream File;
};

#endif
