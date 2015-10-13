/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2009 Kitware, Inc.
  Copyright 2004 Alexander Neundorf (neundorf@kde.org)

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGlobalKdevelopGenerator.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmake.h"
#include "cmSourceFile.h"
#include "cmGeneratedFileStream.h"
#include "cmSystemTools.h"

#include <cmsys/SystemTools.hxx>
#include <cmsys/Directory.hxx>
#include <cmsys/FStream.hxx>

//----------------------------------------------------------------------------
void cmGlobalKdevelopGenerator
::GetDocumentation(cmDocumentationEntry& entry, const std::string&) const
{
  entry.Name = this->GetName();
  entry.Brief = "Generates KDevelop 3 project files.";
}

cmGlobalKdevelopGenerator::cmGlobalKdevelopGenerator()
:cmExternalMakefileProjectGenerator()
{
  this->SupportedGlobalGenerators.push_back("Unix Makefiles");
#ifdef CMAKE_USE_NINJA
  this->SupportedGlobalGenerators.push_back("Ninja");
#endif
}

void cmGlobalKdevelopGenerator::Generate()
{
  // for each sub project in the project create
  // a kdevelop project
  for (std::map<std::string, std::vector<cmLocalGenerator*> >::const_iterator
       it = this->GlobalGenerator->GetProjectMap().begin();
      it!= this->GlobalGenerator->GetProjectMap().end();
      ++it)
    {
    cmMakefile* mf = it->second[0]->GetMakefile();
    std::string outputDir=mf->GetCurrentBinaryDirectory();
    std::string projectDir=mf->GetHomeDirectory();
    std::string projectName=mf->GetProjectName();
    std::string cmakeFilePattern("CMakeLists.txt;*.cmake;");
    std::string fileToOpen;
    const std::vector<cmLocalGenerator*>& lgs= it->second;
    // create the project.kdevelop.filelist file
    if(!this->CreateFilelistFile(lgs, outputDir, projectDir,
                                 projectName, cmakeFilePattern, fileToOpen))
      {
      cmSystemTools::Error("Can not create filelist file");
      return;
      }
    //try to find the name of an executable so we have something to
    //run from kdevelop for now just pick the first executable found
    std::string executable;
    for (std::vector<cmLocalGenerator*>::const_iterator lg=lgs.begin();
         lg!=lgs.end(); lg++)
      {
      cmMakefile* makefile=(*lg)->GetMakefile();
      cmTargets& targets=makefile->GetTargets();
      for (cmTargets::iterator ti = targets.begin();
           ti != targets.end(); ti++)
        {
        if (ti->second.GetType()==cmTarget::EXECUTABLE)
          {
          executable = ti->second.GetLocation("");
          break;
          }
        }
      if (!executable.empty())
        {
        break;
        }
      }

    // now create a project file
    this->CreateProjectFile(outputDir, projectDir, projectName,
                            executable, cmakeFilePattern, fileToOpen);
    }
}

