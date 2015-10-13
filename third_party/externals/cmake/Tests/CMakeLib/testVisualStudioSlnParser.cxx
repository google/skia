#include "testVisualStudioSlnParser.h"

#include "cmVisualStudioSlnData.h"
#include "cmVisualStudioSlnParser.h"

#include <cmsys/ios/iostream>

//----------------------------------------------------------------------------
static bool parsedRight(cmVisualStudioSlnParser& parser,
                        const std::string& file, cmSlnData& data,
                        cmVisualStudioSlnParser::ParseResult expected =
                        cmVisualStudioSlnParser::ResultOK)
{
  if (parser.ParseFile(SOURCE_DIR "/testVisualStudioSlnParser_data/" + file
                        + "." SLN_EXTENSION,
                        data, cmVisualStudioSlnParser::DataGroupProjects))
    {
    if (expected == cmVisualStudioSlnParser::ResultOK)
      {
      return true;
      }
    }
  else
    {
    if (parser.GetParseResult() == expected)
      {
      return true;
      }
    }
  cmsys_ios::cerr << "cmVisualStudioSlnParser mis-parsed " << file
                  << "." SLN_EXTENSION << "; expected result " << expected
                  << ", got " << parser.GetParseResult()
                  << cmsys_ios::endl;
  return false;
}

//----------------------------------------------------------------------------
int testVisualStudioSlnParser(int, char*[])
{
  cmVisualStudioSlnParser parser;

  // Test clean parser
  if (parser.GetParseResult() != cmVisualStudioSlnParser::ResultOK)
    {
    cmsys_ios::cerr << "cmVisualStudioSlnParser initialisation failed"
                    << cmsys_ios::endl;
    return 1;
    }

  // Test parsing valid sln
  {
  cmSlnData data;
  if (!parsedRight(parser, "valid", data))
    {
    return 1;
    }
  const std::vector<cmSlnProjectEntry>& projects = data.GetProjects();
  const char * const names[] =
    {
    "3rdParty", "ALL_BUILD", "CMakeLib", "CMakeLibTests",
    "CMakePredefinedTargets", "CPackLib", "CTestDashboardTargets", "CTestLib",
    "Continuous", "Documentation",
    "Experimental", "INSTALL", "KWSys", "LIBCURL", "Nightly",
    "NightlyMemoryCheck", "PACKAGE", "RUN_TESTS", "Tests", "Utilities",
    "Win9xCompat", "ZERO_CHECK", "cmIML_test", "cmake", "cmbzip2", "cmcldeps",
    "cmcompress", "cmcurl", "cmexpat", "cmlibarchive", "cmsys",
    "cmsysEncodeExecutable", "cmsysProcessFwd9x", "cmsysTestDynload",
    "cmsysTestProcess", "cmsysTestSharedForward", "cmsysTestsC",
    "cmsysTestsCxx", "cmsys_c", "cmw9xcom", "cmzlib", "cpack", "ctest",
    "documentation", "memcheck_fail", "pseudo_BC", "pseudo_purify",
    "pseudo_valgrind", "test_clean", "uninstall"
    };
  const size_t expectedProjectCount = sizeof(names) / sizeof(*names);
  if (projects.size() != expectedProjectCount)
    {
    cmsys_ios::cerr << "cmVisualStudioSlnParser returned bad number of "
                    << "projects (" << projects.size() << " instead of "
                    << expectedProjectCount << ')'
                    << cmsys_ios::endl;
    return 1;
    }
  for (size_t idx = 0; idx < expectedProjectCount; ++idx)
    {
    if (projects[idx].GetName() != names[idx])
      {
      cmsys_ios::cerr << "cmVisualStudioSlnParser returned bad project #"
                      << idx << "; expected \"" << names[idx] << "\", got \""
                      << projects[idx].GetName() << '"'
                      << cmsys_ios::endl;
      return 1;
      }
    }
  if (projects[0].GetRelativePath() != "Utilities\\3rdParty")
    {
    cmsys_ios::cerr << "cmVisualStudioSlnParser returned bad relative path of "
                    << "project 3rdParty; expected \"Utilities\\3rdParty\", "
                    << "got \"" << projects[0].GetRelativePath() << '"'
                    << cmsys_ios::endl;
    return 1;
    }
  if (projects[2].GetGUID() != "{59BCCCCD-3AD1-4491-B8F4-C5793AC007E2}")
    {
    cmsys_ios::cerr << "cmVisualStudioSlnParser returned bad relative path of "
                    << "project CMakeLib; expected "
                    << "\"{59BCCCCD-3AD1-4491-B8F4-C5793AC007E2}\", "
                    << "got \"" << projects[2].GetGUID() << '"'
                    << cmsys_ios::endl;
    return 1;
    }
  }

  // Test BOM parsing
  {
  cmSlnData data;

  if (!parsedRight(parser, "bom", data))
    {
    return 1;
    }
  if (!parser.GetParseHadBOM())
    {
    cmsys_ios::cerr << "cmVisualStudioSlnParser didn't find BOM in bom."
                    << SLN_EXTENSION
                    << cmsys_ios::endl;
    return 1;
    }

  if (!parsedRight(parser, "nobom", data))
    {
    return 1;
    }
  if (parser.GetParseHadBOM())
    {
    cmsys_ios::cerr << "cmVisualStudioSlnParser found BOM in nobom."
                    << SLN_EXTENSION
                    << cmsys_ios::endl;
    return 1;
    }
  }

  // Test invalid sln
  {
  {
  cmSlnData data;
  if (!parsedRight(parser, "err-nonexistent", data,
                   cmVisualStudioSlnParser::ResultErrorOpeningInput))
    {
    return 1;
    }
  }
  {
  cmSlnData data;
  if (!parsedRight(parser, "err-empty", data,
                   cmVisualStudioSlnParser::ResultErrorReadingInput))
    {
    return 1;
    }
  }
  const char * const files[] =
    {
    "header", "projectArgs", "topLevel", "projectContents", "projectSection",
    "global", "unclosed", "strayQuote", "strayParen", "strayQuote2"
    };
  for (size_t idx = 0; idx < sizeof(files) / sizeof(files[0]); ++idx)
    {
    cmSlnData data;
    if (!parsedRight(parser, std::string("err-structure-") + files[idx], data,
                     cmVisualStudioSlnParser::ResultErrorInputStructure))
      {
      return 1;
      }
    }
  {
  cmSlnData data;
  if (!parsedRight(parser, "err-data", data,
                   cmVisualStudioSlnParser::ResultErrorInputData))
    {
    return 1;
    }
  }
  }

  // All is well
  return 0;
}
