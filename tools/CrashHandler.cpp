/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/CrashHandler.h"

#include "include/private/SkLeanWindows.h"

#include <stdlib.h>

#if defined(SK_BUILD_FOR_GOOGLE3)
    #include "base/process_state.h"
    void SetupCrashHandler() { InstallSignalHandlers(); }

#else

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

            SkDebugf("\nSignal %d:\n", sig);
            while (unw_step(&cursor) > 0) {
                static const size_t kMax = 256;
                char mangled[kMax], demangled[kMax];
                unw_word_t offset;
                unw_get_proc_name(&cursor, mangled, kMax, &offset);

                int ok;
                size_t len = kMax;
                abi::__cxa_demangle(mangled, demangled, &len, &ok);

                SkDebugf("%s (+0x%zx)\n", ok == 0 ? demangled : mangled, (size_t)offset);
            }
            SkDebugf("\n");

            // Exit NOW.  Don't notify other threads, don't call anything registered with atexit().
            _Exit(sig);
        }

    #elif defined(SK_BUILD_FOR_UNIX)
        // We'd use libunwind here too, but it's a pain to get installed for
        // both 32 and 64 bit on bots.  Doesn't matter much: catchsegv is best anyway.
        #include <cxxabi.h>
        #include <dlfcn.h>
        #include <execinfo.h>
        #include <string.h>

        static void handler(int sig) {
            void* stack[64];
            const int count = backtrace(stack, SK_ARRAY_COUNT(stack));
            char** symbols = backtrace_symbols(stack, count);

            SkDebugf("\nSignal %d [%s]:\n", sig, strsignal(sig));
            for (int i = 0; i < count; i++) {
                Dl_info info;
                if (dladdr(stack[i], &info) && info.dli_sname) {
                    char demangled[256];
                    size_t len = SK_ARRAY_COUNT(demangled);
                    int ok;

                    abi::__cxa_demangle(info.dli_sname, demangled, &len, &ok);
                    if (ok == 0) {
                        SkDebugf("    %s\n", demangled);
                        continue;
                    }
                }
                SkDebugf("    %s\n", symbols[i]);
            }

            // Exit NOW.  Don't notify other threads, don't call anything registered with atexit().
            _Exit(sig);
        }

    #endif

    #if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_UNIX)
        #include <signal.h>

        void SetupCrashHandler() {
            static const int kSignals[] = {
                SIGABRT,
                SIGBUS,
                SIGFPE,
                SIGILL,
                SIGSEGV,
                SIGTRAP,
            };

            for (size_t i = 0; i < sizeof(kSignals) / sizeof(kSignals[0]); i++) {
                // Register our signal handler unless something's already done so (e.g. catchsegv).
                void (*prev)(int) = signal(kSignals[i], handler);
                if (prev != SIG_DFL) {
                    signal(kSignals[i], prev);
                }
            }
        }

    #elif defined(SK_BUILD_FOR_WIN)

        #include <DbgHelp.h>
        #include "include/private/SkMalloc.h"

        static const struct {
            const char* name;
            const DWORD code;
        } kExceptions[] = {
        #define _(E) {#E, E}
            _(EXCEPTION_ACCESS_VIOLATION),
            _(EXCEPTION_BREAKPOINT),
            _(EXCEPTION_INT_DIVIDE_BY_ZERO),
            _(EXCEPTION_STACK_OVERFLOW),
            // TODO: more?
        #undef _
        };

        static LONG WINAPI handler(EXCEPTION_POINTERS* e) {
            const DWORD code = e->ExceptionRecord->ExceptionCode;
            SkDebugf("\nCaught exception %u", code);
            for (size_t i = 0; i < SK_ARRAY_COUNT(kExceptions); i++) {
                if (kExceptions[i].code == code) {
                    SkDebugf(" %s", kExceptions[i].name);
                }
            }
            SkDebugf("\n");

            // We need to run SymInitialize before doing any of the stack walking below.
            HANDLE hProcess = GetCurrentProcess();
            SymInitialize(hProcess, 0, true);

            STACKFRAME64 frame;
            sk_bzero(&frame, sizeof(frame));
            // Start frame off from the frame that triggered the exception.
            CONTEXT* c = e->ContextRecord;
            frame.AddrPC.Mode      = AddrModeFlat;
            frame.AddrStack.Mode   = AddrModeFlat;
            frame.AddrFrame.Mode   = AddrModeFlat;
        #if defined(_X86_)
            frame.AddrPC.Offset    = c->Eip;
            frame.AddrStack.Offset = c->Esp;
            frame.AddrFrame.Offset = c->Ebp;
            const DWORD machineType = IMAGE_FILE_MACHINE_I386;
        #elif defined(_AMD64_)
            frame.AddrPC.Offset    = c->Rip;
            frame.AddrStack.Offset = c->Rsp;
            frame.AddrFrame.Offset = c->Rbp;
            const DWORD machineType = IMAGE_FILE_MACHINE_AMD64;
        #elif defined(_M_ARM64)
            frame.AddrPC.Offset    = c->Pc;
            frame.AddrStack.Offset = c->Sp;
            frame.AddrFrame.Offset = c->Fp;
            const DWORD machineType = IMAGE_FILE_MACHINE_ARM64;
        #endif

            while (StackWalk64(machineType,
                               GetCurrentProcess(),
                               GetCurrentThread(),
                               &frame,
                               c,
                               nullptr,
                               SymFunctionTableAccess64,
                               SymGetModuleBase64,
                               nullptr)) {
                // Buffer to store symbol name in.
                static const int kMaxNameLength = 1024;
                uint8_t buffer[sizeof(IMAGEHLP_SYMBOL64) + kMaxNameLength];
                sk_bzero(buffer, sizeof(buffer));

                // We have to place IMAGEHLP_SYMBOL64 at the front, and fill in
                // how much space it can use.
                IMAGEHLP_SYMBOL64* symbol = reinterpret_cast<IMAGEHLP_SYMBOL64*>(&buffer);
                symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
                symbol->MaxNameLength = kMaxNameLength - 1;

                // Translate the current PC into a symbol and byte offset from the symbol.
                DWORD64 offset;
                SymGetSymFromAddr64(hProcess, frame.AddrPC.Offset, &offset, symbol);

                SkDebugf("%s +%x\n", symbol->Name, offset);
            }

            // Exit NOW.  Don't notify other threads, don't call anything registered with atexit().
            _exit(1);

            // The compiler wants us to return something.  This is what we'd do
            // if we didn't _exit().
            return EXCEPTION_EXECUTE_HANDLER;
        }

        void SetupCrashHandler() {
            SetUnhandledExceptionFilter(handler);
        }

    #else

        void SetupCrashHandler() { }

    #endif
#endif // SK_BUILD_FOR_GOOGLE3?
