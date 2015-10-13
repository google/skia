/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2015 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCTestCurl.h"

#include "cmSystemTools.h"
#include "cmCTest.h"

cmCTestCurl::cmCTestCurl(cmCTest* ctest)
{
  this->CTest = ctest;
  this->SetProxyType();
  this->UseHttp10 = false;
  // In windows, this will init the winsock stuff
  ::curl_global_init(CURL_GLOBAL_ALL);
  // default is to verify https
  this->VerifyPeerOff = false;
  this->VerifyHostOff = false;
  this->TimeOutSeconds = 0;
  this->Curl = curl_easy_init();
}

cmCTestCurl::~cmCTestCurl()
{
  ::curl_easy_cleanup(this->Curl);
  ::curl_global_cleanup();
}

std::string cmCTestCurl::Escape(std::string const& source)
{
  char* data1 = curl_easy_escape(this->Curl, source.c_str(), 0);
  std::string ret = data1;
  curl_free(data1);
  return ret;
}

namespace
{
static size_t
curlWriteMemoryCallback(void *ptr, size_t size, size_t nmemb,
  void *data)
{
  int realsize = (int)(size * nmemb);

  std::vector<char> *vec
    = static_cast<std::vector<char>* >(data);
  const char* chPtr = static_cast<char*>(ptr);
  vec->insert(vec->end(), chPtr, chPtr + realsize);
  return realsize;
}

static size_t
curlDebugCallback(CURL *, curl_infotype, char *chPtr,
  size_t size, void *data)
{
  std::vector<char> *vec
    = static_cast<std::vector<char>* >(data);
  vec->insert(vec->end(), chPtr, chPtr + size);

  return size;
}

}

void cmCTestCurl::SetCurlOptions(std::vector<std::string> const& args)
{
  for( std::vector<std::string>::const_iterator i = args.begin();
       i != args.end(); ++i)
    {
    if(*i == "CURLOPT_SSL_VERIFYPEER_OFF")
      {
      this->VerifyPeerOff = true;
      }
    if(*i == "CURLOPT_SSL_VERIFYHOST_OFF")
      {
      this->VerifyHostOff = true;
      }
    }
}

bool cmCTestCurl::InitCurl()
{
  if(!this->Curl)
    {
    return false;
    }
  if(this->VerifyPeerOff)
    {
    curl_easy_setopt(this->Curl, CURLOPT_SSL_VERIFYPEER, 0);
    }
  if(this->VerifyHostOff)
    {
    curl_easy_setopt(this->Curl, CURLOPT_SSL_VERIFYHOST, 0);
    }
  if(this->HTTPProxy.size())
    {
    curl_easy_setopt(this->Curl, CURLOPT_PROXY, this->HTTPProxy.c_str());
    curl_easy_setopt(this->Curl, CURLOPT_PROXYTYPE, this->HTTPProxyType);
    if (this->HTTPProxyAuth.size() > 0)
      {
      curl_easy_setopt(this->Curl, CURLOPT_PROXYUSERPWD,
                       this->HTTPProxyAuth.c_str());
      }
    }
  if(this->UseHttp10)
    {
    curl_easy_setopt(this->Curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
    }
  // enable HTTP ERROR parsing
  curl_easy_setopt(this->Curl, CURLOPT_FAILONERROR, 1);
  return true;
}


bool cmCTestCurl::UploadFile(std::string const& local_file,
                             std::string const& url,
                             std::string const& fields,
                             std::string& response)
{
  response = "";
  if(!this->InitCurl())
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE, "Initialization of curl failed");
    return false;
    }
  /* enable uploading */
  curl_easy_setopt(this->Curl, CURLOPT_UPLOAD, 1);
  // if there is little to no activity for too long stop submitting
  if(this->TimeOutSeconds)
    {
    ::curl_easy_setopt(this->Curl, CURLOPT_LOW_SPEED_LIMIT, 1);
    ::curl_easy_setopt(this->Curl, CURLOPT_LOW_SPEED_TIME,
                       this->TimeOutSeconds);
    }
  /* HTTP PUT please */
  ::curl_easy_setopt(this->Curl, CURLOPT_PUT, 1);
  ::curl_easy_setopt(this->Curl, CURLOPT_VERBOSE, 1);

  FILE* ftpfile = cmsys::SystemTools::Fopen(local_file, "rb");
  if(!ftpfile)
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Could not open file for upload: " << local_file << "\n");
    return false;
    }
  // set the url
  std::string upload_url = url;
  upload_url += "?";
  upload_url += fields;
  ::curl_easy_setopt(this->Curl, CURLOPT_URL, upload_url.c_str());
  // now specify which file to upload
  ::curl_easy_setopt(this->Curl, CURLOPT_INFILE, ftpfile);
  unsigned long filelen = cmSystemTools::FileLength(local_file);
  // and give the size of the upload (optional)
  ::curl_easy_setopt(this->Curl, CURLOPT_INFILESIZE,
                     static_cast<long>(filelen));
  ::curl_easy_setopt(this->Curl, CURLOPT_WRITEFUNCTION,
                     curlWriteMemoryCallback);
  ::curl_easy_setopt(this->Curl, CURLOPT_DEBUGFUNCTION,
                     curlDebugCallback);
  std::vector<char> responseData;
  std::vector<char> debugData;
  ::curl_easy_setopt(this->Curl, CURLOPT_FILE, (void *)&responseData);
  ::curl_easy_setopt(this->Curl, CURLOPT_DEBUGDATA, (void *)&debugData);
  ::curl_easy_setopt(this->Curl, CURLOPT_FAILONERROR, 1);
  // Now run off and do what you've been told!
  ::curl_easy_perform(this->Curl);
  ::fclose(ftpfile);

  if ( responseData.size() > 0 )
    {
    response = std::string(responseData.begin(), responseData.end());
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
               "Curl response: [" << response << "]\n");
    }
  std::string curlDebug;
  if ( debugData.size() > 0 )
    {
    curlDebug = std::string(debugData.begin(), debugData.end());
    cmCTestLog(this->CTest, DEBUG, "Curl debug: [" << curlDebug << "]\n");
    }
  if(response.size() == 0)
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE, "No response from server.\n" <<
      curlDebug);
    return false;
    }
  return true;
}

