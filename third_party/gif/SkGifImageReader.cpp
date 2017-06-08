/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Chris Saari <saari@netscape.com>
 *   Apple Computer
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

/*
The Graphics Interchange Format(c) is the copyright property of CompuServe
Incorporated. Only CompuServe Incorporated is authorized to define, redefine,
enhance, alter, modify or change in any way the definition of the format.

CompuServe Incorporated hereby grants a limited, non-exclusive, royalty-free
license for the use of the Graphics Interchange Format(sm) in computer
software; computer software utilizing GIF(sm) must acknowledge ownership of the
Graphics Interchange Format and its Service Mark by CompuServe Incorporated, in
User and Technical Documentation. Computer software utilizing GIF, which is
distributed or may be distributed without User or Technical Documentation must
display to the screen or printer a message acknowledging ownership of the
Graphics Interchange Format and the Service Mark by CompuServe Incorporated; in
this case, the acknowledgement may be displayed in an opening screen or leading
banner, or a closing screen or trailing banner. A message such as the following
may be used:

    "The Graphics Interchange Format(c) is the Copyright property of
    CompuServe Incorporated. GIF(sm) is a Service Mark property of
    CompuServe Incorporated."

For further information, please contact :

    CompuServe Incorporated
    Graphics Technology Department
    5000 Arlington Center Boulevard
    Columbus, Ohio  43220
    U. S. A.

CompuServe Incorporated maintains a mailing list with all those individuals and
organizations who wish to receive copies of this document when it is corrected
or revised. This service is offered free of charge; please provide us with your
mailing address.
*/

#include "SkGifImageReader.h"
#include "SkColorPriv.h"
#include "SkGifCodec.h"

#include <algorithm>
#include <string.h>


namespace {

static constexpr int kMaxColors = 256u;
static constexpr int kBytesPerColormapEntry = 3;

}  // namespace

// GETN(n, s) requests at least 'n' bytes available from 'q', at start of state
// 's'.
//
// Note: the hold will never need to be bigger than 256 bytes, as each GIF block
// (except colormaps) can never be bigger than 256 bytes. Colormaps are directly
// copied in the resp. global_colormap or dynamically allocated local_colormap,
// so a fixed buffer in SkGIFImageReader is good enough. This buffer is only
// needed to copy left-over data from one GifWrite call to the next.
#define GETN(n, s)           \
  do {                       \
    bytes_to_consume_ = (n); \
    state_ = (s);            \
  } while (0)

// Get a 16-bit value stored in little-endian format.
#define GETINT16(p) ((p)[1] << 8 | (p)[0])

// Send the data to the display front-end.
bool SkGIFLZWContext::OutputRow(const unsigned char* row_begin) {
  int drow_start = irow;
  int drow_end = irow;

  // Haeberli-inspired hack for interlaced GIFs: Replicate lines while
  // displaying to diminish the "venetian-blind" effect as the image is
  // loaded. Adjust pixel vertical positions to avoid the appearance of the
  // image crawling up the screen as successive passes are drawn.
  if (frame_context_->ProgressiveDisplay() && frame_context_->Interlaced() &&
      ipass < 4) {
    unsigned row_dup = 0;
    unsigned row_shift = 0;

    switch (ipass) {
      case 1:
        row_dup = 7;
        row_shift = 3;
        break;
      case 2:
        row_dup = 3;
        row_shift = 1;
        break;
      case 3:
        row_dup = 1;
        row_shift = 0;
        break;
      default:
        break;
    }

    drow_start -= row_shift;
    drow_end = drow_start + row_dup;

    // Extend if bottom edge isn't covered because of the shift upward.
    if ((unsigned)((frame_context_->height() - 1) - drow_end) <= row_shift)
      drow_end = frame_context_->height() - 1;

    // Clamp first and last rows to upper and lower edge of image.
    if (drow_start < 0)
      drow_start = 0;

    if (drow_end >= frame_context_->height())
      drow_end = frame_context_->height() - 1;
  }

  // Protect against too much image data.
  if (drow_start >= frame_context_->height())
    return true;

  // CALLBACK: Let the client know we have decoded a row.
  const bool write_transparent_pixels = (SkCodec::kNone == frame_context_->getRequiredFrame());
  if (!client_->haveDecodedRow(frame_context_->frameId(), row_begin,
                                drow_start, drow_end - drow_start + 1,
                                write_transparent_pixels)) {
    return false;
  }

  if (!frame_context_->Interlaced())
    irow++;
  else {
    do {
      switch (ipass) {
        case 1:
          irow += 8;
          if (irow >= (unsigned) frame_context_->height()) {
            ipass++;
            irow = 4;
          }
          break;

        case 2:
          irow += 8;
          if (irow >= (unsigned) frame_context_->height()) {
            ipass++;
            irow = 2;
          }
          break;

        case 3:
          irow += 4;
          if (irow >= (unsigned) frame_context_->height()) {
            ipass++;
            irow = 1;
          }
          break;

        case 4:
          irow += 2;
          if (irow >= (unsigned) frame_context_->height()) {
            ipass++;
            irow = 0;
          }
          break;

        default:
          break;
      }
    } while (irow > (unsigned) (frame_context_->height() - 1));
  }
  return true;
}

