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


// GETN(n, s) requests at least 'n' bytes available from 'q', at start of state 's'.
//
// Note, the hold will never need to be bigger than 256 bytes to gather up in the hold,
// as each GIF block (except colormaps) can never be bigger than 256 bytes.
// Colormaps are directly copied in the resp. global_colormap or dynamically allocated local_colormap.
// So a fixed buffer in SkGifImageReader is good enough.
// This buffer is only needed to copy left-over data from one GifWrite call to the next
#define GETN(n, s) \
    do { \
        m_bytesToConsume = (n); \
        m_state = (s); \
    } while (0)

// Get a 16-bit value stored in little-endian format.
#define GETINT16(p)   ((p)[1]<<8|(p)[0])

namespace {
    bool is_palette_index_valid(int transparentIndex) {
        // -1 is a signal that there is no transparent index.
        // Otherwise, it is encoded in 8 bits, and all 256 values are considered
        // valid since a GIF may use an index outside of the palette to be
        // transparent.
        return transparentIndex >= 0;
    }
} // anonymous namespace

// Send the data to the display front-end.
void SkGIFLZWContext::outputRow(const unsigned char* rowBegin)
{
    int drowStart = irow;
    int drowEnd = irow;

    // Haeberli-inspired hack for interlaced GIFs: Replicate lines while
    // displaying to diminish the "venetian-blind" effect as the image is
    // loaded. Adjust pixel vertical positions to avoid the appearance of the
    // image crawling up the screen as successive passes are drawn.
    if (m_frameContext->progressiveDisplay() && m_frameContext->interlaced() && ipass < 4) {
        unsigned rowDup = 0;
        unsigned rowShift = 0;

        switch (ipass) {
        case 1:
            rowDup = 7;
            rowShift = 3;
            break;
        case 2:
            rowDup = 3;
            rowShift = 1;
            break;
        case 3:
            rowDup = 1;
            rowShift = 0;
            break;
        default:
            break;
        }

        drowStart -= rowShift;
        drowEnd = drowStart + rowDup;

        // Extend if bottom edge isn't covered because of the shift upward.
        if ((unsigned)((m_frameContext->height() - 1) - drowEnd) <= rowShift)
            drowEnd = m_frameContext->height() - 1;

        // Clamp first and last rows to upper and lower edge of image.
        if (drowStart < 0)
            drowStart = 0;

        if (drowEnd >= m_frameContext->height())
            drowEnd = m_frameContext->height() - 1;
    }

    // Protect against too much image data.
    if (drowStart >= m_frameContext->height())
        return;

    // CALLBACK: Let the client know we have decoded a row.
    const bool writeTransparentPixels =
            SkCodec::kNoFrame == m_frameContext->getRequiredFrame();
    m_client->haveDecodedRow(m_frameContext->frameId(), rowBegin,
            drowStart, drowEnd - drowStart + 1, writeTransparentPixels);

    if (!m_frameContext->interlaced())
        irow++;
    else {
        do {
            switch (ipass) {
            case 1:
                irow += 8;
                if (irow >= (unsigned) m_frameContext->height()) {
                    ipass++;
                    irow = 4;
                }
                break;

            case 2:
                irow += 8;
                if (irow >= (unsigned) m_frameContext->height()) {
                    ipass++;
                    irow = 2;
                }
                break;

            case 3:
                irow += 4;
                if (irow >= (unsigned) m_frameContext->height()) {
                    ipass++;
                    irow = 1;
                }
                break;

            case 4:
                irow += 2;
                if (irow >= (unsigned) m_frameContext->height()) {
                    ipass++;
                    irow = 0;
                }
                break;

            default:
                break;
            }
        } while (irow > (unsigned) (m_frameContext->height() - 1));
    }
}

