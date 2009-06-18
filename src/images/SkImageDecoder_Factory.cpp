/* libs/graphics/ports/SkImageDecoder_Factory.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkImageDecoder.h"
#include "SkMovie.h"
#include "SkStream.h"
#include "SkTRegistry.h"

typedef SkTRegistry<SkImageDecoder*, SkStream*> DecodeReg;

template DecodeReg* DecodeReg::gHead;

SkImageDecoder* SkImageDecoder::Factory(SkStream* stream) {
    const DecodeReg* curr = DecodeReg::Head();
    while (curr) {
        SkImageDecoder* codec = curr->factory()(stream);
        // we rewind here, because we promise later when we call "decode", that
        // the stream will be at its beginning.
        stream->rewind();
        if (codec) {
            return codec;
        }
        curr = curr->next();
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////

typedef SkTRegistry<SkMovie*, SkStream*> MovieReg;

SkMovie* SkMovie::DecodeStream(SkStream* stream) {
    const MovieReg* curr = MovieReg::Head();
    while (curr) {
        SkMovie* movie = curr->factory()(stream);
        if (movie) {
            return movie;
        }
        // we must rewind only if we got NULL, since we gave the stream to the
        // movie, who may have already started reading from it
        stream->rewind();
        curr = curr->next();
    }
    return NULL;
}

