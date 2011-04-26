/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef SkGPipe_DEFINED
#define SkGPipe_DEFINED

#include "SkWriter32.h"

class SkCanvas;

class SkGPipeReader {
public:
    SkGPipeReader(SkCanvas* target);
    ~SkGPipeReader();

    enum Status {
        kDone_Status,   //!< no more data expected from reader
        kEOF_Status,    //!< need more data from reader
        kError_Status   //!< encountered error
    };

    Status playback(const void* data, size_t length);

private:
    SkCanvas*           fCanvas;
    class SkGPipeState* fState;
};

///////////////////////////////////////////////////////////////////////////////

class SkGPipeWriter {
public:
    SkGPipeWriter();
    ~SkGPipeWriter();

    bool isRecording() const { return NULL != fCanvas; }
    SkCanvas* startRecording();
    void endRecording();

    size_t flatten(void* buffer);

private:
    class SkGPipeCanvas* fCanvas;
    SkWriter32 fWriter;
};

#endif
