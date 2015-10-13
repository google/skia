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

#include "foo.h"

#include <stdio.h>

class FooFoo : public QObject
{
  Q_OBJECT
  public:
    FooFoo():QObject() {}
  public slots:
    int getValue() const { return 12; }
};

Foo::Foo()
:QObject()
{
}


void Foo::doFoo()
{
  FooFoo ff;
  printf("Hello automoc: %d\n", ff.getValue());
}

#include "foo.moc"
