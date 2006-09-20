#ifndef SkPreConfig_DEFINED
#define SkPreConfig_DEFINED

#ifdef ANDROID
    #define SK_BUILD_FOR_UNIX
    #define SK_SCALAR_IS_FIXED
    #define SK_CAN_USE_FLOAT
#endif

#if !defined(SK_CPU_BENDIAN) && !defined(SK_CPU_LENDIAN)
    #if defined(__APPLE__) || defined(__MC68K__)
        #define SK_CPU_BENDIAN
    #else
        #define SK_CPU_LENDIAN
    #endif
#endif

//////////////////////////////////////////////////////////////////////

#if !defined(SK_BUILD_FOR_PALM) && !defined(SK_BUILD_FOR_WINCE) && !defined(SK_BUILD_FOR_WIN32) && !defined(SK_BUILD_FOR_SYMBIAN) && !defined(SK_BUILD_FOR_UNIX) && !defined(SK_BUILD_FOR_MAC)

    #if defined(PALMOS_SDK_VERSION)
        #define SK_BUILD_FOR_PALM
    #elif defined(UNDER_CE)
        #define SK_BUILD_FOR_WINCE
    #elif defined(WIN32)
        #define SK_BUILD_FOR_WIN32
    #elif defined(__SYMBIAN32__)
        #define SK_BUILD_FOR_WIN32
    #elif defined(linux)
        #define SK_BUILD_FOR_UNIX
    #else
        #define SK_BUILD_FOR_MAC
    #endif

#endif

//////////////////////////////////////////////////////////////////////

#if !defined(SK_DEBUG) && !defined(SK_RELEASE)
	#ifdef NDEBUG
		#define SK_RELEASE
	#else
		#define SK_DEBUG
	#endif
#endif

//////////////////////////////////////////////////////////////////////

#ifdef SK_BUILD_FOR_WIN32
	#define SK_SCALAR_IS_FLOAT
#endif

#if defined(SK_BUILD_FOR_WIN32) || defined(SK_BUILD_FOR_MAC)
	#define SK_CAN_USE_FLOAT
    #define SK_SCALAR_IS_FIXED
	#define SK_CAN_USE_LONGLONG
#endif

//////////////////////////////////////////////////////////////////////

#ifdef SK_CAN_USE_LONGLONG
	#ifdef SK_BUILD_FOR_WIN32
		#define SkLONGLONG	__int64
	#else
		#define SkLONGLONG	long long
	#endif
#endif

//////////////////////////////////////////////////////////////////////

#if !defined(SK_CPU_BENDIAN) && !defined(SK_CPU_LENDIAN)

#ifdef SK_BUILD_FOR_MAC
	#define SK_CPU_BENDIAN
#else
	#define SK_CPU_LENDIAN
#endif

#endif

#if defined(SK_BUILD_FOR_BREW) || defined(SK_BUILD_FOR_WINCE) || (defined(SK_BUILD_FOR_SYMBIAN) && !defined(__MARM_THUMB__))
	/* e.g. the ARM instructions have conditional execution, making tiny branches cheap */
	#define SK_CPU_HAS_CONDITIONAL_INSTR
#endif

//////////////////////////////////////////////////////////////////////
// Conditional features based on build target

#if defined(SK_BUILD_FOR_WIN32) || defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_UNIX)
    #ifndef SK_BUILD_NO_IMAGE_ENCODE
        #define SK_SUPPORT_IMAGE_ENCODE
    #endif
#endif

#ifdef SK_BUILD_FOR_SYMBIAN
    #define SK_USE_RUNTIME_GLOBALS
#endif

#endif

