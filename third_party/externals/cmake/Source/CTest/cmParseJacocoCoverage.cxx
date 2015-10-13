#include "cmStandardIncludes.h"
#include <stdio.h>
#include <stdlib.h>
#include "cmSystemTools.h"
#include "cmXMLParser.h"
#include "cmParseJacocoCoverage.h"
#include <cmsys/Directory.hxx>
#include <cmsys/Glob.hxx>
#include <cmsys/FStream.hxx>


class cmParseJacocoCoverage::XMLParser: public cmXMLParser
{
  public:
    XMLParser(cmCTest* ctest, cmCTestCoverageHandlerContainer& cont)
      : CTest(ctest), Coverage(cont)
      {
      this->PackageName = "";
      this->ModuleName = "";
      this->FileName = "";
      this->CurFileName = "";
      this->FilePaths.push_back(this->Coverage.SourceDir);
      }

    virtual ~XMLParser()
      {
      }

  protected:

    virtual void EndElement(const std::string&)
      {
      }

    virtual void StartElement(const std::string& name,
      const char** atts)
      {
      if(name == "package")
        {
        this->PackageName = atts[1];
        std::string FilePath = this->Coverage.SourceDir +
          "/" + this->ModuleName + "/src/main/java/" +
          this->PackageName;
        this->FilePaths.push_back(FilePath);
        FilePath = this->Coverage.SourceDir +
         "/src/main/java/" + this->PackageName;
        this->FilePaths.push_back(FilePath);
        }
      else if(name == "sourcefile")
        {
        this->FileName = atts[1];
        cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
          "Reading file: " << this->FileName << std::endl,
          this->Coverage.Quiet);
          for(size_t i=0;i < FilePaths.size();i++)
            {
            std::string finalpath = FilePaths[i] + "/" + this->FileName;
            if(cmSystemTools::FileExists(finalpath.c_str()))
              {
              this->CurFileName = finalpath;
              break;
              }
            }
          cmsys::ifstream fin(this->CurFileName.c_str());
          if(this->CurFileName == "" || !fin )
          {
            this->CurFileName = this->Coverage.BinaryDir + "/" +
                                   this->FileName;
            fin.open(this->CurFileName.c_str());
            if (!fin)
            {
              cmCTestLog(this->CTest, ERROR_MESSAGE,
                         "Jacoco Coverage: Error opening " << this->CurFileName
                         << std::endl);
              this->Coverage.Error++;
            }
          }
          std::string line;
          FileLinesType& curFileLines =
            this->Coverage.TotalCoverage[this->CurFileName];
          if(fin)
            {
            curFileLines.push_back(-1);
            }
          while(cmSystemTools::GetLineFromStream(fin, line))
          {
            curFileLines.push_back(-1);
          }
        }
      else if(name == "report")
        {
        this->ModuleName=atts[1];
        }
      else if(name == "line")
        {
        int tagCount = 0;
        int nr = -1;
        int ci = -1;
        while(true)
          {
          if(strcmp(atts[tagCount],"ci") == 0)
            {
            ci = atoi(atts[tagCount+1]);
            }
          else if (strcmp(atts[tagCount],"nr") == 0)
            {
            nr = atoi(atts[tagCount+1]);
            }
          if (ci > -1 && nr > 0)
            {
            FileLinesType& curFileLines=
              this->Coverage.TotalCoverage[this->CurFileName];
            if(!curFileLines.empty())
               {
               curFileLines[nr-1] = ci;
               }
            break;
            }
          ++tagCount;
          }
        }
      }

  private:
    std::string PackageName;
    std::string FileName;
    std::string ModuleName;
    std::string CurFileName;
    std::vector<std::string> FilePaths;
    typedef cmCTestCoverageHandlerContainer::SingleFileCoverageVector
     FileLinesType;
    cmCTest* CTest;
    cmCTestCoverageHandlerContainer& Coverage;
};

cmParseJacocoCoverage::cmParseJacocoCoverage(
  cmCTestCoverageHandlerContainer& cont,
  cmCTest* ctest)
  :Coverage(cont), CTest(ctest)
  {
  }

bool cmParseJacocoCoverage::LoadCoverageData(
  const std::vector<std::string> files)
{
  // load all the jacoco.xml files in the source directory
  cmsys::Directory dir;
  size_t i;
  std::string path;
  size_t numf = files.size();
  for (i = 0; i < numf; i++)
    {
    path = files[i];

    cmCTestOptionalLog(this->CTest,HANDLER_VERBOSE_OUTPUT,
      "Reading XML File " << path  << std::endl, this->Coverage.Quiet);
    if(cmSystemTools::GetFilenameLastExtension(path) == ".xml")
      {
      if(!this->ReadJacocoXML(path.c_str()))
        {
        return false;
        }
      }
    }
  return true;
}

bool cmParseJacocoCoverage::ReadJacocoXML(const char* file)
{
  cmParseJacocoCoverage::XMLParser
    parser(this->CTest, this->Coverage);
  parser.ParseFile(file);
  return true;
}
