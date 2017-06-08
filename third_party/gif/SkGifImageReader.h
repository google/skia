/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef SkGifImageReader_h
#define SkGifImageReader_h

// Define ourselves as the clientPtr.  Mozilla just hacked their C++ callback class into this old C decoder,
// so we will too.
class SkGifCodec;

#include "SkCodec.h"
#include "SkCodecPriv.h"
#include "SkCodecAnimation.h"
#include "SkColorTable.h"
#include "SkData.h"
#include "SkFrameHolder.h"
#include "SkImageInfo.h"
#include "SkStreamBuffer.h"
#include "../private/SkTArray.h"
#include <memory>
#include <vector>

typedef SkTArray<unsigned char, true> SkGIFRow;

// List of possible parsing states.
enum SkGIFState {
  SkGIFType,
  SkGIFGlobalHeader,
  SkGIFGlobalColormap,
  SkGIFImageStart,
  SkGIFImageHeader,
  SkGIFImageColormap,
  SkGIFImageBody,
  SkGIFLZWStart,
  SkGIFLZW,
  SkGIFSubBlock,
  SkGIFExtension,
  SkGIFControlExtension,
  SkGIFConsumeBlock,
  SkGIFSkipBlock,
  SkGIFDone,
  SkGIFCommentExtension,
  SkGIFApplicationExtension,
  SkGIFNetscapeExtensionBlock,
  SkGIFConsumeNetscapeExtension,
  SkGIFConsumeComment
};

class SkGIFFrameContext;
class SkGIFColorMap;

// LZW decoder state machine.
class SkGIFLZWContext final : public SkNoncopyable {
public:
  SkGIFLZWContext(SkGifCodec* client, const SkGIFFrameContext* frame_context)
      : codesize(0),
        codemask(0),
        clear_code(0),
        avail(0),
        oldcode(0),
        firstchar(0),
        bits(0),
        datum(0),
        ipass(0),
        irow(0),
        rows_remaining(0),
        row_iter(0),
        client_(client),
        frame_context_(frame_context) {}

  bool PrepareToDecode();
  bool OutputRow(const unsigned char* row_begin);
  bool DoLZW(const unsigned char* block, size_t bytes_in_block);
  bool HasRemainingRows() { return SkToBool(rows_remaining); }

 private:
  enum {
    kMaxDictionaryEntryBits = 12,
    // 2^kMaxDictionaryEntryBits
    kMaxDictionaryEntries = 4096,
  };

  // LZW decoding states and output states.
  int codesize;
  int codemask;
  int clear_code;  // Codeword used to trigger dictionary reset.
  int avail;      // Index of next available slot in dictionary.
  int oldcode;
  unsigned char firstchar;
  int bits;              // Number of unread bits in "datum".
  int datum;             // 32-bit input buffer.
  int ipass;             // Interlace pass; Ranges 1-4 if interlaced.
  size_t irow;           // Current output row, starting at zero.
  size_t rows_remaining;  // Rows remaining to be output.

  unsigned short prefix[kMaxDictionaryEntries];
  unsigned char suffix[kMaxDictionaryEntries];
  unsigned short suffix_length[kMaxDictionaryEntries];
  SkGIFRow row_buffer; // Single scanline temporary buffer.
  unsigned char* row_iter;

  SkGifCodec* const client_;
  const SkGIFFrameContext* const frame_context_;
};

// Data structure for one LZW block.
struct SkGIFLZWBlock {
 public:
  SkGIFLZWBlock(size_t position, size_t size)
      : block_position(position), block_size(size) {}
  size_t block_position;
  size_t block_size;
};

class SkGIFColorMap final {
 public:
  static constexpr int kNotFound = -1;
  SkGIFColorMap()
      : is_defined_(false),
        position_(0),
        colors_(0),
        trans_pixel_(kNotFound),
        pack_color_proc_(nullptr) {}

  void SetNumColors(int colors) {
    SkASSERT(!colors_);
    SkASSERT(!position_);

    colors_ = colors;
  }

  void SetTablePosition(size_t position) {
    SkASSERT(!is_defined_);

    position_ = position;
    is_defined_ = true;
  }

