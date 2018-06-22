/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCodec_wuffs_gif_DEFINED
#define SkCodec_wuffs_gif_DEFINED

#include "SkCodec.h"

#include "wuffs-v0.2/wuffs-v0.2.h"

#define SK_WUFFS_GIF_CODEC_BUFFER_SIZE 4096

class SkWuffsGifCodec final : public SkCodec {
 public:
  static bool IsGif(const void*, size_t);

  static std::unique_ptr<SkCodec> MakeFromStream(std::unique_ptr<SkStream>,
                                                 Result*);

  ~SkWuffsGifCodec() override;

 protected:
  SkEncodedImageFormat onGetEncodedFormat() const override;

  bool onRewind() override;

  Result onGetPixels(const SkImageInfo&,
                     void*,
                     size_t,
                     const Options&,
                     int*) override;

  Result onStartIncrementalDecode(const SkImageInfo& dstInfo,
                                  void* dst,
                                  size_t rowBytes,
                                  const SkCodec::Options&) override;
  Result onIncrementalDecode(int* rowsDecoded) override;

  int onGetFrameCount() override;
  bool onGetFrameInfo(int, FrameInfo*) const override;
  int onGetRepetitionCount() override;

 private:
  SkWuffsGifCodec(const SkEncodedInfo& encodedInfo,
                  const SkImageInfo& imageInfo,
                  std::unique_ptr<SkStream>,
                  wuffs_gif__decoder* dec,
                  wuffs_base__image_buffer imgbuf,
                  wuffs_base__io_buffer iobuf);

  // TODO(nigeltao): ideally, we'd like to drop the * on the next line of code,
  // to avoid the indirection, and to avoid having to fiddle with malloc and
  // free calls. But the way Skia works is that the SkCodec constructor has to
  // be called *after* decoding the width and height, so we have to allocate
  // the wuffs_gif__decoder (in order to determine that width and height)
  // *before* we construct the SkWuffsGifCodec (as SkWuffsGifCodec inherits
  // from SkCodec), so we have to be indirect here and pass the pointer via the
  // constructor.
  //
  // TODO(nigeltao): use a C++ smart pointer type, such as std::unique_ptr?
  wuffs_gif__decoder* fDecoder;

  wuffs_base__image_buffer fImageBuffer;
  wuffs_base__io_buffer fIOBuffer;

  // Incremental decoding state.
  uint8_t* fIncrDecDst;
  size_t fIncrDecRowBytes;

  // TODO(nigeltao): instead of copying from the SkStream into this buffer, can
  // we access the underlying SkStream's buffer directly, if available (e.g.
  // for mmap'ed files)?
  uint8_t fBuffer[SK_WUFFS_GIF_CODEC_BUFFER_SIZE];

  typedef SkCodec INHERITED;
};

#endif  // SkCodec_wuffs_gif_DEFINED
