/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmDependsJavaParserHelper_h
#define cmDependsJavaParserHelper_h

#include "cmStandardIncludes.h"

#define YYSTYPE cmDependsJavaParserHelper::ParserType
#define YYSTYPE_IS_DECLARED
#define YY_EXTRA_TYPE cmDependsJavaParserHelper*
#define YY_DECL int cmDependsJava_yylex(YYSTYPE* yylvalp, yyscan_t yyscanner)

/** \class cmDependsJavaParserHelper
 * \brief Helper class for parsing java source files
 *
 * Finds dependencies for java file and list of outputs
 */

class cmDependsJavaParserHelper
{
public:
  typedef struct {
    char* str;
  } ParserType;

  cmDependsJavaParserHelper();
  ~cmDependsJavaParserHelper();

  int ParseString(const char* str, int verb);
  int ParseFile(const char* file);

  // For the lexer:
  void AllocateParserType(cmDependsJavaParserHelper::ParserType* pt,
    const char* str, int len = 0);

  int LexInput(char* buf, int maxlen);
  void Error(const char* str);

  // For yacc
  void AddClassFound(const char* sclass);
  void PrepareElement(ParserType* opt);
  void DeallocateParserType(char** pt);
  void CheckEmpty(int line, int cnt, ParserType* pt);
  void StartClass(const char* cls);
  void EndClass();
  void AddPackagesImport(const char* sclass);
  void SetCurrentPackage(const char* pkg) { this->CurrentPackage = pkg; }
  const char* GetCurrentPackage() { return this->CurrentPackage.c_str(); }
  void SetCurrentCombine(const char* cmb) { this->CurrentCombine = cmb; }
  const char* GetCurrentCombine() { return this->CurrentCombine.c_str(); }
  void UpdateCombine(const char* str1, const char* str2);

  std::vector<std::string>& GetClassesFound() { return this->ClassesFound; }

  std::vector<std::string> GetFilesProduced();

private:
  class CurrentClass
  {
  public:
    std::string Name;
    std::vector<CurrentClass>* NestedClasses;
    CurrentClass()
      {
        this->NestedClasses = new std::vector<CurrentClass>;
      }
    ~CurrentClass()
      {
        delete this->NestedClasses;
      }
    CurrentClass& operator=(CurrentClass const& c)
      {
        this->NestedClasses->clear();
        this->Name = c.Name;
        std::copy(
          c.NestedClasses->begin(),
          c.NestedClasses->end(),
          std::back_inserter(
            *this->NestedClasses)
          );
        return *this;
      }
    CurrentClass(CurrentClass const& c)
      {
        (*this) = c;
      }
    void AddFileNamesForPrinting(std::vector<std::string> *files,
                                 const char* prefix, const char* sep);
  };
  std::string CurrentPackage;
  std::string::size_type InputBufferPos;
  std::string InputBuffer;
  std::vector<char> OutputBuffer;
  std::vector<std::string> ClassesFound;
  std::vector<std::string> PackagesImport;
  std::string CurrentCombine;

  std::vector<CurrentClass> ClassStack;

  int CurrentLine;
  int UnionsAvailable;
  int LastClassId;
  int CurrentDepth;
  int Verbose;

  std::vector<char*> Allocates;

  void PrintClasses();

  void Print(const char* place, const char* str);
  void CombineUnions(char** out, const char* in1, char** in2,
                     const char* sep);
  void SafePrintMissing(const char* str, int line, int cnt);

  void CleanupParser();
};

#endif

