/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmcmd.h"
#include "cmMakefile.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmQtAutoGenerators.h"
#include "cmVersion.h"
#include "cmAlgorithms.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
# include "cmDependsFortran.h" // For -E cmake_copy_f90_mod callback.
#endif

#include <cmsys/Directory.hxx>
#include <cmsys/Process.h>
#include <cmsys/FStream.hxx>
#include <cmsys/Terminal.h>

#if defined(CMAKE_HAVE_VS_GENERATORS)
#include "cmCallVisualStudioMacro.h"
#include "cmVisualStudioWCEPlatformParser.h"
#endif

#include <time.h>

#include <stdlib.h> // required for atoi

void CMakeCommandUsage(const char* program)
{
  std::ostringstream errorStream;

#ifdef CMAKE_BUILD_WITH_CMAKE
  errorStream
    << "cmake version " << cmVersion::GetCMakeVersion() << "\n";
#else
  errorStream
    << "cmake bootstrap\n";
#endif
  // If you add new commands, change here,
  // and in cmakemain.cxx in the options table
  errorStream
    << "Usage: " << program << " -E [command] [arguments ...]\n"
    << "Available commands: \n"
    << "  chdir dir cmd [args]...   - run command in a given directory\n"
    << "  compare_files file1 file2 - check if file1 is same as file2\n"
    << "  copy file destination     - copy file to destination (either file "
       "or directory)\n"
    << "  copy_directory source destination   - copy directory 'source' "
       "content to directory 'destination'\n"
    << "  copy_if_different in-file out-file  - copy file if input has "
       "changed\n"
    << "  echo [string]...          - displays arguments as text\n"
    << "  echo_append [string]...   - displays arguments as text but no new "
       "line\n"
    << "  env [--unset=NAME]... [NAME=VALUE]... COMMAND [ARG]...\n"
    << "                            - run command in a modified environment\n"
    << "  environment               - display the current environment\n"
    << "  make_directory dir        - create a directory\n"
    << "  md5sum file1 [...]        - compute md5sum of files\n"
    << "  remove [-f] file1 file2 ... - remove the file(s), use -f to force "
       "it\n"
    << "  remove_directory dir      - remove a directory and its contents\n"
    << "  rename oldname newname    - rename a file or directory "
       "(on one volume)\n"
    << "  tar [cxt][vf][zjJ] file.tar [file/dir1 file/dir2 ...]\n"
    << "                            - create or extract a tar or zip archive\n"
    << "  sleep <number>...         - sleep for given number of seconds\n"
    << "  time command [args] ...   - run command and return elapsed time\n"
    << "  touch file                - touch a file.\n"
    << "  touch_nocreate file       - touch a file but do not create it.\n"
#if defined(_WIN32) && !defined(__CYGWIN__)
    << "Available on Windows only:\n"
    << "  delete_regv key           - delete registry value\n"
    << "  env_vs8_wince sdkname     - displays a batch file which sets the "
       "environment for the provided Windows CE SDK installed in VS2005\n"
    << "  env_vs9_wince sdkname     - displays a batch file which sets the "
       "environment for the provided Windows CE SDK installed in VS2008\n"
    << "  write_regv key value      - write registry value\n"
#else
    << "Available on UNIX only:\n"
    << "  create_symlink old new    - create a symbolic link new -> old\n"
#endif
    ;

  cmSystemTools::Error(errorStream.str().c_str());
}

static bool cmTarFilesFrom(std::string const& file,
                           std::vector<std::string>& files)
{
  if (cmSystemTools::FileIsDirectory(file))
    {
    std::ostringstream e;
    e << "-E tar --files-from= file '" << file << "' is a directory";
    cmSystemTools::Error(e.str().c_str());
    return false;
    }
  cmsys::ifstream fin(file.c_str());
  if (!fin)
    {
    std::ostringstream e;
    e << "-E tar --files-from= file '" << file << "' not found";
    cmSystemTools::Error(e.str().c_str());
    return false;
    }
  std::string line;
  while (cmSystemTools::GetLineFromStream(fin, line))
    {
    if (line.empty())
      {
      continue;
      }
    if (cmHasLiteralPrefix(line, "--add-file="))
      {
      files.push_back(line.substr(11));
      }
    else if (cmHasLiteralPrefix(line, "-"))
      {
      std::ostringstream e;
      e << "-E tar --files-from='" << file << "' file invalid line:\n"
        << line << "\n";
      cmSystemTools::Error(e.str().c_str());
      return false;
      }
    else
      {
      files.push_back(line);
      }
    }
  return true;
}