sk_sp<SkColorTable> SkGIFColorMap::BuildTable(SkStreamBuffer* stream_buffer,
                                              SkColorType color_type,
                                              int transparent_pixel) const {
  if (!is_defined_)
    return nullptr;

  const PackColorProc proc = choose_pack_color_proc(false, color_type);
  if (table_ && proc == pack_color_proc_ && trans_pixel_ == transparent_pixel) {
    SkASSERT(transparent_pixel == kNotFound || transparent_pixel > table_->count()
             || table_->operator[](transparent_pixel) == SK_ColorTRANSPARENT);
    // This SkColorTable has already been built with the same transparent color and
    // packing proc. Reuse it.
    return table_;
  }
  pack_color_proc_ = proc;
  trans_pixel_ = transparent_pixel;
  const size_t bytes = colors_ * kBytesPerColormapEntry;
  sk_sp<SkData> raw_data(stream_buffer->getDataAtPosition(position_, bytes));
  if (!raw_data) {
    return nullptr;
  }
  SkASSERT(colors_ <= kMaxColors);
  const uint8_t* src_color_map = raw_data->bytes();
  SkPMColor colorStorage[kMaxColors];
  for (int i = 0; i < colors_; i++) {
    if (i == transparent_pixel) {
      colorStorage[i] = SK_ColorTRANSPARENT;
    } else {
      colorStorage[i] = proc(255, src_color_map[0], src_color_map[1], src_color_map[2]);
    }
    src_color_map += kBytesPerColormapEntry;
  }
  for (int i = colors_; i < kMaxColors; i++) {
    colorStorage[i] = SK_ColorTRANSPARENT;
  }
  table_ = sk_sp<SkColorTable>(new SkColorTable(colorStorage, kMaxColors));
  return table_;
}

// Performs Lempel-Ziv-Welch decoding. Returns whether decoding was successful.
// If successful, the block will have been completely consumed and/or
// rowsRemaining will be 0.
bool SkGIFLZWContext::DoLZW(const unsigned char* block, size_t bytes_in_block) {
  const int width = frame_context_->width();

  if (row_iter == row_buffer.end())
    return true;

  for (const unsigned char* ch = block; bytes_in_block-- > 0; ch++) {
    // Feed the next byte into the decoder's 32-bit input buffer.
    datum += ((int)*ch) << bits;
    bits += 8;

    // Check for underflow of decoder's 32-bit input buffer.
    while (bits >= codesize) {
      // Get the leading variable-length symbol from the data stream.
      int code = datum & codemask;
      datum >>= codesize;
      bits -= codesize;

      // Reset the dictionary to its original state, if requested.
      if (code == clear_code) {
        codesize = frame_context_->DataSize() + 1;
        codemask = (1 << codesize) - 1;
        avail = clear_code + 2;
        oldcode = -1;
        continue;
      }

      // Check for explicit end-of-stream code.
      if (code == (clear_code + 1)) {
        // end-of-stream should only appear after all image data.
        if (!rows_remaining)
          return true;
        return false;
      }

      const int temp_code = code;
      unsigned short code_length = 0;
      if (code < avail) {
        // This is a pre-existing code, so we already know what it
        // encodes.
        code_length = suffix_length[code];
        row_iter += code_length;
      } else if (code == avail && oldcode != -1) {
        // This is a new code just being added to the dictionary.
        // It must encode the contents of the previous code, plus
        // the first character of the previous code again.
        code_length = suffix_length[oldcode] + 1;
        row_iter += code_length;
        *--row_iter = firstchar;
        code = oldcode;
      } else {
        // This is an invalid code. The dictionary is just initialized
        // and the code is incomplete. We don't know how to handle
        // this case.
        return false;
      }

      while (code >= clear_code) {
        *--row_iter = suffix[code];
        code = prefix[code];
      }

      *--row_iter = firstchar = suffix[code];

      // Define a new codeword in the dictionary as long as we've read
      // more than one value from the stream.
      if (avail < kMaxDictionaryEntries && oldcode != -1) {
        prefix[avail] = oldcode;
        suffix[avail] = firstchar;
        suffix_length[avail] = suffix_length[oldcode] + 1;
        ++avail;

        // If we've used up all the codewords of a given length
        // increase the length of codewords by one bit, but don't
        // exceed the specified maximum codeword size.
        if (!(avail & codemask) && avail < kMaxDictionaryEntries) {
          ++codesize;
          codemask += avail;
        }
      }
      oldcode = temp_code;
      row_iter += code_length;

      // Output as many rows as possible.
      unsigned char* row_begin = row_buffer.begin();
      for (; row_begin + width <= row_iter; row_begin += width) {
        if (!OutputRow(row_begin))
          return false;
        rows_remaining--;
        if (!rows_remaining)
          return true;
      }

      if (row_begin != row_buffer.begin()) {
        // Move the remaining bytes to the beginning of the buffer.
        const size_t bytes_to_copy = row_iter - row_begin;
        memcpy(row_buffer.begin(), row_begin, bytes_to_copy);
        row_iter = row_buffer.begin() + bytes_to_copy;
      }
    }
  }
  return true;
}

