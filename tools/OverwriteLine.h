#ifndef OverwriteLine_DEFINED
#define OverwriteLine_DEFINED

// Print this string to reset and clear your current terminal line.
static const char* kSkOverwriteLine =
#ifdef SK_BUILD_FOR_WIN
"\r                                                                               \r"
#elif defined(SK_BUILD_FOR_IOS)
"\r"
#else
"\r\033[K"
#endif
;

#endif//OverwriteLine_DEFINED
