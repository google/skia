/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSemaphore.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
    #include <mach/mach.h>
    struct SkSemaphore::OSSemaphore {
        semaphore_t fSemaphore;

        OSSemaphore()  {
            semaphore_create(mach_task_self(), &fSemaphore, SYNC_POLICY_LIFO, 0/*initial count*/);
        }
        ~OSSemaphore() { semaphore_destroy(mach_task_self(), fSemaphore); }

        void signal(int n) { while (n --> 0) { semaphore_signal(fSemaphore); } }
        void wait() { semaphore_wait(fSemaphore); }
    };
#elif defined(SK_BUILD_FOR_WIN32)
    struct SkSemaphore::OSSemaphore {
        HANDLE fSemaphore;

        OSSemaphore()  {
            fSemaphore = CreateSemaphore(NULL    /*security attributes, optional*/,
                                         0       /*initial count*/,
                                         MAXLONG /*max count*/,
                                         NULL    /*name, optional*/);
        }
        ~OSSemaphore() { CloseHandle(fSemaphore); }

        void signal(int n) {
            ReleaseSemaphore(fSemaphore, n, NULL/*returns previous count, optional*/);
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

SkSemaphore::SkSemaphore() : fCount(0), fOSSemaphore(SkNEW(OSSemaphore)) {}
SkSemaphore::~SkSemaphore() { SkDELETE(fOSSemaphore); }

void SkSemaphore::signal(int n) {
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
    int prev = fCount.fetch_add(n, sk_memory_order_release);
    int toSignal = SkTMin(-prev, n);
    if (toSignal > 0) {
        fOSSemaphore->signal(toSignal);
    }
}

void SkSemaphore::wait() {
    // We only wait() on the OS semaphore if the count drops <= 0,
    // i.e. when we need to make this thread sleep to wait for it to go back up.
    if (fCount.fetch_add(-1, sk_memory_order_acquire) <= 0) {
        fOSSemaphore->wait();
    }
}