// Perform Lempel-Ziv-Welch decoding.
// Returns true if decoding was successful. In this case the block will have been completely consumed and/or rowsRemaining will be 0.
// Otherwise, decoding failed; returns false in this case, which will always cause the SkGifImageReader to set the "decode failed" flag.
bool SkGIFLZWContext::doLZW(const unsigned char* block, size_t bytesInBlock)
{
    if (rowIter == rowBuffer.end())
        return true;
    const int width = m_frameContext->width();

    for (const unsigned char* ch = block; bytesInBlock-- > 0; ch++) {
        // Feed the next byte into the decoder's 32-bit input buffer.
        datum += ((int) *ch) << bits;
        bits += 8;

        // Check for underflow of decoder's 32-bit input buffer.
        while (bits >= codesize) {
            // Get the leading variable-length symbol from the data stream.
            int code = datum & codemask;
            datum >>= codesize;
            bits -= codesize;

            // Reset the dictionary to its original state, if requested.
            if (code == clearCode) {
                codesize = m_frameContext->dataSize() + 1;
                codemask = (1 << codesize) - 1;
                avail = clearCode + 2;
                oldcode = -1;
                continue;
            }

            // Check for explicit end-of-stream code.
            if (code == (clearCode + 1)) {
                // end-of-stream should only appear after all image data.
                if (!rowsRemaining)
                    return true;
                return false;
            }

            const int tempCode = code;
            if (code > avail) {
                // This is an invalid code. The dictionary is just initialized
                // and the code is incomplete. We don't know how to handle
                // this case.
                return false;
            }

            if (code == avail) {
                if (oldcode != -1) {
                    // This is a new code just being added to the dictionary.
                    // It must encode the contents of the previous code, plus
                    // the first character of the previous code again.
                    // Now we know avail is the new code we can use oldcode
                    // value to get the code related to that.
                    code = oldcode;
                } else {
                    // This is an invalid code. The dictionary is just initialized
                    // and the code is incomplete. We don't know how to handle
                    // this case.
                    return false;
                }
            }

            // code length of the oldcode for new code which is
            // avail = oldcode + firstchar of the oldcode
            int remaining = suffixLength[code];

            // Round remaining up to multiple of SK_DICTIONARY_WORD_SIZE, because that's
            // the granularity of the chunks we copy.  The last chunk may contain
            // some garbage but it'll be overwritten by the next code or left unused.
            // The working buffer is padded to account for this.
            remaining += -remaining & (SK_DICTIONARY_WORD_SIZE - 1) ;
            unsigned char* p = rowIter + remaining;

            // Place rowIter so that after writing pixels rowIter can be set to firstchar, thereby
            // completing the code.
            rowIter += suffixLength[code];

            while (remaining > 0) {
                p -= SK_DICTIONARY_WORD_SIZE;
                std::copy_n(suffix[code].begin(), SK_DICTIONARY_WORD_SIZE, p);
                code = prefix[code];
                remaining -= SK_DICTIONARY_WORD_SIZE;
            }
            const int firstchar = static_cast<unsigned char>(code);  // (strictly `suffix[code][0]`)

            // This completes the new code avail and writing the corresponding
            // pixels on target.
            if (tempCode == avail) {
                *rowIter++ = firstchar;
            }

            // Define a new codeword in the dictionary as long as we've read
            // more than one value from the stream.
            if (avail < SK_MAX_DICTIONARY_ENTRIES && oldcode != -1) {
                // now add avail to the dictionary for future reference
                unsigned short codeLength = suffixLength[oldcode] + 1;
                int l = (codeLength - 1) & (SK_DICTIONARY_WORD_SIZE - 1);
                // If the suffix buffer is full (l == 0) then oldcode becomes the new
                // prefix, otherwise copy and extend oldcode's buffer and use the same
                // prefix as oldcode used.
                prefix[avail] = (l == 0) ? oldcode : prefix[oldcode];
                suffix[avail] = suffix[oldcode];
                suffix[avail][l] = firstchar;
                suffixLength[avail] = codeLength;
                ++avail;

                // If we've used up all the codewords of a given length
                // increase the length of codewords by one bit, but don't
                // exceed the specified maximum codeword size.
                if (!(avail & codemask) && avail < SK_MAX_DICTIONARY_ENTRIES) {
                    ++codesize;
                    codemask += avail;
                }
            }
            oldcode = tempCode;

            // Output as many rows as possible.
            unsigned char* rowBegin = rowBuffer.begin();
            for (; rowBegin + width <= rowIter; rowBegin += width) {
                outputRow(rowBegin);
                rowsRemaining--;
                if (!rowsRemaining)
                    return true;
            }

            if (rowBegin != rowBuffer.begin()) {
                // Move the remaining bytes to the beginning of the buffer.
                const size_t bytesToCopy = rowIter - rowBegin;
                memcpy(&rowBuffer.front(), rowBegin, bytesToCopy);
                rowIter = rowBuffer.begin() + bytesToCopy;
            }
        }
    }
    return true;
}

