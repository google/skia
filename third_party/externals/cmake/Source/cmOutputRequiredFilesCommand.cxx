/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmOutputRequiredFilesCommand.h"
#include "cmMakeDepend.h"
#include "cmAlgorithms.h"
#include <cmsys/FStream.hxx>

class cmLBDepend : public cmMakeDepend
{
  /**
   * Compute the depend information for this class.
   */
  virtual void DependWalk(cmDependInformation* info);
};

void cmLBDepend::DependWalk(cmDependInformation* info)
{
  cmsys::ifstream fin(info->FullPath.c_str());
  if(!fin)
    {
    cmSystemTools::Error("error can not open ", info->FullPath.c_str());
    return;
    }

  std::string line;
  while(cmSystemTools::GetLineFromStream(fin, line))
    {
    if(cmHasLiteralPrefix(line.c_str(), "#include"))
      {
      // if it is an include line then create a string class
      std::string currentline = line;
      size_t qstart = currentline.find('\"', 8);
      size_t qend;
      // if a quote is not found look for a <
      if(qstart == std::string::npos)
        {
        qstart = currentline.find('<', 8);
        // if a < is not found then move on
        if(qstart == std::string::npos)
          {
          cmSystemTools::Error("unknown include directive ",
                               currentline.c_str() );
          continue;
          }
        else
          {
          qend = currentline.find('>', qstart+1);
          }
        }
      else
        {
        qend = currentline.find('\"', qstart+1);
        }
      // extract the file being included
      std::string includeFile = currentline.substr(qstart+1, qend - qstart-1);
      // see if the include matches the regular expression
      if(!this->IncludeFileRegularExpression.find(includeFile))
        {
        if(this->Verbose)
          {
          std::string message = "Skipping ";
          message += includeFile;
          message += " for file ";
          message += info->FullPath.c_str();
          cmSystemTools::Error(message.c_str(), 0);
          }
        continue;
        }

      // Add this file and all its dependencies.
      this->AddDependency(info, includeFile.c_str());
      /// add the cxx file if it exists
      std::string cxxFile = includeFile;
      std::string::size_type pos = cxxFile.rfind('.');
      if(pos != std::string::npos)
        {
        std::string root = cxxFile.substr(0, pos);
        cxxFile = root + ".cxx";
        bool found = false;
        // try jumping to .cxx .cpp and .c in order
        if(cmSystemTools::FileExists(cxxFile.c_str()))
          {
          found = true;
          }
        for(std::vector<std::string>::iterator i =
              this->IncludeDirectories.begin();
            i != this->IncludeDirectories.end(); ++i)
          {
          std::string path = *i;
          path = path + "/";
          path = path + cxxFile;
          if(cmSystemTools::FileExists(path.c_str()))
            {
            found = true;
            }
          }
        if (!found)
          {
          cxxFile = root + ".cpp";
          if(cmSystemTools::FileExists(cxxFile.c_str()))
            {
            found = true;
            }
          for(std::vector<std::string>::iterator i =
                this->IncludeDirectories.begin();
              i != this->IncludeDirectories.end(); ++i)
            {
            std::string path = *i;
            path = path + "/";
            path = path + cxxFile;
            if(cmSystemTools::FileExists(path.c_str()))
              {
              found = true;
              }
            }
          }
        if (!found)
          {
          cxxFile = root + ".c";
          if(cmSystemTools::FileExists(cxxFile.c_str()))
            {
            found = true;
            }
          for(std::vector<std::string>::iterator i =
                this->IncludeDirectories.begin();
              i != this->IncludeDirectories.end(); ++i)
            {
            std::string path = *i;
            path = path + "/";
            path = path + cxxFile;
            if(cmSystemTools::FileExists(path.c_str()))
              {
              found = true;
              }
            }
          }
        if (!found)
          {
          cxxFile = root + ".txx";
          if(cmSystemTools::FileExists(cxxFile.c_str()))
            {
            found = true;
            }
          for(std::vector<std::string>::iterator i =
                this->IncludeDirectories.begin();
              i != this->IncludeDirectories.end(); ++i)
            {
            std::string path = *i;
            path = path + "/";
            path = path + cxxFile;
            if(cmSystemTools::FileExists(path.c_str()))
              {
              found = true;
              }
            }
          }
        if (found)
          {
          this->AddDependency(info, cxxFile.c_str());
          }
        }
      }
    }
}

// cmOutputRequiredFilesCommand
bool cmOutputRequiredFilesCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(this->Disallowed(cmPolicies::CMP0032,
      "The output_required_files command should not be called; see CMP0032."))
    { return true; }
  if(args.size() != 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // store the arg for final pass
  this->File = args[0];
  this->OutputFile = args[1];

  // compute the list of files
  cmLBDepend md;
  md.SetMakefile(this->Makefile);
  md.AddSearchPath(this->Makefile->GetCurrentSourceDirectory());
  // find the depends for a file
  const cmDependInformation *info = md.FindDependencies(this->File.c_str());
  if (info)
    {
    // write them out
    FILE *fout = cmsys::SystemTools::Fopen(this->OutputFile.c_str(),"w");
    if(!fout)
      {
      std::string err = "Can not open output file: ";
      err += this->OutputFile;
      this->SetError(err);
      return false;
      }
    std::set<cmDependInformation const*> visited;
    this->ListDependencies(info,fout, &visited);
    fclose(fout);
    }

  return true;
}

void cmOutputRequiredFilesCommand::
ListDependencies(cmDependInformation const *info,
                 FILE *fout,
                 std::set<cmDependInformation const*> *visited)
{
  // add info to the visited set
  visited->insert(info);
  // now recurse with info's dependencies
  for(cmDependInformation::DependencySetType::const_iterator d =
        info->DependencySet.begin();
      d != info->DependencySet.end(); ++d)
    {
    if (visited->find(*d) == visited->end())
      {
      if(info->FullPath != "")
        {
        std::string tmp = (*d)->FullPath;
        std::string::size_type pos = tmp.rfind('.');
        if(pos != std::string::npos && (tmp.substr(pos) != ".h"))
          {
          tmp = tmp.substr(0, pos);
          fprintf(fout,"%s\n",(*d)->FullPath.c_str());
          }
        }
      this->ListDependencies(*d,fout,visited);
      }
    }
}