sk_sp<SkColorTable> SkGifImageReader::GetColorTable(SkColorType color_type, int index) {
  if (index < 0 || static_cast<size_t>(index) >= frames_.size()) {
    return nullptr;
  }

  const SkGIFFrameContext* frame_context = frames_[index].get();
  const SkGIFColorMap& local_color_map = frame_context->LocalColorMap();
  const int trans_pix = frame_context->TransparentPixel();
  if (local_color_map.IsDefined()) {
    return local_color_map.BuildTable(&stream_buffer_, color_type, trans_pix);
  }
  if (global_color_map_.IsDefined()) {
    return global_color_map_.BuildTable(&stream_buffer_, color_type, trans_pix);
  }
  return nullptr;
}

// Decodes this frame. |frameDecoded| will be set to true if the entire frame is
// decoded. Returns true if decoding progressed further than before without
// error, or there is insufficient new data to decode further. Otherwise, a
// decoding error occurred; returns false in this case.
bool SkGIFFrameContext::Decode(SkStreamBuffer* stream_buffer,
                               SkGifCodec* client,
                               bool* frame_decoded) {
  *frame_decoded = false;
  if (!lzw_context_) {
    // Wait for more data to properly initialize SkGIFLZWContext.
    if (!IsDataSizeDefined() || !IsHeaderDefined())
      return true;

    lzw_context_.reset(new SkGIFLZWContext(client, this));
    if (!lzw_context_->PrepareToDecode()) {
      lzw_context_.reset();
      return false;
    }

    current_lzw_block_ = 0;
  }

  // Some bad GIFs have extra blocks beyond the last row, which we don't want to
  //decode.
  while (static_cast<size_t>(current_lzw_block_) < lzw_blocks_.size() &&
         lzw_context_->HasRemainingRows()) {
    const auto& block = lzw_blocks_[current_lzw_block_];
    const size_t len = block.block_size;

    sk_sp<SkData> data(stream_buffer->getDataAtPosition(block.block_position, len));
    if (!data) {
      return false;
    }
    if (!lzw_context_->DoLZW(reinterpret_cast<const unsigned char*>(data->data()), len)) {
      return false;
    }
    ++current_lzw_block_;
  }

  // If this frame is data complete then the previous loop must have completely
  // decoded all LZW blocks.
  // There will be no more decoding for this frame so it's time to cleanup.
  if (IsComplete()) {
    *frame_decoded = true;
    lzw_context_.reset();
  }
  return true;
}

// Decodes a frame using SkGIFFrameContext:Decode(). Returns true if decoding has
// progressed, or false if an error has occurred.
bool SkGifImageReader::Decode(int frame_index, bool* frame_decoded) {
  SkGIFFrameContext* current_frame = frames_[frame_index].get();

  return current_frame->Decode(&stream_buffer_, client_, frame_decoded);
}

