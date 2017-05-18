/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSingleOwner_DEFINED
#define GrSingleOwner_DEFINED

#include "SkTypes.h"

#ifdef SK_DEBUG
#include "SkMutex.h"
#include "SkThreadID.h"

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
         SkAutoMutexAcquire lock(fMutex);
         SkThreadID self = SkGetThreadID();
         SkASSERT(fOwner == self || fOwner == kIllegalThreadID);
         fReentranceCount++;
         fOwner = self;
     }

     void exit() {
         SkAutoMutexAcquire lock(fMutex);
         SkASSERT(fOwner == SkGetThreadID());
         fReentranceCount--;
         if (fReentranceCount == 0) {
             fOwner = kIllegalThreadID;
         }
     }

     SkMutex fMutex;
     SkThreadID fOwner;    // guarded by fMutex
     int fReentranceCount; // guarded by fMutex
};
#else
class GrSingleOwner {}; // Provide a dummy implementation so we can pass pointers to constructors
#endif

#endif
