/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCTestGlobalVC.h"

#include "cmCTest.h"
#include "cmSystemTools.h"
#include "cmXMLWriter.h"

#include <cmsys/RegularExpression.hxx>

//----------------------------------------------------------------------------
cmCTestGlobalVC::cmCTestGlobalVC(cmCTest* ct, std::ostream& log):
  cmCTestVC(ct, log)
{
  this->PriorRev = this->Unknown;
}

//----------------------------------------------------------------------------
cmCTestGlobalVC::~cmCTestGlobalVC()
{
}

//----------------------------------------------------------------------------
const char* cmCTestGlobalVC::LocalPath(std::string const& path)
{
  return path.c_str();
}

//----------------------------------------------------------------------------
void cmCTestGlobalVC::DoRevision(Revision const& revision,
                                 std::vector<Change> const& changes)
{
  // Ignore changes in the old revision.
  if(revision.Rev == this->OldRevision)
    {
    this->PriorRev = revision;
    return;
    }

  // Indicate we found a revision.
  cmCTestLog(this->CTest, HANDLER_OUTPUT, "." << std::flush);

  // Store the revision.
  this->Revisions.push_back(revision);

  // Report this revision.
  Revision const& rev = this->Revisions.back();
  this->Log << "Found revision " << rev.Rev << "\n"
            << "  author = " << rev.Author << "\n"
            << "  date = " << rev.Date << "\n";

  // Update information about revisions of the changed files.
  for(std::vector<Change>::const_iterator ci = changes.begin();
      ci != changes.end(); ++ci)
    {
    if(const char* local = this->LocalPath(ci->Path))
      {
      std::string dir = cmSystemTools::GetFilenamePath(local);
      std::string name = cmSystemTools::GetFilenameName(local);
      File& file = this->Dirs[dir][name];
      file.PriorRev = file.Rev? file.Rev : &this->PriorRev;
      file.Rev = &rev;
      this->Log << "  " << ci->Action << " " << local << " " << "\n";
      }
    }
}

//----------------------------------------------------------------------------
void cmCTestGlobalVC::DoModification(PathStatus status,
                                     std::string const& path)
{
  std::string dir = cmSystemTools::GetFilenamePath(path);
  std::string name = cmSystemTools::GetFilenameName(path);
  File& file = this->Dirs[dir][name];
  file.Status = status;
  // For local modifications the current rev is unknown and the
  // prior rev is the latest from svn.
  if(!file.Rev && !file.PriorRev)
    {
    file.PriorRev = &this->PriorRev;
    }
}

//----------------------------------------------------------------------------
void cmCTestGlobalVC::WriteXMLDirectory(cmXMLWriter& xml,
                                        std::string const& path,
                                        Directory const& dir)
{
  const char* slash = path.empty()? "":"/";
  xml.StartElement("Directory");
  xml.Element("Name", path);
  for(Directory::const_iterator fi = dir.begin(); fi != dir.end(); ++fi)
    {
    std::string full = path + slash + fi->first;
    this->WriteXMLEntry(xml, path, fi->first, full, fi->second);
    }
  xml.EndElement(); // Directory
}

//----------------------------------------------------------------------------
void cmCTestGlobalVC::WriteXMLGlobal(cmXMLWriter& xml)
{
  if(!this->NewRevision.empty())
    {
    xml.Element("Revision", this->NewRevision);
    }
  if(!this->OldRevision.empty() && this->OldRevision != this->NewRevision)
    {
    xml.Element("PriorRevision", this->OldRevision);
    }
}

//----------------------------------------------------------------------------
bool cmCTestGlobalVC::WriteXMLUpdates(cmXMLWriter& xml)
{
  cmCTestLog(this->CTest, HANDLER_OUTPUT,
             "   Gathering version information (one . per revision):\n"
             "    " << std::flush);
  this->LoadRevisions();
  cmCTestLog(this->CTest, HANDLER_OUTPUT, std::endl);

  this->LoadModifications();

  this->WriteXMLGlobal(xml);

  for(std::map<std::string, Directory>::const_iterator
        di = this->Dirs.begin(); di != this->Dirs.end(); ++di)
    {
    this->WriteXMLDirectory(xml, di->first, di->second);
    }

  return true;
}