// Parse incoming GIF data stream into internal data structures.
// Return true if parsing has progressed or there is not enough data.
// Return false if a fatal error is encountered.
bool SkGifImageReader::Parse(SkGifImageReader::SkGIFParseQuery query) {
  if (parse_completed_) {
    return true;
  }

  if (SkGIFLoopCountQuery == query && loop_count_ != kLoopCountNotSeen) {
    // Loop count has already been parsed.
    return true;
  }

  // SkGIFSizeQuery and SkGIFFrameCountQuery are negative, so this is only meaningful when >= 0.
  const int last_frame_to_parse = (int) query;
  if (last_frame_to_parse >= 0 && (int) frames_.size() > last_frame_to_parse &&
      frames_[last_frame_to_parse]->IsComplete()) {
    // We have already parsed this frame.
    return true;
  }

  while (true) {
    if (!stream_buffer_.buffer(bytes_to_consume_)) {
      // The stream does not yet have enough data.
      return true;
    }

    switch (state_) {
      case SkGIFLZW: {
        SkASSERT(!frames_.empty());
        auto* frame = frames_.back().get();
        frame->AddLzwBlock(stream_buffer_.markPosition(), bytes_to_consume_);
        GETN(1, SkGIFSubBlock);
        break;
      }
      case SkGIFLZWStart: {
        SkASSERT(!frames_.empty());
        auto* current_frame = frames_.back().get();

        current_frame->SetDataSize(this->getOneByte());
        GETN(1, SkGIFSubBlock);
        break;
      }

      case SkGIFType: {
        const char* current_component = stream_buffer_.get();

        // All GIF files begin with "GIF87a" or "GIF89a".
        if (!memcmp(current_component, "GIF89a", 6))
          version_ = 89;
        else if (!memcmp(current_component, "GIF87a", 6))
          version_ = 87;
        else {
          // This prevents attempting to continue reading this invalid stream.
          GETN(0, SkGIFDone);
          return false;
        }
        GETN(7, SkGIFGlobalHeader);
        break;
      }

      case SkGIFGlobalHeader: {
        const unsigned char* current_component =
            reinterpret_cast<const unsigned char*>(stream_buffer_.get());

        // This is the height and width of the "screen" or frame into which
        // images are rendered. The individual images can be smaller than
        // the screen size and located with an origin anywhere within the
        // screen.
        // Note that we don't inform the client of the size yet, as it might
        // change after we read the first frame's image header.
        fScreenWidth = GETINT16(current_component);
        fScreenHeight = GETINT16(current_component + 2);

        const int global_color_map_colors = 2 << (current_component[4] & 0x07);

        if ((current_component[4] & 0x80) && global_color_map_colors > 0) { /* global map */
          global_color_map_.SetNumColors(global_color_map_colors);
          GETN(kBytesPerColormapEntry * global_color_map_colors, SkGIFGlobalColormap);
          break;
        }

        GETN(1, SkGIFImageStart);
        break;
      }

      case SkGIFGlobalColormap: {
        global_color_map_.SetTablePosition(stream_buffer_.markPosition());
        GETN(1, SkGIFImageStart);
        break;
      }

      case SkGIFImageStart: {
        const char current_component = stream_buffer_.get()[0];

        if (current_component == '!') { // extension.
          GETN(2, SkGIFExtension);
          break;
        }

        if (current_component == ',') { // image separator.
          GETN(9, SkGIFImageHeader);
          break;
        }

        // If we get anything other than ',' (image separator), '!'
        // (extension), or ';' (trailer), there is extraneous data
        // between blocks. The GIF87a spec tells us to keep reading
        // until we find an image separator, but GIF89a says such
        // a file is corrupt. We follow Mozilla's implementation and
        // proceed as if the file were correctly terminated, so the
        // GIF will display.
        GETN(0, SkGIFDone);
        break;
      }

      case SkGIFExtension: {
        const unsigned char* current_component =
          reinterpret_cast<const unsigned char*>(stream_buffer_.get());

        size_t bytes_in_block = current_component[1];
        SkGIFState exception_state = SkGIFSkipBlock;

        switch (*current_component) {
          case 0xf9:
            exception_state = SkGIFControlExtension;
            // The GIF spec mandates that the GIFControlExtension header block
            // length is 4 bytes, and the parser for this block reads 4 bytes,
            // so we must enforce that the buffer contains at least this many
            // bytes. If the GIF specifies a different length, we allow that, so
            // long as it's larger; the additional data will simply be ignored.
            bytes_in_block = std::max(bytes_in_block, static_cast<size_t>(4));
            break;

            // The GIF spec also specifies the lengths of the following two
            // extensions' headers (as 12 and 11 bytes, respectively). Because we
            // ignore the plain text extension entirely and sanity-check the
            // actual length of the application extension header before reading
            // it, we allow GIFs to deviate from these values in either direction.
            // This is important for real-world compatibility, as GIFs in the wild
            // exist with application extension headers that are both shorter and
            // longer than 11 bytes.
          case 0x01:
            // ignoring plain text extension
            break;

          case 0xff:
            exception_state = SkGIFApplicationExtension;
            break;

          case 0xfe:
            exception_state = SkGIFConsumeComment;
            break;
        }

        if (bytes_in_block)
          GETN(bytes_in_block, exception_state);
        else
          GETN(1, SkGIFImageStart);
        break;
      }

      case SkGIFConsumeBlock: {
        const unsigned char current_component = this->getOneByte();
        if (!current_component)
          GETN(1, SkGIFImageStart);
        else
          GETN(current_component, SkGIFSkipBlock);
        break;
      }

      case SkGIFSkipBlock: {
        GETN(1, SkGIFConsumeBlock);
        break;
      }

      case SkGIFControlExtension: {
        const unsigned char* current_component =
            reinterpret_cast<const unsigned char*>(stream_buffer_.get());

        AddFrameIfNecessary();
        SkGIFFrameContext* current_frame = frames_.back().get();
        if (*current_component & 0x1)
          current_frame->SetTransparentPixel(current_component[3]);

          // We ignore the "user input" bit.

          // NOTE: This relies on the values in the FrameDisposalMethod enum
          // matching those in the GIF spec!
          int raw_disposal_method = ((*current_component) >> 2) & 0x7;
          switch (raw_disposal_method) {
            case 1:
            case 2:
            case 3:
              current_frame->setDisposalMethod((SkCodecAnimation::DisposalMethod) raw_disposal_method);
              break;
            case 4:
              // Some specs say that disposal method 3 is "overwrite previous", others that setting
              // the third bit of the field (i.e. method 4) is. We map both to the same value.
              current_frame->setDisposalMethod(SkCodecAnimation::DisposalMethod::kRestorePrevious);
              break;
            default:
              // Other values use the default.
              current_frame->setDisposalMethod(SkCodecAnimation::DisposalMethod::kKeep);
              break;
          }
          current_frame->setDuration(GETINT16(current_component + 1) * 10);
          GETN(1, SkGIFConsumeBlock);
          break;
      }

      case SkGIFCommentExtension: {
        const unsigned char current_component = this->getOneByte();
        if (current_component)
          GETN(current_component, SkGIFConsumeComment);
        else
          GETN(1, SkGIFImageStart);
        break;
      }

      case SkGIFConsumeComment: {
        GETN(1, SkGIFCommentExtension);
        break;
      }

      case SkGIFApplicationExtension: {
        // Check for netscape application extension.
        if (bytes_to_consume_ == 11) {
          const unsigned char* current_component =
              reinterpret_cast<const unsigned char*>(stream_buffer_.get());

          if (!memcmp(current_component, "NETSCAPE2.0", 11) || !memcmp(current_component, "ANIMEXTS1.0", 11))
            GETN(1, SkGIFNetscapeExtensionBlock);
        }

        if (state_ != SkGIFNetscapeExtensionBlock)
          GETN(1, SkGIFConsumeBlock);
        break;
      }

      // Netscape-specific GIF extension: animation looping.
      case SkGIFNetscapeExtensionBlock: {
        const int current_component = this->getOneByte();
        // SkGIFConsumeNetscapeExtension always reads 3 bytes from the stream; we
        // should at least wait for this amount.
        if (current_component)
          GETN(std::max(3, current_component), SkGIFConsumeNetscapeExtension);
        else
          GETN(1, SkGIFImageStart);
        break;
      }

      // Parse netscape-specific application extensions
      case SkGIFConsumeNetscapeExtension: {
        const unsigned char* current_component =
            reinterpret_cast<const unsigned char*>(stream_buffer_.get());

        int netscape_extension = current_component[0] & 7;

        // Loop entire animation specified # of times. Only read the loop count
        // during the first iteration.
        if (netscape_extension == 1) {
          loop_count_ = GETINT16(current_component + 1);

          // Zero loop count is infinite animation loop request.
          if (!loop_count_)
            loop_count_ = SkCodec::kRepetitionCountInfinite;

          GETN(1, SkGIFNetscapeExtensionBlock);

          if (SkGIFLoopCountQuery == query) {
            stream_buffer_.flush();
            return true;
          }
        } else if (netscape_extension == 2) {
          // Wait for specified # of bytes to enter buffer.

          // Don't do this, this extension doesn't exist (isn't used at all)
          // and doesn't do anything, as our streaming/buffering takes care of it all...
          // See: http://semmix.pl/color/exgraf/eeg24.htm
          GETN(1, SkGIFNetscapeExtensionBlock);
        } else {
          // 0,3-7 are yet to be defined netscape extension codes
          // This prevents attempting to continue reading this invalid stream.
          GETN(0, SkGIFDone);
          return false;
        }
        break;
      }

      case SkGIFImageHeader: {
        int height, width, xOffset, yOffset;
        const unsigned char* current_component =
            reinterpret_cast<const unsigned char*>(stream_buffer_.get());

        /* Get image offsets, with respect to the screen origin */
        xOffset = GETINT16(current_component);
        yOffset = GETINT16(current_component + 2);

        /* Get image width and height. */
        width  = GETINT16(current_component + 4);
        height = GETINT16(current_component + 6);

        // Some GIF files have frames that don't fit in the specified
        // overall image size. For the first frame, we can simply enlarge
        // the image size to allow the frame to be visible.  We can't do
        // this on subsequent frames because the rest of the decoding
        // infrastructure assumes the image size won't change as we
        // continue decoding, so any subsequent frames that are even
        // larger will be cropped.
        // Luckily, handling just the first frame is sufficient to deal
        // with most cases, e.g. ones where the image size is erroneously
        // set to zero, since usually the first frame completely fills
        // the image.
        if (CurrentFrameIsFirstFrame()) {
          fScreenHeight = std::max(fScreenHeight, yOffset + height);
          fScreenWidth = std::max(fScreenWidth, xOffset + width);
        }

        // NOTE: Chromium placed this block after setHeaderDefined, down
        // below we returned true when asked for the size. So Chromium
        // created an image which would fail. Is this the correct behavior?
        // We choose to return false early, so we will not create an
        // SkCodec.

        // Work around more broken GIF files that have zero image width or
        // height.
        if (!height || !width) {
          height = fScreenHeight;
          width = fScreenWidth;
          if (!height || !width) {
            // This prevents attempting to continue reading this invalid stream.
            GETN(0, SkGIFDone);
            return false;
          }
        }

        const bool is_local_colormap_defined = SkToBool(current_component[8] & 0x80);
        // The three low-order bits of current_component[8] specify the bits per pixel.
        const int num_colors = 2 << (current_component[8] & 0x7);
        if (CurrentFrameIsFirstFrame()) {
          const int trans_pix = frames_.empty() ? SkGIFColorMap::kNotFound
                                                : frames_[0]->TransparentPixel();
          if (this->HasTransparency(trans_pix, is_local_colormap_defined, num_colors)) {
            first_frame_has_alpha_ = true;
            first_frame_supports_index8_ = true;
          } else {
            const bool frameIsSubset = xOffset > 0 || yOffset > 0
                || width < fScreenWidth
                || height < fScreenHeight;
            first_frame_has_alpha_ = frameIsSubset;
            first_frame_supports_index8_ = !frameIsSubset;
          }
        }

        AddFrameIfNecessary();
        SkGIFFrameContext* current_frame = frames_.back().get();
        current_frame->SetHeaderDefined();

        if (query == SkGIFSizeQuery) {
          // The decoder needs to stop, so we return here, before
          // flushing the buffer. Next time through, we'll be in the same
          // state, requiring the same amount in the buffer.
          return true;
        }


        current_frame->setXYWH(xOffset, yOffset, width, height);
        current_frame->SetInterlaced(SkToBool(current_component[8] & 0x40));

        // Overlaying interlaced, transparent GIFs over
        // existing image data using the Haeberli display hack
        // requires saving the underlying image in order to
        // avoid jaggies at the transparency edges. We are
        // unprepared to deal with that, so don't display such
        // images progressively. Which means only the first
        // frame can be progressively displayed.
        // FIXME: It is possible that a non-transparent frame
        // can be interlaced and progressively displayed.
        current_frame->SetProgressiveDisplay(CurrentFrameIsFirstFrame());

        if (is_local_colormap_defined) {
          current_frame->LocalColorMap().SetNumColors(num_colors);
          GETN(kBytesPerColormapEntry * num_colors, SkGIFImageColormap);
          break;
        }

        setAlphaAndRequiredFrame(current_frame);
        GETN(1, SkGIFLZWStart);
        break;
      }

      case SkGIFImageColormap: {
        SkASSERT(!frames_.empty());
        auto* current_frame = frames_.back().get();
        auto& cmap = current_frame->LocalColorMap();
        cmap.SetTablePosition(stream_buffer_.markPosition());
        setAlphaAndRequiredFrame(current_frame);
        GETN(1, SkGIFLZWStart);
        break;
      }

      case SkGIFSubBlock: {
        const size_t bytes_in_block = this->getOneByte();
        if (bytes_in_block)
          GETN(bytes_in_block, SkGIFLZW);
        else {
          // Finished parsing one frame; Process next frame.
          SkASSERT(!frames_.empty());
          // Note that some broken GIF files do not have enough LZW blocks to
          // fully decode all rows; we treat this case as "frame complete".
          frames_.back()->SetComplete();
          GETN(1, SkGIFImageStart);
          if (last_frame_to_parse >= 0 && (int) frames_.size() > last_frame_to_parse) {
            stream_buffer_.flush();
            return true;
          }
        }
        break;
      }

      case SkGIFDone: {
        parse_completed_ = true;
        return true;
      }

      default:
        // We shouldn't ever get here.
        // This prevents attempting to continue reading this invalid stream.
        GETN(0, SkGIFDone);
        return false;
        break;
      }   // switch
      stream_buffer_.flush();
  }

  return true;
}

