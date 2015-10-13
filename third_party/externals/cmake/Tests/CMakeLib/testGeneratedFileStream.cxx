/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2011 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGeneratedFileStream.h"
#include "cmSystemTools.h"

#define cmFailed(m1, m2) \
  std::cout << "FAILED: " << m1 << m2 << "\n"; failed=1

int testGeneratedFileStream(int, char*[])
{
  int failed = 0;
  cmGeneratedFileStream gm;
  std::string file1 = "generatedFile1";
  std::string file2 = "generatedFile2";
  std::string file3 = "generatedFile3";
  std::string file4 = "generatedFile4";
  std::string file1tmp = file1 + ".tmp";
  std::string file2tmp = file2 + ".tmp";
  std::string file3tmp = file3 + ".tmp";
  std::string file4tmp = file4 + ".tmp";
  gm.Open(file1.c_str());
  gm << "This is generated file 1";
  gm.Close();
  gm.Open(file2.c_str());
  gm << "This is generated file 2";
  gm.Close();
  gm.Open(file3.c_str());
  gm << "This is generated file 3";
  gm.Close();
  gm.Open(file4.c_str());
  gm << "This is generated file 4";
  gm.Close();
  if ( cmSystemTools::FileExists(file1.c_str()) )
    {
    if ( cmSystemTools::FileExists(file2.c_str()) )
      {
      if ( cmSystemTools::FileExists(file3.c_str()) )
        {
        if ( cmSystemTools::FileExists(file4.c_str()) )
          {
          if ( cmSystemTools::FileExists(file1tmp.c_str()) )
            {
            cmFailed("Something wrong with cmGeneratedFileStream. Temporary file is still here: ", file1tmp.c_str());
            }
          else if ( cmSystemTools::FileExists(file2tmp.c_str()) )
            {
            cmFailed("Something wrong with cmGeneratedFileStream. Temporary file is still here: ", file2tmp.c_str());
            }
          else if ( cmSystemTools::FileExists(file3tmp.c_str()) )
            {
            cmFailed("Something wrong with cmGeneratedFileStream. Temporary file is still here: ", file3tmp.c_str());
            }
          else if ( cmSystemTools::FileExists(file4tmp.c_str()) )
            {
            cmFailed("Something wrong with cmGeneratedFileStream. Temporary file is still here: ", file4tmp.c_str());
            }
          else
            {
            std::cout << "cmGeneratedFileStream works\n";
            }
          }
        else
          {
          cmFailed("Something wrong with cmGeneratedFileStream. Cannot find file: ", file4.c_str());
          }
        }
      else
        {
        cmFailed("Something wrong with cmGeneratedFileStream. Found file: ", file3.c_str());
        }
      }
    else
      {
      cmFailed("Something wrong with cmGeneratedFileStream. Cannot find file: ", file2.c_str());
      }
    }
  else
    {
    cmFailed("Something wrong with cmGeneratedFileStream. Cannot find file: ", file1.c_str());
    }
  cmSystemTools::RemoveFile(file1.c_str());
  cmSystemTools::RemoveFile(file2.c_str());
  cmSystemTools::RemoveFile(file3.c_str());
  cmSystemTools::RemoveFile(file4.c_str());
  cmSystemTools::RemoveFile(file1tmp.c_str());
  cmSystemTools::RemoveFile(file2tmp.c_str());
  cmSystemTools::RemoveFile(file3tmp.c_str());
  cmSystemTools::RemoveFile(file4tmp.c_str());

  return failed;
}
