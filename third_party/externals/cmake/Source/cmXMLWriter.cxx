/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Daniel Pfeifer <daniel@pfeifer-mail.de>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmXMLWriter.h"
#include "cmXMLSafe.h"

#include <cassert>
#include <fstream>

cmXMLWriter::cmXMLWriter(std::ostream& output, std::size_t level)
: Output(output)
, Level(level)
, ElementOpen(false)
, BreakAttrib(false)
, IsContent(false)
{
}

cmXMLWriter::~cmXMLWriter()
{
  assert(this->Elements.empty());
}

void cmXMLWriter::StartDocument(const char* encoding)
{
  this->Output << "<?xml version=\"1.0\" encoding=\"" << encoding << "\"?>";
}

void cmXMLWriter::EndDocument()
{
  assert(this->Elements.empty());
  this->Output << '\n';
}

void cmXMLWriter::StartElement(std::string const& name)
{
  this->CloseStartElement();
  this->ConditionalLineBreak(!this->IsContent, this->Elements.size());
  this->Output << '<' << name;
  this->Elements.push(name);
  this->ElementOpen = true;
  this->BreakAttrib = false;
}

void cmXMLWriter::EndElement()
{
  assert(!this->Elements.empty());
  if (this->ElementOpen)
    {
    this->Output << "/>";
    }
  else
    {
    this->ConditionalLineBreak(!this->IsContent, this->Elements.size() - 1);
    this->IsContent = false;
    this->Output << "</" << this->Elements.top() << '>';
    }
  this->Elements.pop();
  this->ElementOpen = false;
}

void cmXMLWriter::BreakAttributes()
{
  this->BreakAttrib = true;
}

void cmXMLWriter::Comment(const char* comment)
{
  this->CloseStartElement();
  this->ConditionalLineBreak(!this->IsContent, this->Elements.size());
  this->Output << "<!-- " << comment << " -->";
}

void cmXMLWriter::CData(std::string const& data)
{
  this->PreContent();
  this->Output << "<![CDATA[" << data << "]]>";
}

void cmXMLWriter::ProcessingInstruction(const char* target, const char* data)
{
  this->CloseStartElement();
  this->ConditionalLineBreak(!this->IsContent, this->Elements.size());
  this->Output << "<?" << target << ' ' << data << "?>";
}

void cmXMLWriter::FragmentFile(const char* fname)
{
  this->CloseStartElement();
  std::ifstream fin(fname, std::ios::in | std::ios::binary);
  this->Output << fin.rdbuf();
}

void cmXMLWriter::ConditionalLineBreak(bool condition, std::size_t indent)
{
  if (condition)
    {
    this->Output << '\n' << std::string(indent + this->Level, '\t');
    }
}

void cmXMLWriter::PreAttribute()
{
  assert(this->ElementOpen);
  this->ConditionalLineBreak(this->BreakAttrib, this->Elements.size());
  if (!this->BreakAttrib)
    {
    this->Output << ' ';
    }
}

void cmXMLWriter::PreContent()
{
  this->CloseStartElement();
  this->IsContent = true;
}

void cmXMLWriter::CloseStartElement()
{
  if (this->ElementOpen)
    {
    this->ConditionalLineBreak(this->BreakAttrib, this->Elements.size());
    this->Output << '>';
    this->ElementOpen = false;
    }
}
