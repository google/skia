/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTSAN_DEFINED
#define SkTSAN_DEFINED

#include "SkTypes.h"

#if !defined(__DYNAMIC_ANNOTATIONS_H__)

    #if !defined(__has_feature)
        #define __has_feature(x) 0
    #endif

    #if __has_feature(thread_sanitizer)

        /* Report that a lock has been created at address "lock". */
        #define ANNOTATE_RWLOCK_CREATE(lock) \
            AnnotateRWLockCreate(__FILE__, __LINE__, lock)

        /* Report that the lock at address "lock" is about to be destroyed. */
        #define ANNOTATE_RWLOCK_DESTROY(lock) \
            AnnotateRWLockDestroy(__FILE__, __LINE__, lock)

        /* Report that the lock at address "lock" has been acquired.
           is_w=1 for writer lock, is_w=0 for reader lock. */
        #define ANNOTATE_RWLOCK_ACQUIRED(lock, is_w) \
            AnnotateRWLockAcquired(__FILE__, __LINE__, lock, is_w)

        /* Report that the lock at address "lock" is about to be released. */
        #define ANNOTATE_RWLOCK_RELEASED(lock, is_w) \
          AnnotateRWLockReleased(__FILE__, __LINE__, lock, is_w)

        #if defined(DYNAMIC_ANNOTATIONS_WANT_ATTRIBUTE_WEAK)
            #if defined(__GNUC__)
                #define DYNAMIC_ANNOTATIONS_ATTRIBUTE_WEAK __attribute__((weak))
            #else
                /* TODO(glider): for Windows support we may want to change this macro in order
                   to prepend __declspec(selectany) to the annotations' declarations. */
                #error weak annotations are not supported for your compiler
            #endif
        #else
            #define DYNAMIC_ANNOTATIONS_ATTRIBUTE_WEAK
        #endif

        extern "C" {
        void AnnotateRWLockCreate(
            const char *file, int line,
            const volatile void *lock) DYNAMIC_ANNOTATIONS_ATTRIBUTE_WEAK;
        void AnnotateRWLockDestroy(
            const char *file, int line,
            const volatile void *lock) DYNAMIC_ANNOTATIONS_ATTRIBUTE_WEAK;
        void AnnotateRWLockAcquired(
            const char *file, int line,
            const volatile void *lock, long is_w) DYNAMIC_ANNOTATIONS_ATTRIBUTE_WEAK;
        void AnnotateRWLockReleased(
            const char *file, int line,
            const volatile void *lock, long is_w) DYNAMIC_ANNOTATIONS_ATTRIBUTE_WEAK;
        }

    #else

        #define ANNOTATE_RWLOCK_CREATE(lock)
        #define ANNOTATE_RWLOCK_DESTROY(lock)
        #define ANNOTATE_RWLOCK_ACQUIRED(lock, is_w)
        #define ANNOTATE_RWLOCK_RELEASED(lock, is_w)

    #endif

#endif//!defined(__DYNAMIC_ANNOTATIONS_H__)

#endif//SkTSAN_DEFINED