sk_sp<SkColorTable> SkGIFColorMap::buildTable(SkStreamBuffer* streamBuffer, SkColorType colorType,
                                              int transparentPixel) const
{
    if (!m_isDefined)
        return nullptr;

    const PackColorProc proc = choose_pack_color_proc(false, colorType);
    if (m_table && proc == m_packColorProc && m_transPixel == transparentPixel) {
        SkASSERT(transparentPixel == kNotFound || transparentPixel > m_table->count()
                || m_table->operator[](transparentPixel) == SK_ColorTRANSPARENT);
        // This SkColorTable has already been built with the same transparent color and
        // packing proc. Reuse it.
        return m_table;
    }
    m_packColorProc = proc;
    m_transPixel = transparentPixel;

    const size_t bytes = m_colors * SK_BYTES_PER_COLORMAP_ENTRY;
    sk_sp<SkData> rawData(streamBuffer->getDataAtPosition(m_position, bytes));
    if (!rawData) {
        return nullptr;
    }

    SkASSERT(m_colors <= SK_MAX_COLORS);
    const uint8_t* srcColormap = rawData->bytes();
    SkPMColor colorStorage[SK_MAX_COLORS];
    for (int i = 0; i < m_colors; i++) {
        if (i == transparentPixel) {
            colorStorage[i] = SK_ColorTRANSPARENT;
        } else {
            colorStorage[i] = proc(255, srcColormap[0], srcColormap[1], srcColormap[2]);
        }
        srcColormap += SK_BYTES_PER_COLORMAP_ENTRY;
    }
    for (int i = m_colors; i < SK_MAX_COLORS; i++) {
        colorStorage[i] = SK_ColorTRANSPARENT;
    }
    m_table = sk_sp<SkColorTable>(new SkColorTable(colorStorage, SK_MAX_COLORS));
    return m_table;
}

sk_sp<SkColorTable> SkGifImageReader::getColorTable(SkColorType colorType, int index) {
    if (index < 0 || index >= m_frames.count()) {
        return nullptr;
    }

    const SkGIFFrameContext* frameContext = m_frames[index].get();
    const SkGIFColorMap& localColorMap = frameContext->localColorMap();
    const int transPix = frameContext->transparentPixel();
    if (localColorMap.isDefined()) {
        return localColorMap.buildTable(&m_streamBuffer, colorType, transPix);
    }
    if (m_globalColorMap.isDefined()) {
        return m_globalColorMap.buildTable(&m_streamBuffer, colorType, transPix);
    }
    return nullptr;
}

// Perform decoding for this frame. frameComplete will be true if the entire frame is decoded.
// Returns false if a decoding error occurred. This is a fatal error and causes the SkGifImageReader to set the "decode failed" flag.
// Otherwise, either not enough data is available to decode further than before, or the new data has been decoded successfully; returns true in this case.
bool SkGIFFrameContext::decode(SkStreamBuffer* streamBuffer, SkGifCodec* client,
                               bool* frameComplete)
{
    *frameComplete = false;
    if (!m_lzwContext) {
        // Wait for more data to properly initialize SkGIFLZWContext.
        if (!isDataSizeDefined() || !isHeaderDefined())
            return true;

        m_lzwContext.reset(new SkGIFLZWContext(client, this));
        if (!m_lzwContext->prepareToDecode()) {
            m_lzwContext.reset();
            return false;
        }

        m_currentLzwBlock = 0;
    }

    // Some bad GIFs have extra blocks beyond the last row, which we don't want to decode.
    while (m_currentLzwBlock < m_lzwBlocks.count() && m_lzwContext->hasRemainingRows()) {
        const auto& block = m_lzwBlocks[m_currentLzwBlock];
        const size_t len = block.blockSize;

        sk_sp<SkData> data(streamBuffer->getDataAtPosition(block.blockPosition, len));
        if (!data) {
            return false;
        }
        if (!m_lzwContext->doLZW(reinterpret_cast<const unsigned char*>(data->data()), len)) {
            return false;
        }
        ++m_currentLzwBlock;
    }

    // If this frame is data complete then the previous loop must have completely decoded all LZW blocks.
    // There will be no more decoding for this frame so it's time to cleanup.
    if (isComplete()) {
        *frameComplete = true;
        m_lzwContext.reset();
    }
    return true;
}

