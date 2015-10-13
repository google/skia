/*============================================================================
  KWSys - Kitware System Library
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int testFail(int argc, char* argv[])
{
  char* env = getenv("DASHBOARD_TEST_FROM_CTEST");
  int oldCtest = 0;
  if(env)
    {
    if(strcmp(env, "1") == 0)
      {
      oldCtest = 1;
      }
    printf("DASHBOARD_TEST_FROM_CTEST = %s\n", env);
    }
  printf("%s: This test intentionally fails\n", argv[0]);
  if(oldCtest)
    {
    printf("The version of ctest is not able to handle intentionally failing tests, so pass.\n");
    return 0;
    }
  return argc;
}
