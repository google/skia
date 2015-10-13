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
#include KWSYS_HEADER(CommandLineArguments.hxx)
#include KWSYS_HEADER(ios/iostream)
#include KWSYS_HEADER(stl/vector)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "CommandLineArguments.hxx.in"
# include "kwsys_ios_iostream.h.in"
#endif

#include <stddef.h> /* size_t */
#include <string.h> /* strcmp */

static void* random_ptr = reinterpret_cast<void*>(0x123);

static int argument(const char* arg, const char* value, void* call_data)
{
  kwsys_ios::cout << "Got argument: \"" << arg << "\" value: \"" << (value?value:"(null)") << "\"" << kwsys_ios::endl;
  if ( call_data != random_ptr )
    {
    kwsys_ios::cerr << "Problem processing call_data" << kwsys_ios::endl;
    return 0;
    }
  return 1;
}

static int unknown_argument(const char* argument, void* call_data)
{
  kwsys_ios::cout << "Got unknown argument: \"" << argument << "\"" << kwsys_ios::endl;
  if ( call_data != random_ptr )
    {
    kwsys_ios::cerr << "Problem processing call_data" << kwsys_ios::endl;
    return 0;
    }
  return 1;
}

static bool CompareTwoItemsOnList(bool i1, bool i2) { return i1 == i2; }
static bool CompareTwoItemsOnList(int i1, int i2) { return i1 == i2; }
static bool CompareTwoItemsOnList(double i1, double i2) { return i1 == i2; }
static bool CompareTwoItemsOnList(const char* i1,
  const char* i2) { return strcmp(i1, i2) == 0; }
static bool CompareTwoItemsOnList(const kwsys_stl::string& i1,
  const kwsys_stl::string& i2) { return i1 == i2; }