  int NumColors() const { return colors_; }

  bool IsDefined() const { return is_defined_; }

  // Build RGBA table using the data stream.
  sk_sp<SkColorTable> BuildTable(SkStreamBuffer*, SkColorType dst_color_type,
                                 int transparent_pixel) const;

 private:
  bool                         is_defined_;
  size_t                       position_;
  int                          colors_;
  // Cached values. If these match on a new request, we can reuse table_.
  mutable int                  trans_pixel_;
  mutable PackColorProc        pack_color_proc_;
  mutable sk_sp<SkColorTable>  table_;
};

class SkGifImageReader;

// LocalFrame output state machine.
class SkGIFFrameContext : public SkFrame {
 public:
  SkGIFFrameContext(SkGifImageReader* reader, int id)
      : INHERITED(id),
        owner_(reader),
        transparent_pixel_(SkGIFColorMap::kNotFound),
        data_size_(0),
        progressive_display_(false),
        interlaced_(false),
        current_lzw_block_(0),
        is_complete_(false),
        is_header_defined_(false),
        is_data_size_defined_(false) {}

  ~SkGIFFrameContext() override {}

  void AddLzwBlock(size_t position, size_t size) {
    lzw_blocks_.push_back(SkGIFLZWBlock(position, size));
  }
  bool Decode(SkStreamBuffer*, SkGifCodec* client, bool* frame_decoded);

  int TransparentPixel() const { return transparent_pixel_; }
  void SetTransparentPixel(size_t pixel) { transparent_pixel_ = pixel; }
  bool IsComplete() const { return is_complete_; }
  void SetComplete() { is_complete_ = true; }
  bool IsHeaderDefined() const { return is_header_defined_; }
  void SetHeaderDefined() { is_header_defined_ = true; }
  bool IsDataSizeDefined() const { return is_data_size_defined_; }
  int DataSize() const { return data_size_; }
  void SetDataSize(int size) {
    data_size_ = size;
    is_data_size_defined_ = true;
  }
  bool ProgressiveDisplay() const { return progressive_display_; }
  void SetProgressiveDisplay(bool progressive_display) {
    progressive_display_ = progressive_display;
  }
  bool Interlaced() const { return interlaced_; }
  void SetInterlaced(bool interlaced) { interlaced_ = interlaced; }

  void ClearDecodeState() { lzw_context_.reset(); }
  const SkGIFColorMap& LocalColorMap() const { return local_color_map_; }
  SkGIFColorMap& LocalColorMap() { return local_color_map_; }

 protected:
  bool onReportsAlpha() const override;

 private:
  // Unowned pointer to the object that owns this frame.
  const SkGifImageReader* const owner_;
  int transparent_pixel_;  // Index of transparent pixel. Value is kNotFound
                           // if there is no transparent pixel.
  int data_size_;

  bool progressive_display_;  // If true, do Haeberli interlace hack.
  bool interlaced_;           // True, if scanlines arrive interlaced order.

  std::unique_ptr<SkGIFLZWContext> lzw_context_;
  std::vector<SkGIFLZWBlock> lzw_blocks_;  // LZW blocks for this frame.
  SkGIFColorMap local_color_map_;

  int current_lzw_block_;
  bool is_complete_;
  bool is_header_defined_;
  bool is_data_size_defined_;

  typedef SkFrame INHERITED;
};

class SkGifImageReader final : public SkFrameHolder {
 public:
  // This takes ownership of stream.
  SkGifImageReader(SkStream* stream)
      : client_(nullptr),
        state_(SkGIFType),
        // Number of bytes for GIF type, either "GIF87a" or "GIF89a".
        bytes_to_consume_(6),
        version_(0),
        loop_count_(kLoopCountNotSeen),
        stream_buffer_(stream),
        parse_completed_(false),
        first_frame_has_alpha_(false),
        first_frame_supports_index8_(false) {}

  ~SkGifImageReader() override {}

  void SetClient(SkGifCodec* client) { client_ = client; }

