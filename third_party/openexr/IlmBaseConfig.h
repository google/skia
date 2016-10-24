/* config/IlmBaseConfig.h.  Generated from IlmBaseConfig.h.in by configure.  */
//
// Define and set to 1 if the target system has POSIX thread support
// and you want IlmBase to use it for multithreaded file I/O.
//

#define HAVE_PTHREAD 1

//
// Define and set to 1 if the target system supports POSIX semaphores
// and you want OpenEXR to use them; otherwise, OpenEXR will use its
// own semaphore implementation.
//

#define HAVE_POSIX_SEMAPHORES 1


#define HAVE_UCONTEXT_H 1


//
// Dealing with FPEs
//
#define ILMBASE_HAVE_CONTROL_REGISTER_SUPPORT 1


//
// Define and set to 1 if the target system has support for large
// stack sizes.
//

#define ILMBASE_HAVE_LARGE_STACK 1

//
// Current (internal) library namepace name and corresponding public
// client namespaces.
//
#define ILMBASE_INTERNAL_NAMESPACE_CUSTOM 1
#define IMATH_INTERNAL_NAMESPACE Imath_2_2
#define IEX_INTERNAL_NAMESPACE Iex_2_2
#define ILMTHREAD_INTERNAL_NAMESPACE IlmThread_2_2

/* #undef ILMBASE_NAMESPACE_CUSTOM */
#define IMATH_NAMESPACE Imath
#define IEX_NAMESPACE Iex
#define ILMTHREAD_NAMESPACE IlmThread


//
// Define and set to 1 if the target system has support for large
// stack sizes.
//

#define ILMBASE_HAVE_LARGE_STACK 1


//
// Version information
//
#define ILMBASE_VERSION_STRING "2.2.0"
#define ILMBASE_PACKAGE_STRING "IlmBase 2.2.0"

#define ILMBASE_VERSION_MAJOR 2
#define ILMBASE_VERSION_MINOR 2
#define ILMBASE_VERSION_PATCH 0

// Version as a single hex number, e.g. 0x01000300 == 1.0.3
#define ILMBASE_VERSION_HEX ((ILMBASE_VERSION_MAJOR << 24) | \
                             (ILMBASE_VERSION_MINOR << 16) | \
                             (ILMBASE_VERSION_PATCH <<  8))