// Decode a frame.
// This method uses SkGIFFrameContext:decode() to decode the frame; decoding error is reported to client as a critical failure.
// Return true if decoding has progressed. Return false if an error has occurred.
bool SkGifImageReader::decode(int frameIndex, bool* frameComplete)
{
    SkGIFFrameContext* currentFrame = m_frames[frameIndex].get();

    return currentFrame->decode(&m_streamBuffer, m_client, frameComplete);
}

// Parse incoming GIF data stream into internal data structures.
SkCodec::Result SkGifImageReader::parse(SkGifImageReader::SkGIFParseQuery query)
{
    if (m_parseCompleted) {
        return SkCodec::kSuccess;
    }

    if (SkGIFLoopCountQuery == query && m_loopCount != cLoopCountNotSeen) {
        // Loop count has already been parsed.
        return SkCodec::kSuccess;
    }

    // SkGIFSizeQuery and SkGIFFrameCountQuery are negative, so this is only meaningful when >= 0.
    const int lastFrameToParse = (int) query;
    if (lastFrameToParse >= 0 && m_frames.count() > lastFrameToParse
                && m_frames[lastFrameToParse]->isComplete()) {
        // We have already parsed this frame.
        return SkCodec::kSuccess;
    }

    while (true) {
        if (!m_streamBuffer.buffer(m_bytesToConsume)) {
            // The stream does not yet have enough data.
            return SkCodec::kIncompleteInput;
        }

        switch (m_state) {
        case SkGIFLZW: {
            SkASSERT(!m_frames.empty());
            auto* frame = m_frames.back().get();
            frame->addLzwBlock(m_streamBuffer.markPosition(), m_bytesToConsume);
            GETN(1, SkGIFSubBlock);
            break;
        }
        case SkGIFLZWStart: {
            SkASSERT(!m_frames.empty());
            auto* currentFrame = m_frames.back().get();

            currentFrame->setDataSize(this->getOneByte());
            GETN(1, SkGIFSubBlock);
            break;
        }

        case SkGIFType: {
            const char* currentComponent = m_streamBuffer.get();

            // All GIF files begin with "GIF87a" or "GIF89a".
            if (!memcmp(currentComponent, "GIF89a", 6))
                m_version = 89;
            else if (!memcmp(currentComponent, "GIF87a", 6))
                m_version = 87;
            else {
                // This prevents attempting to continue reading this invalid stream.
                GETN(0, SkGIFDone);
                return SkCodec::kInvalidInput;
            }
            GETN(7, SkGIFGlobalHeader);
            break;
        }

        case SkGIFGlobalHeader: {
            const unsigned char* currentComponent =
                reinterpret_cast<const unsigned char*>(m_streamBuffer.get());

            // This is the height and width of the "screen" or frame into which
            // images are rendered. The individual images can be smaller than
            // the screen size and located with an origin anywhere within the
            // screen.
            // Note that we don't inform the client of the size yet, as it might
            // change after we read the first frame's image header.
            fScreenWidth = GETINT16(currentComponent);
            fScreenHeight = GETINT16(currentComponent + 2);

            const int globalColorMapColors = 2 << (currentComponent[4] & 0x07);

            if ((currentComponent[4] & 0x80) && globalColorMapColors > 0) { /* global map */
                m_globalColorMap.setNumColors(globalColorMapColors);
                GETN(SK_BYTES_PER_COLORMAP_ENTRY * globalColorMapColors, SkGIFGlobalColormap);
                break;
            }

            GETN(1, SkGIFImageStart);
            break;
        }

        case SkGIFGlobalColormap: {
            m_globalColorMap.setTablePosition(m_streamBuffer.markPosition());
            GETN(1, SkGIFImageStart);
            break;
        }

        case SkGIFImageStart: {
            const char currentComponent = m_streamBuffer.get()[0];

            if (currentComponent == '!') { // extension.
                GETN(2, SkGIFExtension);
                break;
            }

            if (currentComponent == ',') { // image separator.
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
            const unsigned char* currentComponent =
                reinterpret_cast<const unsigned char*>(m_streamBuffer.get());

            size_t bytesInBlock = currentComponent[1];
            SkGIFState exceptionState = SkGIFSkipBlock;

            switch (*currentComponent) {
            case 0xf9:
                // The GIF spec mandates that the GIFControlExtension header block length is 4 bytes,
                exceptionState = SkGIFControlExtension;
                // and the parser for this block reads 4 bytes, so we must enforce that the buffer
                // contains at least this many bytes. If the GIF specifies a different length, we
                // allow that, so long as it's larger; the additional data will simply be ignored.
                bytesInBlock = std::max(bytesInBlock, static_cast<size_t>(4));
                break;

            // The GIF spec also specifies the lengths of the following two extensions' headers
            // (as 12 and 11 bytes, respectively). Because we ignore the plain text extension entirely
            // and sanity-check the actual length of the application extension header before reading it,
            // we allow GIFs to deviate from these values in either direction. This is important for
            // real-world compatibility, as GIFs in the wild exist with application extension headers
            // that are both shorter and longer than 11 bytes.
            case 0x01:
                // ignoring plain text extension
                break;

            case 0xff:
                exceptionState = SkGIFApplicationExtension;
                break;

            case 0xfe:
                exceptionState = SkGIFConsumeComment;
                break;
            }

            if (bytesInBlock)
                GETN(bytesInBlock, exceptionState);
            else
                GETN(1, SkGIFImageStart);
            break;
        }

        case SkGIFConsumeBlock: {
            const unsigned char currentComponent = this->getOneByte();
            if (!currentComponent)
                GETN(1, SkGIFImageStart);
            else
                GETN(currentComponent, SkGIFSkipBlock);
            break;
        }

        case SkGIFSkipBlock: {
            GETN(1, SkGIFConsumeBlock);
            break;
        }

        case SkGIFControlExtension: {
            const unsigned char* currentComponent =
                reinterpret_cast<const unsigned char*>(m_streamBuffer.get());

            addFrameIfNecessary();
            SkGIFFrameContext* currentFrame = m_frames.back().get();
            if (*currentComponent & 0x1)
                currentFrame->setTransparentPixel(currentComponent[3]);

            // We ignore the "user input" bit.

            // NOTE: This relies on the values in the FrameDisposalMethod enum
            // matching those in the GIF spec!
            int rawDisposalMethod = ((*currentComponent) >> 2) & 0x7;
            switch (rawDisposalMethod) {
            case 1:
            case 2:
            case 3:
                currentFrame->setDisposalMethod((SkCodecAnimation::DisposalMethod) rawDisposalMethod);
                break;
            case 4:
                // Some specs say that disposal method 3 is "overwrite previous", others that setting
                // the third bit of the field (i.e. method 4) is. We map both to the same value.
                currentFrame->setDisposalMethod(SkCodecAnimation::DisposalMethod::kRestorePrevious);
                break;
            default:
                // Other values use the default.
                currentFrame->setDisposalMethod(SkCodecAnimation::DisposalMethod::kKeep);
                break;
            }
            currentFrame->setDuration(GETINT16(currentComponent + 1) * 10);
            GETN(1, SkGIFConsumeBlock);
            break;
        }

        case SkGIFCommentExtension: {
            const unsigned char currentComponent = this->getOneByte();
            if (currentComponent)
                GETN(currentComponent, SkGIFConsumeComment);
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
            if (m_bytesToConsume == 11) {
                const unsigned char* currentComponent =
                    reinterpret_cast<const unsigned char*>(m_streamBuffer.get());

                if (!memcmp(currentComponent, "NETSCAPE2.0", 11) || !memcmp(currentComponent, "ANIMEXTS1.0", 11))
                    GETN(1, SkGIFNetscapeExtensionBlock);
            }

            if (m_state != SkGIFNetscapeExtensionBlock)
                GETN(1, SkGIFConsumeBlock);
            break;
        }

        // Netscape-specific GIF extension: animation looping.
        case SkGIFNetscapeExtensionBlock: {
            const int currentComponent = this->getOneByte();
            // SkGIFConsumeNetscapeExtension always reads 3 bytes from the stream; we should at least wait for this amount.
            if (currentComponent)
                GETN(std::max(3, currentComponent), SkGIFConsumeNetscapeExtension);
            else
                GETN(1, SkGIFImageStart);
            break;
        }

        // Parse netscape-specific application extensions
        case SkGIFConsumeNetscapeExtension: {
            const unsigned char* currentComponent =
                reinterpret_cast<const unsigned char*>(m_streamBuffer.get());

            int netscapeExtension = currentComponent[0] & 7;

            // Loop entire animation specified # of times. Only read the loop count during the first iteration.
            if (netscapeExtension == 1) {
                m_loopCount = GETINT16(currentComponent + 1);

                // Zero loop count is infinite animation loop request.
                if (!m_loopCount)
                    m_loopCount = SkCodec::kRepetitionCountInfinite;

                GETN(1, SkGIFNetscapeExtensionBlock);

                if (SkGIFLoopCountQuery == query) {
                    m_streamBuffer.flush();
                    return SkCodec::kSuccess;
                }
            } else if (netscapeExtension == 2) {
                // Wait for specified # of bytes to enter buffer.

                // Don't do this, this extension doesn't exist (isn't used at all)
                // and doesn't do anything, as our streaming/buffering takes care of it all...
                // See: http://semmix.pl/color/exgraf/eeg24.htm
                GETN(1, SkGIFNetscapeExtensionBlock);
            } else {
                // 0,3-7 are yet to be defined netscape extension codes
                // This prevents attempting to continue reading this invalid stream.
                GETN(0, SkGIFDone);
                return SkCodec::kInvalidInput;
            }
            break;
        }

        case SkGIFImageHeader: {
            int height, width, xOffset, yOffset;
            const unsigned char* currentComponent =
                reinterpret_cast<const unsigned char*>(m_streamBuffer.get());

            /* Get image offsets, with respect to the screen origin */
            xOffset = GETINT16(currentComponent);
            yOffset = GETINT16(currentComponent + 2);

            /* Get image width and height. */
            width  = GETINT16(currentComponent + 4);
            height = GETINT16(currentComponent + 6);

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
            if (currentFrameIsFirstFrame()) {
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
                    return SkCodec::kInvalidInput;
                }
            }

            const bool isLocalColormapDefined = SkToBool(currentComponent[8] & 0x80);
            // The three low-order bits of currentComponent[8] specify the bits per pixel.
            const int numColors = 2 << (currentComponent[8] & 0x7);
            if (currentFrameIsFirstFrame()) {
                const int transPix = m_frames.empty() ? SkGIFColorMap::kNotFound
                                                      : m_frames[0]->transparentPixel();
                if (is_palette_index_valid(transPix)) {
                    m_firstFrameHasAlpha = true;
                } else {
                    const bool frameIsSubset = xOffset > 0 || yOffset > 0
                            || width < fScreenWidth
                            || height < fScreenHeight;
                    m_firstFrameHasAlpha = frameIsSubset;
                }
            }

            addFrameIfNecessary();
            SkGIFFrameContext* currentFrame = m_frames.back().get();
            currentFrame->setHeaderDefined();

            if (query == SkGIFSizeQuery) {
                // The decoder needs to stop, so we return here, before
                // flushing the buffer. Next time through, we'll be in the same
                // state, requiring the same amount in the buffer.
                return SkCodec::kSuccess;
            }


            currentFrame->setXYWH(xOffset, yOffset, width, height);
            currentFrame->setInterlaced(SkToBool(currentComponent[8] & 0x40));

            // Overlaying interlaced, transparent GIFs over
            // existing image data using the Haeberli display hack
            // requires saving the underlying image in order to
            // avoid jaggies at the transparency edges. We are
            // unprepared to deal with that, so don't display such
            // images progressively. Which means only the first
            // frame can be progressively displayed.
            // FIXME: It is possible that a non-transparent frame
            // can be interlaced and progressively displayed.
            currentFrame->setProgressiveDisplay(currentFrameIsFirstFrame());

            if (isLocalColormapDefined) {
                currentFrame->localColorMap().setNumColors(numColors);
                GETN(SK_BYTES_PER_COLORMAP_ENTRY * numColors, SkGIFImageColormap);
                break;
            }

            setAlphaAndRequiredFrame(currentFrame);
            GETN(1, SkGIFLZWStart);
            break;
        }

        case SkGIFImageColormap: {
            SkASSERT(!m_frames.empty());
            auto* currentFrame = m_frames.back().get();
            auto& cmap = currentFrame->localColorMap();
            cmap.setTablePosition(m_streamBuffer.markPosition());
            setAlphaAndRequiredFrame(currentFrame);
            GETN(1, SkGIFLZWStart);
            break;
        }

        case SkGIFSubBlock: {
            const size_t bytesInBlock = this->getOneByte();
            if (bytesInBlock)
                GETN(bytesInBlock, SkGIFLZW);
            else {
                // Finished parsing one frame; Process next frame.
                SkASSERT(!m_frames.empty());
                // Note that some broken GIF files do not have enough LZW blocks to fully
                // decode all rows but we treat it as frame complete.
                m_frames.back()->setComplete();
                GETN(1, SkGIFImageStart);
                if (lastFrameToParse >= 0 && m_frames.count() > lastFrameToParse) {
                    m_streamBuffer.flush();
                    return SkCodec::kSuccess;
                }
            }
            break;
        }

        case SkGIFDone: {
            m_parseCompleted = true;
            return SkCodec::kSuccess;
        }

        default:
            // We shouldn't ever get here.
            // This prevents attempting to continue reading this invalid stream.
            GETN(0, SkGIFDone);
            return SkCodec::kInvalidInput;
            break;
        }   // switch
        m_streamBuffer.flush();
    }
}