int cmcmd::ExecuteCMakeCommand(std::vector<std::string>& args)
{
  // IF YOU ADD A NEW COMMAND, DOCUMENT IT ABOVE and in cmakemain.cxx
  if (args.size() > 1)
    {
    // Copy file
    if (args[1] == "copy" && args.size() == 4)
      {
      if(!cmSystemTools::cmCopyFile(args[2].c_str(), args[3].c_str()))
        {
        std::cerr << "Error copying file \"" << args[2]
                  << "\" to \"" << args[3] << "\".\n";
        return 1;
        }
      return 0;
      }

    // Copy file if different.
    if (args[1] == "copy_if_different" && args.size() == 4)
      {
      if(!cmSystemTools::CopyFileIfDifferent(args[2].c_str(),
          args[3].c_str()))
        {
        std::cerr << "Error copying file (if different) from \""
                  << args[2] << "\" to \"" << args[3]
                  << "\".\n";
        return 1;
        }
      return 0;
      }

    // Copy directory content
    if (args[1] == "copy_directory" && args.size() == 4)
      {
      if(!cmSystemTools::CopyADirectory(args[2], args[3]))
        {
        std::cerr << "Error copying directory from \""
                  << args[2] << "\" to \"" << args[3]
                  << "\".\n";
        return 1;
        }
      return 0;
      }

    // Rename a file or directory
    if (args[1] == "rename" && args.size() == 4)
      {
      if(!cmSystemTools::RenameFile(args[2].c_str(), args[3].c_str()))
        {
        std::string e = cmSystemTools::GetLastSystemError();
        std::cerr << "Error renaming from \""
                  << args[2] << "\" to \"" << args[3]
                  << "\": " << e << "\n";
        return 1;
        }
      return 0;
      }

    // Compare files
    if (args[1] == "compare_files" && args.size() == 4)
      {
      if(cmSystemTools::FilesDiffer(args[2], args[3]))
        {
        std::cerr << "Files \""
                  << args[2] << "\" to \"" << args[3]
                  << "\" are different.\n";
        return 1;
        }
      return 0;
      }

    // run include what you use command and then run the compile
    // command. This is an internal undocumented option and should
    // only be used by CMake itself when running iwyu.
    else if (args[1] == "__run_iwyu")
      {
      if (args.size() < 3)
        {
        std::cerr << "__run_iwyu Usage: -E __run_iwyu [--iwyu=/path/iwyu]"
          " -- compile command\n";
        return 1;
        }
      bool doing_options = true;
      std::vector<std::string> orig_cmd;
      std::string iwyu;
      for (std::string::size_type cc = 2; cc < args.size(); cc ++)
        {
        std::string const& arg = args[cc];
        if (arg == "--")
          {
          doing_options = false;
          }
        else if (doing_options && cmHasLiteralPrefix(arg, "--iwyu="))
          {
          iwyu = arg.substr(7);
          }
        else if (doing_options)
          {
          std::cerr << "__run_iwyu given unknown argument: " << arg << "\n";
          return 1;
          }
        else
          {
          orig_cmd.push_back(arg);
          }
        }
      if (iwyu.empty())
        {
        std::cerr << "__run_iwyu missing --iwyu=\n";
        return 1;
        }
      if (orig_cmd.empty())
        {
        std::cerr << "__run_iwyu missing compile command after --\n";
        return 1;
        }

      // Construct the iwyu command line by taking what was given
      // and adding all the arguments we give to the compiler.
      std::vector<std::string> iwyu_cmd;
      cmSystemTools::ExpandListArgument(iwyu, iwyu_cmd, true);
      iwyu_cmd.insert(iwyu_cmd.end(), orig_cmd.begin()+1, orig_cmd.end());

      // Run the iwyu command line.  Capture its stderr and hide its stdout.
      int ret = 0;
      std::string stdErr;
      if(!cmSystemTools::RunSingleCommand(iwyu_cmd, 0, &stdErr, &ret,
                                          0, cmSystemTools::OUTPUT_NONE))
        {
        std::cerr << "Error running '" << iwyu_cmd[0] << "': "
                  << stdErr << "\n";
        return 1;
        }

      // Warn if iwyu reported anything.
      if(stdErr.find("should remove these lines:") != stdErr.npos
         || stdErr.find("should add these lines:") != stdErr.npos)
        {
        std::cerr << "Warning: include-what-you-use reported diagnostics:\n"
                  << stdErr << "\n";
        }

      // Now run the real compiler command and return its result value.
      if(!cmSystemTools::RunSingleCommand(orig_cmd, 0, &stdErr, &ret, 0,
                                          cmSystemTools::OUTPUT_PASSTHROUGH))
        {
        std::cerr << "Error running '" << orig_cmd[0] << "': "
                  << stdErr << "\n";
        return 1;
        }
      return ret;
      }

    // Echo string
    else if (args[1] == "echo" )
      {
      std::cout << cmJoin(cmRange(args).advance(2), " ") << std::endl;
      return 0;
      }

    // Echo string no new line
    else if (args[1] == "echo_append" )
      {
      std::cout << cmJoin(cmRange(args).advance(2), " ");
      return 0;
      }

    else if (args[1] == "env" )
      {
      std::vector<std::string>::const_iterator ai = args.begin() + 2;
      std::vector<std::string>::const_iterator ae = args.end();
      for(; ai != ae; ++ai)
        {
        std::string const& a = *ai;
        if(cmHasLiteralPrefix(a, "--unset="))
          {
          // Unset environment variable.
          cmSystemTools::UnPutEnv(a.c_str() + 8);
          }
        else if(!a.empty() && a[0] == '-')
          {
          // Environment variable and command names cannot start in '-',
          // so this must be an unknown option.
          std::cerr << "cmake -E env: unknown option '" << a << "'"
                    << std::endl;
          return 1;
          }
        else if(a.find("=") != a.npos)
          {
          // Set environment variable.
          cmSystemTools::PutEnv(a);
          }
        else
          {
          // This is the beginning of the command.
          break;
          }
        }

      if(ai == ae)
        {
        std::cerr << "cmake -E env: no command given" << std::endl;
        return 1;
        }

      // Execute command from remaining arguments.
      std::vector<std::string> cmd(ai, ae);
      int retval;
      if(cmSystemTools::RunSingleCommand(
           cmd, 0, 0, &retval, NULL, cmSystemTools::OUTPUT_PASSTHROUGH))
        {
        return retval;
        }
      return 1;
      }

#if defined(CMAKE_BUILD_WITH_CMAKE)
    else if (args[1] == "environment" )
      {
      std::vector<std::string> env = cmSystemTools::GetEnvironmentVariables();
      std::vector<std::string>::iterator it;
      for ( it = env.begin(); it != env.end(); ++ it )
        {
        std::cout << *it << std::endl;
        }
      return 0;
      }
#endif

    else if (args[1] == "make_directory" && args.size() == 3)
      {
      if(!cmSystemTools::MakeDirectory(args[2].c_str()))
        {
        std::cerr << "Error making directory \"" << args[2]
                  << "\".\n";
        return 1;
        }
      return 0;
      }

    else if (args[1] == "remove_directory" && args.size() == 3)
      {
      if(cmSystemTools::FileIsDirectory(args[2]) &&
         !cmSystemTools::RemoveADirectory(args[2]))
        {
        std::cerr << "Error removing directory \"" << args[2]
                  << "\".\n";
        return 1;
        }
      return 0;
      }

    // Remove file
    else if (args[1] == "remove" && args.size() > 2)
      {
      bool force = false;
      for (std::string::size_type cc = 2; cc < args.size(); cc ++)
        {
        if(args[cc] == "\\-f" || args[cc] == "-f")
          {
          force = true;
          }
        else
          {
          // Complain if the file could not be removed, still exists,
          // and the -f option was not given.
          if(!cmSystemTools::RemoveFile(args[cc]) && !force &&
             cmSystemTools::FileExists(args[cc].c_str()))
            {
            return 1;
            }
          }
        }
      return 0;
      }
    // Touch file
    else if (args[1] == "touch" && args.size() > 2)
      {
      for (std::string::size_type cc = 2; cc < args.size(); cc ++)
        {
        if(!cmSystemTools::Touch(args[cc], true))
          {
          return 1;
          }
        }
      return 0;
      }
    // Touch file
    else if (args[1] == "touch_nocreate" && args.size() > 2)
      {
      for (std::string::size_type cc = 2; cc < args.size(); cc ++)
        {
        // Complain if the file could not be removed, still exists,
        // and the -f option was not given.
        if(!cmSystemTools::Touch(args[cc], false))
          {
          return 1;
          }
        }
      return 0;
      }

    // Sleep command
    else if (args[1] == "sleep" && args.size() > 2)
      {
      double total = 0;
      for(size_t i = 2; i < args.size(); ++i)
        {
        double num = 0.0;
        char unit;
        char extra;
        int n = sscanf(args[i].c_str(), "%lg%c%c", &num, &unit, &extra);
        if((n == 1 || (n == 2 && unit == 's')) && num >= 0)
          {
          total += num;
          }
        else
          {
          std::cerr << "Unknown sleep time format \"" << args[i] << "\".\n";
          return 1;
          }
        }
      if(total > 0)
        {
        cmSystemTools::Delay(static_cast<unsigned int>(total*1000));
        }
      return 0;
      }

    // Clock command
    else if (args[1] == "time" && args.size() > 2)
      {
      std::string command = cmJoin(cmRange(args).advance(2), " ");

      clock_t clock_start, clock_finish;
      time_t time_start, time_finish;

      time(&time_start);
      clock_start = clock();
      int ret =0;
      cmSystemTools::RunSingleCommand(command.c_str(), 0, 0, &ret);

      clock_finish = clock();
      time(&time_finish);

      double clocks_per_sec = static_cast<double>(CLOCKS_PER_SEC);
      std::cout << "Elapsed time: "
        << static_cast<long>(time_finish - time_start) << " s. (time)"
        << ", "
        << static_cast<double>(clock_finish - clock_start) / clocks_per_sec
        << " s. (clock)"
        << "\n";
      return ret;
      }
    // Command to calculate the md5sum of a file
    else if (args[1] == "md5sum" && args.size() >= 3)
      {
      char md5out[32];
      int retval = 0;
      for (std::string::size_type cc = 2; cc < args.size(); cc ++)
        {
        const char *filename = args[cc].c_str();
        // Cannot compute md5sum of a directory
        if(cmSystemTools::FileIsDirectory(filename))
          {
          std::cerr << "Error: " << filename << " is a directory" << std::endl;
          retval++;
          }
        else if(!cmSystemTools::ComputeFileMD5(filename, md5out))
          {
          // To mimic md5sum behavior in a shell:
          std::cerr << filename << ": No such file or directory" << std::endl;
          retval++;
          }
        else
          {
          std::cout << std::string(md5out,32) << "  " << filename << std::endl;
          }
        }
      return retval;
      }

    // Command to change directory and run a program.
    else if (args[1] == "chdir" && args.size() >= 4)
      {
      std::string directory = args[2];
      if(!cmSystemTools::FileExists(directory.c_str()))
        {
        cmSystemTools::Error("Directory does not exist for chdir command: ",
                             args[2].c_str());
        return 1;
        }

      std::string command = cmWrap('"', cmRange(args).advance(3), '"', " ");
      int retval = 0;
      int timeout = 0;
      if ( cmSystemTools::RunSingleCommand(command.c_str(), 0, 0, &retval,
             directory.c_str(), cmSystemTools::OUTPUT_NORMAL, timeout) )
        {
        return retval;
        }

      return 1;
      }

    // Command to start progress for a build
    else if (args[1] == "cmake_progress_start" && args.size() == 4)
      {
      // basically remove the directory
      std::string dirName = args[2];
      dirName += "/Progress";
      cmSystemTools::RemoveADirectory(dirName);

      // is the last argument a filename that exists?
      FILE *countFile = cmsys::SystemTools::Fopen(args[3],"r");
      int count;
      if (countFile)
        {
        if (1!=fscanf(countFile,"%i",&count))
          {
          cmSystemTools::Message("Could not read from count file.");
          }
        fclose(countFile);
        }
      else
        {
        count = atoi(args[3].c_str());
        }
      if (count)
        {
        cmSystemTools::MakeDirectory(dirName.c_str());
        // write the count into the directory
        std::string fName = dirName;
        fName += "/count.txt";
        FILE *progFile = cmsys::SystemTools::Fopen(fName,"w");
        if (progFile)
          {
          fprintf(progFile,"%i\n",count);
          fclose(progFile);
          }
        }
      return 0;
      }

    // Command to report progress for a build
    else if (args[1] == "cmake_progress_report" && args.size() >= 3)
      {
      // This has been superseded by cmake_echo_color --progress-*
      // options.  We leave it here to avoid errors if somehow this
      // is invoked by an existing makefile without regenerating.
      return 0;
      }

    // Command to create a symbolic link.  Fails on platforms not
    // supporting them.
    else if (args[1] == "create_symlink" && args.size() == 4)
      {
      const char* destinationFileName = args[3].c_str();
      if((cmSystemTools::FileExists(destinationFileName) ||
          cmSystemTools::FileIsSymlink(destinationFileName)) &&
         !cmSystemTools::RemoveFile(destinationFileName))
        {
        std::string emsg = cmSystemTools::GetLastSystemError();
        std::cerr <<
          "failed to create symbolic link '" << destinationFileName <<
          "' because existing path cannot be removed: " << emsg << "\n";
        return 1;
        }
      if(!cmSystemTools::CreateSymlink(args[2], args[3]))
        {
        std::string emsg = cmSystemTools::GetLastSystemError();
        std::cerr <<
          "failed to create symbolic link '" << destinationFileName <<
          "': " << emsg << "\n";
        return 1;
        }
      return 0;
      }

    // Internal CMake shared library support.
    else if (args[1] == "cmake_symlink_library" && args.size() == 5)
      {
      return cmcmd::SymlinkLibrary(args);
      }
    // Internal CMake versioned executable support.
    else if (args[1] == "cmake_symlink_executable" && args.size() == 4)
      {
      return cmcmd::SymlinkExecutable(args);
      }

#if defined(CMAKE_HAVE_VS_GENERATORS)
    // Internal CMake support for calling Visual Studio macros.
    else if (args[1] == "cmake_call_visual_studio_macro" && args.size() >= 4)
      {
      // args[2] = full path to .sln file or "ALL"
      // args[3] = name of Visual Studio macro to call
      // args[4..args.size()-1] = [optional] args for Visual Studio macro

      std::string macroArgs;

      if (args.size() > 4)
        {
        macroArgs = args[4];

        for (size_t i = 5; i < args.size(); ++i)
          {
          macroArgs += " ";
          macroArgs += args[i];
          }
        }

      return cmCallVisualStudioMacro::CallMacro(args[2], args[3],
        macroArgs, true);
      }
#endif

    // Internal CMake dependency scanning support.
    else if (args[1] == "cmake_depends" && args.size() >= 6)
      {
      // Use the make system's VERBOSE environment variable to enable
      // verbose output. This can be skipped by also setting CMAKE_NO_VERBOSE
      // (which is set by the Eclipse and KDevelop generators).
      bool verbose = ((cmSystemTools::GetEnv("VERBOSE") != 0)
                       && (cmSystemTools::GetEnv("CMAKE_NO_VERBOSE") == 0));

      // Create a cmake object instance to process dependencies.
      cmake cm;
      std::string gen;
      std::string homeDir;
      std::string startDir;
      std::string homeOutDir;
      std::string startOutDir;
      std::string depInfo;
      bool color = false;
      if(args.size() >= 8)
        {
        // Full signature:
        //
        //   -E cmake_depends <generator>
        //                    <home-src-dir> <start-src-dir>
        //                    <home-out-dir> <start-out-dir>
        //                    <dep-info> [--color=$(COLOR)]
        //
        // All paths are provided.
        gen = args[2];
        homeDir = args[3];
        startDir = args[4];
        homeOutDir = args[5];
        startOutDir = args[6];
        depInfo = args[7];
        if(args.size() >= 9 &&
           args[8].length() >= 8 &&
           args[8].substr(0, 8) == "--color=")
          {
          // Enable or disable color based on the switch value.
          color = (args[8].size() == 8 ||
                   cmSystemTools::IsOn(args[8].substr(8).c_str()));
          }
        }
      else
        {
        // Support older signature for existing makefiles:
        //
        //   -E cmake_depends <generator>
        //                    <home-out-dir> <start-out-dir>
        //                    <dep-info>
        //
        // Just pretend the source directories are the same as the
        // binary directories so at least scanning will work.
        gen = args[2];
        homeDir = args[3];
        startDir = args[4];
        homeOutDir = args[3];
        startOutDir = args[3];
        depInfo = args[5];
        }

      // Create a local generator configured for the directory in
      // which dependencies will be scanned.
      homeDir = cmSystemTools::CollapseFullPath(homeDir);
      startDir = cmSystemTools::CollapseFullPath(startDir);
      homeOutDir = cmSystemTools::CollapseFullPath(homeOutDir);
      startOutDir = cmSystemTools::CollapseFullPath(startOutDir);
      cm.SetHomeDirectory(homeDir);
      cm.SetHomeOutputDirectory(homeOutDir);
      if(cmGlobalGenerator* ggd = cm.CreateGlobalGenerator(gen))
        {
        cm.SetGlobalGenerator(ggd);
        cmsys::auto_ptr<cmLocalGenerator> lgd(ggd->MakeLocalGenerator());
        lgd->GetMakefile()->SetCurrentSourceDirectory(startDir);
        lgd->GetMakefile()->SetCurrentBinaryDirectory(startOutDir);

        // Actually scan dependencies.
        return lgd->UpdateDependencies(depInfo.c_str(),
                                       verbose, color)? 0 : 2;
        }
      return 1;
      }

    // Internal CMake link script support.
    else if (args[1] == "cmake_link_script" && args.size() >= 3)
      {
      return cmcmd::ExecuteLinkScript(args);
      }

    // Internal CMake unimplemented feature notification.
    else if (args[1] == "cmake_unimplemented_variable")
      {
      std::cerr << "Feature not implemented for this platform.";
      if(args.size() == 3)
        {
        std::cerr << "  Variable " << args[2] << " is not set.";
        }
      std::cerr << std::endl;
      return 1;
      }
    else if (args[1] == "vs_link_exe")
      {
      return cmcmd::VisualStudioLink(args, 1);
      }
    else if (args[1] == "vs_link_dll")
      {
      return cmcmd::VisualStudioLink(args, 2);
      }
    // Internal CMake color makefile support.
    else if (args[1] == "cmake_echo_color")
      {
      return cmcmd::ExecuteEchoColor(args);
      }
#ifdef CMAKE_BUILD_WITH_CMAKE
    else if (args[1] == "cmake_autogen" && args.size() >= 4)
      {
        cmQtAutoGenerators autogen;
        std::string const& config = args[3];
        bool autogenSuccess = autogen.Run(args[2], config);
        return autogenSuccess ? 0 : 1;
      }
#endif

    // Tar files
    else if (args[1] == "tar" && args.size() > 3)
      {
      const char* knownFormats[] =
        {
        "7zip",
        "gnutar",
        "pax",
        "paxr",
        "zip"
        };

      std::string flags = args[2];
      std::string outFile = args[3];
      std::vector<std::string> files;
      std::string mtime;
      std::string format;
      bool doing_options = true;
      for (std::string::size_type cc = 4; cc < args.size(); cc ++)
        {
        std::string const& arg = args[cc];
        if (doing_options && cmHasLiteralPrefix(arg, "--"))
          {
          if (arg == "--")
            {
            doing_options = false;
            }
          else if (cmHasLiteralPrefix(arg, "--mtime="))
            {
            mtime = arg.substr(8);
            }
          else if (cmHasLiteralPrefix(arg, "--files-from="))
            {
            std::string const& files_from = arg.substr(13);
            if (!cmTarFilesFrom(files_from, files))
              {
              return 1;
              }
            }
          else if (cmHasLiteralPrefix(arg, "--format="))
            {
            format = arg.substr(9);
            bool isKnown = std::find(cmArrayBegin(knownFormats),
              cmArrayEnd(knownFormats), format) != cmArrayEnd(knownFormats);

            if(!isKnown)
              {
              cmSystemTools::Error("Unknown -E tar --format= argument: ",
                format.c_str());
              return 1;
              }
            }
          else
            {
            cmSystemTools::Error("Unknown option to -E tar: ", arg.c_str());
            return 1;
            }
          }
        else
          {
          files.push_back(arg);
          }
        }
      cmSystemTools::cmTarCompression compress =
        cmSystemTools::TarCompressNone;
      bool verbose = false;
      int nCompress = 0;
      if ( flags.find_first_of('j') != flags.npos )
        {
        compress = cmSystemTools::TarCompressBZip2;
        ++nCompress;
        }
      if ( flags.find_first_of('J') != flags.npos )
        {
        compress = cmSystemTools::TarCompressXZ;
        ++nCompress;
        }
      if ( flags.find_first_of('z') != flags.npos )
        {
        compress = cmSystemTools::TarCompressGZip;
        ++nCompress;
        }
      if ( (format == "7zip" || format == "zip") && nCompress > 0 )
        {
        cmSystemTools::Error("Can not use compression flags with format: ",
          format.c_str());
        return 1;
        }
      else if ( nCompress > 1 )
        {
        cmSystemTools::Error("Can only compress a tar file one way; "
                             "at most one flag of z, j, or J may be used");
        return 1;
        }
      if ( flags.find_first_of('v') != flags.npos )
        {
        verbose = true;
        }

      if ( flags.find_first_of('t') != flags.npos )
        {
        if ( !cmSystemTools::ListTar(outFile.c_str(), verbose) )
          {
          cmSystemTools::Error("Problem listing tar: ", outFile.c_str());
          return 1;
          }
        }
      else if ( flags.find_first_of('c') != flags.npos )
        {
        if ( !cmSystemTools::CreateTar(
               outFile.c_str(), files, compress, verbose, mtime, format) )
          {
          cmSystemTools::Error("Problem creating tar: ", outFile.c_str());
          return 1;
          }
        }
      else if ( flags.find_first_of('x') != flags.npos )
        {
        if ( !cmSystemTools::ExtractTar(
            outFile.c_str(), verbose) )
          {
          cmSystemTools::Error("Problem extracting tar: ", outFile.c_str());
          return 1;
          }
#ifdef WIN32
        // OK, on windows 7 after we untar some files,
        // sometimes we can not rename the directory after
        // the untar is done. This breaks the external project
        // untar and rename code.  So, by default we will wait
        // 1/10th of a second after the untar.  If CMAKE_UNTAR_DELAY
        // is set in the env, its value will be used instead of 100.
        int delay = 100;
        const char* delayVar = cmSystemTools::GetEnv("CMAKE_UNTAR_DELAY");
        if(delayVar)
          {
          delay = atoi(delayVar);
          }
        if(delay)
          {
          cmSystemTools::Delay(delay);
          }
#endif
        }
      return 0;
      }

#if defined(CMAKE_BUILD_WITH_CMAKE)
    // Internal CMake Fortran module support.
    else if (args[1] == "cmake_copy_f90_mod" && args.size() >= 4)
      {
      return cmDependsFortran::CopyModule(args)? 0 : 1;
      }
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
    // Write registry value
    else if (args[1] == "write_regv" && args.size() > 3)
      {
      return cmSystemTools::WriteRegistryValue(args[2].c_str(),
                                               args[3].c_str()) ? 0 : 1;
      }

    // Delete registry value
    else if (args[1] == "delete_regv" && args.size() > 2)
      {
      return cmSystemTools::DeleteRegistryValue(args[2].c_str()) ? 0 : 1;
      }
    // Remove file
    else if (args[1] == "comspec" && args.size() > 2)
      {
      std::cerr << "Win9x helper \"cmake -E comspec\" no longer supported\n";
      return 1;
      }
    else if (args[1] == "env_vs8_wince" && args.size() == 3)
      {
      return cmcmd::WindowsCEEnvironment("8.0", args[2]);
      }
    else if (args[1] == "env_vs9_wince" && args.size() == 3)
      {
      return cmcmd::WindowsCEEnvironment("9.0", args[2]);
      }
#endif
    }

  ::CMakeCommandUsage(args[0].c_str());
  return 1;
}

