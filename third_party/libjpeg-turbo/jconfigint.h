/* jconfigint.h.  Customized for android on the basis of jconfigint.h.in. */

#ifndef __JCONFIGINT_H__
#define __JCONFIGINT_H__


#define BUILD ""

/* How to obtain function inlining. */
#ifndef INLINE
  #ifndef TURBO_FOR_WINDOWS
    #define INLINE inline __attribute__((always_inline))
  #else
    #if defined(__GNUC__)
      #define INLINE inline __attribute__((always_inline))
    #elif defined(_MSC_VER)
      #define INLINE __forceinline
    #else
      #define INLINE
    #endif
  #endif
#endif

/* Define to the full name of this package. */
#define PACKAGE_NAME "libjpeg-turbo"

/* Version number of package */
#define VERSION "2.0.0"

/* The size of `size_t', as reported by the compiler through the
 * builtin macro __SIZEOF_SIZE_T__. If the compiler does not
 * report __SIZEOF_SIZE_T__ add a custom rule for the compiler
 * here. */
#ifdef __SIZEOF_SIZE_T__
#define SIZEOF_SIZE_T __SIZEOF_SIZE_T__
#elif __WORDSIZE==64 || defined(_WIN64)
#define SIZEOF_SIZE_T 8
#else
#define SIZEOF_SIZE_T 4
#endif

#endif // __JCONFIGINT_H__
