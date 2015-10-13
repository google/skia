/*============================================================================
  KWSys - Kitware System Library
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
// Setup for tests that use result of stl namespace test.
#if defined(KWSYS_STL_HAVE_STD)
# if KWSYS_STL_HAVE_STD
#  define kwsys_stl std
# else
#  define kwsys_stl
# endif
#endif

// Setup for tests that use iostreams.
#if defined(KWSYS_IOS_USE_ANSI) && defined(KWSYS_IOS_HAVE_STD)
# if defined(_MSC_VER)
#  pragma warning (push,1)
# endif
# if KWSYS_IOS_USE_ANSI
#  include <iostream>
# else
#  include <iostream.h>
# endif
# if defined(_MSC_VER)
#  pragma warning (pop)
# endif
# if KWSYS_IOS_HAVE_STD
#  define kwsys_ios std
# else
#  define kwsys_ios
# endif
#endif

#ifdef TEST_KWSYS_STL_HAVE_STD
#include <list>
void f(std ::list<int>*) {}
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_IOS_USE_ANSI
#include <iosfwd>
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_IOS_HAVE_STD
#include <iosfwd>
void f(std ::ostream*) {}
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_IOS_USE_SSTREAM
#include <sstream>
#if defined(__GNUC__) && __GNUC__ == 2 && __GNUC_MINOR__ == 96
# error "GCC 2.96 stringstream is buggy"
#endif
int main()
{
  std ::ostringstream ostr;
  ostr << "hello";
  if(ostr.str().size() == 5)
    {
    return 0;
    }
  return -1;
}
#endif

#ifdef TEST_KWSYS_IOS_USE_STRSTREAM_H
#include <strstream.h>
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_IOS_USE_STRSTREA_H
#include <strstrea.h>
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_STL_STRING_HAVE_OSTREAM
# include <iostream.h>
# include <string>
void f(ostream& os, const kwsys_stl::string& s) { os << s; }
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_STL_STRING_HAVE_ISTREAM
# include <iostream.h>
# include <string>
void f(istream& is, kwsys_stl::string& s) { is >> s; }
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_STL_STRING_HAVE_NEQ_CHAR
# include <string>
bool f(const kwsys_stl::string& s) { return s != ""; }
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_CXX_HAS_CSTDIO
#include <cstdio>
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_CXX_HAS_CSTDDEF
#include <cstddef>
void f(size_t) {}
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_CXX_HAS_LONG_LONG
long long f(long long n) { return n; }
int main()
{
  long long n = 0;
  return static_cast<int>(f(n));
}
#endif

#ifdef TEST_KWSYS_CXX_HAS___INT64
__int64 f(__int64 n) { return n; }
int main()
{
  __int64 n = 0;
  return static_cast<int>(f(n));
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_NULL_TEMPLATE_ARGS
template <class T> class A;
template <class T> int f(A<T>&);
template <class T> class A
{
public:
  // "friend int f<>(A<T>&)" would conform
  friend int f(A<T>&);
private:
  int x;
};

template <class T> int f(A<T>& a) { return a.x = 0; }
template int f(A<int>&);

int main()
{
  A<int> a;
  return f(a);
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_MEMBER_TEMPLATES
template <class U>
class A
{
public:
  U u;
  A(): u(0) {}
  template <class V> V m(V* p) { return *p = u; }
};

int main()
{
  A<short> a;
  int s = 1;
  return a.m(&s);
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_FULL_SPECIALIZATION
template <class T> struct A {};
template <> struct A<int*>
{
  static int f() { return 0; }
};
int main() { return A<int*>::f(); }
#endif

#ifdef TEST_KWSYS_CXX_HAS_ARGUMENT_DEPENDENT_LOOKUP
namespace N
{
  class A {};
  int f(A*) { return 0; }
}
void f(void*);
int main()
{
  N::A* a = 0;
  return f(a);
}
#endif

#ifdef TEST_KWSYS_STL_HAS_ITERATOR_TRAITS
#include <iterator>
#include <list>
void f(kwsys_stl::iterator_traits<kwsys_stl::list<int>::iterator>::iterator_category const&) {}
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_STL_HAS_ITERATOR_CATEGORY
#include <iterator>
#include <list>
void f(kwsys_stl::list<int>::iterator x) { kwsys_stl::iterator_category(x); }
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_STL_HAS___ITERATOR_CATEGORY
#include <iterator>
#include <list>
void f(kwsys_stl::list<int>::iterator x) { kwsys_stl::__iterator_category(x); }
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_STL_HAS_ALLOCATOR_TEMPLATE
#include <memory>
template <class Alloc>
void f(const Alloc&)
{
  typedef typename Alloc::size_type alloc_size_type;
}
int main()
{
  f(kwsys_stl::allocator<char>());
  return 0;
}
#endif

#ifdef TEST_KWSYS_STL_HAS_ALLOCATOR_NONTEMPLATE
#include <memory>
void f(kwsys_stl::allocator::size_type const&) {}
int main() { return 0; }
#endif

#ifdef TEST_KWSYS_STL_HAS_ALLOCATOR_REBIND
#include <memory>
template <class T, class Alloc>
void f(const T&, const Alloc&)
{
  typedef typename Alloc::template rebind<T>::other alloc_type;
}
int main()
{
  f(0, kwsys_stl::allocator<char>());
  return 0;
}
#endif

#ifdef TEST_KWSYS_STL_HAS_ALLOCATOR_MAX_SIZE_ARGUMENT
#include <memory>
void f(kwsys_stl::allocator<char> const& a)
{
  a.max_size(sizeof(int));
}
int main()
{
  f(kwsys_stl::allocator<char>());
  return 0;
}
#endif

#ifdef TEST_KWSYS_STL_HAS_ALLOCATOR_OBJECTS
#include <vector>
void f(kwsys_stl::vector<int> const& v1)
{
  kwsys_stl::vector<int>(1, 1, v1.get_allocator());
}
int main()
{
  f(kwsys_stl::vector<int>());
  return 0;
}
#endif

#ifdef TEST_KWSYS_STAT_HAS_ST_MTIM
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
int main()
{
  struct stat stat1;
  (void)stat1.st_mtim.tv_sec;
  (void)stat1.st_mtim.tv_nsec;
  return 0;
}
#endif

#ifdef TEST_KWSYS_CXX_SAME_LONG_AND___INT64
void function(long**) {}
int main()
{
  __int64** p = 0;
  function(p);
  return 0;
}
#endif

#ifdef TEST_KWSYS_CXX_SAME_LONG_LONG_AND___INT64
void function(long long**) {}
int main()
{
  __int64** p = 0;
  function(p);
  return 0;
}
#endif

#ifdef TEST_KWSYS_CAN_CONVERT_UI64_TO_DOUBLE
void function(double& l, unsigned __int64 const& r)
{
  l = static_cast<double>(r);
}

int main()
{
  double tTo = 0.0;
  unsigned __int64 tFrom = 0;
  function(tTo, tFrom);
  return 0;
}
#endif

#ifdef TEST_KWSYS_IOS_HAVE_BINARY
int test_binary(int, ...)
{
  return 0;
}
int main()
{
  return test_binary(1, kwsys_ios::ios::binary);
}
#endif

#ifdef TEST_KWSYS_IOS_HAS_ISTREAM_LONG_LONG
int test_istream(kwsys_ios::istream& is, long long& x)
{
  return (is >> x)? 1:0;
}
int main()
{
  long long x = 0;
  return test_istream(kwsys_ios::cin, x);
}
#endif

#ifdef TEST_KWSYS_IOS_HAS_OSTREAM_LONG_LONG
int test_ostream(kwsys_ios::ostream& os, long long x)
{
  return (os << x)? 1:0;
}
int main()
{
  long long x = 0;
  return test_ostream(kwsys_ios::cout, x);
}
#endif

#ifdef TEST_KWSYS_IOS_HAS_ISTREAM___INT64
int test_istream(kwsys_ios::istream& is, __int64& x)
{
  return (is >> x)? 1:0;
}
int main()
{
  __int64 x = 0;
  return test_istream(kwsys_ios::cin, x);
}
#endif

#ifdef TEST_KWSYS_IOS_HAS_OSTREAM___INT64
int test_ostream(kwsys_ios::ostream& os, __int64 x)
{
  return (os << x)? 1:0;
}
int main()
{
  __int64 x = 0;
  return test_ostream(kwsys_ios::cout, x);
}
#endif

#ifdef TEST_KWSYS_CHAR_IS_SIGNED
/* Return 0 for char signed and 1 for char unsigned.  */
int main()
{
  unsigned char uc = 255;
  return (*reinterpret_cast<char*>(&uc) < 0)?0:1;
}
#endif

