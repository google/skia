/* libs/graphics/ports/SkImageDecoder_Factory.cpp
**
** Copyright 2006, Google Inc.
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
#include "SkStream.h"

/*  Each of these is in their respective SkImageDecoder_libXXX.cpp file
*/
extern SkImageDecoder* SkImageDecoder_GIF_Factory(SkStream*);
extern SkImageDecoder* SkImageDecoder_ICO_Factory(SkStream*);
extern SkImageDecoder* SkImageDecoder_JPEG_Factory(SkStream*);
extern SkImageDecoder* SkImageDecoder_PNG_Factory(SkStream*);
extern SkImageDecoder* SkImageDecoder_ZBM_Factory(SkStream*);

typedef SkImageDecoder* (*SkImageDecoderFactoryProc)(SkStream*);

static const SkImageDecoderFactoryProc gProc[] = {
    SkImageDecoder_ZBM_Factory,
    SkImageDecoder_GIF_Factory,
    SkImageDecoder_PNG_Factory,
    SkImageDecoder_ICO_Factory,
    SkImageDecoder_JPEG_Factory // JPEG must be last, as it doesn't have a sniffer/detector yet
};

SkImageDecoder* SkImageDecoder::Factory(SkStream* stream)
{
    for (unsigned i = 0; i < SK_ARRAY_COUNT(gProc); i++) {
        SkImageDecoder* codec = gProc[i](stream);
        stream->rewind();
        if (codec)
            return codec;
    }
    return NULL;
}

