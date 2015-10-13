/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmExprParserHelper.h"

#include "cmSystemTools.h"
#include "cmMakefile.h"

#include "cmExprLexer.h"

int cmExpr_yyparse( yyscan_t yyscanner );
//
cmExprParserHelper::cmExprParserHelper()
{
  this->FileLine = -1;
  this->FileName = 0;
}


cmExprParserHelper::~cmExprParserHelper()
{
  this->CleanupParser();
}

int cmExprParserHelper::ParseString(const char* str, int verb)
{
  if ( !str)
    {
    return 0;
    }
  //printf("Do some parsing: %s\n", str);

  this->Verbose = verb;
  this->InputBuffer = str;
  this->InputBufferPos = 0;
  this->CurrentLine = 0;

  this->Result = 0;

  yyscan_t yyscanner;
  cmExpr_yylex_init(&yyscanner);
  cmExpr_yyset_extra(this, yyscanner);
  int res = cmExpr_yyparse(yyscanner);
  cmExpr_yylex_destroy(yyscanner);
  if ( res != 0 )
    {
    //str << "CAL_Parser returned: " << res << std::endl;
    //std::cerr << "When parsing: [" << str << "]" << std::endl;
    return 0;
    }

  this->CleanupParser();

  if ( Verbose )
    {
    std::cerr << "Expanding [" << str << "] produced: ["
              << this->Result << "]" << std::endl;
    }
  return 1;
}

void cmExprParserHelper::CleanupParser()
{
}

int cmExprParserHelper::LexInput(char* buf, int maxlen)
{
  //std::cout << "JPLexInput ";
  //std::cout.write(buf, maxlen);
  //std::cout << std::endl;
  if ( maxlen < 1 )
    {
    return 0;
    }
  if ( this->InputBufferPos < this->InputBuffer.size() )
    {
    buf[0] = this->InputBuffer[ this->InputBufferPos++ ];
    if ( buf[0] == '\n' )
      {
      this->CurrentLine ++;
      }
    return(1);
    }
  else
    {
    buf[0] = '\n';
    return( 0 );
    }
}

void cmExprParserHelper::Error(const char* str)
{
  unsigned long pos = static_cast<unsigned long>(this->InputBufferPos);
  std::ostringstream ostr;
  ostr << str << " (" << pos << ")";
  this->ErrorString = ostr.str();
}

void cmExprParserHelper::SetResult(int value)
{
  this->Result = value;
}