bool cmGlobalKdevelopGenerator
::CreateFilelistFile(const std::vector<cmLocalGenerator*>& lgs,
                     const std::string& outputDir,
                     const std::string& projectDirIn,
                     const std::string& projectname,
                     std::string& cmakeFilePattern,
                     std::string& fileToOpen)
{
  std::string projectDir = projectDirIn + "/";
  std::string filename = outputDir+ "/" + projectname +".kdevelop.filelist";

  std::set<std::string> files;
  std::string tmp;

  for (std::vector<cmLocalGenerator*>::const_iterator it=lgs.begin();
       it!=lgs.end(); it++)
    {
    cmMakefile* makefile=(*it)->GetMakefile();
    const std::vector<std::string>& listFiles=makefile->GetListFiles();
    for (std::vector<std::string>::const_iterator lt=listFiles.begin();
         lt!=listFiles.end(); lt++)
      {
      tmp=*lt;
      cmSystemTools::ReplaceString(tmp, projectDir.c_str(), "");
      // make sure the file is part of this source tree
      if ((tmp[0]!='/') &&
          (strstr(tmp.c_str(),
                  cmake::GetCMakeFilesDirectoryPostSlash())==0))
        {
        files.insert(tmp);
        tmp=cmSystemTools::GetFilenameName(tmp);
        //add all files which dont match the default
        // */CMakeLists.txt;*cmake; to the file pattern
        if ((tmp!="CMakeLists.txt")
            && (strstr(tmp.c_str(), ".cmake")==0))
          {
          cmakeFilePattern+=tmp+";";
          }
        }
      }

    //get all sources
    cmTargets& targets=makefile->GetTargets();
    for (cmTargets::iterator ti = targets.begin();
         ti != targets.end(); ti++)
      {
      std::vector<cmSourceFile*> sources;
      ti->second.GetSourceFiles(sources, ti->second.GetMakefile()
                                    ->GetSafeDefinition("CMAKE_BUILD_TYPE"));
      for (std::vector<cmSourceFile*>::const_iterator si=sources.begin();
           si!=sources.end(); si++)
        {
        tmp=(*si)->GetFullPath();
        std::string headerBasename=cmSystemTools::GetFilenamePath(tmp);
        headerBasename+="/";
        headerBasename+=cmSystemTools::GetFilenameWithoutExtension(tmp);

        cmSystemTools::ReplaceString(tmp, projectDir.c_str(), "");

        if ((tmp[0]!='/')  &&
            (strstr(tmp.c_str(),
                  cmake::GetCMakeFilesDirectoryPostSlash())==0) &&
           (cmSystemTools::GetFilenameExtension(tmp)!=".moc"))
          {
          files.insert(tmp);

          // check if there's a matching header around
          for(std::vector<std::string>::const_iterator
                ext = makefile->GetHeaderExtensions().begin();
              ext !=  makefile->GetHeaderExtensions().end(); ++ext)
            {
            std::string hname=headerBasename;
            hname += ".";
            hname += *ext;
            if(cmSystemTools::FileExists(hname.c_str()))
              {
              cmSystemTools::ReplaceString(hname, projectDir.c_str(), "");
              files.insert(hname);
              break;
              }
            }
          }
        }
      for (std::vector<std::string>::const_iterator lt=listFiles.begin();
           lt!=listFiles.end(); lt++)
        {
        tmp=*lt;
        cmSystemTools::ReplaceString(tmp, projectDir.c_str(), "");
        if ((tmp[0]!='/') &&
            (strstr(tmp.c_str(),
                    cmake::GetCMakeFilesDirectoryPostSlash())==0))
          {
          files.insert(tmp);
          }
        }
      }
    }

  //check if the output file already exists and read it
  //insert all files which exist into the set of files
  cmsys::ifstream oldFilelist(filename.c_str());
  if (oldFilelist)
    {
    while (cmSystemTools::GetLineFromStream(oldFilelist, tmp))
      {
      if (tmp[0]=='/')
        {
        continue;
        }
      std::string completePath=projectDir+tmp;
      if (cmSystemTools::FileExists(completePath.c_str()))
        {
        files.insert(tmp);
        }
      }
    oldFilelist.close();
    }

  //now write the new filename
  cmGeneratedFileStream fout(filename.c_str());
  if(!fout)
    {
    return false;
    }

  fileToOpen="";
  for (std::set<std::string>::const_iterator it=files.begin();
       it!=files.end(); it++)
    {
    // get the full path to the file
    tmp=cmSystemTools::CollapseFullPath(*it, projectDir.c_str());
    // just select the first source file
    if (fileToOpen.empty())
    {
       std::string ext = cmSystemTools::GetFilenameExtension(tmp);
       if ((ext==".c") || (ext==".cc") || (ext==".cpp")  || (ext==".cxx")
           || (ext==".C") || (ext==".h") || (ext==".hpp"))
       {
       fileToOpen=tmp;
       }
    }
    // make it relative to the project dir
    cmSystemTools::ReplaceString(tmp, projectDir.c_str(), "");
    // only put relative paths
    if (!tmp.empty() && tmp[0] != '/')
      {
      fout << tmp.c_str() <<"\n";
      }
    }
  return true;
}


