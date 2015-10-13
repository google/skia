/*============================================================================
  KWSys - Kitware System Library
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "kwsysPrivate.h"

#if defined(_MSC_VER)
# pragma warning (disable:4786)
#endif

#include KWSYS_HEADER(SystemTools.hxx)
#include KWSYS_HEADER(ios/iostream)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "SystemTools.hxx.in"
# include "kwsys_ios_iostream.h.in"
#endif

// Include with <> instead of "" to avoid getting any in-source copy
// left on disk.
#include <testSystemTools.h>

#include <string.h> /* strcmp */

//----------------------------------------------------------------------------
static const char* toUnixPaths[][2] =
{
    { "/usr/local/bin/passwd", "/usr/local/bin/passwd" },
    { "/usr/lo cal/bin/pa sswd", "/usr/lo cal/bin/pa sswd" },
    { "/usr/lo\\ cal/bin/pa\\ sswd", "/usr/lo\\ cal/bin/pa\\ sswd" },
    { "c:/usr/local/bin/passwd", "c:/usr/local/bin/passwd" },
    { "c:/usr/lo cal/bin/pa sswd", "c:/usr/lo cal/bin/pa sswd" },
    { "c:/usr/lo\\ cal/bin/pa\\ sswd", "c:/usr/lo\\ cal/bin/pa\\ sswd" },
    { "\\usr\\local\\bin\\passwd", "/usr/local/bin/passwd" },
    { "\\usr\\lo cal\\bin\\pa sswd", "/usr/lo cal/bin/pa sswd" },
    { "\\usr\\lo\\ cal\\bin\\pa\\ sswd", "/usr/lo\\ cal/bin/pa\\ sswd" },
    { "c:\\usr\\local\\bin\\passwd", "c:/usr/local/bin/passwd" },
    { "c:\\usr\\lo cal\\bin\\pa sswd", "c:/usr/lo cal/bin/pa sswd" },
    { "c:\\usr\\lo\\ cal\\bin\\pa\\ sswd", "c:/usr/lo\\ cal/bin/pa\\ sswd" },
    { "\\\\usr\\local\\bin\\passwd", "//usr/local/bin/passwd" },
    { "\\\\usr\\lo cal\\bin\\pa sswd", "//usr/lo cal/bin/pa sswd" },
    { "\\\\usr\\lo\\ cal\\bin\\pa\\ sswd", "//usr/lo\\ cal/bin/pa\\ sswd" },
    {0, 0}
};