int testCommandLineArguments(int argc, char* argv[])
{
  // Example run: ./testCommandLineArguments --some-int-variable 4
  // --another-bool-variable --some-bool-variable=yes
  // --some-stl-string-variable=foobar --set-bool-arg1 --set-bool-arg2
  // --some-string-variable=hello

  int res = 0;
  kwsys::CommandLineArguments arg;
  arg.Initialize(argc, argv);

  // For error handling
  arg.SetClientData(random_ptr);
  arg.SetUnknownArgumentCallback(unknown_argument);

  int some_int_variable = 10;
  double some_double_variable = 10.10;
  char* some_string_variable = 0;
  kwsys_stl::string some_stl_string_variable = "";
  bool some_bool_variable = false;
  bool some_bool_variable1 = false;
  bool bool_arg1 = false;
  int bool_arg2 = 0;

  kwsys_stl::vector<int> numbers_argument;
  int valid_numbers[] = { 5, 1, 8, 3, 7, 1, 3, 9, 7, 1 };

  kwsys_stl::vector<double> doubles_argument;
  double valid_doubles[] = { 12.5, 1.31, 22 };

  kwsys_stl::vector<bool> bools_argument;
  bool valid_bools[] = { true, true, false };

  kwsys_stl::vector<char*> strings_argument;
  const char* valid_strings[] = { "andy", "bill", "brad", "ken" };

  kwsys_stl::vector<kwsys_stl::string> stl_strings_argument;
  kwsys_stl::string valid_stl_strings[] = { "ken", "brad", "bill", "andy" };

  typedef kwsys::CommandLineArguments argT;

  arg.AddArgument("--some-int-variable", argT::SPACE_ARGUMENT, &some_int_variable, "Set some random int variable");
  arg.AddArgument("--some-double-variable", argT::CONCAT_ARGUMENT, &some_double_variable, "Set some random double variable");
  arg.AddArgument("--some-string-variable", argT::EQUAL_ARGUMENT, &some_string_variable, "Set some random string variable");
  arg.AddArgument("--some-stl-string-variable", argT::EQUAL_ARGUMENT, &some_stl_string_variable, "Set some random stl string variable");
  arg.AddArgument("--some-bool-variable", argT::EQUAL_ARGUMENT, &some_bool_variable, "Set some random bool variable");
  arg.AddArgument("--another-bool-variable", argT::NO_ARGUMENT, &some_bool_variable1, "Set some random bool variable 1");
  arg.AddBooleanArgument("--set-bool-arg1", &bool_arg1, "Test AddBooleanArgument 1");
  arg.AddBooleanArgument("--set-bool-arg2", &bool_arg2, "Test AddBooleanArgument 2");
  arg.AddArgument("--some-multi-argument", argT::MULTI_ARGUMENT, &numbers_argument, "Some multiple values variable");
  arg.AddArgument("-N", argT::SPACE_ARGUMENT, &doubles_argument, "Some explicit multiple values variable");
  arg.AddArgument("-BB", argT::CONCAT_ARGUMENT, &bools_argument, "Some explicit multiple values variable");
  arg.AddArgument("-SS", argT::EQUAL_ARGUMENT, &strings_argument, "Some explicit multiple values variable");
  arg.AddArgument("-SSS", argT::MULTI_ARGUMENT, &stl_strings_argument, "Some explicit multiple values variable");

  arg.AddCallback("-A", argT::NO_ARGUMENT, argument, random_ptr, "Some option -A. This option has a multiline comment. It should demonstrate how the code splits lines.");
  arg.AddCallback("-B", argT::SPACE_ARGUMENT, argument, random_ptr, "Option -B takes argument with space");
  arg.AddCallback("-C", argT::EQUAL_ARGUMENT, argument, random_ptr, "Option -C takes argument after =");
  arg.AddCallback("-D", argT::CONCAT_ARGUMENT, argument, random_ptr, "This option takes concatinated argument");
  arg.AddCallback("--long1", argT::NO_ARGUMENT, argument, random_ptr, "-A");
  arg.AddCallback("--long2", argT::SPACE_ARGUMENT, argument, random_ptr, "-B");
  arg.AddCallback("--long3", argT::EQUAL_ARGUMENT, argument, random_ptr, "Same as -C but a bit different");
  arg.AddCallback("--long4", argT::CONCAT_ARGUMENT, argument, random_ptr, "-C");

  if ( !arg.Parse() )
    {
    kwsys_ios::cerr << "Problem parsing arguments" << kwsys_ios::endl;
    res = 1;
    }
  kwsys_ios::cout << "Help: " << arg.GetHelp() << kwsys_ios::endl;

  kwsys_ios::cout << "Some int variable was set to: " << some_int_variable << kwsys_ios::endl;
  kwsys_ios::cout << "Some double variable was set to: " << some_double_variable << kwsys_ios::endl;
  if ( some_string_variable && strcmp(some_string_variable, "test string with space") == 0)
    {
    kwsys_ios::cout << "Some string variable was set to: " << some_string_variable << kwsys_ios::endl;
    delete [] some_string_variable;
    }
  else
    {
    kwsys_ios::cerr << "Problem setting string variable" << kwsys_ios::endl;
    res = 1;
    }
  size_t cc;
#define CompareTwoLists(list1, list_valid, lsize) \
  if ( list1.size() != lsize ) \
    { \
    kwsys_ios::cerr << "Problem setting " #list1 ". Size is: " << list1.size() \
     << " should be: " << lsize << kwsys_ios::endl; \
    res = 1; \
    } \
  else \
    { \
    kwsys_ios::cout << #list1 " argument set:"; \
    for ( cc =0; cc < lsize; ++ cc ) \
      { \
      kwsys_ios::cout << " " << list1[cc]; \
      if ( !CompareTwoItemsOnList(list1[cc], list_valid[cc]) ) \
        { \
        kwsys_ios::cerr << "Problem setting " #list1 ". Value of " \
        << cc << " is: [" << list1[cc] << "] <> [" \
        << list_valid[cc] << "]" << kwsys_ios::endl; \
        res = 1; \
        break; \
        } \
      } \
    kwsys_ios::cout << kwsys_ios::endl; \
    }

  CompareTwoLists(numbers_argument, valid_numbers, 10);
  CompareTwoLists(doubles_argument, valid_doubles, 3);
  CompareTwoLists(bools_argument, valid_bools, 3);
  CompareTwoLists(strings_argument, valid_strings, 4);
  CompareTwoLists(stl_strings_argument, valid_stl_strings, 4);

  kwsys_ios::cout << "Some STL String variable was set to: " << some_stl_string_variable << kwsys_ios::endl;
  kwsys_ios::cout << "Some bool variable was set to: " << some_bool_variable << kwsys_ios::endl;
  kwsys_ios::cout << "Some bool variable was set to: " << some_bool_variable1 << kwsys_ios::endl;
  kwsys_ios::cout << "bool_arg1 variable was set to: " << bool_arg1 << kwsys_ios::endl;
  kwsys_ios::cout << "bool_arg2 variable was set to: " << bool_arg2 << kwsys_ios::endl;
  kwsys_ios::cout << kwsys_ios::endl;

  for ( cc = 0; cc < strings_argument.size(); ++ cc )
    {
    delete [] strings_argument[cc];
    strings_argument[cc] = 0;
    }
  return res;
}
