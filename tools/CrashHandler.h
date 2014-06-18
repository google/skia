#ifndef CrashHandler_DEFINED
#define CrashHandler_DEFINED

// If possible (and not already done) register a handler for a stack trace when we crash.
// Currently this works on Linux and Mac, hopefully Windows soon.
// On Linux, you will get much better results than we can deliver by running "catchsegv <program>".
void SetupCrashHandler();

#endif//CrashHandler_DEFINED