static bool CheckConvertToUnixSlashes(kwsys_stl::string input,
                                      kwsys_stl::string output)
{
  kwsys_stl::string result = input;
  kwsys::SystemTools::ConvertToUnixSlashes(result);
  if ( result != output )
    {
    kwsys_ios::cerr
      << "Problem with ConvertToUnixSlashes - input: " << input
      << " output: " << result << " expected: " << output
      << kwsys_ios::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
static const char* checkEscapeChars[][4] =
{
  { "1 foo 2 bar 2", "12", "\\", "\\1 foo \\2 bar \\2"},
  { " {} ", "{}", "#", " #{#} "},
  {0, 0, 0, 0}
};

static bool CheckEscapeChars(kwsys_stl::string input,
                             const char *chars_to_escape,
                             char escape_char,
                             kwsys_stl::string output)
{
  kwsys_stl::string result = kwsys::SystemTools::EscapeChars(
    input.c_str(), chars_to_escape, escape_char);
  if (result != output)
    {
    kwsys_ios::cerr
      << "Problem with CheckEscapeChars - input: " << input
      << " output: " << result << " expected: " << output
      << kwsys_ios::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
static bool CheckFileOperations()
{
  bool res = true;
  const kwsys_stl::string testBinFile(TEST_SYSTEMTOOLS_SOURCE_DIR
    "/testSystemTools.bin");
  const kwsys_stl::string testTxtFile(TEST_SYSTEMTOOLS_SOURCE_DIR
    "/testSystemTools.cxx");
  const kwsys_stl::string testNewDir(TEST_SYSTEMTOOLS_BINARY_DIR
    "/testSystemToolsNewDir");
  const kwsys_stl::string testNewFile(testNewDir + "/testNewFile.txt");

  if (kwsys::SystemTools::DetectFileType(testBinFile.c_str()) !=
      kwsys::SystemTools::FileTypeBinary)
    {
    kwsys_ios::cerr
      << "Problem with DetectFileType - failed to detect type of: "
      << testBinFile << kwsys_ios::endl;
    res = false;
    }

  if (kwsys::SystemTools::DetectFileType(testTxtFile.c_str()) !=
      kwsys::SystemTools::FileTypeText)
    {
    kwsys_ios::cerr
      << "Problem with DetectFileType - failed to detect type of: "
      << testTxtFile << kwsys_ios::endl;
    res = false;
    }

  if (kwsys::SystemTools::FileLength(testBinFile) != 766)
    {
    kwsys_ios::cerr
      << "Problem with FileLength - incorrect length for: "
      << testBinFile << kwsys_ios::endl;
    res = false;
    }

  if (!kwsys::SystemTools::MakeDirectory(testNewDir))
    {
    kwsys_ios::cerr
      << "Problem with MakeDirectory for: "
      << testNewDir << kwsys_ios::endl;
    res = false;
    }

  if (!kwsys::SystemTools::Touch(testNewFile.c_str(), true))
    {
    kwsys_ios::cerr
      << "Problem with Touch for: "
      << testNewFile << kwsys_ios::endl;
    res = false;
    }

  if (!kwsys::SystemTools::RemoveFile(testNewFile))
    {
    kwsys_ios::cerr
      << "Problem with RemoveFile: "
      << testNewFile << kwsys_ios::endl;
    res = false;
    }

  kwsys_stl::string const testFileMissing(testNewDir + "/testMissingFile.txt");
  if (!kwsys::SystemTools::RemoveFile(testFileMissing))
    {
    std::string const& msg = kwsys::SystemTools::GetLastSystemError();
    kwsys_ios::cerr <<
      "RemoveFile(\"" << testFileMissing << "\") failed: " << msg << "\n";
    res = false;
    }

  kwsys_stl::string const testFileMissingDir(testNewDir + "/missing/file.txt");
  if (!kwsys::SystemTools::RemoveFile(testFileMissingDir))
    {
    std::string const& msg = kwsys::SystemTools::GetLastSystemError();
    kwsys_ios::cerr <<
      "RemoveFile(\"" << testFileMissingDir << "\") failed: " << msg << "\n";
    res = false;
    }

  kwsys::SystemTools::Touch(testNewFile.c_str(), true);
  if (!kwsys::SystemTools::RemoveADirectory(testNewDir))
    {
    kwsys_ios::cerr
      << "Problem with RemoveADirectory for: "
      << testNewDir << kwsys_ios::endl;
    res = false;
    }

#ifdef KWSYS_TEST_SYSTEMTOOLS_LONG_PATHS
  // Perform the same file and directory creation and deletion tests but
  // with paths > 256 characters in length.

  const kwsys_stl::string testNewLongDir(
    TEST_SYSTEMTOOLS_BINARY_DIR "/"
    "012345678901234567890123456789012345678901234567890123456789"
    "012345678901234567890123456789012345678901234567890123456789"
    "012345678901234567890123456789012345678901234567890123456789"
    "012345678901234567890123456789012345678901234567890123456789"
    "01234567890123");
  const kwsys_stl::string testNewLongFile(testNewLongDir + "/"
    "012345678901234567890123456789012345678901234567890123456789"
    "012345678901234567890123456789012345678901234567890123456789"
    "012345678901234567890123456789012345678901234567890123456789"
    "012345678901234567890123456789012345678901234567890123456789"
    "0123456789.txt");

  if (!kwsys::SystemTools::MakeDirectory(testNewLongDir))
    {
    kwsys_ios::cerr
      << "Problem with MakeDirectory for: "
      << testNewLongDir << kwsys_ios::endl;
    res = false;
    }

  if (!kwsys::SystemTools::Touch(testNewLongFile.c_str(), true))
    {
    kwsys_ios::cerr
      << "Problem with Touch for: "
      << testNewLongFile << kwsys_ios::endl;
    res = false;
    }

  if (!kwsys::SystemTools::RemoveFile(testNewLongFile))
    {
    kwsys_ios::cerr
      << "Problem with RemoveFile: "
      << testNewLongFile << kwsys_ios::endl;
    res = false;
    }

  kwsys::SystemTools::Touch(testNewLongFile.c_str(), true);
  if (!kwsys::SystemTools::RemoveADirectory(testNewLongDir))
    {
    kwsys_ios::cerr
      << "Problem with RemoveADirectory for: "
      << testNewLongDir << kwsys_ios::endl;
    res = false;
    }
#endif

  return res;
}

//----------------------------------------------------------------------------
static bool CheckStringOperations()
{
  bool res = true;

  kwsys_stl::string test = "mary had a little lamb.";
  if (kwsys::SystemTools::CapitalizedWords(test) != "Mary Had A Little Lamb.")
    {
    kwsys_ios::cerr
      << "Problem with CapitalizedWords "
      << '"' << test << '"' << kwsys_ios::endl;
    res = false;    
    }

  test = "Mary Had A Little Lamb.";
  if (kwsys::SystemTools::UnCapitalizedWords(test) != 
      "mary had a little lamb.")
    {
    kwsys_ios::cerr
      << "Problem with UnCapitalizedWords "
      << '"' << test << '"' << kwsys_ios::endl;
    res = false;    
    }

  test = "MaryHadTheLittleLamb.";
  if (kwsys::SystemTools::AddSpaceBetweenCapitalizedWords(test) != 
      "Mary Had The Little Lamb.")
    {
    kwsys_ios::cerr
      << "Problem with AddSpaceBetweenCapitalizedWords "
      << '"' << test << '"' << kwsys_ios::endl;
    res = false;    
    }

  char * cres = 
    kwsys::SystemTools::AppendStrings("Mary Had A"," Little Lamb.");
  if (strcmp(cres,"Mary Had A Little Lamb."))
    {
    kwsys_ios::cerr
      << "Problem with AppendStrings "
      << "\"Mary Had A\" \" Little Lamb.\"" << kwsys_ios::endl;
    res = false;    
    }
  delete [] cres;

  cres = 
    kwsys::SystemTools::AppendStrings("Mary Had"," A ","Little Lamb.");
  if (strcmp(cres,"Mary Had A Little Lamb."))
    {
    kwsys_ios::cerr
      << "Problem with AppendStrings "
      << "\"Mary Had\" \" A \" \"Little Lamb.\"" << kwsys_ios::endl;
    res = false;    
    }
  delete [] cres;

  if (kwsys::SystemTools::CountChar("Mary Had A Little Lamb.",'a') != 3)
    {
    kwsys_ios::cerr
      << "Problem with CountChar "
      << "\"Mary Had A Little Lamb.\"" << kwsys_ios::endl;
    res = false;    
    }

  cres = 
    kwsys::SystemTools::RemoveChars("Mary Had A Little Lamb.","aeiou");
  if (strcmp(cres,"Mry Hd A Lttl Lmb."))
    {
    kwsys_ios::cerr
      << "Problem with RemoveChars "
      << "\"Mary Had A Little Lamb.\"" << kwsys_ios::endl;
    res = false;    
    }
  delete [] cres;

  cres = 
    kwsys::SystemTools::RemoveCharsButUpperHex("Mary Had A Little Lamb.");
  if (strcmp(cres,"A"))
    {
    kwsys_ios::cerr
      << "Problem with RemoveCharsButUpperHex "
      << "\"Mary Had A Little Lamb.\"" << kwsys_ios::endl;
    res = false;    
    }
  delete [] cres;

  char *cres2 = new char [strlen("Mary Had A Little Lamb.")+1];
  strcpy(cres2,"Mary Had A Little Lamb.");
  kwsys::SystemTools::ReplaceChars(cres2,"aeiou",'X');
  if (strcmp(cres2,"MXry HXd A LXttlX LXmb."))
    {
    kwsys_ios::cerr
      << "Problem with ReplaceChars "
      << "\"Mary Had A Little Lamb.\"" << kwsys_ios::endl;
    res = false;    
    }
  delete [] cres2;

  if (!kwsys::SystemTools::StringStartsWith("Mary Had A Little Lamb.",
                                            "Mary "))
    {
    kwsys_ios::cerr
      << "Problem with StringStartsWith "
      << "\"Mary Had A Little Lamb.\"" << kwsys_ios::endl;
    res = false;    
    }

  if (!kwsys::SystemTools::StringEndsWith("Mary Had A Little Lamb.",
                                          " Lamb."))
    {
    kwsys_ios::cerr
      << "Problem with StringEndsWith "
      << "\"Mary Had A Little Lamb.\"" << kwsys_ios::endl;
    res = false;    
    }

  cres = kwsys::SystemTools::DuplicateString("Mary Had A Little Lamb.");
  if (strcmp(cres,"Mary Had A Little Lamb."))
    {
    kwsys_ios::cerr
      << "Problem with DuplicateString "
      << "\"Mary Had A Little Lamb.\"" << kwsys_ios::endl;
    res = false;    
    }
  delete [] cres;

  test = "Mary Had A Little Lamb.";
  if (kwsys::SystemTools::CropString(test,13) != 
      "Mary ...Lamb.")
    {
    kwsys_ios::cerr
      << "Problem with CropString "
      << "\"Mary Had A Little Lamb.\"" << kwsys_ios::endl;
    res = false;    
    }

  kwsys_stl::vector<kwsys_stl::string> lines;
  kwsys::SystemTools::Split("Mary Had A Little Lamb.",lines,' ');
  if (lines[0] != "Mary" || lines[1] != "Had" ||
      lines[2] != "A" || lines[3] != "Little" || lines[4] != "Lamb.")
    {
    kwsys_ios::cerr
      << "Problem with Split "
      << "\"Mary Had A Little Lamb.\"" << kwsys_ios::endl;
    res = false;
    }

#ifdef _WIN32
  if (kwsys::SystemTools::ConvertToWindowsExtendedPath
      ("L:\\Local Mojo\\Hex Power Pack\\Iffy Voodoo") !=
      L"\\\\?\\L:\\Local Mojo\\Hex Power Pack\\Iffy Voodoo")
    {
    kwsys_ios::cerr
      << "Problem with ConvertToWindowsExtendedPath "
      << "\"L:\\Local Mojo\\Hex Power Pack\\Iffy Voodoo\""
      << kwsys_ios::endl;
    res = false;
    }

  if (kwsys::SystemTools::ConvertToWindowsExtendedPath
      ("L:/Local Mojo/Hex Power Pack/Iffy Voodoo") !=
      L"\\\\?\\L:\\Local Mojo\\Hex Power Pack\\Iffy Voodoo")
    {
    kwsys_ios::cerr
      << "Problem with ConvertToWindowsExtendedPath "
      << "\"L:/Local Mojo/Hex Power Pack/Iffy Voodoo\""
      << kwsys_ios::endl;
    res = false;
    }

  if (kwsys::SystemTools::ConvertToWindowsExtendedPath
      ("\\\\Foo\\Local Mojo\\Hex Power Pack\\Iffy Voodoo") !=
      L"\\\\?\\UNC\\Foo\\Local Mojo\\Hex Power Pack\\Iffy Voodoo")
    {
    kwsys_ios::cerr
      << "Problem with ConvertToWindowsExtendedPath "
      << "\"\\\\Foo\\Local Mojo\\Hex Power Pack\\Iffy Voodoo\""
      << kwsys_ios::endl;
    res = false;
    }

  if (kwsys::SystemTools::ConvertToWindowsExtendedPath
      ("//Foo/Local Mojo/Hex Power Pack/Iffy Voodoo") !=
      L"\\\\?\\UNC\\Foo\\Local Mojo\\Hex Power Pack\\Iffy Voodoo")
    {
    kwsys_ios::cerr
      << "Problem with ConvertToWindowsExtendedPath "
      << "\"//Foo/Local Mojo/Hex Power Pack/Iffy Voodoo\""
      << kwsys_ios::endl;
    res = false;
    }

  if (kwsys::SystemTools::ConvertToWindowsExtendedPath("//") !=
      L"//")
    {
    kwsys_ios::cerr
      << "Problem with ConvertToWindowsExtendedPath "
      << "\"//\""
      << kwsys_ios::endl;
    res = false;
    }

  if (kwsys::SystemTools::ConvertToWindowsExtendedPath("\\\\.\\") !=
      L"\\\\.\\")
    {
    kwsys_ios::cerr
      << "Problem with ConvertToWindowsExtendedPath "
      << "\"\\\\.\\\""
      << kwsys_ios::endl;
    res = false;
    }

  if (kwsys::SystemTools::ConvertToWindowsExtendedPath("\\\\.\\X") !=
      L"\\\\.\\X")
    {
    kwsys_ios::cerr
      << "Problem with ConvertToWindowsExtendedPath "
      << "\"\\\\.\\X\""
      << kwsys_ios::endl;
    res = false;
    }

  if (kwsys::SystemTools::ConvertToWindowsExtendedPath("\\\\.\\X:") !=
      L"\\\\?\\X:")
    {
    kwsys_ios::cerr
      << "Problem with ConvertToWindowsExtendedPath "
      << "\"\\\\.\\X:\""
      << kwsys_ios::endl;
    res = false;
    }

  if (kwsys::SystemTools::ConvertToWindowsExtendedPath("\\\\.\\X:\\") !=
      L"\\\\?\\X:\\")
    {
    kwsys_ios::cerr
      << "Problem with ConvertToWindowsExtendedPath "
      << "\"\\\\.\\X:\\\""
      << kwsys_ios::endl;
    res = false;
    }

  if (kwsys::SystemTools::ConvertToWindowsExtendedPath("NUL") !=
      L"\\\\.\\NUL")
    {
    kwsys_ios::cerr
      << "Problem with ConvertToWindowsExtendedPath "
      << "\"NUL\""
      << kwsys_ios::endl;
    res = false;
    }

#endif

  if (kwsys::SystemTools::ConvertToWindowsOutputPath
      ("L://Local Mojo/Hex Power Pack/Iffy Voodoo") != 
      "\"L:\\Local Mojo\\Hex Power Pack\\Iffy Voodoo\"")
    {
    kwsys_ios::cerr
      << "Problem with ConvertToWindowsOutputPath "
      << "\"L://Local Mojo/Hex Power Pack/Iffy Voodoo\""
      << kwsys_ios::endl;
    res = false;    
    }
  
  if (kwsys::SystemTools::ConvertToWindowsOutputPath
      ("//grayson/Local Mojo/Hex Power Pack/Iffy Voodoo") != 
      "\"\\\\grayson\\Local Mojo\\Hex Power Pack\\Iffy Voodoo\"")
    {
    kwsys_ios::cerr
      << "Problem with ConvertToWindowsOutputPath "
      << "\"//grayson/Local Mojo/Hex Power Pack/Iffy Voodoo\""
      << kwsys_ios::endl;
    res = false;    
    }

  if (kwsys::SystemTools::ConvertToUnixOutputPath
      ("//Local Mojo/Hex Power Pack/Iffy Voodoo") != 
      "//Local\\ Mojo/Hex\\ Power\\ Pack/Iffy\\ Voodoo")
    {
    kwsys_ios::cerr
      << "Problem with ConvertToUnixOutputPath "
      << "\"//Local Mojo/Hex Power Pack/Iffy Voodoo\""
      << kwsys_ios::endl;
    res = false;    
    }

  return res;
}

//----------------------------------------------------------------------------

static bool CheckPutEnv(const kwsys_stl::string& env, const char* name, const char* value)
{
  if(!kwsys::SystemTools::PutEnv(env))
    {
    kwsys_ios::cerr << "PutEnv(\"" << env
                    << "\") failed!" << kwsys_ios::endl;
    return false;
    }
  const char* v = kwsys::SystemTools::GetEnv(name);
  v = v? v : "(null)";
  if(strcmp(v, value) != 0)
    {
    kwsys_ios::cerr << "GetEnv(\"" << name << "\") returned \""
                    << v << "\", not \"" << value << "\"!" << kwsys_ios::endl;
    return false;
    }
  return true;
}

static bool CheckUnPutEnv(const char* env, const char* name)
{
  if(!kwsys::SystemTools::UnPutEnv(env))
    {
    kwsys_ios::cerr << "UnPutEnv(\"" << env << "\") failed!"
                    << kwsys_ios::endl;
    return false;
    }
  if(const char* v = kwsys::SystemTools::GetEnv(name))
    {
    kwsys_ios::cerr << "GetEnv(\"" << name << "\") returned \""
                    << v << "\", not (null)!" << kwsys_ios::endl;
    return false;
    }
  return true;
}

static bool CheckEnvironmentOperations()
{
  bool res = true;
  res &= CheckPutEnv("A=B", "A", "B");
  res &= CheckPutEnv("B=C", "B", "C");
  res &= CheckPutEnv("C=D", "C", "D");
  res &= CheckPutEnv("D=E", "D", "E");
  res &= CheckUnPutEnv("A", "A");
  res &= CheckUnPutEnv("B=", "B");
  res &= CheckUnPutEnv("C=D", "C");
  /* Leave "D=E" in environment so a memory checker can test for leaks.  */
  return res;
}


static bool CheckRelativePath(
  const kwsys_stl::string& local,
  const kwsys_stl::string& remote,
  const kwsys_stl::string& expected)
{
  kwsys_stl::string result = kwsys::SystemTools::RelativePath(local, remote);
  if(expected != result)
    {
    kwsys_ios::cerr << "RelativePath(" << local << ", " << remote
      << ")  yielded " << result << " instead of " << expected << kwsys_ios::endl;
    return false;
    }
  return true;
}

static bool CheckRelativePaths()
{
  bool res = true;
  res &= CheckRelativePath("/usr/share", "/bin/bash", "../../bin/bash");
  res &= CheckRelativePath("/usr/./share/", "/bin/bash", "../../bin/bash");
  res &= CheckRelativePath("/usr//share/", "/bin/bash", "../../bin/bash");
  res &= CheckRelativePath("/usr/share/../bin/", "/bin/bash", "../../bin/bash");
  res &= CheckRelativePath("/usr/share", "/usr/share//bin", "bin");
  return res;
}

static bool CheckCollapsePath(
  const kwsys_stl::string& path,
  const kwsys_stl::string& expected)
{
  kwsys_stl::string result = kwsys::SystemTools::CollapseFullPath(path);
  if(expected != result)
    {
    kwsys_ios::cerr << "CollapseFullPath(" << path
      << ")  yielded " << result << " instead of " << expected << kwsys_ios::endl;
    return false;
    }
  return true;
}

static bool CheckCollapsePath()
{
  bool res = true;
  res &= CheckCollapsePath("/usr/share/*", "/usr/share/*");
  res &= CheckCollapsePath("C:/Windows/*", "C:/Windows/*");
  return res;
}

//----------------------------------------------------------------------------
int testSystemTools(int, char*[])
{
  bool res = true;

  int cc;
  for ( cc = 0; toUnixPaths[cc][0]; cc ++ )
    {
    res &= CheckConvertToUnixSlashes(toUnixPaths[cc][0], toUnixPaths[cc][1]);
    }

  // Special check for ~
  kwsys_stl::string output;
  if(kwsys::SystemTools::GetEnv("HOME", output))
    {
    output += "/foo bar/lala";
    res &= CheckConvertToUnixSlashes("~/foo bar/lala", output);
    }

  for (cc = 0; checkEscapeChars[cc][0]; cc ++ )
    {
    res &= CheckEscapeChars(checkEscapeChars[cc][0], checkEscapeChars[cc][1], 
                            *checkEscapeChars[cc][2], checkEscapeChars[cc][3]);
    }

  res &= CheckFileOperations();

  res &= CheckStringOperations();

  res &= CheckEnvironmentOperations();

  res &= CheckRelativePaths();

  res &= CheckCollapsePath();

  return res ? 0 : 1;
}
