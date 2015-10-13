/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmConditionEvaluator_h
#define cmConditionEvaluator_h

#include "cmCommand.h"
#include "cmExpandedCommandArgument.h"

#include <list>

class cmConditionEvaluator
{
public:
  typedef std::list<cmExpandedCommandArgument> cmArgumentList;

  cmConditionEvaluator(cmMakefile& makefile);

  // this is a shared function for both If and Else to determine if the
  // arguments were valid, and if so, was the response true. If there is
  // an error, the errorString will be set.
  bool IsTrue(const std::vector<cmExpandedCommandArgument> &args,
      std::string &errorString,
      cmake::MessageType &status);

private:
  // Filter the given variable definition based on policy CMP0054.
  const char* GetDefinitionIfUnquoted(
      const cmExpandedCommandArgument& argument) const;

  const char* GetVariableOrString(
      const cmExpandedCommandArgument& argument) const;

  bool IsKeyword(std::string const& keyword,
    cmExpandedCommandArgument& argument) const;

  bool GetBooleanValue(
    cmExpandedCommandArgument& arg) const;

  bool GetBooleanValueOld(
    cmExpandedCommandArgument const& arg, bool one) const;

  bool GetBooleanValueWithAutoDereference(
    cmExpandedCommandArgument &newArg,
    std::string &errorString,
    cmake::MessageType &status,
    bool oneArg = false) const;

  void IncrementArguments(
    cmArgumentList &newArgs,
    cmArgumentList::iterator &argP1,
    cmArgumentList::iterator &argP2) const;

  void HandlePredicate(bool value, int &reducible,
     cmArgumentList::iterator &arg,
     cmArgumentList &newArgs,
     cmArgumentList::iterator &argP1,
     cmArgumentList::iterator &argP2) const;

  void HandleBinaryOp(bool value, int &reducible,
     cmArgumentList::iterator &arg,
     cmArgumentList &newArgs,
     cmArgumentList::iterator &argP1,
     cmArgumentList::iterator &argP2);

  bool HandleLevel0(cmArgumentList &newArgs,
      std::string &errorString,
      cmake::MessageType &status);

  bool HandleLevel1(cmArgumentList &newArgs,
      std::string &, cmake::MessageType &);

  bool HandleLevel2(cmArgumentList &newArgs,
      std::string &errorString,
      cmake::MessageType &status);

  bool HandleLevel3(cmArgumentList &newArgs,
      std::string &errorString,
      cmake::MessageType &status);

  bool HandleLevel4(cmArgumentList &newArgs,
      std::string &errorString,
      cmake::MessageType &status);

  cmMakefile& Makefile;
  cmPolicies::PolicyStatus Policy12Status;
  cmPolicies::PolicyStatus Policy54Status;
  cmPolicies::PolicyStatus Policy57Status;
};

#endif
