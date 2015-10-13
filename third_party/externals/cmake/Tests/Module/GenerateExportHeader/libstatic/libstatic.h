
#ifndef LIBSTATIC_H
#define LIBSTATIC_H

#include "libstatic_export.h"

class LIBSTATIC_EXPORT Libstatic {
public:
  int libstatic() const;

  int libstatic_exported() const;

  int LIBSTATIC_DEPRECATED libstatic_deprecated() const;

  int libstatic_not_exported() const;

  int LIBSTATIC_NO_EXPORT libstatic_excluded() const;
};

class LibstaticNotExported {
public:
  int libstatic() const;

  int LIBSTATIC_EXPORT libstatic_exported() const;

  int LIBSTATIC_DEPRECATED libstatic_deprecated() const;

  int libstatic_not_exported() const;

  int LIBSTATIC_NO_EXPORT libstatic_excluded() const;
};

class LIBSTATIC_NO_EXPORT LibstaticExcluded {
public:
  int libstatic() const;

  int LIBSTATIC_EXPORT libstatic_exported() const;

  int LIBSTATIC_DEPRECATED libstatic_deprecated() const;

  int libstatic_not_exported() const;

  int LIBSTATIC_NO_EXPORT libstatic_excluded() const;
};

LIBSTATIC_EXPORT int libstatic_exported();

LIBSTATIC_DEPRECATED_EXPORT int libstatic_deprecated();

int libstatic_not_exported();

int LIBSTATIC_NO_EXPORT libstatic_excluded();

#endif