//----------------------------------------------------------------------------
int cmcmd::SymlinkLibrary(std::vector<std::string>& args)
{
  int result = 0;
  std::string realName = args[2];
  std::string soName = args[3];
  std::string name = args[4];
  if(soName != realName)
    {
    if(!cmcmd::SymlinkInternal(realName, soName))
      {
      cmSystemTools::ReportLastSystemError("cmake_symlink_library");
      result = 1;
      }
    }
  if(name != soName)
    {
    if(!cmcmd::SymlinkInternal(soName, name))
      {
      cmSystemTools::ReportLastSystemError("cmake_symlink_library");
      result = 1;
      }
    }
  return result;
}

//----------------------------------------------------------------------------
int cmcmd::SymlinkExecutable(std::vector<std::string>& args)
{
  int result = 0;
  std::string realName = args[2];
  std::string name = args[3];
  if(name != realName)
    {
    if(!cmcmd::SymlinkInternal(realName, name))
      {
      cmSystemTools::ReportLastSystemError("cmake_symlink_executable");
      result = 1;
      }
    }
  return result;
}

//----------------------------------------------------------------------------
bool cmcmd::SymlinkInternal(std::string const& file, std::string const& link)
{
  if(cmSystemTools::FileExists(link.c_str()) ||
     cmSystemTools::FileIsSymlink(link))
    {
    cmSystemTools::RemoveFile(link);
    }
#if defined(_WIN32) && !defined(__CYGWIN__)
  return cmSystemTools::CopyFileAlways(file.c_str(), link.c_str());
#else
  std::string linktext = cmSystemTools::GetFilenameName(file);
  return cmSystemTools::CreateSymlink(linktext, link);
#endif
}

