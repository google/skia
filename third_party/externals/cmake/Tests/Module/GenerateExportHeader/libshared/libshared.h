
#ifndef LIBSHARED_H
#define LIBSHARED_H

#include "libshared_export.h"

class LIBSHARED_EXPORT Libshared {
public:
  int libshared() const;

  int libshared_exported() const;

  int LIBSHARED_DEPRECATED libshared_deprecated() const;

  int libshared_not_exported() const;

  int LIBSHARED_NO_EXPORT libshared_excluded() const;
};

class LibsharedNotExported {
public:
  int libshared() const;

  int LIBSHARED_EXPORT libshared_exported() const;

  int LIBSHARED_DEPRECATED_EXPORT libshared_deprecated() const;

  int libshared_not_exported() const;

  int LIBSHARED_NO_EXPORT libshared_excluded() const;
};

class LIBSHARED_NO_EXPORT LibsharedExcluded {
public:
  int libshared() const;

  int LIBSHARED_EXPORT libshared_exported() const;

  int LIBSHARED_DEPRECATED_EXPORT libshared_deprecated() const;

  int libshared_not_exported() const;

  int LIBSHARED_NO_EXPORT libshared_excluded() const;
};

LIBSHARED_EXPORT int libshared_exported();

LIBSHARED_DEPRECATED_EXPORT int libshared_deprecated();

int libshared_not_exported();

int LIBSHARED_NO_EXPORT libshared_excluded();

#endif