void SkGifImageReader::addFrameIfNecessary()
{
    if (m_frames.empty() || m_frames.back()->isComplete()) {
        const int i = m_frames.count();
        m_frames.emplace_back(new SkGIFFrameContext(i));
    }
}

SkEncodedInfo::Alpha SkGIFFrameContext::onReportedAlpha() const {
    // Note: We could correct these after decoding - i.e. some frames may turn out to be
    // independent and opaque if they do not use the transparent pixel, but that would require
    // checking whether each pixel used the transparent index.
    return is_palette_index_valid(this->transparentPixel()) ? SkEncodedInfo::kBinary_Alpha
                                                            : SkEncodedInfo::kOpaque_Alpha;
}

// FIXME: Move this method to close to doLZW().
bool SkGIFLZWContext::prepareToDecode()
{
    SkASSERT(m_frameContext->isDataSizeDefined() && m_frameContext->isHeaderDefined());

    // Since we use a codesize of 1 more than the datasize, we need to ensure
    // that our datasize is strictly less than the SK_MAX_DICTIONARY_ENTRY_BITS.
    if (m_frameContext->dataSize() >= SK_MAX_DICTIONARY_ENTRY_BITS)
        return false;
    clearCode = 1 << m_frameContext->dataSize();
    avail = clearCode + 2;
    oldcode = -1;
    codesize = m_frameContext->dataSize() + 1;
    codemask = (1 << codesize) - 1;
    datum = bits = 0;
    ipass = m_frameContext->interlaced() ? 1 : 0;
    irow = 0;

    // We want to know the longest sequence encodable by a dictionary with
    // SK_MAX_DICTIONARY_ENTRIES entries. If we ignore the need to encode the base
    // values themselves at the beginning of the dictionary, as well as the need
    // for a clear code or a termination code, we could use every entry to
    // encode a series of multiple values. If the input value stream looked
    // like "AAAAA..." (a long string of just one value), the first dictionary
    // entry would encode AA, the next AAA, the next AAAA, and so forth. Thus
    // the longest sequence would be SK_MAX_DICTIONARY_ENTRIES + 1 values.
    //
    // However, we have to account for reserved entries. The first |datasize|
    // bits are reserved for the base values, and the next two entries are
    // reserved for the clear code and termination code. In theory a GIF can
    // set the datasize to 0, meaning we have just two reserved entries, making
    // the longest sequence (SK_MAX_DICTIONARY_ENTIRES + 1) - 2 values long. Since
    // each value is a byte, this is also the number of bytes in the longest
    // encodable sequence.
    constexpr size_t kMaxSequence = SK_MAX_DICTIONARY_ENTRIES - 1;
    constexpr size_t kMaxBytes = (kMaxSequence + SK_DICTIONARY_WORD_SIZE - 1)
                         & ~(SK_DICTIONARY_WORD_SIZE - 1);

    // Now allocate the output buffer. We decode directly into this buffer
    // until we have at least one row worth of data, then call outputRow().
    // This means worst case we may have (row width - 1) bytes in the buffer
    // and then decode a sequence |kMaxBytes| long to append.
    rowBuffer.reset(m_frameContext->width() - 1 + kMaxBytes);
    rowIter = rowBuffer.begin();
    rowsRemaining = m_frameContext->height();

    // Clearing the whole suffix table lets us be more tolerant of bad data.
    for (int i = 0; i < clearCode; ++i) {
        std::fill_n(suffix[i].begin(), SK_DICTIONARY_WORD_SIZE, 0);
        suffix[i][0] = i;
        suffixLength[i] = 1;
        prefix[i] = i;  // ensure that we have a place to find firstchar
    }
    return true;
}

