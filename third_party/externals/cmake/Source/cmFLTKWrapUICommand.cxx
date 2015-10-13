/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmFLTKWrapUICommand.h"

#include "cmSourceFile.h"

// cmFLTKWrapUICommand
bool cmFLTKWrapUICommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // what is the current source dir
  std::string cdir = this->Makefile->GetCurrentSourceDirectory();
  const char* fluid_exe =
    this->Makefile->GetRequiredDefinition("FLTK_FLUID_EXECUTABLE");

  // get parameter for the command
  this->Target = args[0];  // Target that will use the generated files

  // get the list of GUI files from which .cxx and .h will be generated
  std::string outputDirectory = this->Makefile->GetCurrentBinaryDirectory();

  {
  // Some of the generated files are *.h so the directory "GUI"
  // where they are created have to be added to the include path
  std::vector<std::string> outputDirectories;
  outputDirectories.push_back(outputDirectory);
  this->Makefile->AddIncludeDirectories( outputDirectories );
  }

  for(std::vector<std::string>::const_iterator i = (args.begin() + 1);
      i != args.end(); i++)
    {
    cmSourceFile *curr = this->Makefile->GetSource(*i);
    // if we should use the source GUI
    // to generate .cxx and .h files
    if (!curr || !curr->GetPropertyAsBool("WRAP_EXCLUDE"))
      {
      std::string outName = outputDirectory;
      outName += "/";
      outName += cmSystemTools::GetFilenameWithoutExtension(*i);
      std::string hname = outName;
      hname += ".h";
      std::string origname = cdir + "/" + *i;
      // add starting depends
      std::vector<std::string> depends;
      depends.push_back(origname);
      depends.push_back(fluid_exe);
      std::string cxxres = outName;
      cxxres += ".cxx";

      cmCustomCommandLine commandLine;
      commandLine.push_back(fluid_exe);
      commandLine.push_back("-c"); // instructs Fluid to run in command line
      commandLine.push_back("-h"); // optionally rename .h files
      commandLine.push_back(hname);
      commandLine.push_back("-o"); // optionally rename .cxx files
      commandLine.push_back(cxxres);
      commandLine.push_back(origname);// name of the GUI fluid file
      cmCustomCommandLines commandLines;
      commandLines.push_back(commandLine);

      // Add command for generating the .h and .cxx files
      std::string no_main_dependency = "";
      const char* no_comment = 0;
      const char* no_working_dir = 0;
      this->Makefile->AddCustomCommandToOutput(cxxres,
                                           depends, no_main_dependency,
                                           commandLines, no_comment,
                                           no_working_dir);
      this->Makefile->AddCustomCommandToOutput(hname,
                                           depends, no_main_dependency,
                                           commandLines, no_comment,
                                           no_working_dir);

      cmSourceFile *sf = this->Makefile->GetSource(cxxres);
      sf->AddDepend(hname.c_str());
      sf->AddDepend(origname.c_str());
      this->GeneratedSourcesClasses.push_back(sf);
      }
    }

  // create the variable with the list of sources in it
  size_t lastHeadersClass = this->GeneratedSourcesClasses.size();
  std::string sourceListValue;
  for(size_t classNum = 0; classNum < lastHeadersClass; classNum++)
    {
    if (classNum)
      {
      sourceListValue += ";";
      }
    sourceListValue += this->GeneratedSourcesClasses[classNum]->GetFullPath();
    }
  std::string varName = this->Target;
  varName += "_FLTK_UI_SRCS";
  this->Makefile->AddDefinition(varName, sourceListValue.c_str());

  return true;
}

void cmFLTKWrapUICommand::FinalPass()
{
  // people should add the srcs to the target themselves, but the old command
  // didn't support that, so check and see if they added the files in and if
  // they didn;t then print a warning and add then anyhow
  cmTarget* target = this->Makefile->FindTarget(this->Target);
  if(!target)
    {
    std::string msg =
      "FLTK_WRAP_UI was called with a target that was never created: ";
    msg += this->Target;
    msg +=".  The problem was found while processing the source directory: ";
    msg += this->Makefile->GetCurrentSourceDirectory();
    msg += ".  This FLTK_WRAP_UI call will be ignored.";
    cmSystemTools::Message(msg.c_str(),"Warning");
    return;
    }
  std::vector<cmSourceFile*> srcs;
  target->GetSourceFiles(srcs, "");
  bool found = false;
  for (unsigned int i = 0; i < srcs.size(); ++i)
    {
    if (srcs[i]->GetFullPath() ==
        this->GeneratedSourcesClasses[0]->GetFullPath())
      {
      found = true;
      break;
      }
    }
  if (!found)
    {
    std::string msg =
      "In CMake 2.2 the FLTK_WRAP_UI command sets a variable to the list of "
      "source files that should be added to your executable or library. It "
      "appears that you have not added these source files to your target. "
      "You should change your CMakeLists.txt file to "
      "directly add the generated files to the target. "
      "For example FTLK_WRAP_UI(foo src1 src2 src3) "
      "will create a variable named foo_FLTK_UI_SRCS that contains the list "
      "of sources to add to your target when you call ADD_LIBRARY or "
      "ADD_EXECUTABLE. For now CMake will add the sources to your target "
      "for you as was done in CMake 2.0 and earlier. In the future this may "
      "become an error.";
    msg +="The problem was found while processing the source directory: ";
    msg += this->Makefile->GetCurrentSourceDirectory();
    cmSystemTools::Message(msg.c_str(),"Warning");
    // first we add the rules for all the .fl to .h and .cxx files
    size_t lastHeadersClass = this->GeneratedSourcesClasses.size();

    // Generate code for all the .fl files
    for(size_t classNum = 0; classNum < lastHeadersClass; classNum++)
      {
      this->Makefile->GetTargets()[this->Target]
        .AddSource(this->GeneratedSourcesClasses[classNum]->GetFullPath());
      }
    }
}



