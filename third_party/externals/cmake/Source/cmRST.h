/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2013 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef _cmRST_h
#define _cmRST_h

#include "cmStandardIncludes.h"

#include <cmsys/RegularExpression.hxx>

/** \class cmRST
 * \brief Perform basic .rst processing for command-line help
 *
 * This class implements a subset of reStructuredText and Sphinx
 * document processing.  It is used to print command-line help.
 *
 * If you modify the capabilities of this class, be sure to update
 * the Help/manual/cmake-developer.7.rst documentation and to update
 * the Tests/CMakeLib/testRST.(rst|expect) test input and output.
 */
class cmRST
{
public:
  cmRST(std::ostream& os, std::string const& docroot);
  bool ProcessFile(std::string const& fname, bool isModule = false);
private:
  enum IncludeType
  {
    IncludeNormal,
    IncludeModule,
    IncludeTocTree
  };
  enum MarkupType
  {
    MarkupNone,
    MarkupNormal,
    MarkupEmpty
  };
  enum DirectiveType
  {
    DirectiveNone,
    DirectiveParsedLiteral,
    DirectiveLiteralBlock,
    DirectiveCodeBlock,
    DirectiveReplace,
    DirectiveTocTree
  };

  void ProcessRST(std::istream& is);
  void ProcessModule(std::istream& is);
  void Reset();
  void ProcessLine(std::string const& line);
  void NormalLine(std::string const& line);
  void OutputLine(std::string const& line, bool inlineMarkup);
  std::string ReplaceSubstitutions(std::string const& line);
  void OutputMarkupLines(bool inlineMarkup);
  bool ProcessInclude(std::string file, IncludeType type);
  void ProcessDirectiveParsedLiteral();
  void ProcessDirectiveLiteralBlock();
  void ProcessDirectiveCodeBlock();
  void ProcessDirectiveReplace();
  void ProcessDirectiveTocTree();
  static void UnindentLines(std::vector<std::string>& lines);

  std::ostream& OS;
  std::string DocRoot;
  int IncludeDepth;
  bool OutputLinePending;
  bool LastLineEndedInColonColon;
  MarkupType Markup;
  DirectiveType Directive;
  cmsys::RegularExpression CMakeDirective;
  cmsys::RegularExpression CMakeModuleDirective;
  cmsys::RegularExpression ParsedLiteralDirective;
  cmsys::RegularExpression CodeBlockDirective;
  cmsys::RegularExpression ReplaceDirective;
  cmsys::RegularExpression IncludeDirective;
  cmsys::RegularExpression TocTreeDirective;
  cmsys::RegularExpression ProductionListDirective;
  cmsys::RegularExpression NoteDirective;
  cmsys::RegularExpression ModuleRST;
  cmsys::RegularExpression CMakeRole;
  cmsys::RegularExpression Substitution;

  std::vector<std::string> MarkupLines;
  std::string DocDir;
  std::map<std::string, std::string> Replace;
  std::set<std::string> Replaced;
  std::string ReplaceName;
};

#endif
