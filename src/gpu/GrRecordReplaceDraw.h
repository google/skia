/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRecordReplaceDraw_DEFINED
#define GrRecordReplaceDraw_DEFINED

#include "SkChecksum.h"
#include "SkDrawPictureCallback.h"
#include "SkImage.h"
#include "SkRect.h"
#include "SkTDynamicHash.h"

class SkBBoxHierarchy;
class SkBitmap;
class SkCanvas;
class SkImage;
class SkMatrix;
class SkPaint;
class SkPicture;
class SkRecord;

// GrReplacements collects op ranges that can be replaced with
// a single drawBitmap call (using a precomputed bitmap).
class GrReplacements {
public:
    // All the operations between fStart and fStop (inclusive) will be replaced with
    // a single drawBitmap call using fPos, fImage and fPaint.
    class ReplacementInfo {
    public:
        struct Key {
            Key(uint32_t pictureID, unsigned int start, const SkMatrix& ctm)
            : fPictureID(pictureID)
            , fStart(start)
            , fCTM(ctm) {
                fCTM.getType(); // force initialization of type so hashes match

                // Key needs to be tightly packed.
                GR_STATIC_ASSERT(sizeof(Key) == sizeof(uint32_t) +      // picture ID
                                                sizeof(int) +           // start
                                                9 * sizeof(SkScalar)    // 3x3 from CTM
                                                +sizeof(uint32_t));     // matrix's type
            }

            bool operator==(const Key& other) const { 
                return fPictureID == other.fPictureID &&
                       fStart == other.fStart &&
                       fCTM.cheapEqualTo(other.fCTM); // TODO: should be fuzzy
            }

            uint32_t     pictureID() const { return fPictureID; }
            unsigned int start() const { return fStart; }

        private:
            const uint32_t     fPictureID;
            const unsigned int fStart;
            const SkMatrix     fCTM;
        };

        static const Key& GetKey(const ReplacementInfo& layer) { return layer.fKey; }
        static uint32_t Hash(const Key& key) {
            return SkChecksum::Murmur3(reinterpret_cast<const uint32_t*>(&key), sizeof(Key));
        }

        ReplacementInfo(uint32_t pictureID, unsigned int start, const SkMatrix& ctm)
            : fKey(pictureID, start, ctm)
            , fImage(NULL)
            , fPaint(NULL) {
        }
        ~ReplacementInfo() { fImage->unref(); SkDELETE(fPaint); }

        unsigned int start() const { return fKey.start(); }

        const Key       fKey;
        unsigned        fStop;
        SkIPoint        fPos;
        SkImage*        fImage;  // Owns a ref
        const SkPaint*  fPaint;  // Owned by this object

        SkIRect         fSrcRect;
    };

    ~GrReplacements() { this->freeAll(); }

    // Add a new replacement range.
    ReplacementInfo* newReplacement(uint32_t pictureID, unsigned int start, const SkMatrix& ctm);

    // look up a replacement range by its pictureID, start offset and the CTM
    // TODO: also need to add clip to lookup
    const ReplacementInfo* lookupByStart(uint32_t pictureID, size_t start, 
                                         const SkMatrix& ctm) const;

private:
    SkTDynamicHash<ReplacementInfo, ReplacementInfo::Key> fReplacementHash;

    void freeAll();
};

// Draw an SkPicture into an SkCanvas replacing saveLayer/restore blocks with
// drawBitmap calls.  A convenience wrapper around SkRecords::Draw.
// It returns the number of saveLayer/restore blocks replaced with drawBitmap calls.
int GrRecordReplaceDraw(const SkPicture*,
                        SkCanvas*,
                        const GrReplacements*,
                        const SkMatrix& initialMatrix,
                        SkDrawPictureCallback*);

#endif // GrRecordReplaceDraw_DEFINED
