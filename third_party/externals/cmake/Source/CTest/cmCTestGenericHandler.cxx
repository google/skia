/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCTestGenericHandler.h"
#include "cmSystemTools.h"

#include "cmCTest.h"

//----------------------------------------------------------------------
cmCTestGenericHandler::cmCTestGenericHandler()
{
  this->HandlerVerbose = cmSystemTools::OUTPUT_NONE;
  this->CTest = 0;
  this->SubmitIndex = 0;
  this->AppendXML = false;
  this->Quiet = false;
}

//----------------------------------------------------------------------
cmCTestGenericHandler::~cmCTestGenericHandler()
{
}

//----------------------------------------------------------------------
void cmCTestGenericHandler::SetOption(const std::string& op, const char* value)
{
  if ( !value )
    {
    cmCTestGenericHandler::t_StringToString::iterator remit
      = this->Options.find(op);
    if ( remit != this->Options.end() )
      {
      this->Options.erase(remit);
      }
    return;
    }

  this->Options[op] = value;
}

//----------------------------------------------------------------------
void cmCTestGenericHandler::SetPersistentOption(const std::string& op,
                                                const char* value)
{
  this->SetOption(op, value);
  if ( !value )
    {
    cmCTestGenericHandler::t_StringToString::iterator remit
      = this->PersistentOptions.find(op);
    if ( remit != this->PersistentOptions.end() )
      {
      this->PersistentOptions.erase(remit);
      }
    return;
    }

  this->PersistentOptions[op] = value;
}

//----------------------------------------------------------------------
void cmCTestGenericHandler::Initialize()
{
  this->AppendXML = false;
  this->Options.clear();
  t_StringToString::iterator it;
  for ( it = this->PersistentOptions.begin();
    it != this->PersistentOptions.end();
    ++ it )
    {
    this->Options[it->first] = it->second.c_str();
    }
}

//----------------------------------------------------------------------
const char* cmCTestGenericHandler::GetOption(const std::string& op)
{
  cmCTestGenericHandler::t_StringToString::iterator remit
    = this->Options.find(op);
  if ( remit == this->Options.end() )
    {
    return 0;
    }
  return remit->second.c_str();
}

//----------------------------------------------------------------------
bool cmCTestGenericHandler::StartResultingXML(cmCTest::Part part,
                                              const char* name,
                                              cmGeneratedFileStream& xofs)
{
  if ( !name )
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
      "Cannot create resulting XML file without providing the name"
      << std::endl;);
    return false;
    }
  std::ostringstream ostr;
  ostr << name;
  if ( this->SubmitIndex > 0 )
    {
    ostr << "_" << this->SubmitIndex;
    }
  ostr << ".xml";
  if(this->CTest->GetCurrentTag().empty())
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Current Tag empty, this may mean NightlyStartTime / "
               "CTEST_NIGHTLY_START_TIME was not set correctly. Or "
               "maybe you forgot to call ctest_start() before calling "
               "ctest_configure()." << std::endl);
    cmSystemTools::SetFatalErrorOccured();
    return false;
    }
  if( !this->CTest->OpenOutputFile(this->CTest->GetCurrentTag(),
      ostr.str(), xofs, true) )
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
      "Cannot create resulting XML file: " << ostr.str()
      << std::endl);
    return false;
    }
  this->CTest->AddSubmitFile(part, ostr.str().c_str());
  return true;
}

//----------------------------------------------------------------------
bool cmCTestGenericHandler::StartLogFile(const char* name,
  cmGeneratedFileStream& xofs)
{
  if ( !name )
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
      "Cannot create log file without providing the name" << std::endl;);
    return false;
    }
  std::ostringstream ostr;
  ostr << "Last" << name;
  if ( this->SubmitIndex > 0 )
    {
    ostr << "_" << this->SubmitIndex;
    }
  if ( !this->CTest->GetCurrentTag().empty() )
    {
    ostr << "_" << this->CTest->GetCurrentTag();
    }
  ostr << ".log";
  if( !this->CTest->OpenOutputFile("Temporary", ostr.str(), xofs) )
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE, "Cannot create log file: "
      << ostr.str() << std::endl);
    return false;
    }
  return true;
}
