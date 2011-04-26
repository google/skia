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

class SkGPipeControler {
public:
    struct Block {
        void*   fAddr;
        size_t  fSize;
    };

    enum Status {
        kSuccess_Status,
        kFailure_Status
    };

    /**
     *  To record drawing commands, we request blocks from the controller for
     *  subsequent writes, and we want to send/flush blocks of commands we have
     *  already written.
     *
     *  For each call to handleBlock, the send block will contain the block
     *  (previously returned in a request parameter) that we have written, and
     *  if there is more to be recorded, the request block will receive the
     *  new block of memory to write into. When the writer detects that there
     *  are no more drawing commands expected, it will call handleBlock with
     *  NULL for the request parameter.
     *
     *  If handleBlock ever returns kFailure_Status, the writer will cease to
     *  call handleBlock.
     */
    virtual Status handleBlock(const Block& send, Block* request) = 0;
};

class SkGPipeWriter {
public:
    SkGPipeWriter();
    ~SkGPipeWriter();

    bool isRecording() const { return NULL != fCanvas; }
    SkCanvas* startRecording(SkGPipeControler*);
    void endRecording();

    size_t flatten(void* buffer);

private:
    class SkGPipeCanvas* fCanvas;
    SkGPipeControler*    fControler;
    SkWriter32 fWriter;
};

#endif
