/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSemaphore.h"
#include "src/core/SkLeanWindows.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
    #include <mach/mach.h>

    // We've got to teach TSAN that there is a happens-before edge between
    // semaphore_signal() and semaphore_wait().
    #if __has_feature(thread_sanitizer)
        extern "C" void AnnotateHappensBefore(const char*, int, void*);
        extern "C" void AnnotateHappensAfter (const char*, int, void*);
    #else
        static void AnnotateHappensBefore(const char*, int, void*) {}
        static void AnnotateHappensAfter (const char*, int, void*) {}
    #endif

    struct SkSemaphore::OSSemaphore {
        semaphore_t fSemaphore;

        OSSemaphore()  {
            semaphore_create(mach_task_self(), &fSemaphore, SYNC_POLICY_LIFO, 0/*initial count*/);
        }
        ~OSSemaphore() { semaphore_destroy(mach_task_self(), fSemaphore); }

        void signal(int n) {
            while (n --> 0) {
                AnnotateHappensBefore(__FILE__, __LINE__, &fSemaphore);
                semaphore_signal(fSemaphore);
            }
        }
        void wait() {
            while (true) {
                kern_return_t result = semaphore_wait(fSemaphore);
                if (result == KERN_SUCCESS) {
                    AnnotateHappensAfter(__FILE__, __LINE__, &fSemaphore);
                    return;
                }
                SkASSERT(result == KERN_ABORTED);
            }
        }
    };
#elif defined(SK_BUILD_FOR_WIN)
    struct SkSemaphore::OSSemaphore {
        HANDLE fSemaphore;

        OSSemaphore()  {
            fSemaphore = CreateSemaphore(nullptr    /*security attributes, optional*/,
                                         0       /*initial count*/,
                                         MAXLONG /*max count*/,
                                         nullptr    /*name, optional*/);
        }
        ~OSSemaphore() { CloseHandle(fSemaphore); }

        void signal(int n) {
            ReleaseSemaphore(fSemaphore, n, nullptr/*returns previous count, optional*/);
        }
        void wait() { WaitForSingleObject(fSemaphore, INFINITE/*timeout in ms*/); }
    };
#else
    // It's important we test for Mach before this.  This code will compile but not work there.
    #include <errno.h>
    #include <semaphore.h>
    struct SkSemaphore::OSSemaphore {
        sem_t fSemaphore;

        OSSemaphore()  { sem_init(&fSemaphore, 0/*cross process?*/, 0/*initial count*/); }
        ~OSSemaphore() { sem_destroy(&fSemaphore); }

        void signal(int n) { while (n --> 0) { sem_post(&fSemaphore); } }
        void wait() {
            // Try until we're not interrupted.
            while(sem_wait(&fSemaphore) == -1 && errno == EINTR);
        }
    };
#endif

///////////////////////////////////////////////////////////////////////////////

SkSemaphore::~SkSemaphore() {
    delete fOSSemaphore;
}

void SkSemaphore::osSignal(int n) {
    fOSSemaphoreOnce([this] { fOSSemaphore = new OSSemaphore; });
    fOSSemaphore->signal(n);
}

void SkSemaphore::osWait() {
    fOSSemaphoreOnce([this] { fOSSemaphore = new OSSemaphore; });
    fOSSemaphore->wait();
}

bool SkSemaphore::try_wait() {
    int count = fCount.load(std::memory_order_relaxed);
    if (count > 0) {
        return fCount.compare_exchange_weak(count, count-1, std::memory_order_acquire);
    }
    return false;
}
