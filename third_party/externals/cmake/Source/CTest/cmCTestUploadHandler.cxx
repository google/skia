/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCTestUploadHandler.h"

#include "cmGeneratedFileStream.h"
#include "cmVersion.h"
#include "cmXMLWriter.h"

//----------------------------------------------------------------------------
cmCTestUploadHandler::cmCTestUploadHandler()
{
  this->Initialize();
}

//----------------------------------------------------------------------------
void cmCTestUploadHandler::Initialize()
{
  this->Superclass::Initialize();
  this->Files.clear();
}

void cmCTestUploadHandler::SetFiles(const cmCTest::SetOfStrings& files)
{
  this->Files = files;
}

//----------------------------------------------------------------------------
int cmCTestUploadHandler::ProcessHandler()
{
  cmGeneratedFileStream ofs;
  if ( !this->CTest->OpenOutputFile(this->CTest->GetCurrentTag(),
                                    "Upload.xml", ofs))
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
      "Cannot open Upload.xml file" << std::endl);
    return -1;
    }
  std::string buildname = cmCTest::SafeBuildIdField(
    this->CTest->GetCTestConfiguration("BuildName"));
  cmCTest::SetOfStrings::const_iterator it;

  cmXMLWriter xml(ofs);
  xml.StartDocument();
  xml.ProcessingInstruction("xml-stylesheet", "type=\"text/xsl\" "
    "href=\"Dart/Source/Server/XSL/Build.xsl "
    "<file:///Dart/Source/Server/XSL/Build.xsl> \"");
  xml.StartElement("Site");
  xml.Attribute("BuildName", buildname);
  xml.Attribute("BuildStamp",
    this->CTest->GetCurrentTag() + "-" + this->CTest->GetTestModelString());
  xml.Attribute("Name", this->CTest->GetCTestConfiguration("Site"));
  xml.Attribute("Generator",
    std::string("ctest") + cmVersion::GetCMakeVersion());
  this->CTest->AddSiteProperties(xml);
  xml.StartElement("Upload");

  for ( it = this->Files.begin(); it != this->Files.end(); it ++ )
    {
    cmCTestOptionalLog(this->CTest, OUTPUT,
      "\tUpload file: " << *it << std::endl, this->Quiet);
    xml.StartElement("File");
    xml.Attribute("filename", *it);
    xml.StartElement("Content");
    xml.Attribute("encoding", "base64");
    xml.Content(this->CTest->Base64EncodeFile(*it));
    xml.EndElement(); // Content
    xml.EndElement(); // File
    }
  xml.EndElement(); // Upload
  xml.EndElement(); // Site
  xml.EndDocument();
  return 0;
}
