/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDecodingImageGenerator_DEFINED
#define SkDecodingImageGenerator_DEFINED

#include "SkDiscardableMemory.h"
#include "SkImageGenerator.h"
#include "SkImageInfo.h"

class SkBitmap;
class SkStreamRewindable;

/**
 * Calls into SkImageDecoder::DecodeMemoryToTarget to implement a
 * SkImageGenerator
 */
class SkDecodingImageGenerator : public SkImageGenerator {
public:
    /*
     *  The constructor will take a reference to the SkData.  The
     *  destructor will unref() it.
     */
    explicit SkDecodingImageGenerator(SkData* data);

    /*
     *  The SkData version of this constructor is preferred.  If the
     *  stream has an underlying SkData (such as a SkMemoryStream)
     *  pass that in.
     *
     *  This object will unref the stream when done.  Since streams
     *  have internal state (position), the caller should not pass a
     *  shared stream in.  Pass either a new duplicated stream in or
     *  transfer ownership of the stream.  In the latter case, be sure
     *  that there are no other consumers of the stream who will
     *  modify the stream's position.  This constructor asserts
     *  stream->unique().
     *
     *  For example:
     *    SkStreamRewindable* stream;
     *    ...
     *    SkImageGenerator* gen
     *        = SkNEW_ARGS(SkDecodingImageGenerator,
     *                     (stream->duplicate()));
     *    ...
     *    SkDELETE(gen);
     */
    explicit SkDecodingImageGenerator(SkStreamRewindable* stream);

    virtual ~SkDecodingImageGenerator();

    virtual SkData* refEncodedData() SK_OVERRIDE;

    virtual bool getInfo(SkImageInfo* info) SK_OVERRIDE;

    virtual bool getPixels(const SkImageInfo& info,
                           void* pixels,
                           size_t rowBytes) SK_OVERRIDE;

    /**
     *  Install the SkData into the destination bitmap, using a new
     *  SkDiscardablePixelRef and a new SkDecodingImageGenerator.
     *
     *  @param data Contains the encoded image data that will be used
     *  by the SkDecodingImageGenerator.  Will be ref()ed.
     *
     *  @param destination Upon success, this bitmap will be
     *  configured and have a pixelref installed.
     *
     *  @param factory If not NULL, this object will be used as a
     *  source of discardable memory when decoding.  If NULL, then
     *  SkDiscardableMemory::Create() will be called.
     *
     *  @return true iff successful.
     */
    static bool Install(SkData* data, SkBitmap* destination,
                        SkDiscardableMemory::Factory* factory = NULL);
    /**
     *  Install the stream into the destination bitmap, using a new
     *  SkDiscardablePixelRef and a new SkDecodingImageGenerator.
     *
     *  The SkData version of this function is preferred.  If the
     *  stream has an underlying SkData (such as a SkMemoryStream)
     *  pass that in.
     *
     *  @param stream The source of encoded data that will be passed
     *  to the decoder.  The installed SkDecodingImageGenerator will
     *  unref the stream when done.  If false is returned, this
     *  function will perform the unref.  Since streams have internal
     *  state (position), the caller should not pass a shared stream
     *  in.  Pass either a new duplicated stream in or transfer
     *  ownership of the stream.  In the latter case, be sure that
     *  there are no other consumers of the stream who will modify the
     *  stream's position.  This function will fail if
     *  (!stream->unique()).
     *
     *  @param destination Upon success, this bitmap will be
     *  configured and have a pixelref installed.
     *
     *  @param factory If not NULL, this object will be used as a
     *  source of discardable memory when decoding.  If NULL, then
     *  SkDiscardableMemory::Create() will be called.
     *
     *  @return true iff successful.
     */
    static bool Install(SkStreamRewindable* stream, SkBitmap* destination,
                        SkDiscardableMemory::Factory* factory = NULL);

private:
    SkData*             fData;
    SkStreamRewindable* fStream;
    SkImageInfo         fInfo;
    bool                fHasInfo;
    bool                fDoCopyTo;
};
#endif  // SkDecodingImageGenerator_DEFINED
