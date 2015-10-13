/*============================================================================
  KWSys - Kitware System Library
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifdef _WIN32
#define DL_EXPORT __declspec( dllexport )
#else
#define DL_EXPORT
#endif

DL_EXPORT int TestDynamicLoaderData = 0;

DL_EXPORT void TestDynamicLoaderSymbolPointer()
{
}
