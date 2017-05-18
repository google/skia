/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#if defined(SK_BUILD_FOR_WIN32)

#include "SkThreadUtils.h"
#include "SkThreadUtils_win.h"

SkThread_WinData::SkThread_WinData(SkThread::entryPointProc entryPoint, void* data)
    : fHandle(nullptr)
    , fParam(data)
    , fThreadId(0)
    , fEntryPoint(entryPoint)
    , fStarted(false)
{
    fCancelEvent = CreateEvent(
        nullptr,  // default security attributes
        false, //auto reset
        false, //not signaled
        nullptr); //no name
}

SkThread_WinData::~SkThread_WinData() {
    CloseHandle(fCancelEvent);
}

static DWORD WINAPI thread_start(LPVOID data) {
    SkThread_WinData* winData = static_cast<SkThread_WinData*>(data);

    //See if this thread was canceled before starting.
    if (WaitForSingleObject(winData->fCancelEvent, 0) == WAIT_OBJECT_0) {
        return 0;
    }

    winData->fEntryPoint(winData->fParam);
    return 0;
}

SkThread::SkThread(entryPointProc entryPoint, void* data) {
    SkThread_WinData* winData = new SkThread_WinData(entryPoint, data);
    fData = winData;

    if (nullptr == winData->fCancelEvent) {
        return;
    }

    winData->fHandle = CreateThread(
        nullptr,                   // default security attributes
        0,                      // use default stack size
        thread_start,           // thread function name (proxy)
        winData,                // argument to thread function (proxy args)
        CREATE_SUSPENDED,       // we used to set processor affinity, which needed this
        &winData->fThreadId);   // returns the thread identifier
}

SkThread::~SkThread() {
    if (fData != nullptr) {
        SkThread_WinData* winData = static_cast<SkThread_WinData*>(fData);
        // If created thread but start was never called, kill the thread.
        if (winData->fHandle != nullptr && !winData->fStarted) {
            if (SetEvent(winData->fCancelEvent) != 0) {
                if (this->start()) {
                    this->join();
                }
            } else {
                //kill with prejudice
                TerminateThread(winData->fHandle, -1);
            }
        }
        delete winData;
    }
}

bool SkThread::start() {
    SkThread_WinData* winData = static_cast<SkThread_WinData*>(fData);
    if (nullptr == winData->fHandle) {
        return false;
    }

    if (winData->fStarted) {
        return false;
    }
    winData->fStarted = -1 != ResumeThread(winData->fHandle);
    return winData->fStarted;
}

void SkThread::join() {
    SkThread_WinData* winData = static_cast<SkThread_WinData*>(fData);
    if (nullptr == winData->fHandle || !winData->fStarted) {
        return;
    }

    WaitForSingleObject(winData->fHandle, INFINITE);
}

#endif//defined(SK_BUILD_FOR_WIN32)
