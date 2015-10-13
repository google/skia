/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cm_xmlrpc_h
#define cm_xmlrpc_h

/* Use the xmlrpc library configured for CMake.  */
#include "cmThirdParty.h"
#ifdef CTEST_USE_XMLRPC
# include <xmlrpc.h>
# include <xmlrpc_client.h>
#endif

#endif
