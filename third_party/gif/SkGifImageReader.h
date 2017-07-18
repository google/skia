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


#define SK_MAX_DICTIONARY_ENTRY_BITS 12
#define SK_MAX_DICTIONARY_ENTRIES    4096 // 2^SK_MAX_DICTIONARY_ENTRY_BITS
#define SK_MAX_COLORS                256
#define SK_BYTES_PER_COLORMAP_ENTRY  3

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
    SkGIFLZWContext(SkGifCodec* client, const SkGIFFrameContext* frameContext)
        : codesize(0)
        , codemask(0)
        , clearCode(0)
        , avail(0)
        , oldcode(0)
        , firstchar(0)
        , bits(0)
        , datum(0)
        , ipass(0)
        , irow(0)
        , rowsRemaining(0)
        , rowIter(0)
        , m_client(client)
        , m_frameContext(frameContext)
    { }

    bool prepareToDecode();
    bool outputRow(const unsigned char* rowBegin);
    bool doLZW(const unsigned char* block, size_t bytesInBlock);
    bool hasRemainingRows() { return SkToBool(rowsRemaining); }

private:
    // LZW decoding states and output states.
    int codesize;
    int codemask;
    int clearCode; // Codeword used to trigger dictionary reset.
    int avail; // Index of next available slot in dictionary.
    int oldcode;
    unsigned char firstchar;
    int bits; // Number of unread bits in "datum".
    int datum; // 32-bit input buffer.
    int ipass; // Interlace pass; Ranges 1-4 if interlaced.
    size_t irow; // Current output row, starting at zero.
    size_t rowsRemaining; // Rows remaining to be output.

    unsigned short prefix[SK_MAX_DICTIONARY_ENTRIES];
    unsigned char suffix[SK_MAX_DICTIONARY_ENTRIES];
    unsigned short suffixLength[SK_MAX_DICTIONARY_ENTRIES];
    SkGIFRow rowBuffer; // Single scanline temporary buffer.
    unsigned char* rowIter;

    SkGifCodec* const m_client;
    const SkGIFFrameContext* m_frameContext;
};

struct SkGIFLZWBlock {
 public:
  SkGIFLZWBlock(size_t position, size_t size)
      : blockPosition(position), blockSize(size) {}

  size_t blockPosition;
  size_t blockSize;
};

class SkGIFColorMap final {
public:
    static constexpr int kNotFound = -1;

    SkGIFColorMap()
        : m_isDefined(false)
        , m_position(0)
        , m_colors(0)
        , m_transPixel(kNotFound)
        , m_packColorProc(nullptr)
    {
    }

    void setNumColors(int colors) {
        SkASSERT(!m_colors);
        SkASSERT(!m_position);

        m_colors = colors;
    }

    void setTablePosition(size_t position) {
        SkASSERT(!m_isDefined);

        m_position = position;
        m_isDefined = true;
    }

    int numColors() const { return m_colors; }

    bool isDefined() const { return m_isDefined; }

    // Build RGBA table using the data stream.
    sk_sp<SkColorTable> buildTable(SkStreamBuffer*, SkColorType dstColorType,
                                   int transparentPixel) const;

private:
    bool m_isDefined;
    size_t m_position;
    int m_colors;
    // Cached values. If these match on a new request, we can reuse m_table.
    mutable int m_transPixel;
    mutable PackColorProc m_packColorProc;
    mutable sk_sp<SkColorTable> m_table;
};

class SkGifImageReader;

// LocalFrame output state machine.
class SkGIFFrameContext : public SkFrame {
public:
    SkGIFFrameContext(SkGifImageReader* reader, int id)
        : INHERITED(id)
        , m_owner(reader)
        , m_transparentPixel(SkGIFColorMap::kNotFound)
        , m_dataSize(0)
        , m_progressiveDisplay(false)
        , m_interlaced(false)
        , m_delayTime(0)
        , m_currentLzwBlock(0)
        , m_isComplete(false)
        , m_isHeaderDefined(false)
        , m_isDataSizeDefined(false)
    {
    }

    ~SkGIFFrameContext() override
    {
    }

    void addLzwBlock(size_t position, size_t size)
    {
        m_lzwBlocks.push_back(SkGIFLZWBlock(position, size));
    }

    bool decode(SkStreamBuffer*, SkGifCodec* client, bool* frameDecoded);

    int transparentPixel() const { return m_transparentPixel; }
    void setTransparentPixel(int pixel) { m_transparentPixel = pixel; }

    unsigned delayTime() const { return m_delayTime; }
    void setDelayTime(unsigned delay) { m_delayTime = delay; }
    bool isComplete() const { return m_isComplete; }
    void setComplete() { m_isComplete = true; }
    bool isHeaderDefined() const { return m_isHeaderDefined; }
    void setHeaderDefined() { m_isHeaderDefined = true; }
    bool isDataSizeDefined() const { return m_isDataSizeDefined; }
    int dataSize() const { return m_dataSize; }
    void setDataSize(int size)
    {
        m_dataSize = size;
        m_isDataSizeDefined = true;
    }
    bool progressiveDisplay() const { return m_progressiveDisplay; }
    void setProgressiveDisplay(bool progressiveDisplay) { m_progressiveDisplay = progressiveDisplay; }
    bool interlaced() const { return m_interlaced; }
    void setInterlaced(bool interlaced) { m_interlaced = interlaced; }