  // Option to pass to parse(). All enums are negative, because a non-negative value is used to
  // indicate that the Reader should parse up to and including the frame indicated.
  enum SkGIFParseQuery {
    // Parse enough to determine the size. Note that this parses the first frame's header,
    // since we may decide to expand based on the frame's dimensions.
    SkGIFSizeQuery        = -1,
    // Parse to the end, so we know about all frames.
    SkGIFFrameCountQuery  = -2,
    // Parse until we see the loop count.
    SkGIFLoopCountQuery   = -3,
  };

  // Parse incoming GIF data stream into internal data structures.
  // Non-negative values are used to indicate to parse through that frame.
  // Return true if parsing has progressed or there is not enough data.
  // Return false if a fatal error is encountered.
  bool Parse(SkGIFParseQuery);

  // Decode the frame indicated by frameIndex.
  // frameComplete will be set to true if the frame is completely decoded.
  // The method returns false if there is an error.
  bool Decode(int frame_index, bool* frameComplete);

  int ImagesCount() const {
    // Report the first frame immediately, so the parser can stop when it
    // sees the size on a SizeQuery.
    const size_t frames = frames_.size();
    if (frames <= 1) {
      return static_cast<int>(frames);
    }

    // This avoids counting an empty frame when the file is truncated (or
    // simply not yet complete) after receiving SkGIFControlExtension (and
    // possibly SkGIFImageHeader) but before reading the color table. This
    // ensures that we do not count a frame before we know its required
    // frame.
    return static_cast<int>(frames_.back()->reachedStartOfData() ?
                            frames : frames - 1);
  }
  int LoopCount() const {
    if (kLoopCountNotSeen == loop_count_) {
      return 0;
    }
    return loop_count_;
  }

  const SkGIFColorMap& GlobalColorMap() const { return global_color_map_; }

  const SkGIFFrameContext* FrameContext(int index) const {
    return index >= 0 && index < static_cast<int>(frames_.size())
      ? frames_[index].get() : 0;
  }

  // FIXME (scroggo): Not necessary?
  //bool ParseCompleted() const { return parse_completed_; }

  void ClearDecodeState() {
    for (size_t index = 0; index < frames_.size(); index++) {
      frames_[index]->ClearDecodeState();
    }
  }

  // Return the color table for frame index (which may be the global color table).
  sk_sp<SkColorTable> GetColorTable(SkColorType dst_color_type, int index);

  bool FirstFrameHasAlpha() const { return first_frame_has_alpha_; }

  bool FirstFrameSupportsIndex8() const { return first_frame_supports_index8_; }

  // Helper function that returns whether an SkGIFFrameContext has transparency.
  // This method is sometimes called before creating one/parsing its color map,
  // so it cannot rely on SkGIFFrameContext::transparentPixel or ::localColorMap().
  bool HasTransparency(int trans_pix, bool has_local_color_map,
                       int local_map_colors) const;

 protected:
  const SkFrame* onGetFrame(int i) const override {
    return static_cast<const SkFrame*>(this->FrameContext(i));
  }

 private:
  // Requires that one byte has been buffered into stream_buffer_.
  unsigned char getOneByte() const {
    return reinterpret_cast<const unsigned char*>(stream_buffer_.get())[0];
  }

  void AddFrameIfNecessary();
  bool CurrentFrameIsFirstFrame() const {
    return frames_.empty() ||
           (frames_.size() == 1u && !frames_[0]->IsComplete());
  }

  // Unowned pointer
  SkGifCodec* client_;

  // Parsing state machine.
  SkGIFState state_;         // Current decoder master state.
  size_t bytes_to_consume_;  // Number of bytes to consume for next stage of
                             // parsing.

  // Global (multi-image) state.
  int version_;              // Either 89 for GIF89 or 87 for GIF87.
  SkGIFColorMap global_color_map_;

  static constexpr int kLoopCountNotSeen = -2;
  int loop_count_;  // Netscape specific extension block to control the number
                    // of animation loops a GIF renders.

  std::vector<std::unique_ptr<SkGIFFrameContext>> frames_;

  SkStreamBuffer stream_buffer_;
  bool parse_completed_;
  // These values can be computed before we create a SkGIFFrameContext, so we
  // store them here instead of on frames_[0].
  bool first_frame_has_alpha_;
  bool first_frame_supports_index8_;
};

#endif
