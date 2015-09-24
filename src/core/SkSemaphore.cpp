/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../private/SkSemaphore.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
    #include <mach/mach.h>
    struct SkBaseSemaphore::OSSemaphore {
        semaphore_t fSemaphore;

        OSSemaphore()  {
            semaphore_create(mach_task_self(), &fSemaphore, SYNC_POLICY_LIFO, 0/*initial count*/);
        }
        ~OSSemaphore() { semaphore_destroy(mach_task_self(), fSemaphore); }

        void signal(int n) { while (n --> 0) { semaphore_signal(fSemaphore); } }
        void wait() { semaphore_wait(fSemaphore); }
    };
#elif defined(SK_BUILD_FOR_WIN32)
    struct SkBaseSemaphore::OSSemaphore {
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
    struct SkBaseSemaphore::OSSemaphore {
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

void SkBaseSemaphore::signal(int n) {
    SkASSERT(n >= 0);

    // We only want to call the OS semaphore when our logical count crosses
    // from <= 0 to >0 (when we need to wake sleeping threads).
    //
    // This is easiest to think about with specific examples of prev and n.
    // If n == 5 and prev == -3, there are 3 threads sleeping and we signal
    // SkTMin(-(-3), 5) == 3 times on the OS semaphore, leaving the count at 2.
    //
    // If prev >= 0, no threads are waiting, SkTMin(-prev, n) is always <= 0,
    // so we don't call the OS semaphore, leaving the count at (prev + n).
    int prev = sk_atomic_fetch_add(&fCount, n, sk_memory_order_release);
    int toSignal = SkTMin(-prev, n);
    if (toSignal > 0) {
        this->osSignal(toSignal);
    }
}

static SkBaseSemaphore::OSSemaphore* semaphore(SkBaseSemaphore* semaphore) {
    return semaphore->fOSSemaphore.get([](){ return new SkBaseSemaphore::OSSemaphore(); });
}

void SkBaseSemaphore::osSignal(int n) { semaphore(this)->signal(n); }

void SkBaseSemaphore::osWait() { semaphore(this)->wait(); }

void SkBaseSemaphore::deleteSemaphore() {
    delete (OSSemaphore*) fOSSemaphore;
}

///////////////////////////////////////////////////////////////////////////////

SkSemaphore::SkSemaphore(){ fBaseSemaphore = {0, {0}}; }

SkSemaphore::~SkSemaphore() { fBaseSemaphore.deleteSemaphore(); }

void SkSemaphore::wait() { fBaseSemaphore.wait(); }

void SkSemaphore::signal(int n) {fBaseSemaphore.signal(n); }
