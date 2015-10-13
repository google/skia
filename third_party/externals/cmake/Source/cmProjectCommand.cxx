/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmProjectCommand.h"

// cmProjectCommand
bool cmProjectCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1 )
    {
    this->SetError("PROJECT called with incorrect number of arguments");
    return false;
    }
  this->Makefile->SetProjectName(args[0].c_str());

  std::string bindir = args[0];
  bindir += "_BINARY_DIR";
  std::string srcdir = args[0];
  srcdir += "_SOURCE_DIR";

  this->Makefile->AddCacheDefinition
    (bindir,
     this->Makefile->GetCurrentBinaryDirectory(),
     "Value Computed by CMake", cmState::STATIC);
  this->Makefile->AddCacheDefinition
    (srcdir,
     this->Makefile->GetCurrentSourceDirectory(),
     "Value Computed by CMake", cmState::STATIC);

  bindir = "PROJECT_BINARY_DIR";
  srcdir = "PROJECT_SOURCE_DIR";

  this->Makefile->AddDefinition(bindir,
          this->Makefile->GetCurrentBinaryDirectory());
  this->Makefile->AddDefinition(srcdir,
          this->Makefile->GetCurrentSourceDirectory());

  this->Makefile->AddDefinition("PROJECT_NAME", args[0].c_str());

  // Set the CMAKE_PROJECT_NAME variable to be the highest-level
  // project name in the tree. If there are two project commands
  // in the same CMakeLists.txt file, and it is the top level
  // CMakeLists.txt file, then go with the last one, so that
  // CMAKE_PROJECT_NAME will match PROJECT_NAME, and cmake --build
  // will work.
  if(!this->Makefile->GetDefinition("CMAKE_PROJECT_NAME")
     || (this->Makefile->GetLocalGenerator()->IsRootMakefile()))
    {
    this->Makefile->AddDefinition("CMAKE_PROJECT_NAME", args[0].c_str());
    this->Makefile->AddCacheDefinition
      ("CMAKE_PROJECT_NAME",
       args[0].c_str(),
       "Value Computed by CMake", cmState::STATIC);
    }

  bool haveVersion = false;
  bool haveLanguages = false;
  std::string version;
  std::vector<std::string> languages;
  enum Doing { DoingLanguages, DoingVersion };
  Doing doing = DoingLanguages;
  for(size_t i = 1; i < args.size(); ++i)
    {
    if(args[i] == "LANGUAGES")
      {
      if(haveLanguages)
        {
        this->Makefile->IssueMessage
          (cmake::FATAL_ERROR, "LANGUAGES may be specified at most once.");
        cmSystemTools::SetFatalErrorOccured();
        return true;
        }
      haveLanguages = true;
      doing = DoingLanguages;
      }
    else if (args[i] == "VERSION")
      {
      if(haveVersion)
        {
        this->Makefile->IssueMessage
          (cmake::FATAL_ERROR, "VERSION may be specified at most once.");
        cmSystemTools::SetFatalErrorOccured();
        return true;
        }
      haveVersion = true;
      doing = DoingVersion;
      }
    else if(doing == DoingVersion)
      {
      doing = DoingLanguages;
      version = args[i];
      }
    else // doing == DoingLanguages
      {
      languages.push_back(args[i]);
      }
    }

  if (haveVersion && !haveLanguages && !languages.empty())
    {
    this->Makefile->IssueMessage
      (cmake::FATAL_ERROR,
       "project with VERSION must use LANGUAGES before language names.");
    cmSystemTools::SetFatalErrorOccured();
    return true;
    }
  if (haveLanguages && languages.empty())
    {
    languages.push_back("NONE");
    }

  cmPolicies::PolicyStatus cmp0048 =
    this->Makefile->GetPolicyStatus(cmPolicies::CMP0048);
  if (haveVersion)
    {
    // Set project VERSION variables to given values
    if (cmp0048 == cmPolicies::OLD ||
        cmp0048 == cmPolicies::WARN)
      {
      this->Makefile->IssueMessage
        (cmake::FATAL_ERROR,
         "VERSION not allowed unless CMP0048 is set to NEW");
      cmSystemTools::SetFatalErrorOccured();
      return true;
      }

    cmsys::RegularExpression
      vx("^([0-9]+(\\.[0-9]+(\\.[0-9]+(\\.[0-9]+)?)?)?)?$");
    if(!vx.find(version))
      {
      std::string e = "VERSION \"" + version + "\" format invalid.";
      this->Makefile->IssueMessage(cmake::FATAL_ERROR, e);
      cmSystemTools::SetFatalErrorOccured();
      return true;
      }

    std::string vs;
    const char* sep = "";
    char vb[4][64];
    unsigned int v[4] = {0,0,0,0};
    int vc = sscanf(version.c_str(), "%u.%u.%u.%u",
                    &v[0], &v[1], &v[2], &v[3]);
    for(int i=0; i < 4; ++i)
      {
      if(i < vc)
        {
        sprintf(vb[i], "%u", v[i]);
        vs += sep;
        vs += vb[i];
        sep = ".";
        }
      else
        {
        vb[i][0] = 0;
        }
      }

    std::string vv;
    vv = args[0] + "_VERSION";
    this->Makefile->AddDefinition("PROJECT_VERSION", vs.c_str());
    this->Makefile->AddDefinition(vv, vs.c_str());
    vv = args[0] + "_VERSION_MAJOR";
    this->Makefile->AddDefinition("PROJECT_VERSION_MAJOR", vb[0]);
    this->Makefile->AddDefinition(vv, vb[0]);
    vv = args[0] + "_VERSION_MINOR";
    this->Makefile->AddDefinition("PROJECT_VERSION_MINOR", vb[1]);
    this->Makefile->AddDefinition(vv, vb[1]);
    vv = args[0] + "_VERSION_PATCH";
    this->Makefile->AddDefinition("PROJECT_VERSION_PATCH", vb[2]);
    this->Makefile->AddDefinition(vv, vb[2]);
    vv = args[0] + "_VERSION_TWEAK";
    this->Makefile->AddDefinition("PROJECT_VERSION_TWEAK", vb[3]);
    this->Makefile->AddDefinition(vv, vb[3]);
    }
  else if(cmp0048 != cmPolicies::OLD)
    {
    // Set project VERSION variables to empty
    std::vector<std::string> vv;
    vv.push_back("PROJECT_VERSION");
    vv.push_back("PROJECT_VERSION_MAJOR");
    vv.push_back("PROJECT_VERSION_MINOR");
    vv.push_back("PROJECT_VERSION_PATCH");
    vv.push_back("PROJECT_VERSION_TWEAK");
    vv.push_back(args[0] + "_VERSION");
    vv.push_back(args[0] + "_VERSION_MAJOR");
    vv.push_back(args[0] + "_VERSION_MINOR");
    vv.push_back(args[0] + "_VERSION_PATCH");
    vv.push_back(args[0] + "_VERSION_TWEAK");
    std::string vw;
    for(std::vector<std::string>::iterator i = vv.begin();
        i != vv.end(); ++i)
      {
      const char* v = this->Makefile->GetDefinition(*i);
      if(v && *v)
        {
        if(cmp0048 == cmPolicies::WARN)
          {
          vw += "\n  ";
          vw += *i;
          }
        else
          {
          this->Makefile->AddDefinition(*i, "");
          }
        }
      }
    if(!vw.empty())
      {
      std::ostringstream w;
      w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0048)
        << "\nThe following variable(s) would be set to empty:" << vw;
      this->Makefile->IssueMessage(cmake::AUTHOR_WARNING, w.str());
      }
    }

  if (languages.empty())
    {
    // if no language is specified do c and c++
    languages.push_back("C");
    languages.push_back("CXX");
    }
  this->Makefile->EnableLanguage(languages, false);
  std::string extraInclude = "CMAKE_PROJECT_" + args[0] + "_INCLUDE";
  const char* include = this->Makefile->GetDefinition(extraInclude);
  if(include)
    {
    std::string fullFilePath;
    bool readit =
      this->Makefile->ReadDependentFile(include);
    if(!readit && !cmSystemTools::GetFatalErrorOccured())
      {
      std::string m =
        "could not find file:\n"
        "  ";
      m += include;
      this->SetError(m);
      return false;
      }
    }
  return true;
}

