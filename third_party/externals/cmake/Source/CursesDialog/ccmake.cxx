/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "../cmSystemTools.h"
#include "../cmake.h"
#include "../cmDocumentation.h"

#include <signal.h>
#include <sys/ioctl.h>

#include "cmCursesMainForm.h"
#include "cmCursesStandardIncludes.h"
#include <cmsys/Encoding.hxx>

#include <form.h>

//----------------------------------------------------------------------------
static const char * cmDocumentationName[][2] =
{
  {0,
   "  ccmake - Curses Interface for CMake."},
  {0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationUsage[][2] =
{
  {0,
   "  ccmake <path-to-source>\n"
   "  ccmake <path-to-existing-build>"},
  {0,
   "Specify a source directory to (re-)generate a build system for "
   "it in the current working directory.  Specify an existing build "
   "directory to re-generate its build system."},
  {0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationUsageNote[][2] =
{
  {0,
   "Run 'ccmake --help' for more information."},
  {0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationOptions[][2] =
{
  CMAKE_STANDARD_OPTIONS_TABLE,
  {0,0}
};

cmCursesForm* cmCursesForm::CurrentForm=0;

extern "C"
{

void onsig(int)
{
  if (cmCursesForm::CurrentForm)
    {
    endwin();
    initscr(); /* Initialization */
    noecho(); /* Echo off */
    cbreak(); /* nl- or cr not needed */
    keypad(stdscr,TRUE); /* Use key symbols as
                            KEY_DOWN*/
    refresh();
    int x,y;
    getmaxyx(stdscr, y, x);
    cmCursesForm::CurrentForm->Render(1,1,x,y);
    cmCursesForm::CurrentForm->UpdateStatusBar();
    }
  signal(SIGWINCH, onsig);
}

}

void CMakeMessageHandler(const char* message, const char* title, bool&,
                         void* clientData)
{
  cmCursesForm* self = static_cast<cmCursesForm*>( clientData );
  self->AddError(message, title);
}

int main(int argc, char const* const* argv)
{
  cmsys::Encoding::CommandLineArguments encoding_args =
    cmsys::Encoding::CommandLineArguments::Main(argc, argv);
  argc = encoding_args.argc();
  argv = encoding_args.argv();

  cmSystemTools::FindCMakeResources(argv[0]);
  cmDocumentation doc;
  doc.addCMakeStandardDocSections();
  if(doc.CheckOptions(argc, argv))
    {
    cmake hcm;
    hcm.SetHomeDirectory("");
    hcm.SetHomeOutputDirectory("");
    hcm.AddCMakePaths();
    std::vector<cmDocumentationEntry> generators;
    hcm.GetGeneratorDocumentation(generators);
    doc.SetName("ccmake");
    doc.SetSection("Name",cmDocumentationName);
    doc.SetSection("Usage",cmDocumentationUsage);
    if ( argc == 1 )
      {
      doc.AppendSection("Usage",cmDocumentationUsageNote);
      }
    doc.SetSection("Generators",generators);
    doc.PrependSection("Options",cmDocumentationOptions);
    return doc.PrintRequestedDocumentation(std::cout)? 0:1;
    }

  bool debug = false;
  unsigned int i;
  int j;
  std::vector<std::string> args;
  for(j =0; j < argc; ++j)
    {
    if(strcmp(argv[j], "-debug") == 0)
      {
      debug = true;
      }
    else
      {
      args.push_back(argv[j]);
      }
    }

  std::string cacheDir = cmSystemTools::GetCurrentWorkingDirectory();
  for(i=1; i < args.size(); ++i)
    {
    std::string arg = args[i];
    if(arg.find("-B",0) == 0)
      {
      cacheDir = arg.substr(2);
      }
    }

  cmSystemTools::DisableRunCommandOutput();

  if (debug)
    {
    cmCursesForm::DebugStart();
    }

  initscr(); /* Initialization */
  noecho(); /* Echo off */
  cbreak(); /* nl- or cr not needed */
  keypad(stdscr,TRUE); /* Use key symbols as
                          KEY_DOWN*/

  signal(SIGWINCH, onsig);

  int x,y;
  getmaxyx(stdscr, y, x);
  if ( x < cmCursesMainForm::MIN_WIDTH  ||
       y < cmCursesMainForm::MIN_HEIGHT )
    {
    endwin();
    std::cerr << "Window is too small. A size of at least "
              << cmCursesMainForm::MIN_WIDTH << " x "
              <<  cmCursesMainForm::MIN_HEIGHT
              << " is required to run ccmake." <<  std::endl;
    return 1;
    }


  cmCursesMainForm* myform;

  myform = new cmCursesMainForm(args, x);
  if(myform->LoadCache(cacheDir.c_str()))
    {
    curses_clear();
    touchwin(stdscr);
    endwin();
    delete myform;
    std::cerr << "Error running cmake::LoadCache().  Aborting.\n";
    return 1;
    }

  cmSystemTools::SetMessageCallback(CMakeMessageHandler, myform);

  cmCursesForm::CurrentForm = myform;

  myform->InitializeUI();
  if ( myform->Configure(1) == 0 )
    {
    myform->Render(1, 1, x, y);
    myform->HandleInput();
    }

  // Need to clean-up better
  curses_clear();
  touchwin(stdscr);
  endwin();
  delete cmCursesForm::CurrentForm;
  cmCursesForm::CurrentForm = 0;

  std::cout << std::endl << std::endl;

  return 0;

}
