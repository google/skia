
#ifndef LIBC_H
#define LIBC_H

#include "libc_export.h"

#include <QObject>
#include "libB.h"

class LIBC_EXPORT LibC : public QObject
{
  Q_OBJECT
public:
  explicit LibC(QObject *parent = 0);


  int foo();
private:
  LibB b;
};

#endif