//----------------------------------------------------------------------------
static void cmcmdProgressReport(std::string const& dir,
                                std::string const& num)
{
  std::string dirName = dir;
  dirName += "/Progress";
  std::string fName;
  FILE *progFile;

  // read the count
  fName = dirName;
  fName += "/count.txt";
  progFile = cmsys::SystemTools::Fopen(fName,"r");
  int count = 0;
  if (!progFile)
    {
    return;
    }
  else
    {
    if (1!=fscanf(progFile,"%i",&count))
      {
      cmSystemTools::Message("Could not read from progress file.");
      }
    fclose(progFile);
    }
  const char* last = num.c_str();
  for(const char* c = last;; ++c)
    {
    if (*c == ',' || *c == '\0')
      {
      if (c != last)
        {
        fName = dirName;
        fName += "/";
        fName.append(last, c-last);
        progFile = cmsys::SystemTools::Fopen(fName,"w");
        if (progFile)
          {
          fprintf(progFile,"empty");
          fclose(progFile);
          }
        }
      if(*c == '\0')
        {
        break;
        }
      last = c + 1;
      }
    }
  int fileNum = static_cast<int>
    (cmsys::Directory::GetNumberOfFilesInDirectory(dirName));
  if (count > 0)
    {
    // print the progress
    fprintf(stdout,"[%3i%%] ",((fileNum-3)*100)/count);
    }
}