    void clearDecodeState() { m_lzwContext.reset(); }
    const SkGIFColorMap& localColorMap() const { return m_localColorMap; }
    SkGIFColorMap& localColorMap() { return m_localColorMap; }

protected:
    bool onReportsAlpha() const override;

private:
    // Unowned pointer to the object that owns this frame.
    const SkGifImageReader* m_owner;

    int m_transparentPixel; // Index of transparent pixel. Value is kNotFound if there is no transparent pixel.
    int m_dataSize;

    bool m_progressiveDisplay; // If true, do Haeberli interlace hack.
    bool m_interlaced; // True, if scanlines arrive interlaced order.

    unsigned m_delayTime; // Display time, in milliseconds, for this image in a multi-image GIF.

    std::unique_ptr<SkGIFLZWContext> m_lzwContext;
    // LZW blocks for this frame.
    std::vector<SkGIFLZWBlock> m_lzwBlocks;

    SkGIFColorMap m_localColorMap;

    int m_currentLzwBlock;
    bool m_isComplete;
    bool m_isHeaderDefined;
    bool m_isDataSizeDefined;

    typedef SkFrame INHERITED;
};

class SkGifImageReader final : public SkFrameHolder {
public:
    // This takes ownership of stream.
    SkGifImageReader(SkStream* stream)
        : m_client(nullptr)
        , m_state(SkGIFType)
        , m_bytesToConsume(6) // Number of bytes for GIF type, either "GIF87a" or "GIF89a".
        , m_version(0)
        , m_loopCount(cLoopCountNotSeen)
        , m_streamBuffer(stream)
        , m_parseCompleted(false)
        , m_firstFrameHasAlpha(false)
    {
    }

    ~SkGifImageReader() override
    {
    }

    void setClient(SkGifCodec* client) { m_client = client; }

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
    SkCodec::Result parse(SkGIFParseQuery);

    // Decode the frame indicated by frameIndex.
    // frameComplete will be set to true if the frame is completely decoded.
    // The method returns false if there is an error.
    bool decode(int frameIndex, bool* frameComplete);

    int imagesCount() const
    {
        const size_t frames = m_frames.size();
        if (!frames) {
            return 0;
        }

        // This avoids counting an empty frame when the file is truncated (or
        // simply not yet complete) after receiving SkGIFControlExtension (and
        // possibly SkGIFImageHeader) but before reading the color table. This
        // ensures that we do not count a frame before we know its required
        // frame.
        return static_cast<int>(m_frames.back()->reachedStartOfData() ? frames : frames - 1);
    }
    int loopCount() const {
        if (cLoopCountNotSeen == m_loopCount) {
            return 0;
        }
        return m_loopCount;
    }

    const SkGIFColorMap& globalColorMap() const
    {
        return m_globalColorMap;
    }

    const SkGIFFrameContext* frameContext(int index) const
    {
        return index >= 0 && index < static_cast<int>(m_frames.size())
                ? m_frames[index].get() : 0;
    }

    void clearDecodeState() {
        for (size_t index = 0; index < m_frames.size(); index++) {
            m_frames[index]->clearDecodeState();
        }
    }

    // Return the color table for frame index (which may be the global color table).
    sk_sp<SkColorTable> getColorTable(SkColorType dstColorType, int index);

    bool firstFrameHasAlpha() const { return m_firstFrameHasAlpha; }

    // Helper function that returns whether an SkGIFFrameContext has transparency.
    // This method is sometimes called before creating one/parsing its color map,
    // so it cannot rely on SkGIFFrameContext::transparentPixel or ::localColorMap().
    bool hasTransparency(int transPix, bool hasLocalColorMap, int localMapColors) const;

protected:
    const SkFrame* onGetFrame(int i) const override {
        return static_cast<const SkFrame*>(this->frameContext(i));
    }

private:
    // Requires that one byte has been buffered into m_streamBuffer.
    unsigned char getOneByte() const {
        return reinterpret_cast<const unsigned char*>(m_streamBuffer.get())[0];
    }

    void addFrameIfNecessary();
    bool currentFrameIsFirstFrame() const
    {
        return m_frames.empty() || (m_frames.size() == 1u && !m_frames[0]->isComplete());
    }

    // Unowned pointer
    SkGifCodec* m_client;

    // Parsing state machine.
    SkGIFState m_state; // Current decoder master state.
    size_t m_bytesToConsume; // Number of bytes to consume for next stage of parsing.

    // Global (multi-image) state.
    int m_version; // Either 89 for GIF89 or 87 for GIF87.
    SkGIFColorMap m_globalColorMap;

    static constexpr int cLoopCountNotSeen = -2;
    int m_loopCount; // Netscape specific extension block to control the number of animation loops a GIF renders.

    std::vector<std::unique_ptr<SkGIFFrameContext>> m_frames;

    SkStreamBuffer m_streamBuffer;
    bool m_parseCompleted;

    // This value can be computed before we create a SkGIFFrameContext, so we
    // store it here instead of on m_frames[0].
    bool m_firstFrameHasAlpha;
};

#endif
