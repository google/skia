/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmStandardIncludes.h"
#include <stdio.h>
#include <stdlib.h>
#include "cmSystemTools.h"
#include "cmParseBlanketJSCoverage.h"
#include <cmsys/Directory.hxx>
#include <cmsys/Glob.hxx>
#include <cmsys/FStream.hxx>


class cmParseBlanketJSCoverage::JSONParser
  {
public:
  typedef cmCTestCoverageHandlerContainer::
      SingleFileCoverageVector FileLinesType;
  JSONParser(cmCTestCoverageHandlerContainer& cont)
      : Coverage(cont)
    {
    }

  virtual ~JSONParser()
    {
    }

  std::string getValue(std::string line, int type)
    {
    size_t begIndex;
    size_t endIndex;
    endIndex = line.rfind(',');
    begIndex = line.find_first_of(':');
    if(type == 0)
      {
      //  A unique substring to remove the extra characters
      //  around the files name in the JSON (extra " and ,)
      std::string foundFileName =
          line.substr(begIndex+3,endIndex-(begIndex+4));
      return foundFileName;
      }
    else
      {
      return line.substr(begIndex,line.npos);
      }
    }
  bool ParseFile(std::string file)
    {
    FileLinesType localCoverageVector;
    std::string filename;
    bool foundFile = false;
    bool inSource  = false;
    std::string covResult;
    std::string line;

    cmsys::ifstream in(file.c_str());
    if(!in)
      {
      return false;
      }
    while(  cmSystemTools::GetLineFromStream(in, line))
      {
      if(line.find("filename") != line.npos)
        {
        if(foundFile)
          {
          /*
          * Upon finding a second file name, generate a
          * vector within the total coverage to capture the
          * information in the local vector
          */
          FileLinesType& CoverageVector =
              this->Coverage.TotalCoverage[filename];
          CoverageVector = localCoverageVector;
          localCoverageVector.clear();
          }
        foundFile= true;
        inSource = false;
        filename = getValue(line,0).c_str();
        }
      else if((line.find("coverage") != line.npos) && foundFile && inSource )
        {
        /*
        *  two types of "coverage" in the JSON structure
        *
        *  The coverage result over the file or set of files
        *  and the coverage for each individual line
        *
        *  FoundFile and foundSource ensure that
        *  only the value of the line coverage is captured
        */
        std::string result = getValue(line,1);
        result = result.substr(2,result.npos);
        if(result == "\"\"")
          {
          // Empty quotation marks indicate that the
          // line is not executable
          localCoverageVector.push_back(-1);
          }
        else
          {
          // Else, it contains the number of time executed
          localCoverageVector.push_back(atoi(result.c_str()));
          }
        }
      else if(line.find("source") != line.npos)
        {
        inSource=true;
        }
      }

    // On exit, capture end of last file covered.
    FileLinesType& CoverageVector =
        this->Coverage.TotalCoverage[filename];
    CoverageVector = localCoverageVector;
    localCoverageVector.clear();
    return true;
    }
private:
  cmCTestCoverageHandlerContainer& Coverage;
};

cmParseBlanketJSCoverage::cmParseBlanketJSCoverage(
  cmCTestCoverageHandlerContainer& cont,  cmCTest* ctest)
  :Coverage(cont), CTest(ctest)
  {
  }

bool cmParseBlanketJSCoverage::LoadCoverageData(std::vector<std::string> files)
  {
  size_t i=0;
  std::string path;
      cmCTestOptionalLog(this->CTest,HANDLER_VERBOSE_OUTPUT,
       "Found " << files.size() <<" Files" << std::endl, this->Coverage.Quiet);
  for(i=0;i<files.size();i++)
    {
    cmCTestOptionalLog(this->CTest,HANDLER_VERBOSE_OUTPUT,
       "Reading JSON File " << files[i]  << std::endl, this->Coverage.Quiet);

    if(!this->ReadJSONFile(files[i]))
      {
      return false;
      }
    }
  return true;
  }

bool cmParseBlanketJSCoverage::ReadJSONFile(std::string file)
  {
  cmParseBlanketJSCoverage::JSONParser parser
     (this->Coverage);
  cmCTestOptionalLog(this->CTest,HANDLER_VERBOSE_OUTPUT,
       "Parsing " << file << std::endl, this->Coverage.Quiet);
  parser.ParseFile(file);
  return true;
  }
