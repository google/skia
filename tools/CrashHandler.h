#ifndef CrashHandler_DEFINED
#define CrashHandler_DEFINED

// If possible (and not already done), and SK_CRASH_HANDLER is defined,
// register a handler for a stack trace when we crash.
//
// Currently this works on Linux and Mac and Windows.
// On Linux, our output is garbage compared to catchsegv.  Use catchsegv if possible.
void SetupCrashHandler();

#endif//CrashHandler_DEFINED
