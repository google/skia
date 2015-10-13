/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2011 Kitware, Inc.
  Copyright 2011 Alexander Neundorf (neundorf@kde.org)

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/


#include "yaf.h"
#include "yaf_p.h"

#include <stdio.h>

Yaf::Yaf()
{
}


void Yaf::doYaf()
{
  YafP yafP;
  yafP.doYafP();
}

// check that including a moc file from a private header the wrong way works:
#include "yaf_p.moc"