bool cmCTestCurl::HttpRequest(std::string const& url,
                              std::string const& fields,
                              std::string& response)
{
  response = "";
  cmCTestLog(this->CTest, DEBUG, "HttpRequest\n"
             << "url: " << url << "\n"
             << "fields " << fields << "\n");
  if(!this->InitCurl())
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE, "Initialization of curl failed");
    return false;
    }
  curl_easy_setopt(this->Curl, CURLOPT_POST, 1);
  curl_easy_setopt(this->Curl, CURLOPT_POSTFIELDS, fields.c_str());
  ::curl_easy_setopt(this->Curl, CURLOPT_URL, url.c_str());
  ::curl_easy_setopt(this->Curl, CURLOPT_FOLLOWLOCATION, 1);
  //set response options
  ::curl_easy_setopt(this->Curl, CURLOPT_WRITEFUNCTION,
                     curlWriteMemoryCallback);
  ::curl_easy_setopt(this->Curl, CURLOPT_DEBUGFUNCTION,
        curlDebugCallback);
  std::vector<char> responseData;
  std::vector<char> debugData;
  ::curl_easy_setopt(this->Curl, CURLOPT_FILE, (void *)&responseData);
  ::curl_easy_setopt(this->Curl, CURLOPT_DEBUGDATA, (void *)&debugData);
  ::curl_easy_setopt(this->Curl, CURLOPT_FAILONERROR, 1);

  CURLcode res = ::curl_easy_perform(this->Curl);

  if ( responseData.size() > 0 )
    {
    response = std::string(responseData.begin(), responseData.end());
    cmCTestLog(this->CTest, DEBUG, "Curl response: [" << response << "]\n");
    }
  if ( debugData.size() > 0 )
    {
    std::string curlDebug = std::string(debugData.begin(), debugData.end());
    cmCTestLog(this->CTest, DEBUG, "Curl debug: [" << curlDebug << "]\n");
    }
  cmCTestLog(this->CTest, DEBUG, "Curl res: " << res << "\n");
  return (res == 0);
}

void cmCTestCurl::SetProxyType()
{
  if ( cmSystemTools::GetEnv("HTTP_PROXY") )
    {
    this->HTTPProxy = cmSystemTools::GetEnv("HTTP_PROXY");
    if ( cmSystemTools::GetEnv("HTTP_PROXY_PORT") )
      {
      this->HTTPProxy += ":";
      this->HTTPProxy += cmSystemTools::GetEnv("HTTP_PROXY_PORT");
      }
    if ( cmSystemTools::GetEnv("HTTP_PROXY_TYPE") )
      {
      // this is the default
      this->HTTPProxyType = CURLPROXY_HTTP;
      std::string type = cmSystemTools::GetEnv("HTTP_PROXY_TYPE");
      // HTTP/SOCKS4/SOCKS5
      if ( type == "HTTP" )
        {
        this->HTTPProxyType = CURLPROXY_HTTP;
        }
      else if ( type == "SOCKS4" )
        {
        this->HTTPProxyType = CURLPROXY_SOCKS4;
        }
      else if ( type == "SOCKS5" )
        {
        this->HTTPProxyType = CURLPROXY_SOCKS5;
        }
      }
    if ( cmSystemTools::GetEnv("HTTP_PROXY_USER") )
      {
      this->HTTPProxyAuth = cmSystemTools::GetEnv("HTTP_PROXY_USER");
      }
    if ( cmSystemTools::GetEnv("HTTP_PROXY_PASSWD") )
      {
      this->HTTPProxyAuth += ":";
      this->HTTPProxyAuth += cmSystemTools::GetEnv("HTTP_PROXY_PASSWD");
      }
    }
}