bool SkGifImageReader::HasTransparency(int transparent_pixel, bool is_local_colormap_defined,
                                       int local_colors) const {
  const int global_colors = global_color_map_.NumColors();
  if (!is_local_colormap_defined && global_colors == 0) {
    // No color table for this frame, so it is completely transparent.
    return true;
  }

  if (transparent_pixel < 0) {
    SkASSERT(SkGIFColorMap::kNotFound == transparent_pixel);
    return false;
  }

  if (is_local_colormap_defined) {
    return transparent_pixel < local_colors;
  }

  // If there is a global color table, it will be parsed before reaching
  // here. If its numColors is set, it will be defined.
  SkASSERT(global_colors > 0);
  SkASSERT(global_color_map_.IsDefined());
  return transparent_pixel < global_colors;
}

void SkGifImageReader::AddFrameIfNecessary() {
  if (frames_.empty() || frames_.back()->IsComplete()) {
    const size_t i = frames_.size();
    std::unique_ptr<SkGIFFrameContext> frame(new SkGIFFrameContext(this, static_cast<int>(i)));
    frames_.push_back(std::move(frame));
  }
}

static SkIRect frame_rect_on_screen(SkIRect frameRect,
                                    const SkIRect& screenRect) {
  if (!frameRect.intersect(screenRect)) {
    return SkIRect::MakeEmpty();
  }

  return frameRect;
}