/* create the project file, if it already exists, merge it with the
existing one, otherwise create a new one */
void cmGlobalKdevelopGenerator
::CreateProjectFile(const std::string& outputDir,
                    const std::string& projectDir,
                    const std::string& projectname,
                    const std::string& executable,
                    const std::string& cmakeFilePattern,
                    const std::string& fileToOpen)
{
  this->Blacklist.clear();

  std::string filename=outputDir+"/";
  filename+=projectname+".kdevelop";
  std::string sessionFilename=outputDir+"/";
  sessionFilename+=projectname+".kdevses";

  if (cmSystemTools::FileExists(filename.c_str()))
    {
    this->MergeProjectFiles(outputDir, projectDir, filename,
                            executable, cmakeFilePattern,
                            fileToOpen, sessionFilename);
    }
  else
    {
    // add all subdirectories which are cmake build directories to the
    // kdevelop blacklist so they are not monitored for added or removed files
    // since this is handled by adding files to the cmake files
    cmsys::Directory d;
    if (d.Load(projectDir))
      {
      size_t numf = d.GetNumberOfFiles();
      for (unsigned int i = 0; i < numf; i++)
        {
        std::string nextFile = d.GetFile(i);
        if ((nextFile!=".") && (nextFile!=".."))
          {
          std::string tmp = projectDir;
          tmp += "/";
          tmp += nextFile;
          if (cmSystemTools::FileIsDirectory(tmp))
            {
            tmp += "/CMakeCache.txt";
            if ((nextFile == "CMakeFiles")
                || (cmSystemTools::FileExists(tmp.c_str())))
              {
              this->Blacklist.push_back(nextFile);
              }
            }
          }
        }
      }
    this->CreateNewProjectFile(outputDir, projectDir, filename,
                               executable, cmakeFilePattern,
                               fileToOpen, sessionFilename);
    }

}

void cmGlobalKdevelopGenerator
::MergeProjectFiles(const std::string& outputDir,
                    const std::string& projectDir,
                    const std::string& filename,
                    const std::string& executable,
                    const std::string& cmakeFilePattern,
                    const std::string& fileToOpen,
                    const std::string& sessionFilename)
{
  cmsys::ifstream oldProjectFile(filename.c_str());
  if (!oldProjectFile)
    {
    this->CreateNewProjectFile(outputDir, projectDir, filename,
                               executable, cmakeFilePattern,
                               fileToOpen, sessionFilename);
    return;
    }

  /* Read the existing project file (line by line), copy all lines
    into the new project file, except the ones which can be reliably
    set from contents of the CMakeLists.txt */
  std::string tmp;
  std::vector<std::string> lines;
  while (cmSystemTools::GetLineFromStream(oldProjectFile, tmp))
    {
    lines.push_back(tmp);
    }
  oldProjectFile.close();

  cmGeneratedFileStream fout(filename.c_str());
  if(!fout)
    {
    return;
    }

  for (std::vector<std::string>::const_iterator it=lines.begin();
       it!=lines.end(); it++)
    {
    const char* line=(*it).c_str();
    // skip these tags as they are always replaced
    if ((strstr(line, "<projectdirectory>")!=0)
        || (strstr(line, "<projectmanagement>")!=0)
        || (strstr(line, "<absoluteprojectpath>")!=0)
        || (strstr(line, "<filelistdirectory>")!=0)
        || (strstr(line, "<buildtool>")!=0)
        || (strstr(line, "<builddir>")!=0))
      {
      continue;
      }

    // output the line from the file if it is not one of the above tags
    fout<<*it<<"\n";
    // if this is the <general> tag output the stuff that goes in the
    // general tag
    if (strstr(line, "<general>"))
      {
      fout<< "  <projectmanagement>KDevCustomProject</projectmanagement>\n";
      fout<< "  <projectdirectory>" <<projectDir
          << "</projectdirectory>\n";   //this one is important
      fout<<"  <absoluteprojectpath>true</absoluteprojectpath>\n";
      //and this one
      }
    // inside kdevcustomproject the <filelistdirectory> must be put
    if (strstr(line, "<kdevcustomproject>"))
      {
      fout<<"    <filelistdirectory>"<<outputDir
          <<"</filelistdirectory>\n";
      }
    // buildtool and builddir go inside <build>
    if (strstr(line, "<build>"))
      {
      fout<<"      <buildtool>make</buildtool>\n";
      fout<<"      <builddir>"<<outputDir<<"</builddir>\n";
      }
    }
}

