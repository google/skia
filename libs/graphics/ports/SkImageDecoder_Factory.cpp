#include "SkImageDecoder.h"
#include "SkStream.h"

/*  Each of these is in their respective SkImageDecoder_libXXX.cpp file
*/
extern SkImageDecoder* SkImageDecoder_GIF_Factory(SkStream*);
extern SkImageDecoder* SkImageDecoder_JPEG_Factory(SkStream*);
extern SkImageDecoder* SkImageDecoder_PNG_Factory(SkStream*);

typedef SkImageDecoder* (*SkImageDecoderFactoryProc)(SkStream*);

static const SkImageDecoderFactoryProc gProc[] = {
    SkImageDecoder_GIF_Factory,
    SkImageDecoder_PNG_Factory,
    SkImageDecoder_JPEG_Factory // JPEG must be last, as it doesn't have a sniffer/detector yet
};

SkImageDecoder* SkImageDecoder::Factory(SkStream* stream)
{
    for (unsigned i = 0; i < SK_ARRAY_COUNT(gProc); i++) {
        SkImageDecoder* codec = gProc[i](stream);
        if (codec)
            return codec;
        stream->rewind();
    }
	return NULL;
}