static bool independent(const SkFrame& frame) {
  return frame.getRequiredFrame() == SkCodec::kNone;
}

static bool restore_bg(const SkFrame& frame) {
  return frame.getDisposalMethod() == SkCodecAnimation::DisposalMethod::kRestoreBGColor;
}

bool SkGIFFrameContext::onReportsAlpha() const {
  // Note: We could correct these after decoding - i.e. some frames may turn out to be
  // independent and opaque if they do not use the transparent pixel, but that would require
  // checking whether each pixel used the transparent index.
  return owner_->HasTransparency(this->TransparentPixel(),
                                 local_color_map_.IsDefined(),
                                 local_color_map_.NumColors());
}

void SkFrameHolder::setAlphaAndRequiredFrame(SkFrame* frame) {
    const bool reportsAlpha = frame->reportsAlpha();
    const auto screenRect = SkIRect::MakeWH(fScreenWidth, fScreenHeight);
    const auto frameRect = frame_rect_on_screen(frame->frameRect(), screenRect);

    const int i = frame->frameId();
    if (0 == i) {
        frame->setHasAlpha(reportsAlpha || frameRect != screenRect);
        frame->setRequiredFrame(SkCodec::kNone);
        return;
    }


    const bool blendWithPrevFrame = frame->getBlend() == SkCodecAnimation::Blend::kPriorFrame;
    if ((!reportsAlpha || !blendWithPrevFrame) && frameRect == screenRect) {
        frame->setHasAlpha(reportsAlpha);
        frame->setRequiredFrame(SkCodec::kNone);
        return;
    }

    const SkFrame* prevFrame = this->getFrame(i-1);
    while (prevFrame->getDisposalMethod() == SkCodecAnimation::DisposalMethod::kRestorePrevious) {
        const int prevId = prevFrame->frameId();
        if (0 == prevId) {
            frame->setHasAlpha(true);
            frame->setRequiredFrame(SkCodec::kNone);
            return;
        }

        prevFrame = this->getFrame(prevId - 1);
    }

    const bool clearPrevFrame = restore_bg(*prevFrame);
    auto prevFrameRect = frame_rect_on_screen(prevFrame->frameRect(), screenRect);

    if (clearPrevFrame) {
        if (prevFrameRect == screenRect || independent(*prevFrame)) {
            frame->setHasAlpha(true);
            frame->setRequiredFrame(SkCodec::kNone);
            return;
        }
    }

    if (reportsAlpha && blendWithPrevFrame) {
        // Note: We could be more aggressive here. If prevFrame clears
        // to background color and covers its required frame (and that
        // frame is independent), prevFrame could be marked independent.
        // Would this extra complexity be worth it?
        frame->setRequiredFrame(prevFrame->frameId());
        frame->setHasAlpha(prevFrame->hasAlpha() || clearPrevFrame);
        return;
    }

    while (frameRect.contains(prevFrameRect)) {
        const int prevRequiredFrame = prevFrame->getRequiredFrame();
        if (prevRequiredFrame == SkCodec::kNone) {
            frame->setRequiredFrame(SkCodec::kNone);
            frame->setHasAlpha(true);
            return;
        }

        prevFrame = this->getFrame(prevRequiredFrame);
        prevFrameRect = frame_rect_on_screen(prevFrame->frameRect(), screenRect);
    }

    if (restore_bg(*prevFrame)) {
        frame->setHasAlpha(true);
        if (prevFrameRect == screenRect || independent(*prevFrame)) {
            frame->setRequiredFrame(SkCodec::kNone);
        } else {
            // Note: As above, frame could still be independent, e.g. if
            // prevFrame covers its required frame and that frame is
            // independent.
            frame->setRequiredFrame(prevFrame->frameId());
        }
        return;
    }

    SkASSERT(prevFrame->getDisposalMethod() == SkCodecAnimation::DisposalMethod::kKeep);
    frame->setRequiredFrame(prevFrame->frameId());
    frame->setHasAlpha(prevFrame->hasAlpha() || (reportsAlpha && !blendWithPrevFrame));
}

