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

#include "blub.h"

#include <stdio.h>

class BlubBlub : public QObject
{
  Q_OBJECT
  public:
    BlubBlub():QObject() {}
  public slots:
    int getValue() const { return 13; }
};

Blub::Blub()
{
}


void Blub::blubber()
{
  BlubBlub bb;
  printf("Blub blub %d ! \n", bb.getValue());
}

// test the case that the wrong moc-file is included, it should
// actually be "blub.moc"
#include "moc_blub.cpp"
