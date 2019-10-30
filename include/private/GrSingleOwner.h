/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSingleOwner_DEFINED
#define GrSingleOwner_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_DEBUG
#include "include/private/SkMutex.h"
#include "include/private/SkThreadID.h"

// This is a debug tool to verify an object is only being used from one thread at a time.
class GrSingleOwner {
public:
     GrSingleOwner() : fOwner(kIllegalThreadID), fReentranceCount(0) {}

     struct AutoEnforce {
         AutoEnforce(GrSingleOwner* so) : fSO(so) { fSO->enter(); }
         ~AutoEnforce() { fSO->exit(); }

         GrSingleOwner* fSO;
     };

private:
     void enter() {
         SkAutoMutexExclusive lock(fMutex);
         SkThreadID self = SkGetThreadID();
         SkASSERT(fOwner == self || fOwner == kIllegalThreadID);
         fReentranceCount++;
         fOwner = self;
     }

     void exit() {
         SkAutoMutexExclusive lock(fMutex);
         SkASSERT(fOwner == SkGetThreadID());
         fReentranceCount--;
         if (fReentranceCount == 0) {
             fOwner = kIllegalThreadID;
         }
     }

     SkMutex fMutex;
     SkThreadID fOwner    SK_GUARDED_BY(fMutex);
     int fReentranceCount SK_GUARDED_BY(fMutex);
};
#else
class GrSingleOwner {}; // Provide a dummy implementation so we can pass pointers to constructors
#endif

#endif