// FIXME: Move this method to close to doLZW().
bool SkGIFLZWContext::PrepareToDecode() {
  SkASSERT(frame_context_->IsDataSizeDefined());
  SkASSERT(frame_context_->IsHeaderDefined());

  // Since we use a codesize of 1 more than the datasize, we need to ensure
  // that our datasize is strictly less than the kMaxDictionaryEntryBits.
  if (frame_context_->DataSize() >= kMaxDictionaryEntryBits)
    return false;
  clear_code = 1 << frame_context_->DataSize();
  avail = clear_code + 2;
  oldcode = -1;
  codesize = frame_context_->DataSize() + 1;
  codemask = (1 << codesize) - 1;
  datum = bits = 0;
  ipass = frame_context_->Interlaced() ? 1 : 0;
  irow = 0;

  // We want to know the longest sequence encodable by a dictionary with
  // kMaxDictionaryEntries entries. If we ignore the need to encode the base
  // values themselves at the beginning of the dictionary, as well as the need
  // for a clear code or a termination code, we could use every entry to
  // encode a series of multiple values. If the input value stream looked
  // like "AAAAA..." (a long string of just one value), the first dictionary
  // entry would encode AA, the next AAA, the next AAAA, and so forth. Thus
  // the longest sequence would be kMaxDictionaryEntries + 1 values.
  //
  // However, we have to account for reserved entries. The first |datasize|
  // bits are reserved for the base values, and the next two entries are
  // reserved for the clear code and termination code. In theory a GIF can
  // set the datasize to 0, meaning we have just two reserved entries, making
  // the longest sequence (kMaxDictionaryEntries + 1) - 2 values long. Since
  // each value is a byte, this is also the number of bytes in the longest
  // encodable sequence.
  const size_t kMaxBytes = kMaxDictionaryEntries - 1;

  // Now allocate the output buffer. We decode directly into this buffer
  // until we have at least one row worth of data, then call outputRow().
  // This means worst case we may have (row width - 1) bytes in the buffer
  // and then decode a sequence |maxBytes| long to append.
  row_buffer.reset(frame_context_->width() - 1 + kMaxBytes);
  row_iter = row_buffer.begin();
  rows_remaining = frame_context_->height();

  // Clearing the whole suffix table lets us be more tolerant of bad data.
  for (int i = 0; i < clear_code; ++i) {
    suffix[i] = i;
    suffix_length[i] = 1;
  }
  return true;
}

