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

#ifndef ABC_P_H
#define ABC_P_H

#include <QObject>

#include <stdio.h>

class AbcP : public QObject
{
  Q_OBJECT
  public:
    AbcP() {}
  public slots:
    void doAbcP() { printf("I am private abc !\n"); }
};

#endif
