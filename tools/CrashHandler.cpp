#include "CrashHandler.h"

#include "SkTypes.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#if defined(SK_BUILD_FOR_MAC)

// We only use local unwinding, so we can define this to select a faster implementation.
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#include <cxxabi.h>

static void handler(int sig) {
    unw_context_t context;
    unw_getcontext(&context);

    unw_cursor_t cursor;
    unw_init_local(&cursor, &context);

    fprintf(stderr, "\nSignal %d:\n", sig);
    while (unw_step(&cursor) > 0) {
        static const size_t kMax = 256;
        char mangled[kMax], demangled[kMax];
        unw_word_t offset;
        unw_get_proc_name(&cursor, mangled, kMax, &offset);

        int ok;
        size_t len = kMax;
        abi::__cxa_demangle(mangled, demangled, &len, &ok);

        fprintf(stderr, "%s (+0x%zx)\n", ok == 0 ? demangled : mangled, (size_t)offset);
    }
    fprintf(stderr, "\n");

    // Exit NOW.  Don't notify other threads, don't call anything registered with atexit().
    _Exit(sig);
}

#elif defined(SK_BUILD_FOR_UNIX)

// We'd use libunwind here too, but it's a pain to get installed for both 32 and 64 bit on bots.
// Doesn't matter much: catchsegv is best anyway.
#include <execinfo.h>

static void handler(int sig) {
    static const int kMax = 64;
    void* stack[kMax];
    const int count = backtrace(stack, kMax);

    fprintf(stderr, "\nSignal %d:\n", sig);
    backtrace_symbols_fd(stack, count, 2/*stderr*/);

    // Exit NOW.  Don't notify other threads, don't call anything registered with atexit().
    _Exit(sig);
}

#endif

#if defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_MAC)

void SetupCrashHandler() {
    static const int kSignals[] = {
        SIGABRT,
        SIGBUS,
        SIGFPE,
        SIGILL,
        SIGSEGV,
    };

    for (size_t i = 0; i < sizeof(kSignals) / sizeof(kSignals[0]); i++) {
        // Register our signal handler unless something's already done so (e.g. catchsegv).
        void (*prev)(int) = signal(kSignals[i], handler);
        if (prev != SIG_DFL) {
            signal(kSignals[i], prev);
        }
    }
}

// TODO: #elif defined(SK_BUILD_FOR_WIN) when I find a Windows machine to work from.

#else

void SetupCrashHandler() { }

#endif
