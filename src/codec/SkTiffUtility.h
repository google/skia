/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTiffUtility_codec_DEFINED
#define SkTiffUtility_codec_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"

#include <cstddef>
#include <cstdint>
#include <memory>

namespace SkTiff {

// Constants for endian signature.
inline constexpr size_t kEndianSize = 4;
inline constexpr uint8_t kEndianBig[kEndianSize] = {'M', 'M', 0, 42};
inline constexpr uint8_t kEndianLittle[kEndianSize] = {'I', 'I', 42, 0};

// Constants for types.
inline constexpr uint16_t kTypeUnsignedByte = 1;
inline constexpr uint16_t kTypeAsciiString = 2;
inline constexpr uint16_t kTypeUnsignedShort = 3;
inline constexpr uint16_t kTypeUnsignedLong = 4;
inline constexpr uint16_t kTypeUnsignedRational = 5;
inline constexpr uint16_t kTypeSignedByte = 6;
inline constexpr uint16_t kTypeUndefined = 7;
inline constexpr uint16_t kTypeSignedShort = 8;
inline constexpr uint16_t kTypeSignedLong = 9;
inline constexpr uint16_t kTypeSignedRational = 10;
inline constexpr uint16_t kTypeSingleFloat = 11;
inline constexpr uint16_t kTypeDoubleFloat = 12;

inline constexpr size_t kSizeEntry = 12;
inline constexpr size_t kSizeShort = 2;
inline constexpr size_t kSizeLong = 4;

/*
 * Helper function for parsing a Tiff Image File Directory (IFD) structure. This structure is used
 * by EXIF tags, multi-picture, and maker note metadata.
 */
class ImageFileDirectory {
public:
    /*
     * Parse |data| to read the endian-ness into |outLittleEndian| and the IFD offset into
     * |outIfdOffset|. Return true if the endian-ness was successfully parsed and there was
     * the IFD offset was read.
     */
    static bool ParseHeader(const SkData* data, bool* outLittleEndian, uint32_t* outIfdOffset);

    /*
     * Create an object for parsing an IFD at offset |ifdOffset| inside |data| which has endianness
     * indicated by |littleEndian|. If |allowTruncated| is true, then parse as much of |data| as is
     * possible, otherwise reject any incomplete IFDs.
     */
    static std::unique_ptr<ImageFileDirectory> MakeFromOffset(sk_sp<SkData> data,
                                                              bool littleEndian,
                                                              uint32_t ifdOffset,
                                                              bool allowTruncated = false);

    /*
     * Return the number of entries.
     */
    uint16_t getNumEntries() const { return fNumEntries; }

    /*
     * Return the offset (within the specified SkData) of the next IFD in the list of IFDs.
     */
    uint32_t nextIfdOffset() const { return fNextIfdOffset; }

    /*
     * Return the tag, of a specific entry.
     */
    uint16_t getEntryTag(uint16_t entryIndex) const;

    /*
     * If |entryIndex| has type unsigned short (3), unsigned long (4), or signed rational (10), and
     * count |count|, then populate |values| with the data for the tag and return true. Otherwise
     * return false.
     */
    bool getEntryUnsignedShort(uint16_t entryIndex, uint32_t count, uint16_t* values) const {
        return getEntryValuesGeneric(entryIndex, kTypeUnsignedShort, count, values);
    }
    bool getEntryUnsignedLong(uint16_t entryIndex, uint32_t count, uint32_t* values) const {
        return getEntryValuesGeneric(entryIndex, kTypeUnsignedLong, count, values);
    }
    bool getEntrySignedRational(uint16_t entryIndex, uint32_t count, float* values) const {
        return getEntryValuesGeneric(entryIndex, kTypeSignedRational, count, values);
    }
    bool getEntryUnsignedRational(uint16_t entryIndex, uint32_t count, float* values) const {
        return getEntryValuesGeneric(entryIndex, kTypeUnsignedRational, count, values);
    }

    /*
     * If |entryIndex| has type undefined (7), then return the bytes specified by the count field
     * and the offset (read from the value field as an unsigned long).
     */
    sk_sp<SkData> getEntryUndefinedData(uint16_t entryIndex) const;

private:
    static bool IsValidType(uint16_t type);
    static size_t BytesForType(uint16_t type);

    ImageFileDirectory(sk_sp<SkData> data,
                       bool littleEndian,
                       uint32_t offset,
                       uint16_t ifdNumEntries,
                       uint32_t ifdNextOffset);

    /*
     * Return the tag, type, count, and data for the specified entry. Return false if the type
     * is invalid, or if the data in the IFD is out of bounds.
     */
    bool getEntryRawData(uint16_t entryIndex,
                         uint16_t* outTag,
                         uint16_t* outType,
                         uint32_t* outCount,
                         const uint8_t** outData,
                         size_t* outDataSize) const;

    /*
     * Helper function for assorted getTag functions.
     */
    bool getEntryValuesGeneric(uint16_t entryIndex,
                               uint16_t type,
                               uint32_t count,
                               void* values) const;

    // The data that the IFD indexes into.
    const sk_sp<SkData> fData;

    // True if the data is little endian.
    const bool fLittleEndian;

    // The offset where the IFD starts.
    const uint32_t fOffset;

    // The number of entries of the IFD (read from the first 2 bytes at the IFD offset).
    const uint16_t fNumEntries;

    // The offset of the next IFD (read from the next 4 bytes after the IFD entries).
    const uint32_t fNextIfdOffset;
};

}  // namespace SkTiff

#endif
