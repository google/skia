
#ifndef SHARED_AND_STATIC_H
#define SHARED_AND_STATIC_H

#include "libshared_and_static_export.h"

class MYPREFIX_LIBSHARED_AND_STATIC_EXPORT LibsharedAndStatic {
public:
  int libshared_and_static() const;

  int libshared_and_static_exported() const;

  int MYPREFIX_LIBSHARED_AND_STATIC_DEPRECATED libshared_and_static_deprecated() const;

  int libshared_and_static_not_exported() const;

  int MYPREFIX_LIBSHARED_AND_STATIC_NO_EXPORT libshared_and_static_excluded() const;
};

class LibsharedAndStaticNotExported {
public:
  int libshared_and_static() const;

  int MYPREFIX_LIBSHARED_AND_STATIC_EXPORT libshared_and_static_exported() const;

  int MYPREFIX_LIBSHARED_AND_STATIC_DEPRECATED libshared_and_static_deprecated() const;

  int libshared_and_static_not_exported() const;

  int MYPREFIX_LIBSHARED_AND_STATIC_NO_EXPORT libshared_and_static_excluded() const;
};

class MYPREFIX_LIBSHARED_AND_STATIC_NO_EXPORT LibsharedAndStaticExcluded {
public:
  int libshared_and_static() const;

  int MYPREFIX_LIBSHARED_AND_STATIC_EXPORT libshared_and_static_exported() const;

  int MYPREFIX_LIBSHARED_AND_STATIC_DEPRECATED libshared_and_static_deprecated() const;

  int libshared_and_static_not_exported() const;

  int MYPREFIX_LIBSHARED_AND_STATIC_NO_EXPORT libshared_and_static_excluded() const;
};

MYPREFIX_LIBSHARED_AND_STATIC_EXPORT int libshared_and_static_exported();

MYPREFIX_LIBSHARED_AND_STATIC_DEPRECATED_EXPORT int libshared_and_static_deprecated();

int libshared_and_static_not_exported();

int MYPREFIX_LIBSHARED_AND_STATIC_NO_EXPORT libshared_and_static_excluded();

#endif
