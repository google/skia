/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestSubmitHandler_h
#define cmCTestSubmitHandler_h

#include "cmCTestGenericHandler.h"

/** \class cmCTestSubmitHandler
 * \brief Helper class for CTest
 *
 * Submit testing results
 *
 */
class cmCTestSubmitHandler : public cmCTestGenericHandler
{
public:
  cmTypeMacro(cmCTestSubmitHandler, cmCTestGenericHandler);

  cmCTestSubmitHandler();
  ~cmCTestSubmitHandler() { this->LogFile = 0; }

  /*
   * The main entry point for this class
   */
  int ProcessHandler();

  void Initialize();

  /** Specify a set of parts (by name) to submit.  */
  void SelectParts(std::set<cmCTest::Part> const& parts);

  /** Specify a set of files to submit.  */
  void SelectFiles(cmCTest::SetOfStrings const& files);

  // handle the cdash file upload protocol
  int HandleCDashUploadFile(std::string const& file, std::string const& type);

  void ConstructCDashURL(std::string& dropMethod, std::string& url);

private:
  void SetLogFile(std::ostream* ost) { this->LogFile = ost; }

  /**
   * Submit file using various ways
   */
  bool SubmitUsingFTP(const std::string& localprefix,
                      const std::set<std::string>& files,
                      const std::string& remoteprefix,
                      const std::string& url);
  bool SubmitUsingHTTP(const std::string& localprefix,
                       const std::set<std::string>& files,
                       const std::string& remoteprefix,
                       const std::string& url);
  bool SubmitUsingSCP(const std::string& scp_command,
                      const std::string& localprefix,
                      const std::set<std::string>& files,
                      const std::string& remoteprefix,
                      const std::string& url);

  bool SubmitUsingCP( const std::string& localprefix,
                      const std::set<std::string>& files,
                      const std::string& remoteprefix,
                      const std::string& url);

  bool TriggerUsingHTTP(const std::set<std::string>& files,
                        const std::string& remoteprefix,
                        const std::string& url);

  bool SubmitUsingXMLRPC(const std::string& localprefix,
                       const std::set<std::string>& files,
                       const std::string& remoteprefix,
                       const std::string& url);

  typedef std::vector<char> cmCTestSubmitHandlerVectorOfChar;

  void ParseResponse(cmCTestSubmitHandlerVectorOfChar chunk);

  std::string GetSubmitResultsPrefix();

  class         ResponseParser;
  std::string   HTTPProxy;
  int           HTTPProxyType;
  std::string   HTTPProxyAuth;
  std::string   FTPProxy;
  int           FTPProxyType;
  std::ostream* LogFile;
  bool SubmitPart[cmCTest::PartCount];
  bool CDash;
  bool HasWarnings;
  bool HasErrors;
  cmCTest::SetOfStrings Files;
};

#endif