#ifdef TEST_KWSYS_LFS_WORKS
/* Return 0 when LFS is available and 1 otherwise.  */
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE
#define _LARGE_FILES
#define _FILE_OFFSET_BITS 64
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#if KWSYS_CXX_HAS_CSTDIO
# include <cstdio>
#endif
#include <stdio.h>

int main(int, char **argv)
{
  /* check that off_t can hold 2^63 - 1 and perform basic operations... */
#define OFF_T_64 (((off_t) 1 << 62) - 1 + ((off_t) 1 << 62))
  if (OFF_T_64 % 2147483647 != 1)
    return 1;

  // stat breaks on SCO OpenServer
  struct stat buf;
  stat( argv[0], &buf );
  if (!S_ISREG(buf.st_mode))
    return 2;

  FILE *file = fopen( argv[0], "r" );
  off_t offset = ftello( file );
  fseek( file, offset, SEEK_CUR );
  fclose( file );
  return 0;
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_SETENV
#include <stdlib.h>
int main()
{
  return setenv("A", "B", 1);
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_UNSETENV
#include <stdlib.h>
int main()
{
  unsetenv("A");
  return 0;
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_ENVIRON_IN_STDLIB_H
#include <stdlib.h>
int main()
{
  char* e = environ[0];
  return e? 0:1;
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_RLIMIT64
# if defined(KWSYS_HAS_LFS)
#  define _LARGEFILE_SOURCE
#  define _LARGEFILE64_SOURCE
#  define _LARGE_FILES
#  define _FILE_OFFSET_BITS 64
# endif
# include <sys/resource.h>
int main()
{
  struct rlimit64 rlim;
  return getrlimit64(0,&rlim);
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_ATOLL
#include <stdlib.h>
int main()
{
  const char *str="1024";
  return static_cast<int>(atoll(str));
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_ATOL
#include <stdlib.h>
int main()
{
  const char *str="1024";
  return static_cast<int>(atol(str));
}
#endif

#ifdef TEST_KWSYS_CXX_HAS__ATOI64
#include <stdlib.h>
int main()
{
  const char *str="1024";
  return static_cast<int>(_atoi64(str));
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_UTIMES
#include <sys/time.h>
int main()
{
  struct timeval* current_time = 0;
  return utimes("/example", current_time);
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_UTIMENSAT
#include <fcntl.h>
#include <sys/stat.h>
int main()
{
  struct timespec times[2] = {{0,UTIME_OMIT},{0,UTIME_NOW}};
  return utimensat(AT_FDCWD, "/example", times, AT_SYMLINK_NOFOLLOW);
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_BACKTRACE
#if defined(__PATHSCALE__) || defined(__PATHCC__) \
  || (defined(__LSB_VERSION__) && (__LSB_VERSION__ < 41))
backtrace doesnt work with this compiler or os
#endif
#if (defined(__GNUC__) || defined(__PGI)) && !defined(_GNU_SOURCE)
# define _GNU_SOURCE
#endif
#include <execinfo.h>
int main()
{
  void *stackSymbols[256];
  backtrace(stackSymbols,256);
  backtrace_symbols(&stackSymbols[0],1);
  return 0;
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_DLADDR
#if (defined(__GNUC__) || defined(__PGI)) && !defined(_GNU_SOURCE)
# define _GNU_SOURCE
#endif
#include <dlfcn.h>
int main()
{
  Dl_info info;
  int ierr=dladdr((void*)main,&info);
  return 0;
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_CXXABI
#if (defined(__GNUC__) || defined(__PGI)) && !defined(_GNU_SOURCE)
# define _GNU_SOURCE
#endif
#if defined(__SUNPRO_CC) && __SUNPRO_CC >= 0x5130 \
     && __linux && __SUNPRO_CC_COMPAT == 'G'
#  include <iostream>
#endif
#include <cxxabi.h>
int main()
{
  int status = 0;
  size_t bufferLen = 512;
  char buffer[512] = {'\0'};
  const char *function="_ZN5kwsys17SystemInformation15GetProgramStackEii";
  char *demangledFunction =
    abi::__cxa_demangle(function, buffer, &bufferLen, &status);
  return status;
}
#endif

#ifdef TEST_KWSYS_CXX_TYPE_INFO
/* Collect fundamental type information and save it to a CMake script.  */

/* Include limits.h to get macros indicating long long and __int64.
   Note that certain compilers need special macros to define these
   macros in limits.h.  */
#if defined(_MSC_VER) && !defined(_MSC_EXTENSIONS)
# define _MSC_EXTENSIONS
#endif
#if defined(__GNUC__) && __GNUC__ < 3
# define _GNU_SOURCE
#endif
#include <limits.h>

#include <stdio.h>
#include <string.h>

/* Due to shell differences and limitations of ADD_DEFINITIONS the
   KWSYS_CXX_TYPE_INFO_FILE macro will sometimes have double quotes
   and sometimes not.  This macro will make sure the value is treated
   as a double-quoted string.  */
#define TO_STRING(x) TO_STRING0(x)
#define TO_STRING0(x) TO_STRING1(x)
#define TO_STRING1(x) #x

void f() {}

int main()
{
  /* Construct the output file name.  Some preprocessors will add an
     extra level of double quotes, so strip them.  */
  char fbuf[] = TO_STRING(KWSYS_CXX_TYPE_INFO_FILE);
  char* fname = fbuf;
  if(fname[0] == '"')
    {
    ++fname;
    int len = static_cast<int>(strlen(fname));
    if(len > 0 && fname[len-1] == '"')
      {
      fname[len-1] = 0;
      }
    }

  /* Try to open the output file.  */
  if(FILE* fout = fopen(fname, "w"))
    {
    /* Set the size of standard types.  */
    fprintf(fout, "SET(KWSYS_SIZEOF_CHAR %d)\n", static_cast<int>(sizeof(char)));
    fprintf(fout, "SET(KWSYS_SIZEOF_SHORT %d)\n", static_cast<int>(sizeof(short)));
    fprintf(fout, "SET(KWSYS_SIZEOF_INT %d)\n", static_cast<int>(sizeof(int)));
    fprintf(fout, "SET(KWSYS_SIZEOF_LONG %d)\n", static_cast<int>(sizeof(long)));

    /* Set the size of some non-standard but common types.  */
    /* Check for a limits.h macro for long long to see if the type exists.  */
#if defined(LLONG_MAX) || defined(LONG_LONG_MAX) || defined(LONGLONG_MAX)
    fprintf(fout, "SET(KWSYS_SIZEOF_LONG_LONG %d)\n", static_cast<int>(sizeof(long long)));
#else
    fprintf(fout, "SET(KWSYS_SIZEOF_LONG_LONG 0) # No long long available.\n");
#endif
    /* Check for a limits.h macro for __int64 to see if the type exists.  */
#if defined(_I64_MIN)
    fprintf(fout, "SET(KWSYS_SIZEOF___INT64 %d)\n", static_cast<int>(sizeof(__int64)));
#else
    fprintf(fout, "SET(KWSYS_SIZEOF___INT64 0) # No __int64 available.\n");
#endif

    /* Set the size of some pointer types.  */
    fprintf(fout, "SET(KWSYS_SIZEOF_PDATA %d)\n", static_cast<int>(sizeof(void*)));
    fprintf(fout, "SET(KWSYS_SIZEOF_PFUNC %d)\n", static_cast<int>(sizeof(&f)));

    /* Set whether the native type "char" is signed or unsigned.  */
    unsigned char uc = 255;
    fprintf(fout, "SET(KWSYS_CHAR_IS_SIGNED %d)\n",
            (*reinterpret_cast<char*>(&uc) < 0)?1:0);

    fclose(fout);
    return 0;
    }
  else
    {
    fprintf(stderr, "Failed to write fundamental type info to \"%s\".\n",
            fname);
    return 1;
    }
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_BORLAND_ASM
int main()
{
  int a = 1;
  __asm {
    xor EBX, EBX;
    mov a, EBX;
  }

  return a;
}
#endif

#ifdef TEST_KWSYS_CXX_HAS_BORLAND_ASM_CPUID
int main()
{
  int a = 0;
  __asm {
    xor EAX, EAX;
    cpuid;
    mov a, EAX;
  }

  return a;
}
#endif

#ifdef TEST_KWSYS_STL_HAS_WSTRING
#include <string>
void f(std ::wstring*) {}
int main() { return 0; }
#endif
