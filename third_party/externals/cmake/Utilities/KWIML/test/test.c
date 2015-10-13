/*============================================================================
  Kitware Information Macro Library
  Copyright 2010-2011 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifdef __cplusplus
extern "C" {
#endif
extern int test_ABI_C(void);
extern int test_INT_C(void);
extern int test_ABI_CXX(void);
extern int test_INT_CXX(void);
extern int test_include_C(void);
extern int test_include_CXX(void);
#ifdef __cplusplus
} // extern "C"
#endif

int main(void)
{
  int result = 1;
#ifdef KWIML_LANGUAGE_C
  result = test_ABI_C() && result;
  result = test_INT_C() && result;
  result = test_include_C() && result;
#endif
#ifdef KWIML_LANGUAGE_CXX
  result = test_ABI_CXX() && result;
  result = test_INT_CXX() && result;
  result = test_include_CXX() && result;
#endif
  return result? 0 : 1;
}
