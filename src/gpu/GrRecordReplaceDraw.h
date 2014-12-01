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
            Key(uint32_t pictureID, const SkMatrix& initialMat,
                const int* key, int keySize, bool copyKey = false)
            : fKeySize(keySize)
            , fFreeKey(copyKey) {
                fIDMatrix.fPictureID = pictureID;
                fIDMatrix.fInitialMat = initialMat;
                fIDMatrix.fInitialMat.getType(); // force initialization of type so hashes match

                if (copyKey) {
                    int* tempKey = SkNEW_ARRAY(int, keySize);
                    memcpy(tempKey, key, keySize * sizeof(int));
                    fKey = tempKey;
                } else {
                    fKey = key;
                }

                // The pictureID/matrix portion needs to be tightly packed.
                GR_STATIC_ASSERT(sizeof(IDMatrix) == sizeof(uint32_t)+                // pictureID
                                              9 * sizeof(SkScalar)+sizeof(uint32_t)); // matrix
            }

            ~Key() {
                if (fFreeKey) {
                    SkDELETE_ARRAY(fKey);
                }
            }
            bool operator==(const Key& other) const {
                if (fKeySize != other.fKeySize) {
                    return false;
                }
                return fIDMatrix.fPictureID == other.fIDMatrix.fPictureID &&
                       fIDMatrix.fInitialMat.cheapEqualTo(other.fIDMatrix.fInitialMat) &&
                       !memcmp(fKey, other.fKey, fKeySize * sizeof(int));
            }

            uint32_t hash() const {
                uint32_t hash = SkChecksum::Murmur3(reinterpret_cast<const uint32_t*>(fKey),
                                                    fKeySize * sizeof(int));
                return SkChecksum::Murmur3(reinterpret_cast<const uint32_t*>(&fIDMatrix),
                                           sizeof(IDMatrix), hash);
            }

        private:
            struct IDMatrix {
                uint32_t fPictureID;
                SkMatrix fInitialMat;
            }              fIDMatrix;

            const int*     fKey;
            const int      fKeySize;
            const bool     fFreeKey;
        };

        static const Key& GetKey(const ReplacementInfo& layer) { return layer.fKey; }
        static uint32_t Hash(const Key& key) { return key.hash(); }

        ReplacementInfo(uint32_t pictureID, const SkMatrix& initialMat,
                        const int* key, int keySize)
            : fKey(pictureID, initialMat, key, keySize, true)
            , fImage(NULL)
            , fPaint(NULL) {
        }
        ~ReplacementInfo() { fImage->unref(); SkDELETE(fPaint); }

        const Key       fKey;
        unsigned        fStop;
        SkIPoint        fPos;
        SkImage*        fImage;  // Owns a ref
        const SkPaint*  fPaint;  // Owned by this object

        SkIRect         fSrcRect;
    };

    ~GrReplacements() { this->freeAll(); }

    // Add a new replacement range.
    ReplacementInfo* newReplacement(uint32_t pictureID, const SkMatrix& initialMat,
                                    const int* key, int keySize);

    const ReplacementInfo* lookup(uint32_t pictureID, const SkMatrix& initalMat,
                                  const int* key, int keySize) const;

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