void cmGlobalKdevelopGenerator
::CreateNewProjectFile(const std::string& outputDir,
                       const std::string& projectDir,
                       const std::string& filename,
                       const std::string& executable,
                       const std::string& cmakeFilePattern,
                       const std::string& fileToOpen,
                       const std::string& sessionFilename)
{
  cmGeneratedFileStream fout(filename.c_str());
  if(!fout)
    {
    return;
    }

  // check for a version control system
  bool hasSvn = cmSystemTools::FileExists((projectDir + "/.svn").c_str());
  bool hasCvs = cmSystemTools::FileExists((projectDir + "/CVS").c_str());

  bool enableCxx = (this->GlobalGenerator->GetLanguageEnabled("C")
                          || this->GlobalGenerator->GetLanguageEnabled("CXX"));
  bool enableFortran = this->GlobalGenerator->GetLanguageEnabled("Fortran");
  std::string primaryLanguage = "C++";
  if (enableFortran && !enableCxx)
    {
    primaryLanguage="Fortran77";
    }

  fout<<"<?xml version = '1.0'?>\n"
        "<kdevelop>\n"
        "  <general>\n"
        "  <author></author>\n"
        "  <email></email>\n"
        "  <version>$VERSION$</version>\n"
        "  <projectmanagement>KDevCustomProject</projectmanagement>\n"
        "  <primarylanguage>" << primaryLanguage << "</primarylanguage>\n"
        "  <ignoreparts/>\n"
        "  <projectdirectory>" << projectDir <<
        "</projectdirectory>\n";   //this one is important
  fout<<"  <absoluteprojectpath>true</absoluteprojectpath>\n"; //and this one

  // setup additional languages
  fout<<"  <secondaryLanguages>\n";
  if (enableFortran && enableCxx)
    {
    fout<<"     <language>Fortran</language>\n";
    }
  if (enableCxx)
    {
    fout<<"     <language>C</language>\n";
    }
  fout<<"  </secondaryLanguages>\n";

  if (hasSvn)
    {
    fout << "  <versioncontrol>kdevsubversion</versioncontrol>\n";
    }
  else if (hasCvs)
    {
    fout << "  <versioncontrol>kdevcvsservice</versioncontrol>\n";
    }

  fout<<"  </general>\n"
        "  <kdevcustomproject>\n"
        "    <filelistdirectory>" << outputDir <<
        "</filelistdirectory>\n"
        "    <run>\n"
        "      <mainprogram>" << executable << "</mainprogram>\n"
        "      <directoryradio>custom</directoryradio>\n"
        "      <customdirectory>"<<outputDir<<"</customdirectory>\n"
        "      <programargs></programargs>\n"
        "      <terminal>false</terminal>\n"
        "      <autocompile>true</autocompile>\n"
        "      <envvars/>\n"
        "    </run>\n"
        "    <build>\n"
        "      <buildtool>make</buildtool>\n"; //this one is important
  fout<<"      <builddir>"<<outputDir<<"</builddir>\n";  //and this one
  fout<<"    </build>\n"
        "    <make>\n"
        "      <abortonerror>false</abortonerror>\n"
        "      <numberofjobs>1</numberofjobs>\n"
        "      <dontact>false</dontact>\n"
        "      <makebin>" << this->GlobalGenerator->GetLocalGenerators()[0]->
            GetMakefile()->GetRequiredDefinition("CMAKE_MAKE_PROGRAM")
            << " </makebin>\n"
        "      <selectedenvironment>default</selectedenvironment>\n"
        "      <environments>\n"
        "        <default>\n"
        "          <envvar value=\"1\" name=\"VERBOSE\" />\n"
        "          <envvar value=\"1\" name=\"CMAKE_NO_VERBOSE\" />\n"
        "        </default>\n"
        "      </environments>\n"
        "    </make>\n";

  fout<<"    <blacklist>\n";
  for(std::vector<std::string>::const_iterator dirIt=this->Blacklist.begin();
      dirIt != this->Blacklist.end();
      ++dirIt)
    {
    fout<<"      <path>" << *dirIt << "</path>\n";
    }
  fout<<"    </blacklist>\n";

  fout<<"  </kdevcustomproject>\n"
        "  <kdevfilecreate>\n"
        "    <filetypes/>\n"
        "    <useglobaltypes>\n"
        "      <type ext=\"ui\" />\n"
        "      <type ext=\"cpp\" />\n"
        "      <type ext=\"h\" />\n"
        "    </useglobaltypes>\n"
        "  </kdevfilecreate>\n"
        "  <kdevdoctreeview>\n"
        "    <projectdoc>\n"
        "      <userdocDir>html/</userdocDir>\n"
        "      <apidocDir>html/</apidocDir>\n"
        "    </projectdoc>\n"
        "    <ignoreqt_xml/>\n"
        "    <ignoredoxygen/>\n"
        "    <ignorekdocs/>\n"
        "    <ignoretocs/>\n"
        "    <ignoredevhelp/>\n"
        "  </kdevdoctreeview>\n";

  if (enableCxx)
    {
    fout<<"  <cppsupportpart>\n"
          "    <filetemplates>\n"
          "      <interfacesuffix>.h</interfacesuffix>\n"
          "      <implementationsuffix>.cpp</implementationsuffix>\n"
          "    </filetemplates>\n"
          "  </cppsupportpart>\n"
          "  <kdevcppsupport>\n"
          "    <codecompletion>\n"
          "      <includeGlobalFunctions>true</includeGlobalFunctions>\n"
          "      <includeTypes>true</includeTypes>\n"
          "      <includeEnums>true</includeEnums>\n"
          "      <includeTypedefs>false</includeTypedefs>\n"
          "      <automaticCodeCompletion>true</automaticCodeCompletion>\n"
          "      <automaticArgumentsHint>true</automaticArgumentsHint>\n"
          "      <automaticHeaderCompletion>true</automaticHeaderCompletion>\n"
          "      <codeCompletionDelay>250</codeCompletionDelay>\n"
          "      <argumentsHintDelay>400</argumentsHintDelay>\n"
          "      <headerCompletionDelay>250</headerCompletionDelay>\n"
          "    </codecompletion>\n"
          "    <references/>\n"
          "  </kdevcppsupport>\n";
    }

  if (enableFortran)
    {
    fout<<"  <kdevfortransupport>\n"
          "    <ftnchek>\n"
          "      <division>false</division>\n"
          "      <extern>false</extern>\n"
          "      <declare>false</declare>\n"
          "      <pure>false</pure>\n"
          "      <argumentsall>false</argumentsall>\n"
          "      <commonall>false</commonall>\n"
          "      <truncationall>false</truncationall>\n"
          "      <usageall>false</usageall>\n"
          "      <f77all>false</f77all>\n"
          "      <portabilityall>false</portabilityall>\n"
          "      <argumentsonly/>\n"
          "      <commononly/>\n"
          "      <truncationonly/>\n"
          "      <usageonly/>\n"
          "      <f77only/>\n"
          "      <portabilityonly/>\n"
          "    </ftnchek>\n"
          "  </kdevfortransupport>\n";
    }

  // set up file groups. maybe this can be used with the CMake SOURCE_GROUP()
  // command
  fout<<"  <kdevfileview>\n"
        "    <groups>\n"
        "      <group pattern=\"" << cmakeFilePattern <<
        "\" name=\"CMake\" />\n";

  if (enableCxx)
    {
    fout<<"      <group pattern=\"*.h;*.hxx;*.hpp\" name=\"Header\" />\n"
          "      <group pattern=\"*.c\" name=\"C Sources\" />\n"
          "      <group pattern=\"*.cpp;*.C;*.cxx;*.cc\" name=\"C++ Sources\""
          "/>\n";
    }

  if (enableFortran)
    {
    fout<<"      <group pattern=\"*.f;*.F;*.f77;*.F77;*.f90;*.F90;*.for;*.f95;"
          "*.F95\" name=\"Fortran Sources\" />\n";
    }

  fout<<"      <group pattern=\"*.ui\" name=\"Qt Designer files\" />\n"
        "      <hidenonprojectfiles>true</hidenonprojectfiles>\n"
        "    </groups>\n"
        "    <tree>\n"
        "      <hidepatterns>*.o,*.lo,CVS,*~,cmake*</hidepatterns>\n"
        "      <hidenonprojectfiles>true</hidenonprojectfiles>\n"
        "    </tree>\n"
        "  </kdevfileview>\n"
        "</kdevelop>\n";

  if (sessionFilename.empty())
    {
    return;
    }

  // and a session file, so that kdevelop opens a file if it opens the
  // project the first time
  cmGeneratedFileStream devses(sessionFilename.c_str());
  if(!devses)
    {
    return;
    }
  devses<<"<?xml version = '1.0' encoding = \'UTF-8\'?>\n"
          "<!DOCTYPE KDevPrjSession>\n"
          "<KDevPrjSession>\n"
          " <DocsAndViews NumberOfDocuments=\"1\" >\n"
          "  <Doc0 NumberOfViews=\"1\" URL=\"file://" << fileToOpen <<
          "\" >\n"
          "   <View0 line=\"0\" Type=\"Source\" />\n"
          "  </Doc0>\n"
          " </DocsAndViews>\n"
          "</KDevPrjSession>\n";
}

