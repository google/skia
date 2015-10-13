/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2015 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestCurl_h
#define cmCTestCurl_h

#include "cmStandardIncludes.h"

#include "cm_curl.h"

class cmCTest;

class cmCTestCurl
{
public:
  cmCTestCurl(cmCTest*);
  ~cmCTestCurl();
  bool UploadFile(std::string const& url,
                  std::string const& file,
                  std::string const& fields,
                  std::string& response);
  bool HttpRequest(std::string const& url,
                   std::string const& fields,
                   std::string& response);
  // currently only supports CURLOPT_SSL_VERIFYPEER_OFF
  // and CURLOPT_SSL_VERIFYHOST_OFF
  void SetCurlOptions(std::vector<std::string> const& args);
  void SetUseHttp10On() { this->UseHttp10 = true;}
  void SetTimeOutSeconds(int s) { this->TimeOutSeconds = s;}
  std::string Escape(std::string const& source);
protected:
  void SetProxyType();
  bool InitCurl();
private:
  cmCTest* CTest;
  CURL* Curl;
  std::string HTTPProxyAuth;
  std::string HTTPProxy;
  curl_proxytype HTTPProxyType;
  bool VerifyHostOff;
  bool VerifyPeerOff;
  bool UseHttp10;
  int TimeOutSeconds;
};

#endif