//----------------------------------------------------------------------------
int cmcmd::ExecuteEchoColor(std::vector<std::string>& args)
{
  // The arguments are
  //   argv[0] == <cmake-executable>
  //   argv[1] == cmake_echo_color

  bool enabled = true;
  int color = cmsysTerminal_Color_Normal;
  bool newline = true;
  std::string progressDir;
  for(unsigned int i=2; i < args.size(); ++i)
    {
    if(args[i].find("--switch=") == 0)
      {
      // Enable or disable color based on the switch value.
      std::string value = args[i].substr(9);
      if(!value.empty())
        {
        if(cmSystemTools::IsOn(value.c_str()))
          {
          enabled = true;
          }
        else
          {
          enabled = false;
          }
        }
      }
    else if(cmHasLiteralPrefix(args[i], "--progress-dir="))
      {
      progressDir = args[i].substr(15);
      }
    else if(cmHasLiteralPrefix(args[i], "--progress-num="))
      {
      if (!progressDir.empty())
        {
        std::string const& progressNum = args[i].substr(15);
        cmcmdProgressReport(progressDir, progressNum);
        }
      }
    else if(args[i] == "--normal")
      {
      color = cmsysTerminal_Color_Normal;
      }
    else if(args[i] == "--black")
      {
      color = cmsysTerminal_Color_ForegroundBlack;
      }
    else if(args[i] == "--red")
      {
      color = cmsysTerminal_Color_ForegroundRed;
      }
    else if(args[i] == "--green")
      {
      color = cmsysTerminal_Color_ForegroundGreen;
      }
    else if(args[i] == "--yellow")
      {
      color = cmsysTerminal_Color_ForegroundYellow;
      }
    else if(args[i] == "--blue")
      {
      color = cmsysTerminal_Color_ForegroundBlue;
      }
    else if(args[i] == "--magenta")
      {
      color = cmsysTerminal_Color_ForegroundMagenta;
      }
    else if(args[i] == "--cyan")
      {
      color = cmsysTerminal_Color_ForegroundCyan;
      }
    else if(args[i] == "--white")
      {
      color = cmsysTerminal_Color_ForegroundWhite;
      }
    else if(args[i] == "--bold")
      {
      color |= cmsysTerminal_Color_ForegroundBold;
      }
    else if(args[i] == "--no-newline")
      {
      newline = false;
      }
    else if(args[i] == "--newline")
      {
      newline = true;
      }
    else
      {
      // Color is enabled.  Print with the current color.
      cmSystemTools::MakefileColorEcho(color, args[i].c_str(),
                                       newline, enabled);
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
int cmcmd::ExecuteLinkScript(std::vector<std::string>& args)
{
  // The arguments are
  //   argv[0] == <cmake-executable>
  //   argv[1] == cmake_link_script
  //   argv[2] == <link-script-name>
  //   argv[3] == --verbose=?
  bool verbose = false;
  if(args.size() >= 4)
    {
    if(args[3].find("--verbose=") == 0)
      {
      if(!cmSystemTools::IsOff(args[3].substr(10).c_str()))
        {
        verbose = true;
        }
      }
    }

  // Allocate a process instance.
  cmsysProcess* cp = cmsysProcess_New();
  if(!cp)
    {
    std::cerr << "Error allocating process instance in link script."
              << std::endl;
    return 1;
    }

  // Children should share stdout and stderr with this process.
  cmsysProcess_SetPipeShared(cp, cmsysProcess_Pipe_STDOUT, 1);
  cmsysProcess_SetPipeShared(cp, cmsysProcess_Pipe_STDERR, 1);

  // Run the command lines verbatim.
  cmsysProcess_SetOption(cp, cmsysProcess_Option_Verbatim, 1);

  // Read command lines from the script.
  cmsys::ifstream fin(args[2].c_str());
  if(!fin)
    {
    std::cerr << "Error opening link script \""
              << args[2] << "\"" << std::endl;
    return 1;
    }

  // Run one command at a time.
  std::string command;
  int result = 0;
  while(result == 0 && cmSystemTools::GetLineFromStream(fin, command))
    {
    // Skip empty command lines.
    if(command.find_first_not_of(" \t") == command.npos)
      {
      continue;
      }

    // Setup this command line.
    const char* cmd[2] = {command.c_str(), 0};
    cmsysProcess_SetCommand(cp, cmd);

    // Report the command if verbose output is enabled.
    if(verbose)
      {
      std::cout << command << std::endl;
      }

    // Run the command and wait for it to exit.
    cmsysProcess_Execute(cp);
    cmsysProcess_WaitForExit(cp, 0);

    // Report failure if any.
    switch(cmsysProcess_GetState(cp))
      {
      case cmsysProcess_State_Exited:
        {
        int value = cmsysProcess_GetExitValue(cp);
        if(value != 0)
          {
          result = value;
          }
        }
        break;
      case cmsysProcess_State_Exception:
        std::cerr << "Error running link command: "
                  << cmsysProcess_GetExceptionString(cp) << std::endl;
        result = 1;
        break;
      case cmsysProcess_State_Error:
        std::cerr << "Error running link command: "
                  << cmsysProcess_GetErrorString(cp) << std::endl;
        result = 2;
        break;
      default:
        break;
      };
    }

  // Free the process instance.
  cmsysProcess_Delete(cp);

  // Return the final resulting return value.
  return result;
}

//----------------------------------------------------------------------------
int cmcmd::WindowsCEEnvironment(const char* version, const std::string& name)
{
#if defined(CMAKE_HAVE_VS_GENERATORS)
  cmVisualStudioWCEPlatformParser parser(name.c_str());
  parser.ParseVersion(version);
  if (parser.Found())
    {
    std::cout << "@echo off" << std::endl;
    std::cout << "echo Environment Selection: " << name << std::endl;
    std::cout << "set PATH=" << parser.GetPathDirectories() << std::endl;
    std::cout << "set INCLUDE=" << parser.GetIncludeDirectories() <<std::endl;
    std::cout << "set LIB=" << parser.GetLibraryDirectories() <<std::endl;
    return 0;
    }
#else
  (void)version;
#endif

  std::cerr << "Could not find " << name;
  return -1;
}

// For visual studio 2005 and newer manifest files need to be embedded into
// exe and dll's.  This code does that in such a way that incremental linking
// still works.
int cmcmd::VisualStudioLink(std::vector<std::string>& args, int type)
{
  if(args.size() < 2)
    {
    return -1;
    }
  bool verbose = false;
  if(cmSystemTools::GetEnv("VERBOSE"))
    {
    verbose = true;
    }
  std::vector<std::string> expandedArgs;
  for(std::vector<std::string>::iterator i = args.begin();
      i != args.end(); ++i)
    {
    // check for nmake temporary files
    if((*i)[0] == '@' && i->find("@CMakeFiles") != 0 )
      {
      cmsys::ifstream fin(i->substr(1).c_str());
      std::string line;
      while(cmSystemTools::GetLineFromStream(fin,
                                             line))
        {
        cmSystemTools::ParseWindowsCommandLine(line.c_str(), expandedArgs);
        }
      }
    else
      {
      expandedArgs.push_back(*i);
      }
    }
  bool hasIncremental = false;
  bool hasManifest = true;
  for(std::vector<std::string>::iterator i = expandedArgs.begin();
      i != expandedArgs.end(); ++i)
    {
    if(cmSystemTools::Strucmp(i->c_str(), "/INCREMENTAL:YES") == 0)
      {
      hasIncremental = true;
      }
    if(cmSystemTools::Strucmp(i->c_str(), "/INCREMENTAL") == 0)
      {
      hasIncremental = true;
      }
    if(cmSystemTools::Strucmp(i->c_str(), "/MANIFEST:NO") == 0)
      {
      hasManifest = false;
      }
    }
  if(hasIncremental && hasManifest)
    {
    if(verbose)
      {
      std::cout << "Visual Studio Incremental Link with embedded manifests\n";
      }
    return cmcmd::VisualStudioLinkIncremental(expandedArgs, type, verbose);
    }
  if(verbose)
    {
    if(!hasIncremental)
      {
      std::cout << "Visual Studio Non-Incremental Link\n";
      }
    else
      {
      std::cout << "Visual Studio Incremental Link without manifests\n";
      }
    }
  return cmcmd::VisualStudioLinkNonIncremental(expandedArgs,
                                               type, hasManifest, verbose);
}

int cmcmd::ParseVisualStudioLinkCommand(std::vector<std::string>& args,
                                        std::vector<std::string>& command,
                                        std::string& targetName)
{
  std::vector<std::string>::iterator i = args.begin();
  i++; // skip -E
  i++; // skip vs_link_dll or vs_link_exe
  command.push_back(*i);
  i++; // move past link command
  for(; i != args.end(); ++i)
    {
    command.push_back(*i);
    if(i->find("/Fe") == 0)
      {
      targetName = i->substr(3);
      }
    if(i->find("/out:") == 0)
      {
      targetName = i->substr(5);
      }
    }
  if(targetName.empty() || command.empty())
    {
    return -1;
    }
  return 0;
}

bool cmcmd::RunCommand(const char* comment,
                       std::vector<std::string>& command,
                       bool verbose,
                       int* retCodeOut)
{
  if(verbose)
    {
    std::cout << comment << ":\n";
    std::cout << cmJoin(command, " ") << "\n";
    }
  std::string output;
  int retCode =0;
  // use rc command to create .res file
  cmSystemTools::RunSingleCommand(command,
                                  &output, &output,
                                  &retCode, 0, cmSystemTools::OUTPUT_NONE);
  // always print the output of the command, unless
  // it is the dumb rc command banner, but if the command
  // returned an error code then print the output anyway as
  // the banner may be mixed with some other important information.
  if(output.find("Resource Compiler Version") == output.npos
     || retCode !=0)
    {
    std::cout << output;
    }
  // if retCodeOut is requested then always return true
  // and set the retCodeOut to retCode
  if(retCodeOut)
    {
    *retCodeOut = retCode;
    return true;
    }
  if(retCode != 0)
    {
    std::cout << comment << " failed. with " << retCode << "\n";
    }
  return retCode == 0;
}

int cmcmd::VisualStudioLinkIncremental(std::vector<std::string>& args,
                                       int type, bool verbose)
{
  // This follows the steps listed here:
  // http://blogs.msdn.com/zakramer/archive/2006/05/22/603558.aspx

  //    1.  Compiler compiles the application and generates the *.obj files.
  //    2.  An empty manifest file is generated if this is a clean build and if
  //    not the previous one is reused.
  //    3.  The resource compiler (rc.exe) compiles the *.manifest file to a
  //    *.res file.
  //    4.  Linker generates the binary (EXE or DLL) with the /incremental
  //    switch and embeds the dummy manifest file. The linker also generates
  //    the real manifest file based on the binaries that your binary depends
  //    on.
  //    5.  The manifest tool (mt.exe) is then used to generate the final
  //    manifest.

  // If the final manifest is changed, then 6 and 7 are run, if not
  // they are skipped, and it is done.

  //    6.  The resource compiler is invoked one more time.
  //    7.  Finally, the Linker does another incremental link, but since the
  //    only thing that has changed is the *.res file that contains the
  //    manifest it is a short link.
  std::vector<std::string> linkCommand;
  std::string targetName;
  if(cmcmd::ParseVisualStudioLinkCommand(args, linkCommand, targetName) == -1)
    {
    return -1;
    }
  std::string manifestArg = "/MANIFESTFILE:";
  std::vector<std::string> rcCommand;
  rcCommand.push_back(cmSystemTools::FindProgram("rc.exe"));
  std::vector<std::string> mtCommand;
  mtCommand.push_back(cmSystemTools::FindProgram("mt.exe"));
  std::string tempManifest;
  tempManifest = targetName;
  tempManifest += ".intermediate.manifest";
  std::string resourceInputFile = targetName;
  resourceInputFile += ".resource.txt";
  if(verbose)
    {
    std::cout << "Create " << resourceInputFile << "\n";
    }
  // Create input file for rc command
  cmsys::ofstream fout(resourceInputFile.c_str());
  if(!fout)
    {
    return -1;
    }
  std::string manifestFile = targetName;
  manifestFile += ".embed.manifest";
  std::string fullPath= cmSystemTools::CollapseFullPath(manifestFile);
  fout << type << " /* CREATEPROCESS_MANIFEST_RESOURCE_ID "
    "*/ 24 /* RT_MANIFEST */ " << "\"" << fullPath << "\"";
  fout.close();
  manifestArg += tempManifest;
  // add the manifest arg to the linkCommand
  linkCommand.push_back("/MANIFEST");
  linkCommand.push_back(manifestArg);
  // if manifestFile is not yet created, create an
  // empty one
  if(!cmSystemTools::FileExists(manifestFile.c_str()))
    {
    if(verbose)
      {
      std::cout << "Create empty: " << manifestFile << "\n";
      }
    cmsys::ofstream foutTmp(manifestFile.c_str());
    }
  std::string resourceFile = manifestFile;
  resourceFile += ".res";
  // add the resource file to the end of the link command
  linkCommand.push_back(resourceFile);
  std::string outputOpt = "/fo";
  outputOpt += resourceFile;
  rcCommand.push_back(outputOpt);
  rcCommand.push_back(resourceInputFile);
  // Run rc command to create resource
  if(!cmcmd::RunCommand("RC Pass 1", rcCommand, verbose))
    {
    return -1;
    }
  // Now run the link command to link and create manifest
  if(!cmcmd::RunCommand("LINK Pass 1", linkCommand, verbose))
    {
    return -1;
    }
  // create mt command
  std::string outArg("/out:");
  outArg+= manifestFile;
  mtCommand.push_back("/nologo");
  mtCommand.push_back(outArg);
  mtCommand.push_back("/notify_update");
  mtCommand.push_back("/manifest");
  mtCommand.push_back(tempManifest);
  //  now run mt.exe to create the final manifest file
  int mtRet =0;
  cmcmd::RunCommand("MT", mtCommand, verbose, &mtRet);
  // if mt returns 0, then the manifest was not changed and
  // we do not need to do another link step
  if(mtRet == 0)
    {
    return 0;
    }
  // check for magic mt return value if mt returns the magic number
  // 1090650113 then it means that it updated the manifest file and we need
  // to do the final link.  If mt has any value other than 0 or 1090650113
  // then there was some problem with the command itself and there was an
  // error so return the error code back out of cmake so make can report it.
  // (when hosted on a posix system the value is 187)
  if(mtRet != 1090650113 && mtRet != 187)
    {
    return mtRet;
    }
  // update the resource file with the new manifest from the mt command.
  if(!cmcmd::RunCommand("RC Pass 2", rcCommand, verbose))
    {
    return -1;
    }
  // Run the final incremental link that will put the new manifest resource
  // into the file incrementally.
  if(!cmcmd::RunCommand("FINAL LINK", linkCommand, verbose))
    {
    return -1;
    }
  return 0;
}

int cmcmd::VisualStudioLinkNonIncremental(std::vector<std::string>& args,
                                          int type,
                                          bool hasManifest,
                                          bool verbose)
{
  std::vector<std::string> linkCommand;
  std::string targetName;
  if(cmcmd::ParseVisualStudioLinkCommand(args, linkCommand, targetName) == -1)
    {
    return -1;
    }
  // Run the link command as given
  if (hasManifest)
    {
    linkCommand.push_back("/MANIFEST");
    }
  if(!cmcmd::RunCommand("LINK", linkCommand, verbose))
    {
    return -1;
    }
  if(!hasManifest)
    {
    return 0;
    }
  std::vector<std::string> mtCommand;
  mtCommand.push_back(cmSystemTools::FindProgram("mt.exe"));
  mtCommand.push_back("/nologo");
  mtCommand.push_back("/manifest");
  std::string manifestFile = targetName;
  manifestFile += ".manifest";
  mtCommand.push_back(manifestFile);
  std::string outresource = "/outputresource:";
  outresource += targetName;
  outresource += ";#";
  if(type == 1)
    {
    outresource += "1";
    }
  else if(type == 2)
    {
    outresource += "2";
    }
  mtCommand.push_back(outresource);
  // Now use the mt tool to embed the manifest into the exe or dll
  if(!cmcmd::RunCommand("MT", mtCommand, verbose))
    {
    return -1;
    }
  return 0;
}
